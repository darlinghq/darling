#!/usr/bin/env python3
"""Makes the damaged images dmg-api checks that opening them fails cleanly or stays in bounds.

Usage: make-crafted-fixtures.py <raw HFS+ image of a volume named "V"> <unmount-test.dmg> <output dir>

long-name-key.hfs: the raw image (made on macOS with `hdiutil create -size 1m -fs HFS+ -layout NONE -volname V`,
then `hdiutil convert -format UDTO`) with the catalog key of the root folder claiming a 65535-character name.
bad-blkx.dmg: unmount-test.dmg with one character of the Apple_HFS partition's 'blkx' base64 data replaced by '*'.
"""
import sys

raw = bytearray(open(sys.argv[1], "rb").read())
# catalog key of the root folder: key length 8, parent ID 1 (root parent), name length 1, "V"
key = bytes.fromhex("0008 00000001 0001 0056")
at = raw.find(key)
assert at > 0 and raw.find(key, at + 1) < 0, "expected exactly one root folder key"
raw[at + 6:at + 8] = b"\xff\xff"
open(sys.argv[3] + "/long-name-key.hfs", "wb").write(raw)

dmg = bytearray(open(sys.argv[2], "rb").read())
blkx = dmg.find(b"<key>blkx</key>")
hfs = dmg.find(b"Apple_HFS", blkx)  # the partition the volume is read from
start = dmg.rfind(b"<dict>", blkx, hfs)
data = dmg.find(b"<data>", start) + len(b"<data>")
assert blkx > 0 and hfs > 0 and start > 0 and data < dmg.find(b"</dict>", hfs)
while dmg[data] in b" \t\r\n":
    data += 1
dmg[data + 10] = ord("*")
open(sys.argv[3] + "/bad-blkx.dmg", "wb").write(dmg)
