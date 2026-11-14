// Copyright (C) 2026 Lubos Dolezel
//
// The Swift spellings the AppKit overlay and applications rely on: nested typed-enum types with their
// members, NS_ENUM cases, and the functions the overlay wraps (imported with a "__" prefix). Values are
// the public ones, so the same file runs against macOS. Typecheck against the SDK with a Foundation
// overlay module; running it needs the AppKit framework.
import AppKit

var failures = 0
func check(_ ok: Bool, _ what: String) {
    if !ok {
        print("FAIL: \(what)")
        failures += 1
    }
}

// Types that Swift nests in their class, and that the overlay treats as numeric values.
func rawNumeric<T: RawRepresentable>(_ type: T.Type) where T.RawValue: Comparable & Numeric {}
rawNumeric(NSAppKitVersion.self)
rawNumeric(NSLayoutConstraint.Priority.self)
rawNumeric(NSStackView.VisibilityPriority.self)
rawNumeric(NSToolbarItem.VisibilityPriority.self)
rawNumeric(NSTouchBarItem.Priority.self)
rawNumeric(NSWindow.Level.self)
rawNumeric(NSFont.Weight.self)

check(NSWindow.Level.normal.rawValue == 0, "NSWindow.Level.normal")
check(NSWindow.Level.floating.rawValue == 3, "NSWindow.Level.floating")
check(NSWindow.Level.statusBar.rawValue > NSWindow.Level.floating.rawValue, "NSWindow.Level.statusBar above floating")
check(NSWindow.Level.screenSaver.rawValue > NSWindow.Level.popUpMenu.rawValue, "NSWindow.Level.screenSaver above popUpMenu")
check(NSStackView.VisibilityPriority.mustHold.rawValue == 1000, "NSStackView.VisibilityPriority.mustHold")
check(NSStackView.VisibilityPriority.notVisible.rawValue == 0, "NSStackView.VisibilityPriority.notVisible")
check(NSToolbarItem.VisibilityPriority.high.rawValue == 1000, "NSToolbarItem.VisibilityPriority.high")
check(NSToolbarItem.VisibilityPriority.user.rawValue == 2000, "NSToolbarItem.VisibilityPriority.user")
check(NSTouchBarItem.Priority.low.rawValue == -1000, "NSTouchBarItem.Priority.low")
check(NSFont.Weight.regular.rawValue == 0, "NSFont.Weight.regular")
check(NSFont.Weight.bold.rawValue > NSFont.Weight.regular.rawValue, "NSFont.Weight.bold above regular")
check(NSAppKitVersion.current.rawValue > 0, "NSAppKitVersion.current")
check(NSColorSpaceName.calibratedRGB.rawValue == "NSCalibratedRGBColorSpace", "NSColorSpaceName.calibratedRGB")
check(NSColorSpaceName.deviceCMYK.rawValue == "NSDeviceCMYKColorSpace", "NSColorSpaceName.deviceCMYK")

// NS_ENUM cases.
check(NSCompositingOperation.clear.rawValue == 0, "NSCompositingOperation.clear")
check(NSCompositingOperation.sourceOver.rawValue == 2, "NSCompositingOperation.sourceOver")
check(NSCompositingOperation.xor.rawValue == 10, "NSCompositingOperation.xor")
check(NSCompositingOperation.luminosity.rawValue == 28, "NSCompositingOperation.luminosity")
check(NSAnimationEffect.poof.rawValue == 10, "NSAnimationEffect.poof")
check(NSOpenGLGlobalOption.formatCacheSize.rawValue == 501, "NSOpenGLGlobalOption.formatCacheSize")
check(NSOpenGLGlobalOption.clearFormatCache.rawValue == 502, "NSOpenGLGlobalOption.clearFormatCache")
check(NSOpenGLGlobalOption.useBuildCache.rawValue == 506, "NSOpenGLGlobalOption.useBuildCache")
check(NSWindow.Depth.twentyfourBitRGB.rawValue == 0x208, "NSWindow.Depth.twentyfourBitRGB")
check(NSWindow.Depth.sixtyfourBitRGB.bitsPerSample == 16, "NSWindow.Depth.bitsPerSample")
check(NSWindow.Depth.onehundredtwentyeightBitRGB.isPlanar == false, "NSWindow.Depth.isPlanar")

// Properties the overlay reads.
func properties(_ event: NSEvent, _ context: NSGraphicsContext) {
    let _: String? = event.charactersIgnoringModifiers
    let _: NSCompositingOperation = context.compositingOperation
    let _: CGContext = context.cgContext
    let _: UnsafeMutableRawPointer = context.graphicsPort
    let _: NSGraphicsContext? = NSGraphicsContext.current
}

// The functions the overlay wraps keep their C shapes under the "__" names.
let rectFill: (NSRect, NSCompositingOperation) -> Void = __NSRectFillUsingOperation
let frameRect: (NSRect, CGFloat, NSCompositingOperation) -> Void = __NSFrameRectWithWidthUsingOperation
let rectClip: (NSRect) -> Void = __NSRectClip
let rectClipList: (UnsafePointer<NSRect>, Int) -> Void = __NSRectClipList
let rectFillList: (UnsafePointer<NSRect>, Int, NSCompositingOperation) -> Void = __NSRectFillListUsingOperation
let rectFillColors: (UnsafePointer<NSRect>, UnsafePointer<NSColor>, Int, NSCompositingOperation) -> Void =
    __NSRectFillListWithColorsUsingOperation
let bestDepth: (NSColorSpaceName, Int, Int, Bool, UnsafeMutablePointer<ObjCBool>?) -> NSWindow.Depth = __NSBestDepth
let availableDepths: () -> UnsafePointer<NSWindow.Depth> = __NSAvailableWindowDepths
let showEffect: (NSAnimationEffect, NSPoint, NSSize, Any?, Selector?, UnsafeMutableRawPointer?) -> Void =
    __NSShowAnimationEffect
let beep: () -> Void = __NSBeep
let openGLGet: (NSOpenGLGlobalOption, UnsafeMutablePointer<GLint>) -> Void = __NSOpenGLGetOption
let openGLSet: (NSOpenGLGlobalOption, GLint) -> Void = __NSOpenGLSetOption
let openGLVersion: (UnsafeMutablePointer<GLint>?, UnsafeMutablePointer<GLint>?) -> Void = __NSOpenGLGetVersion
let appLoad: () -> Bool = __NSApplicationLoad
let appMain: (Int32, UnsafeMutablePointer<UnsafePointer<CChar>>) -> Int32 = __NSApplicationMain

// CGContext methods the overlay calls.
func contextMethods(_ context: CGContext) {
    context.saveGState()
    context.setFillColor(gray: 0.5, alpha: 1)
    context.restoreGState()
}

if failures == 0 {
    print("ALL PASSED")
} else {
    exit(1)
}
