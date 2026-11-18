// Copyright (C) 2026 Lubos Dolezel
//
// The host EGL (libglvnd) loads every vendor driver it finds unless told
// otherwise, and each costs resident memory in every GUI process. Where the
// window system shows that a vendor cannot serve it, only the others should
// be loaded. Checked here: on an X server that offers neither the NVIDIA
// driver's GLX extension nor DRI3, NVIDIA's EGL vendor library is not loaded
// by starting the GUI; glvnd's override variable does not appear in the
// process or its children; and a GL window still renders and presents.
// Without a host to learn from the test only checks the last two.
// Host libraries cannot create files under the guest's /Users, so the host
// shader cache is pointed into the prefix: after compiling and drawing with a
// GLSL program the cache directory under the guest's Library/Caches must hold
// entries, and the variable that selects it must stay out of the guest's and
// child processes' environment. With MESA_SHADER_CACHE_DIR or
// MESA_SHADER_CACHE_DISABLE set by the user, or without a Mesa renderer, that
// part is skipped.
#import <AppKit/AppKit.h>
#import <CoreGraphics/CGSubWindow.h>
#import <OpenGL/CGLInternal.h>
#import <OpenGL/gl.h>
#include <dirent.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

@interface NSWindow (VendorFixture)
- (CGSubWindow *) _createSubWindowWithFrame:(CGRect)frame;
@end

static int host_maps_contain(const char *needle) {
    FILE *f = fopen("/Volumes/SystemRoot/proc/self/maps", "r");
    if (!f)
        return -1;
    char line[2048];
    int found = 0;
    while (fgets(line, sizeof(line), f))
        if (strstr(line, needle))
            found = 1;
    fclose(f);
    return found;
}

// 1 if the X server has neither NV-GLX nor DRI3, 0 if it has one, -1 if unknown.
static int x_server_is_software_only(void) {
    const char *backend = getenv("DARLING_APPKIT_BACKEND");
    if (!getenv("DISPLAY") || (backend && !strcmp(backend, "wayland")))
        return -1;
    void *xlib = dlopen("/usr/lib/native/libX11.dylib", RTLD_NOW | RTLD_LOCAL);
    if (!xlib)
        return -1;
    void *(*open_display)(const char *) = dlsym(xlib, "XOpenDisplay");
    int (*close_display)(void *) = dlsym(xlib, "XCloseDisplay");
    int (*query)(void *, const char *, int *, int *, int *) = dlsym(xlib, "XQueryExtension");
    if (!open_display || !close_display || !query)
        return -1;
    void *dpy = open_display(NULL);
    if (!dpy)
        return -1;
    int a, b, c;
    int result = !query(dpy, "NV-GLX", &a, &b, &c) && !query(dpy, "DRI3", &a, &b, &c);
    close_display(dpy);
    return result;
}

// Counts the entries below dir, recursively.
static int count_entries(const char *dir) {
    DIR *d = opendir(dir);
    if (!d)
        return -1;
    int count = 0;
    struct dirent *e;
    while ((e = readdir(d))) {
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
            continue;
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir, e->d_name);
        int sub = e->d_type == DT_DIR ? count_entries(path) : 0;
        count += 1 + (sub > 0 ? sub : 0);
    }
    closedir(d);
    return count;
}

// Compiles, links and draws with a small GLSL program so the driver has a
// shader to cache.
static void draw_with_program(void) {
    unsigned (*create_shader)(unsigned) = dlsym(RTLD_DEFAULT, "glCreateShader");
    void (*shader_source)(unsigned, int, const char **, const int *) = dlsym(RTLD_DEFAULT, "glShaderSource");
    void (*compile_shader)(unsigned) = dlsym(RTLD_DEFAULT, "glCompileShader");
    unsigned (*create_program)(void) = dlsym(RTLD_DEFAULT, "glCreateProgram");
    void (*attach_shader)(unsigned, unsigned) = dlsym(RTLD_DEFAULT, "glAttachShader");
    void (*link_program)(unsigned) = dlsym(RTLD_DEFAULT, "glLinkProgram");
    void (*use_program)(unsigned) = dlsym(RTLD_DEFAULT, "glUseProgram");
    CHECK(create_shader && shader_source && compile_shader && create_program && attach_shader && link_program && use_program);
    if (!(create_shader && shader_source && compile_shader && create_program && attach_shader && link_program && use_program))
        return;
    const char *vs_text = "#version 120\nvoid main() { gl_Position = ftransform(); }\n";
    const char *fs_text = "#version 120\nvoid main() { gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); }\n";
    unsigned vs = create_shader(0x8B31), fs = create_shader(0x8B30), program = create_program();
    shader_source(vs, 1, &vs_text, NULL);
    shader_source(fs, 1, &fs_text, NULL);
    compile_shader(vs);
    compile_shader(fs);
    attach_shader(program, vs);
    attach_shader(program, fs);
    link_program(program);
    use_program(program);
    glBegin(GL_TRIANGLES);
    glVertex2f(-1, -1); glVertex2f(3, -1); glVertex2f(-1, 3);
    glEnd();
    unsigned char pixel[4] = {0};
    glReadPixels(10, 10, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    CHECK(pixel[0] == 0 && pixel[1] == 255 && pixel[2] == 0);
    use_program(0);
}

@interface VendorFixture : NSObject
- (void) run:(NSTimer *)timer;
@end

@implementation VendorFixture
- (void) run:(NSTimer *)timer {
    int software_only = x_server_is_software_only();
    if (software_only == 1) {
        int nvidia = host_maps_contain("libEGL_nvidia");
        CHECK(nvidia == 0);
        printf("software-only X server: NVIDIA EGL vendor mapped=%d\n", nvidia);
    } else {
        printf("vendor selection not checked (software-only server: %d)\n", software_only);
    }
    CHECK(getenv("__EGL_VENDOR_LIBRARY_FILENAMES") == NULL);

    NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(30, 30, 160, 100)
        styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO];
    [window setReleasedWhenClosed:NO];
    [window makeKeyAndOrderFront:nil];
    CGSubWindow *child = [[window _createSubWindowWithFrame:CGRectMake(0, 0, 160, 100)] retain];
    CGLContextObj context = NULL;
    CGLWindowRef drawable = NULL;
    CHECK(child != nil);
    CHECK(CGLCreateContext(NULL, NULL, &context) == kCGLNoError && context);
    if (child && context) {
        drawable = CGLGetWindowForContext(context, [child nativeWindow]);
        CHECK(drawable && CGLContextMakeCurrentAndAttachToWindow(context, drawable) == kCGLNoError);
    }
    if (drawable) {
        glViewport(0, 0, 160, 100);
        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        unsigned char pixel[4] = {0};
        glReadPixels(10, 10, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        CHECK(pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 0);
        CHECK(glGetError() == GL_NO_ERROR);
        CHECK(CGLFlushDrawable(context) == kCGLNoError);
        CHECK(glGetError() == GL_NO_ERROR);
    }
    // A shell started from here must not see the override either.
    CHECK(system("test -z \"$__EGL_VENDOR_LIBRARY_FILENAMES\"") == 0);

    if (drawable) {
        BOOL user_chose = getenv("MESA_SHADER_CACHE_DIR") || getenv("MESA_SHADER_CACHE_DISABLE")
            || getenv("MESA_GLSL_CACHE_DIR") || getenv("MESA_GLSL_CACHE_DISABLE") || getenv("XDG_CACHE_HOME");
        char cache[1024];
        snprintf(cache, sizeof(cache), "%s/Library/Caches/Mesa", getenv("HOME") ?: "");
        draw_with_program();
        CHECK(glGetError() == GL_NO_ERROR);
        CGLFlushDrawable(context);
        if (!user_chose && getenv("DARLING_PREFIX") && software_only == 1) {
            // The driver writes entries on a worker thread.
            int entries = count_entries(cache);
            for (int i = 0; i < 50 && entries <= 0; i++) {
                usleep(100000);
                entries = count_entries(cache);
            }
            printf("shader cache %s: %d entries\n", cache, entries);
            CHECK(entries > 0);
        } else {
            printf("shader cache not checked\n");
        }
        // The cache location is for host libraries only.
        if (!user_chose) {
            CHECK(getenv("MESA_SHADER_CACHE_DIR") == NULL);
            CHECK(system("test -z \"$MESA_SHADER_CACHE_DIR\"") == 0);
        }
    }

    CGLSetCurrentContext(NULL);
    if (drawable) CGLDestroyWindow(drawable);
    if (context) CGLReleaseContext(context);
    [child release]; [window close]; [window release];
    printf("%s\n", g_failures ? "FAIL" : "PASS");
    fflush(stdout);
    exit(g_failures ? 1 : 0);
}
@end

int main(void) {
    NSAutoreleasePool *pool = [NSAutoreleasePool new];
    [NSApplication sharedApplication];
    [NSScreen screens];
    VendorFixture *driver = [VendorFixture new];
    [NSTimer scheduledTimerWithTimeInterval:0.2 target:driver selector:@selector(run:) userInfo:nil repeats:NO];
    [[NSRunLoop currentRunLoop] run];
    [pool drain];
    return 1;
}
