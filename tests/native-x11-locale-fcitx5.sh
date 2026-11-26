#!/bin/bash
# SPDX-License-Identifier: MIT
# Copyright (C) 2026 Lubos Dolezel
# native-x11-locale.sh with a real input method server: a private fcitx5 (its own D-Bus session, configuration and
# runtime directories, with the keyboard input method that composes dead-key and compose-key sequences itself) on
# the X server $DISPLAY. The guest program talks to it over XIM (XMODIFIERS=@im=fcitx), so the composed text it gets is
# the input method's commit, which reaches the client in the client library's locale encoding.
# Usage: native-x11-locale-fcitx5.sh PROGRAM RUNNER SCRATCH-DIR (as native-x11-locale.sh). Needs fcitx5,
# dbus-run-session, setxkbmap and xdotool; prints SKIP when one is missing.
set -u
here=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
for tool in fcitx5 dbus-run-session xdotool setxkbmap; do
    command -v "$tool" > /dev/null || { echo "SKIP: $tool is not installed"; exit 0; }
done
temp=$(mktemp -d)
trap 'rm -rf "$temp"' EXIT
mkdir -p "$temp/config/fcitx5" "$temp/data" "$temp/cache" "$temp/runtime" "$temp/home"
chmod 700 "$temp/runtime"
cat > "$temp/config/fcitx5/profile" <<PROFILE
[Groups/0]
Name=Default
Default Layout=us
DefaultIM=keyboard-us

[Groups/0/Items/0]
Name=keyboard-us
Layout=

[GroupOrder]
0=Default
PROFILE

cat > "$temp/session.sh" <<'SESSION'
#!/usr/bin/env bash
# $1: scratch directory of the session, then the arguments of native-x11-locale.sh
set -u
temp=$1
shift
fcitx5 -D --disable all --enable xcb,xim,dbus,dbusfrontend,keyboard,imselector > "$temp/fcitx5.log" 2>&1 &
server=$!
trap 'kill "$server" 2>/dev/null || true' EXIT
for _ in $(seq 100); do grep -q 'Service name change' "$temp/fcitx5.log" && break; sleep 0.1; done
grep -q 'Service name change' "$temp/fcitx5.log" || { echo "FAIL: the private fcitx5 did not start"; cat "$temp/fcitx5.log"; exit 1; }
sleep 1
"$@"
SESSION
chmod +x "$temp/session.sh"

env -u DBUS_SESSION_BUS_ADDRESS -u WAYLAND_DISPLAY -u WAYLAND_SOCKET -u XDG_SESSION_TYPE \
    HOME="$temp/home" XDG_CONFIG_HOME="$temp/config" XDG_DATA_HOME="$temp/data" XDG_CACHE_HOME="$temp/cache" \
    XDG_RUNTIME_DIR="$temp/runtime" XMODIFIERS=@im=fcitx LANG="${LANG:-en_US.UTF-8}" \
    dbus-run-session -- "$temp/session.sh" "$temp" "$here/native-x11-locale.sh" "$@"
