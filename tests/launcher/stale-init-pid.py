# Copyright (C) 2026 Tim Besard
# SPDX-License-Identifier: GPL-3.0-or-later
"""Check that the launcher's findInitProcess() takes a darlingserver that exited but was not reaped
(a zombie) for no server, and removes the prefix's pid file. The function is compiled from
darling.c against a temporary prefix; the "server" is a child of the harness."""
import argparse
import pathlib
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument("startup", type=pathlib.Path, help="src/startup")
args = parser.parse_args()
source = (args.startup / "darling.c").read_text()
start = source.index("static pid_t findInitProcess(void)\n{")
end = source.index("\n}\n", start) + 3
function = source[start:end]

header = r"""
#include <alloca.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "launcher-server.h"
static char prefix[4096];
static uid_t g_originalUid = 0, g_originalGid = 0;
"""
tail = r"""
static char pidPath[4200];

static void writePid(pid_t pid) {
    FILE *f = fopen(pidPath, "w");
    fprintf(f, "%d\n", (int)pid);
    fclose(f);
}

int main(int argc, char **argv) {
    int p[2], failed = 0;
    char byte;
    siginfo_t info;
    (void)argc;
    snprintf(prefix, sizeof(prefix), "%s", argv[1]);
    snprintf(pidPath, sizeof(pidPath), "%s/.init.pid", prefix);
    if (pipe(p) != 0)
        return 1;
    pid_t server = fork();
    if (server == 0) {
        prctl(PR_SET_NAME, "darlingserver", 0, 0, 0);
        if (write(p[1], "x", 1) != 1)
            _exit(1);
        for (;;)
            pause();
    }
    if (server < 0 || read(p[0], &byte, 1) != 1)
        return 1;

    writePid(server);
    pid_t found = findInitProcess();
    bool kept = access(pidPath, F_OK) == 0;
    printf("%s running server: found=%d pidfile=%d\n", found == server && kept ? "PASS" : "FAIL", (int)found, kept);
    failed += !(found == server && kept);

    if (kill(server, SIGKILL) != 0 || waitid(P_PID, server, &info, WEXITED | WNOWAIT) != 0) {
        printf("FAIL cannot make the server a zombie\n");
        return 1;
    }
    found = findInitProcess();
    kept = access(pidPath, F_OK) == 0;
    printf("%s zombie server: found=%d pidfile=%d\n", found == 0 && !kept ? "PASS" : "FAIL", (int)found, kept);
    failed += !(found == 0 && !kept);

    waitpid(server, NULL, 0);
    return failed ? 1 : 0;
}
"""
with tempfile.TemporaryDirectory(prefix="darling-stale-init-pid-") as tmp:
    generated = pathlib.Path(tmp) / "stale-init-pid.c"
    binary = pathlib.Path(tmp) / "stale-init-pid"
    prefix = pathlib.Path(tmp) / "prefix"
    prefix.mkdir()
    generated.write_text(header + function + tail)
    subprocess.run(["cc", "-std=gnu11", "-Wall", "-Werror", "-D_GNU_SOURCE", "-I", str(args.startup), str(generated),
                    str(args.startup / "launcher-server.c"), "-o", str(binary)], check=True)
    raise SystemExit(subprocess.run([str(binary), str(prefix)]).returncode)
