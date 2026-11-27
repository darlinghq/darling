#!/bin/bash
# Copyright (C) 2026 Lubos Dolezel
# NSSound playback in the private PipeWire session (own session bus, PipeWire, WirePlumber without hardware
# monitors, the null sink; no compositor): runs nssound_play (behaviour, delegate order and timing) and then the
# level check (two tones recorded from the null sink).
#   [NSSOUND_BACKEND=pulse] nssound_run.sh <darling> <nssound_play binary> <scratch dir>
# With NSSOUND_BACKEND=pulse the guest cannot reach PipeWire's own socket and its HAL falls back to the pulse
# backend, which then talks to PipeWire's pulse server. The report says which kind of stream played.
# <darling> is the launcher to use, with DPREFIX in the environment. Programs are run with `timeout -s KILL`.
set -eu
darling=$1; prog=$(realpath "$2"); dir=$(realpath -m "$3")
here=$(dirname "$(realpath "$0")")
"$here/session.sh" start "$dir" audio > "$dir.env"
trap '"$here/session.sh" stop "$dir"' EXIT
. "$dir.env"
env=$(cut -d' ' -f2- "$dir.env" | tr '\n' ' ')
if [ "${NSSOUND_BACKEND:-pipewire}" = pulse ]; then
	env="$env PIPEWIRE_REMOTE=$dir/no-such-socket PULSE_SERVER=unix:$dir/pulse/native"
fi
gdir=/Volumes/SystemRoot$dir
echo "== behaviour"
rc=0
timeout -s KILL 90 "$darling" shell env $env "/Volumes/SystemRoot$prog" "$gdir" > "$dir/behaviour.log" 2>&1 < /dev/null || rc=$?
cat "$dir/behaviour.log"
echo "== levels"
pw-record --target pwt-null-sink -P stream.capture.sink=true --format s16 --rate 48000 --channels 2 "$dir/levels.wav" &
rec=$!
sleep 1
timeout -s KILL 60 "$darling" shell env $env "/Volumes/SystemRoot$prog" "$gdir" levels > "$dir/levels.log" 2>&1 < /dev/null &
play=$!
sleep 2.5
pw-dump > "$dir/dump.json" || true
wait $play || rc=$?
if grep -q '"pulse.server.type"' "$dir/dump.json"; then echo "playback stream: through the pulse server"; else echo "playback stream: native PipeWire"; fi
cat "$dir/levels.log"
sleep 0.5
kill $rec; wait $rec 2>/dev/null || true
python3 -I "$here/nssound_level.py" "$dir/levels.wav" || rc=1
exit $rc
