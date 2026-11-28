#!/bin/sh
# Runs the bundle directly (the process is started by this shell) and through LaunchServices (open), then prints both.
cd "$(dirname "$0")" || exit 1
echo "PROBE item-e.run direct exec of the bundle executable -> (see lines below)"
./probe-e.app/Contents/MacOS/probe-e 2>&1 | grep '^PROBE'
OUT=$(mktemp)
echo "PROBE item-e.run launched through LaunchServices (open -W -n) -> (see lines below)"
open -W -n --stdout "$OUT" --stderr "$OUT" ./probe-e.app
grep '^PROBE' "$OUT"
rm -f "$OUT"
