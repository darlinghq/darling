#!/usr/bin/env python3
# Host test: CGLDestroyWindow and CGLClearDrawable disassociate context egl_surface
# to prevent dangling EGLSurface dereferences when contexts are reused after detachment.
# usage: python3 tests/cgl-destroy-window-surface.py [REPO_ROOT]

import os
import re
import subprocess
import sys
import tempfile

def main():
    root = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    opengl_c_path = os.path.join(root, "src/frameworks/OpenGL/OpenGL.c")

    if not os.path.exists(opengl_c_path):
        sys.exit(f"OpenGL.c not found at {opengl_c_path}")

    with open(opengl_c_path, "r", encoding="utf-8") as f:
        src = f.read()

    # 1. Source checks
    if "g_activeContexts" not in src:
        sys.exit("FAIL: OpenGL.c must maintain an active context registry (g_activeContexts)")
    if "CGLClearDrawable" not in src:
        sys.exit("FAIL: OpenGL.c must implement CGLClearDrawable")
    if "STUB: CGLClearDrawable" in src:
        sys.exit("FAIL: CGLClearDrawable must not be an empty stub")

    # Verify that CGLDestroyWindow function body iterates active contexts
    cgl_destroy_split = src.split("void CGLDestroyWindow")
    if len(cgl_destroy_split) < 2:
        sys.exit("FAIL: CGLDestroyWindow definition not found in OpenGL.c")
    cgl_destroy_body = cgl_destroy_split[1].split("CGL_EXPORT")[0]
    if "g_activeContexts" not in cgl_destroy_body:
        sys.exit("FAIL: CGLDestroyWindow must scan active contexts to clear matching surfaces")

    # 2. Dynamic execution under Darling shell or natively if already inside Darling
    test_src = os.path.join(root, "tests/cgl-destroy-window-surface.m")
    if not os.path.exists(test_src):
        sys.exit(f"Test source not found at {test_src}")

    has_darling = subprocess.run(["darling", "shell", "true"], capture_output=True).returncode == 0
    guest_bin = "/tmp/cgl_destroy_window_test"
    src_path = f"/Volumes/SystemRoot{test_src}" if has_darling else test_src

    compile_cmd = [
        "clang",
        "-framework", "Foundation",
        "-framework", "AppKit",
        "-framework", "OpenGL",
        src_path,
        "-o", guest_bin
    ]
    if has_darling:
        compile_cmd = ["darling", "shell"] + compile_cmd

    try:
        res = subprocess.run(compile_cmd, capture_output=True, text=True)
        if res.returncode != 0:
            sys.exit(f"Compilation failed:\n{res.stderr}\n{res.stdout}")

        display = os.environ.get("DISPLAY", ":1")
        if has_darling:
            run_cmd = ["darling", "shell", "bash", "-c", f"DISPLAY={display} {guest_bin}"]
        else:
            run_cmd = ["bash", "-c", f"DISPLAY={display} {guest_bin}"]

        res = subprocess.run(run_cmd, capture_output=True, text=True)
        if res.returncode != 0:
            sys.exit(f"Execution failed (code {res.returncode}):\n{res.stderr}\n{res.stdout}")

        if "PASS:" not in res.stdout:
            sys.exit(f"Execution did not report PASS:\n{res.stdout}")

        print("PASS: CGLDestroyWindow and CGLClearDrawable disassociate context surface verified")
    finally:
        cleanup_cmd = ["darling", "shell", "rm", "-f", guest_bin] if has_darling else ["rm", "-f", guest_bin]
        subprocess.run(cleanup_cmd, capture_output=True)

if __name__ == "__main__":
    main()
