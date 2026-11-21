#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
"""Generates the .DS_Store fixtures in tests/fixtures/dsstore/ (committed; rerun after changing them).
Usage: make-dsstore-fixtures.py [<output directory>]. Each fixture is described in tests/finder.md."""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsstore import Raw, iloc, write_store  # noqa: E402

DECOMPOSED_U = "ü"


def all_types():
    return [
        (".", "vSrn", "long", 1),
        (".", "ICVO", "bool", True),
        ("a", "lg1S", "comp", 123456789012),
        ("a", "modD", "dutc", 0x0123456789ABCDEF),
        (DECOMPOSED_U, "type", "type", "icnv"),
        ("日本", "cmmt", "ustr", "Kommentar 😀"),
        ("😀", "Iloc", "blob", iloc(10, 20)),
        ("😀", "dscl", "shor", 7),
    ]


def multi_level(count=3000):
    return [("item %d.txt" % i, "Iloc", "blob", iloc(i, i * 2)) for i in range(count)]


def utf16(name):
    data = name.encode("utf-16-be")
    return struct.pack(">I", len(data) // 2) + data


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.abspath(__file__)), "dsstore")
    os.makedirs(out, exist_ok=True)
    fixtures = {
        "all-types": write_store(all_types()),
        "multi-level": write_store(multi_level()),
        "empty": write_store([]),
        "cycle": write_store(multi_level(200), corrupt="cycle"),
        "block-out-of-range": write_store(all_types(), corrupt="block-out-of-range"),
        "bad-type": write_store([Raw(utf16("x") + b"Ilocnope" + b"\0\0\0\0", "x")]),
        "huge-count": write_store(all_types(), corrupt="huge-count"),
        "long-name": write_store([Raw(struct.pack(">I", 0x7FFFFFFF) + "x".encode("utf-16-be") + b"Iloclong\0\0\0\1", "x")]),
        "bad-magic": write_store(all_types(), corrupt="bad-magic"),
    }
    for name, data in fixtures.items():
        with open(os.path.join(out, name + ".DS_Store"), "wb") as f:
            f.write(data)


if __name__ == "__main__":
    main()
