#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
"""Generates the .DS_Store fixtures in tests/fixtures/dsstore/ (committed; rerun after changing them).
Usage: make-dsstore-fixtures.py [<output directory>]. Each fixture is described in tests/finder.md."""
import os
import plistlib
import struct
import sys
import unicodedata

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsstore import Raw, alias_v2, bookmark, bwsp, icvp, iloc, write_store  # noqa: E402

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


def styled(picture):
    """The settings of the Styled Test disk image's window (tests/fixtures/make-styled-fixture.py), with the
    background picture given as alias or bookmark data."""
    return [
        (".", "bwsp", "blob", bwsp({"WindowBounds": "{{200, 120}, {640, 400}}", "ShowToolbar": False,
                                     "ShowSidebar": False})),
        (".", "icvp", "blob", icvp({"iconSize": 128.0, "textSize": 13.0, "labelOnBottom": True, "backgroundType": 2,
                                     "backgroundImageAlias": picture})),
        (".", "vSrn", "long", 1),
        (".", "vstl", "type", "icnv"),
        ("Styled App.app", "Iloc", "blob", iloc(160, 200)),
        ("Applications", "Iloc", "blob", iloc(480, 200)),
        (unicodedata.normalize("NFD", "Ünï 😀.txt"), "Iloc", "blob", iloc(320, 330)),
    ]


def picture_with_colour(path):
    """A picture background (type 2) at path inside the volume "Styled Test", with the colour (0.2, 0.4, 0.6) too."""
    return [(".", "icvp", "blob", icvp({"iconSize": 64.0, "backgroundType": 2, "backgroundColorRed": 0.2,
                                        "backgroundColorGreen": 0.4, "backgroundColorBlue": 0.6,
                                        "backgroundImageAlias": alias_v2("Styled Test", path)}))]


def icv4(size, label):
    """An older 'icvo' blob: "icv4", the icon size (2 bytes), the arrangement ("none"), the label position ("botm" or
    "rght"), then 12 more bytes."""
    return b"icv4" + struct.pack(">H", size) + b"none" + label.encode("ascii") + b"\0" * 12


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
        "styled": write_store(styled(alias_v2("Styled Test", "/.background/background.png"))),
        "styled-bookmark": write_store(styled(bookmark("/Volumes/Styled Test",
                                                        "/Volumes/Styled Test/.background/background.png"))),
        "escape-dotdot": write_store(styled(alias_v2("Styled Test", "/../../etc/passwd"))),
        "escape-link": write_store(styled(alias_v2("Styled Test", "/.background/link.png"))),
        "colour": write_store([(".", "icvp", "blob", icvp({"backgroundType": 1, "backgroundColorRed": 0.2,
                                                           "backgroundColorGreen": 0.4, "backgroundColorBlue": 0.6}))]),
        # BKGD "ClrB": the colour as 16-bit red, green and blue, then 2 bytes
        "legacy": write_store([(".", "icvo", "blob", icv4(72, "rght")),
                               (".", "BKGD", "blob", b"ClrB" + struct.pack(">HHHH", 0x3333, 0x6666, 0x9999, 0))]),
        # Only what Finder keeps about sizes and dates (as in /Applications/.DS_Store on macOS 26.7.3)
        "no-settings": write_store([("Xcode.app", "lg1S", "comp", 4096), ("Xcode.app", "ph1S", "comp", 8192),
                                    (".", "vSrn", "long", 1)]),
        # Positions off the top left, far to the right, two items at one point, and one for an item that isn't there
        "hostile-positions": write_store([("a.txt", "Iloc", "blob", iloc(-500 & 0xFFFFFFFF, -500 & 0xFFFFFFFF)),
                                          ("b.txt", "Iloc", "blob", iloc(100000, 100)),
                                          ("c.txt", "Iloc", "blob", iloc(50, 50)),
                                          ("d.txt", "Iloc", "blob", iloc(50, 50)),
                                          ("missing.txt", "Iloc", "blob", iloc(10, 10))]),
        "bounds-small": write_store([(".", "bwsp", "blob", bwsp({"WindowBounds": "{{0, 0}, {10, 10}}"}))]),
        "bounds-huge": write_store([(".", "bwsp", "blob", bwsp({"WindowBounds": "{{-500, -500}, {100000, 100000}}"}))]),
        "bounds-garbage": write_store([(".", "bwsp", "blob", bwsp({"WindowBounds": "nonsense"}))]),
        "icon-tiny": write_store([(".", "icvp", "blob", icvp({"iconSize": 0.0}))]),
        "icon-huge": write_store([(".", "icvp", "blob", icvp({"iconSize": 4096.0}))]),
        "list": write_store([(".", "vstl", "type", "Nlsv")]),
        # Numbers that aren't finite: sizes and colour components NaN, window bounds that scan as infinite
        "icon-nan": write_store([(".", "icvp", "blob", icvp({"iconSize": float("nan"), "textSize": float("nan"),
                                                             "backgroundType": 1, "backgroundColorRed": float("nan"),
                                                             "backgroundColorGreen": 0.4, "backgroundColorBlue": 0.6}))]),
        "bounds-inf": write_store([(".", "bwsp", "blob", bwsp({"WindowBounds": "{{0, 0}, {1e400, 1e400}}"}))]),
        # Finite, but so large that adding a title bar's height changes nothing
        "bounds-vast": write_store([(".", "bwsp", "blob", bwsp({"WindowBounds": "{{0, 0}, {1e39, 1e39}}"}))]),
        "bounds-nan": write_store([(".", "bwsp", "blob", bwsp({"WindowBounds": "{{0, 0}, {nan, nan}}"}))]),
        # A picture that can't be used, with a colour to fall back on: a file that isn't an image, and one wider than
        # any screen
        "picture-not-image": write_store(picture_with_colour("/.background/notes.png")),
        "picture-too-wide": write_store(picture_with_colour("/.background/wide.png")),
        "wrong-types": write_store([("Styled App.app", "Iloc", "long", 5),
                                    (".", "bwsp", "blob", plistlib.dumps(["WindowBounds"], fmt=plistlib.FMT_BINARY)),
                                    (".", "icvp", "blob", icvp({"iconSize": "big"}))]),
    }
    for name, data in fixtures.items():
        with open(os.path.join(out, name + ".DS_Store"), "wb") as f:
            f.write(data)


if __name__ == "__main__":
    main()
