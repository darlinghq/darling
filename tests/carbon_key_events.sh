#!/bin/bash
# Copyright (C) 2026 Lubos Dolezel
#
# Runs the guest test carbon_key_events in its external form and types the keys it expects into its window:
# a, Shift-b, Backspace, Command-a, Command-q (Command is the Super key).
#
#   KEYS_MODE=x11   [DISPLAY=:93]                  carbon_key_events.sh <command that runs carbon_key_events external>
#   KEYS_MODE=sway|kwin KEYS_INJECTOR=<injector>   as above on the compositor named by $WAYLAND_DISPLAY and $XDG_RUNTIME_DIR
#
# X11 needs xdotool and a window manager. On Wayland the injector takes a pause in milliseconds and evdev key
# codes with 1 or 0 for press and release: virtual-keyboard-keys on sway (a headless sway whose windows float,
# named by $SWAYSOCK; needs swaymsg) and kwin-keys on a private kwin_wayland --virtual --no-lockscreen (see the
# headers of virtual-keyboard-keys.c and kwin-keys.c in the CoreGraphics tests for how they are built).
set -u
mode=${KEYS_MODE:-x11}
work=$(mktemp -d)
out=$work/out
"$@" > "$out" 2>&1 < /dev/null &
pid=$!
trap 'kill $pid 2>/dev/null; rm -rf "$work"' EXIT

for _ in $(seq 300); do grep -q '^READY' "$out" && break; sleep 0.1; done
grep -q '^READY' "$out" || { echo "FAIL: the test never became ready"; cat "$out"; exit 1; }
sleep 2

if [ "$mode" = x11 ]; then
    win=$(xdotool search --name '^carbon-key-events$' | head -1)
    [ -n "$win" ] || { echo "FAIL: no window"; cat "$out"; exit 1; }
    xdotool windowactivate --sync "$win" 2>/dev/null
    xdotool windowfocus --sync "$win" 2>/dev/null
    sleep 0.5
    D="--delay 120"
    key() { xdotool "$@"; sleep 0.3; }
    key key $D a
    key key $D shift+b
    key key $D BackSpace
    key key $D super+a
    key key $D super+q
else
    [ "$mode" = sway ] && swaymsg -q '[title="carbon-key-events"] focus'
    sleep 0.5
    # evdev codes: A 30, B 48, Q 16, Backspace 14, Left Shift 42, Left Meta (Super) 125
    K() { "${KEYS_INJECTOR:?the key injector}" 100 "$@"; sleep 0.3; }
    K 30:1 30:0
    K 42:1 48:1 48:0 42:0
    K 14:1 14:0
    K 125:1 30:1 30:0 125:0
    K 125:1 16:1 16:0 125:0
fi

for _ in $(seq 150); do grep -q -E '^(PASS|FAIL)' "$out" && break; sleep 0.1; done
wait $pid
status=$?
cat "$out"
exit $status
