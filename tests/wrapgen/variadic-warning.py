#!/usr/bin/env python3
#
# This file is part of Darling.
#
# Copyright (C) 2026 Lubos Dolezel
#
# Darling is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Darling is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Darling.  If not, see <http://www.gnu.org/licenses/>.

# Checks that wrapgen flags wrapped host functions known to be variadic with a
# #warning in the generated source (the guest and host disagree on how the
# variadic tail is passed), keeps generating a wrapper for them, and does not
# warn about ordinary functions.
#
# Usage: variadic-warning.py --wrapgen WRAPGEN
# CC selects the compiler (default: cc).

import argparse
import os
import shlex
import subprocess
import sys
import tempfile

FIXTURE = """
int dbus_message_append_args(void *message, int first_type, ...) { return 0; }
int TIFFGetField(void *tif, unsigned tag, ...) { return 0; }
int plain_function(int a) { return a; }
"""

failures = 0


def check(cond, message):
    global failures
    print(("PASS " if cond else "FAIL ") + message)
    if not cond:
        failures += 1


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--wrapgen", required=True)
    opts = parser.parse_args()
    cc = shlex.split(os.environ.get("CC", "cc"))

    with tempfile.TemporaryDirectory(prefix="wrapgen-variadic-") as tmp:
        src = os.path.join(tmp, "fixture.c")
        with open(src, "w") as f:
            f.write(FIXTURE)
        fixture = os.path.join(tmp, "libvariadicfixture.so")
        r = subprocess.run(cc + ["-shared", "-fPIC", src, "-o", fixture],
                           stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                           universal_newlines=True)
        if r.returncode != 0:
            print(r.stdout)
            return 1
        wrapper = os.path.join(tmp, "wrapper.c")
        r = subprocess.run([opts.wrapgen, fixture, wrapper, os.path.join(tmp, "vars.h")],
                           stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                           universal_newlines=True)
        if r.returncode != 0:
            print(r.stdout)
            return 1
        with open(wrapper) as f:
            lines = f.read().splitlines()

    warnings = [l for l in lines if l.startswith("#warning")]
    for sym in ("dbus_message_append_args", "TIFFGetField"):
        check(any("'" + sym + "'" in w and "variadic" in w for w in warnings),
              "variadic function %s gets a #warning" % sym)
        check(any(("struct lazy_symbol __elf_lazy_symbol_" + sym + " ") in l for l in lines),
              "variadic function %s still gets a wrapper" % sym)
    check(not any("plain_function" in w for w in warnings),
          "ordinary function gets no #warning")
    # A warning text must stay a single well-formed string literal.
    for w in warnings:
        check(w.count('"') == 2 and w.rstrip().endswith('"'),
              "warning text is one string literal: " + w[:40])

    return 1 if failures else 0


sys.exit(main())
