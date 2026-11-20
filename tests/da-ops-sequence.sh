#!/bin/sh
# Runs in the guest, for diskarbitration.sh: the unmount, eject and approval cases of da-ops.c, in the order the macOS
# observations in diskarbitration.md were made. Prints "<case>: <result>" lines.
# Usage: da-ops-sequence.sh <directory with da-ops, da-disks and unmount-test.dmg>
dir=$1
ops="$dir/da-ops"
mp="/Volumes/Unmount Test"

status() { "$ops" "$@" | tr '\n' ' ' | sed 's/ $//'; }
attach() {
	hdiutil attach -quiet "$dir/unmount-test.dmg" || { echo "attach: failed"; exit 1; }
	leaf=$("$dir/da-disks" --name "$mp")
	whole=${leaf%s1}
}

attach
echo "eject leaf: $(status eject "$leaf")"
echo "unmount unknown leaf: $(status unmount "${whole}s9" 0)"
echo "eject whole while mounted: $(status eject "$whole")"
echo "unmount with a busy file: $(status busy-file "$mp")"
echo "mount mounted leaf: $(status mount "$leaf")"

"$ops" deny 30 >"$dir/deny.log" &
deny=$!
sleep 2
echo "unmount, dissenter: $(status unmount "$leaf" 0)"
echo "whole unmount, dissenter: $(status unmount "$whole" 1)"
echo "eject, dissenter: $(status eject "$whole")"
echo "forced unmount, dissenter: $(status unmount "$leaf" 0x80000)"
test -e "$mp" && echo "mount point after unmount: still there" || echo "mount point after unmount: gone"
kill $deny
wait $deny 2>/dev/null
grep -q "approval asked for $leaf\$" "$dir/deny.log" && echo "leaf approval asked: yes" || echo "leaf approval asked: no"
grep -q "approval asked for $whole\$" "$dir/deny.log" && echo "whole approval asked: yes" || echo "whole approval asked: no"

echo "unmount again: $(status unmount "$leaf" 0)"
echo "mount unmounted leaf: $(status mount "$leaf")"
echo "whole unmount, nothing mounted: $(status unmount "$whole" 1)"
echo "eject: $(status eject "$whole")"
echo "eject again: $(status eject "$whole")"

# an approver that never answers delays a request by the approval timeout, not forever
attach
"$ops" hang 60 >/dev/null &
hang=$!
sleep 2
start=$(date +%s)
result=$(status unmount "$leaf" 0)
echo "unmount, hanging approver: $result in $(( $(date +%s) - start )) s"
kill $hang
"$ops" eject "$whole" >/dev/null

# an approver that was killed does not delay anything
attach
"$ops" deny 60 >/dev/null &
deny=$!
sleep 2
kill -KILL $deny
start=$(date +%s)
result=$(status unmount "$leaf" 0)
echo "unmount, killed approver: $result in $(( $(date +%s) - start )) s"
"$ops" eject "$whole" >/dev/null

# the volume goes away while its unmount waits for approval: it is unmounted with a bare unmount(2). (Killing the
# image's FUSE process would not do: Linux keeps its mount, disconnected.)
attach
"$ops" hang 60 >/dev/null &
hang=$!
sleep 2
start=$(date +%s)
"$ops" unmount "$leaf" 0 >"$dir/vanish.out" &
unmount=$!
sleep 2
"$dir/da-disks" --unmount "$mp" >/dev/null
wait $unmount
echo "unmount, volume vanished: $(tr '\n' ' ' < "$dir/vanish.out" | sed 's/ $//') in $(( $(date +%s) - start )) s"
echo "disk after vanishing: $(status describe "$leaf")"
kill $hang
