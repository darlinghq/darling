#!/usr/bin/env python3
# Host test: the LSInfo.m lookups report errors through outError correctly.
# usage: python3 tests/launchservices-outerror-null.py [REPO_ROOT]
# Every guarded `*outError = ...;` statement is cut out of LSInfo.m verbatim and
# built as plain C with a stand-in CFErrorCreate. Each one is run twice:
#  - with outError == NULL (callers may pass NULL): it must not dereference it
#  - with outError pointing at a NULL CFErrorRef (the usual `CFErrorRef err = NULL;
#    f(..., &err)`): the error must be stored
# A guard that tests *outError instead of outError crashes in the first case and
# never stores the error in the second. The test also checks that no write to
# *outError in the file escapes the extraction.

import os
import re
import shlex
import subprocess
import sys
import tempfile

root = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
source = os.path.join(root, "src/frameworks/CoreServices/src/LaunchServices/LSInfo.m")
with open(source, encoding="utf-8") as f:
    text = f.read()

guard = re.compile(r"^([ \t]*if \(([^\n]*)\)\n[ \t]*\*outError = ([^\n]*);)$", re.M)
sites = [(text.count("\n", 0, m.start()) + 1, m.group(1), m.group(3) == "NULL") for m in guard.finditer(text)]
writes = len(re.findall(r"\*outError = ", text))
if not sites:
    sys.exit("FAIL: no outError writes found in LSInfo.m")
if writes != len(sites):
    sys.exit(f"FAIL: {writes} writes to *outError but only {len(sites)} follow an if; update the test")

cases = []
for i, (line, stmt, _) in enumerate(sites):
    cases.append(f"static void site_{i}(CFErrorRef *outError) {{\n\tconst int status = kLSApplicationNotFoundErr;\n{stmt}\n}}")
# a site that clears the error starts from a stale one; the others start from NULL
table = ", ".join(f"{{ {line}, site_{i}, {int(clears)} }}" for i, (line, _, clears) in enumerate(sites))

probe = f"""#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
typedef struct __CFError *CFErrorRef;
typedef long CFIndex;
static struct __CFError {{ int unused; }} sentinel;
#define kCFErrorDomainOSStatus 0
enum {{ paramErr = -50, fnfErr = -43, kLSApplicationNotFoundErr = -10814 }};
static CFErrorRef CFErrorCreate(const void *a, int domain, CFIndex code, const void *info)
{{ (void) a; (void) domain; (void) code; (void) info; return &sentinel; }}
{chr(10).join(cases)}
static const struct {{ int line; void (*run)(CFErrorRef *); int clears; }} sites[] = {{ {table} }};
int main(void) {{
    int failed = 0;
    for (size_t i = 0; i < sizeof(sites) / sizeof(sites[0]); i++) {{
        pid_t pid = fork();
        if (pid == 0) {{ sites[i].run(NULL); _exit(0); }}
        int st = 0;
        waitpid(pid, &st, 0);
        if (!WIFEXITED(st) || WEXITSTATUS(st) != 0) {{
            printf("FAIL: LSInfo.m:%d dereferences a NULL outError\\n", sites[i].line);
            failed = 1;
            continue;
        }}
        CFErrorRef err = sites[i].clears ? &sentinel : NULL;
        sites[i].run(&err);
        if (err != (sites[i].clears ? NULL : &sentinel)) {{
            printf("FAIL: LSInfo.m:%d does not store through a valid outError\\n", sites[i].line);
            failed = 1;
        }}
    }}
    if (!failed)
        printf("PASS: %zu outError sites tolerate NULL and store the error\\n", sizeof(sites) / sizeof(sites[0]));
    return failed;
}}
"""

with tempfile.TemporaryDirectory(prefix="ls-outerror-") as tmp:
    with open(os.path.join(tmp, "probe.c"), "w", encoding="utf-8") as f:
        f.write(probe)
    cc = shlex.split(os.environ.get("CC", "clang"))
    subprocess.check_call(cc + ["-O0", "-w", os.path.join(tmp, "probe.c"), "-o", os.path.join(tmp, "probe")])
    run = subprocess.run([os.path.join(tmp, "probe")], stdout=subprocess.PIPE, text=True)
    print(run.stdout.strip())
    if run.returncode != 0:
        sys.exit("launchservices outError regression")
