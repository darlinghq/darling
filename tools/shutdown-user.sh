#!/bin/sh

if [ "$(whoami)" != "root" ]
then
	echo "$0 must be run as root, invoking sudo"
	exec sudo su -c "$0" "$@"
fi

echo "Seeing if Darling is currently running"

# A container's launchd shows up with "/sbin/launchd" as the first word of its
# args (mldr drops its own argv[0]). Match only that word, so commands that
# merely mention launchd do not count. Skip zombies, which cannot be shut down
# and are left to their parent to reap.
running_launchd() {
	ps -eo pid=,stat=,args= | awk '$2 !~ /^Z/ && $3 == "/sbin/launchd" { print $1 }'
}

# Each round shuts down every container still running. The bound is on rounds,
# not on one PID, so one stuck container cannot starve the others and a
# container that comes back under a new PID cannot loop forever.
rounds=0
PIDS=$(running_launchd)

while [ -n "$PIDS" ]
do
	if [ "$rounds" -ge 5 ]
	then
		echo "Darling (launchd PIDs $(echo $PIDS)) is still running after $rounds shutdown rounds, giving up." >&2
		echo "It may belong to a prefix that \`darling shutdown\` does not select by default (DPREFIX)." >&2
		exit 1
	fi
	for PID in $PIDS
	do
		RUNNING_USER=$(ps -o uname= -p "$PID" | head -n1 | tr -d '[:space:]')
		# The process may have exited between the two ps calls.
		if [ -n "$RUNNING_USER" ]
		then
			echo "Darling currently running for $RUNNING_USER, shutting it down..."
			timeout 15s su --login "$RUNNING_USER" -c "darling shutdown"
		fi
	done
	rounds=$((rounds + 1))
	sleep 2
	PIDS=$(running_launchd)
done

echo "No instances running now"
