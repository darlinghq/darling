#!/usr/bin/env python3
# Host contract test for the PipeWire CoreAudio volume/mute boundary.
#
# Public contract sources:
#   https://developer.apple.com/documentation/coreaudio/kaudiodevicepropertyvolumescalar
#   https://developer.apple.com/documentation/coreaudio/kaudiodevicepropertymute
#   https://docs.pipewire.org/page_man_pipewire.html
#
# The HAL must not keep volume and mute as process-local settings. It asks the
# shared PipeWire client for the server-selected default sink/source state and
# asks the server to apply changes, then mirrors server notifications to CoreAudio
# property listeners.
#
# usage: python3 src/libpipewire/tests/audio_server_state_contract_host.py [REPO_ROOT]

import pathlib
import re
import sys

root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '.').resolve()
files = {
    'header': root / 'src/libpipewire/include/pipe_client.h',
    'client': root / 'src/libpipewire/pipe_client.c',
    'hal_header': root / 'src/CoreAudio/CoreAudio/pipewire/AudioHardwareImplPW.h',
    'hal': root / 'src/CoreAudio/CoreAudio/pipewire/AudioHardwareImplPW.cpp',
}

text = {}
for name, path in files.items():
    if not path.is_file():
        sys.exit(f'FAIL: missing {path}')
    text[name] = path.read_text(encoding='utf-8')

failures = []


def need(name, pattern, description):
    if re.search(pattern, text[name], re.MULTILINE) is None:
        failures.append(f'{description} ({files[name]})')


def reject(name, pattern, description):
    if re.search(pattern, text[name], re.MULTILINE) is not None:
        failures.append(f'{description} ({files[name]})')


# The shared client follows the server metadata's default audio nodes and
# exchanges their SPA_PROP_volume / SPA_PROP_mute properties with the server.
need('header', r'darling_pw_audio_state_create\s*\(', 'server audio-state constructor')
need('header', r'darling_pw_audio_state_get\s*\(', 'server audio-state getter')
need('header', r'darling_pw_audio_state_set_volume\s*\(', 'server volume setter')
need('header', r'darling_pw_audio_state_set_mute\s*\(', 'server mute setter')
need('client', r'"default\.audio\.sink"', 'default sink metadata key')
need('client', r'"default\.audio\.source"', 'default source metadata key')
need('client', r'SPA_PROP_volume', 'server volume property')
need('client', r'SPA_PROP_mute', 'server mute property')
need('client', r'pw_node_set_param\s*\(', 'server node parameter write')
need('client', r'pw_node_subscribe_params\s*\(', 'server node parameter subscription')

# The CoreAudio HAL reads and writes through that boundary and relays server
# notifications. It must not synthesize set-only local control state.
need('hal_header', r'darling_pw_audio_state\s*\*', 'HAL owns a server audio-state monitor')
need('hal', r'darling_pw_audio_state_create\s*\(', 'HAL creates the monitor')
need('hal', r'darling_pw_audio_state_destroy\s*\(', 'HAL releases the monitor')
need('hal', r'darling_pw_audio_state_get\s*\(', 'HAL reads volume/mute from the server')
need('hal', r'darling_pw_audio_state_set_volume\s*\(', 'HAL asks the server to set volume')
need('hal', r'darling_pw_audio_state_set_mute\s*\(', 'HAL asks the server to set mute')
need('hal', r'notifyPropertyChanged\s*\(\s*kAudioDevicePropertyVolumeScalar',
     'HAL notifies volume listeners from server changes')
need('hal', r'notifyPropertyChanged\s*\(\s*kAudioDevicePropertyMute',
     'HAL notifies mute listeners from server changes')

if failures:
    for failure in failures:
        print('FAIL:', failure)
    sys.exit(1)
print('PASS: audio server-state contract')
