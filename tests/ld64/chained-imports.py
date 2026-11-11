#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
#
# Prints the chained-fixup import table of a thin 64-bit Mach-O file, one
# "name lib_ordinal weak" line per import. lib_ordinal is printed signed
# (-2 is BIND_SPECIAL_DYLIB_FLAT_LOOKUP).
import struct
import sys

LC_DYLD_CHAINED_FIXUPS = 0x80000034

data = open(sys.argv[1], 'rb').read()
magic, cputype, cpusub, ftype, ncmds, sizeofcmds, flags, _ = struct.unpack_from('<8I', data, 0)
if magic != 0xfeedfacf:
    sys.exit('not a thin 64-bit Mach-O file')
off = 32
fixups = None
for _ in range(ncmds):
    cmd, size = struct.unpack_from('<2I', data, off)
    if cmd == LC_DYLD_CHAINED_FIXUPS:
        fixups = struct.unpack_from('<2I', data, off + 8)
    off += size
if fixups is None:
    sys.exit('no LC_DYLD_CHAINED_FIXUPS')
base = fixups[0]
version, starts, imports, symbols, count, ifmt, sfmt = struct.unpack_from('<7I', data, base)
sizes = {1: 4, 2: 8, 3: 16}
for i in range(count):
    p = base + imports + i * sizes[ifmt]
    if ifmt == 1:
        v, = struct.unpack_from('<I', data, p)
        ordinal, weak, name = v & 0xff, (v >> 8) & 1, v >> 9
        if ordinal > 0xf0:
            ordinal -= 0x100
    elif ifmt == 2:
        v, _ = struct.unpack_from('<Ii', data, p)
        ordinal, weak, name = v & 0xff, (v >> 8) & 1, v >> 9
        if ordinal > 0xf0:
            ordinal -= 0x100
    else:
        v, _ = struct.unpack_from('<QQ', data, p)
        ordinal, weak, name = v & 0xffff, (v >> 16) & 1, v >> 32
        if ordinal > 0xfff0:
            ordinal -= 0x10000
    s = base + symbols + name
    print(data[s:data.index(b'\0', s)].decode(), ordinal, weak)
