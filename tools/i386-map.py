#!/usr/bin/env python

import fileinput
import subprocess
import sys, os

devnull = open(os.devnull, 'wb') 

for line in fileinput.input():
    line = line.rstrip()
    orig_line = line
    fixed = False

    while True:
        proc = subprocess.Popen("readelf -Ws /home/lubos/Projects/darling-root/lib32/darling/libstdcxx.so | grep " + line + "@@", shell=True, stdout=devnull, stderr=devnull)
        proc.wait()

        if proc.returncode != 0:
            pos = line.rfind('m')
            if pos == -1:
                break
            ll = list(line)
            ll[pos] = 'j'
            line = "".join(ll)
        else:
            fixed = True
            break

    if fixed:
        print orig_line + ' = ' + line + ';'
    else:
        sys.stderr.write('Cannot fix ' + orig_line + '\n')


