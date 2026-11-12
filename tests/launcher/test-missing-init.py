# Copyright (C) 2026 Lubos Dolezel
# SPDX-License-Identifier: GPL-3.0-or-later
"""Exercise the launcher refusal branch with a mocked process scan."""
import argparse
import pathlib
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument("source", type=pathlib.Path)
parser.add_argument("--run", action="store_true", help="compile and run the generated harness")
args = parser.parse_args()
source = args.source.read_text()
start = source.index("\t// If prefix's init is not running, start it up")
end = source.index("\n#if USE_LINUX_4_11_HACK", start)
branch = source[start:end]
for required in (
    "if (pidInit == 0)",
    "launcherFindPrefixServer(",
    "unlink(socketPath)",
    "setupWorkdir()",
    "spawnInitProcess()",
    "putInitPid(pidInit)",
):
    if required not in branch:
        raise SystemExit("unsupported launcher source shape: " + required)

header = r"""
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#define INSTALL_PREFIX "/opt/darling"
#define SHELLSPAWN_SOCKPATH "/var/run/shellspawn.sock"
static const char *prefix = "/synthetic-prefix";
static uid_t g_originalUid = 1000;
static int scan_result;
static pid_t detected_server;
static int unlinks, setups, spawns, pid_writes;
int launcherFindPrefixServer(const char *proc, const char *p, uid_t uid,
    const char *exe, pid_t *server) {
    (void)proc; (void)p; (void)uid; (void)exe;
    *server = detected_server;
    return scan_result;
}
static int orphan_result;
int launcherFindOrphanedInit(const char *proc, const char *p, uid_t uid,
    const char *mldr, pid_t *init) {
    (void)proc; (void)p; (void)uid; (void)mldr;
    *init = 8003;
    return orphan_result;
}
static void useInvokingIds(void) {}
static void restoreRootIds(void) {}
int unlink(const char *path) { (void)path; ++unlinks; return 0; }
static void setupWorkdir(void) { ++setups; }
static pid_t spawnInitProcess(void) { ++spawns; return 8002; }
static void putInitPid(pid_t pid) { (void)pid; ++pid_writes; }
int faccessat(int fd, const char *path, int mode, int flags) {
    (void)fd; (void)path; (void)mode; (void)flags; return 0;
}
unsigned int sleep(unsigned int seconds) { (void)seconds; return 0; }
static int startup(pid_t pidInit) {
"""
tail = r"""
    return 0;
}
static int check(const char *label, int scan, pid_t live, int expected_rc, int expected_mutations) {
    orphan_result = 0;
    scan_result = scan; detected_server = live;
    unlinks = setups = spawns = pid_writes = 0;
    int rc = startup(0);
    int mutations = unlinks + setups + spawns + pid_writes;
    int passed = rc == expected_rc && mutations == expected_mutations;
    printf("%s %s rc=%d unlink=%d setup=%d spawn=%d pidwrite=%d\n",
        passed ? "PASS" : "FAIL", label, rc, unlinks, setups, spawns, pid_writes);
    return !passed;
}
int main(void) {
    int failed = 0;
    failed += check("missing-or-invalid-pidfile/live-server", 1, 8001, 1, 0);
    failed += check("inspection-error", -1, 0, 1, 0);
    failed += check("verified-no-server", 0, 0, 0, 4);
    // a container whose darlingserver exited must not be started over
    int expected_rc = 1;
    scan_result = 0; detected_server = 0; orphan_result = 1;
    unlinks = setups = spawns = pid_writes = 0;
    int rc = startup(0);
    int mutations = unlinks + setups + spawns + pid_writes;
    printf("%s orphaned-container rc=%d mutations=%d\n",
        rc == expected_rc && mutations == 0 ? "PASS" : "FAIL", rc, mutations);
    failed += !(rc == expected_rc && mutations == 0);
    return failed ? 1 : 0;
}
"""
code = header + branch + tail
with tempfile.TemporaryDirectory(prefix="darling-missing-init-") as tmp:
    generated = pathlib.Path(tmp) / "missing-init-generated.c"
    binary = pathlib.Path(tmp) / "missing-init-test"
    generated.write_text(code)
    if args.run:
        subprocess.run(["cc", "-std=c11", "-Wall", "-Werror", str(generated), "-o", str(binary)], check=True)
        raise SystemExit(subprocess.run([str(binary)]).returncode)
