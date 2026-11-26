#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
#
# Runs a guest program that opens modal dialogs and acts as the user of them: when the program says that
# dialog N is shown (the line "DIALOG_READY N"), a key is typed, Return to accept the dialog or Escape to cancel
# it, as the list of keys says. The program ends the run with the line "PASS <its name>".
#
#   carbon_dialog_user.py MODE PROGRAM KEYS
#
# MODE
#   x11       the X server in $DISPLAY (it needs a window manager and xdotool)
#   wayland   the compositor that $WAYLAND_DISPLAY and $XDG_RUNTIME_DIR name (sway on a headless output or a
#             private kwin_wayland --virtual --no-lockscreen); $KEYS_INJECTOR is the program that types for the
#             user, as for carbon_key_events.sh: it takes a pause in milliseconds and evdev key codes with 1 or 0
#             for press and release (virtual-keyboard-keys on sway, kwin-keys on KWin)
# PROGRAM is the guest program (the host path), KEYS for example Return,Escape: the key for dialog 1, 2, ...
# DARLING names the launcher (default: darling).
import os
import subprocess
import sys
import time

EVDEV = {"Return": 28, "Escape": 1}


def wait_for(path, text, seconds):
    end = time.time() + seconds
    while time.time() < end:
        try:
            if text in open(path).read():
                return True
        except FileNotFoundError:
            pass
        time.sleep(0.2)
    return False


def read(path):
    try:
        return open(path).read()
    except FileNotFoundError:
        return ""


def main():
    mode, program, keys = sys.argv[1], sys.argv[2], sys.argv[3].split(",")
    name = os.path.basename(program).rsplit("-", 1)[0]
    log = os.path.join(os.environ.get("TMPDIR", "/tmp"), "carbon-dialog-%d.out" % os.getpid())
    darling = os.environ.get("DARLING", "darling")
    if mode == "wayland":
        env = ["WAYLAND_DISPLAY=" + os.environ["WAYLAND_DISPLAY"], "DARLING_APPKIT_BACKEND=wayland"]
        if os.environ.get("XDG_RUNTIME_DIR"):
            env.append("XDG_RUNTIME_DIR=" + os.environ["XDG_RUNTIME_DIR"])
        injector = os.environ["KEYS_INJECTOR"]

        def key(x11_name):
            code = EVDEV[x11_name]
            subprocess.run([injector, "100", "%d:1" % code, "%d:0" % code], timeout=30)
    elif mode == "x11":
        env = ["DISPLAY=" + os.environ["DISPLAY"]]

        def key(x11_name):
            subprocess.run(["xdotool", "key", x11_name], timeout=30)
    else:
        sys.exit("unknown mode " + mode)
    failures = 0
    guest = subprocess.Popen(["timeout", "-s", "KILL", "150", darling, "shell", "env"] + env +
                             ["/Volumes/SystemRoot" + os.path.abspath(program)], stdin=subprocess.DEVNULL,
                             stdout=open(log, "w"), stderr=subprocess.STDOUT)
    try:
        for number, name_of_key in enumerate(keys, 1):
            ok = wait_for(log, "DIALOG_READY %d" % number, 120)
            print(("PASS: " if ok else "FAIL: ") + "dialog %d is shown" % number)
            if not ok:
                failures += 1
                break
            time.sleep(3)  # the window is up and has the keyboard focus
            key(name_of_key)
        ok = wait_for(log, "PASS " + name, 60)
        print(("PASS: " if ok else "FAIL: ") + "the program reports that all of its checks passed")
        if not ok:
            failures += 1
            print(read(log))
    finally:
        try:
            guest.wait(timeout=30)
        except subprocess.TimeoutExpired:
            guest.kill()
            guest.wait()
        subprocess.run([darling, "shutdown"], stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL, timeout=60)
        try:
            os.unlink(log)
        except FileNotFoundError:
            pass
    print("%d failure(s)" % failures)
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
