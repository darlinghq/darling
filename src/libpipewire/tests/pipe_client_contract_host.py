#!/usr/bin/env python3
# Host contract test for the shared PipeWire client boundary.
#
# The HAL exposes server-side volume and mute through CoreAudio; this client is
# the process-wide owner of that state. The screen and camera clients must use
# the portal connection's core, and audio buffers must use negotiated layouts.

import os
import re
import sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else "."
CLIENT = os.path.join(ROOT, "src/libpipewire/pipe_client.c")
HEADER = os.path.join(ROOT, "src/libpipewire/include/pipe_client.h")
CMAKE = os.path.join(ROOT, "src/libpipewire/CMakeLists.txt")
HAL = os.path.join(ROOT, "src/CoreAudio/CoreAudio/pipewire/AudioHardwareImplPW.cpp")
PWCONFIG = os.path.join(ROOT, "src/CoreAudio/CoreAudio/pipewire/PWConfig.cpp")
AUDIOSTREAM = os.path.join(ROOT, "src/CoreAudio/CoreAudio/pipewire/AudioHardwareStreamPW.cpp")
CAMERABACKEND = os.path.join(ROOT, "src/frameworks/AVFoundation/src/camera/DarlingCameraBackend.m")


def read(path):
    with open(path, encoding="utf-8") as f:
        return f.read()


def need(name, pattern, text):
    if not re.search(pattern, text, re.MULTILINE):
        print(f"FAIL: {name}")
        raise SystemExit(1)


client = read(CLIENT)
header = read(HEADER)
cmake = read(CMAKE)
hal = read(HAL)
pwconfig = read(PWCONFIG)
audiostream = read(AUDIOSTREAM)
camerabackend = read(CAMERABACKEND)

# The PipeWire remote fd is a connection handle: it must be used to construct
# the core used by the stream, not silently replaced by a fresh connection.
need("portal core", r"pw_stream_new\s*\(\s*stream->core", client)
need("portal fd ownership", r"close\(remote_fd\)", client)

# Audio clients can handle several PCM layouts; the shared client must expose
# which one it negotiated rather than assuming interleaved float32 everywhere.
need("format enum", r"enum darling_pw_audio_format", header)
need("audio format field", r"enum darling_pw_audio_format audio_format", header)
for fmt in ("U8", "S16", "S24", "S32", "F32"):
    need("audio format " + fmt, r"DARLING_PW_AUDIO_FORMAT_" + fmt, header)
    need("audio format mapping " + fmt, r"DARLING_PW_AUDIO_FORMAT_" + fmt, client)
    need("ASBD mapping " + fmt, r"DARLING_PW_AUDIO_FORMAT_" + fmt, pwconfig)
need("interleaved only", r"kAudioFormatFlagIsNonInterleaved", pwconfig)
need("integer sample sizes", r"case 16:", pwconfig)
need("24-bit sample size", r"case 24:", pwconfig)

# Volume and mute are properties of the PipeWire node, never cached by one
# framework. The shared client reads/writes node Props and the HAL delegates.
need("device state api", r"darling_pw_device_get_state", header)
need("device state setter", r"darling_pw_device_set_state", header)
need("node props subscription", r"SPA_PARAM_Props", client)
need("volume prop", r"SPA_PROP_volume", client)
need("mute prop", r"SPA_PROP_mute", client)
need("server set param", r"pw_node_set_param", client)
need("hal get volume", r"darling_pw_device_get_state", hal)
need("hal set volume", r"darling_pw_device_set_state", hal)
if "m_volume" in hal or "m_mute" in hal:
    print("FAIL: HAL caches local volume/mute state")
    raise SystemExit(1)

# A connected stream can be paused until activated. Waiting only for STREAMING
# makes device startup fail for perfectly valid, inactive graph nodes.
need("paused wait", r"PW_STREAM_STATE_PAUSED", client)
# Negotiation alone does not move a graph node out of the paused state. A
# streaming client must activate its stream before waiting for buffers.
need("audio activation", r"darling_pw_stream_set_active\(m_stream, 1\)", audiostream)
need("camera activation", r"darling_pw_stream_set_active\(m_stream, 1\)", camerabackend)

# The client must be a single dylib installed where the frameworks can find it;
# static linking would create one portal/session state per consumer dylib.
need("shared library", r"add_darling_library\(darling_pipewire_client SHARED", cmake)
need("install name", r"/usr/lib/darling/libdarling_pipewire_client\.dylib", cmake)
need("install rule", r"install\(TARGETS darling_pipewire_client", cmake)

print("PASS: PipeWire client contract")
