#!/usr/bin/env bash
# tests/coreaudio/fixture-hal.sh
# Usage:
#   ./fixture-hal.sh up    -- loads PulseAudio null devices that differ in rate, channels and properties
#   ./fixture-hal.sh down  -- unloads them
# The devices (name: what it is):
#   darling-hal-48k   sink, 48000 Hz stereo, bus usb, form factor headset
#   darling-hal-44k   sink, 44100 Hz stereo, bus pci
#   darling-hal-51    sink, 32000 Hz 5.1 (6 channels), bus bluetooth
#   darling-hal-int   sink, 48000 Hz stereo, bus pci, form factor internal
#   darling-hal-soft  sink, 48000 Hz stereo, no bus (software device)
#   darling-hal-mic   source, 16000 Hz mono, no bus (PipeWire ignores the bus of a pipe source)
#   darling-hal-line  source, 44100 Hz stereo, no bus
# The sources are pipe sources (module-pipe-source), as PipeWire's PulseAudio server has no null source.
# The modules loaded are remembered in a file of the runtime directory, so that down unloads exactly them.
set -euo pipefail

STATE="${XDG_RUNTIME_DIR:-/tmp}/darling-hal-fixture.modules"
PIPES="${XDG_RUNTIME_DIR:-/tmp}/darling-hal-fixture.pipes"

load() {
	local id
	id=$(pactl load-module "$@")
	echo "$id" >> "$STATE"
}

up() {
	if [ -s "$STATE" ]; then
		echo "fixture is already up" >&2
		exit 1
	fi
	: > "$STATE"
	load module-null-sink sink_name=darling-hal-48k rate=48000 channels=2 channel_map=front-left,front-right \
		sink_properties="device.description=HalSink48k device.bus=usb device.form_factor=headset device.vendor.name=DarlingVendor device.product.name=DarlingUSBHeadset device.serial=SN-48K"
	load module-null-sink sink_name=darling-hal-44k rate=44100 channels=2 channel_map=front-left,front-right \
		sink_properties="device.description=HalSink44k device.bus=pci"
	load module-null-sink sink_name=darling-hal-51 rate=32000 channels=6 channel_map=surround-51 \
		sink_properties="device.description=HalSink51 device.bus=bluetooth"
	load module-null-sink sink_name=darling-hal-int rate=48000 channels=2 channel_map=front-left,front-right \
		sink_properties="device.description=HalSinkInternal device.bus=pci device.form_factor=internal"
	load module-null-sink sink_name=darling-hal-soft rate=48000 channels=2 channel_map=front-left,front-right \
		sink_properties="device.description=HalSinkSoft"
	mkdir -p "$PIPES"
	mkfifo "$PIPES/mic" "$PIPES/line"
	load module-pipe-source source_name=darling-hal-mic file="$PIPES/mic" format=s16le rate=16000 channels=1 channel_map=mono \
		source_properties="device.description=HalMic"
	load module-pipe-source source_name=darling-hal-line file="$PIPES/line" format=s16le rate=44100 channels=2 channel_map=front-left,front-right \
		source_properties="device.description=HalLine"
	# The sound server remembers the volume and mute of a device by its name and restores them when it is loaded
	# again, so a test that changed them (test_hal_external) would hand its state to the next run.
	for s in darling-hal-48k darling-hal-44k darling-hal-51 darling-hal-int darling-hal-soft; do
		pactl set-sink-volume "$s" 100%
		pactl set-sink-mute "$s" 0
	done
	for s in darling-hal-mic darling-hal-line; do
		pactl set-source-volume "$s" 100%
		pactl set-source-mute "$s" 0
	done
	echo "sinks=darling-hal-48k,darling-hal-44k,darling-hal-51,darling-hal-int,darling-hal-soft sources=darling-hal-mic,darling-hal-line"
}

down() {
	if [ -f "$STATE" ]; then
		while read -r id; do
			pactl unload-module "$id" || true
		done < "$STATE"
		rm -f "$STATE"
	fi
	rm -rf "$PIPES"
}

case "${1:-}" in
	up) up ;;
	down) down ;;
	*) echo "usage: $0 up|down" >&2; exit 1 ;;
esac
