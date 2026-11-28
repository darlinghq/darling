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
HOST_CC = os.environ.get('HOST_CC', CLANG)
AE_INCLUDE = ROOT / 'src/frameworks/CoreServices/include'
QUARTZ_INCLUDE = ROOT / 'src/frameworks/Quartz/QuickLookUI/include'
COCOTRON_APPKIT_INCLUDE = ROOT / 'src/external/cocotron/AppKit/include'
COCOTRON_APPKIT_PRIVATE_INCLUDE = ROOT / 'src/external/cocotron/AppKit/private-include'
COCOTRON_APPKIT_ROOT = ROOT / 'src/external/cocotron/AppKit'
COCOTRON_ROOT = ROOT / 'src/external/cocotron'
NSS_INCLUDE = ROOT / 'src/frameworks/NSS/include'
NSPR_INCLUDE = ROOT / 'src/frameworks/NSPR/include'
ELFCALLS_INCLUDE = ROOT / 'src/startup/mldr/elfcalls'


def sdk_usable():
    return shutil.which(CLANG) is not None and (SDK / 'usr/include/stdio.h').exists()


def host_compiler_usable():
    return shutil.which(HOST_CC) is not None


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

    def test_cocotron_modern_appkit_implementations_compile(self):
        sources = [
            'NSItemBadge.m',
            'NSComboButton.m',
            'NSGlassEffectView.m',
            'NSImage.m',
            'NSWindow.m',
            'NSControl.m',
            'NSButton.m',
            'NSSlider.m',
            'NSSegmentedControl/NSSegmentedControl.m',
            'NSToolbar.subproj/NSToolbarItem.m',
            'CoreGraphics/CGS.m',
        ]
        for name in sources:
            path = (COCOTRON_ROOT / name if name.startswith('CoreGraphics/')
                    else COCOTRON_APPKIT_ROOT / name)
            res = self.clang(None, '-I', str(COCOTRON_APPKIT_INCLUDE),
                             '-I', str(COCOTRON_APPKIT_PRIVATE_INCLUDE),
                             '-I', str(COCOTRON_ROOT), source_file=path)
            self.assertEqual(res.returncode, 0, f'{name}: {res.stderr}')

    def test_handbrake_cli_media_surface(self):
        source = '''\
#include <CoreMedia/CoreMedia.h>
#include <CoreVideo/CoreVideo.h>
#include <VideoToolbox/VideoToolbox.h>

void handbrake_cli_probe(void)
{
    CMVideoFormatDescriptionRef desc = NULL;
    (void)CMVideoFormatDescriptionGetCodecType(desc);
    (void)kCMFormatDescriptionFieldDetail_TemporalBottomFirst;
    (void)kCMFormatDescriptionFieldDetail_TemporalTopFirst;
    (void)kCMFormatDescriptionKey_PixelAspectRatioHorizontalSpacing;
    (void)kCMFormatDescriptionKey_PixelAspectRatioVerticalSpacing;
    (void)kCMSampleAttachmentKey_PartialSync;
    (void)kCVImageBufferChromaLocationTopFieldKey;
    (void)kCVImageBufferChromaLocation_Left;
    (void)kCVImageBufferDisplayDimensionsKey;
    (void)kCVImageBufferDisplayHeightKey;
    (void)kCVImageBufferDisplayWidthKey;
    (void)kCVPixelBufferExtendedPixelsBottomKey;
    (void)kCVPixelBufferExtendedPixelsRightKey;
    (void)kCVPixelFormatComponentRange_FullRange;
    (void)kCVPixelFormatComponentRange_VideoRange;
    (void)kCVPixelFormatType_422YpCbCr16BiPlanarVideoRange;
    (void)kCVPixelFormatType_444YpCbCr16BiPlanarVideoRange;
    (void)kCVPixelFormatType_444YpCbCr8BiPlanarFullRange;
    (void)kCVPixelFormatType_444YpCbCr8BiPlanarVideoRange;
    (void)kVTCompressionPropertyKey_AllowTemporalCompression;
    (void)kVTCompressionPropertyKey_ConstantQualityFactor;
    (void)kVTCompressionPropertyKey_FieldCount;
    (void)kVTCompressionPropertyKey_FieldDetail;
    (void)kVTCompressionPropertyKey_H264EntropyMode;
    (void)kVTCompressionPropertyKey_MaxAllowedFrameQP;
    (void)kVTCompressionPropertyKey_MinAllowedFrameQP;
    (void)kVTCompressionPropertyKey_MultiPassStorage;
    (void)kVTCompressionPropertyKey_PreserveDynamicHDRMetadata;
    (void)kVTCompressionPropertyKey_PrioritizeEncodingSpeedOverQuality;
    (void)kVTCompressionPropertyKey_ReferenceBufferCount;
    (void)kVTCompressionPropertyKey_SuggestedLookAheadFrameCount;
    (void)kVTCompressionPropertyKey_VBVBufferDuration;
    (void)kVTCompressionPropertyKey_VBVMaxBitRate;
    (void)kVTCompressionPropertyKey_VBVInitialDelayPercentage;
    (void)kVTCompressionPropertyKey_VariableBitRate;
    (void)kVTCompressionSessionBeginFinalPass;
    (void)kVTH264EntropyMode_CABAC;
    (void)kVTH264EntropyMode_CAVLC;
    (void)kVTPropertySupportedValueListKey;
    (void)kVTUnlimitedFrameDelayCount;
    (void)kVTVideoEncoderSpecification_RequiredEncoderGPURegistryID;

    VTMultiPassStorageRef storage = NULL;
    (void)VTMultiPassStorageCreate(kCFAllocatorDefault, NULL, kCMTimeRangeInvalid, NULL, &storage);
    (void)VTMultiPassStorageGetTypeID();
    (void)VTMultiPassStorageClose(storage);
    (void)VTCompressionSessionBeginPass(NULL, kVTCompressionSessionBeginFinalPass, 0);
    Boolean further = false;
    (void)VTCompressionSessionEndPass(NULL, &further, 0);
    CMItemCount count = 0;
    const CMTimeRange *ranges = NULL;
    (void)VTCompressionSessionGetTimeRangesForNextPass(NULL, &count, &ranges);
}
'''
        res = self.clang(source)
        self.assertEqual(res.returncode, 0, res.stderr)

    def test_qt_cocoa_modern_appkit_surface(self):
        source = '''\
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

@interface QModernAppKitProbe : NSObject <NSMenuValidation>
@end

@implementation QModernAppKitProbe
- (BOOL) validateMenuItem: (NSMenuItem *) item { return YES; }
@end

void qt_cocoa_probe(NSImage *image, NSMenu *menu) {
    NSAccessibilityOrientation orientation = NSAccessibilityOrientationVertical;
    (void)orientation;
    (void)NSAccessibilityOrientationUnknown;
    (void)NSAccessibilityOrientationHorizontal;
    (void)NSWindowDidOrderOnScreenAndFinishAnimatingNotification;
    (void)NSImageHintUserInterfaceLayoutDirection;

    NSImageSymbolConfiguration *configuration =
            [NSImageSymbolConfiguration configurationWithPointSize: 12
                                                            weight: NSFontWeightRegular];
    (void)[image imageByApplyingSymbolConfiguration: configuration];

    NSComboButton *button = [NSComboButton comboButtonWithImage: image
                                                          menu: menu
                                                        target: nil
                                                        action: NULL];
    button.style = NSComboButtonStyleUnified;
    (void)button;

    QModernAppKitProbe *probe = [QModernAppKitProbe new];
    (void)probe;
}
'''
        res = self.clang(source)
        self.assertEqual(res.returncode, 0, res.stderr)

    def test_handbrake_gui_modern_appkit_surface(self):
        source = '''\
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

void handbrake_gui_probe(NSMenu *menu, NSImage *image) {
    NSComboButton *button = [NSComboButton comboButtonWithImage: image
                                                          menu: menu
                                                        target: nil
                                                        action: NULL];
    button.style = NSComboButtonStyleSplit;
    (void)[NSComboButton comboButtonWithTitle: @"Preset"
                                        image: image
                                         menu: menu
                                       target: nil
                                       action: NULL];

    NSItemBadge *badge = [NSItemBadge badgeWithText: @"3"];
    (void)[NSItemBadge badgeWithCount: 3];
    (void)[NSItemBadge indicatorBadge];
    NSToolbarItem *item = [[NSToolbarItem alloc] initWithItemIdentifier: @"probe"];
    item.badge = badge;
    (void)item.badge.text;

    NSGlassEffectView *glass = [[NSGlassEffectView alloc] initWithFrame: NSMakeRect(0, 0, 10, 10)];
    glass.style = NSGlassEffectViewStyleClear;
    glass.cornerRadius = 8;
    glass.tintColor = [NSColor redColor];
    glass.contentView = [[NSView alloc] initWithFrame: NSMakeRect(0, 0, 10, 10)];

    NSButton *standardButton = [[NSButton alloc] initWithFrame: NSMakeRect(0, 0, 10, 10)];
    standardButton.borderShape = NSControlBorderShapeCapsule;
    standardButton.tintProminence = NSTintProminenceNone;
    NSSegmentedControl *segmented = [[NSSegmentedControl alloc] initWithFrame: NSMakeRect(0, 0, 10, 10)];
    segmented.borderShape = NSControlBorderShapeCapsule;
    NSSlider *slider = [[NSSlider alloc] initWithFrame: NSMakeRect(0, 0, 10, 10)];
    slider.tintProminence = NSTintProminenceNone;

    (void)NSControlBorderShapeAutomatic;
    (void)NSControlBorderShapeRoundedRectangle;
    (void)NSControlBorderShapeCircle;
    (void)NSTintProminenceAutomatic;
    (void)NSTintProminencePrimary;
    (void)NSTintProminenceSecondary;
    (void)NSTokenStyleNone;
    (void)NSTokenStylePlainSquared;
    (void)NSVisualEffectMaterialDark;
}
'''
        res = self.clang(source)
        self.assertEqual(res.returncode, 0, res.stderr)

    def test_gecko_nss_nspr_surface(self):
        source = '''\
#include <nss/nss.h>
#include <nspr/nspr.h>

void gecko_crypto_probe(unsigned char *buffer, int length)
{
    PRNetAddr address;
    (void)PR_InitializeNetAddr(PR_IpAddrAny, 443, &address);
    (void)PR_OpenTCPSocket(PR_AF_INET);

    (void)NSS_Init("sql:/tmp/nss-probe");
    (void)NSS_Initialize("sql:/tmp/nss-probe", "", "", "secmod.db", NSS_INIT_READONLY);
    (void)PK11_GenerateRandom(buffer, length);
    (void)CERT_GetDefaultCertDB();
    (void)SSL_ImportFD(NULL, NULL);
    (void)SECITEM_AllocItem(NULL, NULL, 0);
}
'''
        res = self.clang(source, '-I', str(NSS_INCLUDE), '-I', str(NSPR_INCLUDE),
                         language='c')
        self.assertEqual(res.returncode, 0, res.stderr)

    @unittest.skipUnless(host_compiler_usable(),
                         'a host C compiler is required for the shim tests')
    def test_nss_nspr_host_forwarding_contracts(self):
        sources = [
            ROOT / 'tests/host-shim/host_shim.c',
            ROOT / 'src/frameworks/NSS/src/NSSHost.c',
            ROOT / 'src/frameworks/NSPR/src/NSPRHost.c',
        ]
        binary = Path(self.tmp.name) / 'nss-nspr-host-shim'
        cmd = [HOST_CC, '-std=gnu11', '-I', str(NSS_INCLUDE), '-I', str(NSPR_INCLUDE),
               '-I', str(ELFCALLS_INCLUDE), *[str(path) for path in sources],
               '-lpthread', '-o', str(binary)]
        res = subprocess.run(cmd, capture_output=True, text=True)
        self.assertEqual(res.returncode, 0, res.stderr)
        res = subprocess.run([str(binary)], capture_output=True, text=True)
        self.assertEqual(res.returncode, 0, res.stderr)

    @unittest.skipUnless(host_compiler_usable(),
                         'a host C compiler is required for the shim tests')
    def test_nss_nspr_unavailable_host_contracts(self):
        sources = [
            ROOT / 'tests/host-shim/host_shim_unavailable.c',
            ROOT / 'src/frameworks/NSS/src/NSSHost.c',
            ROOT / 'src/frameworks/NSPR/src/NSPRHost.c',
        ]
        binary = Path(self.tmp.name) / 'nss-nspr-unavailable'
        cmd = [HOST_CC, '-std=gnu11', '-I', str(NSS_INCLUDE), '-I', str(NSPR_INCLUDE),
               '-I', str(ELFCALLS_INCLUDE), *[str(path) for path in sources],
               '-lpthread', '-o', str(binary)]
        res = subprocess.run(cmd, capture_output=True, text=True)
        self.assertEqual(res.returncode, 0, res.stderr)
        res = subprocess.run([str(binary)], capture_output=True, text=True)
        self.assertEqual(res.returncode, 0, res.stderr)


if __name__ == '__main__':
    unittest.main()
