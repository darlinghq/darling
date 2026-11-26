#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
#
# Runs the guest program carbon_navigation_put_file and acts as the user of its dialogs: the first dialog is
# accepted (Return), the second is cancelled (Escape).
#
#   carbon_navigation_put_file.py MODE PROGRAM
#
# MODE
#   x11       the X server in $DISPLAY (it needs a window manager and xdotool)
#   wayland   the compositor that $WAYLAND_DISPLAY and $XDG_RUNTIME_DIR name (sway on a headless output or a
#             private kwin_wayland --virtual --no-lockscreen); $KEYS_INJECTOR is the program that types for the
#             user, as for carbon_key_events.sh: it takes a pause in milliseconds and evdev key codes with 1 or 0
#             for press and release (virtual-keyboard-keys on sway, kwin-keys on KWin)
# PROGRAM is carbon_navigation_put_file built for the guest (the host path). DARLING names the launcher
# (default: darling).
import os
import subprocess
import sys
import time


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


EVDEV_RETURN, EVDEV_ESCAPE = 28, 1


def main():
    mode, program = sys.argv[1], sys.argv[2]
    log = os.path.join(os.environ.get("TMPDIR", "/tmp"), "carbon-nav-%d.out" % os.getpid())
    darling = os.environ.get("DARLING", "darling")
    if mode == "wayland":
        env = ["WAYLAND_DISPLAY=" + os.environ["WAYLAND_DISPLAY"], "DARLING_APPKIT_BACKEND=wayland"]
        if os.environ.get("XDG_RUNTIME_DIR"):
            env.append("XDG_RUNTIME_DIR=" + os.environ["XDG_RUNTIME_DIR"])
        injector = os.environ["KEYS_INJECTOR"]

        def key(x11_name, code):
            subprocess.run([injector, "100", "%d:1" % code, "%d:0" % code], timeout=30)
    elif mode == "x11":
        env = ["DISPLAY=" + os.environ["DISPLAY"]]

        def key(x11_name, code):
            subprocess.run(["xdotool", "key", x11_name], timeout=30)
    else:
        sys.exit("unknown mode " + mode)
    failures = 0
    guest = subprocess.Popen(["timeout", "-s", "KILL", "150", darling, "shell", "env"] + env +
                             ["/Volumes/SystemRoot" + os.path.abspath(program)], stdin=subprocess.DEVNULL,
                             stdout=open(log, "w"), stderr=subprocess.STDOUT)
    try:
        for number, name, code in ((1, "Return", EVDEV_RETURN), (2, "Escape", EVDEV_ESCAPE)):
            ok = wait_for(log, "NAV_READY %d" % number, 120)
            print(("PASS: " if ok else "FAIL: ") + "the dialog %d is shown" % number)
            if not ok:
                failures += 1
                break
            time.sleep(3)  # the window is up and has the keyboard focus
            key(name, code)
        ok = wait_for(log, "PASS carbon_navigation_put_file", 60)
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
