#!/usr/bin/env python3
# Copyright (C) 2026 Darling Developers
"""Test the guest environment the launcher builds (setupShellspawnEnv in src/startup/darling.c).

Usage: binfmt-launcher.py <darling.c> [--run]

The function is extracted and compiled against mocks so that the test observes the SHELLSPAWN_SETENV
commands without starting a container. In particular, the binfmt guest marker must be set by the
launcher itself and must never be copied from the invoking user's environment.
"""
import argparse
import pathlib
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument("source", type=pathlib.Path)
parser.add_argument("--run", action="store_true", help="compile and run the generated harness")
args = parser.parse_args()

source = args.source.read_text()
try:
    start = source.index("void setupShellspawnEnv(int sockfd)")
    end = source.index("\nvoid setupWorkingDir(int sockfd)", start)
except ValueError:
    raise SystemExit("unsupported launcher source shape")
function = source[start:end]

header = r"""
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SHELLSPAWN_SETENV 7
static const char* recorded[64];
static int recorded_count;
static char* prefix = "/prefix";
static char g_loginName[256] = "user";
static char* test_environ[] = {
    "__darling_binfmt_guest=evil",
    "__darling_binfmt_launch=1",
    "KEEP=1",
    "DROP=1",
    NULL
};
char** environ = test_environ;
void pushShellspawnCommand(int sockfd, int command, const char* argument)
{
    (void)sockfd;
    if (command == SHELLSPAWN_SETENV && recorded_count < 64)
        recorded[recorded_count++] = argument;
}
bool launcherGuestMayGet(const char* var) { return strcmp(var, "KEEP=1") == 0; }
bool lookUpLoginName(void) { return true; }
"""
tail = r"""
int main(void)
{
    int guest_markers = 0, failures = 0, kept = 0;
    setupShellspawnEnv(0);

    for (int i = 0; i < recorded_count; i++)
    {
        if (strcmp(recorded[i], "__darling_binfmt_guest=1") == 0)
            guest_markers++;
        if (strcmp(recorded[i], "__darling_binfmt_guest=evil") == 0 ||
            strcmp(recorded[i], "__darling_binfmt_launch=1") == 0 ||
            strcmp(recorded[i], "DROP=1") == 0)
            failures++;
        if (strcmp(recorded[i], "KEEP=1") == 0)
            kept++;
    }

    if (guest_markers != 1 || failures != 0 || kept != 1)
    {
        printf("FAIL: guest markers=%d forbidden=%d kept=%d\n", guest_markers, failures, kept);
        return 1;
    }
    puts("PASS: the launcher sets one clean binfmt guest marker");
    return 0;
}
"""
code = header + function + tail
with tempfile.TemporaryDirectory(prefix="darling-binfmt-launcher-") as tmp:
    generated = pathlib.Path(tmp) / "binfmt-launcher-generated.c"
    binary = pathlib.Path(tmp) / "binfmt-launcher-test"
    generated.write_text(code)
    if args.run:
        subprocess.run(["cc", "-std=c11", "-Wall", "-Werror", str(generated), "-o", str(binary)], check=True)
        raise SystemExit(subprocess.run([str(binary)]).returncode)
    print(generated.read_text())
