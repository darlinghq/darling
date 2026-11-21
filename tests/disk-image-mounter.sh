#!/bin/bash
# DiskImageMounter.app: opening disk images with LaunchServices (open in the guest), with a stand-in for Finder
# (com.apple.finder, with Finder's folder and volume document types at rank Owner, so that it wins over an installed
# Finder) that records the open event it gets for the volume. See disk-image-mounter.md.
# Needs DARLING_SKILLS (a darling-skills checkout), DARLING_ROOT (the installed tree), and a display of your own,
# exported before the Darling container starts (it keeps the environment it started with; run `darling shutdown`
# first): X11 with DISPLAY (and xdotool), or Wayland with WAYLAND_DISPLAY, XDG_RUNTIME_DIR, SWAYSOCK and
# DARLING_APPKIT_BACKEND=wayland (and swaymsg, wtype). The test types keys into DiskImageMounter's windows.
# Prints PASS/FAIL lines and exits non-zero if anything failed.
set -u

here=$(cd "$(dirname "$0")" && pwd)
work=$(mktemp -d)
gw="/Volumes/SystemRoot$work"
failures=0
kb=
trap 'kill $kb 2>/dev/null; rm -rf "$work"' EXIT

pass() { echo "PASS: $1"; }
fail() { echo "FAIL: $1"; failures=$((failures + 1)); }

if [ -n "${WAYLAND_DISPLAY:-}" ] && [ "${DARLING_APPKIT_BACKEND:-}" = wayland ]; then
	display=wayland
	# A long-lived virtual keyboard first: a transient one leaves the seat without the keyboard capability.
	wtype -k Shift_L -s 600000 -k Shift_L &
	kb=$!
elif [ -n "${DISPLAY:-}" ]; then
	display=x11
else
	echo "set DISPLAY, or WAYLAND_DISPLAY and DARLING_APPKIT_BACKEND=wayland, to a display of your own" >&2
	exit 2
fi

# runs a shell command line in the guest; stdout, stderr and exit status go to $work/<name>.{out,err,rc}
guest() {
	name=$1
	shift
	timeout -s KILL 300 darling shell /bin/sh -c "{ $1; } >'$gw/$name.out' 2>'$gw/$name.err'; echo \$? >'$gw/$name.rc'" </dev/null >/dev/null 2>&1
}
rc() { cat "$work/$1.rc" 2>/dev/null; }
outputs() { cat "$work/$1.out" "$work/$1.err" 2>/dev/null | tr '\n' '|'; }

build() { # <program> <source> [<guest-cc arguments>...]
	program=$1
	source=$2
	shift 2
	"$DARLING_SKILLS/darling-guest-testing/scripts/guest-cc.sh" -o "$work/$program" "$source" "$@" \
		|| { fail "$program does not build"; return 1; }
}

dim="$here/../src/DiskImageMounter"
dmg=(-I"$here/../src/external/darling-dmg/include" -L"$DARLING_ROOT/usr/lib/darling" -ldarling-dmg)
for f in unmount-test sla-test sla-one sla-rtf sla-nostrings bad-blkx; do cp "$here/fixtures/$f.dmg" "$work/"; done
echo "This is not a disk image." > "$work/not-an-image.dmg"
# Spaces, & and non-ASCII characters in the path. The names are precomposed, as Linux names usually are; Foundation
# hands the kernel decomposed names, which the lookup finds as APFS does (tests/path-normalization.md).
mkdir -p "$work/Dir With Späce"
cp "$here/fixtures/unmount-test.dmg" "$work/Dir With Späce/ünï & co.dmg"

# The volume of the fixtures, and whether DiskImageMounter runs; both in the guest.
volume="/Volumes/Unmount Test"
mounts() { # <name>: the number of mounted "Unmount Test" volumes into $work/<name>.out
	guest "$1" "for d in '$volume'*; do [ -d \"\$d\" ] && [ \"\$(stat -f %d \"\$d\")\" != \"\$(stat -f %d /Volumes)\" ] && echo \"\$d\"; done | wc -l | tr -d ' '"
	cat "$work/$1.out"
}
running() { guest running "ps ax | grep -c '[D]iskImageMounter.app'"; cat "$work/running.out"; }
detach_all() { guest detach-all "for d in '$volume'*; do hdiutil detach -quiet \"\$d\"; done; true"; }
wait_gone() { # <seconds>: until DiskImageMounter is not running; 0 if it went
	local i
	for ((i = 0; i < $1; i++)); do [ "$(running)" = 0 ] && return 0; sleep 1; done
	return 1
}
events() { cat "$work/finder/events.txt" 2>/dev/null; }
odocs() { # <URL>: how many open-documents events the stand-in Finder got for that URL (with 'aapd' or not)
	events | awk -v u="$1" '$1 == "aevt" && $2 == "odoc" && $NF == u && NF <= 4' | wc -l
}
wait_event() { # <line> <seconds>: until the stand-in Finder has recorded that line
	local i
	for ((i = 0; i < $2; i++)); do [ "$(odocs "$1")" -ge 1 ] && return 0; sleep 1; done
	return 1
}

# The window of DiskImageMounter, by title (empty: any); waits up to $2 seconds. 0 if there is one.
window() {
	local i
	for ((i = 0; i < $2; i++)); do
		if [ $display = x11 ]; then
			WID=$(xdotool search --onlyvisible --class DiskImageMounter 2>/dev/null | while read -r w; do
				[ -z "$1" ] || [ "$(xdotool getwindowname "$w")" = "$1" ] && echo "$w"; done | tail -1)
		else
			WID=$(swaymsg -t get_tree | python3 -I -c '
import json, sys
title = sys.argv[1]
def walk(n):
    if n.get("app_id") == "DiskImageMounter" and (not title or n.get("name") == title):
        print(n["id"])
    for c in n.get("nodes", []) + n.get("floating_nodes", []):
        walk(c)
walk(json.load(sys.stdin))' "$1" | tail -1)
		fi
		[ -n "$WID" ] && return 0
		sleep 1
	done
	return 1
}
windows() { # <title>: how many windows of DiskImageMounter have it
	if [ $display = x11 ]; then
		xdotool search --onlyvisible --class DiskImageMounter 2>/dev/null | while read -r w; do
			[ "$(xdotool getwindowname "$w")" = "$1" ] && echo "$w"; done | wc -l
	else
		swaymsg -t get_tree | grep -c "\"name\": \"$1\""
	fi
}
key() { # <key>: to the window window() found, once it has had time to become key
	sleep 2
	if [ $display = x11 ]; then
		xdotool windowactivate --sync "$WID" 2>/dev/null || xdotool windowfocus "$WID"
		sleep 0.5
		xdotool key "$1"
	else
		swaymsg -q "[con_id=$WID] focus"
		sleep 0.5
		wtype -k "$1"
	fi
	sleep 1
}

# make_app <dir> <name> <bundle ID> [<more Info.plist keys>]: an application bundle whose executable is ae-receiver
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
${4:-}
</dict>
</plist>
PLIST
}
# The stand-in's rank is Owner, so it is the volumes' handler even where the real Finder (rank Default) is installed
FINDER_TYPES='<key>CFBundleDocumentTypes</key><array>
	<dict><key>CFBundleTypeName</key><string>Folder</string><key>CFBundleTypeOSTypes</key><array><string>fold</string></array><key>CFBundleTypeRole</key><string>Editor</string><key>LSHandlerRank</key><string>Owner</string></dict>
	<dict><key>CFBundleTypeName</key><string>Volume</string><key>CFBundleTypeOSTypes</key><array><string>disk</string></array><key>CFBundleTypeRole</key><string>Editor</string><key>LSHandlerRank</key><string>Owner</string></dict>
</array>'

# opener: DIMImageOpener in process (see src/dim_image_opener.m)
if build dim_image_opener "$here/src/dim_image_opener.m" "$dim/DIMImageOpener.m" -I"$dim" -fobjc-arc \
	-framework AppKit "${dmg[@]}"; then
	guest opener "'$gw/dim_image_opener' '$gw'"
	[ "$(rc opener)" = 0 ] && pass "opener" || fail "opener: $(outputs opener)"
	detach_all
fi

# license window (in process): DIMLicenseWindowController (see src/dim_license_window.m)
if build dim_license_window "$here/src/dim_license_window.m" "$dim/DIMLicenseWindowController.m" -I"$dim" \
	-fobjc-arc -framework AppKit "${dmg[@]}"; then
	guest license-window "'$gw/dim_license_window' '$gw'"
	[ "$(rc license-window)" = 0 ] && pass "license window (in process)" \
		|| fail "license window (in process): $(outputs license-window)"
fi

# The stand-in Finder, registered with LaunchServices; removed at the end
if build ae-receiver "$here/src/ae-receiver.m" -framework AppKit \
	&& build ls_register "$here/src/ls_register.m" -framework Foundation -framework CoreServices; then
	make_app "$work" Finder com.apple.finder "$FINDER_TYPES"
	mv "$work/Finder.app" "$work/finder.tmp" && mkdir "$work/finder" && mv "$work/finder.tmp" "$work/finder/Finder.app"
	guest register "'$gw/ls_register' '$gw/finder/Finder.app'"
	[ "$(rc register)" = 0 ] || fail "the stand-in Finder cannot be registered: $(outputs register)"
fi
odoc="file:///Volumes/Unmount%20Test/"

# open: an image without a licence agreement is attached and its volume opened; DiskImageMounter quits
detach_all
guest open "open '$gw/unmount-test.dmg'"
if wait_event "$odoc" 60 && wait_gone 15 && [ "$(mounts open-mounts)" = 1 ]; then
	pass "open"
else
	fail "open: rc $(rc open) $(outputs open), events: $(events | tr '\n' '|'), running $(running), mounts $(mounts open-mounts)"
fi

# open twice: the attached image is opened again, not attached again
guest open2 "open '$gw/unmount-test.dmg'"
for ((i = 0; i < 60; i++)); do [ "$(odocs "$odoc")" -ge 2 ] && break; sleep 1; done
if [ "$(odocs "$odoc")" -ge 2 ] && wait_gone 15 && [ "$(mounts open2-mounts)" = 1 ]; then
	pass "open twice"
else
	fail "open twice: events: $(events | tr '\n' '|'), mounts $(mounts open2-mounts)"
fi
detach_all

# odd path: spaces, & and non-ASCII characters in the image's path
: > "$work/finder/events.txt"
guest odd "open '$gw/Dir With Späce/ünï & co.dmg'"
if wait_event "$odoc" 60 && wait_gone 15; then pass "odd path"; else fail "odd path: $(outputs odd), events: $(events | tr '\n' '|')"; fi
detach_all

# bad image: an error alert; after Return nothing is mounted and DiskImageMounter quits
: > "$work/finder/events.txt"
guest bad "open '$gw/bad-blkx.dmg'"
if window "" 60; then
	key Return
	if wait_gone 15 && [ "$(mounts bad-mounts)" = 0 ] && [ -z "$(events)" ]; then
		pass "bad image"
	else
		fail "bad image: running $(running), mounts $(mounts bad-mounts), events: $(events | tr '\n' '|')"
	fi
else
	fail "bad image: no window; $(outputs bad)"
fi

# license window: Escape declines (nothing mounted, DiskImageMounter quits); opened again, Return agrees
detach_all
: > "$work/finder/events.txt"
guest lw1 "open '$gw/sla-test.dmg'"
if window sla-test.dmg 60; then
	key Escape
	if wait_gone 15 && [ "$(mounts lw1-mounts)" = 0 ] && [ -z "$(events)" ]; then
		guest lw2 "open '$gw/sla-test.dmg'"
		if window sla-test.dmg 60; then
			key Return
			if wait_event "$odoc" 60 && wait_gone 15 && [ "$(mounts lw2-mounts)" = 1 ]; then
				pass "license window"
			else
				fail "license window: after Return: events: $(events | tr '\n' '|'), running $(running), mounts $(mounts lw2-mounts)"
			fi
		else
			fail "license window: no window when opened again; $(outputs lw2)"
		fi
	else
		fail "license window: after Escape: running $(running), mounts $(mounts lw1-mounts), events: $(events | tr '\n' '|')"
	fi
else
	fail "license window: no window titled sla-test.dmg; $(outputs lw1)"
fi
detach_all

# license twice: two opens of a licence image, one second apart, before answering: one window; after Return, one mount
: > "$work/finder/events.txt"
guest lt1 "open '$gw/sla-test.dmg'"
sleep 1
guest lt2 "open '$gw/sla-test.dmg'"
if window sla-test.dmg 60; then
	sleep 3
	n=$(windows sla-test.dmg)
	key Return
	if [ "$n" = 1 ] && wait_event "$odoc" 60 && wait_gone 15 && [ "$(mounts lt-mounts)" = 1 ]; then
		pass "license twice"
	else
		fail "license twice: $n windows, events: $(events | tr '\n' '|'), running $(running), mounts $(mounts lt-mounts)"
	fi
else
	fail "license twice: no window titled sla-test.dmg; $(outputs lt1)"
fi
detach_all

# kill during license: DiskImageMounter killed while the agreement shows; nothing is mounted
guest kl "open '$gw/sla-test.dmg'"
if window sla-test.dmg 60; then
	guest kill-dim1 "kill \$(ps ax | grep '[D]iskImageMounter.app' | awk '{print \$1}')"
	if wait_gone 15 && [ "$(mounts kl-mounts)" = 0 ]; then
		pass "kill during license"
	else
		fail "kill during license: running $(running), mounts $(mounts kl-mounts)"
	fi
else
	fail "kill during license: no window titled sla-test.dmg; $(outputs kl)"
fi
detach_all

# attached license image: attached already (agreed to with hdiutil), opened without asking; the volume is opened
: > "$work/finder/events.txt"
guest al-attach "printf 'y\n' | hdiutil attach '$gw/sla-test.dmg'"
guest al "open '$gw/sla-test.dmg'"
if wait_event "$odoc" 60 && ! window sla-test.dmg 1 && wait_gone 15 && [ "$(mounts al-mounts)" = 1 ]; then
	pass "attached license image"
else
	fail "attached license image: attach rc $(rc al-attach) $(outputs al-attach), events: $(events | tr '\n' '|'), running $(running), mounts $(mounts al-mounts)"
fi
detach_all

guest kill-dim "kill \$(ps ax | grep '[D]iskImageMounter.app' | awk '{print \$1}') 2>/dev/null; true"
detach_all
rm -rf "$work/finder"
[ "$failures" = 0 ]
