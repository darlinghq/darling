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
$([ -n "$scripts" ] && echo '<scripts><postinstall file="./postinstall"/></scripts>')
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

	# a postinstall script the kernel cannot launch (no interpreter line, so exec fails with ENOEXEC)
	mkdir scripts-src
	printf 'exit 0\n' > scripts-src/postinstall
	chmod 755 scripts-src/postinstall
	bsdtar --format odc -cf - -C root . | gzip > payload.gz
	pkgxar badscript org.darling.test.badscript scripts-src
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

echo
if [ "$fails" -eq 0 ]; then echo "all passed"; else echo "$fails failed"; exit 1; fi
