#!/usr/bin/env python3
"""
Host test: shellspawn forwards signals to the foreground process group.
usage: python3 tests/shellspawn/forwarded-signals.py [REPO_ROOT]

Verifies:
1. In TTY mode: when a shell process has placed a child job in the foreground
   process group (via tcsetpgrp on its controlling terminal), sending
   SHELLSPAWN_SIGNAL delivers the signal to that foreground process group,
   even though the shell itself ignores the signal.
2. In non-TTY mode: when stdin is not a tty, the shell has no controlling
   terminal and therefore no foreground process group, so the signal goes
   to the shell's own process group, which includes the child job.
"""

import os
import re
import shlex
import subprocess
import sys
import tempfile

root = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
shellspawn_src = os.path.join(root, "src", "shellspawn", "shellspawn.c")
duct_src = os.path.join(root, "src", "shellspawn", "duct_signals.c")
shellspawn_inc = os.path.join(root, "src", "shellspawn")

if not os.path.exists(shellspawn_src):
    sys.exit(f"FAIL: shellspawn.c not found at {shellspawn_src}")

with open(shellspawn_src, "r", encoding="utf-8") as f:
    src_content = f.read()

# Build the test harness in a temporary directory
with tempfile.TemporaryDirectory(prefix="shellspawn-signal-test-") as td:
    stub_dir = os.path.join(td, "stub_inc")
    os.makedirs(os.path.join(stub_dir, "sys"), exist_ok=True)
    with open(os.path.join(stub_dir, "sys", "event.h"), "w") as f:
        f.write("/* empty stub for kqueue */\n")

    # A mock job process that acts like a shell running a foreground child job
    job_helper_src = os.path.join(td, "job_helper.c")
    with open(job_helper_src, "w") as f:
        f.write(r"""#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile sig_atomic_t g_got_signal = 0;
static void handle_sig(int sig) {
    (void)sig;
    g_got_signal = 1;
}

int main(void) {
    int tty_fd = -1;
    if (isatty(STDIN_FILENO)) {
        tty_fd = STDIN_FILENO;
    } else {
        tty_fd = open("/dev/tty", O_RDWR);
    }

    // Ignore terminal stop signals so child can set foreground pgrp
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    pid_t child = fork();
    if (child == 0) {
        // Child job: with a controlling terminal, it gets its own process
        // group and takes the foreground, as a job-control shell would do.
        // Without one, it stays in the shell's process group.
        if (tty_fd >= 0) {
            setpgid(0, 0);
            tcsetpgrp(tty_fd, getpgrp());
        }

        signal(SIGINT, handle_sig);

        // Tell test driver that foreground job is ready via stdout
        printf("READY\n");
        fflush(stdout);

        for (int i = 0; i < 50; i++) {
            if (g_got_signal) {
                _exit(42); // Success: foreground job received SIGINT
            }
            usleep(100000);
        }
        _exit(1); // Timed out without signal
    }

    // "Shell" parent: ignores SIGINT while waiting for child
    signal(SIGINT, SIG_IGN);

    int status = 0;
    waitpid(child, &status, 0);

    if (tty_fd >= 0 && tty_fd != STDIN_FILENO) {
        close(tty_fd);
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) == 42) {
        return 0; // Child received signal!
    }
    return 2; // Child did not receive signal
}
""")

    cc = shlex.split(os.environ.get("CC", "clang"))
    job_bin = os.path.join(td, "job_helper")
    subprocess.check_call(cc + ["-O2", job_helper_src, "-o", job_bin])

    # Harness wrapper that runs spawnShell against job_bin
    harness_c = os.path.join(td, "harness.c")
    # Patch main out of shellspawn.c and provide our test runner
    patched_shellspawn = re.sub(r"\bint\s+main\s*\([^)]*\)\s*\{", "static int unused_main(void) {", src_content)

    with open(harness_c, "w") as f:
        f.write(r"""#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <pty.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "duct_signals.h"
#include "shellspawn.h"

// Bring in shellspawn source directly
""")
        f.write(patched_shellspawn)
        f.write(r"""

static void send_cmd(int sock, uint32_t cmd, const void *data, uint32_t len) {
    struct shellspawn_cmd h = { cmd, len };
    write(sock, &h, sizeof(h));
    if (len > 0)
        write(sock, data, len);
}

static void send_go(int sock, int fds[3]) {
    struct shellspawn_cmd h = { SHELLSPAWN_GO, 0 };
    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));

    struct iovec iov = { &h, sizeof(h) };
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    char cmsg_buf[CMSG_SPACE(sizeof(int) * 3)];
    msg.msg_control = cmsg_buf;
    msg.msg_controllen = sizeof(cmsg_buf);

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int) * 3);
    memcpy(CMSG_DATA(cmsg), fds, sizeof(int) * 3);

    sendmsg(sock, &msg, 0);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <mode: tty|notty> <job_bin>\n", argv[0]);
        return 1;
    }
    const char *mode = argv[1];
    const char *job_bin = argv[2];

    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) {
        perror("socketpair");
        return 1;
    }

    int in_fd = -1, out_fd = -1, err_fd = -1;
    int pty_m = -1, pty_s = -1;
    int client_out_fd = -1;

    if (strcmp(mode, "tty") == 0) {
        if (openpty(&pty_m, &pty_s, NULL, NULL, NULL) < 0) {
            perror("openpty");
            return 1;
        }
        in_fd = pty_s;
        out_fd = dup(pty_s);
        err_fd = dup(pty_s);
        client_out_fd = pty_m;
    } else {
        // Non-tty: use pipes for stdio
        int p_in[2], p_out[2], p_err[2];
        if (pipe(p_in) < 0 || pipe(p_out) < 0 || pipe(p_err) < 0) {
            perror("pipe");
            return 1;
        }
        in_fd = p_in[0];
        out_fd = p_out[1];
        err_fd = p_err[1];
        close(p_in[1]);
        close(p_err[0]);
        client_out_fd = p_out[0];
    }

    pid_t sp_pid = fork();
    if (sp_pid == 0) {
        close(sv[0]);
        if (pty_m >= 0) close(pty_m);
        if (client_out_fd >= 0 && client_out_fd != pty_m) close(client_out_fd);

        spawnShell(sv[1]);
        _exit(0);
    }

    close(sv[1]);

    // Client side: configure shellspawn
    // SETEXEC -> job_bin
    send_cmd(sv[0], SHELLSPAWN_SETEXEC, job_bin, strlen(job_bin) + 1);

    // ADDARG -> argv[0]
    send_cmd(sv[0], SHELLSPAWN_ADDARG, job_bin, strlen(job_bin) + 1);

    int fds_to_send[3] = { in_fd, out_fd, err_fd };
    send_go(sv[0], fds_to_send);
    close(in_fd);
    close(out_fd);
    close(err_fd);

    // Wait for the foreground job in job_helper to signal readiness
    char line[64];
    memset(line, 0, sizeof(line));
    ssize_t n = read(client_out_fd, line, sizeof(line) - 1);
    if (n <= 0 || strstr(line, "READY") == NULL) {
        fprintf(stderr, "Foreground job never became ready (read %zd: '%s')\n", n, line);
        kill(sp_pid, SIGKILL);
        return 1;
    }

    // Give it 50ms to settle in foreground
    usleep(50000);

    // Send SHELLSPAWN_SIGNAL (LINUX_SIGINT = 2)
    int linux_sig = 2; // SIGINT
    send_cmd(sv[0], SHELLSPAWN_SIGNAL, &linux_sig, sizeof(linux_sig));

    // Wait for shellspawn to report child exit status
    int wstatus = -1;
    if (read(sv[0], &wstatus, sizeof(wstatus)) != sizeof(wstatus)) {
        fprintf(stderr, "Failed to read exit status from shellspawn\n");
        return 1;
    }
    close(sv[0]);
    if (client_out_fd >= 0) close(client_out_fd);

    waitpid(sp_pid, NULL, 0);

    if (wstatus == 0) {
        return 0; // Success!
    } else {
        fprintf(stderr, "Exit code was %d (expected 0)\n", wstatus);
        return 2;
    }
}
""")

    harness_bin = os.path.join(td, "harness")
    cmd = cc + [
        "-O2", "-D_GNU_SOURCE",
        "-I" + stub_dir,
        "-I" + shellspawn_inc,
        harness_c, duct_src,
        "-lutil", "-o", harness_bin
    ]
    res = subprocess.run(cmd, capture_output=True, text=True)
    if res.returncode != 0:
        print("Compilation failed:\n", res.stderr)
        sys.exit(1)

    # Test 1: TTY mode
    try:
        res_tty = subprocess.run([harness_bin, "tty", job_bin], timeout=5, capture_output=True, text=True)
        if res_tty.returncode != 0:
            print("FAIL: TTY mode failed:\n", res_tty.stderr)
            sys.exit(1)
    except subprocess.TimeoutExpired:
        print("FAIL: TTY mode timed out waiting for signal delivery")
        sys.exit(1)

    # Test 2: Non-TTY mode, no controlling terminal
    try:
        res_notty = subprocess.run([harness_bin, "notty", job_bin], timeout=5, capture_output=True, text=True)
        if res_notty.returncode != 0:
            print("FAIL: Non-TTY mode failed:\n", res_notty.stderr)
            sys.exit(1)
    except subprocess.TimeoutExpired:
        print("FAIL: Non-TTY mode timed out waiting for signal delivery")
        sys.exit(1)

    print("PASS: shellspawn forwarded signals reached foreground process group in both TTY and non-TTY modes")
