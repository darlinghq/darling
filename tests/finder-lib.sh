# Helpers for the GUI tests of Finder and the apps around it (finder.sh, open-disk-image.sh), sourced after `set -u`
# with $here set to the tests directory. They need what finder.sh's header lists. Sets up $work (a scratch directory
# removed on exit), $gw (its guest path) and $display (x11 or wayland); the window functions look for windows of the
# application named in $app (Finder unless set).
app=${app:-Finder}
work=$(mktemp -d)
gw="/Volumes/SystemRoot$work"
failures=0
kb=
vp=
trap 'kill $kb $vp 2>/dev/null; chmod -R u+rwx "$work" 2>/dev/null; rm -rf "$work"' EXIT

pass() { echo "PASS: $1"; }
fail() { echo "FAIL: $1"; failures=$((failures + 1)); }
skip() { echo "SKIP: $1"; }

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

running() { guest running "ps ax | grep -c '[F]inder.app/Contents/MacOS/Finder'"; cat "$work/running.out"; }
wait_gone() { # <seconds>: until Finder is not running; 0 if it went
	local i
	for ((i = 0; i < $1; i++)); do [ "$(running)" = 0 ] && return 0; sleep 1; done
	return 1
}
quit_finder() {
	guest quit "for p in \$(ps ax | awk '/[F]inder.app\\/Contents\\/MacOS\\/Finder/ { print \$1 }'); do kill \$p; done; true"
	wait_gone 15
}
volume="/Volumes/Unmount Test"
detach_all() { guest detach-all "for d in '$volume'*; do hdiutil detach -quiet \"\$d\"; done; true"; }

# $app's windows with that title, one id per line (empty title: every window of $app but the menu bar)
# Titles are compared as canonically equivalent Unicode: Foundation decomposes file paths (as on macOS), so a folder
# whose name is stored precomposed gets a window titled with the decomposed form
nfc() { python3 -I -c 'import sys, unicodedata; print(unicodedata.normalize("NFC", sys.argv[1]))' "$1"; }
find_windows() {
	if [ $display = x11 ]; then
		local want
		want=$(nfc "$1")
		xdotool search --onlyvisible --class "$app" 2>/dev/null | while read -r w; do
			xprop -id "$w" _NET_WM_WINDOW_TYPE 2>/dev/null | grep -q _DOCK && continue
			[ -z "$1" ] || [ "$(nfc "$(xdotool getwindowname "$w")")" = "$want" ] && echo "$w"; done
	else
		swaymsg -t get_tree | python3 -I -c '
import json, sys, unicodedata
title = unicodedata.normalize("NFC", sys.argv[1])
def walk(n):
    if n.get("app_id") == sys.argv[2] and n.get("type") in ("con", "floating_con") and (not title or unicodedata.normalize("NFC", n.get("name") or "") == title):
        print(n["id"])
    for c in n.get("nodes", []) + n.get("floating_nodes", []):
        walk(c)
walk(json.load(sys.stdin))' "$1" "$app"
	fi
}
windows() { find_windows "$1" | grep -c .; }
window() { # <title> <seconds>: waits for a Finder window with that title; its id in WID; 0 if there is one
	local i
	for ((i = 0; i < $2; i++)); do
		WID=$(find_windows "$1" | tail -1)
		[ -n "$WID" ] && return 0
		sleep 1
	done
	return 1
}
window_gone() { # <title> <seconds>: until no Finder window has that title
	local i
	for ((i = 0; i < $2; i++)); do [ "$(windows "$1")" = 0 ] && return 0; sleep 1; done
	return 1
}
key() { # <key>: to the window window() found, once it has had time to become key; cmd+<key> is Command-<key>
	sleep 2
	if [ $display = x11 ]; then
		xdotool windowactivate --sync "$WID" 2>/dev/null || xdotool windowfocus "$WID"
		sleep 0.5
		case $1 in
			cmd+*) xdotool key "super+${1#cmd+}" ;;
			*) xdotool key "$1" ;;
		esac
	else
		swaymsg -q "[con_id=$WID] focus"
		sleep 0.5
		case $1 in
			cmd+shift+*) wtype -M logo -M shift -k "${1#cmd+shift+}" -m shift -m logo ;;
			cmd+*) wtype -M logo -k "${1#cmd+}" -m logo ;;
			*) wtype -k "$1" ;;
		esac
	fi
	sleep 1
}

click() { # <x> <y>: a click in the window window() found, <x> from its left edge and <y> up from its bottom edge
	# (X11 only)
	local height
	height=$(xdotool getwindowgeometry --shell "$WID" | sed -n 's/^HEIGHT=//p')
	xdotool windowactivate --sync "$WID" 2>/dev/null
	xdotool mousemove --window "$WID" "$1" $((height - $2)) click 1
	sleep 1
}

shot() { # <name>: an image of the window window() found, in $work/<name>.png
	if [ $display = x11 ]; then
		import -silent -window "$WID" "$work/$1.png" 2>/dev/null
	else
		local rect
		rect=$(swaymsg -t get_tree | python3 -I -c '
import json, sys
want = int(sys.argv[1])
def walk(n):
    if n.get("id") == want:
        r = n["rect"]
        print("%d,%d %dx%d" % (r["x"], r["y"], r["width"], r["height"]))
    for c in n.get("nodes", []) + n.get("floating_nodes", []):
        walk(c)
walk(json.load(sys.stdin))' "$WID")
		grim -g "$rect" "$work/$1.png" 2>/dev/null
	fi
}
differ() { # <name> <name>: 0 if the two images differ
	local n
	n=$(compare -metric AE "$work/$1.png" "$work/$2.png" null: 2>&1 | cut -d' ' -f1)
	[ -n "$n" ] && [ "${n%.*}" != 0 ] 2>/dev/null
}

build() { # <program> <source> [<guest-cc arguments>...]
	local program=$1 source=$2
	shift 2
	"$DARLING_SKILLS/darling-guest-testing/scripts/guest-cc.sh" -o "$work/$program" "$source" "$@" \
		|| { fail "$program does not build"; return 1; }
}

# Pointer drags. X11 uses xdotool; Wayland the virtual pointer of cocotron's tests/wayland-virtual-pointer.c, built
# and named in $VIRTUAL_POINTER (a headless sway has no pointer of its own); without it the drag cases are skipped.
pointer_ok=1
if [ $display = wayland ]; then
	if [ -n "${VIRTUAL_POINTER:-}" ] && mkfifo "$work/pointer.in"; then
		"$VIRTUAL_POINTER" <"$work/pointer.in" >"$work/pointer.out" 2>&1 &
		vp=$!
		exec 3>"$work/pointer.in"
		for ((i = 0; i < 50; i++)); do grep -q '^READY' "$work/pointer.out" && break; sleep 0.1; done
		grep -q '^READY' "$work/pointer.out" || pointer_ok=0
	else
		pointer_ok=0
	fi
fi
pointer() { # <command line>: to the virtual pointer, waiting for its answer
	local n i
	n=$(grep -c . "$work/pointer.out")
	echo "$1" >&3
	for ((i = 0; i < 100; i++)); do [ "$(grep -c . "$work/pointer.out")" -gt "$n" ] && break; sleep 0.1; done
}
at() { # <x> <y>: the screen point <x> from the left edge of the window window() found and <y> up from its bottom
	local fx fy fh
	if [ $display = x11 ]; then
		eval "$(xdotool getwindowgeometry --shell "$WID")"
		fx=$X fy=$Y fh=$HEIGHT
	else
		read -r fx fy fh <<<"$(swaymsg -t get_tree | python3 -I -c '
import json, sys
want = int(sys.argv[1])
def walk(n):
    if n.get("id") == want:
        print(n["rect"]["x"], n["rect"]["y"], n["rect"]["height"])
    for c in n.get("nodes", []) + n.get("floating_nodes", []):
        walk(c)
walk(json.load(sys.stdin))' "$WID")"
	fi
	echo "$((fx + $1)) $((fy + fh - $2))"
}
place() { # <x> <y>: moves the window window() found so that its top left is at that point of the screen
	if [ $display = x11 ]; then xdotool windowmove --sync "$WID" "$1" "$2"; else swaymsg -q "[con_id=$WID] move position $1 $2"; fi
	sleep 1
}
press() { # <x> <y>: a click at that point of the screen
	if [ $display = x11 ]; then xdotool mousemove "$1" "$2" click 1; else pointer "$1 $2"; fi
	sleep 1
}
drag() { # <x> <y> <x> <y> [option]: a drag between two points of the screen, holding Option if asked
	local held=
	if [ $display = x11 ]; then
		[ -n "${5:-}" ] && xdotool keydown alt
		xdotool mousemove "$1" "$2"
		sleep 0.3
		xdotool mousedown 1
		for ((i = 1; i <= 10; i++)); do
			xdotool mousemove $(($1 + ($3 - $1) * i / 10)) $(($2 + ($4 - $2) * i / 10))
			sleep 0.1
		done
		sleep 0.5
		xdotool mouseup 1
		[ -n "${5:-}" ] && xdotool keyup alt
	else
		if [ -n "${5:-}" ]; then
			# Held for the whole drag, then released (a wtype that is killed leaves Option held)
			wtype -M alt -s 4000 -m alt &
			held=$!
			sleep 0.5
		fi
		pointer "drag $1 $2 $(($3 - $1)) $(($4 - $2))"
		[ -n "$held" ] && wait $held
	fi
	sleep 2
}
typed() { # <text>: typed into the window window() found
	if [ $display = x11 ]; then xdotool type "$1"; else wtype "$1"; fi
	sleep 1
}
waitfor() { # <seconds> <path>...: until every path exists (on the host side); 0 if they do
	local i p ok
	for ((i = 0; i < $1; i++)); do
		ok=1
		for p in "${@:2}"; do [ -e "$p" ] || ok=0; done
		[ $ok = 1 ] && return 0
		sleep 1
	done
	return 1
}
# Where a window's items are, from its left edge and up from its bottom: an 800-point window's first icon row
item_x() { echo $((263 + 115 * $1)); }
item_up=452

# The client area of the window window() found: "<width> <height>"
client_size() {
	if [ $display = x11 ]; then
		xdotool getwindowgeometry --shell "$WID" | sed -n 's/^WIDTH=//p; s/^HEIGHT=//p' | tr '\n' ' '
	else
		swaymsg -t get_tree | python3 -I -c '
import json, sys
want = int(sys.argv[1])
def walk(n):
    if n.get("id") == want:
        print(n["rect"]["width"], n["rect"]["height"])
    for c in n.get("nodes", []) + n.get("floating_nodes", []):
        walk(c)
walk(json.load(sys.stdin))' "$WID"
	fi
}
pixel() { # <name> <x> <y>: the pixel of $work/<name>.png at (x, y) from its top left, as "r g b" (0-255)
	convert "$work/$1.png" -format "%[fx:int(255*p{$2,$3}.r+0.5)] %[fx:int(255*p{$2,$3}.g+0.5)] %[fx:int(255*p{$2,$3}.b+0.5)]" info: 2>/dev/null
}
near() { # <r g b> <r g b> <tolerance>: 0 if every channel is within it
	python3 -I -c 'import sys; a, b = sys.argv[1].split(), sys.argv[2].split(); sys.exit(not (len(a) == 3 and all(abs(int(x) - int(y)) <= int(sys.argv[3]) for x, y in zip(a, b))))' "$1" "$2" "$3"
}
