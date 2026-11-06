#!/usr/bin/env bash
# tests/coreaudio/fixture.sh
# Usage:
#   ./fixture.sh up    -- loads a null sink ("darling-test-sink") and a null
#                          source ("darling-test-source"), prints their names
#   ./fixture.sh down  -- unloads both
set -euo pipefail

SINK_NAME="darling-test-sink"
SOURCE_NAME="darling-test-source"

up() {
	pactl load-module module-null-sink sink_name="$SINK_NAME" sink_properties=device.description="DarlingTestSink" >/tmp/darling-test-sink.module
	pactl load-module module-null-source source_name="$SOURCE_NAME" description="DarlingTestSource" >/tmp/darling-test-source.module
	echo "sink=$SINK_NAME source=$SOURCE_NAME"
}

down() {
	if [ -f /tmp/darling-test-sink.module ]; then
		pactl unload-module "$(cat /tmp/darling-test-sink.module)" || true
		rm -f /tmp/darling-test-sink.module
	fi
	if [ -f /tmp/darling-test-source.module ]; then
		pactl unload-module "$(cat /tmp/darling-test-source.module)" || true
		rm -f /tmp/darling-test-source.module
	fi
}

case "${1:-}" in
	up) up ;;
	down) down ;;
	*) echo "usage: $0 up|down" >&2; exit 1 ;;
esac
