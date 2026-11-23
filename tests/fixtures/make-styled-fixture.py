#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
"""Builds styled-test.dmg, a disk image whose window Finder styles (tests/finder.sh, "styled window"). Runs on macOS
(it needs hdiutil and xattr). Usage: make-styled-fixture.py <out.dmg> [--license <resources.xml>]

With --license, the licence agreement resources of make-sla-fixture.py's output are embedded with
`hdiutil udifrez` (styled-sla-test.dmg, tests/open-disk-image.sh).

The volume "Styled Test" holds:
- "Styled App.app", an application bundle with only Contents/Info.plist (CFBundlePackageType APPL);
- "Applications", a symbolic link to /Applications;
- ".background/background.png", 640x400, solid #2F6F9F with a #F0F0F0 band from x 260 to 380 at y 190 to 210 (from
  the top left);
- "Hidden Notes.txt", made invisible with the Finder's kIsInvisible flag in its com.apple.FinderInfo;
- the .DS_Store of tests/fixtures/dsstore/styled.DS_Store (make-dsstore-fixtures.py): a 640x400 window without
  toolbar or sidebar, 128-point icons, 13-point text, the picture above as the background, "Styled App.app" at
  (160, 200) and "Applications" at (480, 200).
"""
import importlib.util
import os
import plistlib
import struct
import subprocess
import sys
import tempfile
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from dsstore import alias_v2, write_store  # noqa: E402

spec = importlib.util.spec_from_file_location("make_dsstore_fixtures", os.path.join(HERE, "make-dsstore-fixtures.py"))
fixtures = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fixtures)


def png(width, height, pixel):
    """An RGB PNG whose pixel(x, y) gives each pixel's bytes"""
    rows = b"".join(b"\0" + b"".join(pixel(x, y) for x in range(width)) for y in range(height))

    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)

    header = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    return b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", header) + chunk(b"IDAT", zlib.compress(rows, 9)) + chunk(b"IEND", b"")


def background():
    blue, light = bytes([0x2F, 0x6F, 0x9F]), bytes([0xF0, 0xF0, 0xF0])
    return png(640, 400, lambda x, y: light if 260 <= x < 380 and 190 <= y < 210 else blue)


def main():
    if len(sys.argv) not in (2, 4) or (len(sys.argv) == 4 and sys.argv[2] != "--license"):
        sys.exit("usage: make-styled-fixture.py <out.dmg> [--license <resources.xml>]")
    out = os.path.abspath(sys.argv[1])
    license_xml = os.path.abspath(sys.argv[3]) if len(sys.argv) == 4 else None
    with tempfile.TemporaryDirectory() as work:
        root = os.path.join(work, "Styled Test")
        os.makedirs(os.path.join(root, "Styled App.app", "Contents"))
        with open(os.path.join(root, "Styled App.app", "Contents", "Info.plist"), "wb") as f:
            plistlib.dump({"CFBundleIdentifier": "org.darlinghq.test.styled-app", "CFBundleName": "Styled App",
                           "CFBundlePackageType": "APPL"}, f)
        os.symlink("/Applications", os.path.join(root, "Applications"))
        os.makedirs(os.path.join(root, ".background"))
        with open(os.path.join(root, ".background", "background.png"), "wb") as f:
            f.write(background())
        notes = os.path.join(root, "Hidden Notes.txt")
        with open(notes, "w") as f:
            f.write("Finder does not show this file.\n")
        finder_info = "00" * 8 + "4000" + "00" * 22
        subprocess.run(["xattr", "-wx", "com.apple.FinderInfo", finder_info, notes], check=True)
        store = write_store(fixtures.styled(alias_v2("Styled Test", "/.background/background.png")))
        with open(os.path.join(root, ".DS_Store"), "wb") as f:
            f.write(store)
        if os.path.exists(out):
            os.remove(out)
        subprocess.run(["hdiutil", "create", "-quiet", "-fs", "HFS+", "-volname", "Styled Test", "-srcfolder", root,
                        "-format", "UDZO", out], check=True)
        if license_xml:
            subprocess.run(["hdiutil", "udifrez", out, "-xml", license_xml], check=True)


if __name__ == "__main__":
    main()
