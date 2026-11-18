#!/bin/bash
# Copyright (C) 2026 Lubos Dolezel
#
# launchctl load -S Background must load an agent whose plist limits it to the
# Background session, as it does on macOS: the label is registered, its
# MachServices name can be looked up with bootstrap_look_up, a message sent to
# it starts the job, and launchctl unload removes it again.
# usage: launchd-background-session.sh <helper>   (helper built from
# launchd-background-session.c for the architecture under test)
set -u
helper=${1:?usage: $0 <helper>}
work=$(mktemp -d)
label=org.darlinghq.tests.background-session.$$
service=$label.service
plist=$work/agent.plist
failures=0
trap 'launchctl unload -S Background "$plist" >/dev/null 2>&1; rm -rf "$work"' EXIT

check() { if ! "$@"; then echo "FAIL: $*"; failures=$((failures + 1)); fi; }

cat > "$plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>Label</key><string>$label</string>
	<key>LimitLoadToSessionType</key><string>Background</string>
	<key>ProgramArguments</key>
	<array><string>$helper</string><string>serve</string><string>$service</string><string>$work/received</string></array>
	<key>MachServices</key><dict><key>$service</key><true/></dict>
</dict>
</plist>
PLIST

check launchctl load -S Background "$plist"
check "$helper" lookup "$service"
check "$helper" send "$service"
for i in 1 2 3 4 5 6 7 8 9 10; do [ -s "$work/received" ] && break; sleep 1; done
check test "$(cat "$work/received" 2>/dev/null)" = 5067591
check launchctl unload -S Background "$plist"
check test -z "$(launchctl list 2>/dev/null | grep -F "$label")"
[ "$failures" = 0 ] && echo PASS
exit $failures
