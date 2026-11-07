#!/usr/bin/env python3
# Host test: iokitd and OpenGL request X11 EGL platform explicitly via eglGetPlatformDisplay
# to avoid crashes in Wayland environments where EGL_PLATFORM=wayland is set.
# usage: python3 tests/iokitd-egl-platform.py [REPO_ROOT]

import os
import re
import subprocess
import sys
import tempfile

def main():
    root = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    
    iosurface_root_path = os.path.join(root, "src/external/iokitd/src/IOSurfaceRoot.mm")
    opengl_c_path = os.path.join(root, "src/frameworks/OpenGL/OpenGL.c")

    # 1. Source verification for iokitd (IOSurfaceRoot.mm)
    if not os.path.exists(iosurface_root_path):
        sys.exit(f"IOSurfaceRoot.mm not found at {iosurface_root_path}")
        
    with open(iosurface_root_path, "r", encoding="utf-8") as f:
        iokitd_content = f.read()

    if "<EGL/eglext.h>" not in iokitd_content:
        sys.exit("IOSurfaceRoot.mm must include <EGL/eglext.h>")
    if "eglGetPlatformDisplay(EGL_PLATFORM_X11_KHR, IODisplayConnectX11::getDisplay(), NULL)" not in iokitd_content:
        sys.exit("IOSurfaceRoot.mm must use eglGetPlatformDisplay with EGL_PLATFORM_X11_KHR")
    if re.search(r"eglGetDisplay\s*\(\s*IODisplayConnectX11::getDisplay\(\)\s*\)", iokitd_content):
        sys.exit("IOSurfaceRoot.mm must not use unadorned eglGetDisplay for IODisplayConnectX11")

    # 2. Source verification for OpenGL.framework (OpenGL.c)
    if not os.path.exists(opengl_c_path):
        sys.exit(f"OpenGL.c not found at {opengl_c_path}")
        
    with open(opengl_c_path, "r", encoding="utf-8") as f:
        opengl_content = f.read()

    if "EGL_PLATFORM_X11_KHR" not in opengl_content:
        sys.exit("OpenGL.c must define or include EGL_PLATFORM_X11_KHR")
    if not re.search(r"CGLError\s+CGLRegisterNativeDisplay\s*\([^)]*\)\s*\{[^}]*(?:eglGetPlatformDisplay\s*\(\s*|register_native_display\s*\(\s*\w+\s*,\s*)EGL_PLATFORM_X11_KHR", opengl_content):
        sys.exit("CGLRegisterNativeDisplay in OpenGL.c must use eglGetPlatformDisplay with EGL_PLATFORM_X11_KHR")
    if not re.search(r"struct\s+_CGLDisplay\s*\*\s*getCGLDisplay\s*\([^)]*\)\s*\{[^}]*eglGetPlatformDisplay\s*\(\s*platform", opengl_content):
        sys.exit("getCGLDisplay in OpenGL.c must use eglGetPlatformDisplay with the registered platform")

    # 3. Dynamic probe under EGL_PLATFORM=wayland
    with tempfile.TemporaryDirectory(prefix="iokitd-egl-") as tmpdir:
        probe_c = os.path.join(tmpdir, "probe.c")
        probe_bin = os.path.join(tmpdir, "probe")

        with open(probe_c, "w", encoding="utf-8") as f:
            f.write("""#include <stdio.h>
#include <stdlib.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <X11/Xlib.h>

int main(void) {
    Display *x11Display = XOpenDisplay(NULL);
    if (!x11Display) {
        puts("PASS: No X11 display available, test skipped safely");
        return 0;
    }

    EGLDisplay eglDisplay = eglGetPlatformDisplay(EGL_PLATFORM_X11_KHR, x11Display, NULL);
    if (eglDisplay == EGL_NO_DISPLAY) {
        fprintf(stderr, "FAIL: eglGetPlatformDisplay returned EGL_NO_DISPLAY\\n");
        XCloseDisplay(x11Display);
        return 1;
    }

    EGLint major = 0, minor = 0;
    EGLBoolean ok = eglInitialize(eglDisplay, &major, &minor);
    if (ok != EGL_TRUE) {
        fprintf(stderr, "FAIL: eglInitialize failed\\n");
        XCloseDisplay(x11Display);
        return 1;
    }
    if (major < 1) {
        fprintf(stderr, "FAIL: major EGL version < 1\\n");
        eglTerminate(eglDisplay);
        XCloseDisplay(x11Display);
        return 1;
    }

    eglTerminate(eglDisplay);
    XCloseDisplay(x11Display);

    printf("PASS: eglGetPlatformDisplay with EGL_PLATFORM_X11_KHR succeeded under EGL_PLATFORM=wayland (EGL %d.%d)\\n", major, minor);
    return 0;
}
""")

        cc = os.environ.get("CC", "gcc")
        compile_res = subprocess.run([cc, "-Wall", "-Wextra", probe_c, "-lEGL", "-lX11", "-o", probe_bin],
                                     capture_output=True, text=True)
        if compile_res.returncode != 0:
            sys.exit(f"compilation failed:\n{compile_res.stderr}")

        env = os.environ.copy()
        env["EGL_PLATFORM"] = "wayland"
        run_res = subprocess.run([probe_bin], env=env, capture_output=True, text=True)
        if run_res.returncode != 0:
            sys.exit(f"probe execution failed (code {run_res.returncode}):\n{run_res.stderr}\n{run_res.stdout}")

        print(run_res.stdout.strip())

if __name__ == "__main__":
    main()
