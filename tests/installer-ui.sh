#!/bin/bash
# GUI test for Installer.app: opens a package, walks through every step with the keyboard
# and checks what was installed. Copyright (C) 2026 Lubos Dolezel
#
# Usage: installer-ui.sh [path-to-Installer-executable-in-the-guest]
#        (default: /System/Library/CoreServices/Installer.app/Contents/MacOS/Installer)
# Run on the host with Darling installed, on an X display of your own: start Xvfb and a
# window manager on a free display and export DISPLAY before the Darling container starts
# (it keeps the environment it started with; run `darling shutdown` first if needed).
# Do not use a display another session is using: the test clicks and types.
# It builds a product archive with bsdtar, which writes xar. The packages install into a
# directory below $T on the host, which the guest sees under /Volumes/SystemRoot.
# Prints PASS/FAIL lines and exits non-zero if anything failed.

APP=${1:-/System/Library/CoreServices/Installer.app/Contents/MacOS/Installer}
[ -n "$DISPLAY" ] || { echo "set DISPLAY to an X display of your own" >&2; exit 2; }
T=$(mktemp -d /tmp/installerui.XXXXXX) || exit 2
trap 'kill $APP_PID 2>/dev/null; rm -rf "$T"' EXIT
fails=0

pass() { echo "PASS $1"; }
fail() { echo "FAIL $1"; fails=$((fails + 1)); }

# the window of the package, by title; waits up to $2 seconds (default 45: the first start of the container is slow)
wait_window() {
	local i
	for ((i = 0; i < ${2:-45}; i++)); do
		WID=$(xdotool search --onlyvisible --name "$1" 2>/dev/null | tail -1)
		[ -n "$WID" ] && return 0
		sleep 1
	done
	return 1
}

# key KEYS...: to the package window; with KEEP_FOCUS=1 to whatever window has the focus (a dialog)
key() { [ -n "$KEEP_FOCUS" ] || xdotool windowfocus "$WID" 2>/dev/null; xdotool key --delay 100 "$@"; sleep "${KEY_WAIT:-1.5}"; }

# ---- a product archive with every step and a nested choice ----------------
DEST=$T/installed
mkdir -p "$DEST" "$T/product/Resources"
cd "$T/product" || exit 2
for c in core docs tools; do
	mkdir "$c.pkg" "root-$c"
	echo "$c" > "root-$c/$c.txt"
	bsdtar --format odc -cf - -C "root-$c" . 2>/dev/null | gzip > "$c.pkg/Payload"
	cat > "$c.pkg/PackageInfo" <<EOF
<?xml version="1.0" encoding="utf-8"?>
<pkg-info format-version="2" identifier="org.darling.uitest.$c" version="1.0" install-location="/Volumes/SystemRoot$DEST" auth="root">
<payload installKBytes="100" numberOfFiles="1"/>
</pkg-info>
EOF
done
echo '<html><body><h1>Welcome</h1><p>The <b>UI test</b> package.</p></body></html>' > Resources/welcome.html
printf '{\\rtf1\\ansi {\\b Read Me}\\par The read me.}' > Resources/readme.rtf
printf 'LICENSE\n\nUse freely.\n' > Resources/license.txt
echo '<html><body><p>Installed.</p></body></html>' > Resources/conclusion.html
cat > Distribution <<'EOF'
<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="2">
<title>UI Test</title>
<welcome file="welcome.html" mime-type="text/html"/>
<readme file="readme.rtf" mime-type="text/rtf"/>
<license file="license.txt" mime-type="text/plain"/>
<conclusion file="conclusion.html" mime-type="text/html"/>
<choices-outline><line choice="core"/><line choice="extras"><line choice="docs"/><line choice="tools"/></line></choices-outline>
<choice id="core" title="Core"><pkg-ref id="org.darling.uitest.core"/></choice>
<choice id="extras" title="Extras"/>
<choice id="docs" title="Documentation" start_selected="false"><pkg-ref id="org.darling.uitest.docs"/></choice>
<choice id="tools" title="Tools"><pkg-ref id="org.darling.uitest.tools"/></choice>
<pkg-ref id="org.darling.uitest.core" installKBytes="100">#core.pkg</pkg-ref>
<pkg-ref id="org.darling.uitest.docs" installKBytes="100">#docs.pkg</pkg-ref>
<pkg-ref id="org.darling.uitest.tools" installKBytes="100">#tools.pkg</pkg-ref>
</installer-gui-script>
EOF
bsdtar --format xar --options xar:compression=none -cf "$T/ui.pkg" Distribution core.pkg docs.pkg tools.pkg Resources 2>/dev/null || { echo "cannot build the package"; exit 2; }

# ---- the walk ---------------------------------------------------------------
timeout -s KILL 120 darling shell env DISPLAY="$DISPLAY" "$APP" "/Volumes/SystemRoot$T/ui.pkg" </dev/null > "$T/app.out" 2>&1 &
APP_PID=$!

if wait_window "Install UI Test"; then pass "the window of the package opens, titled after the package"; else fail "the window of the package opens, titled after the package"; cat "$T/app.out" | tail -5; exit 1; fi

key Return   # Introduction -> Read Me
key Return   # Read Me -> License
key Return   # License: asks to agree
KEEP_FOCUS=1 key Return   # Agree (the default button of the dialog) -> Destination Select
[ ! -e "$DEST/core.txt" ] && pass "nothing is installed before Install is pressed" || fail "nothing is installed before Install is pressed"
key Return   # Destination Select -> Installation Type
KEY_WAIT=8 key Return   # Install -> Installation -> Summary

[ "$(cat "$DEST/core.txt" 2>/dev/null)" = core ] && pass "the default choices are installed" || fail "the default choices are installed"
[ "$(cat "$DEST/tools.txt" 2>/dev/null)" = tools ] && pass "a selected subchoice is installed" || fail "a selected subchoice is installed"
[ ! -e "$DEST/docs.txt" ] && pass "a choice that starts deselected is not installed" || fail "a choice that starts deselected is not installed"

key Return   # Close
sleep 2
if xdotool search --onlyvisible --name "Install UI Test" >/dev/null 2>&1; then fail "Close closes the window"; else pass "Close closes the window"; fi
wait $APP_PID 2>/dev/null
[ $? -eq 0 ] && pass "the application quits with its last window" || fail "the application quits with its last window"

echo
if [ "$fails" -eq 0 ]; then echo "all passed"; else echo "$fails failed"; exit 1; fi
