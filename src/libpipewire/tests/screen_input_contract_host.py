#!/usr/bin/env python3
# Host contract test for the higher-level screen-capture APIs.
#
# Public contract sources:
#   https://developer.apple.com/documentation/avfoundation/avcapturescreeninput
#   https://developer.apple.com/documentation/avfoundation/avcapturescreeninput/croprect?language=objc
#   https://developer.apple.com/documentation/avfoundation/avcapturescreeninput/scalefactor?language=objc
#   https://developer.apple.com/documentation/avfoundation/avcapturescreeninput/capturescursor?language=objc
#   https://developer.apple.com/documentation/avfoundation/avcapturescreeninput/capturesmouseclicks?language=objc
#   https://developer.apple.com/documentation/avfoundation/avcapturescreeninput/minframeduration?language=objc
#   https://developer.apple.com/documentation/screencapturekit/screencapturekit
#
# A full runtime test needs a Wayland ScreenCast portal, so this checks the
# source-level contract and the layering: AVFoundation and ScreenCaptureKit
# must take their frames from CoreGraphics' CGDisplayStream (which owns the
# ScreenCast/PipeWire backend), not open a private capture stream.
#
# usage: python3 src/libpipewire/tests/screen_input_contract_host.py [REPO_ROOT]

import pathlib
import re
import sys

root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '.').resolve()
files = {
    'header': root / 'src/frameworks/AVFoundation/include/AVFoundation/AVCaptureScreenInput.h',
    'input': root / 'src/frameworks/AVFoundation/src/AVCaptureScreenInput.m',
    'backend': root / 'src/frameworks/AVFoundation/src/screen/DarlingScreenCaptureBackend.m',
    'session': root / 'src/frameworks/AVFoundation/src/AVCaptureSession.m',
    'av_cmake': root / 'src/frameworks/AVFoundation/CMakeLists.txt',
    'sckit': root / 'src/frameworks/ScreenCaptureKit/src/ScreenCaptureKit.m',
    'sckit_header': root / 'src/frameworks/ScreenCaptureKit/include/ScreenCaptureKit/ScreenCaptureKit.h',
    'sckit_cmake': root / 'src/frameworks/ScreenCaptureKit/CMakeLists.txt',
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


# Public AVCaptureScreenInput surface: six documented properties and the
# display-specific initializer.
need('header', r'@interface\s+AVCaptureScreenInput\s*:\s*AVCaptureInput\b',
     'AVCaptureScreenInput must subclass AVCaptureInput')
for prop in ('minFrameDuration', 'cropRect', 'scaleFactor', 'capturesCursor',
             'capturesMouseClicks', 'removesDuplicateFrames'):
    need('header', rf'@property[^;\n]*\b{prop}\b', f'missing public property {prop}')
need('header', r'initWithDisplayID:\s*\(CGDirectDisplayID\)displayID',
     'missing initWithDisplayID: initializer')

# Documented defaults: full display, no scaling, cursor visible, no click
# highlight, no minimum frame duration. The docs do not state a default for
# removesDuplicateFrames, so the conservative default is not to drop frames.
need('input', r'_minFrameDuration\s*=\s*kCMTimeInvalid\s*;', 'minFrameDuration default')
need('input', r'_cropRect\s*=\s*CGRectZero\s*;', 'cropRect default')
need('input', r'_scaleFactor\s*=\s*1\s*;', 'scaleFactor default')
need('input', r'_capturesCursor\s*=\s*YES\s*;', 'capturesCursor default')
need('input', r'_capturesMouseClicks\s*=\s*NO\s*;', 'capturesMouseClicks default')
need('input', r'_removesDuplicateFrames\s*=\s*NO\s*;', 'removesDuplicateFrames default')
need('input', r'CGDisplayBounds\(displayID\)', 'display bounds validate the initializer')

# A screen input exposes a video port and the session starts one shared
# ScreenCast-backed engine for it.
need('input', r'darling_videoPortForScreenInput:\s*self', 'screen input exposes a video port')
need('session', r'isKindOfClass:\s*\[AVCaptureScreenInput class\]',
     'session recognizes screen inputs')
need('session', r'DarlingScreenCaptureEngine alloc\]\s*\n\s*initWithOutput:',
     'session creates a screen capture engine')
need('session', r'screenInput:\s*screenInput', 'session passes the screen input to its engine')
need('session', r'\[engine stop\]', 'session stops capture engines')

# The engine takes frames from CGDisplayStream and applies the documented
# screen-input properties to outgoing sample buffers.
for symbol in ('CGDisplayStreamCreateWithDispatchQueue', 'CGDisplayStreamStart',
               'CGDisplayStreamStop'):
    need('backend', rf'\b{symbol}\s*\(', f'engine must use {symbol}')
if re.search(r'darling_screen_capture', text['backend']):
    failures.append('engine must not open its own screen capture stream')
need('backend', r'CMSampleBufferCreateReadyWithImageBuffer\s*\(',
     'engine must emit CMSampleBuffers')
need('backend', r'bottom-left origin', 'cropRect must use bottom-left screen coordinates')
for expression in (r'\[_screenInput scaleFactor\]',
                   r'\[_screenInput cropRect\]',
                   r'\[_screenInput minFrameDuration\]',
                   r'\[_screenInput removesDuplicateFrames\]'):
    need('backend', expression, f'engine must apply {expression}')

need('av_cmake', r'target_link_libraries\(AVFoundation PRIVATE darling_pipewire_client\)',
     'AVFoundation must link the shared PipeWire client')
if re.search(r'darling_pipewire_client', text['sckit_cmake']):
    failures.append('ScreenCaptureKit must not link the PipeWire client directly')

# ScreenCaptureKit's documented stream-output contract.
need('sckit_header', r'@protocol\s+SCStreamOutput', 'SCStreamOutput protocol')
need('sckit_header', r'didOutputSampleBuffer:\(CMSampleBufferRef\)sampleBuffer\s+ofType:',
     'SCStreamOutput sample-buffer callback')
need('sckit_header', r'@property[^;\n]*isCapturing', 'SCStream.isCapturing')
for symbol in ('SCFrameStatus', 'SCStreamFrameInfo', 'SCContentFilter',
               'SCStreamConfiguration', 'SCDisplay', 'SCWindow'):
    need('sckit_header', rf'\b{symbol}\b', f'ScreenCaptureKit public type {symbol}')
for symbol in ('CGDisplayStreamCreateWithDispatchQueue', 'CGDisplayStreamStart',
               'CGDisplayStreamStop'):
    need('sckit', rf'\b{symbol}\s*\(', f'ScreenCaptureKit must use {symbol}')
if re.search(r'darling_screen_capture', text['sckit']):
    failures.append('ScreenCaptureKit must not open its own screen capture stream')

if failures:
    for failure in failures:
        print('FAIL:', failure)
    sys.exit(1)
print('PASS: screen capture API contract')
