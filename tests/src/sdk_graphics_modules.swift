import CoreGraphics
import CoreText
import QuartzCore

// The three frameworks import as Clang modules straight from the SDK.
let layer = CALayer()
layer.bounds = CGRect(x: 0, y: 0, width: 20, height: 10)
let space = CGColorSpaceCreateDeviceRGB()
let font = CTFontCreateWithName("Helvetica" as CFString, 12, nil)
let size = CTFontGetSize(font)
if layer.bounds.size.width == 20 && CGColorSpaceGetNumberOfComponents(space) == 3 && size == 12 {
    print("ALL PASSED")
} else {
    print("FAIL: \(layer.bounds) \(size)")
    exit(1)
}
