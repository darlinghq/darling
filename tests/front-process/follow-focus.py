#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
#
# Licensed under the MIT license, the same as the rest of this project.
#
# The front process follows the window system's input focus, whichever client takes it. Two guest
# processes (probe.m): A is an AppKit application with a window, K is a Carbon application with a Carbon
# window (a window of CoreGraphics' window server, no AppKit in the process). The test moves the focus with
# the window manager (xdotool on X11, swaymsg on sway) between the two windows, and to a client that is
# not a Darling process (xmessage on X11, zenity on Wayland), and after each move asks both processes for
# GetFrontProcess(), NSWorkspace's frontmost application and CoreGraphics' own front process
# (_CGSGetFrontProcess, which fails while the focus is in a client that is not a Darling process, where
# GetFrontProcess() has no process serial number to give and answers with the calling process), and reads the
# front process changes their observers (_CGSAddFrontProcessObserver) and Carbon application events saw.
# The processes report their own focus (CoreGraphics does it), so K, which has no AppKit to do it, is
# the front process while its window has the focus, and a move of the focus to a client that is not a Darling
# process is seen by every process.
#
#   follow-focus.py x11|sway|kwin PROBE [DISPLAY]
#
# PROBE is probe.m built for the guest. "x11" starts its own Xvfb DISPLAY (default :91) with xfwm4, "sway" a
# headless sway and "kwin" a private virtual KWin started with --no-lockscreen (the harness of
# src/external/cocotron/tests/appkit-global-menu/run-bar-tracking.py). A private KWin has no command to move
# the focus between windows here, so on kwin only the move to a client that is not a Darling process is tested.
import importlib.util
import os
import re
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
TESTS = os.path.join(HERE, "..", "..", "src", "external", "cocotron", "tests", "appkit-global-menu")
sys.path.insert(0, TESTS)
from common import check, finish  # noqa: E402

spec = importlib.util.spec_from_file_location("front_process_run", os.path.join(HERE, "run.py"))
fp = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fp)

DARLING = os.environ.get("DARLING", "darling")


def main():
    argv = sys.argv[1:]
    if len(argv) < 2:
        print(__doc__)
        return 2
    kind, probe = argv[0], os.path.abspath(argv[1])
    work = tempfile.mkdtemp(prefix="front-follows-focus-", dir=os.environ.get("TMPDIR"))
    rbt = fp.load_backends()
    xvfb = wm = None
    if kind == "kwin":
        be = rbt.KWinBackend(work)
        wayland_env = dict(os.environ, WAYLAND_DISPLAY=be.guest_env()[1].split("=", 1)[1],
                           XDG_RUNTIME_DIR=be.session.runtime, DBUS_SESSION_BUS_ADDRESS=be.bus.address, GDK_BACKEND="wayland")
        wayland_env.pop("DISPLAY", None)
    elif kind == "sway":
        be = rbt.SwayBackend(work)
        wayland_env = dict(be.env, GDK_BACKEND="wayland")
    else:
        display = argv[2] if len(argv) > 2 else ":91"
        if os.path.exists("/tmp/.X11-unix/X" + display[1:]):
            print("display %s is in use" % display)
            return 2
        xvfb = subprocess.Popen(["Xvfb", display, "-screen", "0", "1280x800x24", "-nolisten", "tcp"],
                                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        fp.wait_until(lambda: os.path.exists("/tmp/.X11-unix/X" + display[1:]), 20)
        wm = subprocess.Popen(["xfwm4", "--compositor=off", "--sm-client-disable"],
                              env=dict(os.environ, DISPLAY=display), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        time.sleep(2)
        be = rbt.X11Backend(work, display)

    env = be.guest_env()
    events = os.path.join(work, "events.log")
    open(events, "w").close()
    log = fp.Log(events)
    procs = []
    foreign = []

    def start(name, mode):
        exe = fp.bundle(work, probe, name)
        out = open(os.path.join(work, name + ".out"), "w")
        procs.append(subprocess.Popen(
            ["timeout", "-s", "KILL", "300", DARLING, "shell", "env", "FRONT_PROBE_OBSERVE=1"] + env +
            ["/Volumes/SystemRoot" + exe, name, "/Volumes/SystemRoot" + events, mode, "@/Volumes/SystemRoot" + work + "/" + name + ".cmd"],
            stdin=subprocess.DEVNULL, stdout=out, stderr=subprocess.STDOUT))
        open(os.path.join(work, name + ".cmd"), "w").close()

    def send(name, command):
        with open(os.path.join(work, name + ".cmd"), "a") as f:
            f.write(command + "\n")

    def pid_of(name):
        for l in fp.who(log.lines(), name):
            m = re.search(r"self=(\d+)", l)
            if m:
                return int(m.group(1))
        return None

    def focus(title):
        """Moves the input focus to the window with this title; False where the backend cannot."""
        if kind == "x11":
            subprocess.run(["xdotool", "search", "--name", "^%s$" % title, "windowactivate", "--sync"], env=be.xenv,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=30)
            return True
        if kind == "sway":
            be.msg('[title="%s"] focus' % title)
            return True
        return False

    def start_foreign():
        """A window of a client that is not a Darling process, which takes the focus when it appears."""
        if kind == "x11":
            p = subprocess.Popen(["xmessage", "-name", "foreign", "-title", "foreign", "front process test"], env=be.xenv,
                                 stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        else:
            p = subprocess.Popen(["zenity", "--info", "--title=foreign", "--text=front process test"], env=wayland_env,
                                 stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        foreign.append(p)
        # Its window takes a moment to appear (and a Wayland client has no name to look for).
        if kind == "x11":
            fp.wait_until(lambda: subprocess.run(["xdotool", "search", "--onlyvisible", "--name", "^foreign$"], env=be.xenv,
                                                 stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True).stdout.strip(), 20)
        else:
            time.sleep(4)
        return p

    def state(name):
        """What process NAME answers: (GetFrontProcess, NSWorkspace's front, CoreGraphics' front); the last is None
        while no Darling process has the focus."""
        before = len(log.lines())
        send(name, "get")
        line = fp.wait_until(lambda: next((l for l in fp.who(log.lines()[before:], name) if " STATE get " in l), None), 20)
        if line is None:
            return None, None, None
        m = re.search(r"getfront=(-?\d+)\(err (-?\d+)\) ws.front=(-?\d+).* cgsfront=(-?\d+)\(err (-?\d+)\)", line)
        return (int(m.group(1)) if m.group(2) == "0" else None), int(m.group(3)), (int(m.group(4)) if m.group(5) == "0" else None)

    try:
        subprocess.run([DARLING, "shutdown"], stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=60)
        time.sleep(3)
        for n, mode in (("A", "cocoa"), ("K", "carbonwin")):
            start(n, mode)
            fp.wait_until(lambda: pid_of(n) is not None, 90)
            time.sleep(2)
        pid = {n: pid_of(n) for n in "AK"}
        check(all(pid.values()), "both guest processes started (%s)" % pid)
        time.sleep(3)
        check(fp.first(fp.who(log.lines(), "K"), "WINDOW carbon window err=0") is not None, "K has its Carbon window")

        def expect(label, front):
            """Every process answers with the same front process (None: no Darling process is the front one)."""
            time.sleep(1.5)
            for n in "AK":
                got = state(n)
                if front is None:
                    check(got[2] is None, "%s: %s sees no front process (CoreGraphics %s)" % (label, n, got[2]))
                    continue
                # K has no AppKit, so only A can be asked for NSWorkspace's answer.
                check(got[0] == front and got[2] == front and (n == "K" or got[1] == front),
                      "%s: %s sees front process %s (GetFrontProcess %s, NSWorkspace %s, CoreGraphics %s)" %
                      (label, n, front, got[0], got[1], got[2]))

        def moved(change):
            before = len(log.lines())
            change()
            time.sleep(2)
            return log.lines()[before:]

        def observed(lines, name, current):
            return fp.first(fp.who(lines, name), "FRONTOBS previous=") is not None and \
                any(" FRONTOBS " in l and "current=%d" % current in l for l in fp.who(lines, name))

        if kind != "kwin":
            # The starting point: the Carbon window has the focus, whatever the window manager did with the new windows.
            moved(lambda: focus("K"))
            lines = moved(lambda: focus("A"))
            expect("focus to the AppKit window", pid["A"])
            check(observed(lines, "K", pid["A"]), "focus to the AppKit window: K's observer sees the new front process")

            lines = moved(lambda: focus("K"))
            expect("focus to the Carbon window", pid["K"])
            check(observed(lines, "A", pid["K"]) and observed(lines, "K", pid["K"]),
                  "focus to the Carbon window: both observers see the new front process")
            check(fp.first(fp.who(lines, "K"), "CARBON AppActivated") is not None, "focus to the Carbon window: K gets kEventAppActivated")
            check(fp.first(fp.who(lines, "A"), "CARBON AppDeactivated") is not None, "focus to the Carbon window: A gets kEventAppDeactivated")

            lines = moved(lambda: focus("A"))
            expect("focus back to the AppKit window", pid["A"])
            check(fp.first(fp.who(lines, "K"), "CARBON AppDeactivated") is not None, "focus back to the AppKit window: K gets kEventAppDeactivated")
            lines = moved(lambda: focus("K"))
            expect("focus back to the Carbon window", pid["K"])

        lines = moved(start_foreign)
        expect("focus to a client that is not a Darling process", None)
        check(observed(lines, "A", 0) and observed(lines, "K", 0),
              "focus to a client that is not a Darling process: both observers see that no Darling process is the front one")
        check(any(fp.first(fp.who(lines, n), "CARBON AppDeactivated") is not None for n in "AK"),
              "focus to a client that is not a Darling process: the process that had the focus gets kEventAppDeactivated")

        if kind != "kwin":
            lines = moved(lambda: focus("K"))
            expect("focus from the foreign client back to the Carbon window", pid["K"])
            check(observed(lines, "A", pid["K"]) and observed(lines, "K", pid["K"]),
                  "focus from the foreign client back to the Carbon window: both observers see the new front process")
    finally:
        for n in "AK":
            try:
                send(n, "quit")
            except OSError:
                pass
        time.sleep(3)
        for p in foreign + procs:
            p.kill()
        be.close()
        for p in (wm, xvfb):
            if p:
                p.terminate()
                try:
                    p.wait(10)
                except subprocess.TimeoutExpired:
                    p.kill()
        subprocess.run([DARLING, "shutdown"], stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=60)
        print("work directory: " + work)
    return finish()


if __name__ == "__main__":
    sys.exit(main())
