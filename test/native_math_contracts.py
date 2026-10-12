"""Check fixed units, rounding, range failures, and an independent learner."""
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
PACK = ROOT / "CSE-08-unit-10-engineering-math-in-code"
CC = shutil.which(os.environ.get("CSE_CC", "cc"))
assert CC, "An installed Clang or GCC compiler is required"
TASK = os.environ.get("CSE_PARENT_TASK_ID", "c17-math-native-contracts")
LOG = Path(os.environ.get("CSE_VALIDATION_LOG", ROOT / "build/native-math-processes.jsonl"))
LOG.parent.mkdir(parents=True, exist_ok=True)
EXPECTED_TRACE = (
    "temperature: -4000 centi-C -> -4000 centi-F\n"
    "voltage: 512/1023 at 3300 mV -> 1652 mV\n"
    "timer: 1501 us at 1000 Hz -> 2 ticks\n"
)
SCAFFOLDS = [
    """    /* TODO: truncate scaled conversion toward zero, check range, then store. */
    (void)centi_celsius;
    (void)centi_fahrenheit;
    return MATH_BAD_ARGUMENT;""",
    """    /* TODO: round upward to whole ticks, check range, then store. */
    (void)duration_us;
    (void)frequency_hz;
    (void)ticks;
    return MATH_BAD_ARGUMENT;""",
]
# A quotient/remainder decomposition and nonzero-product ceiling distinguish
# this completion from the reference's multiplication/division implementations.
COMPLETIONS = [
    """    if (centi_fahrenheit == NULL) {
        return MATH_BAD_ARGUMENT;
    }
    const int64_t input = centi_celsius;
    const int64_t converted = (input / 5) * 9 + ((input % 5) * 9) / 5 + 3200;
    if (converted > INT32_MAX || converted < INT32_MIN) {
        return MATH_OVERFLOW;
    }
    *centi_fahrenheit = (int32_t)converted;
    return MATH_OK;""",
    """    if (frequency_hz == 0 || ticks == NULL) {
        return MATH_BAD_ARGUMENT;
    }
    const uint64_t scaled = (uint64_t)frequency_hz * duration_us;
    const uint64_t converted = scaled == 0 ? 0 : 1 + (scaled - 1) / 1000000;
    if (converted > UINT32_MAX) {
        return MATH_OVERFLOW;
    }
    *ticks = (uint32_t)converted;
    return MATH_OK;""",
]

def run(args, cwd, expected=0, contains=None, timeout=120):
    args = [str(arg) for arg in args]
    started = datetime.datetime.now(datetime.timezone.utc).isoformat()
    start = time.monotonic()
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
        record = dict(parentTaskId=TASK, cwd=str(cwd), command=args, pid=child.pid,
                      startTime=started, endTime=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                      exitCode=child.returncode, timeoutSeconds=timeout,
                      durationSeconds=round(time.monotonic() - start, 3), childProcessGroupCleanup=cleanup)
        with LOG.open("a") as file:
            file.write(json.dumps(record) + "\n")
    text = (out + err).decode(errors="replace")
    valid = child.returncode != 0 if expected == "failure" else child.returncode == expected
    assert valid, (args, child.returncode, text)
    if contains:
        assert contains in text, (args, text)
    return out.decode(errors="replace")


def source_identity():
    result = {}
    for folder, directories, files in os.walk(PACK):
        directories[:] = [name for name in directories
                          if not name.startswith("build") and
                          name not in {".git", "node_modules", "dist", ".cache"}]
        for name in files:
            path = Path(folder) / name
            result[str(path.relative_to(ROOT))] = hashlib.sha256(path.read_bytes()).hexdigest()
    return result



before = source_identity()
assert before
counts = dict(roles=0, nativeCases=0, unfinishedRolesRejected=0,
              completedLearnerRoles=0, controlledRegressionsRejected=0)
with tempfile.TemporaryDirectory(prefix="c17-math-") as directory:
    temp = Path(directory)
    reference = None
    for role, completed in [("starter", False), ("solution", False), ("starter", True)]:
        source = temp / (role + ("-completed" if completed else ""))
        shutil.copytree(PACK / role, source, ignore=shutil.ignore_patterns("build", "build-*"))
        if completed:
            file = source / "engineering_math.c"
            text = file.read_text()
            for scaffold, completion in zip(SCAFFOLDS, COMPLETIONS):
                assert text.count(scaffold) == 1
                text = text.replace(scaffold, completion)
            file.write_text(text)
            assert all((source / name).read_bytes() == (PACK / role / name).read_bytes()
                       for name in ["main.c", "engineering_math.h", "tests.c", "CMakeLists.txt"])
        build = source / "build"
        name = "cse10_math_" + role
        run(["cmake", "-S", source, "-B", build, "-DCMAKE_BUILD_TYPE=Debug",
             "-DCMAKE_C_COMPILER=" + CC, "-DCSE_SANITIZERS=ON"], ROOT)
        run(["cmake", "--build", build, "--parallel", "2"], ROOT)
        unfinished = role == "starter" and not completed
        run(["ctest", "--test-dir", build, "--output-on-failure"], ROOT,
            expected="failure" if unfinished else 0)
        output = run([build / name], source, expected=78 if unfinished else 0,
                     contains="unfinished or incorrect engineering conversion" if unfinished else None)
        if unfinished:
            assert output == "", output
            counts["unfinishedRolesRejected"] += 1
        else:
            assert output == EXPECTED_TRACE, output
            assert run([build / (name + "_tests")], source) == "52 engineering math checks passed\n"
            counts["nativeCases"] += 52
        if completed:
            counts["completedLearnerRoles"] += 1
        else:
            counts["roles"] += 1
        if role == "solution":
            reference = (source, build, name)

    source, build, name = reference
    file = source / "engineering_math.c"
    original = file.read_text()
    mutations = [
        ("(int64_t)centi_celsius * 9 / 5 + 3200", "(int64_t)(centi_celsius / 5) * 9 + 3200", "temperature case"),
        ("product / 1000000 + (product % 1000000 != 0)", "product / 1000000", "timer case"),
        ("remainder >= (denominator + 1) / 2", "remainder >= denominator", "voltage case"),
        ("    if (value > UINT32_MAX) {\n        return MATH_OVERFLOW;\n    }\n", "", "timer case"),
    ]
    for correct, regression, failure in mutations:
        assert original.count(correct) == 1
        file.write_text(original.replace(correct, regression))
        run(["cmake", "--build", build, "--clean-first", "--parallel", "2"], ROOT)
        run([build / (name + "_tests")], source, expected="failure", contains=failure)
        counts["controlledRegressionsRejected"] += 1
    file.write_text(original)
    run(["cmake", "--build", build, "--clean-first", "--parallel", "2"], ROOT)
    run(["ctest", "--test-dir", build, "--output-on-failure"], ROOT)

assert before == source_identity(), "Verification must leave the original roles unchanged"
assert counts == dict(roles=2, nativeCases=104, unfinishedRolesRejected=1,
                      completedLearnerRoles=1, controlledRegressionsRejected=4), counts
print(json.dumps(dict(event="verified-c17-math-contracts", compiler=CC, **counts,
                      fixedUnits=True, signedAndUnsignedExtremes=True,
                      roundingRules=True, failureOutputsPreserved=True,
                      originalSourcePreserved=True, temporaryBuildCleanup=True, log=str(LOG))))
