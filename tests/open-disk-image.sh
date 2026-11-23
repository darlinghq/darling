#!/bin/bash
# Opening a disk image from the Linux desktop, end to end: the installed desktop entry (darling open %F) opens
# styled-sla-test.dmg; DiskImageMounter asks for its licence agreement, mounts it and has Finder open its styled
# window; an application is dragged out of it into a folder; Command-E ejects it. Also `darling open` on the command
# line. See open-disk-image.md.
# Runs `darling` from PATH, so the caller picks the launcher mode (the setuid darling for the root mode; a non-setuid
# copy with DARLING_ROOTLESS=1 or DARLING_NONROOT=1), and stops the container first to start it as the desktop would.
# Needs DARLING_SKILLS, DARLING_ROOT (the installed tree), gio, setsid, and a display of your own as finder.sh does:
# X11 with DISPLAY (and xdotool), or Wayland with WAYLAND_DISPLAY, XDG_RUNTIME_DIR, SWAYSOCK and
# DARLING_APPKIT_BACKEND=wayland (and swaymsg, wtype; VIRTUAL_POINTER for the drag, skipped without it).
# Prints PASS/FAIL lines and exits non-zero if anything failed.
set -u

here=$(cd "$(dirname "$0")" && pwd)
. "$here/finder-lib.sh"

id=org.darlinghq.Darling.DiskImageMounter
prefix=$(cd "$DARLING_ROOT/../.." && pwd) # DARLING_ROOT is <install prefix>/libexec/darling
installed="$prefix/share/applications/$id.desktop"
launcher=$(command -v darling)
styled="/Volumes/Styled Test"

# runs the launcher on the host with these arguments; stdout, stderr and exit status in $work/<name>.{out,err,rc}
host() {
	local name=$1
	shift
	timeout -s KILL 300 darling "$@" >"$work/$name.out" 2>"$work/$name.err" </dev/null
	echo $? >"$work/$name.rc"
}
mounts() { # <volume path>: how many times it is mounted in the guest
	guest mounts "df | grep -c ' $1\$'; true"
	cat "$work/mounts.out"
}

# The image, at a path a shell would have to quote
mkdir -p "$work/Dir With Späce" "$work/dest" "$work/rel" "$work/rel2"
image="$work/Dir With Späce/Ünï & 'q' \$x.dmg"
cp "$here/fixtures/styled-sla-test.dmg" "$image"
cp "$here/fixtures/unmount-test.dmg" "$work/"
ln -s "$work/unmount-test.dmg" "$work/link.dmg"

# desktop entry: installed, naming the installed launcher; a copy naming the launcher on PATH is what this run uses
# (the installed one is setuid root, which would start the root mode whatever mode this run is in)
exec_line=$(sed -n 's/^Exec=//p' "$installed" 2>/dev/null)
if [ "$exec_line" = "\"$prefix/bin/darling\" open %F" ]; then
	pass "desktop entry"
else
	fail "desktop entry: $installed: Exec=$exec_line"
fi
sed "s|^Exec=.*|Exec=\"$launcher\" open %F|" "$installed" >"$work/$id.desktop" 2>/dev/null

# cold start: with the container stopped and no terminal, the entry launched on the image shows the licence agreement
timeout -s KILL 120 darling shutdown </dev/null >/dev/null 2>&1
setsid -w gio launch "$work/$id.desktop" "$image" </dev/null >"$work/launch.out" 2>&1 &
app=DiskImageMounter
if window "$(basename "$image")" 180; then
	pass "cold start"
	# agreement: Return agrees; the volume is mounted and Finder shows its window
	key Return
	app=Finder
	if window "Styled Test" 60 && [ "$(mounts "$styled")" = 1 ]; then
		pass "agreement"
	else
		fail "agreement: windows: $(windows ''), mounts $(mounts "$styled")"
	fi
else
	fail "cold start: no licence window; launch: $(tr '\n' '|' <"$work/launch.out")"
	app=Finder
fi

# window layout: 640 wide, 400 high plus at most a title bar, its background picture blue near the bottom left
if window "Styled Test" 5; then
	sleep 2
	read -r width height <<<"$(client_size)"
	shot styled
	read -r iw ih <<<"$(identify -format '%w %h' "$work/styled.png" 2>/dev/null)"
	corner=$(pixel styled 20 $((ih - 20)))
	if [ "$width" != 640 ] || [ "$height" -lt 400 ] || [ "$height" -gt 440 ]; then
		fail "window layout: client area ${width}x${height}, want 640x400 to 640x440"
	elif ! near "$corner" "47 111 159" 8; then
		fail "window layout: the background near the bottom left is ($corner), want (47 111 159) (image ${iw}x${ih})"
	else
		pass "window layout"
	fi
else
	fail "window layout: no window"
fi

# copy: Styled App.app dragged from the styled window into a folder opened with darling open is copied there
host open-dest open "$work/dest"
if [ "$pointer_ok" != 1 ]; then
	skip "copy: no pointer (set VIRTUAL_POINTER)"
elif window dest 30 && place 1060 60 && read -r tx ty <<<"$(at 500 200)" && window "Styled Test" 10; then
	place 60 60
	read -r sx sy <<<"$(at 160 200)"
	drag $sx $sy $tx $ty
	waitfor 20 "$work/dest/Styled App.app/Contents/Info.plist"
	sleep 2
	guest diff-styled "diff -r '$styled/Styled App.app' '$gw/dest/Styled App.app'"
	if [ "$(rc diff-styled)" = 0 ]; then pass "copy"; else fail "copy: $(ls "$work/dest" | tr '\n' ' ') $(outputs diff-styled)"; fi
else
	fail "copy: no windows: $(cat "$work/open-dest.err"), windows: $(windows '')"
fi

# eject: Command-E in the volume's window ejects it; the window closes
if window "Styled Test" 5; then
	key cmd+e
	if window_gone "Styled Test" 15 && [ "$(mounts "$styled")" = 0 ]; then
		pass "eject"
	else
		fail "eject: windows $(windows "Styled Test"), mounts $(mounts "$styled")"
	fi
else
	fail "eject: no window"
fi

# two images: the entry launched on two images at once (as a file manager does for a selection) mounts both, the one
# with a licence agreement after it is agreed to, and each volume gets its window
setsid -w gio launch "$work/$id.desktop" "$image" "$work/unmount-test.dmg" </dev/null >"$work/launch2.out" 2>&1 &
app=DiskImageMounter
if window "$(basename "$image")" 60; then
	key Return
	app=Finder
	if window "Styled Test" 60 && window "Unmount Test" 60 && [ "$(mounts "$styled")" = 1 ] \
		&& [ "$(mounts "/Volumes/Unmount Test")" = 1 ]; then
		pass "two images"
	else
		fail "two images: windows: $(windows ''), mounts $(mounts "$styled") $(mounts "/Volumes/Unmount Test")"
	fi
else
	app=Finder
	fail "two images: no licence window; launch: $(tr '\n' '|' <"$work/launch2.out"), windows: $(windows '')"
fi

# command line: a relative path; a symbolic link to an attached image (opened again, not attached again); a missing
# file and no file at all are errors (exit status 1) that open nothing
(cd "$work" && host rel2 open rel2)
window rel2 30 && rel_ok=1 || rel_ok=0
host link open "$work/link.dmg"
sleep 5
link_mounts=$(mounts "/Volumes/Unmount Test")
host missing open "$work/missing.dmg"
host none open
if [ $rel_ok != 1 ]; then
	fail "command line: relative path: rc $(cat "$work/rel2.rc") $(cat "$work/rel2.err"), windows: $(windows '')"
elif [ "$(cat "$work/link.rc")" != 0 ] || [ "$link_mounts" != 1 ] || [ "$(windows "Unmount Test")" != 1 ]; then
	fail "command line: link: rc $(cat "$work/link.rc") $(cat "$work/link.err"), mounts $link_mounts, windows $(windows "Unmount Test")"
elif [ "$(cat "$work/missing.rc")" != 1 ] || [ "$(cat "$work/missing.err")" != "The file $work/missing.dmg does not exist." ]; then
	fail "command line: missing: rc $(cat "$work/missing.rc") $(cat "$work/missing.err")"
elif [ "$(cat "$work/none.rc")" != 1 ] || [ "$(cat "$work/none.err")" != "Usage: darling open <file>..." ]; then
	fail "command line: no file: rc $(cat "$work/none.rc") $(cat "$work/none.err")"
else
	pass "command line"
fi

quit_finder
guest detach-end "hdiutil detach -quiet '/Volumes/Unmount Test'; hdiutil detach -quiet '$styled'; true"

echo "$failures failure(s)"
[ $failures = 0 ]
