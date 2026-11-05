#!/usr/bin/env python3
# Host test: launchd job_new_subjob duplicates stdio paths correctly.
# usage: python3 tests/launchd-subjob-stdio.py [REPO_ROOT]
# The path duplication block in job_new_subjob() is extracted from src/launchd/src/core.c
# and compiled with ASan/UBSan against a minimal job structure.
# Verifies:
#  - When both stdin and stdout are set, stdoutpath copies j->stdoutpath (not stdinpath)
#  - When only stdout is set (stdin is NULL), subjob inherits stdout without calling strdup(NULL)
#  - When only stderr or only stdin is set, paths are copied accurately
#  - Memory is allocated cleanly without leaks or crashes

import os
import re
import subprocess
import sys
import tempfile

root = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
source = os.path.join(root, "src/launchd/src/core.c")
with open(source, encoding="utf-8") as f:
    text = f.read()

m = re.search(r'job_new_subjob\([^)]*\)\s*\{.*?(if\s*\(\s*j->stdinpath\s*\)\s*\{[^}]*\}\s*if\s*\(\s*j->stdoutpath\s*\)\s*\{[^}]*\}\s*if\s*\(\s*j->stderrpath\s*\)\s*\{[^}]*\})', text, re.S)
if not m:
    sys.exit("FAIL: stdio path duplication block not found in job_new_subjob() in core.c")

block = m.group(1)

probe_c = f"""#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

struct job_test {{
    char *stdinpath;
    char *stdoutpath;
    char *stderrpath;
}};

static void copy_stdio_paths(const struct job_test *j, struct job_test *nj) {{
{block}
}}

static int test_case(const char *in, const char *out, const char *err, const char *label) {{
    pid_t pid = fork();
    if (pid == -1) {{
        perror("fork");
        return 1;
    }}
    if (pid == 0) {{
        struct job_test j = {{
            .stdinpath = in ? strdup(in) : NULL,
            .stdoutpath = out ? strdup(out) : NULL,
            .stderrpath = err ? strdup(err) : NULL,
        }};
        struct job_test nj = {{ NULL, NULL, NULL }};

        copy_stdio_paths(&j, &nj);

        int ok = 1;
        if (in && (!nj.stdinpath || strcmp(nj.stdinpath, in) != 0)) {{
            printf("FAIL [%s]: stdinpath expected '%s', got '%s'\\n", label, in, nj.stdinpath ? nj.stdinpath : "(null)");
            ok = 0;
        }}
        if (!in && nj.stdinpath != NULL) {{
            printf("FAIL [%s]: stdinpath expected NULL, got '%s'\\n", label, nj.stdinpath);
            ok = 0;
        }}
        if (out && (!nj.stdoutpath || strcmp(nj.stdoutpath, out) != 0)) {{
            printf("FAIL [%s]: stdoutpath expected '%s', got '%s'\\n", label, out, nj.stdoutpath ? nj.stdoutpath : "(null)");
            ok = 0;
        }}
        if (!out && nj.stdoutpath != NULL) {{
            printf("FAIL [%s]: stdoutpath expected NULL, got '%s'\\n", label, nj.stdoutpath);
            ok = 0;
        }}
        if (err && (!nj.stderrpath || strcmp(nj.stderrpath, err) != 0)) {{
            printf("FAIL [%s]: stderrpath expected '%s', got '%s'\\n", label, err, nj.stderrpath ? nj.stderrpath : "(null)");
            ok = 0;
        }}
        if (!err && nj.stderrpath != NULL) {{
            printf("FAIL [%s]: stderrpath expected NULL, got '%s'\\n", label, nj.stderrpath);
            ok = 0;
        }}

        free(j.stdinpath);
        free(j.stdoutpath);
        free(j.stderrpath);
        free(nj.stdinpath);
        free(nj.stdoutpath);
        free(nj.stderrpath);

        _exit(ok ? 0 : 2);
    }}

    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFSIGNALED(status)) {{
        printf("FAIL [%s]: crashed with signal %d (%s)\\n", label, WTERMSIG(status), strsignal(WTERMSIG(status)));
        return 1;
    }}
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {{
        return 1;
    }}
    return 0;
}}

int main(void) {{
    int failed = 0;

    failed += test_case("/dev/null", "/var/log/stdout.log", "/var/log/stderr.log", "all paths set");
    failed += test_case(NULL, "/var/log/stdout.log", NULL, "stdout only (stdin NULL)");
    failed += test_case("/dev/zero", NULL, NULL, "stdin only");
    failed += test_case(NULL, NULL, "/var/log/stderr.log", "stderr only");
    failed += test_case(NULL, NULL, NULL, "all NULL");

    if (failed == 0) {{
        puts("PASS: launchd subjob stdio path duplication");
    }}
    return failed ? 1 : 0;
}}
"""

with tempfile.TemporaryDirectory() as td:
    src_file = os.path.join(td, "probe.c")
    bin_file = os.path.join(td, "probe")
    with open(src_file, "w", encoding="utf-8") as f:
        f.write(probe_c)
    
    cmd = ["clang", "-Wall", "-Werror", "-fsanitize=address,undefined", src_file, "-o", bin_file]
    res = subprocess.run(cmd, capture_output=True, text=True)
    if res.returncode != 0:
        print("Compilation failed:\n", res.stderr)
        sys.exit(1)
    
    run_env = os.environ.copy()
    run_env["ASAN_OPTIONS"] = "disable_coredump=1"
    run_res = subprocess.run([bin_file], capture_output=True, text=True, env=run_env)
    print(run_res.stdout, end="")
    if run_res.stderr:
        print(run_res.stderr, end="")
    sys.exit(run_res.returncode)
