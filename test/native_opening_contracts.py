"""Verify opening packs with fixed native commands and independent fixtures."""
import datetime
import json
import os
from pathlib import Path
import shutil
import signal
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
CC = shutil.which(os.environ.get("CSE_CC", "cc"))
assert CC, "A Clang or GCC compiler must already be installed"
TASK = os.environ.get("CSE_PARENT_TASK_ID", "c17-opening-native-contracts")
LOG = Path(os.environ.get("CSE_VALIDATION_LOG", ROOT / "build/native-opening-processes.jsonl"))
LOG.parent.mkdir(parents=True, exist_ok=True)
SETUP = "CSE-05-cse0-setup-and-tooling"
LINK = "CSE-06-unit-1-why-c-for-systems-work"
STRICT = [CC, "-std=c17", "-Wall", "-Wextra", "-Wpedantic", "-Werror"]
COUNTS = {"roles": 0, "nativeCases": 0, "unfinishedRolesRejected": 0,
          "completedLearnerRoles": 0, "controlledRegressionsRejected": 0,
          "linkFailuresRejected": 0, "unsafeFixtureDiagnosed": 0}


def run(args, cwd, expected=0, contains=None, timeout=120):
    started = datetime.datetime.now(datetime.timezone.utc).isoformat()
    start = time.monotonic()
    child = subprocess.Popen([str(a) for a in args], cwd=cwd,
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE,
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
        record = dict(parentTaskId=TASK, cwd=str(cwd), command=[str(a) for a in args],
                      pid=child.pid, startTime=started,
                      endTime=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                      exitCode=child.returncode, timeoutSeconds=timeout,
                      durationSeconds=round(time.monotonic() - start, 3),
                      childProcessGroupCleanup=cleanup)
        with LOG.open("a") as file:
            file.write(json.dumps(record) + "\n")
    text = (out + err).decode(errors="replace")
    correct = child.returncode != 0 if expected == "failure" else child.returncode == expected
    assert correct, (args, child.returncode, text)
    if contains:
        assert contains in text, (args, text)
    return out.decode(errors="replace"), text


def verify_role(temp, pack, role, completed=False):
    folder = ROOT / pack / role
    source = temp / (pack + "-" + role + ("-completed" if completed else ""))
    shutil.copytree(folder, source, ignore=shutil.ignore_patterns("build", "build-*"))
    if completed:
        if pack == SETUP:
            path = source / "main.c"
            before = path.read_text()
            scaffold = "    /* TODO: implement the three ranges declared in reading.h. */\n    (void)value;\n    return 0;"
            assert before.count(scaffold) == 1
            # Independent expression, rather than copying the reference branches.
            path.write_text(before.replace(scaffold, "    return (value > 100) - (value < 0);"))
        else:
            path = source / "labels.c"
            before = path.read_text()
            scaffold = "    /* TODO: return the four labels declared in reading.h. */\n    (void)category;\n    return NULL;"
            assert before.count(scaffold) == 1
            path.write_text(before.replace(scaffold,
                '    static const char* labels[] = {"below", "within", "above"};\n'
                '    if (category < -1 || category > 1) {\n'
                '        return "invalid";\n'
                '    }\n    return labels[category + 1];'))
    name = ("cse0_" if pack == SETUP else "cse1_link_") + role
    build = source / "build"
    run(["cmake", "-S", source, "-B", build, "-DCMAKE_BUILD_TYPE=Debug",
         "-DCMAKE_C_COMPILER=" + CC, "-DCSE_SANITIZERS=ON"], ROOT)
    run(["cmake", "--build", build, "--parallel", "2"], ROOT)
    unfinished = role == "starter" and not completed
    run(["ctest", "--test-dir", build, "--output-on-failure"], ROOT,
        expected="failure" if unfinished else 0)
    expected_trace = "-1 => -1\n0 => 0\n50 => 0\n100 => 0\n101 => 1\n" if pack == SETUP else "-1 => below\n0 => within\n50 => within\n100 => within\n101 => above\n"
    out, _ = run([build / name], source, expected=78 if unfinished else 0,
                 contains="unfinished" if unfinished else None)
    if unfinished:
        assert out == "", out
        COUNTS["unfinishedRolesRejected"] += 1
    else:
        assert out == expected_trace, out
        COUNTS["nativeCases"] += 9 if pack == SETUP else 10
    if completed:
        COUNTS["completedLearnerRoles"] += 1
    else:
        COUNTS["roles"] += 1
    return source, build, name


with tempfile.TemporaryDirectory(prefix="c17-opening-") as directory:
    temp = Path(directory)
    references = {}
    for pack in [SETUP, LINK]:
        verify_role(temp, pack, "starter")
        references[pack] = verify_role(temp, pack, "solution")
        verify_role(temp, pack, "starter", completed=True)

    source, build, name = references[SETUP]
    path = source / "main.c"
    correct = path.read_text()
    assert correct.count("value > 100") == 1
    path.write_text(correct.replace("value > 100", "value >= 100"))
    run(["cmake", "--build", build, "--clean-first", "--parallel", "2"], ROOT)
    run(["ctest", "--test-dir", build, "--output-on-failure"], ROOT,
        expected="failure", contains="value=100 expected=0 actual=1")
    COUNTS["controlledRegressionsRejected"] += 1
    path.write_text(correct)
    run(["cmake", "--build", build, "--clean-first", "--parallel", "2"], ROOT)
    run(["ctest", "--test-dir", build, "--output-on-failure"], ROOT)

    source, _, _ = references[LINK]
    run([CC, "-std=c17", "-E", "main.c", "-o", "main.i"], source)
    run([CC, "-std=c17", "-S", "main.c", "-o", "main.s"], source)
    run([*STRICT, "-c", "main.c", "reading.c", "labels.c"], source)
    run(["nm", "main.o", "reading.o", "labels.o"], source, contains="reading_label")
    run([CC, "main.o", "reading.o", "-o", "missing"], source,
        expected="failure", contains="reading_label")
    run([CC, "main.o", "reading.o", "labels.o", "labels.o", "-o", "duplicate"], source,
        expected="failure", contains="reading_label")
    COUNTS["linkFailuresRejected"] += 2
    run([CC, "main.o", "reading.o", "labels.o", "-o", "linked"], source)
    run([source / "linked"], source, contains="101 => above")

    probe = temp / "unsafe-probe"
    run([*STRICT, "-g", "-O0", "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
         ROOT / SETUP / "unsafe_probe.c", "-o", probe], ROOT)
    run([probe], temp, expected="failure", contains="stack-buffer-overflow")
    COUNTS["unsafeFixtureDiagnosed"] += 1

    build = temp / "root-build"
    run(["cmake", "-S", ROOT, "-B", build, "-DCMAKE_C_COMPILER=" + CC], ROOT)
    run(["cmake", "--build", build, "--parallel", "2"], ROOT)
    _, text = run(["ctest", "--test-dir", build, "--output-on-failure"], ROOT)
    assert "100% tests passed" in text, text
    inventory, _ = run(["ctest", "--test-dir", build, "--show-only=json-v1"], ROOT)
    assert {row["name"] for row in json.loads(inventory)["tests"]} == {
        "cse0_solution_contract", "cse1_link_solution_contract",
        "cse8_dispatch_solution_contract"
    }, inventory

print(json.dumps(dict(event="verified-c17-opening-contracts", compiler=CC,
                      **COUNTS, rootReferenceTests=3,
                      temporaryBuildCleanup=True, log=str(LOG))))
