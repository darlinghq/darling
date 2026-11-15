#!/usr/bin/env bash
# tests/openal/measure_tone.sh <guest openal_tone binary> [seconds]
#
# Runs tests/src/openal_tone.c in Darling while recording the output of a
# PulseAudio null sink, and checks the recording: a 440 Hz tone of the
# requested length (default 3 s) whose level rises linearly with no gap, as
# the test's ramped tone does.
#
# The head of a stream on a null sink's monitor can be missing (a property of
# the monitor, not of the player), so the check is on the shape of what was
# captured: constant frequency, steady linear rise and at least 90% of the
# length.
#
# Copyright (C) 2026 Lubos Dolezel
set -euo pipefail

BIN=${1:?usage: measure_tone.sh <binary> [seconds]}
SECS=${2:-3}
HERE=$(cd "$(dirname "$0")" && pwd)
WORK=$(mktemp -d "${TMPDIR:-/tmp}/openal-tone.XXXXXX")
OLD_SINK=$(pactl get-default-sink)
PAREC=

cleanup() {
	[ -n "$PAREC" ] && kill "$PAREC" 2>/dev/null || true
	pactl set-default-sink "$OLD_SINK" || true
	"$HERE/../coreaudio/fixture.sh" down || true
	rm -rf "$WORK"
}
trap cleanup EXIT

"$HERE/../coreaudio/fixture.sh" up >/dev/null
pactl set-default-sink darling-test-sink
timeout -s KILL $((SECS + 25)) parec -d darling-test-sink.monitor \
	--format=s16le --rate=44100 --channels=1 --raw > "$WORK/capture.raw" &
PAREC=$!
sleep 1

timeout -s KILL $((SECS + 40)) darling shell "/Volumes/SystemRoot$(readlink -f "$BIN")" "$SECS" \
	</dev/null > "$WORK/test.out" 2>&1 || true
sleep 1
kill "$PAREC" 2>/dev/null || true
PAREC=

cat "$WORK/test.out"
grep -q '^RESULT PASS' "$WORK/test.out" || { echo "FAIL: the test program reported failures"; exit 1; }

python3 -I - "$WORK/capture.raw" "$SECS" <<'PY'
import math, struct, sys
path, secs = sys.argv[1], float(sys.argv[2])
rate = 44100
raw = open(path, 'rb').read()
s = struct.unpack('<%dh' % (len(raw) // 2), raw[:len(raw) // 2 * 2])
win = rate // 10
levels, freqs = [], []
for i in range(0, len(s) - win, win):
    seg = s[i:i + win]
    levels.append(math.sqrt(sum(x * x for x in seg) / win))
    freqs.append(sum(1 for a, b in zip(seg, seg[1:]) if (a < 0) != (b < 0)) / 2 / (win / rate))
# the audible part: windows above the noise floor
audible = [i for i, l in enumerate(levels) if l > 50]
if not audible:
    print("FAIL: nothing was played (silent capture)"); sys.exit(1)
first, last = audible[0], audible[-1]
span = (last - first + 1) / 10.0
body = range(first + 1, last)          # the edge windows are partial
ok = True
if span < 0.9 * secs:
    print("FAIL: audible %.1f s of %.1f s" % (span, secs)); ok = False
if any(levels[i] <= 50 for i in range(first, last + 1)):
    print("FAIL: silent gap inside the tone"); ok = False
bad = [i for i in body if abs(freqs[i] - 440) > 10]
if bad:
    print("FAIL: frequency off 440 Hz in %d windows" % len(bad)); ok = False
steps = [levels[i + 1] - levels[i] for i in range(first + 1, last - 1)]
mean = sum(steps) / len(steps)
if mean <= 0 or any(abs(d - mean) > 0.35 * mean for d in steps):
    print("FAIL: level does not rise linearly"); ok = False
print("%s: %.1f s audible, 440 Hz tone, level rise per 0.1 s %.0f" % ("PASS" if ok else "FAIL", span, mean))
sys.exit(0 if ok else 1)
PY
