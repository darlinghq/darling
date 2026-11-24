#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
#
# One-off developer tool. Reads the public OpenGL.framework headers of a macOS
# SDK (gliDispatch.h and CGLMacro.h) and writes the machine-readable lists that
# generate.py works from:
#   gli-dispatch.tsv  one row per dispatch slot, in the slot order of the public
#                     struct: slot name, return type, parameter list (without the
#                     leading GLIContext), separated by " | "
#   gl-macros.tsv     one row per gl* name that CGL_MACRO maps: kind (F = function
#                     macro, A = alias of another gl* name), name, slot (F) or
#                     target name (A), parameter names, result cast (or empty)
# Only facts are kept (names, order, types), none of the headers' text.
# Usage: extract-gli-dispatch.py <OpenGL.framework Headers dir> <output dir>
import re, sys, os

hdr, out = sys.argv[1], sys.argv[2]
g = open(os.path.join(hdr, 'gliDispatch.h')).read()
g = re.sub(r'/\*.*?\*/', '', g, flags=re.S)
g = g[g.index('typedef struct __GLIFunctionDispatchRec'):]
g = g[g.index('{') + 1:]
slots = re.findall(r'^\s*([^;(]*?)\(\*(\w+)\)\s*\((.*?)\);', g, re.M | re.S)
with open(os.path.join(out, 'gli-dispatch.tsv'), 'w') as f:
    for ret, name, params in slots:
        plist = [p.strip() for p in params.replace('\n', ' ').split(',')]
        assert plist[0].startswith('GLIContext'), (name, plist)
        rest = ' | '.join(' '.join(p.split()) for p in plist[1:])
        f.write('%s\t%s\t%s\n' % (name, ' '.join(ret.split()), rest))

m = open(os.path.join(hdr, 'CGLMacro.h')).read()
m = re.sub(r'\\\n\s*', ' ', m)
rows = []
fpat = re.compile(r'(?:\((\w+)\)\(\(unsigned long\))?\(\*\(CGL_MACRO_CONTEXT\)->disp\.(\w+)\)\(CGL_MACRO_CONTEXT_RENDERER(?:,\s*(.*?))?\)\)?\s*$')
for name, params, body in re.findall(r'^#define (gl\w+)(\([^)]*\))?\s+(.*)$', m, re.M):
    if params:
        mm = fpat.fullmatch(body.strip())
        assert mm, (name, body)
        cast = mm.group(1) or ''
        names = [p.strip() for p in params[1:-1].split(',') if p.strip()]
        rows.append(('F', name, mm.group(2), ','.join(names), cast))
    else:
        rows.append(('A', name, body.strip(), '', ''))
with open(os.path.join(out, 'gl-macros.tsv'), 'w') as f:
    for r in rows:
        f.write('\t'.join(r) + '\n')
print(len(slots), 'slots,', len(rows), 'macros')
