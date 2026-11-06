# Host test: iokitd and OpenGL request X11 EGL platform explicitly via eglGetPlatformDisplay
# to avoid crashes in Wayland environments where EGL_PLATFORM=wayland is set.
# usage: ruby tests/iokitd-egl-platform.rb [REPO_ROOT]

require 'tmpdir'
require 'open3'

root = ARGV.fetch(0, File.expand_path('..', __dir__))

iosurface_root_path = File.join(root, 'src/external/iokitd/src/IOSurfaceRoot.mm')
abort "IOSurfaceRoot.mm not found at #{iosurface_root_path}" unless File.exist?(iosurface_root_path)

opengl_c_path = File.join(root, 'src/frameworks/OpenGL/OpenGL.c')
abort "OpenGL.c not found at #{opengl_c_path}" unless File.exist?(opengl_c_path)

iokitd_content = File.read(iosurface_root_path)
opengl_content = File.read(opengl_c_path)

# 1. Source verification for iokitd (IOSurfaceRoot.mm)
abort "IOSurfaceRoot.mm must include <EGL/eglext.h>" unless iokitd_content.include?('<EGL/eglext.h>')
abort "IOSurfaceRoot.mm must use eglGetPlatformDisplay with EGL_PLATFORM_X11_KHR" unless iokitd_content.include?('eglGetPlatformDisplay(EGL_PLATFORM_X11_KHR, IODisplayConnectX11::getDisplay(), NULL)')
abort "IOSurfaceRoot.mm must not use unadorned eglGetDisplay for IODisplayConnectX11" if iokitd_content =~ /eglGetDisplay\s*\(\s*IODisplayConnectX11::getDisplay\(\)\s*\)/

# 2. Source verification for OpenGL.framework (OpenGL.c)
abort "OpenGL.c must define or include EGL_PLATFORM_X11_KHR" unless opengl_content.include?('EGL_PLATFORM_X11_KHR')
abort "CGLRegisterNativeDisplay in OpenGL.c must use eglGetPlatformDisplay with EGL_PLATFORM_X11_KHR" unless opengl_content =~ /CGLError\s+CGLRegisterNativeDisplay\s*\([^)]*\)\s*\{[^}]*eglGetPlatformDisplay\s*\(\s*EGL_PLATFORM_X11_KHR/
abort "getCGLDisplay in OpenGL.c must use eglGetPlatformDisplay with the registered platform" unless opengl_content =~ /struct\s+_CGLDisplay\s*\*\s*getCGLDisplay\s*\([^)]*\)\s*\{[^}]*eglGetPlatformDisplay\s*\(\s*platform/

# 3. Dynamic probe under EGL_PLATFORM=wayland
Dir.mktmpdir('iokitd-egl-') do |tmpdir|
  probe_c = File.join(tmpdir, 'probe.c')
  probe_bin = File.join(tmpdir, 'probe')

  File.write(probe_c, <<~'C')
    #include <stdio.h>
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
            fprintf(stderr, "FAIL: eglGetPlatformDisplay returned EGL_NO_DISPLAY\n");
            XCloseDisplay(x11Display);
            return 1;
        }

        EGLint major = 0, minor = 0;
        EGLBoolean ok = eglInitialize(eglDisplay, &major, &minor);
        if (ok != EGL_TRUE) {
            fprintf(stderr, "FAIL: eglInitialize failed\n");
            XCloseDisplay(x11Display);
            return 1;
        }
        if (major < 1) {
            fprintf(stderr, "FAIL: major EGL version < 1\n");
            eglTerminate(eglDisplay);
            XCloseDisplay(x11Display);
            return 1;
        }

        eglTerminate(eglDisplay);
        XCloseDisplay(x11Display);

        printf("PASS: eglGetPlatformDisplay with EGL_PLATFORM_X11_KHR succeeded under EGL_PLATFORM=wayland (EGL %d.%d)\n", major, minor);
        return 0;
    }
  C

  cc = ENV.fetch('CC', 'gcc')
  cmd = [cc, '-Wall', '-Wextra', probe_c, '-lEGL', '-lX11', '-o', probe_bin]
  out, status = Open3.capture2e(*cmd)
  abort "compilation failed:\n#{out}" unless status.success?

  env = { 'EGL_PLATFORM' => 'wayland' }
  env['DISPLAY'] = ENV['DISPLAY'] if ENV['DISPLAY']
  p_out, p_status = Open3.capture2e(env, probe_bin)
  abort "probe execution failed (status #{p_status.exitstatus}):\n#{p_out}" unless p_status.success?

  puts p_out.strip
end
