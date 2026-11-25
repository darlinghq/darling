#!/usr/bin/env python3
"""Check that no two of the given .debs ship the same file, unless one conflicts with the other.

dpkg refuses to unpack a package whose files belong to another installed package, so two
packages that may be installed together must not share files. Directories may be shared. A
conflict limited to some versions doesn't count. Needs dpkg-deb.

usage: tests/deb-file-overlaps.py <deb>...
"""
import re
import subprocess
import sys
import tarfile


def field(deb, name):
    return subprocess.run(['dpkg-deb', '-f', deb, name], check=True,
                          text=True, stdout=subprocess.PIPE).stdout.strip()


def unversioned(relations):
    # "a, b (<< 2), c:any" -> {a, c}
    return {re.sub(r':.*', '', r.strip()) for r in relations.split(',') if r.strip() and '(' not in r}


def files(deb):
    with subprocess.Popen(['dpkg-deb', '--fsys-tarfile', deb], stdout=subprocess.PIPE) as p:
        with tarfile.open(fileobj=p.stdout, mode='r|') as tar:
            paths = {'/' + re.sub(r'^\./', '', m.name) for m in tar if not m.isdir()}
    if p.returncode != 0:
        sys.exit('dpkg-deb failed on ' + deb)
    return paths


def main(debs):
    owners = {}
    conflicts = {}
    for deb in debs:
        package = field(deb, 'Package')
        conflicts[package] = unversioned(field(deb, 'Conflicts'))
        for path in files(deb):
            owners.setdefault(path, []).append(package)

    failed = False
    for path, packages in sorted(owners.items()):
        if any(b not in conflicts[a] and a not in conflicts[b]
               for i, a in enumerate(packages) for b in packages[i + 1:]):
            print('%s: %s' % (path, ' '.join(sorted(packages))))
            failed = True
    return 1 if failed else 0


if __name__ == '__main__':
    if len(sys.argv) < 2:
        sys.exit(__doc__.strip().splitlines()[-1])
    sys.exit(main(sys.argv[1:]))
