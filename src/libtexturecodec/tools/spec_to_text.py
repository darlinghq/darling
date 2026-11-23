#!/usr/bin/env python3
"""Extract section 23 (ASTC) of the Khronos Data Format Specification HTML to
plain text, one table row per line. Usage: spec_to_text.py in.html out.txt"""
import html
import re
import sys


def row(m):
    cells = re.findall(r'<t[dh][^>]*>(.*?)</t[dh]>', m.group(0), re.S)
    cells = [re.sub(r'\s+', ' ', re.sub(r'<[^>]+>', '', c)).strip() for c in cells]
    return '\nROW: ' + ' | '.join(cells) + '\n'


def heading(s, title):
    for m in re.finditer(r'<h[1-6][^>]*>(.{0,300}?)</h[1-6]>', s, re.S):
        if html.unescape(re.sub(r'<[^>]+>', '', m.group(1))).replace('\xa0', ' ').strip().startswith(title):
            return m
    raise SystemExit('heading not found: ' + title)


def main(src, dst):
    s = open(src, encoding='utf-8').read()
    a = heading(s, '23. ASTC Compressed Texture Image Formats').start()
    last = heading(s, '23.25. HDR Profile Support').end()
    b = last + re.search(r'<h[12][^>]*>', s[last:]).start()
    seg = re.sub(r'<script.*?</script>', '', s[a:b], flags=re.S)
    seg = re.sub(r'<tr[^>]*>.*?</tr>', row, seg, flags=re.S)
    seg = re.sub(r'<br\s*/?>', '\n', seg)
    seg = re.sub(r'</(p|div|h\d|li|pre|table)>', '\n', seg)
    seg = html.unescape(re.sub(r'<[^>]+>', '', seg))
    open(dst, 'w', encoding='utf-8').write(re.sub(r'\n\s*\n+', '\n', seg))


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
