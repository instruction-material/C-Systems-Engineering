"""Check indirect dispatch, independent learner completion, and rejected regressions."""
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
PACK = ROOT / "CSE-07-unit-8-function-pointers-and-dispatch"
CC = shutil.which(os.environ.get("CSE_CC", "cc"))
assert CC, "An installed Clang or GCC compiler is required"
TASK = os.environ.get("CSE_PARENT_TASK_ID", "c17-dispatch-native-contracts")
LOG = Path(os.environ.get("CSE_VALIDATION_LOG", ROOT / "build/native-dispatch-processes.jsonl"))
LOG.parent.mkdir(parents=True, exist_ok=True)
EXPECTED_TRACE = (
    "store(4): ok, level=4\n"
    "take(2): ok, level=2\n"
    "store(99): full, level=2\n"
    "clear(0): ok, level=0\n"
    "unknown(0): unknown, level=0\n"
)
SCAFFOLD = """    /* TODO: validate the contract, select the first exact name, and call its handler. */
    (void)state;
    (void)entries;
    (void)count;
    (void)command;
    (void)argument;
    return COMMAND_UNKNOWN;"""
# This independent completion searches backwards but retains the earliest match.
COMPLETION = """    if (state == NULL || command == NULL ||
        state->level < 0 || state->level > DEVICE_CAPACITY ||
        (count != 0 && entries == NULL)) {
        return COMMAND_BAD_ARGUMENT;
    }
    const CommandEntry* selected = NULL;
    size_t remaining = count;
    while (remaining != 0) {
        const CommandEntry* entry = &entries[--remaining];
        if (entry->name == NULL || entry->handler == NULL) {
            return COMMAND_BAD_ARGUMENT;
        }
        if (strcmp(command, entry->name) == 0) {
            selected = entry;
        }
    }
    return selected == NULL ? COMMAND_UNKNOWN : selected->handler(state, argument);"""


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
with tempfile.TemporaryDirectory(prefix="c17-dispatch-") as directory:
    temp = Path(directory)
    reference = None
    for role, completed in [("starter", False), ("solution", False), ("starter", True)]:
        source = temp / (role + ("-completed" if completed else ""))
        shutil.copytree(PACK / role, source, ignore=shutil.ignore_patterns("build", "build-*"))
        if completed:
            file = source / "dispatch.c"
            text = file.read_text()
            assert text.count(SCAFFOLD) == 1
            file.write_text(text.replace(SCAFFOLD, COMPLETION))
            assert all((source / name).read_bytes() == (PACK / role / name).read_bytes()
                       for name in ["main.c", "handlers.c", "dispatch.h", "tests.c", "CMakeLists.txt"])
        build = source / "build"
        name = "cse8_dispatch_" + role
        run(["cmake", "-S", source, "-B", build, "-DCMAKE_BUILD_TYPE=Debug",
             "-DCMAKE_C_COMPILER=" + CC, "-DCSE_SANITIZERS=ON"], ROOT)
        run(["cmake", "--build", build, "--parallel", "2"], ROOT)
        unfinished = role == "starter" and not completed
        run(["ctest", "--test-dir", build, "--output-on-failure"], ROOT,
            expected="failure" if unfinished else 0)
        output = run([build / name], source, expected=78 if unfinished else 0,
                     contains="unfinished or incorrect dispatch" if unfinished else None)
        if unfinished:
            assert output == "", output
            counts["unfinishedRolesRejected"] += 1
        else:
            assert output == EXPECTED_TRACE, output
            assert run([build / (name + "_tests")], source) == "31 dispatch checks passed\n"
            counts["nativeCases"] += 31
        if completed:
            counts["completedLearnerRoles"] += 1
        else:
            counts["roles"] += 1
        if role == "solution":
            reference = (source, build)

    source, build = reference
    file = source / "dispatch.c"
    correct = file.read_text()
    for original, replacement, failed_case in [
        ("strcmp(entries[i].name, command) == 0",
         "strncmp(entries[i].name, command, 1) == 0", "failed: storex"),
        ("entries[i].handler(state, argument)",
         "entries[i].handler(state, argument / 2)", "failed: callback receives unchanged argument")
    ]:
        assert correct.count(original) == 1
        file.write_text(correct.replace(original, replacement))
        run(["cmake", "--build", build, "--clean-first", "--parallel", "2"], ROOT)
        run(["ctest", "--test-dir", build, "--output-on-failure"], ROOT,
            expected="failure", contains=failed_case)
        counts["controlledRegressionsRejected"] += 1
    file.write_text(correct)
    run(["cmake", "--build", build, "--clean-first", "--parallel", "2"], ROOT)
    run(["ctest", "--test-dir", build, "--output-on-failure"], ROOT)

assert source_identity() == before, "Verification must preserve original source files"
print(json.dumps(dict(event="verified-c17-dispatch-contracts", compiler=CC, **counts,
                      callerSuppliedCallbacks=True, firstExactMatch=True,
                      originalSourcePreserved=True, temporaryBuildCleanup=True, log=str(LOG))))
