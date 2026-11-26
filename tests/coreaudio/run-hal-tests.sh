#!/usr/bin/env bash
# tests/coreaudio/run-hal-tests.sh <bin-dir> <arch> [test...]
# Runs the HAL property tests of this directory inside Darling, against the PulseAudio null devices of
# fixture-hal.sh, which are unloaded afterwards. <bin-dir> holds the programs as <test>_<arch>, built with
#   guest-cc.sh -a <arch> -o <bin-dir>/<test>_<arch> tests/coreaudio/<test>.c -framework CoreAudio -framework CoreFoundation
# The host's default sink and source are left as they were. Prints one line per test and exits non-zero if one failed.
set -u
BIN=${1:?usage: run-hal-tests.sh <bin-dir> <arch> [test...]}
ARCH=${2:?usage: run-hal-tests.sh <bin-dir> <arch> [test...]}
shift 2
TESTS=("$@")
[ ${#TESTS[@]} -gt 0 ] || TESTS=(test_device_uid test_hal_rate test_hal_transport test_hal_volume test_hal_properties)
HERE=$(cd "$(dirname "$0")" && pwd)
BIN=$(cd "$BIN" && pwd)

OLD_SINK=$(pactl get-default-sink)
OLD_SOURCE=$(pactl get-default-source)
cleanup() {
	"$HERE/fixture-hal.sh" down
	pactl set-default-sink "$OLD_SINK"
	pactl set-default-source "$OLD_SOURCE"
}
trap cleanup EXIT

"$HERE/fixture-hal.sh" down
"$HERE/fixture-hal.sh" up >/dev/null || exit 1
# the tests that look at the default devices find the fixture's, not the real ones
pactl set-default-sink darling-hal-48k
pactl set-default-source darling-hal-mic

rc=0
for t in "${TESTS[@]}"; do
	out=$(mktemp)
	timeout -s KILL 120 darling shell "/Volumes/SystemRoot$BIN/${t}_$ARCH" </dev/null > "$out" 2>&1
	status=$?
	[ -n "${VERBOSE:-}" ] && cat "$out"
	grep -h "^FAIL\|FAILED\|^test_.*: OK" "$out" | head -40
	if [ $status -ne 0 ]; then
		echo "$t ($ARCH): exit $status"
		grep -hv "^PASS" "$out" | tail -5
		rc=1
	else
		echo "$t ($ARCH): ok ($(grep -c '^PASS' "$out") checks)"
	fi
	rm -f "$out"
done
exit $rc
