#!/usr/bin/env python3
# Copyright (C) 2026 Darling Developers
"""Host-side test of the optional binfmt_misc helper (src/startup/binfmt/darling-binfmt.c).

Usage: binfmt-helper.py <helper> <stub executable> <stand-in launcher> <stand-in mldr>

Runs the helper against two copies of a stand-in executable, so no installed Darling is needed. Outside
the container the kernel's default argv layout must become "darling exec <binary> <arguments>"; inside
one it must become mldr's "mldr!<binary> <binary> <arguments>".
"""
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

BINARY = "/fake/My App.app/Contents/MacOS/My App"


def fail(message):
    print("FAIL:", message)
    sys.exit(1)


def install_stub(source, path):
    path.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, path)
    path.chmod(0o755)


def run(helper, output, guest):
    if output.exists():
        output.unlink()
    env = os.environ.copy()
    env.pop("__darling_binfmt_launch", None)
    env.pop("__darling_binfmt_guest", None)
    env["DARLING_BINFMT_TEST_OUTPUT"] = str(output)
    if guest:
        env["__darling_binfmt_guest"] = "1"

    # binfmt_misc calls the interpreter with argv[0] set to the Mach-O path. Passing the helper as
    # `executable` reproduces that layout.
    result = subprocess.run([BINARY, "one", "two words"], executable=str(helper), env=env,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    if result.returncode != 0:
        fail(f"helper exited {result.returncode}: {result.stdout}")
    return output.read_text().splitlines()


def expect(what, got, want):
    if got != want:
        fail(f"{what}: got {got!r}, want {want!r}")
    print(f"PASS: {what}")


def main():
    if len(sys.argv) != 5:
        fail(__doc__)

    helper = Path(sys.argv[1])
    stub = Path(sys.argv[2])
    launcher = Path(sys.argv[3])
    mldr = Path(sys.argv[4])
    install_stub(stub, launcher)
    install_stub(stub, mldr)

    with tempfile.TemporaryDirectory(prefix="darling-binfmt-test-") as tmp:
        output = Path(tmp) / "argv"

        expect("host invocation", run(helper, output, guest=False), [
            "argv0=darling",
            "arg0=exec",
            f"arg1={BINARY}",
            "arg2=one",
            "arg3=two words",
            "launch=unset",
            "guest=unset",
        ])

        expect("guest invocation", run(helper, output, guest=True), [
            f"argv0=mldr!{BINARY}",
            f"arg0={BINARY}",
            "arg1=one",
            "arg2=two words",
            "launch=unset",
            "guest=1",
        ])


if __name__ == "__main__":
    main()
