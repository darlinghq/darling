# Wayland default framebuffer depth regression

Build `cgl-wayland-default-depth.m` as an Objective-C executable against AppKit, CoreGraphics and OpenGL using the configured Darling toolchain. Run it in a Wayland desktop with `DARLING_APPKIT_BACKEND=wayland`. It creates a small window through the existing native subwindow/CGL path and exits with its result. A valid EGL context/surface is required; unavailable rendering is a failure, not a skip.

The fixture paints its own near red quad, then an overlapping far blue quad. With depth testing enabled, red must remain visible. It checks the actual drawable has at least 24 depth bits, checks its own known pixel through glReadPixels, and requires no GL error. No guest application data, shaders or pixel buffers are inspected.

Before the change, the Asahi default framebuffer reports depth=0, the blue quad overwrites red, and the fixture exits 1. With the change it reports depth=24, red remains visible, and exits 0. Run in a disposable prefix; the test terminates its own app process after cleanup.

This validates the default Wayland drawable, not complete CGL pixel-format selection, buffer-size policies, other EGL platforms, or allocation exhaustion. The 24-bit depth buffer is a deliberate backend default; it is not a Cocoa guarantee for an attribute-free pixel format. Extra depth storage and requiring a compatible EGL config are the tradeoffs.
