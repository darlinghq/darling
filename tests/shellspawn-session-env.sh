#!/bin/sh
# Copyright (C) 2026 Lubos Dolezel
#
# The environment of a command run with `darling shell` comes from the command's own session,
# not from the session that started the container: a display variable the command's session
# does not have must be absent in the guest, and one it has must carry its own value.
# Needs an installed Darling and DPREFIX/PATH set for it; stops the container with
# `darling shutdown` first and last.
fail=0
check() { # name expected actual
	if [ "$2" != "$3" ]; then echo "FAIL $1: expected '$2', got '$3'"; fail=1; fi
}

darling shutdown >/dev/null 2>&1
# the container starts with a Wayland session
env WAYLAND_DISPLAY=/tmp/wl-first DISPLAY=:1 XDG_RUNTIME_DIR=/tmp/xdg-first darling shell true </dev/null >/dev/null 2>&1

got=$(env -u WAYLAND_DISPLAY DISPLAY=:2 XDG_RUNTIME_DIR=/tmp/xdg-second darling shell sh -c 'echo "${WAYLAND_DISPLAY-unset} $DISPLAY $XDG_RUNTIME_DIR"' </dev/null 2>/dev/null)
check "second session without WAYLAND_DISPLAY" "unset :2 /tmp/xdg-second" "$got"

got=$(env -u WAYLAND_DISPLAY -u DISPLAY -u XDG_RUNTIME_DIR darling shell sh -c 'echo "${WAYLAND_DISPLAY-unset} ${DISPLAY-unset} ${XDG_RUNTIME_DIR-unset}"' </dev/null 2>/dev/null)
check "session without display variables" "unset unset unset" "$got"

darling shutdown >/dev/null 2>&1
[ $fail = 0 ] && echo PASS
exit $fail
