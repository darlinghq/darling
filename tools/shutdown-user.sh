#!/bin/sh

if [ "$(whoami)" != "root" ]
then
	echo "$0 must be run as root, invoking sudo"
	exec sudo su -c "$0" "$@"
fi

echo "Seeing if Darling is currently running"

# A container's launchd shows up as exactly "/sbin/launchd" (mldr drops its own
# argv[0]). Match only that, so commands that merely mention launchd do not
# count, and skip zombies, which cannot be shut down and are left to their
# parent to reap.
running_launchd() {
	ps -eo pid=,stat=,args= | awk '$2 !~ /^Z/ && $3 == "/sbin/launchd" { print $1 }'
}

attempts=0
PID=$(running_launchd | head -n1)

while [ -n "$PID" ]
do
	if [ "$attempts" -ge 5 ]
	then
		echo "Darling (launchd PID $PID) is still running after $attempts shutdown attempts, giving up." >&2
		echo "It may belong to a prefix that \`darling shutdown\` does not select by default (DPREFIX)." >&2
		exit 1
	fi
	RUNNING_USER=$(ps -o uname= -p "$PID" | head -n1 | tr -d '[:space:]')
	# The process may have exited between the two ps calls.
	if [ -n "$RUNNING_USER" ]
	then
		echo "Darling currently running for $RUNNING_USER, shutting it down..."
		timeout 15s su --login "$RUNNING_USER" -c "darling shutdown"
	fi
	attempts=$((attempts + 1))
	sleep 2
	PID=$(running_launchd | head -n1)
done

echo "No instances running now"
