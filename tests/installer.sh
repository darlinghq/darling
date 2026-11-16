#!/bin/bash
# Behaviour test for installer(8): payload extraction safety, link targets,
# file modes and script failures. Copyright (C) 2026 Lubos Dolezel
#
# Usage: installer.sh build DIR              make the test packages in DIR
#        installer.sh run DIR [installer]    install them (installer from PATH by default)
# The packages are flat packages made with bsdtar (xar container, gzip'd odc
# cpio payload). Building needs a bsdtar with xar support, which the Darling
# guest lacks, so build on the host and run in the guest (on macOS both steps
# work). Expectations come from the documented behaviour of installer(8) and
# the package format; they are not tied to any application.
# Prints PASS/FAIL lines and exits non-zero if anything failed.

umask 022
MODE=$1
DIR=$(cd "$2" 2>/dev/null && pwd) || { echo "usage: $0 build|run DIR [installer]" >&2; exit 2; }
LONG=$(head -c 300 /dev/zero | tr '\0' 'a')

# ---- build (host) ---------------------------------------------------------
# pkgxar NAME ID [SCRIPTS-DIR]: wraps ./payload.gz (and the scripts) into DIR/NAME.pkg
pkgxar() {
	local name=$1 id=$2 scripts=$3 w
	w=$(mktemp -d "$DIR/work.XXXXXX") || exit 2
	cp payload.gz "$w/Payload"
	cat > "$w/PackageInfo" <<EOF
<?xml version="1.0" encoding="utf-8"?>
<pkg-info format-version="2" identifier="$id" version="1.0" install-location="/" auth="root">
<payload installKBytes="1" numberOfFiles="1"/>
$([ -n "$scripts" ] && echo '<scripts><postinstall file="./postinstall"/><postupgrade file="./postinstall"/></scripts>')
</pkg-info>
EOF
	if [ -n "$scripts" ]; then
		bsdtar -P --format odc -cf - -C "$scripts" . | gzip > "$w/Scripts"
		(cd "$w" && bsdtar --format xar --options xar:compression=none -cf "$DIR/$name.pkg" PackageInfo Payload Scripts) || exit 2
	else
		(cd "$w" && bsdtar --format xar --options xar:compression=none -cf "$DIR/$name.pkg" PackageInfo Payload) || exit 2
	fi
	rm -rf "$w"
}

if [ "$MODE" = build ]; then
	B=$(mktemp -d /tmp/installerbuild.XXXXXX) || exit 2
	trap 'cd /; rm -rf "$B"' EXIT
	cd "$B" || exit 2

	# regular files, a mode, a modification time, a long symlink target
	mkdir -p root/dir
	echo data > root/dir/file
	chmod 640 root/dir/file
	TZ=UTC touch -t 200101010101 root/dir/file
	ln -s "$LONG" root/longlink
	bsdtar --format odc -cf - -C root . | gzip > payload.gz
	pkgxar plain org.darling.test.plain

	# an entry whose name climbs out of the destination
	echo escape > escape-src
	bsdtar -P --format odc -cf - -s '|^escape-src|../escaped|' escape-src | gzip > payload.gz
	pkgxar traversal org.darling.test.traversal

	# a link, then a regular file at the same path
	echo second > second-src
	ln -s "$DIR/victim" link-src
	bsdtar -P --format odc -cf - -s '|^second-src|link-src|' link-src second-src | gzip > payload.gz
	pkgxar linkwrite org.darling.test.linkwrite

	# a postinstall script that cannot be launched (not executable, so exec fails with EACCES)
	mkdir scripts-src
	printf 'exit 0\n' > scripts-src/postinstall
	chmod 644 scripts-src/postinstall
	bsdtar --format odc -cf - -C root . | gzip > payload.gz
	pkgxar badscript org.darling.test.badscript scripts-src

	# a product archive: choices with literal-boolean and JavaScript conditions
	mkdir product && cd product || exit 2
	for c in default startfalse selfalse js inactive; do
		mkdir "$c.pkg" "root-$c"
		echo "$c" > "root-$c/$c.txt"
		bsdtar --format odc -cf - -C "root-$c" . | gzip > "$c.pkg/Payload"
		cat > "$c.pkg/PackageInfo" <<EOF2
<?xml version="1.0" encoding="utf-8"?>
<pkg-info format-version="2" identifier="org.darling.test.$c" version="1.0" install-location="/" auth="root">
<payload installKBytes="1" numberOfFiles="1"/>
</pkg-info>
EOF2
	done
	cat > Distribution <<'EOF2'
<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="2">
<title>Choices</title>
<choices-outline>
<line choice="default"/><line choice="startfalse"/><line choice="selfalse"/>
<line choice="js"/><line choice="inactive"/>
</choices-outline>
<choice id="default" title="Default"><pkg-ref id="org.darling.test.default"/></choice>
<choice id="startfalse" title="Start false" start_selected="false"><pkg-ref id="org.darling.test.startfalse"/></choice>
<choice id="selfalse" title="Selected false" selected="false"><pkg-ref id="org.darling.test.selfalse"/></choice>
<choice id="js" title="Script" selected="system.compareVersions(system.version.ProductVersion, '99.0') &gt;= 0"><pkg-ref id="org.darling.test.js"/></choice>
<choice id="inactive" title="Inactive"><pkg-ref id="org.darling.test.inactive"/></choice>
<pkg-ref id="org.darling.test.default" installKBytes="1">#default.pkg</pkg-ref>
<pkg-ref id="org.darling.test.startfalse" installKBytes="1">#startfalse.pkg</pkg-ref>
<pkg-ref id="org.darling.test.selfalse" installKBytes="1">#selfalse.pkg</pkg-ref>
<pkg-ref id="org.darling.test.js" installKBytes="1">#js.pkg</pkg-ref>
<pkg-ref id="org.darling.test.inactive" installKBytes="1" active="false">#inactive.pkg</pkg-ref>
</installer-gui-script>
EOF2
	bsdtar --format xar --options xar:compression=none -cf "$DIR/choices.pkg" Distribution *.pkg || exit 2
	cd ..
	exit 0
fi

# ---- run (guest) ----------------------------------------------------------
INSTALLER=${3:-installer}
T=$(mktemp -d /tmp/installertest.XXXXXX) || exit 2
trap 'cd /; rm -rf "$T"' EXIT
cd "$T" || exit 2
fails=0

pass() { echo "PASS $1"; }
fail() { echo "FAIL $1"; fails=$((fails + 1)); }
t() { local d=$1; shift; if "$@" >/dev/null 2>&1; then pass "$d"; else fail "$d"; fi; }
nt() { local d=$1; shift; if "$@" >/dev/null 2>&1; then fail "$d"; else pass "$d"; fi; }
eq() { if [ "$2" = "$3" ]; then pass "$1"; else fail "$1 (got '$2', want '$3')"; fi; }

mkdir dst1
t "plain package installs" "$INSTALLER" -pkg "$DIR/plain.pkg" -target "$T/dst1"
eq "file contents are extracted" "$(cat dst1/dir/file 2>/dev/null)" data
eq "file mode is kept" "$(stat -f '%Lp' dst1/dir/file 2>/dev/null)" 640
eq "file modification time is kept" "$(stat -f '%m' dst1/dir/file 2>/dev/null)" 978310860
eq "a symlink target longer than 255 bytes is kept whole" "$(readlink dst1/longlink 2>/dev/null | tr -d '\n' | wc -c | tr -d ' ')" 300

mkdir dst2
nt "an entry with '..' makes the install fail" "$INSTALLER" -pkg "$DIR/traversal.pkg" -target "$T/dst2"
if [ ! -e "$T/escaped" ] && [ ! -e "$T/dst2/../escaped" ]; then pass "nothing is written outside the destination"; else fail "nothing is written outside the destination"; fi

mkdir dst3
rm -f "$DIR/victim"
"$INSTALLER" -pkg "$DIR/linkwrite.pkg" -target "$T/dst3" >/dev/null 2>&1
if [ ! -e "$DIR/victim" ]; then pass "a regular file replaces an earlier link instead of following it"; else fail "a regular file replaces an earlier link instead of following it"; rm -f "$DIR/victim"; fi

mkdir dst4
nt "a script that cannot be executed fails the install" "$INSTALLER" -pkg "$DIR/badscript.pkg" -target "$T/dst4"

# ---- information queries and command-line errors ----------------------
# Expectations were observed on macOS 26 (x86_64) and also hold there.
eq "-pkginfo lists the package name" "$("$INSTALLER" -pkginfo -pkg "$DIR/plain.pkg" 2>&1)" plain
eq "-dominfo lists nothing for a package without domains" "$("$INSTALLER" -dominfo -pkg "$DIR/plain.pkg" 2>&1)" ""
case "$("$INSTALLER" -volinfo -pkg "$DIR/plain.pkg" 2>&1)" in
/*) pass "-volinfo lists mount points" ;;
*) fail "-volinfo lists mount points" ;;
esac
"$INSTALLER" -volinfo -pkg "$DIR/plain.pkg" 2>&1 | grep -qx / && pass "-volinfo includes /" || fail "-volinfo includes /"
eq "-volinfo -plist: first line" "$("$INSTALLER" -volinfo -plist -pkg "$DIR/plain.pkg" 2>&1 | sed -n 1p)" '<?xml version="1.0" encoding="UTF-8"?>'
"$INSTALLER" -pkginfo -plist -pkg "$DIR/plain.pkg" 2>&1 | tr -d ' \n' | grep -q '<key>Package</key><string>plain</string>' && pass "-pkginfo -plist has a Package entry" || fail "-pkginfo -plist has a Package entry"
"$INSTALLER" -dominfo -plist -pkg "$DIR/plain.pkg" 2>&1 | grep -qx '<array/>' && pass "-dominfo -plist is an empty array" || fail "-dominfo -plist is an empty array"
"$INSTALLER" -pkginfo -verbose -pkg "$DIR/plain.pkg" 2>&1 | grep -q '^Package  *: plain$' && pass "-pkginfo -verbose has a Package line" || fail "-pkginfo -verbose has a Package line"
eq "no arguments: exit status 255" "$("$INSTALLER" >/dev/null 2>&1; echo $?)" 255
eq "unknown option: exit status 255" "$("$INSTALLER" -bogus -pkg "$DIR/plain.pkg" >/dev/null 2>&1; echo $?)" 255
"$INSTALLER" -bogus -pkg "$DIR/plain.pkg" 2>&1 | grep -qx 'installer: invalid option -bogus' && pass "unknown option is named" || fail "unknown option is named"
eq "-dominfo without -pkg: exit status 255" "$("$INSTALLER" -dominfo >/dev/null 2>&1; echo $?)" 255
"$INSTALLER" -dominfo 2>&1 | grep -qx 'installer: Error a package must be specified.' && pass "missing package is reported" || fail "missing package is reported"
eq "-tgt is a synonym of -target: bad target message" "$("$INSTALLER" -pkg "$DIR/plain.pkg" -tgt /nonexistent 2>&1)" "installer: Error trying to locate volume at /nonexistent"
eq "bad target: exit status 1" "$("$INSTALLER" -pkg "$DIR/plain.pkg" -tgt /nonexistent >/dev/null 2>&1; echo $?)" 1
eq "missing package file" "$("$INSTALLER" -pkg /nonexistent.pkg -target / 2>&1)" "installer: Error - the package path specified was invalid: '/nonexistent.pkg'."
"$INSTALLER" -vers >/dev/null 2>&1 && pass "-vers succeeds" || fail "-vers succeeds"
"$INSTALLER" -help >/dev/null 2>&1 && pass "-help succeeds" || fail "-help succeeds"

# ---- Distribution choices -------------------------------------------------
mkdir dst5
"$INSTALLER" -pkg "$DIR/choices.pkg" -target "$T/dst5" > choices.out 2>&1
has() { [ -e "dst5/$1.txt" ]; }
has default && pass "a choice is selected by default" || fail "a choice is selected by default"
has startfalse && fail "start_selected=false is not installed" || pass "start_selected=false is not installed"
has selfalse && fail "selected=false is not installed" || pass "selected=false is not installed"
has inactive && fail "a pkg-ref with active=false is not installed" || pass "a pkg-ref with active=false is not installed"
# Darling has no JavaScript engine here: an unevaluated condition keeps the default (selected) and says so.
# (These two checks describe Darling, not macOS, which evaluates the expression.)
has js && pass "a JavaScript selected condition keeps the default" || fail "a JavaScript selected condition keeps the default"
grep -q "Warning: choice Script: the JavaScript in 'selected' is not evaluated" choices.out && pass "an unevaluated JavaScript condition is reported" || fail "an unevaluated JavaScript condition is reported"
"$INSTALLER" -pkginfo -pkg "$DIR/choices.pkg" 2>&1 | tr '\n' ' ' | grep -qx 'default js ' && pass "-pkginfo lists the packages that would be installed" || fail "-pkginfo lists the packages that would be installed"

# ---- choice XML (-showChoicesXML, choice changes files) ------------------
# Observed on macOS 26 (x86_64). The "js" choice is left out below: macOS evaluates its
# JavaScript condition, Darling keeps the default.
flat() { tr -d ' \n'; }
CH="$DIR/choices.pkg"
"$INSTALLER" -pkg "$CH" -showChoicesXML 2>&1 | flat | grep -q '<key>choiceIdentifier</key><string>__ROOT_CHOICE_IDENT_Choices</string><key>choiceIsEnabled</key><true/><key>choiceIsSelected</key><integer>-1</integer>' && pass "-showChoicesXML: root choice with mixed selection" || fail "-showChoicesXML: root choice with mixed selection"
"$INSTALLER" -pkg "$CH" -showChoicesXML 2>&1 | flat | grep -q '<key>choiceIdentifier</key><string>default</string><key>choiceIsEnabled</key><true/><key>choiceIsSelected</key><integer>1</integer><key>choiceIsVisible</key><true/><key>choiceSizeInKilobytes</key><integer>1</integer><key>choiceTitle</key><string>Default</string><key>pathsOfActivePackagesInChoice</key><array><string>file://localhost/.*choices.pkg#default.pkg</string></array>' && pass "-showChoicesXML: a leaf choice" || fail "-showChoicesXML: a leaf choice"
"$INSTALLER" -pkg "$CH" -showChoicesXML 2>&1 | flat | grep -q '<string>inactive</string>.*<key>choiceSizeInKilobytes</key><integer>0</integer><key>choiceTitle</key><string>Inactive</string><key>pathsOfActivePackagesInChoice</key><array/>' && pass "-showChoicesXML: an inactive package is not listed" || fail "-showChoicesXML: an inactive package is not listed"
changes() { flat | grep -o '<integer>[01]</integer><key>choiceAttribute</key><string>selected</string><key>choiceIdentifier</key><string>[a-z]*</string>' | sed 's/<integer>\(.\)<\/integer>.*<string>\([a-z]*\)<\/string>$/\2=\1/' | tr '\n' ' '; }
eq "-showChoiceChangesXML: selected states" "$("$INSTALLER" -pkg "$CH" -target / -showChoiceChangesXML 2>&1 | changes | tr ' ' '\n' | grep -v '^js=' | tr '\n' ' ')" "default=1 startfalse=0 selfalse=0 inactive=1 "
"$INSTALLER" -pkg "$CH" -target / -showChoiceChangesXML 2>&1 | grep -c '<dict>' | grep -qx 15 && pass "-showChoiceChangesXML: visible, enabled and selected per choice" || fail "-showChoiceChangesXML: visible, enabled and selected per choice"

cat > dict.plist <<X
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><array>
<dict><key>choiceIdentifier</key><string>startfalse</string><key>choiceAttribute</key><string>selected</string><key>attributeSetting</key><integer>1</integer></dict>
<dict><key>choiceIdentifier</key><string>default</string><key>choiceAttribute</key><string>selected</string><key>attributeSetting</key><integer>0</integer></dict>
<dict><key>choiceIdentifier</key><string>nosuchchoice</string><key>choiceAttribute</key><string>selected</string><key>attributeSetting</key><integer>1</integer></dict>
</array></plist>
X
cat > strings.plist <<X
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><array><string>startfalse</string><string>default</string></array></plist>
X
for f in dict strings; do
	eq "-showChoicesAfterApplyingChangesXML with a $f file" "$("$INSTALLER" -pkg "$CH" -target / -showChoicesAfterApplyingChangesXML $f.plist 2>&1 | changes | tr ' ' '\n' | grep -v '^js=' | tr '\n' ' ')" "default=0 startfalse=1 selfalse=0 inactive=1 "
done
eq "-showChoicesAfterApplyingChangesXML without -target: exit status 255" "$("$INSTALLER" -pkg "$CH" -showChoicesAfterApplyingChangesXML dict.plist >/dev/null 2>&1; echo $?)" 255
"$INSTALLER" -pkg "$CH" -showChoicesAfterApplyingChangesXML dict.plist 2>&1 | grep -qx 'installer: Error -target parameter required' && pass "a missing -target is reported" || fail "a missing -target is reported"
eq "a missing choices file" "$("$INSTALLER" -pkg "$CH" -target / -showChoicesAfterApplyingChangesXML /nonexistent.plist 2>&1)" "installer: choices file '/nonexistent.plist' either could not be found or was malformed."
eq "a missing choices file: exit status 1" "$("$INSTALLER" -pkg "$CH" -target / -showChoicesAfterApplyingChangesXML /nonexistent.plist >/dev/null 2>&1; echo $?)" 1

mkdir dst6
t "-applyChoiceChangesXML installs" "$INSTALLER" -pkg "$CH" -target "$T/dst6" -applyChoiceChangesXML dict.plist
[ -e dst6/startfalse.txt ] && pass "-applyChoiceChangesXML selects a deselected choice" || fail "-applyChoiceChangesXML selects a deselected choice"
[ ! -e dst6/default.txt ] && pass "-applyChoiceChangesXML deselects a selected choice" || fail "-applyChoiceChangesXML deselects a selected choice"
[ ! -e dst6/selfalse.txt ] && pass "-applyChoiceChangesXML leaves other choices alone" || fail "-applyChoiceChangesXML leaves other choices alone"

echo
if [ "$fails" -eq 0 ]; then echo "all passed"; else echo "$fails failed"; exit 1; fi
