#!/bin/bash
# Copyright (C) 2026 Lubos Dolezel
#
# Runs the guest test carbon_windows in its interactive form on X11 and plays the user: a click in the
# content of the test's window, then the window manager's request to close it (Alt+F4).
#
#   carbon_windows_x11.sh <host path of the test program> <display, for example :93>
#
# The display must have a window manager (xfwm4) and xdotool must be installed; the script starts neither.
set -u
test=$1
display=${2:-:93}
out=$(mktemp)
trap 'rm -f "$out"' EXIT

timeout -s KILL 120 darling shell env DISPLAY=$display "/Volumes/SystemRoot$test" interact </dev/null > "$out" 2>&1 &
guest=$!

wait_for() {
	for i in $(seq 300); do
		grep -q "$1" "$out" && return 0
		sleep 0.2
	done
	return 1
}

if wait_for '^READY '; then
	read -r _ left top right bottom < <(grep '^READY ' "$out" | tail -1)
	x=$((left + 30)); y=$((top + 40))
	DISPLAY=$display xdotool mousemove "$x" "$y"
	sleep 0.3
	DISPLAY=$display xdotool click 1
	echo "clicked at $x $y"
	if wait_for '^READY_CLOSE'; then
		sleep 0.5
		DISPLAY=$display xdotool key alt+F4
	fi
fi
wait "$guest"
echo "guest rc=$?"
cat "$out"
grep -q 'failures=0' "$out" && echo "PASS carbon_windows_x11" || echo "FAIL carbon_windows_x11"
