#!/usr/bin/env python3
"""Generate the reference vectors in tests/vectors with the astcenc release
binary used as a black box (compress synthetic images, decompress again).

Usage: gen_vectors.py ASTCENC OUTDIR SCRATCHDIR [committed|full]

"committed" (default) writes the small set kept in tests/vectors; "full"
writes every footprint x image x profile x preset combination (about 13 MB),
which is how the whole matrix was compared in scratch.

Each vector is NAME.astc (the compressed file as astcenc wrote it) plus
NAME.rgba (width*height*4 bytes of the decompression astcenc produced).
vectors.tsv lists the astcenc arguments of every vector; sources are in
OUTDIR/src as 32-bit TGA files.
"""
import random
import struct
import subprocess
import sys
import os

W, H = 30, 26
FOOTPRINTS = ['4x4', '5x4', '5x5', '6x5', '6x6', '8x5', '8x6', '10x5', '10x6',
              '8x8', '10x8', '10x10', '12x10', '12x12']


# (image, profile, preset) per footprint in the committed set, plus extras.
COMMITTED = [('blobs', 'lin', 'exh'), ('alpha', 'srgb', 'exh'),
             ('noise', 'lin', 'fast'), ('grayalpha', 'srgb', 'exh'),
             ('normal', 'lin', 'exh')]
COMMITTED_EXTRA = {'4x4': [('smooth', 'srgb', 'exh'), ('flat', 'lin', 'exh')],
                   '8x6': [('smooth', 'srgb', 'exh'), ('flat', 'lin', 'exh')],
                   '12x12': [('smooth', 'srgb', 'exh'), ('flat', 'lin', 'exh')]}


def selected(mode, fp, iname, prof, qname):
    if mode == 'full':
        return True
    return (iname, prof, qname) in COMMITTED + COMMITTED_EXTRA.get(fp, [])


def fnv1a64(data):
    h = 0xCBF29CE484222325
    for b in data:
        h = ((h ^ b) * 0x100000001B3) & 0xFFFFFFFFFFFFFFFF
    return h


def clamp(v):
    return max(0, min(255, int(v)))


def img_blobs(rng):
    seeds = [(rng.randrange(W), rng.randrange(H),
              [rng.randrange(256) for _ in range(3)]) for _ in range(6)]
    px = []
    for y in range(H):
        for x in range(W):
            d = min(seeds, key=lambda s: (s[0] - x) ** 2 + (s[1] - y) ** 2)
            px.append((*d[2], 255))
    return px


def img_alpha(rng):
    px = []
    for y in range(H):
        for x in range(W):
            a = 255 if (x // 5 + y // 4) % 2 else 40 + 3 * x
            px.append((clamp(x * 8), clamp(y * 9), clamp(120 + 2 * (x - y)), clamp(a)))
    return px


def img_gray_alpha(rng):
    px = []
    for y in range(H):
        for x in range(W):
            g = clamp(20 + 7 * x + rng.randrange(12))
            a = clamp(255 - 9 * y if (x + y) % 7 else 255)
            px.append((g, g, g, a))
    return px


def img_noise(rng):
    return [tuple(rng.randrange(256) for _ in range(4)) for _ in range(W * H)]


def img_smooth(rng):
    return [(clamp(100 + x + y // 2), clamp(110 + y), clamp(90 + x // 2), 255)
            for y in range(H) for x in range(W)]


def img_flat(rng):
    cols = [(10, 200, 30, 255), (255, 255, 255, 255), (0, 0, 0, 0), (90, 40, 160, 128)]
    return [cols[(x >= W // 2) + 2 * (y >= H // 2)] for y in range(H) for x in range(W)]


def img_normal(rng):
    px = []
    for y in range(H):
        for x in range(W):
            nx = (x - W / 2) / W
            ny = (y - H / 2) / H
            nz = max(0.1, 1 - nx * nx - ny * ny) ** 0.5
            px.append((clamp(128 + 127 * nx), clamp(128 + 127 * ny), clamp(128 + 127 * nz), 255))
    return px


IMAGES = {'blobs': img_blobs, 'alpha': img_alpha, 'grayalpha': img_gray_alpha,
          'noise': img_noise, 'smooth': img_smooth, 'flat': img_flat,
          'normal': img_normal}


def write_tga(path, px):
    hdr = struct.pack('<BBBHHBHHHHBB', 0, 0, 2, 0, 0, 0, 0, 0, W, H, 32, 8 | 0x20)
    with open(path, 'wb') as f:
        f.write(hdr)
        for r, g, b, a in px:
            f.write(bytes((b, g, r, a)))


def tga_pixels(d, off, w, h, bpp, rle):
    if not rle:
        return d[off:off + w * h * bpp]
    out = bytearray()
    while len(out) < w * h * bpp:
        c = d[off]
        off += 1
        n = (c & 0x7F) + 1
        if c & 0x80:
            out += d[off:off + bpp] * n
            off += bpp
        else:
            out += d[off:off + n * bpp]
            off += n * bpp
    return bytes(out)


def read_tga(path):
    d = open(path, 'rb').read()
    idlen, itype = d[0], d[2]
    w, h, depth, desc = struct.unpack('<HHBB', d[12:18])
    assert itype in (2, 10) and depth in (24, 32), (itype, depth)
    bpp = depth // 8
    data = tga_pixels(d, 18 + idlen, w, h, bpp, itype == 10)
    rows = [data[y * w * bpp:(y + 1) * w * bpp] for y in range(h)]
    if not desc & 0x20:
        rows.reverse()
    out = bytearray()
    for row in rows:
        for x in range(w):
            p = row[x * bpp:(x + 1) * bpp]
            out += bytes((p[2], p[1], p[0], p[3] if bpp == 4 else 255))
    return w, h, bytes(out)


def run(cmd):
    subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def main():
    astcenc, outdir, scratch = sys.argv[1:4]
    mode = sys.argv[4] if len(sys.argv) > 4 else 'committed'
    os.makedirs(outdir, exist_ok=True)
    os.makedirs(scratch, exist_ok=True)
    rows = []
    for iname, fn in IMAGES.items():
        src = os.path.join(outdir, 'src', iname + '.tga')
        os.makedirs(os.path.dirname(src), exist_ok=True)
        write_tga(src, fn(random.Random(iname)))
        for fp in FOOTPRINTS:
            for prof in ('l', 's'):
                for q, qname in (('-exhaustive', 'exh'), ('-fast', 'fast')):
                    name = f'{iname}_{fp}_{"srgb" if prof == "s" else "lin"}_{qname}'
                    if not selected(mode, fp, iname, 'srgb' if prof == 's' else 'lin', qname):
                        continue
                    astc = os.path.join(outdir, name + '.astc')
                    tga = os.path.join(scratch, name + '.tga')
                    enc = [astcenc, '-c' + prof, src, astc, fp, q, '-silent']
                    dec = [astcenc, '-d' + prof, astc, tga, '-silent']
                    run(enc)
                    run(dec)
                    w, h, rgba = read_tga(tga)
                    assert (w, h) == (W, H)
                    open(os.path.join(outdir, name + '.rgba'), 'wb').write(rgba)
                    shown_enc = [enc[1], iname + '.tga', name + '.astc'] + enc[4:]
                    shown_dec = [dec[1], name + '.astc', name + '.tga', '-silent']
                    rows.append((name, fp, W, H, 1 if prof == 's' else 0,
                                 ' '.join(shown_enc), ' '.join(shown_dec),
                                 '%016x' % fnv1a64(rgba)))
    with open(os.path.join(outdir, 'vectors.tsv'), 'w') as f:
        f.write('name\tfootprint\twidth\theight\tsrgb\tencode_args\tdecode_args\tfnv1a64\n')
        for r in rows:
            f.write('\t'.join(map(str, r)) + '\n')


if __name__ == '__main__':
    main()
