#!/bin/bash
# Runs the guest test of HID devices with their own report descriptors (hid_real_descriptor) against a
# virtual HID device made through uhid.
#
# usage: run-uhid.sh <guest test binary> <f310|ids|kbd>
#
# Like run.sh the test (inside Darling) prints "SYNC: <tag>" lines and waits for the file
# <controldir>/go-<tag>. This script (on the host) makes the device with uhid (built from uhid.c, writing
# /dev/uhid, which needs permission), feeds it the reports of each step and touches the go file. The
# host user must be able to read and write the new /dev/input/event node (the LEDs of a keyboard are
# written to it): set HID_NODE_HOOK to a command that is run with the path of the new node and grants
# access (for example a script that calls sudo chmod). HID_GUEST_WRAP, if set, is a command prefix for the
# guest run (for example a systemd-run scope with a memory limit). HID_LOCK, if set, is a lock file held
# while the virtual device exists (virtual devices are visible to everything on the host).
set -u
BIN=$1; KIND=${2:-f310}
HERE=$(cd "$(dirname "$0")" && pwd)
UHID=${UHID:-$HERE/uhid}
[ -x "$UHID" ] || cc -O1 -o "$UHID" "$HERE/uhid.c" || exit 2
if [ -n "${HID_LOCK:-}" ]; then exec 8>"$HID_LOCK"; flock 8; fi

# The report descriptors, as hex (the same bytes as in the test)
F310=05010904a101a10275089504150026ff00350046ff0009300931093209358102750495012507463b0165140939814265007501950c2501450105091901290c81020600ff750195102501450109018102c0a1027508950746ff0026ff0009029102c0c0
IDS=05010905a1018501093009311581257f750895028102050919012908150025017501950881028502050109320935150026ff037510950281020509190929101500250175019508810285030600ff0920150026ff0075089504910285040600ff092175089502b102c0
KBD=05010906a10175019508050719e029e715002501810295017508810195057501050819012905910295017503910195067508150025ff0507190029ff8100c0
case $KIND in
	f310) DESC=$F310; PID=5600;;
	ids) DESC=$IDS; PID=5601;;
	kbd) DESC=$KBD; PID=5602;;
	*) echo "unknown kind $KIND"; exit 2;;
esac

CT=$(mktemp -d); OUT=$CT/out.txt; FIFO=$CT/events
mkfifo "$FIFO"; exec 9<>"$FIFO"

${HID_GUEST_WRAP:-} darling shell "$BIN" "/Volumes/SystemRoot$CT" "$KIND" </dev/null > "$OUT" 2>&1 &
TP=$!
VP=
ev() { echo "report $*" >&9; }
n=0
while kill -0 $TP 2>/dev/null; do
	sleep 0.2
	tag=$(grep -a "^SYNC: " "$OUT" | sed -n "$((n + 1))p" | cut -d" " -f2)
	[ -z "$tag" ] && continue
	n=$((n + 1))
	case $tag in
		plug) "$UHID" "Vuhid $KIND" 1234 $PID 3 "$DESC" < "$FIFO" > "$CT/uhid.out" 2>&1 & VP=$!; sleep 1.5
			if [ -n "${HID_NODE_HOOK:-}" ]; then
				for d in /sys/class/input/event*; do
					grep -q '^Vuhid' "$d/device/name" && "$HID_NODE_HOOK" "/dev/input/$(basename "$d")"
				done
			fi;;
		report) ev 80 20 e0 40 53 81 a5 5a;;
		hat[0-7]) ev 80 80 80 80 0${tag#hat} 00 00 00;;
		hat-center) ev 80 80 80 80 08 00 00 00;;
		report1) ev 01 f0 20 05;;
		report2) ev 02 ff 02 00 01 81;;
		key-a) ev 00 00 04 00 00 00 00 00;;
		key-a-up) ev 00 00 00 00 00 00 00 00;;
		shift) ev 02 00 00 00 00 00 00 00;;
		shift-up) ev 00 00 00 00 00 00 00 00;;
		led-check) sleep 0.8; grep -q "^output 02" "$CT/uhid.out" && echo "PASS: the Caps Lock LED report reached the device" || echo "FAIL: the Caps Lock LED report reached the device" >> "$CT/driver.out";;
		led-check2) sleep 0.8; grep -q "^output 01" "$CT/uhid.out" && echo "PASS: the Num Lock LED report reached the device" >> "$CT/driver.out" || echo "FAIL: the Num Lock LED report reached the device" >> "$CT/driver.out";;
		unplug) echo quit >&9; sleep 0.3; kill $VP 2>/dev/null; VP=;;
	esac
	touch "$CT/go-$tag"
done
wait $TP; rc=$?
[ -n "$VP" ] && { echo quit >&9; sleep 0.3; kill $VP 2>/dev/null; }
cat "$OUT"
[ -f "$CT/driver.out" ] && cat "$CT/driver.out" && grep -q '^FAIL' "$CT/driver.out" && rc=1
rm -rf "$CT"
exit $rc
