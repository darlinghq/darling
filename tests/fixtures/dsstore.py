#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
"""Writes .DS_Store files for Finder's tests (synthetic fixtures; nothing here is read back by Darling itself).

The format is undocumented by Apple. This writer follows public third-party descriptions of it: Wim Lewis's
DSStoreFormat notes (in the Mac::Finder::DSStore distribution), Mark Mentovai's description, and the documentation of
the Python ds_store and mac_alias packages. It was written independently, in our own words; no code was copied.

The file, as those describe it:
- 4 bytes 00 00 00 01, then a "buddy allocator" area. Every offset below counts from the start of that area (file
  offset 4).
- The area starts with a 32-byte header: "Bud1", the offset of the allocator's bookkeeping block, its size, the
  offset again, and 16 more bytes (zero here).
- The bookkeeping block holds the block address table (a count, a zero word, then the addresses, the table padded to a
  multiple of 256 entries), a directory of named blocks (a count, then a length byte, the name and a block number for
  each; "DSDB" names the record tree's header block), and 32 free lists (a count and offsets each; empty here).
- A block address keeps log2 of the block's size in its low 5 bits; the rest is the offset. A block is aligned to its
  size.
- The DSDB block: the root node's block number, the number of internal levels, the record count, the node count and
  the page size (0x1000).
- A node: a word P and a count. P = 0 is a leaf, holding count records. Otherwise it is an internal node holding count
  pairs (child block number, record), with P its rightmost child; a record sits between the subtrees before and after
  it.
- A record: the file name (a UTF-16 code unit count, then UTF-16BE), a four-character code saying what the record is
  about ('Iloc', 'bwsp', ...), a four-character data type, and the data:
  'bool' 1 byte; 'long' 4 bytes; 'shor' 4 bytes (a 16-bit value); 'type' 4 bytes (a four-character code);
  'comp' and 'dutc' 8 bytes; 'blob' a 4-byte length and the bytes; 'ustr' a UTF-16 code unit count and UTF-16BE.
- Records are kept sorted by file name, compared case-insensitively, then by code.
"""
import plistlib
import struct

PAGE = 0x1000


class Raw:
    """A record given as raw bytes: written as is, to make malformed stores."""

    def __init__(self, data, sort_name="", code=b"zzzz"):
        self.data = data
        self.sort_name = sort_name
        self.code = code


def _fourcc(code):
    return code if isinstance(code, bytes) else code.encode("latin-1")


def _utf16(name):
    data = name.encode("utf-16-be")
    return struct.pack(">I", len(data) // 2) + data


def encode_record(record):
    if isinstance(record, Raw):
        return record.data
    name, code, kind, value = record
    out = _utf16(name) + _fourcc(code) + _fourcc(kind)
    kind = _fourcc(kind)
    if kind == b"bool":
        out += struct.pack(">B", 1 if value else 0)
    elif kind in (b"long", b"shor"):
        out += struct.pack(">I", value)
    elif kind == b"type":
        out += _fourcc(value)
    elif kind in (b"comp", b"dutc"):
        out += struct.pack(">Q", value)
    elif kind == b"blob":
        out += struct.pack(">I", len(value)) + value
    elif kind == b"ustr":
        out += _utf16(value)
    else:
        raise ValueError("unknown type %r" % kind)
    return out


def _sort_key(record):
    if isinstance(record, Raw):
        return (record.sort_name.encode("utf-16-be"), record.code)
    name, code, _, _ = record
    # case-insensitive, compared as UTF-16 code units
    return (name.lower().encode("utf-16-be"), _fourcc(code))


def sort_records(records):
    return sorted(records, key=_sort_key)


def _leaf(records):
    return struct.pack(">II", 0, len(records)) + b"".join(encode_record(r) for r in records)


def write_store(records, corrupt=None):
    """The .DS_Store bytes holding records (sorted as Finder keeps them). corrupt makes a malformed store:
    "cycle" (the root points at itself), "block-out-of-range" (the root's address is past the end of the file),
    "huge-count" (the first leaf claims 0xffffffff records), "bad-magic" ("Bud2")."""
    records = sort_records(records)
    encoded = [encode_record(r) for r in records]

    # Leaves of whole records, each under a page; one record between neighbouring leaves goes up to the root
    leaves, separators, current, size = [], [], [], 8
    for record, data in zip(records, encoded):
        if current and size + len(data) > PAGE:
            leaves.append(current)
            current, size = [], 8
            separators.append(record)
            continue
        current.append(record)
        size += len(data)
    leaves.append(current)

    # Blocks: 0 the bookkeeping, 1 DSDB, 2 the root, then the leaves (if the root is internal)
    nodes = []
    if len(leaves) == 1:
        nodes.append(_leaf(leaves[0]))
        levels = 0
    else:
        first_leaf = 3
        root = b""
        for i, separator in enumerate(separators):
            root += struct.pack(">I", first_leaf + i) + encode_record(separator)
        root = struct.pack(">II", first_leaf + len(leaves) - 1, len(separators)) + root
        assert len(root) <= PAGE, "too many leaves for one internal node"
        nodes.append(root)
        nodes.extend(_leaf(leaf) for leaf in leaves)
        levels = 1
    if corrupt == "huge-count":
        target = 0 if levels == 0 else 1
        nodes[target] = nodes[target][:4] + struct.pack(">I", 0xFFFFFFFF) + nodes[target][8:]
    if corrupt == "cycle":
        assert levels == 1, "cycle needs an internal root"
        nodes[0] = struct.pack(">I", 2) + nodes[0][4:]

    count = 2 + len(nodes)
    bookkeeping_size = 2048
    addresses = [(2048, 11), (32, 5)]  # (offset, log2 size)
    offset = 4096
    for _ in nodes:
        addresses.append((offset, 12))
        offset += PAGE
    if corrupt == "block-out-of-range":
        addresses[2] = (offset + 16 * PAGE, 12)

    dsdb = struct.pack(">IIIII", 2, levels, len(records), len(nodes), PAGE)

    table = struct.pack(">II", count, 0)
    for off, log in addresses:
        table += struct.pack(">I", off | log)
    padded = (count + 255) // 256 * 256
    table += b"\0" * 4 * (padded - count)
    table += struct.pack(">I", 1) + struct.pack(">B", 4) + b"DSDB" + struct.pack(">I", 1)
    table += struct.pack(">I", 0) * 32
    assert len(table) <= bookkeeping_size

    area = bytearray(offset)
    magic = b"Bud2" if corrupt == "bad-magic" else b"Bud1"
    area[0:32] = magic + struct.pack(">III", 2048, bookkeeping_size, 2048) + b"\0" * 16
    area[32:32 + len(dsdb)] = dsdb
    area[2048:2048 + len(table)] = table
    for i, node in enumerate(nodes):
        at = 4096 + i * PAGE
        area[at:at + len(node)] = node
    return b"\0\0\0\1" + bytes(area)


def iloc(x, y):
    """An 'Iloc' blob: the icon's centre (x, y), top-left origin, then 6 bytes ff and 2 bytes 0."""
    return struct.pack(">II", x, y) + b"\xff" * 6 + b"\0\0"


def bwsp(settings):
    """A 'bwsp' blob (window settings: WindowBounds, ShowToolbar, ShowSidebar, ...): a binary property list."""
    return plistlib.dumps(settings, fmt=plistlib.FMT_BINARY)


def icvp(settings):
    """An 'icvp' blob (icon view settings: iconSize, textSize, backgroundType, backgroundImageAlias, ...): a binary
    property list."""
    return plistlib.dumps(settings, fmt=plistlib.FMT_BINARY)


def _pascal(text, size):
    data = text.encode("utf-8")[:size - 1]
    return struct.pack(">B", len(data)) + data + b"\0" * (size - 1 - len(data))


def alias_v2(volume_name, volume_relative_path, kind=0):
    """A version 2 alias record for a file (kind 0) or folder (1), as the mac_alias documentation describes it:
    a 150-byte fixed part (creator, record length, version 2, kind, the volume name as a 28-byte Pascal string, the
    volume date and file system, the parent folder id, the file name as a 64-byte Pascal string, and file details),
    then tagged items (a 2-byte tag, a 2-byte length, the data padded to an even length) ending with tag 0xffff:
    tag 2 the Carbon path ("Volume:folder:file"), tag 18 the POSIX path inside the volume, tag 19 the volume's mount
    point."""
    name = volume_relative_path.rstrip("/").split("/")[-1]
    fixed = struct.pack(">IHHH", 0, 0, 2, kind)
    fixed += _pascal(volume_name, 28)
    fixed += struct.pack(">I2sHI", 0, b"H+", 5, 0)
    fixed += _pascal(name, 64)
    fixed += struct.pack(">IIIIhhIH", 0, 0, 0, 0, -1, -1, 0, 0)
    fixed += b"\0" * 10
    assert len(fixed) == 150

    def item(tag, data):
        out = struct.pack(">HH", tag, len(data)) + data
        return out + (b"\0" if len(data) % 2 else b"")

    carbon = volume_name + volume_relative_path.replace("/", ":")
    items = item(2, carbon.encode("utf-8"))
    items += item(18, volume_relative_path.encode("utf-8"))
    items += item(19, ("/Volumes/" + volume_name).encode("utf-8"))
    items += struct.pack(">HH", 0xFFFF, 0)
    record = fixed + items
    return record[:4] + struct.pack(">H", len(record)) + record[6:]


def bookmark(volume_path, path):
    """Bookmark data, as the mac_alias documentation describes it: a 48-byte header ("book", the total length and
    0x10040000, little endian, then the header length 48), then the data area, which starts with the offset of the
    table of contents. Items are a length and a type (both 32-bit little endian) and the data padded to 4 bytes;
    0x0101 is a UTF-8 string and 0x0601 an array of item offsets. The table of contents (its length, 0xfffffffe,
    the identifier 1, the next table 0, the entry count, then key, offset and a reserved word per entry; offsets
    count from the data area) here has 0x1004 (the path, as an array of component strings) and 0x2002 (the volume's
    mount point, a string)."""
    area = bytearray(struct.pack("<I", 0))

    def add(kind, data):
        at = len(area)
        area.extend(struct.pack("<II", len(data), kind) + data)
        area.extend(b"\0" * (-len(data) % 4))
        return at

    components = [add(0x0101, c.encode("utf-8")) for c in path.strip("/").split("/")]
    path_item = add(0x0601, b"".join(struct.pack("<I", c) for c in components))
    volume_item = add(0x0101, volume_path.encode("utf-8"))
    entries = [(0x1004, path_item), (0x2002, volume_item)]
    toc = len(area)
    area.extend(struct.pack("<IIIII", 8 + 12 * len(entries), 0xFFFFFFFE, 1, 0, len(entries)))
    for key, at in entries:
        area.extend(struct.pack("<III", key, at, 0))
    area[0:4] = struct.pack("<I", toc)
    return b"book" + struct.pack("<III", 48 + len(area), 0x10040000, 48) + b"\0" * 32 + bytes(area)
