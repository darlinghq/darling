#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
"""
Host test against an installed Darling: the launcher restores the host
terminal settings when it is ended by a signal.

usage: python3 tests/launcher/termios-on-signal.py [path/to/darling]

The launcher puts its terminal in raw mode while a guest runs. The guest
here ignores SIGTERM, SIGHUP, SIGQUIT and SIGINT, so the launcher itself
must end and restore the terminal:
  1. SIGTERM: the launcher exits with status 128 + SIGTERM;
  2. SIGINT twice: the first one goes to the guest only, the second one
     ends the launcher with status 128 + SIGINT.
In both cases ICANON and ECHO are set again on the terminal afterwards.
"""

import os
import pty
import signal
import subprocess
import sys
import termios
import time

darling = sys.argv[1] if len(sys.argv) > 1 else "darling"
GUEST = "trap '' TERM HUP QUIT INT; echo READY; sleep 30"


def lflag(fd):
    return termios.tcgetattr(fd)[3]


def wait_exit(proc, seconds):
    deadline = time.time() + seconds
    while time.time() < deadline:
        rc = proc.poll()
        if rc is not None:
            return rc
        time.sleep(0.1)
    return None


def run_case(name, signals, expected_status):
    master, slave = pty.openpty()
    proc = subprocess.Popen([darling, "shell", GUEST],
                            stdin=slave, stdout=slave, stderr=slave,
                            start_new_session=True)
    try:
        out = b""
        deadline = time.time() + 60
        while b"READY" not in out and time.time() < deadline:
            try:
                out += os.read(master, 4096)
            except OSError:
                break
        if b"READY" not in out:
            print(f"FAIL {name}: guest did not start: {out!r}")
            return False
        if lflag(slave) & termios.ICANON:
            print(f"FAIL {name}: launcher did not put the terminal in raw mode")
            return False

        for i, sig in enumerate(signals):
            os.kill(proc.pid, sig)
            if i + 1 < len(signals):
                time.sleep(0.5)
                if proc.poll() is not None:
                    print(f"FAIL {name}: launcher ended after the first signal")
                    return False

        rc = wait_exit(proc, 5)
        if rc is None:
            print(f"FAIL {name}: launcher still running after the signal")
            return False
        status = rc if rc >= 0 else 128 - rc
        if status != expected_status:
            print(f"FAIL {name}: exit status {status}, expected {expected_status}")
            return False
        flags = lflag(slave)
        if not (flags & termios.ICANON) or not (flags & termios.ECHO):
            print(f"FAIL {name}: terminal left in raw mode")
            return False
        print(f"PASS {name}")
        return True
    finally:
        if proc.poll() is None:
            proc.kill()
            proc.wait()
        os.close(master)
        os.close(slave)


ok = run_case("SIGTERM", [signal.SIGTERM], 128 + signal.SIGTERM)
ok = run_case("SIGINT twice", [signal.SIGINT, signal.SIGINT], 128 + signal.SIGINT) and ok
sys.exit(0 if ok else 1)
