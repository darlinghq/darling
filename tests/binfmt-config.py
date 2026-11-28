#!/usr/bin/env python3
# Copyright (C) 2026 Darling Developers
"""Host-side test of the optional binfmt_misc configuration (src/startup/binfmt/darling-host.conf.in).

Usage: binfmt-config.py <generated config> <installed interpreter path>

Checks the registration syntax documented in the kernel's binfmt_misc admin guide: the three Mach-O
magics (the fat one with a mask that leaves Java class files alone), the host interpreter, and the P
flag that makes the kernel pass the caller's argv[0] to the interpreter after the file path.
"""
import sys
from pathlib import Path

MAGICS = {
    "Darling-host-Mach-O-64": r"\xcf\xfa\xed\xfe",
    "Darling-host-Mach-O-32": r"\xce\xfa\xed\xfe",
    "Darling-host-fat-Mach-O": r"\xca\xfe\xba\xbe\x00\x00\x00\x00",
}
MASKS = {"Darling-host-fat-Mach-O": r"\xff\xff\xff\xff\xff\xff\xff\xe0"}


def fail(message):
    print("FAIL:", message)
    sys.exit(1)


def main():
    if len(sys.argv) != 3:
        fail(__doc__)

    config = Path(sys.argv[1])
    interpreter = sys.argv[2]
    lines = []
    for line in config.read_text().splitlines():
        line = line.strip()
        if line and not line.startswith("#"):
            lines.append(line)

    if len(lines) != len(MAGICS):
        fail(f"expected {len(MAGICS)} registrations, found {len(lines)}")

    found = set()
    for line in lines:
        if not line.startswith(":"):
            fail(f"registration does not start with the delimiter: {line}")
        fields = line[1:].split(":")
        if len(fields) != 7:
            fail(f"expected seven fields, found {len(fields)}: {line}")
        name, kind, offset, magic, mask, actual_interpreter, flags = fields
        if name not in MAGICS or name in found:
            fail(f"unexpected or duplicate registration name: {name}")
        found.add(name)
        if kind != "M":
            fail(f"{name}: expected magic matching, got {kind!r}")
        if offset != "":
            fail(f"{name}: the magic starts at offset zero, got {offset!r}")
        if magic != MAGICS[name]:
            fail(f"{name}: magic is {magic!r}, want {MAGICS[name]!r}")
        if mask != MASKS.get(name, ""):
            fail(f"{name}: mask is {mask!r}, want {MASKS.get(name, )!r}")
        if actual_interpreter != interpreter:
            fail(f"{name}: interpreter is {actual_interpreter!r}, want {interpreter!r}")
        if flags != "P":
            fail(f"{name}: only the P flag is wanted, got {flags!r}")

    if found != set(MAGICS):
        fail(f"missing registrations: {set(MAGICS) - found}")

    print(f"PASS: {config} registers the three Mach-O magics with {interpreter}")


if __name__ == "__main__":
    main()
