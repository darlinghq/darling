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

# workspace volumes: NSWorkspace's volume lists, file system info and eject (see nsworkspace-volumes.m); the dissenter
# is da-ops (see diskarbitration.md)
if build nsworkspace-volumes "$ext/cocotron/tests/nsworkspace-volumes.m" -framework AppKit \
	&& build da-ops "$here/da-ops.c" -framework DiskArbitration -framework CoreFoundation
then
	nwv="'$gw/nsworkspace-volumes'"
	gone="(for i in 1 2 3 4 5 6 7 8 9 10; do test -e '/Volumes/Unmount Test' || exit 0; sleep 0.5; done; echo 'still mounted'; exit 1)"
	guest wv "hdiutil attach -quiet '$gw/unmount-test.dmg'
		$nwv lists; $nwv info '/Volumes/Unmount Test'; $nwv info /; $nwv info /nonexistent
		$nwv eject /nonexistent; $nwv eject /; ls / >/dev/null && echo 'root works'
		$nwv eject '/Volumes/Unmount Test/hello.txt'; $gone && echo 'gone after eject'
		hdiutil attach -quiet '$gw/unmount-test.dmg'; $nwv eject-path '/Volumes/Unmount Test'; $gone && echo 'gone after eject-path'
		hdiutil attach -quiet '$gw/unmount-test.dmg'
		'$gw/da-ops' deny 15 >'$gw/deny.out' & deny=\$!
		for i in 1 2 3 4 5 6 7 8 9 10; do grep -q ready '$gw/deny.out' && break; sleep 0.5; done
		$nwv eject '/Volumes/Unmount Test' | sed 's/^/dissented /'; test -d '/Volumes/Unmount Test' && echo 'mounted after dissent'
		kill \$deny; wait \$deny
		$nwv eject '/Volumes/Unmount Test' | sed 's/^/undissented /'; $gone && echo 'gone after undissented eject'"
	wv_ok=1
	expect_line() { # <expected line>
		grep -qxF "$1" "$work/wv.out" || { wv_ok=0; echo "  missing: $1"; }
	}
	local_paths=$(line wv "local: ")
	case "|${local_paths#local: }|" in *"|/|"*) ;; *) wv_ok=0; echo "  no / in $local_paths" ;; esac
	case "|${local_paths#local: }|" in *"|/Volumes/Unmount Test|"*) ;; *) wv_ok=0; echo "  no image in $local_paths" ;; esac
	expect_line "removable: /Volumes/Unmount Test"
	expect_line "info /Volumes/Unmount Test: 1 1 0 1 hfs hfs"
	case $(line wv "info /: ") in "info /: 1 0 "[01]" 0 "*) ;; *) wv_ok=0; echo "  $(line wv "info /: ")" ;; esac
	expect_line "info /nonexistent: 0 0 0 0 nil nil"
	expect_line "eject /nonexistent: 0 NSOSStatusErrorDomain -36 0"
	expect_line "eject /: 0 NSOSStatusErrorDomain -47 0"
	expect_line "root works"
	expect_line "eject /Volumes/Unmount Test/hello.txt: 1 - 0 0"
	expect_line "gone after eject"
	expect_line "eject-path /Volumes/Unmount Test: 1"
	expect_line "gone after eject-path"
	expect_line "dissented eject /Volumes/Unmount Test: 0 NSOSStatusErrorDomain -47 0"
	expect_line "mounted after dissent"
	expect_line "undissented eject /Volumes/Unmount Test: 1 - 0 0"
	expect_line "gone after undissented eject"
	[ "$wv_ok" = 1 ] && pass "workspace volumes" || fail "workspace volumes: $(outputs wv)"
	guest wv-cleanup "test -e '/Volumes/Unmount Test' && hdiutil detach -quiet '/Volumes/Unmount Test'; true"
fi

# mount notifications: NSWorkspace's DidMount, WillUnmount and DidUnmount, in macOS's order, on the main thread,
# for attach, detach, our own eject and a dissented eject (see nsworkspace-mount-notifications.m)
if build nsworkspace-mount-notifications "$ext/cocotron/tests/nsworkspace-mount-notifications.m" -framework AppKit \
	&& build da-ops "$here/da-ops.c" -framework DiskArbitration -framework CoreFoundation
then
	cp "$here/fixtures/slash-name.dmg" "$work/"
	guest notes "'$gw/nsworkspace-mount-notifications' '$gw/unmount-test.dmg' '/Volumes/Unmount Test' '$gw/da-ops'"
	guest notes-slash "'$gw/nsworkspace-mount-notifications' '$gw/slash-name.dmg' '/Volumes/A:B'"
	guest notes-cleanup "test -e '/Volumes/Unmount Test' && hdiutil detach -quiet '/Volumes/Unmount Test'; test -e '/Volumes/A:B' && hdiutil detach -quiet '/Volumes/A:B'; true"
	# an eject that took less than 5 s is written "elapsed=<5"
	normalize() { sed -E 's/elapsed=([0-4])\.[0-9]/elapsed=<5/' "$work/$1.out"; }
	u="/Volumes/Unmount Test Unmount Test url=1 object=1 main=1"
	cat >"$work/notes.expected" <<EXPECTED
same workspace on another thread=1
step 1
DidMount $u
step 2
WillUnmount $u
DidUnmount $u
step 3
DidMount $u
WillUnmount $u
DidUnmount $u
eject: 1 elapsed=<5 posted-before-return=1
step 4
DidMount $u
WillUnmount $u
dissented eject: 0 elapsed=<5 posted-before-return=0
WillUnmount $u
DidUnmount $u
eject: 1 elapsed=<5 posted-before-return=1
EXPECTED
	if normalize notes | diff "$work/notes.expected" - >"$work/notes.diff" && [ "$(rc notes)" = 0 ]; then
		pass "mount notifications"
	else
		fail "mount notifications: $(tr '\n' '|' <"$work/notes.diff") $(cat "$work/notes.err")"
	fi
	a="/Volumes/A:B A/B url=1 object=1 main=1"
	printf 'same workspace on another thread=1\nstep 1\nDidMount %s\nstep 2\nWillUnmount %s\nDidUnmount %s\n' "$a" "$a" "$a" >"$work/notes-slash.expected"
	if diff "$work/notes-slash.expected" "$work/notes-slash.out" >"$work/notes-slash.diff" && [ "$(rc notes-slash)" = 0 ]; then
		pass "mount notifications, a name with a slash"
	else
		fail "mount notifications, a name with a slash: $(tr '\n' '|' <"$work/notes-slash.diff") $(cat "$work/notes-slash.err")"
	fi
fi

# icon kinds: icons for files, folders, disk images, bundles without an icon and volumes (see nsworkspace-icon-kinds.m),
# and the existing iconForFile: and iconForFileType: checks
if build nsworkspace-icon-kinds "$ext/cocotron/tests/nsworkspace-icon-kinds.m" -framework AppKit -framework UniformTypeIdentifiers; then
	attach
	guest icons "'$gw/nsworkspace-icon-kinds' '/Volumes/Unmount Test'"
	detach
	[ "$(rc icons)" = 0 ] && pass "icon kinds" || fail "icon kinds: $(outputs icons)"
fi
if build nsworkspace-iconforfile "$ext/cocotron/tests/nsworkspace-iconforfile.m" -framework AppKit; then
	guest iconforfile "'$gw/nsworkspace-iconforfile' strict"
	[ "$(rc iconforfile)" = 0 ] && pass "iconforfile strict" || fail "iconforfile strict: $(outputs iconforfile)"
fi
if build appkit-workspace-type-icons "$ext/cocotron/tests/appkit-workspace-type-icons.m" -framework AppKit; then
	guest typeicons "'$gw/appkit-workspace-type-icons'"
	[ "$(rc typeicons)" = 0 ] && pass "type icons" || fail "type icons: $(outputs typeicons)"
fi

# recycle: recycleURLs:completionHandler: (see nsworkspace-recycle.m)
if build nsworkspace-recycle "$ext/cocotron/tests/nsworkspace-recycle.m" -framework AppKit; then
	guest recycle "(d=\$(mktemp -d \"\$HOME/recycle-test.XXXXXX\") || exit 1; '$gw/nsworkspace-recycle' \"\$d\"; r=\$?; rm -rf \"\$d\"; exit \$r)"
	[ "$(rc recycle)" = 0 ] && pass "recycle" || fail "recycle: $(outputs recycle)"
fi

# make_app <dir> <name> <bundle ID>: an application bundle whose executable is ae-receiver (built before)
make_app() {
	mkdir -p "$1/$2.app/Contents/MacOS"
	cp "$work/ae-receiver" "$1/$2.app/Contents/MacOS/ae-receiver"
	cat >"$1/$2.app/Contents/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleExecutable</key><string>ae-receiver</string>
	<key>CFBundleIdentifier</key><string>$3</string>
	<key>CFBundleName</key><string>$2</string>
	<key>CFBundlePackageType</key><string>APPL</string>
</dict>
</plist>
PLIST
}

# LSOpenApplication: launching an application with an initial Apple Event (see launchservices_open_application.m)
if build ae-receiver "$here/src/ae-receiver.m" -framework AppKit \
	&& build launchservices_open_application "$here/src/launchservices_open_application.m" -framework Foundation -framework CoreServices
then
	mkdir "$work/ls"
	make_app "$work/ls" Receiver org.darlinghq.test.ae-receiver
	guest lsopen "'$gw/launchservices_open_application' '$gw/ls'"
	[ "$(rc lsopen)" = 0 ] && pass "LSOpenApplication" || fail "LSOpenApplication: $(outputs lsopen) events: $(tr '\n' '|' <"$work/ls/events.txt" 2>/dev/null)"
	rm -rf "$work/ls"
fi

# reveal: selectFile:inFileViewerRootedAtPath: and activateFileViewerSelectingURLs: (see nsworkspace-reveal.m), with
# receivers standing in for Finder (com.apple.finder) and for an NSFileViewer app; removed afterwards
if [ -x "$work/ae-receiver" ] && build nsworkspace-reveal "$ext/cocotron/tests/nsworkspace-reveal.m" -framework AppKit -framework CoreServices; then
	mkdir -p "$work/rev/finder" "$work/rev/viewer"
	make_app "$work/rev/finder" Finder com.apple.finder
	make_app "$work/rev/viewer" Receiver org.darlinghq.test.ae-receiver
	guest reveal "'$gw/nsworkspace-reveal' '$gw/rev'"
	[ "$(rc reveal)" = 0 ] && pass "reveal" || fail "reveal: $(outputs reveal)"
	rm -rf "$work/rev"
fi

rm -rf "$work"
[ "$failures" = 0 ]
