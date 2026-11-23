#!/usr/bin/env python3
"""Differential vectors from random 128-bit blocks, decoded by astcenc as a
black box. Random blocks are mostly illegal, so a share of them gets a random
but fully structured configuration (random block mode, partition count and
seed, CEM selector), which reaches legal blocks of every kind.

Usage: gen_random_vectors.py ASTCENC OUTDIR SCRATCHDIR BLOCKS_PER_VECTOR [keep-rgba] [SEED]
Writes OUTDIR/rand_<footprint>_<lin|srgb>.astc and vectors.tsv with the FNV-1a
hash of astcenc's RGBA decode; with keep-rgba also the .rgba files (large).
"""
import os
import random
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from gen_vectors import FOOTPRINTS, fnv1a64, read_tga  # noqa: E402

COLS = 25


def put(v, pos, n, val):
    mask = ((1 << n) - 1) << pos
    return (v & ~mask) | ((val << pos) & mask)


def structured(rng):
    v = rng.getrandbits(128)
    v = put(v, 0, 11, rng.randrange(2048))
    v = put(v, 11, 2, rng.randrange(4))
    v = put(v, 23, 2, rng.choice((0, 1, 2, 3)))
    return v


def void_extent(rng):
    v = rng.getrandbits(128)
    v = put(v, 0, 9, 0x1FC)
    kind = rng.random()
    if kind < 0.8:
        v = put(v, 10, 2, 3)
    if kind < 0.4:
        v = put(v, 9, 1, 0)
    if rng.random() < 0.3:
        for i in range(4):
            v = put(v, 12 + 13 * i, 13, 0x1FFF)
    elif rng.random() < 0.7:
        for i in range(2):
            lo = rng.randrange(0x1FFE)
            v = put(v, 12 + 26 * i, 13, lo)
            v = put(v, 25 + 26 * i, 13, rng.randrange(lo + 1, 0x2000))
    return v


def blocks(rng, n):
    out = bytearray()
    for _ in range(n):
        r = rng.random()
        if r < 0.1:
            v = void_extent(rng)
        elif r < 0.8:
            v = structured(rng)
        else:
            v = rng.getrandbits(128)
        out += v.to_bytes(16, 'little')
    return bytes(out)


def header(bw, bh, w, h):
    return (bytes((0x13, 0xAB, 0xA1, 0x5C, bw, bh, 1)) + w.to_bytes(3, 'little') +
            h.to_bytes(3, 'little') + (1).to_bytes(3, 'little'))


def main():
    astcenc, outdir, scratch, n = sys.argv[1:5]
    n = int(n)
    keep = 'keep-rgba' in sys.argv[5:]
    seed = ''.join(a for a in sys.argv[5:] if a != 'keep-rgba')
    fps = FOOTPRINTS
    os.makedirs(outdir, exist_ok=True)
    os.makedirs(scratch, exist_ok=True)
    rows = []
    for fp in fps:
        bw, bh = map(int, fp.split('x'))
        nrows = (n + COLS - 1) // COLS
        for prof in ('l', 's'):
            name = f'rand_{fp}_{"srgb" if prof == "s" else "lin"}'
            rng = random.Random(name + seed)
            data = blocks(rng, COLS * nrows)
            w, h = COLS * bw, nrows * bh
            astc = os.path.join(outdir, name + '.astc')
            open(astc, 'wb').write(header(bw, bh, w, h) + data)
            tga = os.path.join(scratch, name + '.tga')
            subprocess.run([astcenc, '-d' + prof, astc, tga, '-silent'], check=True,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            rw, rh, rgba = read_tga(tga)
            assert (rw, rh) == (w, h)
            if keep:
                open(os.path.join(outdir, name + '.rgba'), 'wb').write(rgba)
            rows.append((name, fp, w, h, 1 if prof == 's' else 0, 'random-blocks',
                         '-d' + prof, '%016x' % fnv1a64(rgba)))
    with open(os.path.join(outdir, 'vectors.tsv'), 'w') as f:
        f.write('name\tfootprint\twidth\theight\tsrgb\tencode_args\tdecode_args\tfnv1a64\n')
        for r in rows:
            f.write('\t'.join(map(str, r)) + '\n')


if __name__ == '__main__':
    main()
