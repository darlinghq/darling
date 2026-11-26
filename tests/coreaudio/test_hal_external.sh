#!/usr/bin/env bash
# tests/coreaudio/test_hal_external.sh <bin-dir> <arch>
# Runs test_hal_external inside Darling and, once it is waiting, changes the volume and the mute of the fixture's
# sink and the volume of its source from the host with pactl. Needs test_hal_external_<arch> in <bin-dir>.
set -u
BIN=$(cd "${1:?usage: test_hal_external.sh <bin-dir> <arch>}" && pwd)
ARCH=${2:?usage: test_hal_external.sh <bin-dir> <arch>}
HERE=$(cd "$(dirname "$0")" && pwd)
TMP=$(mktemp -d)
cleanup() {
	"$HERE/fixture-hal.sh" down
	rm -rf "$TMP"
}
trap cleanup EXIT
"$HERE/fixture-hal.sh" down
"$HERE/fixture-hal.sh" up >/dev/null || exit 1

timeout -s KILL 90 darling shell "/Volumes/SystemRoot$BIN/test_hal_external_$ARCH" </dev/null > "$TMP/out.txt" 2>&1 &
TEST=$!
for i in $(seq 1 300); do
	grep -q READY "$TMP/out.txt" 2>/dev/null && break
	sleep 0.1
done
sleep 1
pactl set-sink-volume darling-hal-48k 30%
sleep 1
pactl set-sink-mute darling-hal-48k 1
sleep 1
pactl set-source-volume darling-hal-mic 60%
wait "$TEST"
status=$?
grep -h "^FAIL\|^test_.*: " "$TMP/out.txt"
darling shutdown >/dev/null 2>&1
exit $status
