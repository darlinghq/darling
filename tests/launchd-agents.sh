#!/bin/bash
# Copyright (C) 2026 Lubos Dolezel
#
# The container's launchd loads the LaunchAgents directories (/System/Library,
# /Library and ~/Library of the prefix user) when the container starts, as the
# user's session would: RunAtLoad starts an agent, Disabled and a
# LimitLoadToSessionType that names no session of the container keep it from
# loading, KeepAlive restarts it. launchctl load / unload of an agent plist
# works as well, including one limited to the Aqua or Background session.
#
# usage: launchd-agents.sh prepare | check | cleanup | manual
#   prepare   write the agent plists (run inside the guest, then restart the container)
#   check     after the restart: the agents that must run did, the others did not
#   cleanup   remove the plists, markers and jobs
#   manual    launchctl load / unload (needs no restart)
# ARCH=i386 (or x86_64) makes the agents run their shell through arch(1).
set -u
mode=${1:?usage: $0 prepare|check|cleanup|manual}
prefix=org.darlinghq.tests.agents
arch_cmd=
[ -n "${ARCH:-}" ] && arch_cmd="<string>/usr/bin/arch</string><string>-$ARCH</string>"
dirs="/System/Library/LaunchAgents /Library/LaunchAgents $HOME/Library/LaunchAgents"
failures=0
check() { if ! "$@"; then echo "FAIL: $*"; failures=$((failures + 1)); fi; }
loaded() { launchctl list "$1" >/dev/null 2>&1; }
not_loaded() { ! loaded "$1"; }
marker() { echo "/tmp/$prefix.$1.marker"; }

# plist LABEL FILE EXTRA-KEYS: a job that records its start in the marker file
plist() {
	cat > "$2" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>Label</key><string>$1</string>
	<key>ProgramArguments</key>
	<array>$arch_cmd<string>/bin/sh</string><string>-c</string><string>echo started >> $(marker "${1#$prefix.}")</string></array>
	<key>RunAtLoad</key><true/>
	$3
</dict>
</plist>
PLIST
}

names() {
	for where in system local user; do echo "enabled-$where disabled-$where"; done
	echo "aqua-array loginwindow keepalive"
}

dir_of() {
	case $1 in
	*-system) echo /System/Library/LaunchAgents ;;
	*-local|aqua-array|loginwindow) echo /Library/LaunchAgents ;;
	*) echo "$HOME/Library/LaunchAgents" ;;
	esac
}

extra_of() {
	case $1 in
	disabled-*) echo "<key>Disabled</key><true/>" ;;
	aqua-array) echo "<key>LimitLoadToSessionType</key><array><string>Aqua</string><string>Background</string></array>" ;;
	loginwindow) echo "<key>LimitLoadToSessionType</key><string>LoginWindow</string>" ;;
	keepalive) echo "<key>KeepAlive</key><true/><key>ThrottleInterval</key><integer>1</integer>" ;;
	esac
}

should_run() { case $1 in disabled-*|loginwindow) return 1 ;; *) return 0 ;; esac; }

case $mode in
prepare)
	for d in $dirs; do mkdir -p "$d"; done
	for n in $(names); do
		rm -f "$(marker "$n")"
		plist "$prefix.$n" "$(dir_of "$n")/$prefix.$n.plist" "$(extra_of "$n")"
	done
	;;
check)
	for i in $(seq 1 20); do
		ok=1
		for n in $(names); do should_run "$n" && [ ! -s "$(marker "$n")" ] && ok=0; done
		[ $ok = 1 ] && break
		sleep 1
	done
	sleep 3
	for n in $(names); do
		if should_run "$n"; then
			check test -s "$(marker "$n")"
			check loaded "$prefix.$n"
		else
			check test ! -e "$(marker "$n")"
			check not_loaded "$prefix.$n"
		fi
	done
	# KeepAlive: the job exits at once and is started again
	check test "$(cat "$(marker keepalive)" 2>/dev/null | wc -l)" -ge 2
	;;
cleanup)
	for n in $(names); do
		launchctl unload "$(dir_of "$n")/$prefix.$n.plist" >/dev/null 2>&1
		rm -f "$(dir_of "$n")/$prefix.$n.plist" "$(marker "$n")"
	done
	;;
manual)
	work=$(mktemp -d)
	trap 'for f in "$work"/*.plist; do launchctl unload "$f" >/dev/null 2>&1; done; rm -rf "$work"; for n in $(names) manual-aqua manual-plain manual-disabled; do rm -f "$(marker "$n")"; done' EXIT
	for n in manual-aqua manual-plain manual-disabled; do rm -f "$(marker "$n")"; done
	plist "$prefix.manual-plain" "$work/plain.plist" ""
	plist "$prefix.manual-aqua" "$work/aqua.plist" "<key>LimitLoadToSessionType</key><array><string>LoginWindow</string><string>Aqua</string></array>"
	plist "$prefix.manual-disabled" "$work/disabled.plist" "<key>Disabled</key><true/>"
	check launchctl load "$work/plain.plist"
	check launchctl load "$work/aqua.plist"
	launchctl load "$work/disabled.plist" >/dev/null 2>&1
	for i in 1 2 3 4 5 6 7 8 9 10; do [ -s "$(marker manual-plain)" ] && [ -s "$(marker manual-aqua)" ] && break; sleep 1; done
	check test -s "$(marker manual-plain)"
	check test -s "$(marker manual-aqua)"
	check loaded "$prefix.manual-plain"
	check loaded "$prefix.manual-aqua"
	check test ! -e "$(marker manual-disabled)"
	check not_loaded "$prefix.manual-disabled"
	check launchctl unload "$work/plain.plist"
	check launchctl unload "$work/aqua.plist"
	check not_loaded "$prefix.manual-plain"
	check not_loaded "$prefix.manual-aqua"
	# -F loads a disabled job anyway
	check launchctl load -F "$work/disabled.plist"
	check loaded "$prefix.manual-disabled"
	check launchctl unload "$work/disabled.plist"
	check not_loaded "$prefix.manual-disabled"
	;;
*)
	echo "unknown mode $mode"; exit 2 ;;
esac
[ "$failures" = 0 ] && echo "PASS ($mode)"
exit $failures
