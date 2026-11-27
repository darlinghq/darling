#!/usr/bin/env python3
# Checks the "DH ..." lines printed by tests/src/commoncrypto_dh_modexp_pkcs7.c:
# each gives the prime p, the peer's private value b, the tested side's public
# key y and the secret z it computed against the peer key 2^b mod p. Since
# z = (2^b)^x = (2^x)^b = y^b mod p, Python's pow() recomputes it without the
# tested side's private value. CCDHComputeKey returns z without leading zeros.
#
# usage: tests/commoncrypto-dh-check.py <test output>...
import re
import sys

checked = failed = 0
for path in sys.argv[1:]:
    for line in open(path, errors='replace'):
        m = re.match(r'DH (\S+) p=([0-9a-f]+) b=([0-9a-f]+) y=([0-9a-f]+) z=([0-9a-f]*)$', line.strip())
        if not m:
            continue
        name, p, b, y, z = m.group(1), *(int(m.group(i) or '0', 16) for i in range(2, 6))
        zhex = m.group(5)
        expected = pow(y, b, p)
        ok = (2 <= y <= p - 2 and z == expected and len(zhex) == 2 * ((expected.bit_length() + 7) // 8))
        checked += 1
        if not ok:
            failed += 1
        print('%s %s: %s' % (path, name, 'ok' if ok else 'MISMATCH'))
print('%d checked, %d failed' % (checked, failed))
sys.exit(1 if failed or not checked else 0)
