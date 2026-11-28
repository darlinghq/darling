#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
#
# Runs a guest program that shows a file panel and prints "SIDEBAR x y w h" (the sidebar in screen pixels) and
# "PANEL_READY", takes a screenshot of the window system and checks that the sidebar is not black: at most 20 % of
# its pixels may be dark (luminance below 48).
#
#   open_panel_sidebar.py MODE PROGRAM SHOT.ppm [ARGS...]
#
# MODE  x11      the X server in $DISPLAY (screenshot with ImageMagick's import)
#       kwin     a private kwin_wayland --virtual --no-lockscreen (no screenshot protocol: the program's own check of the
#                sidebar drawn into a bitmap is the result)
#       wayland  the compositor of $WAYLAND_DISPLAY and $XDG_RUNTIME_DIR (screenshot with grim)
# DARLING names the launcher (default: darling).
import os
import subprocess
import sys
import time


def read_ppm(path):
    data = open(path, "rb").read()
    fields = []
    pos = 0
    while len(fields) < 4:
        while data[pos:pos + 1].isspace():
            pos += 1
        if data[pos:pos + 1] == b"#":
            while data[pos:pos + 1] != b"\n":
                pos += 1
            continue
        start = pos
        while not data[pos:pos + 1].isspace():
            pos += 1
        fields.append(data[start:pos])
    pos += 1
    assert fields[0] == b"P6" and fields[3] == b"255", "the screenshot is not an 8-bit PPM"
    return int(fields[1]), int(fields[2]), data[pos:]


def main():
    mode, program, shot = sys.argv[1], sys.argv[2], sys.argv[3]
    args = sys.argv[4:]
    darling = os.environ.get("DARLING", "darling")
    log = os.path.join(os.environ.get("TMPDIR", "/tmp"), "panel-sidebar-%d.out" % os.getpid())
    if mode in ("wayland", "kwin"):
        env = ["WAYLAND_DISPLAY=" + os.environ["WAYLAND_DISPLAY"], "DARLING_APPKIT_BACKEND=wayland"]
        if os.environ.get("XDG_RUNTIME_DIR"):
            env.append("XDG_RUNTIME_DIR=" + os.environ["XDG_RUNTIME_DIR"])
    else:
        env = ["DISPLAY=" + os.environ["DISPLAY"], "DARLING_APPKIT_BACKEND=x11"]
    if not program.startswith("/Volumes/"):
        program = "/Volumes/SystemRoot" + os.path.abspath(program)
    out = open(log, "w")
    proc = subprocess.Popen(["timeout", "-s", "KILL", "90", darling, "shell", "env"] + env + [program] + args,
                            stdin=subprocess.DEVNULL, stdout=out, stderr=subprocess.STDOUT)
    end = time.time() + 60
    while time.time() < end and "PANEL_READY" not in open(log).read():
        time.sleep(0.3)
    text = open(log).read()
    if "PANEL_READY" not in text:
        print("FAIL: the panel did not appear\n" + text)
        proc.kill()
        return 1
    time.sleep(1.0)
    if mode == "kwin":
        pass
    elif mode == "wayland":
        subprocess.run(["grim", "-t", "ppm", shot], check=True)
    else:
        subprocess.run(["import", "-depth", "8", "-window", "root", "ppm:" + shot], check=True)
    proc.wait(timeout=60)
    text = open(log).read()
    sidebar = [l for l in text.splitlines() if l.startswith("SIDEBAR ")]
    if not sidebar:
        print("FAIL: no SIDEBAR line\n" + text)
        return 1
    if mode == "kwin":
        ok = "PASS" in text
        print(("PASS" if ok else "FAIL") + " open_panel_sidebar kwin (drawn into a bitmap; the compositor has no screenshot protocol)")
        return 0 if ok else 1
    x, y, w, h = [int(v) for v in sidebar[0].split()[1:]]
    width, height, pix = read_ppm(shot)
    dark = total = 0
    for row in range(max(y, 0), min(y + h, height)):
        for col in range(max(x, 0), min(x + w, width)):
            r, g, b = pix[(row * width + col) * 3:(row * width + col) * 3 + 3]
            total += 1
            if (299 * r + 587 * g + 114 * b) / 1000 < 48:
                dark += 1
    frac = dark / total if total else 1.0
    print("sidebar %dx%d at %d,%d: %.1f %% dark pixels" % (w, h, x, y, 100 * frac))
    ok = total > 0 and frac <= 0.2 and "PASS" in text
    print(("PASS" if ok else "FAIL") + " open_panel_sidebar " + mode)
    return 0 if ok else 1


sys.exit(main())
