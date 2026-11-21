#!/bin/sh
# Copyright (C) 2026 Lubos Dolezel
#
# Checks the xattr(1) command-line interface (-l -p -w -d -c -r -s -v -x)
# against behaviour observed identically on macOS 10.14.4 and macOS 26.7.3.
# Only behaviour that is the same on both is asserted; the wording and exit
# status of usage errors differ between the two, so those are only checked
# for "non-zero exit and a usage text".
#
# Usage: xattr-cli.sh            (XATTR=<path> selects another xattr binary;
#                                 XATTR_TEST_KEEP=1 keeps the scratch tree)

XATTR=${XATTR:-xattr}
T=$(mktemp -d "${TMPDIR:-/tmp}/xattr-cli.XXXXXX") || exit 2
if [ -z "$XATTR_TEST_KEEP" ]; then
    trap 'rm -rf "$T"' EXIT
fi
cd "$T" || exit 2

fails=0
checks=0
ok() { checks=$((checks + 1)); }
bad() {
    checks=$((checks + 1)); fails=$((fails + 1))
    echo "FAIL: $1"
    echo "  expected: $2"
    echo "  actual:   $3"
}

# run <name> <expected-rc> <expected-output> command...   (stdout+stderr, sorted)
run() {
    name=$1; erc=$2; eout=$3; shift 3
    aout=$("$@" 2>&1); arc=$?
    # macOS 26 prints a trailing blank after hex bytes; macOS 10.14 does not.
    aout=$(printf '%s\n' "$aout" | sed 's/ *$//' | sort)
    eout=$(printf '%s\n' "$eout" | sed 's/ *$//' | sort)
    if [ "$arc" = "$erc" ] && [ "$aout" = "$eout" ]; then ok; else
        bad "$name" "rc=$erc [$(echo "$eout" | tr '\n' '|')]" "rc=$arc [$(echo "$aout" | tr '\n' '|')]"
    fi
}
# runnz <name> <needle> command...: non-zero exit, output contains needle
runnz() {
    name=$1; needle=$2; shift 2
    aout=$("$@" 2>&1); arc=$?
    case "$aout" in
        *"$needle"*) [ "$arc" != 0 ] && ok || bad "$name" "non-zero exit" "rc=0" ;;
        *) bad "$name" "output containing '$needle'" "$aout" ;;
    esac
}

# Fixture: nested directories, a symlink to a file, a symlink to a directory.
mkdir -p tree/sub/deep other
echo a > tree/a
echo b > tree/sub/b
echo c > tree/sub/deep/c
echo n > tree/sub/deep/noattr
echo o > outside
ln -s ../outside tree/lnk
ln -s sub tree/dlnk
echo f > one
echo g > two

# --- help and usage ---
aout=$($XATTR -h 2>&1); arc=$?
case "$aout" in *"-r: act recursively"*"-s: act on the symbolic link"*) [ $arc = 0 ] && ok || bad "-h exit" 0 $arc ;; *) bad "-h text" "usage with -r and -s" "$aout" ;; esac
runnz "-p -d conflict" "usage:" $XATTR -dp com.vd.k one
runnz "-w -c conflict" "usage:" $XATTR -wc com.vd.k v one
runnz "unknown option" "usage:" $XATTR -Z one

# --- list / -v / -l / -p ---
run "write" 0 "" $XATTR -w com.vd.k v1 one
run "list" 0 "com.vd.k" $XATTR one
run "list -v" 0 "one: com.vd.k" $XATTR -v one
run "list two files" 0 "one: com.vd.k" $XATTR one two
run "list -l" 0 "com.vd.k: v1" $XATTR -l one
run "print" 0 "v1" $XATTR -p com.vd.k one
run "print -v" 0 "one: v1" $XATTR -pv com.vd.k one
run "print two files" 1 "xattr: two: No such xattr: com.vd.k
one: v1" $XATTR -p com.vd.k one two
run "print -l" 0 "com.vd.k: v1" $XATTR -pl com.vd.k one
run "overwrite" 0 "" $XATTR -w com.vd.k v2 one
run "print overwritten" 0 "v2" $XATTR -p com.vd.k one

# --- missing attribute / missing file ---
run "print missing attr" 1 "xattr: one: No such xattr: com.vd.nope" $XATTR -p com.vd.nope one
run "delete missing attr" 1 "xattr: one: No such xattr: com.vd.nope" $XATTR -d com.vd.nope one
run "delete missing attr, 2 files" 1 "xattr: one: No such xattr: com.vd.nope
xattr: two: No such xattr: com.vd.nope" $XATTR -d com.vd.nope one two
run "print missing file" 1 "xattr: No such file: nofile" $XATTR -p com.vd.k nofile
run "delete missing file" 1 "xattr: No such file: nofile" $XATTR -d com.vd.k nofile
run "list missing file" 1 "xattr: No such file: nofile" $XATTR nofile
run "clear missing file" 1 "xattr: No such file: nofile" $XATTR -c nofile

# --- hex (-x) ---
run "write hex" 0 "" $XATTR -wx com.vd.bin 41424344 one
run "print hex" 0 "41 42 43 44" $XATTR -px com.vd.bin one
run "print text of hex value" 0 "ABCD" $XATTR -p com.vd.bin one
run "print -lx" 0 "com.vd.bin:
00000000  41 42 43 44                                      |ABCD|
00000004" $XATTR -plx com.vd.bin one
run "write hex with spaces" 0 "" $XATTR -wx com.vd.bin "de ad be ef" one
run "print hex 2" 0 "DE AD BE EF" $XATTR -px com.vd.bin one

# --- delete / clear ---
run "delete" 0 "" $XATTR -d com.vd.bin one
run "list after delete" 0 "com.vd.k" $XATTR one
run "clear" 0 "" $XATTR -c one
run "list after clear" 0 "" $XATTR one

# --- recursion ---
run "write -r" 0 "" $XATTR -rw com.vd.k top tree
run "list -r" 0 "tree: com.vd.k
tree/a: com.vd.k
tree/sub: com.vd.k
tree/sub/b: com.vd.k
tree/sub/deep: com.vd.k
tree/sub/deep/c: com.vd.k
tree/sub/deep/noattr: com.vd.k
tree/lnk: com.vd.k
tree/dlnk: com.vd.k" $XATTR -r tree
run "list -r wrote through file symlink" 0 "com.vd.k" $XATTR outside
run "print -r" 0 "tree: top
tree/a: top
tree/sub: top
tree/sub/b: top
tree/sub/deep: top
tree/sub/deep/c: top
tree/sub/deep/noattr: top
tree/lnk: top
tree/dlnk: top" $XATTR -rp com.vd.k tree
run "list -r single file shows name only with -v" 0 "tree/a: com.vd.k" $XATTR -rv tree/a
run "print -rl" 0 "tree/a: com.vd.k: top" $XATTR -rpl com.vd.k tree/a
# Selective removal in the middle of the tree, then -r of a partly missing attribute.
run "delete one" 0 "" $XATTR -d com.vd.k tree/sub/deep/noattr
run "print -r with missing attr below" 1 "tree/sub/deep: top
tree/sub/deep/c: top
xattr: tree/sub/deep/noattr: No such xattr: com.vd.k" $XATTR -rp com.vd.k tree/sub/deep
run "delete -r skips entries without the attribute" 0 "" $XATTR -rd com.vd.k tree
run "list -r after delete" 0 "" $XATTR -r tree
run "list through dir symlink -r" 0 "" $XATTR -r tree/dlnk
run "write -r through dir symlink argument" 0 "" $XATTR -rw com.vd.k viadir tree/dlnk
run "dir symlink target is not descended by -r in a tree" 0 "tree/sub: com.vd.k
tree/dlnk: com.vd.k" $XATTR -r tree
run "clear -r" 0 "" $XATTR -rc tree
run "list -r after clear" 0 "" $XATTR -r tree

# --- -s (act on the symlink itself) ---
run "write through link" 0 "" $XATTR -w com.vd.k target tree/lnk
run "list link follows" 0 "com.vd.k" $XATTR tree/lnk
run "list link with -s is empty" 0 "" $XATTR -s tree/lnk
run "-rs on a tree without attributes" 0 "" $XATTR -rs tree
$XATTR -w com.vd.k x tree/a >/dev/null 2>&1
run "-rs lists only real files" 0 "tree/a: com.vd.k" $XATTR -rs tree
run "-rcs" 0 "" $XATTR -rcs tree
run "-rs after clear" 0 "" $XATTR -rs tree

# --- dangling symlink ---
ln -s nowhere dang
run "dangling follows -> no such file" 1 "xattr: No such file: dang" $XATTR dang
run "dangling -s lists nothing" 0 "" $XATTR -s dang
run "dangling in -r tree" 1 "xattr: No such file: other/dang" sh -c "ln -s nowhere other/dang; $XATTR -r other"

# --- usage errors ---
runnz "no attr_name" "usage:" $XATTR -p
runnz "no attr_value" "usage:" $XATTR -w com.vd.k

if [ $fails = 0 ]; then
    echo "PASS: $checks checks"
    exit 0
fi
echo "FAIL: $fails of $checks checks"
exit 1
