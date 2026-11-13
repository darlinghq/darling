import AppKit
import OpenGL

var failures = 0
func check(_ ok: Bool, _ what: String) {
    if !ok {
        print("FAIL: \(what)")
        failures += 1
    }
}

// AppKit and OpenGL import as Clang modules straight from the SDK, AppKit's
// NSLayoutConstraint categories included.
let view: NSView = NSView(frame: NSRect(x: 0, y: 0, width: 20, height: 10))
check(view.frame.size.width == 20 && view.frame.size.height == 10, "NSView frame \(view.frame)")
let constraint: NSLayoutConstraint = NSLayoutConstraint(item: view, attribute: .width, relatedBy: .equal,
                                    toItem: nil, attribute: .notAnAttribute, multiplier: 1, constant: 20)
check(constraint.constant == 20 && constraint.firstAttribute == .width, "NSLayoutConstraint factory")
check(NSLayoutConstraint.Priority.required.rawValue == 1000, "NSLayoutConstraint.Priority.required")
check(GL_TEXTURE_2D == 0x0DE1, "GL_TEXTURE_2D")

if failures == 0 {
    print("ALL PASSED")
} else {
    exit(1)
}
