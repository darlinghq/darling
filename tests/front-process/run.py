#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
#
# Licensed under the MIT license, the same as the rest of this project.
#
# The front process through Carbon, AppKit and NSWorkspace: five guest processes (probe.m, built for the
# guest) in one prefix, four of them observers with a window and one that switches the front process with
# SetFrontProcess() / SetFrontProcessWithOptions() and logs what GetFrontProcess() and NSWorkspace say
# right after. Every process logs the Carbon application events (kEventAppFrontSwitched, kEventAppActivated,
# kEventAppDeactivated), NSApplication's and NSWorkspace's activation notifications, and on each of them
# GetFrontProcess(), NSWorkspace's frontmostApplication and menuBarOwningApplication.
#
#   run.py [--cases basic,errors,repeat] x11|sway|kwin PROBE [DISPLAY]
#
# PROBE is probe.m built for the guest (x86_64 or i386). "x11" starts its own Xvfb DISPLAY (default :91)
# with xfwm4, "sway" a headless sway and "kwin" a private virtual KWin (see the backends of
# src/external/cocotron/tests/appkit-global-menu/run-bar-tracking.py, which are used here).
#
# Expected (the order on macOS 10.14.4 and 26.7.3, mac-probes/front-process): the call returns with
# GetFrontProcess() and NSWorkspace already answering with the new front process; in every other process
# kEventAppFrontSwitched (the new front process in kEventParamProcessID) comes before kEventAppActivated
# (new front process) and kEventAppDeactivated (the process that was front); in a process whose
# NSApplication follows, the Carbon event comes before NSApplicationWillBecomeActiveNotification, which comes
# before DidBecomeActive (likewise for Resign); NSWorkspaceDidActivateApplicationNotification comes before
# DidDeactivate; no event for a request that changes nothing; an application with LSBackgroundOnly cannot be
# made the front process (-606), nor can an unknown process (-600).
import importlib.util
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
TESTS = os.path.join(HERE, "..", "..", "src", "external", "cocotron", "tests", "appkit-global-menu")
sys.path.insert(0, TESTS)
from common import check, finish  # noqa: E402

DARLING = os.environ.get("DARLING", "darling")


def load_backends():
    spec = importlib.util.spec_from_file_location("run_bar_tracking", os.path.join(TESTS, "run-bar-tracking.py"))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def bundle(work, probe, name, ui_element=False, background_only=False):
    """An application bundle called NAME around the probe, with the bundle identifier test.vdfp.NAME."""
    d = os.path.join(work, name + ".app", "Contents", "MacOS")
    os.makedirs(d)
    exe = os.path.join(d, name)
    shutil.copy(probe, exe)
    extra = ""
    if ui_element:
        extra += "<key>LSUIElement</key><true/>"
    if background_only:
        extra += "<key>LSBackgroundOnly</key><true/>"
    with open(os.path.join(work, name + ".app", "Contents", "Info.plist"), "w") as f:
        f.write('<?xml version="1.0" encoding="UTF-8"?><plist version="1.0"><dict>'
                "<key>CFBundleExecutable</key><string>%s</string><key>CFBundleIdentifier</key><string>test.vdfp.%s</string>"
                "<key>CFBundleName</key><string>%s</string><key>CFBundlePackageType</key><string>APPL</string>"
                "<key>NSPrincipalClass</key><string>NSApplication</string>%s</dict></plist>\n" % (name, name, name, extra))
    return exe


class Log:
    def __init__(self, path):
        self.path = path

    def lines(self):
        try:
            return [l.rstrip("\n") for l in open(self.path) if l.strip()]
        except FileNotFoundError:
            return []


def wait_until(condition, seconds=40):
    end = time.time() + seconds
    while time.time() < end:
        value = condition()
        if value:
            return value
        time.sleep(0.2)
    return None


def who(lines, name):
    return [l for l in lines if l.split(" ", 2)[1] == name]


def first(lines, text):
    for i, l in enumerate(lines):
        if text in l:
            return i
    return None


def main():
    argv = sys.argv[1:]
    cases = ["basic", "errors", "repeat"]
    if argv[:1] == ["--cases"] and len(argv) > 1:
        cases = argv[1].split(",")
        argv = argv[2:]
    if len(argv) < 2:
        print(__doc__)
        return 2
    kind, probe = argv[0], os.path.abspath(argv[1])
    work = tempfile.mkdtemp(prefix="front-process-", dir=os.environ.get("TMPDIR"))
    rbt = load_backends()
    xvfb = wm = None
    if kind == "kwin":
        be = rbt.KWinBackend(work)
    elif kind == "sway":
        be = rbt.SwayBackend(work)
    else:
        display = argv[2] if len(argv) > 2 else ":91"
        if os.path.exists("/tmp/.X11-unix/X" + display[1:]):
            print("display %s is in use" % display)
            return 2
        xvfb = subprocess.Popen(["Xvfb", display, "-screen", "0", "1280x800x24", "-nolisten", "tcp"],
                                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        wait_until(lambda: os.path.exists("/tmp/.X11-unix/X" + display[1:]), 20)
        wm = subprocess.Popen(["xfwm4", "--compositor=off", "--sm-client-disable"],
                              env=dict(os.environ, DISPLAY=display), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        time.sleep(2)
        be = rbt.X11Backend(work, display)

    env = be.guest_env()
    events = os.path.join(work, "events.log")
    open(events, "w").close()
    log = Log(events)
    procs = []

    def start(name, ui=False, bg=False, script=None):
        exe = bundle(work, probe, name, ui, bg)
        out = open(os.path.join(work, name + ".out"), "w")
        procs.append(subprocess.Popen(
            ["timeout", "-s", "KILL", "240", DARLING, "shell", "env"] + env +
            ["/Volumes/SystemRoot" + exe, name, "/Volumes/SystemRoot" + events, "cocoa", script or ("@/Volumes/SystemRoot" + work + "/" + name + ".cmd")],
            stdin=subprocess.DEVNULL, stdout=out, stderr=subprocess.STDOUT))
        open(os.path.join(work, name + ".cmd"), "w").close()
        return exe

    def send(name, command):
        with open(os.path.join(work, name + ".cmd"), "a") as f:
            f.write(command + "\n")

    def pid_of(name):
        for l in who(log.lines(), name):
            m = re.search(r"self=(\d+)", l)
            if m:
                return int(m.group(1))
        return None

    try:
        subprocess.run([DARLING, "shutdown"], stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=60)
        time.sleep(3)
        for n, ui, bg in (("A", 0, 0), ("B", 0, 0), ("U", 1, 0), ("P", 0, 1), ("C", 0, 0)):
            start(n, ui, bg)
            wait_until(lambda: pid_of(n) is not None, 90)
            time.sleep(2)
        check(all(pid_of(n) for n in "ABUPC"), "all five guest processes started (%s)" % {n: pid_of(n) for n in "ABUPC"})
        time.sleep(3)
        pid = {n: pid_of(n) for n in "ABUPC"}
        observers = "ABUPC"

        def step(command, settle=3.0, who_sends="C"):
            before = len(log.lines())
            send(who_sends, command)
            wait_until(lambda: any("RESULT" in l for l in log.lines()[before:] if " %s " % who_sends in l), 20)
            time.sleep(settle)
            return log.lines()[before:]

        def result_err(lines, sender="C"):
            for l in lines:
                if " %s RESULT " % sender in l and "err=" in l:
                    return int(re.search(r"err=(-?\d+)", l).group(1))
            return None

        def carbon(lines, name, kind_):
            return [i for i, l in enumerate(lines) if " %s CARBON %s " % (name, kind_) in l]

        def consistent(lines):
            """GetFrontProcess and NSWorkspace agree in every STATE line of the processes."""
            bad = []
            for l in lines:
                m = re.search(r" STATE \S+ getfront=(-?\d+)\(err 0\) ws.front=(-?\d+)", l)
                if m and m.group(1) != m.group(2):
                    bad.append(l)
            return bad

        def switch(command, target, label, previous):
            lines = step(command)
            check(result_err(lines) == 0, "%s: the call succeeds (%s)" % (label, result_err(lines)))
            after = [l for l in lines if " C STATE right-after-set " in l]
            check(bool(after) and "getfront=%d(" % pid[target] in after[0] and "ws.front=%d " % pid[target] in after[0],
                  "%s: when the call returns GetFrontProcess and NSWorkspace answer with the new front process (%s)" % (label, after[:1]))
            for o in observers:
                if o == "C":
                    continue
                fs = [l for l in lines if " %s CARBON AppFrontSwitched " % o in l]
                check(bool(fs) and "psn-pid=%d" % pid[target] in fs[0],
                      "%s: %s gets kEventAppFrontSwitched with the new front process" % (label, o))
            t_lines = who(lines, target)
            i_sw, i_act = first(t_lines, "CARBON AppFrontSwitched"), first(t_lines, "CARBON AppActivated")
            check(i_act is not None and i_sw is not None and i_sw < i_act,
                  "%s: %s gets kEventAppFrontSwitched before kEventAppActivated" % (label, target))
            if previous:
                p_lines = who(lines, previous)
                check(first(p_lines, "CARBON AppDeactivated") is not None, "%s: %s gets kEventAppDeactivated" % (label, previous))
                i_sw, i_de = first(p_lines, "CARBON AppFrontSwitched"), first(p_lines, "CARBON AppDeactivated")
                if previous != "C":
                    check(i_sw is not None and i_de is not None and i_sw < i_de,
                          "%s: %s gets kEventAppFrontSwitched before kEventAppDeactivated" % (label, previous))
                i_de, i_wr = first(p_lines, "CARBON AppDeactivated"), first(p_lines, "NSApplicationWillResignActiveNotification")
                i_dr = first(p_lines, "NSApplicationDidResignActiveNotification")
                if i_wr is not None:
                    check(i_de < i_wr and (i_dr is None or i_wr < i_dr),
                          "%s: %s gets kEventAppDeactivated, then NSApplicationWillResignActive, then DidResignActive" % (label, previous))
            i_wb = first(t_lines, "NSApplicationWillBecomeActiveNotification")
            i_db = first(t_lines, "NSApplicationDidBecomeActiveNotification")
            if i_wb is not None:
                check(i_act < i_wb and (i_db is None or i_wb < i_db),
                      "%s: %s gets kEventAppActivated, then NSApplicationWillBecomeActive, then DidBecomeActive" % (label, target))
            for o in observers:
                o_lines = who(lines, o)
                a = first(o_lines, "NSWorkspaceDidActivateApplicationNotification app-pid=%d" % pid[target])
                check(a is not None, "%s: %s gets NSWorkspaceDidActivateApplicationNotification for %s" % (label, o, target))
                if previous and a is not None:
                    d = first(o_lines, "NSWorkspaceDidDeactivateApplicationNotification app-pid=%d" % pid[previous])
                    check(d is not None and a < d, "%s: %s gets DidActivate before DidDeactivate" % (label, o))
            bad = consistent(lines)
            check(not bad, "%s: GetFrontProcess and NSWorkspace agree in every report (%s)" % (label, bad[:1]))
            return lines

        if "basic" in cases:
            send("C", "front=A")  # starting point, not checked
            time.sleep(4)
            lines = switch("front=B", "B", "A to B", "A")
            lines = switch("front=A,1", "A", "B to A (front window only)", "B")
            # An agent becomes the front process but not the menu bar owner.
            lines = step("front=U")
            check(result_err(lines) == 0, "front=U: the call succeeds for an LSUIElement application")
            l = [x for x in lines if " C STATE right-after-set " in x]
            check(bool(l) and "getfront=%d(" % pid["U"] in l[0] and "ws.front=%d " % pid["U"] in l[0] and "ws.owner=%d " % pid["A"] in l[0],
                  "front=U: U is the front process, A stays the menu bar owner (%s)" % l[:1])
            check(bool(carbon(who(lines, "U"), "U", "AppActivated")) or first(who(lines, "U"), "CARBON AppActivated") is not None,
                  "front=U: U gets kEventAppActivated")
            check(first(who(lines, "A"), "CARBON AppDeactivated") is not None, "front=U: A gets kEventAppDeactivated")
            lines = step("frontpsn=0:2")
            check(result_err(lines) == 0, "kCurrentProcess is the calling process")
            check(first(who(lines, "C"), "CARBON AppActivated") is not None, "kCurrentProcess: C gets kEventAppActivated")
            lines = step("get")
            st = [x for x in lines if " C STATE get " in x]
            check(bool(st) and "getfront=%d(" % pid["C"] in st[0], "C is the front process (%s)" % st[:1])

        if "errors" in cases:
            lines = step("front=U")
            before = [x for x in lines if " C STATE right-after-set " in x]
            lines = step("front=P")
            # QUIRK test: an application with LSBackgroundOnly cannot be made the front process
            check(result_err(lines) == -606, "quirk: SetFrontProcess to a background-only application fails with -606 (%s)" % result_err(lines))
            check(not [x for x in lines if "CARBON" in x], "quirk: and nothing changes (no Carbon events)")
            st = [x for x in lines if " C STATE right-after-set " in x]
            check(bool(st) and "getfront=%d(" % pid["U"] in st[0], "quirk: the front process is still U (%s)" % st[:1])
            lines = step("frontpsn=0:0")
            check(result_err(lines) == -600, "kNoProcess is procNotFound (%s)" % result_err(lines))
            lines = step("frontpsn=99999:99999")
            check(result_err(lines) == -600, "an unknown process is procNotFound (%s)" % result_err(lines))
            check(not [x for x in lines if "CARBON" in x], "no events for the failed requests")

        if "repeat" in cases:
            step("frontself")
            lines = step("frontself")
            check(result_err(lines) == 0, "a request for the front process succeeds")
            check(not [x for x in lines if "CARBON" in x], "and changes nothing: no Carbon events (%s)" % [x for x in lines if "CARBON" in x][:1])
            lines = step("front=B")
            lines = step("front=B")
            check(result_err(lines) == 0 and not [x for x in lines if "CARBON" in x], "the same for another process")
    finally:
        for n in "ABUPC":
            try:
                send(n, "quit")
            except OSError:
                pass
        time.sleep(3)
        for p in procs:
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
