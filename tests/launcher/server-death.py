#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
#
# Needs an installed Darling and a prefix that may be shut down (default
# ~/.darling, or the one named by DPREFIX). Starts the container, kills
# darlingserver with SIGKILL and checks that no process of the container
# survives and that `darling shutdown` then reports it as not running.
# Then starts the container again right after each of several shutdowns: a
# shutdown that returns before the old server is gone makes the next start
# fail with "exited. Try again".
import os
import signal
import subprocess
import sys
import time

prefix = os.environ.get("DPREFIX", os.path.expanduser("~/.darling"))
uid = os.getuid()


def container_processes():
    """Processes of ours that are the first process of a pid namespace, or inside one."""
    found = []
    for name in os.listdir("/proc"):
        if not name.isdigit():
            continue
        try:
            with open("/proc/%s/status" % name) as f:
                status = f.read()
        except OSError:
            continue
        if "State:\tZ" in status:
            continue
        fields = dict(line.split(":\t", 1) for line in status.splitlines() if ":\t" in line)
        if int(fields.get("Uid", "-1").split()[0]) != uid:
            continue
        if len(fields.get("NSpid", "").split()) > 1:
            found.append(int(name))
    return found


subprocess.run(["darling", "shutdown"], stderr=subprocess.DEVNULL)
time.sleep(2)
if subprocess.run(["darling", "shell", "/bin/echo", "up"], stdin=subprocess.DEVNULL).returncode != 0:
    print("FAIL: container did not start")
    sys.exit(1)

with open(os.path.join(prefix, ".init.pid")) as f:
    server = int(f.read().strip())
if not container_processes():
    print("FAIL: no container processes found after start")
    sys.exit(1)

os.kill(server, signal.SIGKILL)
for _ in range(100):
    if not container_processes():
        break
    time.sleep(0.1)
left = container_processes()
if left:
    print("FAIL: container processes outlived darlingserver: %s" % left)
    subprocess.run(["darling", "shutdown"])
    sys.exit(1)

result = subprocess.run(["darling", "shutdown"], stderr=subprocess.PIPE, text=True)
if result.returncode != 1 or "not running" not in result.stderr:
    print("FAIL: shutdown after the server died: rc=%d %r" % (result.returncode, result.stderr))
    sys.exit(1)

for i in range(20):
    subprocess.run(["darling", "shutdown"], stderr=subprocess.DEVNULL)
    result = subprocess.run(["darling", "shell", "/bin/echo", "up"], stdin=subprocess.DEVNULL,
                            stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True)
    if result.returncode != 0:
        print("FAIL: start right after shutdown %d: %s" % (i, result.stderr.strip()))
        sys.exit(1)

print("PASSED")
