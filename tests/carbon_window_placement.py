#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
#
# Runs the guest program carbon_window_placement and asks the window system where the windows are.
#
#   carbon_window_placement.py MODE PROGRAM
#
# MODE
#   kwin        a private KWin (kwin_wayland --virtual --no-lockscreen, its own session bus) with the plasma
#               shell offered to clients (KWIN_WAYLAND_NO_PERMISSION_CHECKS)
#   kwin-plain  the same without the plasma shell, as a desktop session offers it to an ordinary program
#   sway        a headless sway of its own
#   x11         the X server in $DISPLAY (it needs a window manager, xfwm4, and xdotool and xprop)
# PROGRAM is carbon_window_placement built for the guest (the host path). DARLING names the launcher
# (default: darling).
#
# What is checked, in terms of the window classes of Carbon and not of any application:
#   - the menu bar is a surface along the top of the screen the whole width wide (on Wayland a layer-shell
#     surface that other windows keep clear of), not a window in the middle;
#   - a menu opens where the program asked for it, below the menu bar, as a pop-up of the menu bar on Wayland;
#   - the floating window is in front of the document window although it was shown before it, and is its
#     child (Wayland) or has the above state (X11);
#   - the document window has the frame of the window system;
#   - each window is where it was asked to be, where the window system lets a client say so.
import json
import os
import re
import subprocess
import sys
import tempfile
import time

failures = 0


def check(ok, what, detail=""):
    global failures
    print(("PASS: " if ok else "FAIL: ") + what + ((" (" + detail + ")") if detail else ""))
    if not ok:
        failures += 1


def require_no_lockscreen(kwin):
    """Aborts unless this private KWin has no lock screen: input sent to a locked session would be taken as
    password attempts against the host's PAM."""
    def ps(*args):
        return subprocess.run(["ps", "-o", "args="] + list(args), stdout=subprocess.PIPE, text=True).stdout
    problem = None
    if "--no-lockscreen" not in ps("-p", str(kwin.pid)).split():
        problem = "was not started with --no-lockscreen"
    elif "kscreenlocker_greet" in ps("--ppid", str(kwin.pid)):
        problem = "has a kscreenlocker_greet child"
    if problem:
        kwin.kill()
        kwin.wait()
        sys.exit("ABORT: the private KWin " + problem)


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


KWIN_SCRIPT = """
function out(text) { callDBus("org.darlinghq.test", "/", "org.darlinghq.test", "line", String(text)); }
var all = workspace.stackingOrder;
for (var i = 0; i < all.length; i++) {
    var w = all[i], g = w.frameGeometry;
    out("WIN " + JSON.stringify({caption: String(w.caption), x: Math.round(g.x), y: Math.round(g.y),
        w: Math.round(g.width), h: Math.round(g.height), stack: i, popup: !!w.popupWindow, dock: !!w.dock,
        transient: !!w.transient, noBorder: !!w.noBorder, skipTaskbar: !!w.skipTaskbar,
        keepAbove: !!w.keepAbove, layer: Number(w.layer), type: Number(w.windowType),
        transientFor: w.transientFor ? String(w.transientFor.caption) : ""}));
}
out("END");
"""


class Kwin:
    def __init__(self, work, plasma):
        self.work = work
        self.runtime = os.path.join(work, "run")
        os.mkdir(self.runtime, 0o700)
        self.address = "unix:abstract=darling-carbon-placement-%d" % os.getpid()
        out = subprocess.run(["dbus-daemon", "--session", "--address=" + self.address, "--print-pid", "--fork"],
                             stdout=subprocess.PIPE, text=True, check=True)
        self.busPid = int(out.stdout.split()[0])
        self.socket = "darling-placement"
        env = dict(os.environ, DBUS_SESSION_BUS_ADDRESS=self.address, XDG_RUNTIME_DIR=self.runtime,
                   XDG_CONFIG_HOME=os.path.join(work, "config"), XDG_CACHE_HOME=os.path.join(work, "cache"),
                   XDG_DATA_HOME=os.path.join(work, "data"), QT_QPA_PLATFORM="offscreen")
        if plasma:
            env["KWIN_WAYLAND_NO_PERMISSION_CHECKS"] = "1"
        env.pop("DISPLAY", None)
        env.pop("WAYLAND_DISPLAY", None)
        self.env = env
        self.kwin = subprocess.Popen(["kwin_wayland", "--virtual", "--no-lockscreen", "--socket", self.socket,
                                      "--width", "1280", "--height", "1024", "sleep", "1000"], env=env,
                                     stdout=open(os.path.join(work, "kwin.out"), "w"), stderr=subprocess.STDOUT)
        end = time.time() + 30
        while time.time() < end and not os.path.exists(os.path.join(self.runtime, self.socket)):
            time.sleep(0.2)
        require_no_lockscreen(self.kwin)
        self.width = 1280
        self.script = os.path.join(work, "windows.js")
        open(self.script, "w").write(KWIN_SCRIPT)

    def guest_env(self):
        return ["WAYLAND_DISPLAY=" + os.path.join(self.runtime, self.socket), "DARLING_APPKIT_BACKEND=wayland",
                "DBUS_SESSION_BUS_ADDRESS=" + self.address]

    def close(self):
        self.kwin.kill()
        self.kwin.wait()
        try:
            os.kill(self.busPid, 15)
        except ProcessLookupError:
            pass

    def gdbus(self, dest, path, method, *args):
        r = subprocess.run(["gdbus", "call", "--session", "--dest", dest, "--object-path", path, "--method", method]
                           + list(args), env=self.env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
                           timeout=30)
        return r.stdout.strip()

    def windows(self):
        monitor = subprocess.Popen(["dbus-monitor", "--session", "interface=org.darlinghq.test"], env=self.env,
                                   stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
        time.sleep(0.5)
        reply = self.gdbus("org.kde.KWin", "/Scripting", "org.kde.kwin.Scripting.loadScript", self.script,
                           "darling-windows")
        number = re.search(r"\d+", reply).group(0)
        self.gdbus("org.kde.KWin", "/Scripting/Script" + number, "org.kde.kwin.Script.run")
        windows = []
        end = time.time() + 10
        while time.time() < end:
            line = monitor.stdout.readline()
            m = re.search(r'string "(WIN (\{.*\})|END)"\s*$', line)
            if m:
                if m.group(1) == "END":
                    break
                windows.append(json.loads(m.group(2)))
        monitor.kill()
        monitor.wait()
        self.gdbus("org.kde.KWin", "/Scripting", "org.kde.kwin.Scripting.unloadScript", "darling-windows")
        return windows


class Sway:
    def __init__(self, work):
        self.runtime = os.path.join(work, "sw")
        os.mkdir(self.runtime, 0o700)
        config = os.path.join(self.runtime, "config")
        open(config, "w").write("output * resolution 1280x1024\noutput * bg #FFFFFF solid_color\n"
                                "default_border none\ndefault_floating_border none\n"
                                "for_window [shell=\".*\"] floating enable\n")
        env = dict(os.environ, XDG_RUNTIME_DIR=self.runtime, WLR_BACKENDS="headless", WLR_RENDERER="pixman",
                   WLR_LIBINPUT_NO_DEVICES="1")
        env.pop("DISPLAY", None)
        env.pop("WAYLAND_DISPLAY", None)
        self.proc = subprocess.Popen(["sway", "-c", config], env=env, stdout=open(os.path.join(work, "sway.out"), "w"),
                                     stderr=subprocess.STDOUT)
        end = time.time() + 20
        while time.time() < end:
            socks = [f for f in os.listdir(self.runtime) if re.match(r"wayland-\d$", f)]
            ipc = [f for f in os.listdir(self.runtime) if f.startswith("sway-ipc")]
            if socks and ipc:
                break
            time.sleep(0.2)
        self.socket = os.path.join(self.runtime, socks[0])
        self.ipc = os.path.join(self.runtime, ipc[0])
        self.width = 1280

    def guest_env(self):
        return ["WAYLAND_DISPLAY=" + self.socket, "DARLING_APPKIT_BACKEND=wayland"]

    def close(self):
        self.proc.kill()
        self.proc.wait()

    def msg(self, kind):
        out = subprocess.run(["swaymsg", "-s", self.ipc, "-r", "-t", kind], stdout=subprocess.PIPE, text=True).stdout
        return json.loads(out)

    def windows(self):
        found = []

        def walk(node, floating):
            if node.get("type") in ("con", "floating_con") and node.get("name") and not node.get("nodes") \
                    and not node.get("floating_nodes"):
                r = node["rect"]
                found.append({"caption": node["name"], "x": r["x"], "y": r["y"], "w": r["width"], "h": r["height"]})
            for child in node.get("nodes", []):
                walk(child, floating)
            for child in node.get("floating_nodes", []):
                walk(child, True)

        walk(self.msg("get_tree"), False)
        return found

    def workspace_rect(self):
        return self.msg("get_workspaces")[0]["rect"]


def x11(*args):
    return subprocess.run(list(args), stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True).stdout


def x11_windows():
    """All viewable windows of the display: id, name, position and size, and the stacking order."""
    ids = x11("xdotool", "search", "--onlyvisible", "--name", "").split()
    order = re.findall(r"0x[0-9a-f]+", x11("xprop", "-root", "_NET_CLIENT_LIST_STACKING"))
    windows = []
    for i in ids:
        geometry = x11("xdotool", "getwindowgeometry", "--shell", i)
        values = dict(re.findall(r"(\w+)=(-?\d+)", geometry))
        if "WIDTH" not in values:
            continue
        name = x11("xdotool", "getwindowname", i).strip()
        state = x11("xprop", "-id", i, "_NET_WM_STATE", "_NET_WM_WINDOW_TYPE", "WM_TRANSIENT_FOR", "WM_NORMAL_HINTS")
        hexid = "0x%x" % int(i)
        windows.append({"id": i, "caption": name, "x": int(values["X"]), "y": int(values["Y"]), "w": int(values["WIDTH"]),
                        "h": int(values["HEIGHT"]), "state": state,
                        "stack": order.index(hexid) if hexid in order else -1})
    return windows


def screenshot(mode, session, name):
    """A screenshot of the screen into the file the environment variable CARBON_PLACEMENT_SHOTS names as a
    prefix (nothing when it is not set); for a person to look at, nothing is compared."""
    prefix = os.environ.get("CARBON_PLACEMENT_SHOTS")
    if not prefix:
        return
    path = prefix + "-" + name + ".png"
    if mode.startswith("kwin"):
        env = dict(session.env, WAYLAND_DISPLAY=os.path.join(session.runtime, session.socket))
        env.pop("QT_QPA_PLATFORM", None)
        subprocess.run(["spectacle", "-b", "-n", "-f", "-o", path], env=env, stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL, timeout=60)
    elif mode == "sway":
        subprocess.run(["grim", path], env=dict(os.environ, WAYLAND_DISPLAY=session.socket), timeout=60)
    else:
        subprocess.run(["import", "-window", "root", path], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=60)


def brief(windows):
    return [{k: v for k, v in w.items() if k not in ("state", "id")} for w in windows]


def by_caption(windows, caption):
    for w in windows:
        if w["caption"] == caption:
            return w
    return None


def run_guest(session_env, program, log):
    darling = os.environ.get("DARLING", "darling")
    out = open(log, "w")
    return subprocess.Popen(["timeout", "-s", "KILL", "120", darling, "shell", "env"] + session_env +
                            ["/Volumes/SystemRoot" + os.path.abspath(program)], stdin=subprocess.DEVNULL, stdout=out,
                            stderr=subprocess.STDOUT)


def parse_rect(text):
    left, top, right, bottom = [int(v) for v in text.split(",")]
    return left, top, right, bottom


def close_enough(a, b, tolerance):
    return abs(a - b) <= tolerance


def main():
    mode, program = sys.argv[1], sys.argv[2]
    work = tempfile.mkdtemp(prefix="carbon-placement-", dir=os.environ.get("TMPDIR"))
    log = os.path.join(work, "guest.out")
    wayland = mode != "x11"
    if mode in ("kwin", "kwin-plain"):
        session = Kwin(work, mode == "kwin")
        env = session.guest_env()
    elif mode == "sway":
        session = Sway(work)
        env = session.guest_env()
    elif mode == "x11":
        session = None
        env = ["DISPLAY=" + os.environ["DISPLAY"]]
    else:
        sys.exit("unknown mode " + mode)
    guest = None
    try:
        guest = run_guest(env, program, log)
        if not wait_for(log, "WINDOWS_READY", 120):
            print(read(log))
            check(False, "the program shows its windows")
            return finish(session, guest)
        ready = re.search(r"WINDOWS_READY doc=(\S+) palette=(\S+) bar=(\d+)", read(log))
        doc = parse_rect(ready.group(1))
        palette = parse_rect(ready.group(2))
        bar = int(ready.group(3))
        time.sleep(1)
        windows = session.windows() if mode.startswith(("kwin", "sway")) else x11_windows()
        print("windows:", json.dumps(brief(windows)))
        screenshot(mode, session, "windows")
        width = session.width if session else None
        d, p = by_caption(windows, "PlacementDoc"), by_caption(windows, "PlacementPalette")
        check(d is not None and p is not None, "the document window and the floating window are shown")

        if wayland:
            bars = [w for w in windows if w["x"] == 0 and w["y"] == 0 and w["w"] == width and w["h"] == bar]
            if mode == "sway":
                # sway does not list layer surfaces; its workspace area shows that the bar keeps a strip clear
                check(session.workspace_rect()["y"] == bar,
                      "the menu bar is a layer surface along the top: other windows keep clear of it",
                      str(session.workspace_rect()))
            else:
                check(len(bars) == 1, "the menu bar is a surface along the top of the screen, the whole width wide",
                      "%d found" % len(bars))
            if mode.startswith("kwin") and bars:
                # KWin::Layer: 3 is the dock layer
                check(bars[0]["layer"] == 3 and bars[0]["skipTaskbar"], "KWin puts the menu bar in the dock layer, out of the task list", str(bars[0]))
        else:
            bars = [w for w in windows if w["x"] == 0 and w["y"] == 0 and w["h"] == bar and w["w"] > 800]
            check(len(bars) == 1, "the menu bar is a window along the top of the screen, the whole width wide",
                  "%d found" % len(bars))

        if d and p:
            # The floating window is in front of the document window although it was shown first
            # (sway does not keep a child in front of its parent: the windows are only checked to be there)
            if mode != "sway":
                check(p["stack"] > d["stack"], "the floating window is in front of the document window",
                      "stack %s over %s" % (p["stack"], d["stack"]))
            if mode.startswith("kwin"):
                check(p["transientFor"] == "PlacementDoc", "the floating window is a child of the document window",
                      p["transientFor"])
                check(p["skipTaskbar"], "the floating window is left out of the task list")
                # The compositor's frame adds a title bar to the window's own area
                check(not d["noBorder"] and d["h"] > doc[3] - doc[1],
                      "the document window has the compositor's frame (a title bar above its content)", str(d))
            if mode == "x11":
                check("_NET_WM_STATE_ABOVE" in p["state"], "the floating window has the above state")
                check("_NET_WM_STATE_ABOVE" not in d["state"], "the document window does not")

            # Where the windows were asked to be: the window manager of X11 puts the content there; the
            # compositor with the plasma shell puts the window there, the frame the compositor draws
            # included, so a frame's worth of difference is allowed.
            frame = 2 if mode == "x11" else 56
            placeable = mode in ("kwin", "x11")
            if placeable:
                check(close_enough(d["x"], doc[0], frame) and close_enough(d["y"], doc[1], frame),
                      "the document window is where it was asked to be", "asked %s,%s got %s,%s" % (doc[0], doc[1], d["x"], d["y"]))
                check(close_enough(p["x"], palette[0], frame) and close_enough(p["y"], palette[1], frame),
                      "the floating window is where it was asked to be", "asked %s,%s got %s,%s" % (palette[0], palette[1], p["x"], p["y"]))

        if not wait_for(log, "MENU_READY", 60):
            check(False, "the menu is opened")
            return finish(session, guest)
        time.sleep(1)
        windows = session.windows() if mode.startswith(("kwin", "sway")) else x11_windows()
        print("windows with the menu:", json.dumps(brief(windows)))
        screenshot(mode, session, "menu")
        if wayland:
            if mode.startswith("kwin"):
                menus = [w for w in windows if w["popup"]]
                check(len(menus) == 1, "the menu is a pop-up of the menu bar", "%d found" % len(menus))
                if menus:
                    m = menus[0]
                    check(close_enough(m["y"], bar, 6) and close_enough(m["x"], 20, 6), "the menu is where it was asked to be, below the bar", "at %s,%s" % (m["x"], m["y"]))
        else:
            menus = [w for w in windows if w["caption"] == "" and w["w"] < 400 and w["h"] > bar and 0 <= w["x"] <= 80
                     and abs(w["y"] - bar) <= 6]
            check(len(menus) >= 1, "the menu is a window below the menu bar", "%d found" % len(menus))
            if menus:
                m = menus[0]
                check(close_enough(m["y"], bar, 6) and close_enough(m["x"], 20, 6), "the menu is where it was asked to be, below the bar", "at %s,%s" % (m["x"], m["y"]))
        check(wait_for(log, "PASS carbon_window_placement", 60), "the program ends well after the menu was dismissed")
    finally:
        pass
    return finish(session, guest)


def finish(session, guest):
    if guest is not None:
        wait_for_guest(guest)
    if session is not None:
        session.close()
    print("%d failure(s)" % failures)
    return 1 if failures else 0


def wait_for_guest(guest):
    try:
        guest.wait(timeout=60)
    except subprocess.TimeoutExpired:
        guest.kill()
        guest.wait()
    subprocess.run([os.environ.get("DARLING", "darling"), "shutdown"], stdin=subprocess.DEVNULL,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=60)


if __name__ == "__main__":
    sys.exit(main())
