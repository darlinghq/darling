#!/bin/sh

set -e

for i in mach/*.bin; do
    echo "Running $i..."
    ./ld-mac ./$i
done

echo
echo '*** ALL TESTS PASS ***'
