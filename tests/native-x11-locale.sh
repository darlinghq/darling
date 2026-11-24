#!/bin/bash
# SPDX-License-Identifier: MIT
# Copyright (C) 2026 Lubos Dolezel
# Runs native-x11-locale ($1, the guest program) against the X server $DISPLAY: when the program reports READY, the
# keys of one case are sent with xdotool and the program must receive the expected text from Xutf8LookupString().
# $2 is the command that runs a guest binary, such as "darling shell"; $3 is a scratch directory for the output.
# The input method is the one of XMODIFIERS (default "@im=none": Xlib's own compose handling); LANG selects the
# host locale the guest inherits.
#   cases: dead_acute e -> é,  dead_diaeresis o -> ö,  Multi_key o c -> ©,  dead_acute c -> ć,  dead_acute n -> ń
#   (the last two are outside Latin-1: composing them needs a UTF-8 locale in Xlib)
set -u
program=$1
runner=$2
temp=$3
mkdir -p "$temp"
failures=0

# A keymap that has the dead keys and the compose key (Alt_R). The X server must keep it while no client is
# connected, so it has to be started with -noreset (or have a client that stays connected): without that a server
# resets its keymap when the last client leaves. The keys are never remapped while a client runs.
setxkbmap -layout us -variant intl -option compose:ralt || { echo "FAIL: setxkbmap"; exit 1; }

run_case() { # expected key...
    local expected=$1
    shift
    local out="$temp/case.out"
    : > "$out"
    $runner "$program" "$expected" 10 > "$out" 2>&1 < /dev/null &
    local guest=$!
    for _ in $(seq 300); do grep -q '^READY' "$out" && break; sleep 0.1; done
    if ! grep -q '^READY' "$out"; then
        echo "FAIL: the program never became ready"; cat "$out"; kill "$guest" 2>/dev/null; failures=$((failures + 1)); return
    fi
    sleep 0.5
    for key in "$@"; do
        xdotool key "$key"
        sleep 0.3
    done
    wait "$guest" || true
    if grep -q '^PASS' "$out"; then
        echo "PASS: $* -> $expected"
    else
        echo "FAIL: $* -> $expected"; cat "$out"; failures=$((failures + 1))
    fi
}

run_case é dead_acute e
run_case ö dead_diaeresis o
run_case © Multi_key o c
run_case ć dead_acute c
run_case ń dead_acute n
[ "$failures" = 0 ] && echo "ALL PASS"
exit $failures
