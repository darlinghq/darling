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

# volume keys: NSURL's volume resource keys for volumes and for a file in one (see nsurl-volume-keys.m)
value() { # <name> <path> <key>: the value the program printed for that path and key
	awk -v p="== $2" -v k="$3 = " '$0 == p { on = 1; next } /^== / { on = 0 } on && index($0, k) == 1 { print substr($0, length(k) + 1); exit }' "$work/$1.out"
}
expect_key() { # <name> <path> <key> <expected>
	got=$(value "$1" "$2" "NSURL$3Key")
	[ "$got" = "$4" ] || { keys_ok=0; echo "  $2: $3 is '$got', expected '$4'"; }
}
if build nsurl-volume-keys "$ext/corefoundation/tests/nsurl-volume-keys.m" -framework Foundation; then
	cp "$here/fixtures/slash-name.dmg" "$here/fixtures/amp-name.dmg" "$work/"
	guest keys-attach "hdiutil attach -quiet '$gw/unmount-test.dmg' && hdiutil attach -quiet '$gw/slash-name.dmg' && hdiutil attach -quiet '$gw/amp-name.dmg'"
	guest keys "'$gw/nsurl-volume-keys' / '/Volumes/Unmount Test/' '/Volumes/Unmount Test/hello.txt' '/Volumes/A:B' '/Volumes/Tom & Jerry <1>'"
	guest keys-detach "hdiutil detach -quiet '/Volumes/Unmount Test'; hdiutil detach -quiet '/Volumes/A:B'; hdiutil detach -quiet '/Volumes/Tom & Jerry <1>'"
	guest keys-after "'$gw/nsurl-volume-keys' '/Volumes/Unmount Test' /Volumes"
	keys_ok=1
	for p in '/Volumes/Unmount Test/' '/Volumes/Unmount Test/hello.txt'; do
		expect_key keys "$p" VolumeName "Unmount Test"
		expect_key keys "$p" VolumeLocalizedName "Unmount Test"
		for k in VolumeIsEjectable VolumeIsRemovable VolumeIsReadOnly VolumeIsLocal VolumeIsBrowsable; do
			expect_key keys "$p" $k 1
		done
		expect_key keys "$p" VolumeIsRootFileSystem 0
		expect_key keys "$p" VolumeIsInternal nil
		expect_key keys "$p" VolumeLocalizedFormatDescription "Mac OS Extended"
		expect_key keys "$p" VolumeTypeName hfs
		expect_key keys "$p" VolumeURL "file:///Volumes/Unmount%20Test/"
		expect_key keys "$p" VolumeIdentifier "8 bytes"
	done
	expect_key keys '/Volumes/Unmount Test/' IsVolume 1
	expect_key keys '/Volumes/Unmount Test/hello.txt' IsVolume 0
	case $(value keys '/Volumes/Unmount Test/' NSURLVolumeUUIDStringKey) in
		????????-????-????-????-????????????) ;;
		*) keys_ok=0; echo "  the image has no volume UUID" ;;
	esac
	expect_key keys / VolumeIsEjectable 0
	expect_key keys / VolumeIsRemovable 0
	expect_key keys / VolumeIsInternal 1
	expect_key keys / VolumeIsRootFileSystem 1
	expect_key keys / VolumeIsBrowsable 1
	expect_key keys / IsVolume 1
	expect_key keys / VolumeURL "file:///"
	expect_key keys '/Volumes/A:B' VolumeName "A/B"
	expect_key keys '/Volumes/Tom & Jerry <1>' VolumeName "Tom & Jerry <1>"
	[ "$keys_ok" = 1 ] && [ "$(rc keys)" = 0 ] && pass "volume keys" || fail "volume keys: $(rc keys) $(cat "$work/keys.err")"
	# after the image is detached (Review Focus 2)
	keys_ok=1
	expect_key keys-after /Volumes VolumeURL "file:///"
	expect_key keys-after '/Volumes/Unmount Test' VolumeName nil
	[ "$keys_ok" = 1 ] && [ "$(rc keys-after)" = 0 ] && pass "volume keys after detach" \
		|| fail "volume keys after detach: $(rc keys-after) $(outputs keys-after)"
fi

# mounted volumes: Foundation's list of volumes (see filemanager-volumes.m)
if build filemanager-volumes "$ext/foundation/tests/filemanager-volumes.m" -framework Foundation; then
	attach
	hidden=/proc # in non-root mode the guest's /proc is the host's, which is not one of the guest's volumes
	[ "${DARLING_NONROOT:-}" = 1 ] && hidden=
	guest volumes "'$gw/filemanager-volumes' $hidden"
	detach
	[ "$(rc volumes)" = 0 ] && pass "mounted volumes" || fail "mounted volumes: $(outputs volumes)"
fi

# trash on other volumes: NSTrashDirectory and trashItemAtURL: on the home volume, on the host's file system (a volume
# of its own in every mode) and on a read-only image (see filemanager-trash-volumes.m). The other volume's
# .Trashes/<uid> must not exist before; it is removed afterwards, and .Trashes too if the case made it.
if build filemanager-trash-volumes "$ext/foundation/tests/filemanager-trash-volumes.m" -framework Foundation; then
	mkdir "$work/other"
	host_trashes=$(df --output=target "$work" | tail -n 1)/.Trashes
	trashes_existed=0
	[ -e "$host_trashes" ] && trashes_existed=1
	attach
	guest trash "(uid=\$(id -u); test ! -e '/Volumes/SystemRoot$host_trashes/'\$uid || { echo \"$host_trashes/\$uid exists\"; exit 1; }
		d=\$(mktemp -d \"\$HOME/trash-test.XXXXXX\") || exit 1
		'$gw/filemanager-trash-volumes' \"\$d\" '$gw/other' '/Volumes/Unmount Test/hello.txt'; r=\$?
		rm -rf \"\$d\" '/Volumes/SystemRoot$host_trashes/'\$uid
		[ $trashes_existed = 1 ] || rmdir '/Volumes/SystemRoot$host_trashes' 2>/dev/null
		exit \$r)"
	detach
	[ "$(rc trash)" = 0 ] && pass "trash on other volumes" || fail "trash on other volumes: $(outputs trash)"
fi

rm -rf "$work"
[ "$failures" = 0 ]
