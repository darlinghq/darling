#!/usr/bin/env bash
# tests/coreaudio/test_hal_rate_follow.sh <bin-dir> <arch>
# Plays a 1000 Hz tone of 2 s from an AudioQueue made at 22050, 44100 and 48000 Hz to each of the fixture's sinks
# (which run at 48000, 44100 and 32000 Hz), records what each sink is sent, and checks that the tone is still 1000 Hz
# and is not cut short. The start of a stream is missing from the monitor of a null sink, so up to 1 s may be lost.
# The rates of the queue and the device may differ, the pitch may not.
# The host's default sink is put back afterwards. Needs test_hal_rate_follow_<arch> in <bin-dir>.
set -u
BIN=$(cd "${1:?usage: test_hal_rate_follow.sh <bin-dir> <arch>}" && pwd)
ARCH=${2:?usage: test_hal_rate_follow.sh <bin-dir> <arch>}
HERE=$(cd "$(dirname "$0")" && pwd)
OLD_SINK=$(pactl get-default-sink)
TMP=$(mktemp -d)
PAREC=
cleanup() {
	[ -n "$PAREC" ] && kill "$PAREC" 2>/dev/null
	"$HERE/fixture-hal.sh" down
	pactl set-default-sink "$OLD_SINK"
	rm -rf "$TMP"
}
trap cleanup EXIT
"$HERE/fixture-hal.sh" down
"$HERE/fixture-hal.sh" up >/dev/null || exit 1

rc=0
for sink in darling-hal-48k:48000 darling-hal-44k:44100 darling-hal-51:32000; do
	name=${sink%:*}; sinkrate=${sink#*:}
	pactl set-default-sink "$name"
	for qrate in 22050 44100 48000; do
		parec -d "$name.monitor" --format=s16le --rate="$sinkrate" --channels=1 --raw > "$TMP/cap.raw" &
		PAREC=$!
		sleep 1
		timeout -s KILL 60 darling shell "/Volumes/SystemRoot$BIN/test_hal_rate_follow_$ARCH" "$qrate" 2 </dev/null > "$TMP/out.txt" 2>&1
		sleep 1
		kill "$PAREC" 2>/dev/null; wait "$PAREC" 2>/dev/null; PAREC=
		darling shutdown >/dev/null 2>&1
		result=$(python3 -I - "$TMP/cap.raw" "$sinkrate" <<'PY'
import struct, sys
raw = open(sys.argv[1], 'rb').read()
rate = int(sys.argv[2])
s = struct.unpack('<%dh' % (len(raw) // 2), raw[:len(raw) // 2 * 2])
loud = [i for i, v in enumerate(s) if abs(v) > 3000]
if len(loud) < rate // 4:
    print('silent 0 0'); sys.exit()
a, b = loud[0], loud[-1]
# frequency: rising zero crossings of the loud part
seg = s[a:b]
n = sum(1 for i in range(1, len(seg)) if seg[i - 1] < 0 <= seg[i])
dur = (b - a) / rate
print('%.1f %.2f %d' % (n / dur, dur, rate))
PY
)
		devrate=$(grep DEVICE_RATE "$TMP/out.txt" | head -1)
		set -- $result
		freq=$1; dur=$2
		ok=$(python3 -I -c "import sys; f,d=float(sys.argv[1]) if sys.argv[1]!='silent' else 0,float(sys.argv[2]); print('ok' if abs(f-1000)<20 and 1.0<d<2.6 else 'BAD')" "$freq" "$dur")
		echo "$name ($devrate) queue ${qrate} Hz: tone $freq Hz for $dur s: $ok"
		[ "$ok" = ok ] || rc=1
	done
done
exit $rc
