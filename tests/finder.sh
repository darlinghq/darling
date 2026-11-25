#!/bin/bash
# Finder.app: opening folders and volumes with LaunchServices (open in the guest), its windows, and quitting after the
# last one closes. See finder.md.
# Needs DARLING_SKILLS (a darling-skills checkout), DARLING_ROOT (the installed tree), and a display of your own,
# exported before the Darling container starts (it keeps the environment it started with; run `darling shutdown`
# first): X11 with DISPLAY (and xdotool), or Wayland with WAYLAND_DISPLAY, XDG_RUNTIME_DIR, SWAYSOCK and
# DARLING_APPKIT_BACKEND=wayland (and swaymsg, wtype). The test types keys into Finder's windows and drags its items
# with the pointer: on Wayland, set VIRTUAL_POINTER to a build of src/external/cocotron/tests/wayland-virtual-pointer.c
# (the drags are skipped without it).
# Prints PASS/FAIL lines and exits non-zero if anything failed.
set -u

here=$(cd "$(dirname "$0")" && pwd)
. "$here/finder-lib.sh"

# The folders
mkdir -p "$work/F/sub" "$work/G"
touch "$work/F/one.txt" "$work/F/two.txt" "$work/G/three.txt"
mkdir -p "$work/locked"
chmod 000 "$work/locked"
cp "$here/fixtures/unmount-test.dmg" "$work/"
# Finder runs without HOME when LaunchServices launches it, so its home folder is the user database's
guest whoami "id -u; awk -F: -v u=\$(id -u) '\$3 == u { print \$6 }' /etc/passwd"
guest_uid=$(head -1 "$work/whoami.out")
guest_home=$(sed -n 2p "$work/whoami.out")
home_name=$(basename "$guest_home")
quit_finder
detach_all

# open folder: a window titled with the folder's name
guest open-f "open '$gw/F'"
if [ "$(rc open-f)" = 0 ] && window F 30; then pass "open folder"; else fail "open folder: rc $(rc open-f) $(outputs open-f)"; fi

# open twice: still one window for the folder
guest open-f2 "open '$gw/F'"
sleep 3
n=$(windows F)
if [ "$n" = 1 ]; then pass "open twice"; else fail "open twice: $n windows titled F"; fi

# views: Command-2 shows the list, Command-1 the icons again; the window stays, titled F (the in-process tests check
# what each view shows)
if window F 5; then
	sleep 2
	shot icons
	key cmd+2
	sleep 1
	shot list
	key cmd+1
	sleep 1
	shot icons-again
	if ! differ icons list; then
		fail "views: the window looks the same after Command-2"
	elif ! differ list icons-again; then
		fail "views: the window looks the same after Command-1"
	elif [ "$(windows F)" != 1 ]; then
		fail "views: $(windows F) windows titled F"
	else
		pass "views"
	fi
else
	fail "views: no window F"
fi

# quit: closing the last window (Command-W) ends Finder
if window F 5; then
	key cmd+w
	if window_gone F 10 && wait_gone 10; then pass "quit"; else fail "quit: windows $(windows F), running $(running)"; fi
else
	fail "quit: no window F"
fi
quit_finder

# launch: launched without documents, Finder shows the home folder
guest launch "open -a Finder"
if [ "$(rc launch)" = 0 ] && window "$home_name" 30; then pass "launch"; else fail "launch: rc $(rc launch) $(outputs launch), windows: $(windows '')"; fi
quit_finder

# volume: DiskImageMounter opens the image's volume, which LaunchServices hands to Finder
guest open-dmg "open '$gw/unmount-test.dmg'"
if window "Unmount Test" 60; then pass "volume"; else fail "volume: $(outputs open-dmg), windows: $(windows '')"; fi
quit_finder
detach_all

# (the guest has no mount command; df ends each line with the mount point)
mounted() { guest mounted "df | grep -c ' $volume\$'"; cat "$work/mounted.out"; }

# eject: Command-E in the volume's window ejects it; the window closes, and Finder's other windows stay
guest open-f3 "open '$gw/F'"
window F 30
guest open-dmg2 "open '$gw/unmount-test.dmg'"
if window "Unmount Test" 60; then
	key cmd+e
	if ! window_gone "Unmount Test" 15 || [ "$(mounted)" != 0 ]; then
		fail "eject: windows $(windows 'Unmount Test'), mounted $(mounted)"
	elif sleep 2 && [ "$(windows F)" != 1 ]; then
		fail "eject: the window of F went too (Finder running: $(running))"
	else
		pass "eject"
	fi
else
	fail "eject: no window: $(outputs open-dmg2)"
fi
quit_finder
detach_all

# eject button: the eject button of the volume in the sidebar (Locations' second row, 297 points above the bottom
# of a new window) ejects it; the window closes, and Finder's other windows stay
if [ $display = wayland ]; then
	skip "eject button: a headless sway has no pointer to click with (Command-E covers ejecting on Wayland)"
else
guest open-f4 "open '$gw/F'"
window F 30
guest open-dmg4 "open '$gw/unmount-test.dmg'"
if window "Unmount Test" 60; then
	sleep 2
	click 176 297
	if ! window_gone "Unmount Test" 15 || [ "$(mounted)" != 0 ]; then
		fail "eject button: windows $(windows 'Unmount Test'), mounted $(mounted)"
	elif sleep 2 && [ "$(windows F)" != 1 ]; then
		fail "eject button: the window of F went too (Finder running: $(running))"
	else
		pass "eject button"
	fi
else
	fail "eject button: no window: $(outputs open-dmg4)"
fi
quit_finder
detach_all
fi

# detached outside: the volume's window closes when something else detaches it
guest open-dmg3 "open '$gw/unmount-test.dmg'"
if window "Unmount Test" 60; then
	guest detach "hdiutil detach '$volume'"
	if [ "$(rc detach)" != 0 ]; then
		fail "detached outside: hdiutil detach: rc $(rc detach) $(outputs detach)"
	elif window_gone "Unmount Test" 10; then
		pass "detached outside"
	else
		fail "detached outside: the window stays"
	fi
else
	fail "detached outside: no window: $(outputs open-dmg3)"
fi
quit_finder
detach_all

# reveal: open -R shows the file's folder (the in-process test, finder_reveal.m, checks the selection)
guest reveal "open -R '$gw/F/one.txt'"
if [ "$(rc reveal)" = 0 ] && window F 30; then pass "reveal"; else fail "reveal: rc $(rc reveal) $(outputs reveal)"; fi
quit_finder

# reveal several: files in two folders give a window for each
guest reveal2 "open -R '$gw/F/one.txt' '$gw/G/three.txt'"
if [ "$(rc reveal2)" = 0 ] && window F 30 && window G 10; then
	pass "reveal several"
else
	fail "reveal several: rc $(rc reveal2) $(outputs reveal2), windows F $(windows F), G $(windows G)"
fi
quit_finder

# open -R missing: a file that doesn't exist is an error (exit status 1, as on macOS), and nothing is revealed
guest reveal-missing "open -R '$gw/nope' '$gw/F/one.txt'"
sleep 5
if [ "$(rc reveal-missing)" = 1 ] && grep -q "does not exist" "$work/reveal-missing.err" && [ "$(windows '')" = 0 ]; then
	pass "open -R missing"
else
	fail "open -R missing: rc $(rc reveal-missing) $(outputs reveal-missing), windows $(windows '')"
fi
quit_finder

# open file: Command-Down opens the selected file with its application (a stand-in editor for .fndtest files,
# which records the Apple Events it gets)
if build ae-receiver "$here/src/ae-receiver.m" -framework AppKit \
	&& build ls_register "$here/src/ls_register.m" -framework Foundation -framework CoreServices; then
	mkdir -p "$work/editor/Editor.app/Contents/MacOS"
	cp "$work/ae-receiver" "$work/editor/Editor.app/Contents/MacOS/ae-receiver"
	cat >"$work/editor/Editor.app/Contents/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleExecutable</key><string>ae-receiver</string>
	<key>CFBundleIdentifier</key><string>org.darling.test.finder-editor</string>
	<key>CFBundleName</key><string>Editor</string>
	<key>CFBundlePackageType</key><string>APPL</string>
	<key>CFBundleDocumentTypes</key><array><dict>
		<key>CFBundleTypeName</key><string>Finder test document</string>
		<key>CFBundleTypeExtensions</key><array><string>fndtest</string></array>
		<key>CFBundleTypeRole</key><string>Editor</string>
		<key>LSHandlerRank</key><string>Owner</string>
	</dict></array>
</dict>
</plist>
PLIST
	touch "$work/F/doc.fndtest"
	guest register-editor "'$gw/ls_register' '$gw/editor/Editor.app'"
	guest reveal-doc "open -R '$gw/F/doc.fndtest'"
	if [ "$(rc register-editor)" != 0 ]; then
		fail "open file: the stand-in editor cannot be registered: $(outputs register-editor)"
	elif window F 30; then
		key cmd+Down
		for ((i = 0; i < 30; i++)); do grep -q "odoc.*doc.fndtest" "$work/editor/events.txt" 2>/dev/null && break; sleep 1; done
		if grep -q "^aevt odoc .*doc.fndtest" "$work/editor/events.txt" 2>/dev/null; then
			pass "open file"
		else
			fail "open file: the editor got: $(tr '\n' '|' < "$work/editor/events.txt" 2>/dev/null)"
		fi
	else
		fail "open file: no window F: $(outputs reveal-doc)"
	fi
	guest kill-editor "kill \$(ps ax | grep '[E]ditor.app/Contents/MacOS' | awk '{print \$1}') 2>/dev/null; true"
	rm -rf "$work/editor" "$work/F/doc.fndtest"
fi
quit_finder

# odd names: a folder named with accents, CJK and an emoji holds a file whose name has a newline and one whose name
# is 255 bytes long; the folder opens in its window, and revealing either file gives that window
odd="Ünï 日本 😀"
long=$(printf 'n%.0s' $(seq 251)).txt
mkdir -p "$work/$odd"
touch "$work/$odd/line
break.txt" "$work/$odd/$long"
guest open-odd "open '$gw/$odd'"
if window "$odd" 30; then pass "odd names: open"; else fail "odd names: open: $(outputs open-odd)"; fi
quit_finder
guest reveal-newline "open -R '$gw/$odd/line
break.txt'"
if [ "$(rc reveal-newline)" = 0 ] && window "$odd" 30; then
	pass "odd names: reveal a name with a newline"
else
	fail "odd names: reveal a name with a newline: rc $(rc reveal-newline) $(outputs reveal-newline)"
fi
quit_finder
guest reveal-long "open -R '$gw/$odd/$long'"
if [ "$(rc reveal-long)" = 0 ] && window "$odd" 30; then
	pass "odd names: reveal a 255-byte name"
else
	fail "odd names: reveal a 255-byte name: rc $(rc reveal-long) $(outputs reveal-long)"
fi
quit_finder

# large folder: a folder of 2000 files shows its window within 5 s of open returning; End (to the last item) is fine
mkdir -p "$work/Large"
(cd "$work/Large" && seq -f "item %04g.txt" 1 2000 | xargs -d '\n' touch)
guest open-large "open '$gw/Large'"
start=$(date +%s)
if window Large 30; then
	took=$(($(date +%s) - start))
	key End
	if [ $took -le 5 ] && [ "$(windows Large)" = 1 ]; then
		pass "large folder (${took} s)"
	else
		fail "large folder: the window took ${took} s; windows titled Large after End: $(windows Large)"
	fi
else
	fail "large folder: no window: $(outputs open-large)"
fi
quit_finder

# folder removed: a window whose folder is deleted (here from the host side) closes within 10 s
mkdir -p "$work/F2" "$work/F"
touch "$work/F2/x.txt"
guest open-f2 "open '$gw/F'; open '$gw/F2'"
if window F2 30 && window F 5; then
	rm -r "$work/F2"
	for ((i = 0; i < 10; i++)); do [ "$(windows F2)" = 0 ] && break; sleep 1; done
	if [ "$(windows F2)" = 0 ] && [ "$(windows F)" = 1 ]; then
		pass "folder removed"
	else
		fail "folder removed: windows F2 $(windows F2), F $(windows F) after 10 s"
	fi
else
	fail "folder removed: no window: $(outputs open-f2)"
fi
quit_finder


# styled window: the volume of styled-test.dmg (tests/fixtures/make-styled-fixture.py) opens in a window styled by its
# .DS_Store: 640 wide, 400 high plus at most a title bar, its background picture blue with a light band at (320, 200)
styled="/Volumes/Styled Test"
cp "$here/fixtures/styled-test.dmg" "$work/"
guest open-styled "open '$gw/styled-test.dmg'"
if window "Styled Test" 60; then
	sleep 2
	read -r width height <<<"$(client_size)"
	shot styled
	read -r iw ih <<<"$(identify -format '%w %h' "$work/styled.png" 2>/dev/null)"
	corner=$(pixel styled 20 $((ih - 20)))
	band=$(pixel styled 320 $((ih - 200)))
	if [ "$width" != 640 ] || [ "$height" -lt 400 ] || [ "$height" -gt 440 ]; then
		fail "styled window: client area ${width}x${height}, want 640x400 to 640x440"
	elif ! near "$corner" "47 111 159" 8; then
		fail "styled window: the background near the bottom left is ($corner), want (47 111 159) (image ${iw}x${ih})"
	elif ! python3 -I -c 'import sys; c = sys.argv[1].split(); sys.exit(not (len(c) == 3 and all(int(x) > 200 for x in c)))' "$band"; then
		fail "styled window: the band at (320, 200) is ($band), want light"
	else
		pass "styled window"
	fi
else
	fail "styled window: no window: $(outputs open-styled), windows: $(windows '')"
fi

# styled hidden item: Finder's folder model lists the volume's items, not the invisible one or the dot files
if build finder_ls "$here/src/finder_ls.m" -framework Foundation -fobjc-arc -I"$here/../src/Finder" \
	"$here/../src/Finder/FinderDirectory.m"; then
	guest ls-styled "'$gw/finder_ls' '$styled'"
	listed=$(tr '\n' '|' <"$work/ls-styled.out")
	if [ "$(rc ls-styled)" = 0 ] && grep -qx "Styled App.app" "$work/ls-styled.out" && grep -qx Applications "$work/ls-styled.out" \
		&& ! grep -q "Hidden Notes.txt\|^\.background$\|^\.DS_Store$" "$work/ls-styled.out"; then
		pass "styled hidden item"
	else
		fail "styled hidden item: listed $listed $(outputs ls-styled)"
	fi
fi
quit_finder
guest detach-styled "hdiutil detach -quiet '$styled'; true"

# unreadable: a folder that can't be read gives an alert, no window; after it Finder quits
if [ "$guest_uid" = 0 ]; then
	skip "unreadable: the guest runs as root, every folder is readable"
else
	guest open-locked "open '$gw/locked'"
	sleep 5
	all=$(windows '')
	n=$(windows locked)
	if [ "$all" -ge 1 ] && [ "$n" = 0 ] && window '' 1; then
		key Return
		if wait_gone 15; then pass "unreadable"; else fail "unreadable: Finder still runs after the alert"; fi
	else
		fail "unreadable: $all windows, $n titled locked"
	fi
fi
quit_finder


# File operations (see finder.md). Windows are opened at known places; an 800-point window's icons are found by the
# order of their names.
if [ $pointer_ok = 0 ]; then
	skip "file operations by drag: no pointer (set VIRTUAL_POINTER to a built tests/wayland-virtual-pointer.c)"
else

# option-drag copies within a volume: Option-dragging a onto F copies it, a plain drag of b.txt moves it
mkdir -p "$work/O/F"
echo a >"$work/O/a"
echo b >"$work/O/b.txt"
guest open-o "open '$gw/O'"
if window O 30; then
	place 60 60
	sleep 1
	read -r fx fy <<<"$(at "$(item_x 2)" $item_up)"
	read -r sx sy <<<"$(at "$(item_x 0)" $item_up)"
	drag $sx $sy $fx $fy option
	waitfor 15 "$work/O/F/a"
	read -r sx sy <<<"$(at "$(item_x 1)" $item_up)"
	drag $sx $sy $fx $fy
	waitfor 15 "$work/O/F/b.txt"
	if [ ! -e "$work/O/F/a" ] || [ ! -e "$work/O/a" ]; then
		fail "option-drag copies within a volume: F/a $([ -e "$work/O/F/a" ] && echo is there || echo missing), a $([ -e "$work/O/a" ] && echo remains || echo went)"
	elif [ ! -e "$work/O/F/b.txt" ] || [ -e "$work/O/b.txt" ]; then
		fail "option-drag copies within a volume: a plain drag: F/b.txt $([ -e "$work/O/F/b.txt" ] && echo is there || echo missing), b.txt $([ -e "$work/O/b.txt" ] && echo remains || echo went)"
	else
		pass "option-drag copies within a volume"
	fi
else
	fail "option-drag copies within a volume: no window: $(outputs open-o)"
fi
quit_finder

# drop on a folder link: Option-dragging a bundle onto a symbolic link to a folder copies it into that folder
mkdir -p "$work/kit/Kit.bundle/Contents/Resources" "$work/dest2"
echo plist >"$work/kit/Kit.bundle/Contents/Info.plist"
echo data >"$work/kit/Kit.bundle/Contents/Resources/data"
ln -s Info.plist "$work/kit/Kit.bundle/Contents/link"
ln -s ../dest2 "$work/kit/Install"
guest open-kit "open '$gw/kit'"
if window kit 30; then
	place 60 60
	read -r sx sy <<<"$(at "$(item_x 1)" $item_up)"
	read -r tx ty <<<"$(at "$(item_x 0)" $item_up)"
	drag $sx $sy $tx $ty option
	waitfor 15 "$work/dest2/Kit.bundle/Contents/link"
	sleep 1
	if diff -r --no-dereference "$work/kit/Kit.bundle" "$work/dest2/Kit.bundle" >"$work/kit.diff" 2>&1 && [ -d "$work/kit/Kit.bundle" ]; then
		pass "drop on a folder link"
	else
		fail "drop on a folder link: $(ls "$work/dest2" | tr '\n' ' ') $(head -3 "$work/kit.diff" | tr '\n' '|')"
	fi
else
	fail "drop on a folder link: no window: $(outputs open-kit)"
fi
quit_finder

# install from a disk image: the application dragged from the image's styled window into another window is copied
mkdir -p "$work/dest"
guest open-dest "open '$gw/dest'"
guest open-styled2 "open '$gw/styled-test.dmg'"
if window dest 30 && place 1060 60 && read -r tx ty <<<"$(at 500 200)" && window "Styled Test" 60; then
	place 60 60
	# Styled App.app is at (160, 200) of the 640x400 window, from its top left
	read -r sx sy <<<"$(at 160 200)"
	drag $sx $sy $tx $ty
	waitfor 20 "$work/dest/Styled App.app/Contents/Info.plist"
	sleep 2
	guest diff-styled "diff -r '$styled/Styled App.app' '$gw/dest/Styled App.app'"
	if [ "$(rc diff-styled)" = 0 ]; then
		guest detach-styled2 "hdiutil detach '$styled'"
		if [ "$(rc detach-styled2)" = 0 ]; then pass "install from a disk image"; else fail "install from a disk image: eject: $(outputs detach-styled2)"; fi
	else
		fail "install from a disk image: $(ls "$work/dest" | tr '\n' ' ') $(outputs diff-styled)"
	fi
else
	fail "install from a disk image: no windows: $(outputs open-dest) $(outputs open-styled2), windows: $(windows '')"
fi
quit_finder
guest detach-styled3 "hdiutil detach -quiet '$styled'; true"

# progress: copying a large file shows a Copy window; its Stop ends the copy and leaves no file. The file (4 GiB,
# written past the page cache) is read from the disk, so the copy outlasts the window's delay (1 s); where the copy is
# over before the window could show, the case is skipped
mkdir -p "$work/Big/T"
dd if=/dev/zero of="$work/Big/huge" bs=8M count=512 oflag=direct status=none
guest open-big "open '$gw/Big'"
if window Big 30; then
	place 60 60
	read -r sx sy <<<"$(at "$(item_x 0)" $item_up)"
	read -r tx ty <<<"$(at "$(item_x 1)" $item_up)"
	drag $sx $sy $tx $ty option
	if window Copy 3; then
		read -r bx by <<<"$(at 385 54)"
		press $bx $by
		window_gone Copy 15
		sleep 3
		if [ -e "$work/Big/T/huge" ]; then
			fail "progress: after Stop, T/huge is there ($(stat -c %s "$work/Big/T/huge") bytes)"
		elif [ "$(windows Copy)" != 0 ]; then
			fail "progress: the Copy window stays after Stop"
		else
			pass "progress"
		fi
	elif [ "$(stat -c %s "$work/Big/T/huge" 2>/dev/null)" = $((4 << 30)) ]; then
		skip "progress: the copy was over in less than the Copy window's delay"
	else
		fail "progress: no Copy window within 3 s of the drop (T/huge: $(stat -c %s "$work/Big/T/huge" 2>/dev/null) bytes)"
	fi
else
	fail "progress: no window: $(outputs open-big)"
fi
quit_finder
rm -f "$work/Big/huge" "$work/Big/T/huge"
fi

# paste: Command-C in one window, Command-V in another, copies the item
mkdir -p "$work/C" "$work/V"
echo p >"$work/C/p"
guest open-cv "open '$gw/C'; open '$gw/V'"
if window C 30 && window V 10; then
	window C 1
	key cmd+a
	key cmd+c
	window V 1
	key cmd+v
	if waitfor 15 "$work/V/p" && [ -e "$work/C/p" ]; then pass "paste"; else fail "paste: V has: $(ls "$work/V" | tr '\n' ' ')"; fi
else
	fail "paste: no windows: $(outputs open-cv)"
fi

# new folder and rename: Shift-Command-N, then typing a name and Return, makes a folder of that name
if window V 5; then
	key cmd+shift+n
	typed Neu
	key Return
	if waitfor 10 "$work/V/Neu" && [ -d "$work/V/Neu" ]; then pass "new folder and rename"; else fail "new folder and rename: V has: $(ls "$work/V" | tr '\n' '|')"; fi
else
	fail "new folder and rename: no window V"
fi
quit_finder

# trash: Command-Delete moves the selected item to the Trash in the home folder. The item is in a folder the guest
# makes on the home folder's volume (/Volumes/SystemRoot, the host's file system, has no Trash)
gone="gone $$ $RANDOM"
guest make-r "d=\$(mktemp -d) && mkdir \"\$d/R\" && echo r >\"\$d/R/$gone\" && echo \"\$d/R\""
r=$(cat "$work/make-r.out")
guest open-r "open '$r'"
if [ "$(rc make-r)" != 0 ]; then
	fail "trash: the folder can't be made: $(outputs make-r)"
elif window R 30; then
	key cmd+a
	key cmd+BackSpace
	guest trash "for i in \$(seq 15); do [ -e \"$guest_home/.Trash/$gone\" ] && break; sleep 1; done; ls -d \"$guest_home/.Trash/$gone\" && ! [ -e \"$r/$gone\" ]"
	if [ "$(rc trash)" = 0 ]; then pass "trash"; else fail "trash: $(outputs trash)"; fi
else
	fail "trash: no window: $(outputs open-r)"
fi
quit_finder

[ $failures = 0 ]
