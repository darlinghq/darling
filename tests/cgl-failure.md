# CGL creation/flush failure regression

Build cgl-failure.m as an Objective-C executable against Foundation and OpenGL. Run without registering a native EGL display. Default invocation verifies that creation without a registered display either succeeds on the offscreen default display or fails with kCGLBadConnection and clears an output sentinel, and a NULL output address returns kCGLBadAddress. Run with argument `flush` to verify CGLFlushDrawable(NULL) returns kCGLBadContext. The old flush implementation crashes; disable core dumps for that baseline.

Validated against private and installed Darling frameworks on arm64. This fixture does not validate successful EGL presentation, allocation exhaustion or a live surface swap failure.
