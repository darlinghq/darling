#!/bin/bash
# Copyright (C) 2026 Lubos Dolezel
#
# Presses Return every two seconds on the private KWin of session.sh, to accept the portal's consent dialogs.
# Refuses to run unless the compositor is the private one: it checks that it was started with --no-lockscreen
# (so no password prompt can take the keys), that it has no lock screen child, and that WAYLAND_DISPLAY is its socket.
# kwin-keys is built from the cocotron tests (see the header of kwin-keys.c).
#   keys.sh <dir> <first-delay-seconds> <count> <kwin-keys binary>
set -eu
dir=$(realpath "$1"); kwin=$(sed -n 5p "$dir/pids")
ps -o args= -p "$kwin" | grep -q -- --no-lockscreen || { echo "not a --no-lockscreen KWin" >&2; exit 1; }
ps -eo ppid,args | awk -v k="$kwin" '$1 == k && /kscreenlocker/' | grep -q . && { echo "lock screen child" >&2; exit 1; }
export WAYLAND_DISPLAY=$dir/pwt-wl
sleep "$2"
for i in $(seq "$3"); do "$4" 100 28:1 28:0; sleep 2; done
