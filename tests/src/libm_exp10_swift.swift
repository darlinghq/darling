import Darwin

// __exp10/__exp10f are declared in the macOS SDK's <math.h>; Swift code (OpenSwiftUI) calls them directly.
let d = __exp10(3.0)
let f = __exp10f(-2)
if d == 1000 && abs(f - 0.01) < 1e-7 && __exp10(0.5) == pow(10, 0.5) {
    print("ALL PASSED")
} else {
    print("FAIL: __exp10(3) = \(d), __exp10f(-2) = \(f)")
    exit(1)
}
