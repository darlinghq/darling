#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
#
# Integration test: the CoreBluetooth central role works over BlueZ's D-Bus API.
#
# usage: python3 tests/corebluetooth-bluez-central.py <bindir> [arch ...]
#
# <bindir> holds the guest program built from tests/src/corebluetooth_central.m
# as corebluetooth_central.<arch>, for each arch (default: x86_64 i386). The
# 32-bit slice keeps the stubs (no central role), so for i386 the program only
# checks that calling it does not crash.
#
# A private dbus-daemon plays the system bus and tests/corebluetooth-mock-bluez.py
# plays org.bluez on it (Adapter1, Device1 and the Gatt* interfaces), so the
# host's bluetoothd and system bus are never touched; the guest finds the bus
# through DBUS_SYSTEM_BUS_ADDRESS. Needs darling, dbus-daemon and python3 with
# dbus-python and PyGObject on the host.

import os
import shutil
import subprocess
import sys
import tempfile
import time

if len(sys.argv) < 2:
    print("usage: corebluetooth-bluez-central.py <bindir> [arch ...]")
    sys.exit(2)
bindir = os.path.abspath(sys.argv[1])
archs = sys.argv[2:] or ["x86_64", "i386"]
here = os.path.dirname(os.path.abspath(__file__))

for tool in ("darling", "dbus-daemon"):
    if shutil.which(tool) is None:
        print(f"SKIP: {tool} is not installed")
        sys.exit(0)
try:
    import dbus  # noqa: F401
    from gi.repository import GLib  # noqa: F401
except ImportError:
    print("SKIP: dbus-python and PyGObject are needed for the mock BlueZ")
    sys.exit(0)

failures = 0


def check(cond, what):
    global failures
    print(("PASS: " if cond else "FAIL: ") + what)
    if not cond:
        failures += 1


work = tempfile.mkdtemp(prefix="cb-bluez-test-", dir=os.environ.get("TMPDIR"))
bus_path = os.path.join(work, "system_bus_socket")
address = f"unix:path={bus_path}"
bus_conf = os.path.join(work, "bus.conf")
with open(bus_conf, "w") as f:
    f.write("""<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-Bus Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
  <type>system</type>
  <listen>%s</listen>
  <auth>EXTERNAL</auth>
  <policy context="default">
    <allow send_destination="*" eavesdrop="true"/>
    <allow eavesdrop="true"/>
    <allow own="*"/>
  </policy>
</busconfig>
""" % address)
bus = subprocess.Popen(["dbus-daemon", f"--config-file={bus_conf}", "--nofork", "--nopidfile"],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
procs = [bus]


def start_mock(adapter):
    log = os.path.join(work, f"bluez-{adapter}.log")
    p = subprocess.Popen([sys.executable, os.path.join(here, "corebluetooth-mock-bluez.py"),
                          "--address", address, "--log", log, "--adapter", adapter],
                         stdout=subprocess.PIPE, text=True)
    procs.append(p)
    ready = p.stdout.readline().strip() == "READY"
    return p, log, ready


def stop(p):
    p.terminate()
    try:
        p.wait(timeout=10)
    except subprocess.TimeoutExpired:
        p.kill()
    if p in procs:
        procs.remove(p)


def run_guest(arch, mode, out):
    exe = os.path.join(bindir, f"corebluetooth_central.{arch}")
    with open(out, "w") as f:
        subprocess.run(["timeout", "-s", "KILL", "120", "darling", "shell", "env",
                        f"DBUS_SYSTEM_BUS_ADDRESS={address}", "/Volumes/SystemRoot" + exe, mode],
                       stdin=subprocess.DEVNULL, stdout=f, stderr=subprocess.STDOUT)
    return open(out).read()


def lines(path):
    try:
        return open(path).read().splitlines()
    except OSError:
        return []


try:
    deadline = time.time() + 10
    while not os.path.exists(bus_path) and time.time() < deadline:
        time.sleep(0.1)
    for arch in archs:
        print(f"--- {arch} ---")
        if arch == "i386":
            text = run_guest(arch, "powered", os.path.join(work, "guest-i386.out"))
            check("ALL PASSED" in text and "FAIL" not in text, f"{arch}: the 32-bit slice loads: {text.strip()[-200:]!r}")
            continue

        text = run_guest(arch, "uuid", os.path.join(work, f"guest-{arch}-uuid.out"))
        check("ALL PASSED" in text and "FAIL" not in text, f"{arch}: CBUUID widths and equality: {text.strip()[-200:]!r}")

        mock, log, ready = start_mock("on")
        check(ready, f"{arch}: the mock BlueZ owns org.bluez")
        text = run_guest(arch, "powered", os.path.join(work, f"guest-{arch}.out"))
        sys.stdout.write("".join("  guest: " + l + "\n" for l in text.splitlines() if not l.startswith("PASS")))
        passed = [l for l in text.splitlines() if l.startswith("PASS")]
        check("ALL PASSED" in text and "FAIL" not in text,
              f"{arch}: the guest central-role flow passed ({len(passed)} checks)")
        calls = lines(log)
        check("setdiscoveryfilter uuids=0000180f-0000-1000-8000-00805f9b34fb transport=le duplicate=false" in
              [l.lower() for l in calls],
              f"{arch}: a filtered scan sets the BlueZ discovery filter with the expanded 128-bit UUID")
        check("StartDiscovery" in calls and "StopDiscovery" in calls, f"{arch}: scanning starts and stops discovery")
        dev1 = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_01"
        chr1 = dev1 + "/service0010/char0011"
        check(f"Connect {dev1}" in calls and f"Disconnect {dev1}" in calls, f"{arch}: connect and disconnect reach Device1")
        check(f"WriteValue {chr1} data=0a0b type=request" in calls,
              f"{arch}: a write with response sends type=request")
        check(f"WriteValue {chr1} data=0c type=command" in calls,
              f"{arch}: a write without response sends type=command")
        check(any(l.startswith("WriteDescriptor") and "type=" in l and l.endswith("type=") for l in calls),
              f"{arch}: a descriptor write does not send the characteristic-only type option")
        check(f"StartNotify {chr1}" in calls and f"StopNotify {chr1}" in calls,
              f"{arch}: notifications use StartNotify and StopNotify")
        stop(mock)

        for mode, key in (("unpowered", "unpowered"), ("noadapter", "off")):
            mock, log, ready = start_mock(key)
            text = run_guest(arch, mode, os.path.join(work, f"guest-{arch}-{mode}.out"))
            check("ALL PASSED" in text and "FAIL" not in text, f"{arch}: manager state with {mode} adapter: {text.strip()[-160:]!r}")
            stop(mock)

        # without any system bus at all the manager must report Unsupported, not hang or crash
        out = os.path.join(work, f"guest-{arch}-nobus.out")
        with open(out, "w") as f:
            subprocess.run(["timeout", "-s", "KILL", "60", "darling", "shell", "env",
                            f"DBUS_SYSTEM_BUS_ADDRESS=unix:path={work}/missing",
                            "/Volumes/SystemRoot" + os.path.join(bindir, f"corebluetooth_central.{arch}"), "noadapter"],
                           stdin=subprocess.DEVNULL, stdout=f, stderr=subprocess.STDOUT)
        text = open(out).read()
        check("ALL PASSED" in text, f"{arch}: with no system bus the manager reports Unsupported: {text.strip()[-120:]!r}")
finally:
    for p in procs:
        p.terminate()
    for p in procs:
        try:
            p.wait(timeout=10)
        except subprocess.TimeoutExpired:
            p.kill()
    shutil.rmtree(work, ignore_errors=True)

print("FAILED %d" % failures if failures else "ALL PASSED")
sys.exit(1 if failures else 0)
