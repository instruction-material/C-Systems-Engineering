"""Verify checked input, every 16-bit pattern, and independent learner tasks."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import shutil
import signal
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
PACK = ROOT / "CSE1-Hex-and-Binary-Inspector"
CC = shutil.which(os.environ.get("CSE_CC", "cc"))
assert CC, "An installed Clang or GCC compiler is required"
TASK = os.environ.get("CSE_PARENT_TASK_ID", "c17-inspector-native-contracts")
LOG = Path(os.environ.get("CSE_VALIDATION_LOG", ROOT / "build/native-inspector-processes.jsonl"))
LOG.parent.mkdir(parents=True, exist_ok=True)
SCAFFOLDS = [
    """    /* TODO: compute the signed 16-bit interpretation in an int32_t. */
    (void)value;
    return INT32_MIN;""",
    """    /* TODO: extract position 0..3; return UINT_MAX for other positions. */
    (void)value;
    (void)position;
    return UINT_MAX;""",
    """    /* TODO: return 1 when bit 15 is set, otherwise return 0. */
    (void)value;
    return -1;""",
]
# Arithmetic decompositions distinguish this learner completion from the
# reference's widened subtraction, bit shift, and bit mask.
COMPLETIONS = [
    """    return (int32_t)(value % 32768u) - (value >= 32768u ? 32768 : 0);""",
    """    if (position >= 4) {
        return UINT_MAX;
    }
    unsigned int divisor = 1;
    for (unsigned int i = 0; i < position; ++i) {
        divisor *= 16;
    }
    return ((unsigned int)value / divisor) % 16;""",
    """    return value / 32768u != 0;""",
]
INVALID = [
    "", " ", "\t", "-1", "+1", "1 ", " 1", "1\t", "1\n",
    "12junk", "0x", "0X", "0xG", "0xFFFFx", "0x10000", "65536",
    "999999999999999999999999", "0b101", "1.0", "1e2", "0o77",
    "FF", "0x-1", "0x+1", "18446744073709551615",
    "18446744073709551616", "0xFFFFFFFFFFFFFFFF", "0xFFFFFFFFFFFFFFFFF",
]
VALID = [
    (None, 43981), ("0", 0), ("077", 77), ("0x0F", 15), ("255", 255),
    ("256", 256), ("32767", 32767), ("32768", 32768), ("43981", 43981),
    ("65535", 65535), ("0xFFFF", 65535),
]


def run(args, cwd, expected=0, contains=None, timeout=120):
    args = [str(a) for a in args]
    started = datetime.datetime.now(datetime.timezone.utc).isoformat()
    begin = time.monotonic()
    child = subprocess.Popen(args, cwd=cwd, stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE, start_new_session=True)
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
        record = dict(parentTaskId=TASK, cwd=str(cwd), command=args,
                      pid=child.pid, startTime=started,
                      endTime=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                      exitCode=child.returncode, timeoutSeconds=timeout,
                      durationSeconds=round(time.monotonic() - begin, 3),
                      childProcessGroupCleanup=cleanup)
        with LOG.open("a") as file:
            file.write(json.dumps(record) + "\n")
    output = out.decode(errors="replace")
    text = (out + err).decode(errors="replace")
    correct = child.returncode != 0 if expected == "failure" else child.returncode == expected
    assert correct, (args, child.returncode, text)
    if contains:
        assert contains in text, (args, contains, text)
    return output, text


def source_identity():
    return {str(path.relative_to(ROOT)): hashlib.sha256(path.read_bytes()).hexdigest()
            for path in sorted(PACK.rglob("*")) if path.is_file()
            and not any(part.startswith("build") for part in path.relative_to(PACK).parts)}


def expected_output(value):
    signed = value if value < 32768 else value - 65536
    digits = f"{value:04X}"
    bits = f"{value:016b}"
    grouped = "_".join(bits[i:i + 4] for i in range(0, 16, 4))
    return (
        "hex/binary inspector\n"
        f"unsigned 16-bit  : {value}\n"
        f"signed 16-bit    : {signed}\n"
        f"hex              : 0x{digits}\n"
        f"binary           : {grouped}\n"
        f"nibble groups    : {' | '.join(digits)}\n"
        f"high byte        : 0x{value // 256:02X}\n"
        f"low byte         : 0x{value % 256:02X}\n"
        f"top bit set?     : {'yes' if value >= 32768 else 'no'}\n"
        f"two's complement : the same bits name {value} as unsigned or {signed} as signed.\n"
    )


before = source_identity()
assert before
counts = dict(roles=0, bitPatterns=0, scalarChecks=0, argumentChecks=0,
              validCliCases=0, invalidCliCases=0, unfinishedCasesRejected=0,
              completedLearnerRoles=0, controlledRegressionsRejected=0)
with tempfile.TemporaryDirectory(prefix="c17-inspector-") as directory:
    temp = Path(directory)
    reference = None
    for role, completed in [("starter", False), ("solution", False), ("starter", True)]:
        folder = temp / (role + ("-completed" if completed else ""))
        shutil.copytree(PACK / role, folder, ignore=shutil.ignore_patterns("build", "build-*"))
        if completed:
            path = folder / "representation.c"
            content = path.read_text()
            for scaffold, completion in zip(SCAFFOLDS, COMPLETIONS):
                assert content.count(scaffold) == 1
                content = content.replace(scaffold, completion)
            path.write_text(content)
            assert all((folder / name).read_bytes() == (PACK / role / name).read_bytes()
                       for name in ["main.c", "input.c", "input.h", "representation.h",
                                    "tests.c", "CMakeLists.txt"])
        name = "cse1_hex_binary_inspector_" + role
        build = folder / "build"
        run(["cmake", "-S", folder, "-B", build, "-DCMAKE_BUILD_TYPE=Debug",
             "-DCMAKE_C_COMPILER=" + CC, "-DCSE_SANITIZERS=ON"], ROOT)
        run(["cmake", "--build", build, "--parallel", "2"], ROOT)
        unfinished = role == "starter" and not completed
        run(["ctest", "--test-dir", build, "--output-on-failure"], ROOT,
            expected="failure" if unfinished else 0)
        if unfinished:
            for argument in [None, "0", "65535"]:
                args = [build / name] + ([] if argument is None else [argument])
                out, _ = run(args, folder, expected=78, contains="Unfinished")
                assert out == ""
                counts["unfinishedCasesRejected"] += 1
        else:
            out, _ = run([build / (name + "_tests")], folder)
            assert out == "65536 bit patterns and 42 argument checks passed\n", out
            counts["bitPatterns"] += 65536
            counts["scalarChecks"] += 65536 * 6
            counts["argumentChecks"] += 42
            for argument, value in VALID:
                args = [build / name] + ([] if argument is None else [argument])
                out, _ = run(args, folder)
                assert out == expected_output(value), (argument, out)
                counts["validCliCases"] += 1
        for argument in INVALID:
            out, _ = run([build / name, argument], folder,
                         expected=2, contains="Input must be")
            assert out == "", argument
            counts["invalidCliCases"] += 1
        out, _ = run([build / name, "0", "1"], folder, expected=2, contains="Usage:")
        assert out == ""
        counts["invalidCliCases"] += 1
        if completed:
            counts["completedLearnerRoles"] += 1
        else:
            counts["roles"] += 1
        if role == "solution":
            reference = (folder, build, name)

    folder, build, name = reference
    mutations = [
        ("representation.c", "widened >= INT32_C(32768)", "widened > INT32_C(32768)",
         "signed interpretation"),
        ("representation.c", "position * 4", "position * 3", "nibble extraction"),
        ("representation.c", "value & 0x8000u", "value & 0x4000u", "top bit"),
        ("input.c", "value > (UINT16_MAX - digit) / base",
         "value > (UINT32_C(1000000) - digit) / base", "invalid input accepted"),
        ("input.c", "uint32_t base = 10;", "uint32_t base = 8;",
         "valid input rejected or misread"),
    ]
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

assert source_identity() == before, "Native validation must not edit the supplied handouts"
print(json.dumps(dict(event="verified-c17-inspector-contracts", compiler=CC,
                      **counts, sourceFilesUnchanged=True,
                      temporaryBuildCleanup=True, log=str(LOG))))
