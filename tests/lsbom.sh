#!/bin/sh
# Copyright (C) 2026 Lubos Dolezel
#
# Guest test for lsbom(1) (listing a bill of materials made by mkbom):
#   - `lsbom -s` prints every path (all levels, "./" prefix) and nothing else
#   - the default listing is tab separated, without a trailing tab, and has the
#     documented fields: name, mode, uid/gid and for files size and checksum,
#     for symbolic links also the link name
#   - -f/-d/-l select entry types, -p picks fields, -x drops dir/link modes
#
# The BOM is generated with mkbom from a small tree, so the script needs no
# fixture. The expected listing formats were recorded black-box on macOS
# 26.7.3 with Apple's lsbom and mkbom; the script passes against them too.
# Only options on which both implementations agree are checked.
#
# Usage: lsbom.sh [path-to-lsbom] [path-to-mkbom]   (default: from PATH)
# Prints PASS/FAIL lines and exits non-zero if anything failed.
LSBOM=${1:-lsbom}
MKBOM=${2:-mkbom}
fail=0
check() { # name, expected, actual
	if [ "$2" = "$3" ]; then echo "PASS $1"; else echo "FAIL $1"; printf 'expected:\n%s\nactual:\n%s\n' "$2" "$3"; fail=1; fi
}
T=$(mktemp -d /tmp/lsbomtest.XXXXXX) || exit 2
trap 'rm -rf "$T"' EXIT
umask 022
mkdir -p "$T/t/sub"
printf 'hi\n' > "$T/t/f.txt"
printf 'yo\n' > "$T/t/sub/g.txt"
chmod 644 "$T/t/f.txt" "$T/t/sub/g.txt"
ln -s f.txt "$T/t/l"
"$MKBOM" "$T/t" "$T/b.bom" || { echo "FAIL mkbom"; exit 1; }

check "lsbom -s lists all paths" ".
./f.txt
./l
./sub
./sub/g.txt" "$("$LSBOM" -s "$T/b.bom")"

check "lsbom -f -s lists files only" "./f.txt
./sub/g.txt" "$("$LSBOM" -f -s "$T/b.bom")"
check "lsbom -d -s lists directories only" ".
./sub" "$("$LSBOM" -d -s "$T/b.bom")"
check "lsbom -l -s lists links only" "./l" "$("$LSBOM" -l -s "$T/b.bom")"

# default listing: tab separated, no trailing tab, one line per entry
tab=$(printf '\t')
out=$("$LSBOM" "$T/b.bom")
check "default listing has 5 lines" 5 "$(printf '%s\n' "$out" | wc -l | tr -d ' ')"
case "$out" in *"$tab") echo "FAIL trailing tab at end of listing"; fail=1;; *) echo "PASS no trailing tab at end";; esac
printf '%s\n' "$out" | grep -q "$tab\$" && { echo "FAIL a line ends with a tab"; fail=1; } || echo "PASS no line ends with a tab"
printf '%s\n' "$out" | grep -Eq "^\.${tab}40755${tab}[0-9]+/[0-9]+\$" && echo "PASS root dir line" || { echo "FAIL root dir line"; fail=1; }
printf '%s\n' "$out" | grep -Eq "^\./sub${tab}40755${tab}[0-9]+/[0-9]+\$" && echo "PASS sub dir line" || { echo "FAIL sub dir line"; fail=1; }
printf '%s\n' "$out" | grep -Eq "^\./f\.txt${tab}100644${tab}[0-9]+/[0-9]+${tab}3${tab}[0-9]+\$" && echo "PASS file line" || { echo "FAIL file line"; fail=1; }
printf '%s\n' "$out" | grep -Eq "^\./l${tab}120[0-9]{3}${tab}[0-9]+/[0-9]+${tab}5${tab}[0-9]+${tab}f\.txt\$" && echo "PASS link line" || { echo "FAIL link line"; fail=1; }

check "lsbom -p fm" ".${tab}40755
./f.txt${tab}100644
./l${tab}$("$LSBOM" -p fm "$T/b.bom" | sed -n 3p | cut -f2)
./sub${tab}40755
./sub/g.txt${tab}100644" "$("$LSBOM" -p fm "$T/b.bom")"
x=$("$LSBOM" -x -d "$T/b.bom")
printf '%s\n' "$x" | grep -Eq "^\.${tab}[0-9]+/[0-9]+\$" && echo "PASS -x drops dir mode" || { echo "FAIL -x drops dir mode"; fail=1; }
if [ $fail = 0 ]; then echo "ALL PASSED"; else echo "SOME FAILED"; exit 1; fi
