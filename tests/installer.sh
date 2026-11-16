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
	for c in default startfalse selfalse js inactive follows nonbool apis apis2; do
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
<line choice="js"/><line choice="inactive"/><line choice="follows"/><line choice="nonbool"/><line choice="apis"/><line choice="apis2"/>
</choices-outline>
<choice id="default" title="Default"><pkg-ref id="org.darling.test.default"/></choice>
<choice id="startfalse" title="Start false" start_selected="false"><pkg-ref id="org.darling.test.startfalse"/></choice>
<choice id="selfalse" title="Selected false" selected="false"><pkg-ref id="org.darling.test.selfalse"/></choice>
<choice id="js" title="Script" selected="system.compareVersions(system.version.ProductVersion, '99.0') &gt;= 0"><pkg-ref id="org.darling.test.js"/></choice>
<choice id="inactive" title="Inactive"><pkg-ref id="org.darling.test.inactive"/></choice>
<choice id="follows" title="Follows" selected="!choices.default.selected"><pkg-ref id="org.darling.test.follows"/></choice>
<choice id="nonbool" title="Not a boolean" selected="'abc'"><pkg-ref id="org.darling.test.nonbool"/></choice>
<choice id="apis" title="APIs" selected="system.sysctl('kern.ostype') == 'Darwin' &amp;&amp; typeof system.env.PATH == 'string' &amp;&amp; my.target.availableKilobytes &gt; 0 &amp;&amp; choices.default.selected == true"><pkg-ref id="org.darling.test.apis"/></choice>
<choice id="apis2" title="APIs 2" description="Second set" selected="typeof system.version.ProductVersion == 'string' &amp;&amp; system.files.plistAtPath('/System/Library/CoreServices/SystemVersion.plist').ProductVersion == system.version.ProductVersion &amp;&amp; system.files.plistAtPath('/nonexistent.plist') === null &amp;&amp; system.localizedString('nokey') === null &amp;&amp; system.propertiesOf({a: 1, b: 2}).length == 2 &amp;&amp; choices.default.title == 'Default' &amp;&amp; choices.apis2.description == 'Second set' &amp;&amp; choices.default.packages[0].identifier == 'org.darling.test.default' &amp;&amp; ['clean', 'installed', 'upgrade', 'downgrade', 'mixed'].indexOf(choices.default.packageUpgradeAction) &gt;= 0"><pkg-ref id="org.darling.test.apis2"/></choice>
<pkg-ref id="org.darling.test.default" installKBytes="1">#default.pkg</pkg-ref>
<pkg-ref id="org.darling.test.startfalse" installKBytes="1">#startfalse.pkg</pkg-ref>
<pkg-ref id="org.darling.test.selfalse" installKBytes="1">#selfalse.pkg</pkg-ref>
<pkg-ref id="org.darling.test.js" installKBytes="1">#js.pkg</pkg-ref>
<pkg-ref id="org.darling.test.inactive" installKBytes="1" active="false">#inactive.pkg</pkg-ref>
<pkg-ref id="org.darling.test.follows" installKBytes="1">#follows.pkg</pkg-ref>
<pkg-ref id="org.darling.test.nonbool" installKBytes="1">#nonbool.pkg</pkg-ref>
<pkg-ref id="org.darling.test.apis" installKBytes="1">#apis.pkg</pkg-ref>
<pkg-ref id="org.darling.test.apis2" installKBytes="1">#apis2.pkg</pkg-ref>
</installer-gui-script>
EOF2
	bsdtar --format xar --options xar:compression=none -cf "$DIR/choices.pkg" Distribution *.pkg || exit 2

	# a product whose installation-check script fails with a message
	mkdir ../check && cd ../check || exit 2
	cp -R ../product/default.pkg .
	cat > Distribution <<'EOF2'
<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="2">
<title>Check</title>
<script><![CDATA[
function mustFail() {
	my.result.type = 'Fatal';
	my.result.message = 'Needs a newer system';
	return false;
}
]]></script>
<installation-check script="mustFail()"/>
<choices-outline><line choice="default"/></choices-outline>
<choice id="default" title="Default"><pkg-ref id="org.darling.test.default"/></choice>
<pkg-ref id="org.darling.test.default" installKBytes="1">#default.pkg</pkg-ref>
</installer-gui-script>
EOF2
	bsdtar --format xar --options xar:compression=none -cf "$DIR/checkfail.pkg" Distribution default.pkg || exit 2

	# the same, but the result type is Warn: the install continues
	# a product whose choice runs a program from Resources (system.run and runOnce)
	cd .. && mkdir run && cd run || exit 2
	cp -R ../product/default.pkg .
	mkdir Resources
	printf '#!/bin/sh\necho "$@" >> "$DSTVOLUME/run.log"\nexit 7\n' > Resources/probe.sh
	chmod 755 Resources/probe.sh
	cat > Distribution <<'EOF2'
<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="2">
<title>Run</title>
<options allow-external-scripts="true"/>
<choices-outline><line choice="default"/></choices-outline>
<choice id="default" title="Default" selected="system.run('probe.sh', 'a', 'b') == 7 &amp;&amp; system.runOnce('probe.sh', 'once') == 7 &amp;&amp; system.runOnce('probe.sh', 'once') == 7"><pkg-ref id="org.darling.test.default"/></choice>
<pkg-ref id="org.darling.test.default" installKBytes="1">#default.pkg</pkg-ref>
</installer-gui-script>
EOF2
	bsdtar --format xar --options xar:compression=none -cf "$DIR/run.pkg" Distribution default.pkg Resources || exit 2
	sed -e 's/allow-external-scripts="true"/allow-external-scripts="false"/' Distribution > Distribution.off && mv Distribution.off Distribution
	bsdtar --format xar --options xar:compression=none -cf "$DIR/runoff.pkg" Distribution default.pkg Resources || exit 2
	cd ../check

	sed -e "s/'Fatal'/'Warn'/" -e 's|<title>Check</title>|<title>Check warn</title>|' Distribution > Distribution.warn && mv Distribution.warn Distribution
	bsdtar --format xar --options xar:compression=none -cf "$DIR/checkwarn.pkg" Distribution default.pkg || exit 2
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
# JavaScript conditions need JavaScriptCore, an optional component. With it they are evaluated
# (as on macOS: the "js" condition is false there); without it the default (selected) is kept and
# a warning says so.
if grep -q "JavaScriptCore is not available" choices.out; then
	jsc=no
else
	jsc=yes
fi
if [ $jsc = yes ]; then
	has js && fail "a false JavaScript selected condition deselects the choice" || pass "a false JavaScript selected condition deselects the choice"
	has follows && fail "a condition can refer to another choice" || pass "a condition can refer to another choice"
	has nonbool && fail "a condition that is not a boolean is false (observed on macOS)" || pass "a condition that is not a boolean is false (observed on macOS)"
	has apis2 && pass "system.files, localizedString, propertiesOf and the choices properties follow the documentation" || fail "system.files, localizedString, propertiesOf and the choices properties follow the documentation"
	has apis && pass "system.sysctl, system.env, my.target and choices are available to conditions" || fail "system.sysctl, system.env, my.target and choices are available to conditions"
	grep -q "Warning: choice" choices.out && fail "no warning when JavaScript is evaluated" || pass "no warning when JavaScript is evaluated"
else
	has js && pass "without JavaScriptCore a JavaScript condition keeps the default" || fail "without JavaScriptCore a JavaScript condition keeps the default"
	grep -q "Warning: choice Script: the JavaScript in 'selected' is not evaluated" choices.out && pass "an unevaluated JavaScript condition is reported" || fail "an unevaluated JavaScript condition is reported"
fi
echo "(JavaScriptCore: $jsc)"
mkdir dst7
if [ $jsc = yes ]; then
	nt "a failing installation-check stops the install" "$INSTALLER" -pkg "$DIR/checkfail.pkg" -target "$T/dst7"
	"$INSTALLER" -pkg "$DIR/checkfail.pkg" -target "$T/dst7" 2>&1 | grep -qx 'installer: Needs a newer system' && pass "the check's message is reported" || fail "the check's message is reported"
	[ ! -e dst7/default.txt ] && pass "nothing is installed after a failed check" || fail "nothing is installed after a failed check"
	mkdir dst8
	"$INSTALLER" -pkg "$DIR/checkwarn.pkg" -target "$T/dst8" > warn.out 2>&1
	# system.run and runOnce: refused without a terminal, run after the user agrees on one
	mkdir dst9 dst10 dst11
	"$INSTALLER" -pkg "$DIR/run.pkg" -target "$T/dst9" < /dev/null > run-noterm.out 2>&1
	[ ! -e dst9/run.log ] && grep -q "needs your confirmation on a terminal; not running it" run-noterm.out && pass "system.run is not run without a terminal" || fail "system.run is not run without a terminal"
	[ ! -e dst9/default.txt ] && pass "a condition on a program that did not run is false" || fail "a condition on a program that did not run is false"
	if command -v script >/dev/null 2>&1; then
		printf 'y\n' | script -q /dev/null "$INSTALLER" -pkg "$DIR/run.pkg" -target "$T/dst10" > run-term.out 2>&1
		[ "$(cat dst10/run.log 2>/dev/null | tr -d '\r' | tr '\n' '|')" = "a b|once|" ] && pass "system.run runs the program, runOnce only once, with the user's agreement" || fail "system.run runs the program, runOnce only once, with the user's agreement (log: $(cat dst10/run.log 2>/dev/null | tr '\n' '|'))"
		[ -e dst10/default.txt ] && pass "the exit code of the program decides the condition" || fail "the exit code of the program decides the condition"
		printf 'y\n' | script -q /dev/null "$INSTALLER" -pkg "$DIR/runoff.pkg" -target "$T/dst11" > runoff.out 2>&1
		[ ! -e dst11/run.log ] && grep -q "allow-external-scripts is not enabled" runoff.out && pass "system.run needs allow-external-scripts" || fail "system.run needs allow-external-scripts"
	else
		echo "SKIP system.run with a terminal: script(1) not available"
	fi
	"$INSTALLER" -pkg "$DIR/run.pkg" -target "$T/dst9" -showChoiceChangesXML < /dev/null > /dev/null 2>&1
	[ ! -e dst9/run.log ] && pass "information queries never run programs" || fail "information queries never run programs"
	[ -e dst8/default.txt ] && grep -qx 'installer: Warning: Needs a newer system' warn.out && pass "a check whose result type is Warn only warns" || fail "a check whose result type is Warn only warns"
else
	t "without JavaScriptCore an installation-check is skipped with a warning" "$INSTALLER" -pkg "$DIR/checkfail.pkg" -target "$T/dst7"
fi
"$INSTALLER" -pkginfo -pkg "$DIR/choices.pkg" 2>&1 | tr '\n' ' ' | grep -qx "$([ $jsc = yes ] && echo 'default apis apis2 ' || echo 'default js follows nonbool apis apis2 ')" && pass "-pkginfo lists the packages that would be installed" || fail "-pkginfo lists the packages that would be installed"

# ---- choice XML (-showChoicesXML, choice changes files) ------------------
# Observed on macOS 26 (x86_64). The "js" choice is left out below: macOS evaluates its
# JavaScript condition, Darling keeps the default.
flat() { tr -d ' \n'; }
CH="$DIR/choices.pkg"
"$INSTALLER" -pkg "$CH" -showChoicesXML 2>&1 | flat | grep -q '<key>choiceIdentifier</key><string>__ROOT_CHOICE_IDENT_Choices</string><key>choiceIsEnabled</key><true/><key>choiceIsSelected</key><integer>-1</integer>' && pass "-showChoicesXML: root choice with mixed selection" || fail "-showChoicesXML: root choice with mixed selection"
"$INSTALLER" -pkg "$CH" -showChoicesXML 2>&1 | flat | grep -q '<key>choiceIdentifier</key><string>default</string><key>choiceIsEnabled</key><true/><key>choiceIsSelected</key><integer>1</integer><key>choiceIsVisible</key><true/><key>choiceSizeInKilobytes</key><integer>1</integer><key>choiceTitle</key><string>Default</string><key>pathsOfActivePackagesInChoice</key><array><string>file://localhost/.*choices.pkg#default.pkg</string></array>' && pass "-showChoicesXML: a leaf choice" || fail "-showChoicesXML: a leaf choice"
"$INSTALLER" -pkg "$CH" -showChoicesXML 2>&1 | flat | grep -q '<string>inactive</string>.*<key>choiceSizeInKilobytes</key><integer>0</integer><key>choiceTitle</key><string>Inactive</string><key>pathsOfActivePackagesInChoice</key><array/>' && pass "-showChoicesXML: an inactive package is not listed" || fail "-showChoicesXML: an inactive package is not listed"
changes() { flat | grep -o '<integer>[01]</integer><key>choiceAttribute</key><string>selected</string><key>choiceIdentifier</key><string>[a-z]*</string>' | sed 's/<integer>\(.\)<\/integer>.*<string>\([a-z]*\)<\/string>$/\2=\1/' | tr '\n' ' '; }
eq "-showChoiceChangesXML: selected states" "$("$INSTALLER" -pkg "$CH" -target / -showChoiceChangesXML 2>&1 | changes | tr ' ' '\n' | grep -v '^js=\|^follows=\|^nonbool=\|^apis=\|^apis2=' | tr '\n' ' ')" "default=1 startfalse=0 selfalse=0 inactive=1 "
"$INSTALLER" -pkg "$CH" -target / -showChoiceChangesXML 2>&1 | grep -c '<dict>' | grep -qx 27 && pass "-showChoiceChangesXML: visible, enabled and selected per choice" || fail "-showChoiceChangesXML: visible, enabled and selected per choice"

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
	eq "-showChoicesAfterApplyingChangesXML with a $f file" "$("$INSTALLER" -pkg "$CH" -target / -showChoicesAfterApplyingChangesXML $f.plist 2>&1 | changes | tr ' ' '\n' | grep -v '^js=\|^follows=\|^nonbool=\|^apis=\|^apis2=' | tr '\n' ' ')" "default=0 startfalse=1 selfalse=0 inactive=1 "
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
