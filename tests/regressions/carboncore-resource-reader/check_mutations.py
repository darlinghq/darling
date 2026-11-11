#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Reader regression checks, adapted from fr4nck/d4rling 8030c9d.

All mutations must compile, then fail the executable's checks.
No production source is edited: variants exist only in a temporary directory.
"""
from pathlib import Path
import os
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[3]
SOURCE = ROOT / 'src/frameworks/CoreServices/src/CarbonCore/ResourcesImpl.cpp'
RUNNER = Path(__file__).with_name('run.sh')
MUTANTS = (
    ('read type count from fixed header position',
     'const size_t numTypesField = be16(map + typeOffset);',
     'const size_t numTypesField = be16(map + 28);'),
    ('truncate high-bit Pascal names',
     'const size_t nameLength = map[namePos];',
     'const size_t nameLength = map[namePos] & 0x7f;'),
    ('read the 24-bit data offset from the attributes byte',
     'const size_t resourceOffset = be24(reference + 5);',
     'const size_t resourceOffset = be24(reference + 4);'),
    ('index the moved-from resource name',
     'typeData.nameMap.emplace(entry->name, entry);',
     'typeData.nameMap.emplace(res.name, entry);'),
    ('bound payload by file instead of data section',
     'inRange(resourceOffset + 4, blockLength, dataLength)',
     'inRange(resourceOffset + 4, blockLength, length)'),
)

def main():
    compiler = sys.argv[1] if len(sys.argv) > 1 else 'g++'
    source = SOURCE.read_text(encoding='utf8')
    with tempfile.TemporaryDirectory(prefix='darling-resource-reader-mutants-') as temp:
        for index, (name, old, new) in enumerate(MUTANTS, 1):
            if source.count(old) != 1:
                raise RuntimeError('ambiguous mutation: ' + name)
            target = Path(temp) / f'mutant-{index}.cpp'
            target.write_text(source.replace(old, new, 1), encoding='utf8')
            env = dict(os.environ, DARLING_RESOURCE_READER_SOURCE=str(target))
            result = subprocess.run(
                ['bash', str(RUNNER), compiler], env=env, text=True,
                stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=60,
                check=False)
            if result.returncode != 1 or 'FAIL' not in result.stdout + result.stderr:
                raise RuntimeError(
                    f'mutation did not fail runtime checks: {name}\n'
                    f'rc={result.returncode}\n{result.stdout[-500:]}\n{result.stderr[-1200:]}')
            print(f'READER_MUTANT_{index}_KILLED: {name}')
        print(f'READER_MUTANTS: {len(MUTANTS)}/{len(MUTANTS)} killed ({compiler})')

if __name__ == '__main__':
    try:
        main()
    except (OSError, RuntimeError, subprocess.TimeoutExpired) as error:
        print('FAIL: ' + str(error), file=sys.stderr)
        sys.exit(2)
