#!/usr/bin/env python3
# Copyright (C) 2026 Darling Developers
"""Host-side source checks for Cocoa APIs that applications declare in their sources.

These tests compile small translation units against the SDK in this checkout;
they need clang and initialized submodules, but no built Darling prefix. They
pin the source compatibility of the API surface; runtime behavior of the
AppKit additions is covered by tests/appkit-app-api-compat.m, which is run
under Darling.

Usage: tests/app-api-compat.py   (CLANG=/path/to/clang to choose the compiler)
"""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SDK = Path(os.environ.get('SDK', ROOT / 'Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk'))
CLANG = os.environ.get('CLANG', 'clang')
AE_INCLUDE = ROOT / 'src/frameworks/CoreServices/include'
QUARTZ_INCLUDE = ROOT / 'src/frameworks/Quartz/QuickLookUI/include'
COCOTRON_APPKIT_INCLUDE = ROOT / 'src/external/cocotron/AppKit/include'


def sdk_usable():
    return shutil.which(CLANG) is not None and (SDK / 'usr/include/stdio.h').exists()


def cocotron_appkit_usable():
    """True when the checked-out submodule has the AppKit delegate additions."""
    sound_header = COCOTRON_APPKIT_INCLUDE / 'AppKit/NSSound.h'
    toolbar_header = COCOTRON_APPKIT_INCLUDE / 'AppKit/NSToolbarItem.h'
    return (sound_header.exists() and toolbar_header.exists()
            and '@protocol NSSoundDelegate' in sound_header.read_text()
            and '@protocol NSToolbarItemValidation' in toolbar_header.read_text())


@unittest.skipUnless(sdk_usable(), 'clang or the SDK headers (submodules) are not available')
class AppApiCompat(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='darling-app-api-')
        self.addCleanup(self.tmp.cleanup)

    def clang(self, source=None, *extra, language='objective-c', source_file=None):
        if source_file is None:
            suffix = '.mm' if language == 'objective-c++' else '.m'
            source_file = Path(self.tmp.name) / ('probe' + suffix)
            source_file.write_text(source)
        # Clang modules are intentionally off: app builds do not enable them,
        # and the SDK still has unrelated umbrella/module conflicts when several
        # frameworks are combined.
        cmd = [CLANG, '-target', 'x86_64-apple-macos10.15', '-isysroot', str(SDK),
               '-fsyntax-only', '-Wno-everything', '-I', str(AE_INCLUDE), *extra,
               '-x', language, str(source_file)]
        return subprocess.run(cmd, capture_output=True, text=True)

    def test_quicklook_preview_panel_surface(self):
        source = '''\
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <Quartz/Quartz.h>

@interface QuickLookProbe : NSObject <QLPreviewPanelDataSource,
    QLPreviewPanelDelegate>
@end

@implementation QuickLookProbe
- (NSInteger) numberOfPreviewItemsInPreviewPanel: (QLPreviewPanel *) panel { return 0; }
- (id<QLPreviewItem>) previewPanel: (QLPreviewPanel *) panel
                 previewItemAtIndex: (NSInteger) index { return nil; }
@end

void quicklook_probe(NSURL *url) {
    NSWorkspaceOpenConfiguration *configuration = [[NSWorkspaceOpenConfiguration alloc] init];
    [[NSWorkspace sharedWorkspace] openApplicationAtURL: url
                                          configuration: configuration
                                        completionHandler: ^(NSRunningApplication *app, NSError *error) {}];

    QLPreviewPanel *panel = [QLPreviewPanel sharedPreviewPanel];
    QuickLookProbe *probe = [QuickLookProbe new];
    panel.dataSource = probe;
    panel.delegate = probe;
    [panel reloadData];
}
'''
        res = self.clang(source)
        self.assertEqual(res.returncode, 0, res.stderr)

    @unittest.skipUnless(cocotron_appkit_usable(),
                         'cocotron AppKit delegate additions are not in the pinned submodule')
    def test_appkit_delegate_surface(self):
        source = '''\
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

@interface TRAppKitProbe : NSObject <NSSoundDelegate, NSToolbarItemValidation>
@end

@implementation TRAppKitProbe
- (void) sound: (NSSound *) sound didFinishPlaying: (BOOL) successfully {}
@end

void appkit_delegate_probe(void) {
    NSSound *sound = [NSSound soundNamed: @"Ping"];
    sound.delegate = (id<NSSoundDelegate>)nil;
}
'''
        res = self.clang(source)
        self.assertEqual(res.returncode, 0, res.stderr)

    def test_core_image_rotation_surface(self):
        source = '''\
#import <Cocoa/Cocoa.h>
#import <AVFoundation/AVFoundation.h>
#import <AVKit/AVKit.h>
#import <VideoToolbox/VideoToolbox.h>

void rotation_probe(CVPixelBufferRef source, CVPixelBufferRef destination) {
    CIContext *context = [[CIContext alloc] initWithOptions: nil];
    CIImage *image = [[CIImage alloc] initWithCVPixelBuffer: source];
    image = [image imageByApplyingOrientation: kCGImagePropertyOrientationRight];
    [context render: image toCVPixelBuffer: destination];
}
'''
        res = self.clang(source)
        self.assertEqual(res.returncode, 0, res.stderr)

    def test_foundation_and_iokit_surface(self):
        source = '''\
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#import <AVFAudio/AVSpeechSynthesis.h>
#import <IOKit/pwr_mgt/IOPMLib.h>
#import <IOKit/hidsystem/ev_keymap.h>
#import <libproc.h>
#import <mach-o/dyld.h>
#import <os/log.h>

void system_surface_probe(void) {
    NSSavePanel *panel = [NSSavePanel savePanel];
    [panel setAllowedContentTypes: @[ UTTypeData ]];
    UTType *type = [UTType typeWithFilenameExtension: @"txt"];
    (void)type;

    AVSpeechSynthesizer *synthesizer = [[AVSpeechSynthesizer alloc] init];
    [synthesizer pauseSpeakingAtBoundary: AVSpeechBoundaryImmediate];
    AVSpeechUtterance *utterance = [[AVSpeechUtterance alloc] initWithString: @"x"];
    [synthesizer speakUtterance: utterance];

    IOPMAssertionID assertion = kIOPMNullAssertionID;
    IOPMAssertionCreateWithDescription(kIOPMAssertPreventUserIdleDisplaySleep,
                                       CFSTR("a"), CFSTR("b"), CFSTR("c"),
                                       nullptr, 0, nullptr, &assertion);
    IOPMAssertionRelease(assertion);

    char path[1024];
    proc_pidpath(getpid(), path, sizeof(path));
    (void)_dyld_get_image_name(0);
    os_log_t log = os_log_create("dev.darling.probe", "general");
    os_log(log, "probe");
}
'''
        res = self.clang(source, language='objective-c++')
        self.assertEqual(res.returncode, 0, res.stderr)

    def test_imageio_property_header_is_reentrant(self):
        source = '''\
#include <ImageIO/CGImageProperties.h>
#include <ImageIO/CGImageProperties.h>
int main(void) { return kCGImagePropertyOrientationUp == 1 ? 0 : 1; }
'''
        res = self.clang(source)
        self.assertEqual(res.returncode, 0, res.stderr)

    def test_quicklook_implementation_compiles(self):
        path = ROOT / 'src/frameworks/Quartz/QuickLookUI/src/QLPreviewPanel.m'
        res = self.clang(None, '-I', str(QUARTZ_INCLUDE), source_file=path)
        self.assertEqual(res.returncode, 0, res.stderr)

    @unittest.skipUnless(cocotron_appkit_usable(),
                         'cocotron AppKit delegate additions are not in the pinned submodule')
    def test_cocotron_appkit_implementation_compiles(self):
        path = ROOT / 'src/external/cocotron/AppKit/NSSound.m'
        res = self.clang(None, '-I', str(COCOTRON_APPKIT_INCLUDE), source_file=path)
        self.assertEqual(res.returncode, 0, res.stderr)


if __name__ == '__main__':
    unittest.main()
