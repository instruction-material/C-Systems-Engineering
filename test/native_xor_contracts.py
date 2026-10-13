"""Check every XOR byte/key pair, CLI grammar, independent learners and regressions."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import shutil
import signal
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
PACK = ROOT / "CSE3-XOR-Encoder-Decoder"
CC = shutil.which(os.environ.get("CSE_CC", "cc"))
assert CC
TASK = os.environ.get("CSE_PARENT_TASK_ID", "c17-xor-native-contracts")
LOG = Path(os.environ.get("CSE_VALIDATION_LOG", ROOT / "build/native-xor-processes.jsonl"))
LOG.parent.mkdir(parents=True, exist_ok=True)

def run(args, cwd, expected=0, contains=None, timeout=120):
    args = [str(value) for value in args]
    start = datetime.datetime.now(datetime.timezone.utc).isoformat()
    child = subprocess.Popen(args, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                             start_new_session=True)
    cleanup = "process exited"
    try:
        out, err = child.communicate(timeout=timeout)
    except BaseException:
        os.killpg(child.pid, signal.SIGTERM)
        try:
            child.communicate(timeout=5)
        except subprocess.TimeoutExpired:
            os.killpg(child.pid, signal.SIGKILL)
            child.communicate()
        cleanup = "child process group terminated"
        raise
    finally:
        with LOG.open("a") as file:
            file.write(json.dumps(dict(parentTaskId=TASK, cwd=str(cwd), command=args,
                pid=child.pid, startTime=start, endTime=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                exitCode=child.returncode, timeoutSeconds=timeout, childProcessGroupCleanup=cleanup)) + "\n")
    text = (out + err).decode(errors="replace")
    assert (child.returncode != 0 if expected == "failure" else child.returncode == expected), (args, child.returncode, text)
    if contains:
        assert contains in text, (args, contains, text)
    if expected in [2, 78]:
        assert not out and err, (args, out, err)
    return out.decode(errors="replace"), text

SCAFFOLDS = [
"""    /* TODO: validate output, choose 0x5A for NULL text, otherwise use the checked parser. */
    (void)text;
    (void)key;
    return false;""",
"""    /* TODO: return the byte-level XOR result in 0..255. */
    (void)input;
    (void)key;
    return UINT_MAX;"""]
COMPLETIONS = [
"""    if (key == NULL) return false;
    if (text != NULL) return parse_xor_key_input(text, key);
    *key = 90u;
    return true;""",
"""    unsigned int result = 0;
    unsigned int power = 1;
    for (unsigned int index = 0; index < 8u; ++index) {
        const unsigned int a = (unsigned int)input / power % 2u;
        const unsigned int b = (unsigned int)key / power % 2u;
        if (a != b) result += power;
        power *= 2u;
    }
    return result;"""]
INVALID = ["", " ", "\t", "-1", "+1", "1 ", " 1", "1\t", "1\n", "garbage",
           "12junk", "0x", "0X", "0xG", "0xFFjunk", "256", "0x100", "0b1",
           "0o77", "1.0", "1e2", "FF", "0x-1", "0x+1",
           "999999999999999999999999", "18446744073709551615",
           "18446744073709551616", "0xFFFFFFFFFFFFFFFF"]
VALID_KEYS = [(str(value), value) for value in range(256)] + [
    ("077", 77), ("0x00FF", 255), ("0X0f", 15), ("000", 0),
    ("0000000000000000000000000000000000000000", 0)]
UNIT_OUTPUT = "65536 byte/key pairs, 196608 model checks and 1740 key checks passed\n"

def expected_output(message, key, reference):
    original = message.encode("utf-8")
    transformed = [sum(((byte // (2**bit) % 2 + key // (2**bit) % 2) % 2) * (2**bit)
                       for bit in range(8)) for byte in original]
    dump = lambda data: "".join(f"{byte:02X} " for byte in data)
    text = ("xor encoder/decoder\n" + f"message      : {message}\n" +
            f"key          : 0x{key:02X}\n" + f"input bytes  : {dump(original)}\n" +
            f"encoded bytes: {dump(transformed)}\n" + f"decoded text : {message}\n")
    if reference:
        text += "trace        :\n"
        for index, (byte, encoded) in enumerate(zip(original, transformed)):
            text += f"  [{index}] 0x{byte:02X} ^ 0x{key:02X} -> 0x{encoded:02X} -> 0x{byte:02X}\n"
        text += "explanation  : xor with the same key twice returns the original byte.\n"
    return text

def identity():
    return {str(path.relative_to(PACK)): hashlib.sha256(path.read_bytes()).hexdigest()
            for path in sorted(PACK.rglob("*")) if path.is_file()
            and not any(part.startswith("build") for part in path.relative_to(PACK).parts)}

before = identity()
assert len(before) == 15
counts = dict(roles=0, bytePairs=0, modelChecks=0, keyChecks=0, validCliCases=0,
              invalidCliCases=0, unfinishedCasesRejected=0, completedLearnerRoles=0,
              controlledRegressionsRejected=0, releaseProfiles=0, releaseBytePairs=0,
              directBuilds=0)
with tempfile.TemporaryDirectory(prefix="c17-xor-") as directory:
    temp = Path(directory)
    reference = None
    for role, completed in [("starter", False), ("solution", False), ("starter", True)]:
        folder = temp / (role + ("-completed" if completed else ""))
        shutil.copytree(PACK / role, folder, ignore=shutil.ignore_patterns("build", "build-*"))
        if completed:
            target = folder / "transform.c"
            text = target.read_text()
            for scaffold, completion in zip(SCAFFOLDS, COMPLETIONS):
                assert text.count(scaffold) == 1
                text = text.replace(scaffold, completion)
            target.write_text(text)
            assert all((folder / name).read_bytes() == (PACK / role / name).read_bytes()
                       for name in ["main.c", "input.c", "input.h", "transform.h", "tests.c", "CMakeLists.txt"])
        name = "cse3_xor_encoder_" + role
        build = folder / "build"
        run(["cmake", "-S", folder, "-B", build, "-DCMAKE_BUILD_TYPE=Debug",
             "-DCMAKE_C_COMPILER=" + CC, "-DCSE_SANITIZERS=ON"], ROOT)
        run(["cmake", "--build", build, "--parallel", "2"], ROOT)
        unfinished = role == "starter" and not completed
        run(["ctest", "--test-dir", build, "--output-on-failure"], ROOT,
            expected="failure" if unfinished else 0)
        direct = folder / "xor-demo"
        run([CC, "-std=c17", "-Wall", "-Wextra", "-Wpedantic", "-Wconversion", "-Werror",
             "main.c", "input.c", "transform.c", "-o", direct], folder)
        run([direct, "systems", "0x5A"], folder, expected=78 if unfinished else 0,
            contains="Unfinished" if unfinished else "decoded text : systems")
        counts["directBuilds"] += 1
        if unfinished:
            for argv in [[], [""], ["systems", "0"], ["systems", "255"]]:
                run([build / name, *argv], folder, expected=78, contains="Unfinished")
                counts["unfinishedCasesRejected"] += 1
        else:
            out, _ = run([build / (name + "_tests")], folder)
            assert out == UNIT_OUTPUT, out
            counts["bytePairs"] += 65536
            counts["modelChecks"] += 196608
            counts["keyChecks"] += 1740
            reference_trace = role == "solution"
            cases = [([], "systems", 90), (["systems"], "systems", 90),
                     (["hello world"], "hello world", 90)]
            cases += [(["systems", text], "systems", value) for text, value in VALID_KEYS]
            cases += [([message, str(key)], message, key)
                      for message in ["", "A", "hello world", "é", "x" * 127, "é" * 63]
                      for key in [0, 90, 255]]
            assert len(cases) == 282
            for argv, message, key in cases:
                out, _ = run([build / name, *argv], folder)
                assert out == expected_output(message, key, reference_trace), (argv, out)
                counts["validCliCases"] += 1
            release = folder / "release-build"
            run(["cmake", "-S", folder, "-B", release, "-DCMAKE_BUILD_TYPE=Release",
                 "-DCMAKE_C_COMPILER=" + CC, "-DCSE_SANITIZERS=OFF"], ROOT)
            run(["cmake", "--build", release, "--parallel", "2"], ROOT)
            run(["ctest", "--test-dir", release, "--output-on-failure"], ROOT)
            out, _ = run([release / (name + "_tests")], folder)
            assert out == UNIT_OUTPUT
            counts["releaseProfiles"] += 1
            counts["releaseBytePairs"] += 65536
        for argument in INVALID:
            run([build / name, "systems", argument], folder, expected=2, contains="Key must be")
            counts["invalidCliCases"] += 1
        for message in ["x" * 128, "é" * 64]:
            run([build / name, message], folder, expected=2, contains="at most 127 bytes")
            counts["invalidCliCases"] += 1
        run([build / name, "systems", "90", "extra"], folder, expected=2, contains="Usage:")
        counts["invalidCliCases"] += 1
        if completed:
            counts["completedLearnerRoles"] += 1
        else:
            counts["roles"] += 1
        if role == "solution":
            reference = (folder, build)
    folder, build = reference
    mutations = [
        ("transform.c", "(unsigned int)input ^ (unsigned int)key", "(unsigned int)input | (unsigned int)key", "Contract failure: XOR"),
        ("transform.c", "*key = 0x5Au;", "*key = 0x5Bu;", "default key mismatch"),
        ("transform.c", "if (key == NULL) return false;", "if (key == NULL || text == NULL) return false;", "default key rejected"),
        ("input.c", "value > (UINT8_MAX - digit) / base", "value > (UINT32_C(1000000) - digit) / base", "invalid key accepted"),
        ("input.c", "uint32_t base = 10;", "uint32_t base = 8;", "valid key rejected or misread"),
        ("input.c", "if (digit >= base || value > (UINT8_MAX - digit) / base) return false;",
         "if (digit >= base || value > (UINT8_MAX - digit) / base) { *output = 0; return false; }", "invalid key output changed")]
    for filename, correct, regression, failure in mutations:
        path = folder / filename
        original = path.read_text()
        assert original.count(correct) == 1
        path.write_text(original.replace(correct, regression))
        run(["cmake", "--build", build, "--clean-first", "--parallel", "2"], ROOT)
        run(["ctest", "--test-dir", build, "--output-on-failure"], ROOT,
            expected="failure", contains=failure)
        counts["controlledRegressionsRejected"] += 1
        path.write_text(original)
    run(["cmake", "--build", build, "--clean-first", "--parallel", "2"], ROOT)
    run(["ctest", "--test-dir", build, "--output-on-failure"], ROOT)
assert identity() == before
print(json.dumps(dict(event="verified-c17-xor-contracts", compiler=CC, **counts,
                      referenceTrace=True, learnerTraceOptional=True,
                      sourceFilesUnchanged=True, temporaryBuildCleanup=True, log=str(LOG))))
