#!/bin/bash
# Behaviour test for ditto(1). Copyright (C) 2026 Lubos Dolezel
#
# Usage: ditto.sh [path-to-ditto]       (default: ditto from PATH)
# Checks the documented behaviour of each mode; every expectation here was
# also observed on real macOS, so the script passes against Apple's ditto too.
# Prints PASS/FAIL lines and exits non-zero if anything failed.

DITTO=${1:-ditto}
umask 022  # the expected modes assume the usual default
T=$(mktemp -d /tmp/dittotest.XXXXXX) || exit 2
trap 'cd /; rm -rf "$T"' EXIT
cd "$T" || exit 2
fails=0

pass() { echo "PASS $1"; }
fail() { echo "FAIL $1"; fails=$((fails + 1)); }
# t DESC CMD...: the command must succeed
t() { local d=$1; shift; if "$@" >/dev/null 2>&1; then pass "$d"; else fail "$d"; fi; }
# nt DESC CMD...: the command must fail
nt() { local d=$1; shift; if "$@" >/dev/null 2>&1; then fail "$d"; else pass "$d"; fi; }
# eq DESC GOT WANT
eq() { if [ "$2" = "$3" ]; then pass "$1"; else fail "$1 (got '$2', want '$3')"; fi; }
mode() { stat -f '%Lp' "$1"; }
mtime() { stat -f '%m' "$1"; }
inode() { stat -f '%i' "$1"; }
# archive entry names, sorted, one line (zipinfo and cpio show AppleDouble entries as they are)
listing() {
	case $1 in
	*.zip) zipinfo -1 "$1" 2>/dev/null ;;
	*) cpio -it < "$1" 2>/dev/null ;;
	esac | sed 's|^\./||' | sort | tr '\n' ' '
}

# ---- plain copy -----------------------------------------------------------
mkdir -p s/sub s/sub2 d/sub
echo a > s/a; echo b > s/sub/b; echo old > d/a; echo keep > d/keep; echo ob > d/sub/b
chmod 750 s/sub; chmod 700 d/sub; chmod 640 s/a
ln -s a s/lnk; ln s/a s/hl
touch -t 200101010101 s/a
t "merge copy succeeds" $DITTO s d
eq "merge keeps files only in the destination" "$(cat d/keep)" keep
eq "merge overwrites files" "$(cat d/a)" a
eq "merge does not change an existing directory's mode" "$(mode d/sub)" 700
eq "new directory takes the source mode" "$(mode d/sub2)" 755
eq "file mode preserved" "$(mode d/a)" 640
eq "file mtime preserved" "$(mtime d/a)" "$(mtime s/a)"
eq "symlinks are copied as symlinks" "$(readlink d/lnk)" a
eq "hard links are preserved" "$(inode d/a)" "$(inode d/hl)"
eq "merge copies nested files" "$(cat d/sub/b)" b

t "missing intermediate directories are created" $DITTO s x/y/z
eq "copy into created hierarchy" "$(cat x/y/z/sub/b)" b
t "file to new file" $DITTO s/a f1
eq "file copy content" "$(cat f1)" a
t "file into existing directory" $DITTO s/a d
t "several sources into a new directory" $DITTO s/sub s/sub2 multi
[ -f multi/b ] && [ -d multi ] && pass "several sources merge into the directory" || fail "several sources merge into the directory"
t "-rsrc spelling is accepted" $DITTO -rsrc s r1
eq "-rsrc copies" "$(cat r1/a)" a
t "-V and -v are accepted" $DITTO -V s v1
t "documented no-op options are accepted" $DITTO --preserveHFSCompression --nopreserveHFSCompression --hfsCompression --nohfsCompression --nocache --nonAtomicCopies s o1
t "-X is accepted" $DITTO -X s o2
echo x > dfile
nt "directory over file fails" $DITTO s dfile
nt "missing source fails" $DITTO nosuch o3
nt "missing destination fails" $DITTO s
nt "no arguments fails" $DITTO
nt "unknown option fails" $DITTO --bogus s o4
nt "copying a directory onto itself fails" $DITTO s s
mkdir -p t3/a t4; echo q > t3/a/f; touch t4/a
nt "directory over file inside a tree fails" $DITTO t3 t4
nt "--keepParent without -c fails" $DITTO --keepParent s o5
echo ro > s/ro; chmod 444 s/ro
t "read-only file copy" $DITTO s q1
t "read-only file replaced" $DITTO s q1
chmod 644 s/ro
echo A > s/x; $DITTO s q2; chmod 600 q2/x; echo B > s/x; $DITTO s q2
eq "replaced file takes source mode" "$(mode q2/x)" 644
eq "replaced file content" "$(cat q2/x)" B
mkdir e1 e2; echo x > e1/n; ln -s /nonexistent e2/n; $DITTO e1 e2
[ -f e2/n ] && [ ! -L e2/n ] && pass "a symlink in the destination is replaced, not followed" || fail "a symlink in the destination is replaced, not followed"

# ---- metadata ------------------------------------------------------------
echo data > m1
if xattr -w user.dt val m1 2>/dev/null; then
	xattr -w com.apple.quarantine "0083;5f3a8c2b;Test;" m1 2>/dev/null
	mkdir mx; cp m1 mx/f 2>/dev/null; xattr -w user.dt val mx/f; xattr -w com.apple.quarantine "0083;5f3a8c2b;Test;" mx/f
	$DITTO mx mxa; eq "extended attributes are preserved" "$(xattr -p user.dt mxa/f 2>/dev/null)" val
	$DITTO --noextattr mx mxb; eq "--noextattr drops other attributes" "$(xattr -p user.dt mxb/f 2>&1 | grep -c val)" 0
	$DITTO --norsrc mx mxc; eq "--norsrc drops attributes" "$(xattr mxc/f | wc -l | tr -d ' ')" 0
	# macOS keeps an explicitly set quarantine attribute even with --noqtn (the man
	# page says otherwise); it goes with --norsrc/--noextattr like other attributes
	$DITTO --noqtn mx mxd; eq "--noqtn keeps other attributes" "$(xattr -p user.dt mxd/f 2>/dev/null)" val
	eq "--noqtn keeps an explicitly set quarantine attribute (as macOS does)" "$(xattr -l mxd/f | grep -c quarantine)" 1
	DITTONORSRC=1 $DITTO mx mxe; eq "DITTONORSRC drops attributes" "$(xattr mxe/f | wc -l | tr -d ' ')" 0
	DITTONORSRC=1 $DITTO --rsrc --extattr mx mxf; eq "explicit options override DITTONORSRC" "$(xattr -p user.dt mxf/f 2>/dev/null)" val
	$DITTO --acl --noacl mx mxg; t "--acl/--noacl accepted" test -f mxg/f
else
	echo "SKIP extended attribute tests (no xattr support here)"
fi

# ---- PKZip archives ---------------------------------------------------------
rm -rf z; mkdir -p z/s/sub; echo data > z/s/f; echo b > z/s/sub/b; ln -s f z/s/lnk; chmod 640 z/s/f
touch -t 200202020202 z/s/f
HASX=0
if xattr -w user.zt zval z/s/f 2>/dev/null; then HASX=1; fi
cd z
t "-c -k creates a zip" $DITTO -c -k --keepParent s ../a1.zip
eq "-c -k --keepParent entries" "$(listing ../a1.zip | tr ' ' '\n' | grep -v '\._' | tr '\n' ' ')" "s/ s/f s/lnk s/sub/ s/sub/b "
t "-c -k --sequesterRsrc --keepParent" $DITTO -c -k --sequesterRsrc --keepParent s ../a2.zip
t "-c -k --sequesterRsrc without --keepParent" $DITTO -c -k --sequesterRsrc s ../a3.zip
eq "no parent entry without --keepParent" "$(listing ../a3.zip | tr ' ' '\n' | grep -v 'MACOSX' | tr '\n' ' ')" "f lnk sub/ sub/b "
t "-c -k --norsrc" $DITTO -c -k --norsrc --keepParent s ../a4.zip
eq "--norsrc stores no AppleDouble entries" "$(listing ../a4.zip | grep -c '\._')" 0
if [ $HASX = 1 ]; then
	eq "sequestered metadata lives under __MACOSX" "$(listing ../a2.zip | tr ' ' '\n' | grep -c '^__MACOSX/s/\._f$')" 1
	eq "sequestered archive keeps __MACOSX directory entries" "$(listing ../a2.zip | tr ' ' '\n' | grep -c '^__MACOSX/s/$')" 1
	eq "sequestered root metadata path" "$(listing ../a3.zip | tr ' ' '\n' | grep -c '^__MACOSX/\._f$')" 1
	eq "unsequestered metadata sits next to the file" "$(listing ../a1.zip | tr ' ' '\n' | grep -c '^s/\._f$')" 1
fi
cd ..
for z in a1 a2; do
	mkdir -p ex-$z
	t "-x -k extracts $z" $DITTO -x -k $z.zip ex-$z
	eq "$z: file content" "$(cat ex-$z/s/f)" data
	eq "$z: file mode" "$(mode ex-$z/s/f)" 640
	eq "$z: file mtime" "$(mtime ex-$z/s/f)" "$(mtime z/s/f)"
	eq "$z: symlink" "$(readlink ex-$z/s/lnk)" f
	eq "$z: nested file" "$(cat ex-$z/s/sub/b)" b
	[ ! -e ex-$z/__MACOSX ] && pass "$z: no __MACOSX directory after extraction" || fail "$z: no __MACOSX directory after extraction"
	[ ! -e ex-$z/s/._f ] && pass "$z: no ._ file after extraction" || fail "$z: no ._ file after extraction"
	[ $HASX = 1 ] && eq "$z: attributes restored" "$(xattr -p user.zt ex-$z/s/f 2>/dev/null)" zval
done
t "-x -k into a not yet existing directory" $DITTO -x -k a1.zip new/deep/dir
eq "created destination content" "$(cat new/deep/dir/s/f)" data
t "-x -k merges into an existing tree" $DITTO -x -k a1.zip ex-a1
$DITTO -c -k --keepParent z/s/f fz.zip; eq "file source with --keepParent" "$(listing fz.zip | tr ' ' '\n' | grep -v '\._' | tr '\n' ' ')" "s/f "
$DITTO -c -k z/s/f fz2.zip; eq "file source without --keepParent" "$(listing fz2.zip | tr ' ' '\n' | grep -v '\._' | tr '\n' ' ')" "f "
for l in 0 1 9; do
	t "--zlibCompressionLevel $l" $DITTO -c -k --zlibCompressionLevel $l z/s zl$l.zip
	rm -rf zlx; t "level $l archive extracts" $DITTO -x -k zl$l.zip zlx
	eq "level $l content" "$(cat zlx/f)" data
done
nt "--zlibCompressionLevel out of range fails" $DITTO -c -k --zlibCompressionLevel 12 z/s zlbad.zip
nt "-z with -k fails" $DITTO -c -z -k z/s zbad.zip
nt "-c with several sources fails" $DITTO -c z/s z/s cm.cpio
mkdir emptyd; nt "an empty source yields no PKZip archive" $DITTO -c -k emptyd empty.zip
echo junk > bad.zip
nt "a non-archive is rejected as PKZip" $DITTO -x -k bad.zip xbad
nt "a non-archive is rejected as CPIO" $DITTO -x bad.zip xbad2
nt "a missing archive is rejected" $DITTO -x -k nosuch.zip xbad3
$DITTO -c -k z/s - > pipe.zip; rm -rf px; mkdir px; $DITTO -x -k - px < pipe.zip; eq "zip through stdout and stdin" "$(cat px/f)" data

# __MACOSX contents that are not AppleDouble data (needs a zip tool to build the archive)
if command -v zip >/dev/null 2>&1; then
	rm -rf jz; mkdir -p jz/q jz/__MACOSX/q; echo hi > jz/q/f; echo notappledouble > jz/__MACOSX/q/._f; echo plain > jz/__MACOSX/q/other
	(cd jz && zip -qr ../jz.zip q __MACOSX)
	rm -rf jx; mkdir jx; $DITTO -x -k jz.zip jx >/dev/null 2>&1; rc=$?
	eq "an invalid AppleDouble entry for an existing file is an error" "$rc" 1
	eq "the file itself is extracted" "$(cat jx/q/f)" hi
	eq "other __MACOSX entries are ordinary files" "$(cat jx/__MACOSX/q/other)" plain
	rm -rf lz; mkdir -p lz/__MACOSX/lone; echo x > lz/__MACOSX/lone/._nofile
	(cd lz && zip -qr ../lz.zip __MACOSX); rm -rf lx; mkdir lx; $DITTO -x -k lz.zip lx >/dev/null 2>&1
	eq "an AppleDouble entry without its file is dropped without error" "$?" 0
	[ ! -e lx/__MACOSX/lone/._nofile ] && [ ! -e lx/lone/nofile ] && pass "a lone AppleDouble entry creates no file" || fail "a lone AppleDouble entry creates no file"
else
	echo "SKIP invalid AppleDouble zip tests (no zip tool)"
fi

# ---- CPIO archives ------------------------------------------------------------
cd z; ln s/f s/hl; cd ..
t "-c creates a cpio archive" $DITTO -c z/s o.cpio
eq "cpio archives use the portable ASCII format" "$(head -c 6 o.cpio)" 070707
t "-c -z" $DITTO -c -z z/s o.cpgz
eq "-z gzip signature" "$(head -c 2 o.cpgz | od -An -tx1 | tr -d ' ')" 1f8b
t "-c -j" $DITTO -c -j z/s o.cpbz
eq "-j bzip2 signature" "$(head -c 3 o.cpbz)" BZh
$DITTO -c --keepParent z/s o2.cpio; eq "cpio --keepParent entries" "$(listing o2.cpio | tr ' ' '\n' | grep -v '\._' | grep -c '^s/')" 5
for a in o.cpio o.cpgz o.cpbz; do
	rm -rf cx; mkdir cx
	t "-x $a" $DITTO -x $a cx
	eq "$a: file content" "$(cat cx/f)" data
	eq "$a: mode" "$(mode cx/f)" 640
	eq "$a: mtime" "$(mtime cx/f)" "$(mtime z/s/f)"
	eq "$a: symlink" "$(readlink cx/lnk)" f
	eq "$a: hard link" "$(inode cx/f)" "$(inode cx/hl)"
	[ $HASX = 1 ] && eq "$a: attributes restored" "$(xattr -p user.zt cx/f 2>/dev/null)" zval
	[ ! -e cx/._f ] && pass "$a: no ._ file after extraction" || fail "$a: no ._ file after extraction"
done
rm -rf cx; mkdir cx; $DITTO -c z/s - | $DITTO -x - cx; eq "cpio through a pipe" "$(cat cx/f)" data
rm -rf cx; mkdir cx; t "-x with several archives" $DITTO -x o.cpio o.cpgz cx
nt "-x --keepParent fails" $DITTO -x --keepParent o.cpio cx
nt "a PKZip archive is not read as cpio" $DITTO -x -k o.cpio cx

# ---- extraction never leaves the destination -------------------------------------
# odc cpio entries written by hand: header fields are octal text. Like macOS, names
# with .. components or leading slashes are confined to the destination.
cpio_entry() { # name mode type-data
	local name=$1 mode=$2 data=$3
	printf '070707%06o%06o%06o%06o%06o%06o%06o%011o%06o%011o%s\0%s' 0 1 $((mode)) 0 0 1 0 0 $((${#name} + 1)) ${#data} "$name" "$data"
}
{ cpio_entry ../escape 0100644 evil; cpio_entry a/../b 0100644 b1; cpio_entry a/../../c 0100644 c1; cpio_entry TRAILER!!! 0 ""; } > esc.cpio
rm -rf ev; mkdir ev; (cd ev && $DITTO -x ../esc.cpio dst >/dev/null 2>&1)
[ ! -e escape ] && [ ! -e ev/escape ] && pass "a .. entry does not escape" || fail "a .. entry does not escape"
eq "../x is placed inside the destination" "$(cat ev/dst/escape 2>/dev/null)" evil
eq "a/../b becomes b" "$(cat ev/dst/b 2>/dev/null)" b1
eq "a/../../c is clamped to c" "$(cat ev/dst/c 2>/dev/null)" c1
{ cpio_entry "$T/abs-escape" 0100644 evil; cpio_entry TRAILER!!! 0 ""; } > abs.cpio
$DITTO -x abs.cpio ev-dst3 >/dev/null 2>&1
[ ! -e "$T/abs-escape" ] && pass "an absolute entry does not escape" || fail "an absolute entry does not escape"
eq "an absolute entry is placed inside the destination" "$(cat "ev-dst3$T/abs-escape" 2>/dev/null)" evil
mkdir -p outside
{ cpio_entry lk 0120777 "$T/outside"; cpio_entry lk/pwn 0100644 evil; cpio_entry TRAILER!!! 0 ""; } > sl.cpio
rm -rf ev-dst4; $DITTO -x sl.cpio ev-dst4 >/dev/null 2>&1
[ ! -e outside/pwn ] && pass "an entry below an extracted symlink does not escape" || fail "an entry below an extracted symlink does not escape"

# ---- thinning Mach-O files ----------------------------------------------------------
be32() { printf "$(printf '\\%03o\\%03o\\%03o\\%03o' $(($1 >> 24 & 255)) $(($1 >> 16 & 255)) $(($1 >> 8 & 255)) $(($1 & 255)))"; }
# fake thin Mach-O slices: magic, cputype and cpusubtype are real, the rest is filler
{ printf '\xce\xfa\xed\xfe\x07\0\0\0\x03\0\0\0'; head -c 88 /dev/zero | tr '\0' 'i'; } > sl-i386
{ printf '\xcf\xfa\xed\xfe\x07\0\0\x01\x03\0\0\0'; head -c 88 /dev/zero | tr '\0' 'x'; } > sl-x64
{
	be32 0xcafebabe; be32 2
	be32 7; be32 3; be32 4096; be32 100; be32 12
	be32 0x01000007; be32 3; be32 8192; be32 100; be32 12
	head -c $((4096 - 8 - 40)) /dev/zero
	cat sl-i386; head -c $((4096 - 100)) /dev/zero; cat sl-x64
} > fat.bin
t "--arch with a fat file" $DITTO --arch x86_64 fat.bin thin64
cmp -s thin64 sl-x64 && pass "--arch x86_64 leaves only that slice" || fail "--arch x86_64 leaves only that slice"
$DITTO --arch i386 fat.bin thin32; cmp -s thin32 sl-i386 && pass "--arch i386 leaves only that slice" || fail "--arch i386 leaves only that slice"
$DITTO --arch i386 --arch x86_64 fat.bin both; cmp -s both fat.bin && pass "all requested architectures keep the fat file" || fail "all requested architectures keep the fat file"
$DITTO --arch x86_64 sl-x64 thinplain; cmp -s thinplain sl-x64 && pass "--arch leaves a thin file of that architecture alone" || fail "--arch leaves a thin file of that architecture alone"
# observed on macOS: a Mach-O file without any requested architecture is not copied at all
$DITTO --arch arm64 sl-x64 thinother >/dev/null 2>&1; [ ! -e thinother ] && pass "a thin file of another architecture is skipped" || fail "a thin file of another architecture is skipped"
$DITTO --arch arm64 fat.bin thinnone >/dev/null 2>&1; [ ! -e thinnone ] && pass "a fat file without a requested architecture is skipped" || fail "a fat file without a requested architecture is skipped"
$DITTO --arch x86_64 z/s/f thintext; eq "--arch leaves other files alone" "$(cat thintext)" data

# ---- BOM mode -------------------------------------------------------------------------------
if command -v mkbom >/dev/null 2>&1 && command -v lsbom >/dev/null 2>&1; then
	rm -rf bs bsub bd; mkdir -p bs/d1 bs/d2 bsub/d1; echo 1 > bs/d1/a; echo 2 > bs/d2/b; echo 3 > bs/top; echo 1 > bsub/d1/a
	mkbom bsub sub.bom >/dev/null 2>&1
	t "--bom copy" $DITTO --bom sub.bom -- bs bd
	[ -f bd/d1/a ] && [ ! -e bd/d2 ] && [ ! -e bd/top ] && pass "only the BOM's paths are copied" || fail "only the BOM's paths are copied"
	nt "a missing BOM is an error" $DITTO --bom nosuch.bom -- bs bd2
else
	echo "SKIP BOM tests (no mkbom/lsbom)"
fi

echo
if [ $fails = 0 ]; then echo "ALL PASSED"; else echo "$fails FAILED"; fi
exit $((fails != 0))
