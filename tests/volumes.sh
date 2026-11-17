#!/bin/sh
# Volumes, eject, trash, icons and reveal, against the behaviour observed on macOS (see volumes.md).
# Needs DARLING_SKILLS (a darling-skills checkout) and a running Darling.
set -u

here=$(cd "$(dirname "$0")" && pwd)
ext="$here/../src/external"
work=$(mktemp -d)
gw="/Volumes/SystemRoot$work"
failures=0

pass() { echo "PASS: $1"; }
fail() { echo "FAIL: $1"; failures=$((failures + 1)); }

# runs a shell command line in the guest; stdout, stderr and exit status go to $work/<name>.{out,err,rc}
# (a command that uses exit must run in a subshell, or no status is written)
guest() {
	name=$1
	shift
	timeout -s KILL 300 darling shell /bin/sh -c "{ $1; } >'$gw/$name.out' 2>'$gw/$name.err'; echo \$? >'$gw/$name.rc'" </dev/null >/dev/null 2>&1
}
rc() { cat "$work/$1.rc" 2>/dev/null; }
outputs() { cat "$work/$1.out" "$work/$1.err" 2>/dev/null | tr '\n' '|'; }
line() { grep "^$2" "$work/$1.out" 2>/dev/null | head -n 1; } # <name> <prefix>: the first output line with that prefix

build() { # <program> <source> [<guest-cc arguments>...]
	program=$1
	source=$2
	shift 2
	"$DARLING_SKILLS/darling-guest-testing/scripts/guest-cc.sh" -o "$work/$program" "$source" "$@" \
		>"$work/$program.build" 2>&1 || { fail "$program does not build: $(cat "$work/$program.build")"; return 1; }
}

has() { # <line> <word>...: every word is in the line
	l=$1
	shift
	for w in "$@"; do
		case " $l " in *" $w "*) ;; *) return 1 ;; esac
	done
}
lacks() { # <line> <word>...: no word is in the line
	l=$1
	shift
	for w in "$@"; do
		case " $l " in *" $w "*) return 1 ;; esac
	done
}

cp "$here/fixtures/unmount-test.dmg" "$work/"
attach() { guest attach "hdiutil attach -quiet '$gw/unmount-test.dmg'"; }
detach() { guest detach "hdiutil detach -quiet '/Volumes/Unmount Test'"; }

# mount flags: statfs gives the root, a disk image and a hidden volume the flags macOS gives them
if build volume-flags "$ext/xnu/tests/volume-flags.c"; then
	attach
	guest flags "'$gw/volume-flags' / '/Volumes/Unmount Test' /proc"
	detach
	root=$(line flags "/: ")
	image=$(line flags "/Volumes/Unmount Test: ")
	proc=$(line flags "/proc: ")
	if has "$root" LOCAL ROOTFS && lacks "$root" REMOVABLE DONTBROWSE \
		&& has "$image" LOCAL REMOVABLE RDONLY && lacks "$image" ROOTFS DONTBROWSE \
		&& has "$proc" DONTBROWSE
	then
		pass "mount flags"
	else
		fail "mount flags: $(outputs flags)"
	fi
fi

rm -rf "$work"
[ "$failures" = 0 ]
