#!/bin/sh
# Licence agreements in disk images: the libdarling-dmg API (dmg-license.c) and hdiutil attach (see dmg-license.md).
# Needs DARLING_SKILLS (a darling-skills checkout), DARLING_ROOT (the installed tree) and a running Darling.
set -u

here=$(cd "$(dirname "$0")" && pwd)
work=$(mktemp -d)
gw="/Volumes/SystemRoot$work"
failures=0

pass() { echo "PASS: $1"; }
fail() { echo "FAIL: $1"; failures=$((failures + 1)); }

# runs a shell command line in the guest; stdout, stderr and exit status go to $work/<name>.{out,err,rc}
guest() {
	name=$1
	shift
	timeout -s KILL 120 darling shell /bin/sh -c "{ $1; } >'$gw/$name.out' 2>'$gw/$name.err'; echo \$? >'$gw/$name.rc'" </dev/null >/dev/null 2>&1
}
rc() { cat "$work/$1.rc" 2>/dev/null; }

for f in sla-test sla-short-lpic sla-overcount sla-lines unmount-test; do cp "$here/fixtures/$f.dmg" "$work/$f.dmg"; done

"$DARLING_SKILLS/darling-guest-testing/scripts/guest-cc.sh" -o "$work/dmg-license" "$here/dmg-license.c" \
	-I"$here/../src/external/darling-dmg/include" -L"$DARLING_ROOT/usr/lib/darling" -ldarling-dmg \
	&& { guest api "'$gw/dmg-license' '$gw/sla-test.dmg' '$gw/unmount-test.dmg' '$gw/sla-short-lpic.dmg' '$gw/sla-overcount.dmg'"
	     cat "$work/api.out" "$work/api.err"
	     [ "$(rc api)" = 0 ] && pass "licence agreement API" || fail "licence agreement API"; } \
	|| fail "licence agreement API test does not build"

# hdiutil attach with stdin not a terminal: the agreement text goes to stdout, then one answer is read per line.
# After it, attach prints its table (the whole disk, then the volume with its mount point in the last field).
text="SLA TEST AGREEMENT (English) This is the English test agreement."
image="$gw/sla-test.dmg"
mp="/Volumes/Unmount Test"

# attach <name> <answer lines as printf format> [hdiutil options]: runs hdiutil attach with that input and records
# whether the volume got mounted ($work/<name>.mounted), then detaches it again
attach() {
	case_name=$1 answers=$2
	shift 2
	guest "$case_name" "printf '$answers' | hdiutil attach $* '$image'"
	check_and_detach "$case_name"
}
check_and_detach() {
	guest "$1-check" "test -f '$mp/hello.txt' && echo yes || echo no; ps ax | grep -c '[d]arling-dmg-mount'"
	sed -n 1p "$work/$1-check.out" > "$work/$1.mounted"
	sed -n 2p "$work/$1-check.out" > "$work/$1.helpers"
	guest "$1-detach" "hdiutil detach -quiet '$mp'"
}
mounted() { cat "$work/$1.mounted"; }

canceled() { # <name> <description>: text printed, attach canceled, exit 1, nothing mounted, no helper left
	[ "$(rc "$1")" = 1 ] && [ "$(cat "$work/$1.out")" = "$text" ] && [ "$(cat "$work/$1.err")" = "hdiutil: attach canceled" ] \
		&& [ "$(mounted "$1")" = no ] && [ "$(cat "$work/$1.helpers")" = 0 ] \
		&& pass "$2" || fail "$2: rc $(rc "$1"), mounted $(mounted "$1"), helpers $(cat "$work/$1.helpers"), out: $(cat "$work/$1.out"), err: $(cat "$work/$1.err")"
}
agreed() { # <name> <description>: text printed, then the mount point; exit 0, mounted
	[ "$(rc "$1")" = 0 ] && [ "$(sed -n 1p "$work/$1.out")" = "$text" ] && [ "$(tail -n 1 "$work/$1.out" | cut -f 3)" = "$mp" ] \
		&& [ "$(mounted "$1")" = yes ] \
		&& pass "$2" || fail "$2: rc $(rc "$1"), mounted $(mounted "$1"), out: $(cat "$work/$1.out"), err: $(cat "$work/$1.err")"
}

guest eof "hdiutil attach '$image' </dev/null"
check_and_detach eof
canceled eof "no answer: canceled"

attach y 'y\n';          agreed y "answer y"
attach Yes 'Yes\n';      agreed Yes "answer Yes"
attach spacey ' y\n';    agreed spacey "answer ' y'"
attach xy 'x\ny\n';      agreed xy "answer x, then y"
attach n 'n\n';          canceled n "answer n"
attach upperN 'N\n';     canceled upperN "answer N"
attach empty '\n';       canceled empty "empty answer"
attach agree 'agree\n';  canceled agree "answer agree, then nothing"
attach xn 'x\nn\n';      canceled xn "answer x, then n"

attach quiet 'y\n' -quiet
[ "$(rc quiet)" = 1 ] && [ ! -s "$work/quiet.out" ] && [ ! -s "$work/quiet.err" ] && [ "$(mounted quiet)" = no ] \
	&& pass "-quiet does not agree" || fail "-quiet: rc $(rc quiet), mounted $(mounted quiet), out: $(cat "$work/quiet.out"), err: $(cat "$work/quiet.err")"

attach plistn '' -plist; canceled plistn "-plist, no answer"
attach plisty 'y\n' -plist
[ "$(rc plisty)" = 0 ] && [ "$(sed -n 1p "$work/plisty.out")" = "$text" ] && [ "$(sed -n 2p "$work/plisty.out")" = '<?xml version="1.0" encoding="UTF-8"?>' ] \
	&& grep -q "<string>$mp</string>" "$work/plisty.out" && [ "$(mounted plisty)" = yes ] \
	&& pass "-plist, answer y: text, then the plist" || fail "-plist, answer y: rc $(rc plisty), out: $(cat "$work/plisty.out")"

for f in sla-short-lpic sla-overcount; do
	guest "$f" "hdiutil attach '$gw/$f.dmg' </dev/null"
	[ "$(rc "$f")" = 0 ] && [ "$(wc -l < "$work/$f.out")" = 2 ] && [ "$(tail -n 1 "$work/$f.out" | cut -f 3)" = "$mp" ] \
		&& pass "$f: no agreement, attached" \
		|| fail "$f: rc $(rc "$f"), out: $(cat "$work/$f.out"), err: $(cat "$work/$f.err")"
	guest "$f-detach" "hdiutil detach -quiet '$mp'"
done

# line breaks: one becomes a space, two or more a paragraph break; macOS prints "A B\n\nC D\n\n"
guest lines "hdiutil attach '$gw/sla-lines.dmg' </dev/null"
printf 'A B\n\nC D\n\n' > "$work/lines.expected"
cmp -s "$work/lines.out" "$work/lines.expected" && pass "line breaks in the text" \
	|| fail "line breaks in the text: $(od -c "$work/lines.out" | head -3)"
guest lines-detach "hdiutil detach -quiet '$mp'"

rm -rf "$work"
[ "$failures" = 0 ]
