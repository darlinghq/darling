# Explicit-platform CGL display registration

`CGLRegisterNativeDisplayForPlatform(nativeDisplay, platform)` is an additive
internal backend API. The native pointer must match the supplied EGL platform.
It selects an initialized window/OpenGL/RGBA8 configuration supporting interval
zero and only publishes the pair after validation. New contexts on this path
default to interval zero so hidden Wayland surfaces do not wait indefinitely for
compositor frame callbacks. Explicit caller interval requests are retained.
The original X11 registration entry and its interval-one default remain.

The companion Cocotron `AppKit/Wayland.backend/tests/egltest.m` provides live
plain-arm64 coverage with real Wayland EGL drawables, layers and NSOpenGLView.
Private 1x/2x tests include exact GL readback and compositor pixels, hidden swaps,
resize, crop, current-drawable replacement, and teardown. Final 2x deep run passes
64 compositor pixel assertions, exit0 and clean prefix shutdown. NULL registration
rejects early without disturbing an existing display; NULL drawable attachment
rejects without rebinding. These are not injected allocation/config failures.

CGL current-context TLS is published only after successful eglMakeCurrent and
remains coherent if the subsequent swap-interval request fails. Framework callers
must check attachment errors before issuing GL commands; the companion Cocotron
changes do so. The Wayland presentation contract is main-thread-only.

Source retains PR63's existing failure guards. Validation used private framework
copies only; no installed library was replaced. Default X11 and invalid-socket
fallback both pass RGB pixel checks and exit0 with clean shutdown. Full runtime evidence on the
validation host: darling-gui/privbuild/wayland/logs/codex-m4-egl-deep2-clear-20260916.
