#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
#
# Integration test: SecItem passwords of the data protection keychain (the database
# of securityd) are stored: an add, a duplicate add, a copy, an update, a copy of the
# update and a delete work across separate processes.
#
# usage: python3 tests/secitem-dp-keychain.py <bindir> [arch ...]
#
# <bindir> holds the guest program built from tests/src/secitem_dp_keychain.c as
# secitem_dp_keychain.<arch> (default archs: x86_64 i386). securityd serves the data
# protection keychain only to clients that carry an application identifier or keychain
# access groups as entitlements, so the driver signs a copy of each program with
# tests/secitem-dp-keychain-entitlements.plist using adhoc-sign (the tool of the build,
# found through $ADHOC_SIGN, then $DARLING_BUILD/src/buildtools/adhoc-sign, then PATH).
# The prefix is the one of DPREFIX; use a throw-away one.
import os
import shutil
import subprocess
import sys
import tempfile

if len(sys.argv) < 2:
    print("usage: secitem-dp-keychain.py <bindir> [arch ...]")
    sys.exit(2)
bindir = os.path.abspath(sys.argv[1])
archs = sys.argv[2:] or ["x86_64", "i386"]
here = os.path.dirname(os.path.abspath(__file__))
plist = os.path.join(here, "secitem-dp-keychain-entitlements.plist")

signer = os.environ.get("ADHOC_SIGN")
if not signer and os.environ.get("DARLING_BUILD"):
    signer = os.path.join(os.environ["DARLING_BUILD"], "src/buildtools/adhoc-sign")
signer = signer or shutil.which("adhoc-sign")
if not signer or not os.access(signer, os.X_OK) or shutil.which("darling") is None:
    print("SKIP: adhoc-sign and darling are needed")
    sys.exit(0)

failures = 0


def check(cond, what):
    global failures
    print(("PASS: " if cond else "FAIL: ") + what)
    if not cond:
        failures += 1


work = tempfile.mkdtemp(prefix="secitem-dp-", dir=os.environ.get("TMPDIR"))


def step(exe, arch, steps):
    out = os.path.join(work, "out.txt")
    with open(out, "w") as f:
        subprocess.run(["timeout", "-s", "KILL", "120", "darling", "shell", "/Volumes/SystemRoot" + exe, *steps],
                       stdin=subprocess.DEVNULL, stdout=f, stderr=subprocess.STDOUT)
    text = open(out).read()
    ok = "ALL PASSED" in text and "FAIL" not in text
    check(ok, f"{arch}: {' '.join(steps)}" + ("" if ok else f": {text.strip()[-400:]!r}"))
    return text


try:
    for arch in archs:
        print(f"--- {arch} ---")
        exe = os.path.join(work, f"secitem_dp_keychain.{arch}")
        shutil.copy(os.path.join(bindir, f"secitem_dp_keychain.{arch}"), exe)
        subprocess.run([signer, f"--entitlements={plist}", exe], check=True)
        step(exe, arch, ["clean"])
        # every step is a process of its own, so a step only passes if the earlier ones left their items behind
        for name in ("add", "add-again", "copy", "update", "copy-updated", "delete", "gone"):
            step(exe, arch, [name])
        # the whole round trip in one process
        step(exe, arch, ["add", "copy", "update", "copy-updated", "delete", "gone"])
finally:
    shutil.rmtree(work, ignore_errors=True)

print("FAILED %d" % failures if failures else "ALL PASSED")
sys.exit(1 if failures else 0)
