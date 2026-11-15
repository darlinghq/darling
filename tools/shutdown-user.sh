#!/bin/sh

# The launcher treats a setting starting with 1, t or T as enabled.
setting_enabled() {
	case "${1:-}" in
	1*|[tT]*) return 0 ;;
	esac
	return 1
}

CURRENT_USER=$(whoami)
nonroot=0
if [ "$CURRENT_USER" != "root" ]
then
	# Rootless Darling runs the container as the invoking user, so no root
	# privileges and no su are needed to stop it. The launcher has the same
	# variables; DARLING_NONROOT also works on hosts without user namespaces.
	if setting_enabled "$DARLING_NONROOT" || setting_enabled "$DARLING_ROOTLESS"
	then
		nonroot=1
	else
		echo "$0 must be run as root, invoking sudo"
		exec sudo su -c "$0" "$@"
	fi
fi

echo "Seeing if Darling is currently running"

# A container's launchd shows up with "/sbin/launchd" as the first word of its
# args (mldr drops its own argv[0]). Match only that word, so commands that
# merely mention launchd do not count. Skip zombies, which cannot be shut down
# and are left to their parent to reap.
running_launchd() {
	ps -eo pid=,stat=,args= | awk '$2 !~ /^Z/ && $3 == "/sbin/launchd" { print $1 }'
}

# Rootless containers belong to the user who started them. Other users'
# containers cannot be shut down without their privileges, so ignore them
# instead of retrying until the round bound is reached.
launchd_pids() {
	if [ "$nonroot" = 1 ]
	then
		for PID in $(running_launchd)
		do
			RUNNING_USER=$(ps -o uname= -p "$PID" | head -n1 | tr -d '[:space:]')
			[ "$RUNNING_USER" = "$CURRENT_USER" ] && echo "$PID"
		done
	else
		running_launchd
	fi
}

# The prefix a container's launchd belongs to: its parent is darlingserver,
# started as "darlingserver PREFIX UID GID ...". `su --login` drops DPREFIX,
# so without this a container in a non-default prefix could never be stopped.
# Only plain absolute paths are passed on, since the value ends up in `su -c`.
container_prefix() {
	parent=$(ps -o ppid= -p "$1" | tr -d '[:space:]')
	[ -n "$parent" ] || return
	ps -o args= -p "$parent" | awk '$1 == "darlingserver" && $2 ~ /^\/[A-Za-z0-9._\/+-]*$/ { print $2 }'
}

# Each round shuts down every container still running. The bound is on rounds,
# not on one PID, so one stuck container cannot starve the others and a
# container that comes back under a new PID cannot loop forever.
rounds=0
PIDS=$(launchd_pids)

while [ -n "$PIDS" ]
do
	if [ "$rounds" -ge 5 ]
	then
		echo "Darling (launchd PIDs $(echo $PIDS)) is still running after $rounds shutdown rounds, giving up." >&2
		echo "Try \`DPREFIX=<prefix> darling shutdown\` as the user that owns it." >&2
		exit 1
	fi
	for PID in $PIDS
	do
		RUNNING_USER=$(ps -o uname= -p "$PID" | head -n1 | tr -d '[:space:]')
		# The process may have exited between the two ps calls.
		if [ -n "$RUNNING_USER" ]
		then
			PREFIX=$(container_prefix "$PID")
			if [ -n "$PREFIX" ]
			then
				SHUTDOWN_COMMAND="DPREFIX='$PREFIX' darling shutdown"
				echo "Darling currently running for $RUNNING_USER in $PREFIX, shutting it down..."
			else
				SHUTDOWN_COMMAND="darling shutdown"
				echo "Darling currently running for $RUNNING_USER, shutting it down..."
			fi
			if [ "$nonroot" = 1 ]
			then
				timeout 15s sh -c "$SHUTDOWN_COMMAND"
			else
				timeout 15s su --login "$RUNNING_USER" -c "$SHUTDOWN_COMMAND"
			fi
		fi
	done
	rounds=$((rounds + 1))
	sleep 2
	PIDS=$(launchd_pids)
done

echo "No instances running now"
