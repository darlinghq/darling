#!/bin/bash
# Copyright (C) 2026 Lubos Dolezel
#
# A private PipeWire session for the tests in this directory: its own session bus, PipeWire, WirePlumber
# (hardware monitors off), PipeWire's pulse server, a null sink, a video test source with the Camera
# role, a headless KWin (no lock screen), the permission store and the desktop portal with its KDE backend.
# It never touches the user's session: everything lives in the directory given as <dir> and the pids are
# kept in <dir>/pids.
#
#   session.sh start <dir> [audio]   prints the environment to export for the clients; with `audio` only the
#                                    bus, PipeWire, WirePlumber, the pulse server and the null sink are started
#   session.sh stop <dir>
set -eu
cmd=${1:?usage: session.sh start|stop <dir>}; dir=$(realpath -m "${2:?usage: session.sh start|stop <dir>}")
if [ "$cmd" = stop ]; then
	for p in $(tac "$dir/pids"); do kill "$p" 2>/dev/null || true; done
	exit 0
fi
mkdir -p "$dir"; chmod 700 "$dir"
export XDG_RUNTIME_DIR=$dir XDG_CONFIG_HOME=$dir/cfg XDG_DATA_HOME=$dir/data XDG_CACHE_HOME=$dir/cache XDG_STATE_HOME=$dir/state
export DBUS_SESSION_BUS_ADDRESS=unix:path=$dir/bus XDG_CURRENT_DESKTOP=KDE XDG_SESSION_TYPE=wayland
mkdir -p "$XDG_CONFIG_HOME/pipewire/pipewire.conf.d" "$XDG_CONFIG_HOME/wireplumber/wireplumber.conf.d" "$XDG_DATA_HOME" "$XDG_CACHE_HOME" "$XDG_STATE_HOME"
cat > "$XDG_CONFIG_HOME/pipewire/pipewire.conf.d/10-test.conf" <<X
context.objects = [
 { factory = adapter
   args = { factory.name = support.null-audio-sink
            node.name = pwt-null-sink
            node.description = "PWT null sink"
            media.class = Audio/Sink
            audio.position = [ FL FR ]
            object.linger = true } }
 { factory = spa-node-factory
   args = { factory.name = videotestsrc
            library.name = videotestsrc/libspa-videotestsrc
            node.name = pwt-camera
            node.description = "PWT test camera"
            media.class = Video/Source
            media.role = Camera
            pattern = 0 } }
]
X
cat > "$XDG_CONFIG_HOME/wireplumber/wireplumber.conf.d/90-nohw.conf" <<X
wireplumber.profiles = {
  main = {
    monitor.alsa = disabled
    monitor.alsa-midi = disabled
    monitor.bluez = disabled
    monitor.bluez-midi = disabled
    monitor.v4l2 = disabled
    monitor.libcamera = disabled
  }
}
X
: > "$dir/pids"
dbus-daemon --session --address="$DBUS_SESSION_BUS_ADDRESS" --print-pid --fork >> "$dir/pids"
spawn() { n=$1; shift; "$@" > "$dir/$n.log" 2>&1 < /dev/null & echo $! >> "$dir/pids"; }
spawn pipewire pipewire
for i in $(seq 20); do [ -S "$dir/pipewire-0" ] && break; sleep 0.5; done
spawn wireplumber wireplumber
spawn pipewire-pulse pipewire-pulse
if [ "${3:-}" = audio ]; then
	sleep 1
	echo "export XDG_RUNTIME_DIR=$dir DBUS_SESSION_BUS_ADDRESS=$DBUS_SESSION_BUS_ADDRESS"
	exit 0
fi
spawn kwin kwin_wayland --virtual --no-lockscreen --socket pwt-wl --width 1280 --height 720
for i in $(seq 20); do [ -S "$dir/pwt-wl" ] && break; sleep 0.5; done
spawn permstore /usr/lib/xdg-permission-store
spawn portal-kde /usr/lib/xdg-desktop-portal-kde
sleep 1
spawn portal /usr/lib/xdg-desktop-portal -r
sleep 2
spawn pattern env GDK_BACKEND=wayland WAYLAND_DISPLAY="$dir/pwt-wl" python3 "$(dirname "$(realpath "$0")")/pattern.py"
cat <<X
export XDG_RUNTIME_DIR=$dir DBUS_SESSION_BUS_ADDRESS=$DBUS_SESSION_BUS_ADDRESS WAYLAND_DISPLAY=$dir/pwt-wl XDG_CURRENT_DESKTOP=KDE
X
