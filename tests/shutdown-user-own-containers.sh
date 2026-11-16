#!/bin/bash
# Copyright (C) 2026 Lubos Dolezel
#
# tools/shutdown-user.sh given an installation prefix must stop only the
# containers served by that installation's darlingserver, and must do it with
# that installation's darling launcher. Two fake installations are set up; each
# has a "darlingserver" (a copy of bash) whose child plays the container's
# launchd (argv[0] "/sbin/launchd"). The fake launchers record who they were
# asked to stop and then end the container.
# Runs unprivileged, using the rootless code path of the script.
set -u
here=$(cd "$(dirname "$0")" && pwd)
script="$here/../tools/shutdown-user.sh"
work=$(mktemp -d)
pids=()
cleanup() { kill "${pids[@]}" 2>/dev/null; rm -rf "$work"; }
trap cleanup EXIT

make_install() {
	local root=$1 argv0=$2
	mkdir -p "$root/bin"
	cp /bin/bash "$root/bin/darlingserver"
	cat > "$root/bin/darling" <<LAUNCHER
#!/bin/sh
echo "\$DPREFIX" >> "$root/stopped"
kill \$(cat "$root/launchd.pid")
LAUNCHER
	chmod +x "$root/bin/darling"
	# argv[0] given as a bare name or as a full path, as seen in the wild.
	[ "$argv0" = full ] && argv0="$root/bin/darlingserver" || argv0=darlingserver
	# The prefix argument is a script here, so the fake server runs it; it
	# starts the fake launchd.
	cat > "$root/prefix" <<PREFIX
exec -a /sbin/launchd sleep 300 &
echo \$! > '$root/launchd.pid'
wait
PREFIX
	(exec -a "$argv0" "$root/bin/darlingserver" "$root/prefix") &
	pids+=($!)
}

make_install "$work/a" bare
make_install "$work/b" full
sleep 1

export DARLING_NONROOT=1
fail=0
bash "$script" "$work/a" || { echo "FAIL: script failed"; fail=1; }

[ "$(cat "$work/a/stopped" 2>/dev/null)" = "$work/a/prefix" ] \
	|| { echo "FAIL: install a's container was not stopped with its own launcher and prefix"; fail=1; }
[ ! -e "$work/b/stopped" ] \
	|| { echo "FAIL: another installation's launcher was invoked"; fail=1; }
kill -0 "$(cat "$work/b/launchd.pid")" 2>/dev/null \
	|| { echo "FAIL: another installation's container was stopped"; fail=1; }

# A server started with a full-path argv[0] is attributed through its prefix too.
bash "$script" "$work/b" || { echo "FAIL: script failed for b"; fail=1; }
[ "$(cat "$work/b/stopped" 2>/dev/null)" = "$work/b/prefix" ] \
	|| { echo "FAIL: install b's container (full-path argv[0]) was not stopped"; fail=1; }

[ "$fail" = 0 ] && echo "PASS"
exit "$fail"
