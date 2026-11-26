#!/usr/bin/env python3
# Integration test: MediaPlayer now-playing info and remote commands reach a
# D-Bus session bus as MPRIS 2 objects, and MPRIS calls reach the handlers that
# the player registered with MPRemoteCommandCenter.
#
# usage: python3 tests/mediaplayer-mpris-session-bus.py <bindir> [arch ...]
#
# <bindir> holds the guest player built from tests/src/mediaplayer_mpris_player.m,
# as mediaplayer_mpris_player.<arch>, for each arch (default: x86_64 i386).
#
# A private dbus-daemon plays the session bus and a private mediaremoted
# instance (on its own Mach service name) is started in the prefix and pointed
# at it, so the real session bus is never touched. Needs darling, dbus-daemon
# and busctl on the host.

import os
import re
import shutil
import subprocess
import sys
import tempfile
import time

if len(sys.argv) < 2:
    print("usage: mediaplayer-mpris-session-bus.py <bindir> [arch ...]")
    sys.exit(2)
bindir = os.path.abspath(sys.argv[1])
archs = sys.argv[2:] or ["x86_64", "i386"]

for tool in ("darling", "dbus-daemon", "busctl"):
    if shutil.which(tool) is None:
        print(f"SKIP: {tool} is not installed")
        sys.exit(0)

PLAYER_IFACE = "org.mpris.MediaPlayer2.Player"
PATH = "/org/mpris/MediaPlayer2"
PREFIX = "org.mpris.MediaPlayer2.darling."
failures = 0


def check(cond, what):
    global failures
    print(("PASS: " if cond else "FAIL: ") + what)
    if not cond:
        failures += 1


def guest(path):
    return "/Volumes/SystemRoot" + path


def wait_for(fn, timeout=15.0):
    end = time.time() + timeout
    while time.time() < end:
        value = fn()
        if value:
            return value
        time.sleep(0.2)
    return None


work = tempfile.mkdtemp(prefix="mpris-test-", dir=os.environ.get("TMPDIR"))
bus_path = os.path.join(work, "bus")
address = f"unix:path={bus_path}"
service = f"org.darling.test.mediaremoted.{os.getpid()}"
bus = subprocess.Popen(["dbus-daemon", "--session", f"--address={address}", "--nofork", "--nopidfile"])
procs = [bus]


def busctl(*args):
    return subprocess.run(["busctl", f"--address={address}", *args],
                          capture_output=True, text=True, timeout=30)


def names():
    return [n.split()[0] for n in busctl("list", "--no-legend").stdout.splitlines()
            if n.startswith(PREFIX)]


def prop(name, p, iface=PLAYER_IFACE):
    r = busctl("get-property", name, PATH, iface, p)
    return r.stdout.strip() if r.returncode == 0 else None


def call(name, method, *args, iface=PLAYER_IFACE):
    return busctl("call", name, PATH, iface, method, *args)


def start_player(label, arch, title, artist, *flags):
    d = os.path.join(work, label)
    os.mkdir(d)
    # the process name, and so the bus name, comes from the file name
    link = os.path.join(work, label)
    exe = os.path.join(work, f"{label}-bin")
    os.symlink(os.path.join(bindir, f"mediaplayer_mpris_player.{arch}"), exe)
    p = subprocess.Popen(["darling", "shell", "env", f"__MEDIAREMOTED_SERVICE_NAME={service}",
                          guest(exe), guest(d), title, artist, *flags],
                         stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    procs.append(p)
    ok = wait_for(lambda: os.path.exists(os.path.join(d, "ready")), 40)
    check(ok, f"{label}: player started")
    return p, d


def stop_player(p, d):
    open(os.path.join(d, "stop"), "w").close()
    try:
        p.wait(timeout=20)
    except subprocess.TimeoutExpired:
        p.kill()


def log_of(d):
    try:
        return open(os.path.join(d, "log")).read().split("\n")[:-1]
    except OSError:
        return []


def number(text):
    return int(text.split()[-1])


def run_arch(arch):
    print(f"--- {arch} ---")
    # a player that only registers command targets is not a now-playing app
    bare, bare_dir = start_player(f"bare-{arch}", arch, "Bare", "Nobody", "--commands-only")
    time.sleep(1.5)
    check(names() == [], f"{arch}: a player without now-playing info claims no bus name")

    p, d = start_player(f"alpha-{arch}", arch, "Song", "Singer", "--artwork")
    found = wait_for(lambda: names())
    check(found is not None and len(found) == 1 and "alpha" in found[0], f"{arch}: the player owns a bus name ({found})")
    if not found:
        stop_player(p, d)
        stop_player(bare, bare_dir)
        return
    n = found[0]

    check(prop(n, "PlaybackStatus") == 's "Playing"', f"{arch}: PlaybackStatus is Playing")
    meta = prop(n, "Metadata") or ""
    check('"xesam:title" s "Song 1"' in meta, f"{arch}: Metadata has the title")
    check('"xesam:artist" as 1 "Singer"' in meta, f"{arch}: Metadata has the artist as a list")
    check('"xesam:album" s "The Album"' in meta, f"{arch}: Metadata has the album")
    check('"mpris:length" x 240000000' in meta, f"{arch}: Metadata has the length in microseconds")
    check("mpris:artUrl" in meta and "darling_art_" in meta, f"{arch}: Metadata has the artwork URL")
    art = re.search(r'"mpris:artUrl" s "file://([^"]+)"', meta)
    check(art is not None and os.path.getsize(art.group(1)) > 0, f"{arch}: the artwork file exists on the host")
    check(prop(n, "CanPlay") == "b true" and prop(n, "CanPause") == "b true", f"{arch}: CanPlay and CanPause")
    check(prop(n, "CanGoNext") == "b true" and prop(n, "CanSeek") == "b true", f"{arch}: CanGoNext and CanSeek")
    check(prop(n, "Identity", "org.mpris.MediaPlayer2") is not None, f"{arch}: Identity")
    check(prop(n, "NoSuchProperty") is None, f"{arch}: an unknown property is an error")

    # the position advances while playing, from the elapsed time of the info
    p1 = number(prop(n, "Position"))
    check(15_000_000 <= p1 < 40_000_000, f"{arch}: Position starts from the elapsed time ({p1})")
    time.sleep(1.2)
    p2 = number(prop(n, "Position"))
    check(1_000_000 <= p2 - p1 <= 3_000_000, f"{arch}: Position advances in real time ({p2 - p1})")

    # Pause reaches the handler, the state changes, and the position stays where it is
    call(n, "Pause")
    check(wait_for(lambda: prop(n, "PlaybackStatus") == 's "Paused"'), f"{arch}: Pause reaches the pause handler")
    check("pause" in log_of(d), f"{arch}: the handler logged pause")
    q1 = number(prop(n, "Position"))
    time.sleep(1.0)
    q2 = number(prop(n, "Position"))
    check(q1 == q2 and q1 >= p2, f"{arch}: Position does not move while paused or jump back ({q1} {q2} {p2})")

    call(n, "Play")
    check(wait_for(lambda: prop(n, "PlaybackStatus") == 's "Playing"'), f"{arch}: Play reaches the play handler")
    call(n, "PlayPause")
    check(wait_for(lambda: prop(n, "PlaybackStatus") == 's "Paused"'), f"{arch}: PlayPause toggles to Paused")
    call(n, "PlayPause")
    check(wait_for(lambda: prop(n, "PlaybackStatus") == 's "Playing"'), f"{arch}: PlayPause toggles to Playing")
    check(log_of(d).count("togglePlayPause") == 2, f"{arch}: the toggle handler ran twice")
    # Play while playing must not pause
    before = log_of(d).count("togglePlayPause")
    call(n, "Play")
    time.sleep(0.8)
    check(prop(n, "PlaybackStatus") == 's "Playing"' and log_of(d).count("togglePlayPause") == before,
          f"{arch}: Play while playing does not toggle")

    # SetPosition: right track id seeks, a wrong one does nothing
    meta = prop(n, "Metadata")
    tid = re.search(r'"mpris:trackid" o "([^"]+)"', meta).group(1)
    call(n, "SetPosition", "ox", "/org/mpris/MediaPlayer2/Track/9999", "100000000")
    call(n, "SetPosition", "ox", tid, "90000000")
    check(wait_for(lambda: "changePlaybackPosition 90.000" in log_of(d)), f"{arch}: SetPosition reaches the position handler")
    check(not any("100.000" in l for l in log_of(d)), f"{arch}: SetPosition for another track is ignored")
    check(wait_for(lambda: 90_000_000 <= number(prop(n, "Position")) < 95_000_000), f"{arch}: Position follows the seek")

    # Seek is relative to the current position
    cur = number(prop(n, "Position"))
    call(n, "Seek", "x", "5000000")
    check(wait_for(lambda: any(l.startswith("changePlaybackPosition") and abs(float(l.split()[1]) - (cur / 1e6 + 5)) < 1.5
                               for l in log_of(d)[2:])), f"{arch}: Seek moves by the offset from the real position")

    # Next changes the track: new metadata and a new track id
    call(n, "Next")
    check(wait_for(lambda: '"xesam:title" s "Song 2"' in (prop(n, "Metadata") or "")), f"{arch}: Next reaches the handler, the title changes")
    tid2 = re.search(r'"mpris:trackid" o "([^"]+)"', prop(n, "Metadata")).group(1)
    check(tid2 != tid, f"{arch}: a new track has a new track id")
    call(n, "Previous")
    check(wait_for(lambda: "previousTrack" in log_of(d)), f"{arch}: Previous reaches the handler")
    call(n, "Stop")
    check(wait_for(lambda: prop(n, "PlaybackStatus") == 's "Stopped"'), f"{arch}: Stop reaches the handler")

    # a second player that plays takes the name over; the first returns when it leaves
    call(n, "Play")
    wait_for(lambda: prop(n, "PlaybackStatus") == 's "Playing"')
    p2_, d2 = start_player(f"beta-{arch}", arch, "Other", "Someone")
    check(wait_for(lambda: names() and "beta" in names()[0]), f"{arch}: the player that plays last owns the name ({names()})")
    check(len(names()) == 1, f"{arch}: only one player is exposed")
    stop_player(p2_, d2)
    check(wait_for(lambda: names() and "alpha" in names()[0]), f"{arch}: the first player is back after the second one left ({names()})")

    stop_player(p, d)
    check(wait_for(lambda: names() == []), f"{arch}: the name is released when the player exits")
    check(art is None or not os.path.exists(art.group(1)), f"{arch}: the artwork file is removed when the player exits")
    stop_player(bare, bare_dir)


try:
    if not wait_for(lambda: os.path.exists(bus_path), 10):
        print("ERROR: dbus-daemon did not start")
        sys.exit(1)
    daemon = subprocess.Popen(["darling", "shell", "env", f"__MEDIAREMOTED_SERVICE_NAME={service}",
                               f"__MEDIAREMOTED_DBUS_ADDRESS={address}", "/usr/libexec/mediaremoted"],
                              stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    procs.append(daemon)
    time.sleep(3)
    for arch in archs:
        run_arch(arch)
finally:
    for pr in reversed(procs):
        if pr.poll() is None:
            pr.terminate()
            try:
                pr.wait(timeout=10)
            except subprocess.TimeoutExpired:
                pr.kill()
    shutil.rmtree(work, ignore_errors=True)

print(f"Failures: {failures}")
sys.exit(1 if failures else 0)
