"""Observe each native fault boundary, then verify the three-line repair."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
PACK = ROOT / "CSE-09-unit-11-systems-tooling"
CC = shutil.which(os.environ.get("CSE_CC", "cc"))
assert CC, "An installed Clang or GCC compiler is required"
NM = shutil.which("nm")
OBJDUMP = shutil.which("objdump") or shutil.which("llvm-objdump")
DEBUGGER = shutil.which("gdb") or shutil.which("lldb")
assert NM and OBJDUMP and DEBUGGER, f"Installed tools required: nm={NM}, objdump={OBJDUMP}, debugger={DEBUGGER}"
os.environ["UBSAN_OPTIONS"] = "halt_on_error=1:print_stacktrace=1"
TASK = os.environ.get("CSE_PARENT_TASK_ID", "c17-tooling-native-contracts")
LOG = Path(os.environ.get("CSE_VALIDATION_LOG", ROOT / "build/native-tooling-processes.jsonl"))
LOG.parent.mkdir(parents=True, exist_ok=True)
EXPECTED_TRACE = (
    "timer product: 1501000\n"
    "temperature: -4000 centi-C -> -4000 centi-F\n"
    "voltage: 512/1023 at 3300 mV -> 1652 mV\n"
    "timer: 1501 us at 1000 Hz -> 2 ticks\n"
)

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





def replace_once(file, old, new):
    text = file.read_text()
    assert text.count(old) == 1, (file, old)
    file.write_text(text.replace(old, new))


def configure(source, build, sanitized):
    run(["cmake", "-S", source, "-B", build, "-DCMAKE_BUILD_TYPE=Debug",
         "-DCMAKE_C_COMPILER=" + CC, "-DCSE_SANITIZERS=" + ("ON" if sanitized else "OFF")], ROOT)


before = source_identity()
assert before
counts = dict(roles=2, nativeCases=0, unfinishedCompileRejected=0,
              formatDiagnoses=0, linkDiagnoses=0, sanitizerDiagnoses=0,
              completedLearnerRoles=0, objectSymbolInspections=0,
              debuggerObservations=0, disassemblyInspections=0)
with tempfile.TemporaryDirectory(prefix="c17-tooling-") as directory:
    temp = Path(directory)
    source = temp / "starter"
    shutil.copytree(PACK / "starter", source, ignore=shutil.ignore_patterns("build", "build-*"))
    build = source / "build-sanitized"
    configure(source, build, True)
    run(["cmake", "--build", build, "--parallel", "2"], ROOT, expected="failure", contains="format")
    counts["unfinishedCompileRejected"] += 1
    counts["formatDiagnoses"] += 1
    replace_once(source / "main.c", 'printf("timer product: %d\\n", product);',
                 'printf("timer product: %" PRIu64 "\\n", product);')
    run(["cmake", "--build", build, "--parallel", "2"], ROOT,
        expected="failure", contains="duration_to_ticks")
    counts["linkDiagnoses"] += 1
    implementation = list(build.rglob("engineering_math.c.o"))
    driver = list(build.rglob("main.c.o"))
    assert len(implementation) == len(driver) == 1
    defined = run([NM, implementation[0]], ROOT)
    requested = run([NM, driver[0]], ROOT)
    assert re.search(r"\bT\s+_?duration_to_tick\s*$", defined, re.M), defined
    assert re.search(r"\bU\s+_?duration_to_ticks\s*$", requested, re.M), requested
    counts["objectSymbolInspections"] += 1
    replace_once(source / "engineering_math.c", "MathStatus duration_to_tick(",
                 "MathStatus duration_to_ticks(")
    run(["cmake", "--build", build, "--parallel", "2"], ROOT)
    run([build / "cse11_tooling_starter_tests"], source,
        expected="failure", contains="signed integer overflow")
    counts["sanitizerDiagnoses"] += 1
    replace_once(source / "engineering_math.c", "(int64_t)(centi_celsius * 9) / 5 + 3200",
                 "(int64_t)centi_celsius * 9 / 5 + 3200")
    run(["cmake", "--build", build, "--clean-first", "--parallel", "2"], ROOT)
    run(["ctest", "--test-dir", build, "--output-on-failure"], ROOT)
    assert run([build / "cse11_tooling_starter_tests"], source) == "52 engineering math checks passed\n"
    assert run([build / "cse11_tooling_starter"], source) == EXPECTED_TRACE
    counts["nativeCases"] += 52
    counts["completedLearnerRoles"] += 1
    assert all((source / name).read_bytes() == (PACK / "starter" / name).read_bytes()
               for name in ["engineering_math.h", "tests.c", "CMakeLists.txt"])

    normal = source / "build-debugger"
    configure(source, normal, False)
    run(["cmake", "--build", normal, "--parallel", "2"], ROOT)
    run(["ctest", "--test-dir", normal, "--output-on-failure"], ROOT)
    executable = normal / "cse11_tooling_starter"
    assert re.search(r"\bT\s+_?duration_to_ticks\s*$", run([NM, executable], ROOT), re.M)
    assembly = run([OBJDUMP, "-d", executable], ROOT)
    assert "duration_to_ticks" in assembly
    counts["disassemblyInspections"] += 1
    if Path(DEBUGGER).name == "gdb":
        observation = run([DEBUGGER, "--batch", "-ex", "set pagination off", "-ex", "break duration_to_ticks",
                           "-ex", "run", "-ex", "print duration_us", "-ex", "print frequency_hz",
                           "-ex", "continue", executable], source)
        assert "duration_to_ticks" in observation and "Breakpoint" in observation
        assert re.search(r"\$\d+ = 1501\b", observation) and re.search(r"\$\d+ = 1000\b", observation), observation
    else:
        observation = run([DEBUGGER, "--batch", "-o", "breakpoint set --name duration_to_ticks", "-o", "run",
                           "-o", "frame variable duration_us frequency_hz", "-o", "continue", executable], source)
        assert "duration_to_ticks" in observation and "stop reason = breakpoint" in observation
        assert "duration_us = 1501" in observation and "frequency_hz = 1000" in observation, observation
    assert EXPECTED_TRACE in observation, observation
    counts["debuggerObservations"] += 1

    reference = temp / "solution"
    shutil.copytree(PACK / "solution", reference, ignore=shutil.ignore_patterns("build", "build-*"))
    reference_build = reference / "build"
    configure(reference, reference_build, True)
    run(["cmake", "--build", reference_build, "--parallel", "2"], ROOT)
    run(["ctest", "--test-dir", reference_build, "--output-on-failure"], ROOT)
    assert run([reference_build / "cse11_tooling_solution_tests"], reference) == "52 engineering math checks passed\n"
    assert run([reference_build / "cse11_tooling_solution"], reference) == EXPECTED_TRACE
    counts["nativeCases"] += 52

assert before == source_identity(), "Verification must preserve the original learner/reference sources"
assert counts == dict(roles=2, nativeCases=104, unfinishedCompileRejected=1,
                      formatDiagnoses=1, linkDiagnoses=1, sanitizerDiagnoses=1,
                      completedLearnerRoles=1, objectSymbolInspections=1,
                      debuggerObservations=1, disassemblyInspections=1), counts
print(json.dumps(dict(event="verified-c17-tooling-contracts", compiler=CC, **counts,
                      reusedMathUtility=True, repairedOnlyMarkedLines=True,
                      originalSourcePreserved=True, temporaryBuildCleanup=True, log=str(LOG))))
