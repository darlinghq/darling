#!/usr/bin/python

from __future__ import print_function

import argparse
import subprocess
import re
import sys

from macholib.MachO import MachO
from macholib.SymbolTable import SymbolTable
from macholib.ptypes import *
from macholib import mach_o
import macholib

class build_version_command(Structure):
    _fields_ = (
        ('platform', p_uint32),
        ('minos', p_uint32),
        ('sdk', p_uint32),
        ('ntools', p_uint32))

if 0x32 not in mach_o.LC_REGISTRY:
    mach_o.LC_REGISTRY[0x32] = build_version_command

parser = argparse.ArgumentParser()
parser.add_argument("dylib")
args = parser.parse_args()

macho = MachO(args.dylib)

syms = None

for header_index, header in enumerate(macho.headers):

    try:
        symtab = SymbolTable(macho, header=header)
    except TypeError:
        if syms is None:
            # There's a bug in older versions of macholib, fall back on nm
            print("warning: buggy macholib, falling back on nm", file=sys.stderr)
            proc = subprocess.Popen(['nm', '-arch', 'all', args.dylib], stdout=subprocess.PIPE)
            (out, err) = proc.communicate()
            if proc.wait() != 0:
                raise Exception("nm failed")
            syms = list()
            for line in out.splitlines():
                m = re.match(r'([0-9a-fA-F]+)\s+a\s+page_max_size\s*$', line)
                if m:
                    syms.append(int(m.group(1), 16))
        if len(syms) == 0:
            raise Exception("didn't find symbol named 'page_max_size'")
        if len(syms) != len(macho.headers):
            raise Exception("didn't find one 'page_max_size' symbol per arch")
        PAGE_MAX_SIZE = syms[header_index]
    else:
        for nlist, name in symtab.nlists:
            if name == 'page_max_size':
                PAGE_MAX_SIZE = nlist.n_value
                break
        else:
            raise Exception("didn't find symbol named 'page_max_size'")

    for lc, cmd, data in header.commands:
        if lc.cmd == mach_o.LC_SEGMENT_64:
            if cmd.segname.rstrip('\x00') == "__TEXT":
                if cmd.vmsize != 2 * PAGE_MAX_SIZE:
                    raise Exception("__TEXT segment size is wrong")
                if len(data) != 1:
                    raise Exception("__TEXT segment has more than one section")
                sect = data[0]
                if sect.addr != PAGE_MAX_SIZE:
                    raise Exception("__text section has wrong address")
                if sect.size != PAGE_MAX_SIZE:
                    raise Exception("__text section has wrong size")
                break
    else:
        raise Exception("no __TEXT segment!")

