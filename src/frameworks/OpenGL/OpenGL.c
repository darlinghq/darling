#include <stdlib.h>
#include <stdio.h>  // for fprintf(stderr, "unimplemented")
#include <string.h>
#include <stdint.h>

#include <pthread.h>
#include <stdatomic.h>
#include <dlfcn.h>
#include <limits.h>
#include <IOKit/IOKitLib.h>

#define GL_GLEXT_PROTOTYPES 1
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLRenderers.h>
#include <OpenGL/glext.h>
#include <OpenGL/CGLInternal.h>
#include <OpenGL/gliDispatch.h>
#include <stddef.h>
// The layout check below compares this file's context struct with the public one.
#define _CGLContextObject _CGLContextObjectPublic
#include <OpenGL/CGLContext.h>
#undef _CGLContextObject
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFRunLoop.h>
#include <CoreFoundation/CFNotificationCenter.h>
#include <sys/time.h>
#include <IOSurface/IOSurface.h>
#include <IOSurface/IOSurfacePriv.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <elfcalls.h>
#include <pthread.h>

// Try to get the right (generic) type definitions.
// In particular, we really want EGLNativeDisplayType to be void *,
// not int as it is if __APPLE__ is defined.
#undef APPLE
#undef __APPLE__

#define __unix__
#define EGL_NO_X11

#include <EGL/egl.h>
#include <EGL/eglext.h>

#ifndef EGL_PLATFORM_X11_KHR
#define EGL_PLATFORM_X11_KHR 0x31D5
#endif

#include "swap_pacing.h"
#include "gl_exports.h"
#include "drm_formats.h"

#ifndef EGL_PLATFORM_WAYLAND_KHR
#define EGL_PLATFORM_WAYLAND_KHR 0x31D8
#endif

#ifndef EGL_PLATFORM_SURFACELESS_MESA
#define EGL_PLATFORM_SURFACELESS_MESA 0x31DD
#endif

#ifndef EGL_CONTEXT_MAJOR_VERSION
#define EGL_CONTEXT_MAJOR_VERSION 0x3098
#endif

#ifndef EGL_CONTEXT_MINOR_VERSION
#define EGL_CONTEXT_MINOR_VERSION 0x30FB
#endif

#ifndef EGL_CONTEXT_OPENGL_PROFILE_MASK
#define EGL_CONTEXT_OPENGL_PROFILE_MASK 0x30FD
#endif

#ifndef EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT
#define EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT 0x00000001
#endif

#define APPLE
#define __APPLE__

// OpenGL.framework on macOS exports no EGL symbols. Linking the native EGL
// into this framework would put every egl* symbol into the global lookup
// scope of each process that uses OpenGL (dlsym(RTLD_DEFAULT), flat-namespace
// binding), where it shadows an EGL implementation that an application ships
// itself. Load the native EGL with RTLD_LOCAL instead and call it through
// this table.
#define CGL_EGL_FUNCTIONS(X) \
    X(eglBindAPI) \
    X(eglChooseConfig) \
    X(eglCreateContext) \
    X(eglCreatePbufferSurface) \
    X(eglCreateWindowSurface) \
    X(eglDestroyContext) \
    X(eglDestroySurface) \
    X(eglGetConfigAttrib) \
    X(eglGetDisplay) \
    X(eglGetPlatformDisplay) \
    X(eglGetProcAddress) \
    X(eglInitialize) \
    X(eglMakeCurrent) \
    X(eglQueryString) \
    X(eglSwapBuffers) \
    X(eglSwapInterval)

#define CGL_EGL_DECLARE(name) static __typeof__(name) *p_##name;
CGL_EGL_FUNCTIONS(CGL_EGL_DECLARE)
#undef CGL_EGL_DECLARE

static int native_egl_loaded;

// Host EGL vendor selection.
//
// The host's EGL is libglvnd's libEGL, which loads every vendor driver listed
// in its vendor directories the first time any EGL call needs them, so that it
// can answer client-extension queries and ask each vendor for a display. A
// vendor that will never be used still costs its libraries' memory in every
// process, up to a hundred megabytes or more for a software renderer or a
// proprietary driver. Where the window-system connection already shows that a
// vendor cannot serve it, glvnd is told, through its documented
// __EGL_VENDOR_LIBRARY_FILENAMES variable, to load only the others. When that
// cannot be established, or the user set glvnd's own variables, nothing is
// changed and all vendors load as before.
//
// The variable is set in the host C library's environment only (reached the
// way the IOSurface framework reaches host libc), for the duration of glvnd's
// vendor loading. The Darwin environment that child processes inherit is not
// touched.
#define CGL_GLVND_VENDOR_DIRS_COUNT 2
static const char *const glvnd_vendor_dirs[CGL_GLVND_VENDOR_DIRS_COUNT] = {
    "/etc/glvnd/egl_vendor.d",
    "/usr/share/glvnd/egl_vendor.d",
};
// The host file system is visible under this path in the prefix.
#define CGL_HOST_ROOT "/Volumes/SystemRoot"
#define CGL_VENDOR_MAX_FILES 32
#define CGL_VENDOR_NAME_MAX 256

struct vendor_file {
    char host_path[PATH_MAX];
    char name[CGL_VENDOR_NAME_MAX];
    int proprietary_gpu; // the library is NVIDIA's EGL vendor library
};

static int vendor_file_cmp(const void *a, const void *b) {
    return strcmp(((const struct vendor_file *) a)->name, ((const struct vendor_file *) b)->name);
}

// Reads a vendor JSON and tells whether it names NVIDIA's EGL vendor library.
// Only the "library_path" value is looked at.
static int vendor_json_is_nvidia(const char *guest_path) {
    FILE *f = fopen(guest_path, "r");
    if (!f)
        return -1;
    char buf[2048];
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    buf[n] = 0;
    const char *key = strstr(buf, "library_path");
    if (!key)
        return -1;
    const char *colon = strchr(key, ':');
    if (!colon)
        return -1;
    const char *quote = strchr(colon, '"');
    if (!quote)
        return -1;
    const char *end = strchr(quote + 1, '"');
    if (!end)
        return -1;
    size_t len = (size_t) (end - quote - 1);
    return len >= 13 && !strncmp(quote + 1, "libEGL_nvidia", 13);
}

// Lists the vendor files glvnd would read, in its order: sorted by name, a
// file in an earlier directory hiding a later one of the same name.
// Returns the count, or -1 if a file cannot be classified.
static int list_vendor_files(struct vendor_file *files) {
    int count = 0;
    for (int d = 0; d < CGL_GLVND_VENDOR_DIRS_COUNT; d++) {
        char dir[PATH_MAX];
        snprintf(dir, sizeof(dir), CGL_HOST_ROOT "%s", glvnd_vendor_dirs[d]);
        DIR *dp = opendir(dir);
        if (!dp)
            continue;
        struct dirent *ent;
        while ((ent = readdir(dp))) {
            size_t len = strlen(ent->d_name);
            if (len < 6 || strcmp(ent->d_name + len - 5, ".json") || len >= CGL_VENDOR_NAME_MAX)
                continue;
            int dup = 0;
            for (int i = 0; i < count; i++)
                dup |= !strcmp(files[i].name, ent->d_name);
            if (dup)
                continue;
            if (count == CGL_VENDOR_MAX_FILES) {
                closedir(dp);
                return -1;
            }
            struct vendor_file *vf = &files[count];
            char guest_path[PATH_MAX];
            snprintf(guest_path, sizeof(guest_path), "%s/%s", dir, ent->d_name);
            snprintf(vf->host_path, sizeof(vf->host_path), "%s/%s", glvnd_vendor_dirs[d], ent->d_name);
            snprintf(vf->name, sizeof(vf->name), "%s", ent->d_name);
            vf->proprietary_gpu = vendor_json_is_nvidia(guest_path);
            if (vf->proprietary_gpu < 0) {
                closedir(dp);
                return -1;
            }
            count++;
        }
        closedir(dp);
    }
    qsort(files, count, sizeof(files[0]), vendor_file_cmp);
    return count;
}

extern struct elf_calls *_elfcalls;

static int (*host_setenv)(const char *, const char *, int);
static int (*host_unsetenv)(const char *);
static int (*host_access)(const char *, int);

static int resolve_host_libc(void) {
    if (!_elfcalls)
        return 0;
    void *libc = _elfcalls->dlopen("libc.so.6");
    if (!libc)
        return 0;
    host_setenv = _elfcalls->dlsym(libc, "setenv");
    host_unsetenv = _elfcalls->dlsym(libc, "unsetenv");
    host_access = _elfcalls->dlsym(libc, "access");
    return host_setenv && host_unsetenv && host_access;
}

// Whether the NVIDIA proprietary driver has a device on the host (desktop
// driver or the Tegra one), as opposed to its EGL libraries merely being
// installed.
static int host_has_nvidia_device(void) {
    return host_access("/dev/nvidiactl", 0) == 0 || host_access("/dev/nvhost-ctrl", 0) == 0;
}

enum vendor_choice {
    vendors_all,
    vendors_without_nvidia,
    vendors_only_nvidia,
};

// X11: the NVIDIA EGL platform needs an X server driven by the NVIDIA driver
// (the NV-GLX extension) or one offering DRI3; Mesa's software rasterizer
// works without either. A server with neither can only be served by Mesa,
// and one driven by the NVIDIA driver can only be served by NVIDIA's EGL.
// DRI3 alone does not tell whose device it opens, so it decides nothing.
static enum vendor_choice x11_vendor_choice(void *x11_display) {
    void *xlib = dlopen("/usr/lib/native/libX11.dylib", RTLD_NOW | RTLD_LOCAL);
    if (!xlib)
        return vendors_all;
    int (*query_extension)(void *, const char *, int *, int *, int *) = dlsym(xlib, "XQueryExtension");
    if (!query_extension)
        return vendors_all;
    int major, event, error;
    if (query_extension(x11_display, "NV-GLX", &major, &event, &error))
        return vendors_only_nvidia;
    if (query_extension(x11_display, "DRI3", &major, &event, &error))
        return vendors_all;
    return vendors_without_nvidia;
}

// Wayland: NVIDIA's EGL platform shares buffers with the compositor through
// linux-dmabuf or wl_drm. A compositor offering neither, such as one that
// renders in software, can only be served by shared-memory software
// rendering. The interfaces are looked up on a short connection of their own
// (the wire protocol's registry and sync requests) rather than on the
// backend's, whose event queues are not ours to read. Returns 1 only if the
// whole registry was read and neither interface is among its globals.
static int wayland_offers_gpu_buffers(void) {
    const char *name = getenv("WAYLAND_DISPLAY");
    if (!name || !*name)
        return 1;
    // The compositor's socket is on the host file system, which the prefix
    // shows under CGL_HOST_ROOT.
    const char *directory = "";
    if (name[0] != '/') {
        directory = getenv("XDG_RUNTIME_DIR");
        if (!directory || !*directory)
            return 1;
    }
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    if (snprintf(addr.sun_path, sizeof(addr.sun_path), CGL_HOST_ROOT "%s%s%s", directory, *directory ? "/" : "", name)
        >= (int) sizeof(addr.sun_path))
        return 1;
    addr.sun_len = sizeof(addr);
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
        return 1;
    struct timeval timeout = { 2, 0 };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    int offers = 1;
    // wl_display.get_registry(registry = 2), wl_display.sync(callback = 3)
    uint32_t request[6] = { 1, (12u << 16) | 1, 2, 1, (12u << 16) | 0, 3 };
    if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) != 0
        || write(fd, request, sizeof(request)) != (ssize_t) sizeof(request)) {
        close(fd);
        return 1;
    }
    uint32_t buf[4096];
    size_t have = 0;
    int found = 0;
    for (;;) {
        ssize_t n = read(fd, (char *) buf + have, sizeof(buf) - have);
        if (n <= 0)
            break;
        have += n;
        size_t pos = 0;
        int finished = 0;
        while (have - pos >= 8) {
            uint32_t object = buf[pos / 4], size = buf[pos / 4 + 1] >> 16, opcode = buf[pos / 4 + 1] & 0xffff;
            if (size < 8 || size > sizeof(buf) || have - pos < size)
                break;
            const uint32_t *args = &buf[pos / 4 + 2];
            if (object == 2 && opcode == 0 && size >= 20) {
                // wl_registry.global: name, interface (length with NUL, padded), version
                uint32_t len = args[1];
                const char *iface = (const char *) &args[2];
                if (len > 0 && len <= size - 20 && iface[len - 1] == 0
                    && (!strcmp(iface, "zwp_linux_dmabuf_v1") || !strcmp(iface, "wl_drm")))
                    found = 1;
            } else if (object == 3 && opcode == 0) {
                offers = found;
                finished = 1;
                break;
            } else if (object == 1 && opcode == 0) {
                finished = 2; // a protocol error: unsure
                break;
            }
            pos += size;
        }
        if (finished)
            break;
        if (pos) {
            memmove(buf, (char *) buf + pos, have - pos);
            have -= pos;
        } else if (have == sizeof(buf)) {
            break;
        }
    }
    close(fd);
    return offers;
}

static enum vendor_choice choose_vendors(void *native_display, EGLenum platform) {
    if (!native_display)
        return vendors_all;
    enum vendor_choice choice = vendors_all;
    if (platform == EGL_PLATFORM_X11_KHR)
        choice = x11_vendor_choice(native_display);
    // Without an NVIDIA device its EGL cannot be used, whatever the server.
    if (platform == EGL_PLATFORM_WAYLAND_KHR && !wayland_offers_gpu_buffers())
        choice = vendors_without_nvidia;
    if (choice != vendors_without_nvidia && !host_has_nvidia_device()
        && (platform == EGL_PLATFORM_X11_KHR || platform == EGL_PLATFORM_WAYLAND_KHR))
        choice = vendors_without_nvidia;
    return choice;
}

// Sets the vendor list for glvnd, returning 1 if the variable was set and has
// to be removed again.
static int restrict_host_vendors(void *native_display, EGLenum platform) {
    if (getenv("__EGL_VENDOR_LIBRARY_FILENAMES") || getenv("__EGL_VENDOR_LIBRARY_DIRS"))
        return 0;
    if (!resolve_host_libc())
        return 0;
    enum vendor_choice choice = choose_vendors(native_display, platform);
    if (choice == vendors_all)
        return 0;
    struct vendor_file files[CGL_VENDOR_MAX_FILES];
    int count = list_vendor_files(files);
    if (count <= 0)
        return 0;
    char list[PATH_MAX * 4];
    size_t used = 0;
    int kept = 0, dropped = 0;
    for (int i = 0; i < count; i++) {
        if (files[i].proprietary_gpu != (choice == vendors_only_nvidia)) {
            dropped++;
            continue;
        }
        int n = snprintf(list + used, sizeof(list) - used, "%s%s", kept ? ":" : "", files[i].host_path);
        if (n < 0 || (size_t) n >= sizeof(list) - used)
            return 0;
        used += n;
        kept++;
    }
    if (!kept || !dropped)
        return 0;
    return host_setenv("__EGL_VENDOR_LIBRARY_FILENAMES", list, 1) == 0;
}

// Host libraries run with the guest's environment but see the real Linux file
// system, so the guest's HOME (/Users/<name>) does not exist for them and
// Mesa cannot create its shader cache under it. Unless the user chose a cache
// location or turned the cache off, point Mesa at the guest user's cache
// directory inside the prefix, as seen from the host. Mesa derives all its
// cache layouts (multi-file, single-file, database) from this directory. Only
// the host C library's environment is changed, never the guest's.
static void set_host_shader_cache_dir(void) {
    static const char *const user_choices[] = {
        "MESA_SHADER_CACHE_DIR", "MESA_SHADER_CACHE_DISABLE",
        "MESA_GLSL_CACHE_DIR", "MESA_GLSL_CACHE_DISABLE", "XDG_CACHE_HOME",
    };
    for (size_t i = 0; i < sizeof(user_choices) / sizeof(user_choices[0]); i++)
        if (getenv(user_choices[i]))
            return;
    const char *prefix = getenv("DARLING_PREFIX");
    const char *home = getenv("HOME");
    if (!prefix || prefix[0] != '/' || !home || home[0] != '/' || !resolve_host_libc())
        return;
    char dir[PATH_MAX];
    int n = snprintf(dir, sizeof(dir), "%s%s/Library/Caches/Mesa", prefix, home);
    if (n > 0 && (size_t) n < sizeof(dir))
        host_setenv("MESA_SHADER_CACHE_DIR", dir, 0);
}

static void load_native_egl(void *native_display, EGLenum platform) {
    set_host_shader_cache_dir();
    int restricted = restrict_host_vendors(native_display, platform);
    void *handle = dlopen("/usr/lib/native/libEGL.dylib", RTLD_NOW | RTLD_LOCAL);
    if (handle) {
#define CGL_EGL_RESOLVE(name) \
        if (!handle || !(p_##name = (__typeof__(p_##name)) dlsym(handle, #name))) { \
            if (handle) \
                dlclose(handle); \
            handle = NULL; \
        }
        CGL_EGL_FUNCTIONS(CGL_EGL_RESOLVE)
#undef CGL_EGL_RESOLVE
        // Client extensions are answered from the vendor list, so this makes
        // glvnd read the variable now, once, before it is removed again.
        if (handle && restricted)
            p_eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
        native_egl_loaded = handle != NULL;
    }
    if (restricted)
        host_unsetenv("__EGL_VENDOR_LIBRARY_FILENAMES");
}

// The first caller loads the host EGL for good. A registering window-system
// display passes its native display so the vendor can be chosen to match it;
// every other entry point passes none and gets every vendor, as an offscreen
// renderer may want any of them.
static int native_egl_load(void *native_display, EGLenum platform) {
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    static int attempted;
    pthread_mutex_lock(&lock);
    if (!attempted) {
        attempted = 1;
        load_native_egl(native_display, platform);
    }
    int loaded = native_egl_loaded;
    pthread_mutex_unlock(&lock);
    return loaded;
}

// Every entry point that can reach EGL before a context or a window exists
// checks this; contexts and windows only exist once EGL has been loaded.
static int native_egl_available(void) {
    return native_egl_load(NULL, 0);
}

static EGLDisplay display;

static EGLConfig config;
static int num_config;
// kCGLCPSwapInterval is documented to default to 0: CGLFlushDrawable does not
// wait for the vertical refresh unless the application asks for it.
static int default_swap_interval = 0;
// Set while the registered display is Wayland, where the application's swap
// interval is kept in software (swap_pacing.h) and EGL stays at interval 0.
static int default_paced = 0;
// The EGL platform of the registered window-system display. The CoreGraphics
// connection belongs to the same window-system backend, so its native display
// is of this platform too. Until a backend registers, assume X11, the
// platform of the X11 backend's connection.
static EGLenum registered_platform = EGL_PLATFORM_X11_KHR;
static pthread_mutex_t registration_lock = PTHREAD_MUTEX_INITIALIZER;

// Picks a config from those matching attrs. With prefer_unthrottled, one
// whose minimum swap interval is 0 is taken if there is any, so that a swap
// interval of 0 can be honoured; otherwise EGL's first choice is.
static EGLBoolean pick_config(EGLDisplay dpy, const EGLint *attrs, int prefer_unthrottled,
                              EGLConfig *out_config, int *out_count) {
    EGLint count = 0;
    if (!p_eglChooseConfig(dpy, attrs, NULL, 0, &count) || count <= 0)
        return EGL_FALSE;
    EGLConfig *configs = calloc((size_t)count, sizeof(*configs));
    if (!configs)
        return EGL_FALSE;
    EGLBoolean found = EGL_FALSE;
    if (p_eglChooseConfig(dpy, attrs, configs, count, &count) && count > 0) {
        found = EGL_TRUE;
        *out_config = configs[0];
        for (EGLint i = 0; prefer_unthrottled && i < count; i++) {
            EGLint minimum_interval;
            if (p_eglGetConfigAttrib(dpy, configs[i], EGL_MIN_SWAP_INTERVAL, &minimum_interval)
                && minimum_interval == 0) {
                *out_config = configs[i];
                break;
            }
        }
        if (out_count) *out_count = count;
    }
    free(configs);
    return found;
}

// Tries RGBA8 with 24-bit depth and 8-bit stencil, then 16-bit depth, then
// any RGB config with the given surface type.
static EGLBoolean choose_config(EGLDisplay dpy, EGLint surface_type, int prefer_unthrottled,
                                EGLConfig *out_config, int *out_count) {
    EGLint const attrs_24[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_SURFACE_TYPE, surface_type,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };
    EGLint const attrs_16[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_SURFACE_TYPE, surface_type,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };
    EGLint const attrs_fallback[] = {
        EGL_RED_SIZE, 1,
        EGL_GREEN_SIZE, 1,
        EGL_BLUE_SIZE, 1,
        EGL_SURFACE_TYPE, surface_type,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };
    const EGLint *tiers[] = { attrs_24, attrs_16, attrs_fallback };
    for (size_t i = 0; i < sizeof(tiers) / sizeof(tiers[0]); i++) {
        if (pick_config(dpy, tiers[i], prefer_unthrottled, out_config, out_count))
            return EGL_TRUE;
    }
    return EGL_FALSE;
}

// The config of a registered window-system display, the same for every
// platform. Contexts on it may draw into windows and into the pbuffer used
// before a drawable is attached; platforms without pbuffer configs (Wayland
// on some drivers) get a window-only config.
static EGLBoolean choose_registered_config(EGLDisplay dpy, EGLConfig *out_config, int *out_count) {
    return choose_config(dpy, EGL_WINDOW_BIT | EGL_PBUFFER_BIT, 1, out_config, out_count)
        || choose_config(dpy, EGL_WINDOW_BIT, 1, out_config, out_count);
}

// A process that creates a context without registering a window-system
// display, such as an offscreen renderer, gets EGL's default display, whose
// platform EGL picks, or a surfaceless display if that cannot be opened.
// Its contexts render into pbuffers or framebuffer objects.
static EGLDisplay default_display = EGL_NO_DISPLAY;
static EGLConfig default_config;

static void open_default_display(void) {
    EGLDisplay candidates[] = {
        p_eglGetDisplay(EGL_DEFAULT_DISPLAY),
        p_eglGetPlatformDisplay(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, NULL),
    };
    for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++) {
        EGLConfig candidate_config;
        if (candidates[i] == EGL_NO_DISPLAY || !p_eglInitialize(candidates[i], NULL, NULL))
            continue;
        if (!choose_config(candidates[i], EGL_PBUFFER_BIT, 0, &candidate_config, NULL))
            continue;
        default_display = candidates[i];
        default_config = candidate_config;
        return;
    }
}

static EGLDisplay get_default_display(EGLConfig *out_config) {
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    pthread_once(&once, open_default_display);
    *out_config = default_config;
    return default_display;
}

struct _CGLDisplay
{
    EGLDisplay display;
    EGLConfig config;
    int num_config;
};

static CFMutableDictionaryRef g_displays;
static pthread_mutex_t g_displaysMutex = PTHREAD_MUTEX_INITIALIZER;

// Where a context's EGL binding is. EGL binds a context to at most one
// thread at a time and refuses a second thread (EGL_BAD_ACCESS) until the
// first one releases it.
enum cgl_binding {
    CGL_BINDING_FREE,   // bound to no thread
    CGL_BINDING_BOUND,  // bound to binding_thread
    CGL_BINDING_PARKED  // released by binding_thread, which still has it as its current context
};

static pthread_mutex_t g_bindingMutex = PTHREAD_MUTEX_INITIALIZER;
// Signalled (under g_bindingMutex) when a context's binding is released.
static pthread_cond_t g_bindingCond = PTHREAD_COND_INITIALIZER;

struct cgl_profile_strings;

struct _CGLContextObject {
    // The first four members are the public layout (see CGLContext.h): code
    // built with CGLMacro.h reads rend and calls through disp.
    GLIContext rend;
    GLIFunctionDispatch disp;
    CGLPrivateObj priv;
    void *stak;
    GLuint retain_count;
    pthread_mutex_t lock;
    EGLDisplay egl_display;
    EGLConfig egl_config;
    EGLContext egl_context;
    _Atomic EGLSurface egl_surface;
    EGLSurface fallback_pbuffer;
    // The display-covering window CGLSetFullScreen created and the surface on
    // it (fullscreen_window is 0 when there is none); the context owns both.
    // Guarded by lock.
    CGSConnectionID fullscreen_connection;
    CGSWindowID fullscreen_window;
    EGLSurface fullscreen_surface;
    // EGL has no function for getting the current swap interval,
    // so we need to save the last set value. The default is 0.
    // This is the application's value; a paced context never gives it to EGL.
    int swap_interval;
    int paced;
    struct cgl_swap_pacer pacer;
    CGLPixelFormatObj pixel_format;
    // The OpenGL profile requested (kCGLOGLPVersion_*); 0 is a legacy context.
    GLint profile;
    // What glGetString and friends report for the profile (see the profile
    // strings below), made on first use; guarded by profile_lock, not by
    // lock: a thread that waits for the main thread to give the context up
    // holds lock, and the main thread's glGetString must not wait for it.
    pthread_mutex_t profile_lock;
    struct cgl_profile_strings *profile_strings;
    // CGLEnable state, one bit per enable_bit() index. Recorded only: none
    // of the options changes how this implementation renders.
    unsigned enables;
    // Whether the display imports DMA-BUFs (enum dmabuf_import), found out
    // on first use.
    _Atomic int dmabuf_import;
    // How GPU copies between textures are made (enum gpu_copy), found out on
    // first use, and the framebuffers a blit copy uses.
    int gpu_copy;
    GLuint copy_framebuffers[2];
    // Contexts that share object state (textures among them) have the same
    // share group number.
    uint64_t share_group;
    // GL_MAX_COLOR_ATTACHMENTS, found out on first use (0 until then).
    GLint max_color_attachments;
    struct _CGLContextObject *prev_active;
    struct _CGLContextObject *next_active;
    // Which thread the EGL context is bound to (see the thread hand-off
    // notes above setCurrentContext). Guarded by g_bindingMutex.
    enum cgl_binding binding;
    pthread_t binding_thread;
    int binding_thread_is_main;
    // Set once another thread has been refused the context because it was
    // bound elsewhere.
    int multi_thread;
    // A request to release the binding is queued on the main thread.
    int release_pending;
    // The main thread waits for a thread other than the main thread to
    // release the binding (at its next CGLLockContext or CGLUnlockContext).
    int release_wanted;
    // Counts how often the binding was given up (parked or freed), so that a
    // thread that waited for one can tell it happened even if the binding was
    // taken again before the thread woke up. Guarded by g_bindingMutex.
    unsigned release_count;
    // Nesting depth of CGLLockContext and the thread holding it; only that
    // thread writes them, under the context's lock.
    unsigned lock_depth;
    pthread_t lock_owner;
};

_Static_assert(offsetof(struct _CGLContextObject, rend) == offsetof(struct _CGLContextObjectPublic, rend) &&
               offsetof(struct _CGLContextObject, disp) == offsetof(struct _CGLContextObjectPublic, disp) &&
               offsetof(struct _CGLContextObject, priv) == offsetof(struct _CGLContextObjectPublic, priv) &&
               offsetof(struct _CGLContextObject, stak) == offsetof(struct _CGLContextObjectPublic, stak),
               "the context starts with the public layout");

static struct _CGLContextObject *g_activeContexts = NULL;
static pthread_mutex_t g_activeContextsMutex = PTHREAD_MUTEX_INITIALIZER;

struct _CGLPixelFormatObj {
    GLuint retain_count;
    CGLPixelFormatAttribute *attributes;
    EGLDisplay egl_display;
    EGLConfig egl_config;
    int swap_interval;
    int paced;
};

static inline int attribute_has_argument(CGLPixelFormatAttribute attr) {
    switch (attr) {
    case kCGLPFAAuxBuffers:
    case kCGLPFAColorSize:
    case kCGLPFAAlphaSize:
    case kCGLPFADepthSize:
    case kCGLPFAStencilSize:
    case kCGLPFAAccumSize:
    case kCGLPFARendererID:
    case kCGLPFADisplayMask:
    case kCGLPFAOpenGLProfile:
    case kCGLPFASampleBuffers:
    case kCGLPFASamples:
        return 1;
    default:
        return 0;
   }
}

static void writeBackSurface(IOSurfaceRef ioSurface);
static void finishSurface(IOSurfaceRef ioSurface);

__attribute__((constructor))
static void _CGLInitialize(void)
{
    g_displays = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);

    // A binding retains its surface, so a surface is never freed while
    // this framework has state for it and no forget callback is needed.
    // Rendering into textures is reported to IOSurface as it is noted (see
    // "Writing GL rendering back into surfaces").
    static const _IOSurfaceGPUCallbacks callbacks = {
        .version = _IOSURFACE_GPU_CALLBACKS_VERSION_REPORTS_WRITES,
        .writeback = writeBackSurface,
        .forget = NULL,
        .finish = finishSurface,
    };
    _IOSurfaceSetGPUCallbacks(&callbacks);
}

static int attributes_count(const CGLPixelFormatAttribute *attrs) {
    int result;
    for (result = 0; attrs[result] != 0; result++) {
        if (attribute_has_argument(attrs[result])) {
            result++;
        }
    }
    return result;
}

// Opens and checks the native display on the given platform, then makes it
// the registered display. A display that fails any step is not published,
// so a rejected registration leaves a working one in place. Such a display
// is not terminated either: EGL may have returned a handle that a live
// context still uses.
static CGLError register_native_display(void *native_display, EGLenum platform) {
    if (!native_display || !native_egl_load(native_display, platform))
        return kCGLBadConnection;
    EGLDisplay candidate = p_eglGetPlatformDisplay(platform, native_display, NULL);
    if (candidate == EGL_NO_DISPLAY || !p_eglInitialize(candidate, NULL, NULL))
        return kCGLBadConnection;
    EGLConfig candidate_config;
    int count = 0;
    if (!choose_registered_config(candidate, &candidate_config, &count))
        return kCGLBadPixelFormat;
    if (!p_eglBindAPI(EGL_OPENGL_API))
        return kCGLBadState;

    pthread_mutex_lock(&registration_lock);
    display = candidate;
    config = candidate_config;
    num_config = count;
    registered_platform = platform;
    default_swap_interval = 0;
    // On Wayland the swap interval is kept in software: layer animations may
    // keep drawing while their parent is hidden, and waiting for a frame
    // callback on an unmapped surface can block forever.
    default_paced = platform == EGL_PLATFORM_WAYLAND_KHR;
    pthread_mutex_unlock(&registration_lock);
    return kCGLNoError;
}

CGLError CGLRegisterNativeDisplay(void *native_display) {
    return register_native_display(native_display, EGL_PLATFORM_X11_KHR);
}

// For backends whose native display is not an X11 Display, such as Wayland.
CGLError CGLRegisterNativeDisplayForPlatform(void *native_display, unsigned int platform) {
    return register_native_display(native_display, (EGLenum) platform);
}

static struct _CGLDisplay* getCGLDisplay(CGSConnectionID cid)
{
    struct _CGLDisplay* rv;

    if (!native_egl_available())
        return NULL;

    pthread_mutex_lock(&g_displaysMutex);
    rv = (struct _CGLDisplay*) CFDictionaryGetValue(g_displays, (const void*)(unsigned long) cid);
    pthread_mutex_unlock(&g_displaysMutex);

    if (!rv)
    {
        void *native_display = _CGSNativeDisplay(cid);
        if (!native_display)
            return NULL;

        pthread_mutex_lock(&registration_lock);
        EGLenum platform = registered_platform;
        pthread_mutex_unlock(&registration_lock);

        EGLDisplay disp = p_eglGetPlatformDisplay(platform, native_display, NULL);
        if (disp == EGL_NO_DISPLAY)
            return NULL;

        struct _CGLDisplay* new_disp = (struct _CGLDisplay*) malloc(sizeof(*new_disp));
        if (!new_disp)
            return NULL;

        new_disp->display = disp;
        if (!p_eglInitialize(new_disp->display, NULL, NULL))
        {
            free(new_disp);
            return NULL;
        }
        if (!choose_registered_config(new_disp->display, &new_disp->config, &new_disp->num_config))
        {
            // Not terminated: EGL returns the same handle for the same native
            // display, so it may be the registered display that live contexts use.
            free(new_disp);
            return NULL;
        }

        pthread_mutex_lock(&g_displaysMutex);
        rv = (struct _CGLDisplay*) CFDictionaryGetValue(g_displays, (const void*)(unsigned long) cid);
        if (rv)
        {
            pthread_mutex_unlock(&g_displaysMutex);
            free(new_disp);
            return rv;
        }
        CFDictionaryAddValue(g_displays, (const void*)(unsigned long) cid, new_disp);
        pthread_mutex_unlock(&g_displaysMutex);
        rv = new_disp;
    }

    return rv;
}

CGLError CGLSetSurface(CGLContextObj gl, CGSConnectionID cid, CGSWindowID wid, CGSSurfaceID sid)
{
    if (!gl)
        return kCGLBadContext;
    struct _CGLDisplay* disp = getCGLDisplay(cid);
    if (!disp)
        return kCGLBadConnection;

    EGLNativeWindowType window;
    if (sid)
        window = (EGLNativeWindowType) _CGSNativeWindowForSurfaceID(cid, wid, sid);
    else
        window = (EGLNativeWindowType) _CGSNativeWindowForID(cid, wid);

    if (!window)
        return kCGLBadWindow;

    EGLSurface surf = p_eglCreateWindowSurface(gl->egl_display, gl->egl_config, window, NULL);
    atomic_store_explicit(&gl->egl_surface, surf, memory_order_release);
    if (surf == EGL_NO_SURFACE)
        return kCGLBadState;
    return kCGLNoError;
}

// CGLWindowRef is opaque: retain the display that owns its EGL surface.
struct _CGLWindow {
    EGLDisplay display;
    EGLConfig config;
    EGLSurface surface;
    // The window-system handle the surface was made for, and the list of live
    // windows (guarded by g_windowsMutex) that a swap uses to find it.
    void *native_window;
    struct _CGLWindow *next;
};

static pthread_mutex_t g_windowsMutex = PTHREAD_MUTEX_INITIALIZER;
static struct _CGLWindow *g_windows;

// After a swap on a window surface, tells whoever presents that native
// window (its window-system backend, which lives above this framework) that a
// new frame is ready. The notification is posted on the swapping thread to the
// local notification center, with the native window handle as its object.
static void postWindowDidSwap(EGLSurface surface) {
    void *native = NULL;
    pthread_mutex_lock(&g_windowsMutex);
    for (struct _CGLWindow *window = g_windows; window != NULL; window = window->next) {
        if (window->surface == surface) {
            native = window->native_window;
            break;
        }
    }
    pthread_mutex_unlock(&g_windowsMutex);
    if (native != NULL)
        CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(),
            kCGLWindowDidSwapNotification, native, NULL, true);
}

static CGLWindowRef createWindow(EGLDisplay owner, EGLConfig owner_config, void *native_window) {
    if (!native_window || owner == EGL_NO_DISPLAY)
        return NULL;
    struct _CGLWindow *window = malloc(sizeof(*window));
    if (!window)
        return NULL;
    window->display = owner;
    window->config = owner_config;
    window->surface = p_eglCreateWindowSurface(owner, owner_config,
                                             (EGLNativeWindowType) native_window, NULL);
    if (window->surface == EGL_NO_SURFACE) {
        free(window);
        return NULL;
    }
    window->native_window = native_window;
    pthread_mutex_lock(&g_windowsMutex);
    window->next = g_windows;
    g_windows = window;
    pthread_mutex_unlock(&g_windowsMutex);
    return window;
}

CGLWindowRef CGLGetWindow(void *native_window) {
    pthread_mutex_lock(&registration_lock);
    EGLDisplay owner = display;
    EGLConfig owner_config = config;
    pthread_mutex_unlock(&registration_lock);
    return createWindow(owner, owner_config, native_window);
}

CGLWindowRef CGLGetWindowForContext(CGLContextObj context, void *native_window) {
    if (!context)
        return NULL;
    return createWindow(context->egl_display, context->egl_config, native_window);
}

CGLWindowRef CGLGetWindowWithAlpha(void *native_window, unsigned long visual_id) {
    static EGLint const alpha_attributes[] = {
        EGL_RED_SIZE, 1,
        EGL_GREEN_SIZE, 1,
        EGL_BLUE_SIZE, 1,
        EGL_ALPHA_SIZE, 1,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };

    pthread_mutex_lock(&registration_lock);
    EGLDisplay owner = display;
    pthread_mutex_unlock(&registration_lock);

    EGLint count = 0;
    if (!native_window || owner == EGL_NO_DISPLAY ||
        !p_eglChooseConfig(owner, alpha_attributes, NULL, 0, &count) || count <= 0)
        return NULL;
    EGLConfig *configs = calloc((size_t)count, sizeof(*configs));
    if (!configs)
        return NULL;

    CGLWindowRef window = NULL;
    if (p_eglChooseConfig(owner, alpha_attributes, configs, count, &count)) {
        for (EGLint i = 0; i < count && !window; i++) {
            EGLint visual = 0;
            if (visual_id != 0 &&
                (!p_eglGetConfigAttrib(owner, configs[i], EGL_NATIVE_VISUAL_ID, &visual) ||
                 (unsigned long) visual != visual_id))
                continue;
            window = createWindow(owner, configs[i], native_window);
        }
    }
    free(configs);
    return window;
}

static int threadHoldsBinding(CGLContextObj context);

CGL_EXPORT void CGLDestroyWindow(CGLWindowRef ref) {
    struct _CGLWindow *window = ref;
    if (!window)
        return;

    CGLContextObj current = CGLGetCurrentContext();
    int current_cleared = 0;

    if (window->surface != EGL_NO_SURFACE) {
        pthread_mutex_lock(&g_activeContextsMutex);
        for (struct _CGLContextObject *ctx = g_activeContexts; ctx != NULL; ctx = ctx->next_active) {
            EGLSurface target = window->surface;
            if (atomic_compare_exchange_strong_explicit(&ctx->egl_surface, &target, (EGLSurface)EGL_NO_SURFACE,
                                                        memory_order_acq_rel, memory_order_acquire)) {
                if (ctx == current) {
                    current_cleared = 1;
                }
            }
        }
        pthread_mutex_unlock(&g_activeContextsMutex);
    }

    // Disassociate the destroyed window surface from the current EGL thread,
    // if this thread holds the context (a context that is current here but not
    // held is bound without the surface the next time, and binding it here
    // would bypass the record of which thread holds it).
    if (current_cleared && current && current->egl_display != EGL_NO_DISPLAY) {
        pthread_mutex_lock(&g_bindingMutex);
        int held = threadHoldsBinding(current);
        pthread_mutex_unlock(&g_bindingMutex);
        if (held)
            p_eglMakeCurrent(current->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, current->egl_context);
    }

    pthread_mutex_lock(&g_windowsMutex);
    for (struct _CGLWindow **link = &g_windows; *link != NULL; link = &(*link)->next) {
        if (*link == window) {
            *link = window->next;
            break;
        }
    }
    pthread_mutex_unlock(&g_windowsMutex);
    p_eglDestroySurface(window->display, window->surface);
    free(window);
}

CGL_EXPORT CGLError CGLContextMakeCurrentAndAttachToWindow(CGLContextObj context, CGLWindowRef window) {
    if (!context)
        return kCGLBadContext;
    if (!window)
        return kCGLBadDrawable;
    struct _CGLWindow *drawable = window;
    if (drawable->display != context->egl_display)
        return kCGLBadMatch;
    EGLSurface previous_surface = atomic_load_explicit(&context->egl_surface, memory_order_acquire);
    EGLSurface old_fallback = context->fallback_pbuffer;
    atomic_store_explicit(&context->egl_surface, drawable->surface, memory_order_release);
    CGLError error = CGLSetCurrentContext(context);
    if (error != kCGLNoError) {
        atomic_store_explicit(&context->egl_surface, previous_surface, memory_order_release);
        return error;
    }
    if (old_fallback != EGL_NO_SURFACE) {
        p_eglDestroySurface(context->egl_display, old_fallback);
        context->fallback_pbuffer = EGL_NO_SURFACE;
    }
    return kCGLNoError;
}

static pthread_key_t current_context_key;

static void make_key() {
    pthread_key_create(&current_context_key, NULL);
}

// What EGL gets: a paced context must never wait for a frame callback.
static int egl_swap_interval(CGLContextObj context) {
    return context->paced ? 0 : context->swap_interval;
}

static pthread_key_t get_current_context_key() {
    static pthread_once_t key_once = PTHREAD_ONCE_INIT;
    pthread_once(&key_once, make_key);
    return current_context_key;
}

CGLContextObj CGLGetCurrentContext(void) {
    pthread_key_t key = get_current_context_key();
    return pthread_getspecific(key);
}

// The EGL part of making context current on this thread.
static CGLError bindCurrentContext(CGLContextObj context) {
    // A context exists only if EGL was loaded, and without EGL nothing can be current.
    if (!native_egl_available())
        return context ? kCGLBadContext : kCGLNoError;
    if (!p_eglBindAPI(EGL_OPENGL_API))
        return kCGLBadState;
    if (context != NULL) {
        EGLSurface surface = atomic_load_explicit(&context->egl_surface, memory_order_acquire);
        if (surface == EGL_NO_SURFACE) {
            if (!p_eglMakeCurrent(context->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, context->egl_context)) {
                const EGLint pbufferAttribs[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
                EGLSurface pbuf = p_eglCreatePbufferSurface(context->egl_display, context->egl_config, pbufferAttribs);
                if (pbuf != EGL_NO_SURFACE && p_eglMakeCurrent(context->egl_display, pbuf, pbuf, context->egl_context)) {
                    context->fallback_pbuffer = pbuf;
                    atomic_store_explicit(&context->egl_surface, pbuf, memory_order_release);
                    surface = pbuf;
                } else {
                    if (pbuf != EGL_NO_SURFACE)
                        p_eglDestroySurface(context->egl_display, pbuf);
                    EGLint pbufConfAttrs[] = { EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RED_SIZE, 1, EGL_NONE };
                    EGLConfig pbufConfig = NULL;
                    EGLint numPbuf = 0;
                    if (p_eglChooseConfig(context->egl_display, pbufConfAttrs, &pbufConfig, 1, &numPbuf) && numPbuf > 0) {
                        pbuf = p_eglCreatePbufferSurface(context->egl_display, pbufConfig, pbufferAttribs);
                        if (pbuf != EGL_NO_SURFACE && p_eglMakeCurrent(context->egl_display, pbuf, pbuf, context->egl_context)) {
                            context->fallback_pbuffer = pbuf;
                            atomic_store_explicit(&context->egl_surface, pbuf, memory_order_release);
                            surface = pbuf;
                        } else {
                            if (pbuf != EGL_NO_SURFACE)
                                p_eglDestroySurface(context->egl_display, pbuf);
                            return kCGLBadContext;
                        }
                    } else {
                        return kCGLBadContext;
                    }
                }
            }
        } else {
            if (!p_eglMakeCurrent(context->egl_display, surface, surface, context->egl_context)) {
                return kCGLBadContext;
            }
        }
        pthread_setspecific(get_current_context_key(), context);
        if (surface != EGL_NO_SURFACE) {
            p_eglSwapInterval(context->egl_display, egl_swap_interval(context));
        }
    } else {
        CGLContextObj current = CGLGetCurrentContext();
        if (!current)
            return kCGLNoError;
        if (!p_eglMakeCurrent(current->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
            return kCGLBadContext;
        pthread_setspecific(get_current_context_key(), NULL);
    }
    return kCGLNoError;
}

// Thread hand-off. On macOS an application may make one context current on
// several threads: typically the main thread makes it current at set-up and
// a render thread (a display link callback, an NSThread) later makes it
// current there and draws, serialised with CGLLockContext, while the main
// thread goes back to its run loop and never touches GL again. EGL cannot
// move a bound context to another thread, so this framework moves it.
//
// When a thread is refused a context that the main thread has bound, the
// context is marked shared and the main thread is asked, through its run
// loop (CoreFoundation, which the macOS OpenGL framework links as well), to
// release the EGL binding when it next runs. The main thread keeps the
// context as its current context ("parked"): a later CGLSetCurrentContext,
// or CGLLockContext of the current context, binds it again if no other
// thread holds it. The refused thread waits a bounded time for the
// release, once per request, so that it cannot deadlock against a main
// thread that is itself waiting for it; if the main thread does not respond
// it is refused as before. A thread other than the main thread that binds a
// context is not asked: its run loop, if it has one, is not known.
// A context that no other thread asked for is never released this way.
//
// Waiting for the request has a hole: a render thread that holds an
// application lock while it asks for the context deadlocks with a main thread
// that blocks on that lock instead of returning to its run loop (the render
// thread then gives up after the wait below). On macOS the second thread's
// CGLSetCurrentContext succeeds at once whatever the main thread is doing,
// because the context stays current on the main thread as well (observed on
// macOS 26.7.3 with a two-thread probe). To come as close as EGL allows, once
// a context is known to be used by more than one thread the main thread
// releases its binding every time it returns to its run loop (before it
// waits, and when a nested run loop exits), so that the other thread finds
// the context free. The request above remains as the fallback for a main
// thread that has not been back in its run loop yet. The main thread still
// has the context as its current context and binds it again at its next
// CGLSetCurrentContext or CGLLockContext of it. Limitation: raw GL calls on
// the main thread after it was parked, with neither of those calls in
// between, find no EGL context (macOS allows them); EGL gives no cheaper way
// to have the binding follow the thread.
//
// The other direction: the render thread then keeps the binding between
// frames, so the main thread's next CGLSetCurrentContext (or the rebind in
// CGLLockContext) is refused. The main thread then asks the holding thread
// (only a thread other than the main thread can be asked this way) to give
// the binding up where it next enters or leaves CGLLockContext, which is
// between its uses of the context, and waits for that, while still answering
// requests of its own by the same rule. A bind that loses against another
// thread's bind of a shared context is repeated a few times. Raw GL calls
// of the thread that gave the binding up, before its next lock or
// CGLSetCurrentContext, find no EGL context.

// How long a refused thread waits for the main thread to answer. The main
// thread may be in the middle of a long run of GL calls (set-up that compiles
// shaders can take seconds on a software driver) before it is back in its run
// loop; a refused thread draws with no context, which crashes the host GL
// driver, so it is better to wait. Only the first refusal of a request waits.
#define HANDOFF_WAIT_SECONDS 10

static int threadHoldsBinding(CGLContextObj context) {
    return context->binding == CGL_BINDING_BOUND && pthread_equal(context->binding_thread, pthread_self());
}

// Gives the EGL binding up on the calling (main) thread if another thread
// asked for the context. The context stays this thread's current context
// ("parked").
static void releaseRequestedBinding(CGLContextObj context) {
    pthread_mutex_lock(&g_bindingMutex);
    // Not while this thread is inside CGLLockContext (a nested lock, or a
    // nested run loop): it is drawing with the context. The request stays
    // pending, and the thread that made it keeps waiting, until
    // CGLUnlockContext of the outermost lock answers it.
    int in_lock = context->lock_depth > 0 && pthread_equal(context->lock_owner, pthread_self());
    if (in_lock && threadHoldsBinding(context)) {
        pthread_mutex_unlock(&g_bindingMutex);
        return;
    }
    context->release_pending = 0;
    if (threadHoldsBinding(context) &&
        p_eglMakeCurrent(context->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
        context->binding = CGL_BINDING_PARKED;
        context->release_count++;
        if (gl_tls_bound == context)
            gl_tls_bound = NULL;
    }
    pthread_cond_broadcast(&g_bindingCond);
    pthread_mutex_unlock(&g_bindingMutex);
}

// Parks the calling (main) thread's binding if other threads use the context
// or asked for it.
static void parkMultiThreadBinding(CGLContextObj context) {
    pthread_mutex_lock(&g_bindingMutex);
    int park = (context->multi_thread || context->release_pending) && threadHoldsBinding(context);
    pthread_mutex_unlock(&g_bindingMutex);
    if (park)
        releaseRequestedBinding(context);
}

// Run loop observer (on the main run loop): parks the main thread's binding
// of every context that other threads use. Runs on the main thread only.
static void parkMultiThreadBindings(void) {
    pthread_mutex_lock(&g_activeContextsMutex);
    for (struct _CGLContextObject *ctx = g_activeContexts; ctx != NULL; ctx = ctx->next_active) {
        pthread_mutex_lock(&g_bindingMutex);
        int park = ctx->multi_thread && threadHoldsBinding(ctx);
        pthread_mutex_unlock(&g_bindingMutex);
        if (park)
            releaseRequestedBinding(ctx);
    }
    pthread_mutex_unlock(&g_activeContextsMutex);
}

// Makes sure the main run loop gives up the main thread's bindings of contexts
// that other threads use whenever it goes idle: the main thread then holds such
// a context's binding only while it uses it, and its next GL call takes the
// binding again. A context that no other thread has used or asked for keeps
// its binding across idle: unbinding and rebinding every single-thread
// context at every pass of the run loop is not what its application expects.
static void ensureIdleObserver(void) {
    static int observer_added;
    pthread_mutex_lock(&g_bindingMutex);
    int add = !observer_added;
    observer_added = 1;
    pthread_mutex_unlock(&g_bindingMutex);
    if (!add)
        return;
    CFRunLoopObserverRef observer = CFRunLoopObserverCreateWithHandler(
        NULL, kCFRunLoopBeforeWaiting | kCFRunLoopExit, true, LONG_MAX,
        ^(CFRunLoopObserverRef o, CFRunLoopActivity activity) {
            (void) o;
            (void) activity;
            parkMultiThreadBindings();
        });
    if (observer) {
        CFRunLoopAddObserver(CFRunLoopGetMain(), observer, kCFRunLoopCommonModes);
        CFRelease(observer);
    }
}

// Marks context as used by more than one thread.
static void noteMultiThread(CGLContextObj context) {
    pthread_mutex_lock(&g_bindingMutex);
    context->multi_thread = 1;
    pthread_mutex_unlock(&g_bindingMutex);
    ensureIdleObserver();
}

// Runs on the main thread's run loop. The context was retained for it.
static void releaseBindingOnThisThread(CGLContextObj context) {
    releaseRequestedBinding(context);
    CGLReleaseContext(context);
}

// The main thread is about to wait for the context's lock, which the thread
// that asked for the binding may hold while it waits for the main thread:
// answer the request first instead of leaving both waiting.
static void answerPendingRelease(CGLContextObj context) {
    if (!pthread_main_np())
        return;
    pthread_mutex_lock(&g_bindingMutex);
    int pending = context->release_pending && threadHoldsBinding(context);
    pthread_mutex_unlock(&g_bindingMutex);
    if (pending)
        releaseRequestedBinding(context);
}

static CGLError bindCurrentContext(CGLContextObj context);

static struct timespec handoffDeadline(void) {
    struct timeval now;
    gettimeofday(&now, NULL);
    struct timespec deadline = { now.tv_sec + HANDOFF_WAIT_SECONDS, (long) now.tv_usec * 1000 };
    if (deadline.tv_nsec >= 1000 * 1000 * 1000) {
        deadline.tv_sec++;
        deadline.tv_nsec -= 1000 * 1000 * 1000;
    }
    return deadline;
}

// Gives the binding up on the calling thread, which is not the main thread,
// if the main thread is waiting for it. Called where this thread enters or
// leaves the context's lock, i.e. between uses of the context.
static void releaseWantedBinding(CGLContextObj context) {
    if (pthread_main_np())
        return;
    pthread_mutex_lock(&g_bindingMutex);
    if (context->release_wanted && threadHoldsBinding(context) &&
        p_eglMakeCurrent(context->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
        context->binding = CGL_BINDING_PARKED;
        context->release_count++;
        if (gl_tls_bound == context)
            gl_tls_bound = NULL;
        context->release_wanted = 0;
        pthread_cond_broadcast(&g_bindingCond);
    }
    pthread_mutex_unlock(&g_bindingMutex);
}

// Called when binding was refused because another thread holds the context.
// Returns whether a release was requested and awaited (the caller retries).
static int requestRelease(CGLContextObj context) {
    noteMultiThread(context);
    pthread_mutex_lock(&g_bindingMutex);
    // The holder gave the binding up between the refused bind and here: bind
    // again.
    if (context->binding != CGL_BINDING_BOUND) {
        pthread_mutex_unlock(&g_bindingMutex);
        return 1;
    }
    struct timespec deadline = handoffDeadline();
    unsigned start = context->release_count;
    int released;
    if (!context->binding_thread_is_main) {
        // Another thread than the main thread holds the context and the main
        // thread, for which a context is current whatever other threads do,
        // wants it back: that thread gives it up where it next locks or
        // unlocks the context.
        if (!pthread_main_np()) {
            pthread_mutex_unlock(&g_bindingMutex);
            return 0;
        }
        context->release_wanted = 1;
        while (context->release_count == start && context->release_wanted) {
            if (pthread_cond_timedwait(&g_bindingCond, &g_bindingMutex, &deadline) != 0)
                break;
        }
        context->release_wanted = 0;
        released = context->release_count != start;
        pthread_mutex_unlock(&g_bindingMutex);
        return released;
    }
    if (pthread_main_np()) {
        pthread_mutex_unlock(&g_bindingMutex);
        return 0;
    }
    if (!context->release_pending) {
        context->release_pending = 1;
        CGLRetainContext(context);
        CFRunLoopRef main_loop = CFRunLoopGetMain();
        CFRunLoopPerformBlock(main_loop, kCFRunLoopCommonModes, ^{ releaseBindingOnThisThread(context); });
        CFRunLoopWakeUp(main_loop);
    }
    // (A request that another thread made is waited for in the same way.)
    while (context->release_count == start && context->release_pending) {
        if (pthread_cond_timedwait(&g_bindingCond, &g_bindingMutex, &deadline) != 0)
            break;
    }
    released = context->release_count != start;
    // No answer in time: do not leave the request pending, or no later
    // refusal could ask again.
    if (!released)
        context->release_pending = 0;
    pthread_mutex_unlock(&g_bindingMutex);
    return released;
}

// Makes context current without bringing IOSurface contents into its
// rectangle textures; used inside this framework.
static CGLError setCurrentContextEx(CGLContextObj context, int may_wait) {
    CGLContextObj previous = CGLGetCurrentContext();
    CGLError error = bindCurrentContext(context);
    // A bind that fails because another thread holds the context asks that
    // thread to release it; the thread that got released may bind it again
    // before this one does (or be in the middle of binding it), so try again
    // a few times.
    for (int attempt = 0; may_wait && error != kCGLNoError && context != NULL && attempt < 20; attempt++) {
        pthread_mutex_lock(&g_bindingMutex);
        int held_elsewhere = context->binding == CGL_BINDING_BOUND &&
                             !pthread_equal(context->binding_thread, pthread_self());
        int multi_thread = context->multi_thread;
        pthread_mutex_unlock(&g_bindingMutex);
        if (held_elsewhere) {
            if (!requestRelease(context))
                break;
        } else if (multi_thread || attempt < 5) {
            // Another thread may be in the middle of binding the context (its
            // binding is recorded only after EGL accepted it): try again.
            usleep(1000);
        } else {
            break;
        }
        error = bindCurrentContext(context);
    }
    pthread_mutex_lock(&g_bindingMutex);
    if (error == kCGLNoError) {
        // Binding another context (or none) releases the previous one.
        if (previous != NULL && previous != context && threadHoldsBinding(previous)) {
            previous->binding = CGL_BINDING_FREE;
            previous->release_count++;
            gl_tls_bound = NULL;
            // A thread that was refused the context may be waiting for this.
            pthread_cond_broadcast(&g_bindingCond);
        }
        if (context != NULL) {
            context->binding = CGL_BINDING_BOUND;
            context->binding_thread = pthread_self();
            context->binding_thread_is_main = pthread_main_np();
        }
        gl_tls_bound = context;
    }
    pthread_mutex_unlock(&g_bindingMutex);
    return error;
}

static CGLError setCurrentContext(CGLContextObj context) {
    return setCurrentContextEx(context, 1);
}

static void refreshRectangleTextures(CGLContextObj context);

// For a context that more than one thread uses, making it current only
// records it as this thread's current context: the host binding is taken by the
// first GL call (see gl_acquire_binding) and given up again at the outermost
// CGLUnlockContext, so that no thread keeps the binding while it does
// something else. Without this a thread that makes the context current and
// then waits (for an application lock, say) keeps the others from it.
static int setCurrentContextLazily(CGLContextObj context) {
    pthread_mutex_lock(&g_bindingMutex);
    int lazy = context->multi_thread && !threadHoldsBinding(context);
    pthread_mutex_unlock(&g_bindingMutex);
    if (!lazy || !native_egl_available())
        return 0;
    CGLContextObj previous = CGLGetCurrentContext();
    if (previous != NULL && previous != context) {
        pthread_mutex_lock(&g_bindingMutex);
        if (threadHoldsBinding(previous) &&
            p_eglMakeCurrent(previous->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
            previous->binding = CGL_BINDING_FREE;
            previous->release_count++;
            if (gl_tls_bound == previous)
                gl_tls_bound = NULL;
            pthread_cond_broadcast(&g_bindingCond);
        }
        pthread_mutex_unlock(&g_bindingMutex);
    }
    pthread_setspecific(get_current_context_key(), context);
    return 1;
}

CGLError CGLSetCurrentContext(CGLContextObj context) {
    CGLError error = (context != NULL && setCurrentContextLazily(context)) ? kCGLNoError : setCurrentContext(context);
    // A context that becomes current is about to be used, so rectangle
    // textures specified from surfaces the CPU has written since get the
    // new contents.
    if (error == kCGLNoError && context != NULL)
        refreshRectangleTextures(context);
    return error;
}

// The dispatch table of a context (CGL_MACRO). Each entry makes the context
// current on the calling thread when it is not, and calls the host function.
static int gli_enter(GLIContext rend) {
    CGLContextObj context = (CGLContextObj) rend;
    if (CGLGetCurrentContext() != context)
        CGLSetCurrentContext(context);
    if (GL_BOUND_HERE())
        return 1;
    return gl_acquire_binding();
}

__thread void *gl_tls_bound;

static void gli_missing(void) {}

// The host's OpenGL library (the wrapper this framework re-exports): where the
// real functions live, as the framework's own gl* exports have the same names.
// It is not the public libGL.dylib, which re-exports this framework.
static void *gl_native_handle(void) {
    static void *handle;
    if (handle == NULL)
        handle = dlopen(GL_HOST_LIBRARY, RTLD_LAZY);
    return handle;
}

static void *gl_native_symbol(const char *name) {
    void *handle = gl_native_handle();
    return handle != NULL ? dlsym(handle, name) : NULL;
}

// may_wait: a thread refused the context waits for the thread that holds it to
// give it up (see requestRelease); without it the refusal is final.
static int acquireBinding(int may_wait) {
    CGLContextObj context = CGLGetCurrentContext();
    if (context == NULL)
        return 0;
    if (setCurrentContextEx(context, may_wait) != kCGLNoError)
        return 0;
    if (gl_tls_bound == NULL)
        return 0;
    // Outside CGLLockContext, on a context that other threads use, the binding
    // is only for this call.
    pthread_mutex_lock(&g_bindingMutex);
    int in_lock = context->lock_depth > 0 && pthread_equal(context->lock_owner, pthread_self());
    int transient = context->multi_thread && !in_lock;
    pthread_mutex_unlock(&g_bindingMutex);
    return transient ? 2 : 1;
}

int gl_acquire_binding(void) {
    return acquireBinding(1);
}

void gl_release_transient(void) {
    CGLContextObj context = gl_tls_bound;
    if (context == NULL)
        return;
    pthread_mutex_lock(&g_bindingMutex);
    if (threadHoldsBinding(context) &&
        p_eglMakeCurrent(context->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
        context->binding = CGL_BINDING_PARKED;
        context->release_count++;
        gl_tls_bound = NULL;
        pthread_cond_broadcast(&g_bindingCond);
    }
    pthread_mutex_unlock(&g_bindingMutex);
}

extern const char *const gl_export_names[];

// Called by the generated trampolines (gl_trampolines.c) of the host names no
// signature is known for, with the argument registers saved. The binding is
// kept: nothing is left to run after the host function.
void *gl_trampoline_target(unsigned index) {
    if (!GL_BOUND_HERE())
        gl_acquire_binding();
    void *function = gl_trampoline_functions[index];
    if (function == NULL) {
        function = gl_native_symbol(gl_trampoline_names[index]);
        if (function == NULL)
            function = (void *) gli_missing;
        gl_trampoline_functions[index] = function;
    }
    return function;
}

void *gl_resolve_real(unsigned index) {
    void *function = gl_native_symbol(gl_export_names[index]);
    if (function == NULL)
        function = (void *) gli_missing;
    gl_real_functions[index] = function;
    return function;
}

static void *gli_host(unsigned slot);

#include "gli_thunks.inc"

_Static_assert(sizeof(GLIFunctionDispatch) == GLI_SLOT_COUNT * sizeof(void *), "one pointer per dispatch slot");

static void *gli_host_functions[975];

static void *gli_host(unsigned slot) {
    void *function = gli_host_functions[slot];
    if (function == NULL) {
        for (const char *const *name = gli_host_names[slot]; *name != NULL && function == NULL; name++)
            function = gl_native_symbol(*name);
        if (function == NULL)
            function = (void *) gli_missing;
        gli_host_functions[slot] = function;
    }
    return function;
}

static int enable_bit(CGLContextEnable pname) {
    switch (pname) {
    case kCGLCESwapRectangle: return 0;
    case kCGLCESwapLimit: return 1;
    case kCGLCERasterization: return 2;
    case kCGLCEStateValidation: return 3;
    case kCGLCESurfaceBackingSize: return 4;
    case kCGLCEDisplayListOptimization: return 5;
    case kCGLCEMPEngine: return 6;
    default: return -1;
    }
}

static CGLError set_enabled(CGLContextObj ctx, CGLContextEnable pname, int on) {
    if (ctx == NULL)
        return kCGLBadContext;
    int bit = enable_bit(pname);
    if (bit < 0)
        return kCGLBadEnumeration;
    pthread_mutex_lock(&ctx->lock);
    if (on)
        ctx->enables |= 1u << bit;
    else
        ctx->enables &= ~(1u << bit);
    pthread_mutex_unlock(&ctx->lock);
    return kCGLNoError;
}

CGLError CGLEnable(CGLContextObj ctx, CGLContextEnable pname) {
    return set_enabled(ctx, pname, 1);
}

CGLError CGLDisable(CGLContextObj ctx, CGLContextEnable pname) {
    return set_enabled(ctx, pname, 0);
}

CGLError CGLIsEnabled(CGLContextObj ctx, CGLContextEnable pname, GLint *enable) {
    if (enable == NULL)
        return kCGLBadAddress;
    *enable = 0;
    if (ctx == NULL)
        return kCGLBadContext;
    int bit = enable_bit(pname);
    if (bit < 0)
        return kCGLBadEnumeration;
    pthread_mutex_lock(&ctx->lock);
    *enable = (ctx->enables >> bit) & 1;
    pthread_mutex_unlock(&ctx->lock);
    return kCGLNoError;
}

const char *CGLErrorString(CGLError error) {
    switch (error) {
    case kCGLNoError: return "no error";
    case kCGLBadAttribute: return "invalid pixel format attribute";
    case kCGLBadProperty: return "invalid renderer property";
    case kCGLBadPixelFormat: return "invalid pixel format";
    case kCGLBadRendererInfo: return "invalid renderer info";
    case kCGLBadContext: return "invalid context";
    case kCGLBadDrawable: return "invalid drawable";
    case kCGLBadDisplay: return "invalid display";
    case kCGLBadState: return "invalid context state";
    case kCGLBadValue: return "invalid numerical value";
    case kCGLBadMatch: return "invalid share context";
    case kCGLBadEnumeration: return "invalid enumerant";
    case kCGLBadOffScreen: return "invalid offscreen drawable";
    case kCGLBadFullScreen: return "invalid fullscreen drawable";
    case kCGLBadWindow: return "invalid window";
    case kCGLBadAddress: return "invalid pointer";
    case kCGLBadCodeModule: return "invalid code module";
    case kCGLBadAlloc: return "memory allocation failure";
    case kCGLBadConnection: return "invalid CoreGraphics connection";
    default: return "unknown error";
    }
}

// A context always uses virtual screen 0, the only one there is.
CGLError CGLGetVirtualScreen(CGLContextObj context, GLint *screen) {
    if (!context)
        return kCGLBadContext;
    if (!screen)
        return kCGLBadAddress;
    *screen = 0;
    return kCGLNoError;
}

CGLError CGLSetVirtualScreen(CGLContextObj context, GLint screen) {
    if (!context)
        return kCGLBadContext;
    return screen == 0 ? kCGLNoError : kCGLBadValue;
}

// Releases the surface and window of a fullscreen drawable that was taken out
// of a context (see CGLClearDrawable and CGLSetFullScreenOnDisplay).
static void destroyFullScreen(CGLContextObj ctx, CGSConnectionID connection, CGSWindowID window,
                              EGLSurface surface) {
    if (surface != EGL_NO_SURFACE)
        p_eglDestroySurface(ctx->egl_display, surface);
    if (window != 0)
        CGSReleaseWindow(connection, window);
}

// The display a fullscreen context covers: the one named by the display mask,
// or the main display when the pixel format has no mask.
static CGDirectDisplayID fullScreenDisplay(CGLContextObj ctx, int use_format_mask, GLuint mask) {
    if (use_format_mask) {
        GLint format_mask = 0;
        if (CGLDescribePixelFormat(ctx->pixel_format, 0, kCGLPFADisplayMask, &format_mask) != kCGLNoError)
            return CGMainDisplayID();
        mask = (GLuint) format_mask;
        // A pixel format without a display mask attribute describes none.
        if (mask == 0)
            return CGMainDisplayID();
    }
    return CGOpenGLDisplayMaskToDisplayID(mask);
}

static CGLError setFullScreen(CGLContextObj ctx, int use_format_mask, GLuint mask) {
    if (!ctx)
        return kCGLBadContext;

    // Only a context whose pixel format asked for kCGLPFAFullScreen can be
    // given a fullscreen drawable.
    GLint wanted = 0;
    if (!ctx->pixel_format ||
        CGLDescribePixelFormat(ctx->pixel_format, 0, kCGLPFAFullScreen, &wanted) != kCGLNoError ||
        !wanted)
        return kCGLBadFullScreen;

    if (!use_format_mask && mask == 0)
        return kCGLBadDisplay;
    CGDirectDisplayID display = fullScreenDisplay(ctx, use_format_mask, mask);
    if (display == kCGNullDirectDisplay)
        return kCGLBadDisplay;

    CGSConnectionID cid = CGSMainConnectionID();
    if (!getCGLDisplay(cid))
        return kCGLBadConnection;

    // A window of the display's size above everything else, which leaves the
    // application's own windows to receive the input; the context draws into it.
    CGSWindowID window = 0;
    void *drawable = NULL;
    if (_CGSCreateDisplayOverlayWindow(cid, CGDisplayBounds(display), &window, &drawable) != kCGErrorSuccess)
        return kCGLBadWindow;
    EGLSurface surface = p_eglCreateWindowSurface(ctx->egl_display, ctx->egl_config,
                                                  (EGLNativeWindowType) drawable, NULL);
    if (surface == EGL_NO_SURFACE) {
        CGSReleaseWindow(cid, window);
        return kCGLBadState;
    }

    pthread_mutex_lock(&ctx->lock);
    CGSConnectionID old_connection = ctx->fullscreen_connection;
    CGSWindowID old_window = ctx->fullscreen_window;
    EGLSurface old_surface = ctx->fullscreen_surface;
    EGLSurface old_fallback = ctx->fallback_pbuffer;
    ctx->fullscreen_connection = cid;
    ctx->fullscreen_window = window;
    ctx->fullscreen_surface = surface;
    ctx->fallback_pbuffer = EGL_NO_SURFACE;
    atomic_store_explicit(&ctx->egl_surface, surface, memory_order_release);
    int is_current = (CGLGetCurrentContext() == ctx);
    pthread_mutex_unlock(&ctx->lock);

    // A context that is current draws into the new surface from now on.
    if (is_current)
        setCurrentContext(ctx);
    if (old_fallback != EGL_NO_SURFACE)
        p_eglDestroySurface(ctx->egl_display, old_fallback);
    destroyFullScreen(ctx, old_connection, old_window, old_surface);
    return kCGLNoError;
}

CGLError CGLSetFullScreenOnDisplay(CGLContextObj ctx, GLuint display_mask) {
    return setFullScreen(ctx, 0, display_mask);
}

CGLError CGLSetFullScreen(CGLContextObj ctx) {
    return setFullScreen(ctx, 1, 0);
}

CGLError CGLChoosePixelFormat(
    const CGLPixelFormatAttribute *attrs,
    CGLPixelFormatObj *result,
    GLint *number_of_screens
) {
    if (!attrs) {
        return kCGLBadAttribute;
    }
    if (!result) {
        return kCGLBadAddress;
    }

    CGLPixelFormatObj format = malloc(sizeof(struct _CGLPixelFormatObj));
    if (!format) {
        return kCGLBadAlloc;
    }

    int count = attributes_count(attrs);

    pthread_mutex_lock(&registration_lock);
    format->egl_display = display;
    format->egl_config = config;
    format->swap_interval = default_swap_interval;
    format->paced = default_paced;
    pthread_mutex_unlock(&registration_lock);
    format->retain_count = 1;
    format->attributes = malloc(sizeof(CGLPixelFormatAttribute) * (count + 1));
    if (!format->attributes) {
        free(format);
        return kCGLBadAlloc;
    }
    for (int i = 0; i <= count; i++) {
        format->attributes[i] = attrs[i];
    }

    *result = format;
    if (number_of_screens) {
        *number_of_screens = 1;
    }

    return kCGLNoError;
}

CGLError CGLClearDrawable(CGLContextObj ctx)
{
    if (!ctx)
        return kCGLBadContext;

    pthread_mutex_lock(&ctx->lock);
    EGLSurface old_fallback = ctx->fallback_pbuffer;
    ctx->fallback_pbuffer = EGL_NO_SURFACE;
    CGSConnectionID fullscreen_connection = ctx->fullscreen_connection;
    CGSWindowID fullscreen_window = ctx->fullscreen_window;
    EGLSurface fullscreen_surface = ctx->fullscreen_surface;
    ctx->fullscreen_connection = 0;
    ctx->fullscreen_window = 0;
    ctx->fullscreen_surface = EGL_NO_SURFACE;
    atomic_store_explicit(&ctx->egl_surface, (EGLSurface)EGL_NO_SURFACE, memory_order_release);
    int is_current = (CGLGetCurrentContext() == ctx);
    pthread_mutex_unlock(&ctx->lock);
    // Whether this thread holds the context: only then is the surface detached
    // from the EGL binding here. A context that is current but not held is
    // bound without it the next time (see bindCurrentContext).
    pthread_mutex_lock(&g_bindingMutex);
    int held = is_current && threadHoldsBinding(ctx);
    pthread_mutex_unlock(&g_bindingMutex);

    /* egl_display and egl_context are immutable after CGLCreateContext */
    if (ctx->egl_display != EGL_NO_DISPLAY) {
        if (old_fallback != EGL_NO_SURFACE) {
            p_eglDestroySurface(ctx->egl_display, old_fallback);
        }
        if (held) {
            // Disassociate any surface from the current thread's EGL context
            p_eglMakeCurrent(ctx->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx->egl_context);
        }
        // Leaving fullscreen: the surface is unbound now, so it can go together with its window.
        destroyFullScreen(ctx, fullscreen_connection, fullscreen_window, fullscreen_surface);
        // A current context that was drawing fullscreen stays usable: it gets
        // the small pbuffer a context without a drawable draws into, so that
        // its drawing calls do not fail with an incomplete framebuffer.
        if (is_current && fullscreen_surface != EGL_NO_SURFACE) {
            const EGLint pbufferAttribs[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
            EGLSurface pbuf = p_eglCreatePbufferSurface(ctx->egl_display, ctx->egl_config, pbufferAttribs);
            if (pbuf != EGL_NO_SURFACE
                && (!held || p_eglMakeCurrent(ctx->egl_display, pbuf, pbuf, ctx->egl_context))) {
                pthread_mutex_lock(&ctx->lock);
                ctx->fallback_pbuffer = pbuf;
                atomic_store_explicit(&ctx->egl_surface, pbuf, memory_order_release);
                pthread_mutex_unlock(&ctx->lock);
            } else if (pbuf != EGL_NO_SURFACE) {
                p_eglDestroySurface(ctx->egl_display, pbuf);
            }
        }
    }

    return kCGLNoError;
}

CGLError CGLUpdateContext(CGLContextObj ctx)
{
    return kCGLNoError;
}

CGLError CGLDescribePixelFormat(
    CGLPixelFormatObj format,
    GLint sreen_num,
    CGLPixelFormatAttribute attr,
    GLint *value
) {
    if (!format || !format->attributes) {
        return kCGLBadPixelFormat;
    }
    if (!value) {
        return kCGLBadAddress;
    }
    // Every pixel format has a single virtual screen, number 0.
    if (sreen_num != 0) {
        return kCGLBadValue;
    }
    if (attr == kCGLPFAVirtualScreenCount) {
        *value = 1;
        return kCGLNoError;
    }

    for (int i = 0; format->attributes[i] != 0; i++) {
        int has_arg = attribute_has_argument(format->attributes[i]);

        if (format->attributes[i] == attr) {
            if (has_arg) {
                *value = format->attributes[i + 1];
            } else {
                *value = 1;
            }
            return kCGLNoError;
        }

        if (has_arg) {
            i++;
        }
    }

    *value = 0;
    return kCGLNoError;
}

CGLPixelFormatObj CGLRetainPixelFormat(CGLPixelFormatObj format) {
    if (format == NULL) {
        return NULL;
    }

    __sync_fetch_and_add(&format->retain_count, 1);
    return format;
}

void CGLReleasePixelFormat(CGLPixelFormatObj format) {
    if (format == NULL) {
        return;
    }

    if (__sync_sub_and_fetch(&format->retain_count, 1) == 0) {
        free(format->attributes);
        free(format);
    }
}

CGLError CGLDestroyPixelFormat(CGLPixelFormatObj pixelFormat) {
    CGLReleasePixelFormat(pixelFormat);
    return kCGLNoError;
}

GLuint CGLGetPixelFormatRetainCount(CGLPixelFormatObj pixelFormat) {
    return pixelFormat->retain_count;
}

static CGLError createContext(CGLPixelFormatObj pixelFormat, CGLContextObj share,
                              const struct _CGLWindow *window, CGLContextObj *resultp) {

    if (resultp == NULL)
        return kCGLBadAddress;
    *resultp = NULL;
    if (!native_egl_available())
        return kCGLBadConnection;

    pthread_mutex_lock(&registration_lock);
    EGLDisplay owner = share ? share->egl_display : (pixelFormat ? pixelFormat->egl_display : display);
    EGLConfig owner_config = share ? share->egl_config : (pixelFormat ? pixelFormat->egl_config : config);
    int interval = share ? share->swap_interval : (pixelFormat ? pixelFormat->swap_interval : default_swap_interval);
    int paced = share ? share->paced : (pixelFormat ? pixelFormat->paced : default_paced);
    pthread_mutex_unlock(&registration_lock);
    if (window) {
        owner = window->display;
        owner_config = window->config;
    }
    if (owner == EGL_NO_DISPLAY) {
        owner = get_default_display(&owner_config);
        if (owner == EGL_NO_DISPLAY)
            return kCGLBadConnection;
    }
    if (!p_eglBindAPI(EGL_OPENGL_API))
        return kCGLBadState;
    // A NULL share context means no sharing of object state.
    EGLContext egl_share = share ? share->egl_context : EGL_NO_CONTEXT;
    EGLint context_attribs[16] = { EGL_NONE };
    int attr_idx = 0;
    GLint profile = 0;
    if (pixelFormat && CGLDescribePixelFormat(pixelFormat, 0, kCGLPFAOpenGLProfile, &profile) == kCGLNoError) {
        if (profile == kCGLOGLPVersion_3_2_Core) {
            context_attribs[attr_idx++] = EGL_CONTEXT_MAJOR_VERSION;
            context_attribs[attr_idx++] = 3;
            context_attribs[attr_idx++] = EGL_CONTEXT_MINOR_VERSION;
            context_attribs[attr_idx++] = 2;
            context_attribs[attr_idx++] = EGL_CONTEXT_OPENGL_PROFILE_MASK;
            context_attribs[attr_idx++] = EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT;
        } else if (profile == kCGLOGLPVersion_GL4_Core) {
            context_attribs[attr_idx++] = EGL_CONTEXT_MAJOR_VERSION;
            context_attribs[attr_idx++] = 4;
            context_attribs[attr_idx++] = EGL_CONTEXT_MINOR_VERSION;
            context_attribs[attr_idx++] = 1;
            context_attribs[attr_idx++] = EGL_CONTEXT_OPENGL_PROFILE_MASK;
            context_attribs[attr_idx++] = EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT;
        } else if (profile != 0 && profile != kCGLOGLPVersion_Legacy) {
            return kCGLBadValue;
        }
        /* Note: kCGLOGLPVersion_Legacy (0x1000) passes NULL attributes to request a compatibility context */
    }
    context_attribs[attr_idx] = EGL_NONE;
    // A core profile request is not downgraded to a compatibility context,
    // and a share context the new one cannot share with is an error.
    EGLContext egl_context = p_eglCreateContext(owner, owner_config, egl_share, attr_idx > 0 ? context_attribs : NULL);
    if (egl_context == EGL_NO_CONTEXT) {
        return egl_share != EGL_NO_CONTEXT ? kCGLBadMatch : kCGLBadContext;
    }

    CGLContextObj context = malloc(sizeof(struct _CGLContextObject));

    if (context == NULL) {
        p_eglDestroyContext(owner, egl_context);
        return kCGLBadAlloc;
    }

    context->rend = (GLIContext) context;
    memcpy(&context->disp, gli_template, sizeof(context->disp));
    context->priv = NULL;
    context->stak = NULL;
    context->retain_count = 1;
    pthread_mutexattr_t lock_attr;
    pthread_mutexattr_init(&lock_attr);
    pthread_mutexattr_settype(&lock_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&(context->lock), &lock_attr);
    pthread_mutexattr_destroy(&lock_attr);
    context->egl_display = owner;
    context->egl_config = owner_config;
    context->egl_context = egl_context;
    atomic_init(&context->egl_surface, (EGLSurface)EGL_NO_SURFACE);
    context->fallback_pbuffer = EGL_NO_SURFACE;
    context->fullscreen_connection = 0;
    context->fullscreen_window = 0;
    context->fullscreen_surface = EGL_NO_SURFACE;
    context->swap_interval = interval;
    context->paced = paced;
    context->pacer.period_ns = cgl_period_ns_from_millihertz(0);
    context->pacer.last_ns = 0;
    context->pixel_format = pixelFormat ? CGLRetainPixelFormat(pixelFormat) : NULL;
    context->profile = profile == kCGLOGLPVersion_Legacy ? 0 : profile;
    pthread_mutex_init(&context->profile_lock, NULL);
    context->profile_strings = NULL;
    context->enables = 0;
    context->binding = CGL_BINDING_FREE;
    context->multi_thread = 0;
    context->release_pending = 0;
    context->release_wanted = 0;
    context->binding_thread_is_main = 0;
    context->lock_depth = 0;
    atomic_init(&context->dmabuf_import, 0);
    context->gpu_copy = 0;
    context->copy_framebuffers[0] = context->copy_framebuffers[1] = 0;
    static _Atomic uint64_t lastShareGroup;
    context->share_group = share ? share->share_group : atomic_fetch_add(&lastShareGroup, 1) + 1;
    context->max_color_attachments = 0;

    pthread_mutex_lock(&g_activeContextsMutex);
    context->prev_active = NULL;
    context->next_active = g_activeContexts;
    if (g_activeContexts != NULL) {
        g_activeContexts->prev_active = context;
    }
    g_activeContexts = context;
    pthread_mutex_unlock(&g_activeContextsMutex);

    *resultp = context;

    return kCGLNoError;
}

CGLError CGLCreateContext(CGLPixelFormatObj pixelFormat, CGLContextObj share, CGLContextObj *resultp) {
    return createContext(pixelFormat, share, NULL, resultp);
}

CGLError CGLCreateContextForWindow(CGLWindowRef window, CGLContextObj *resultp) {
    if (resultp)
        *resultp = NULL;
    if (!window)
        return kCGLBadDrawable;
    return createContext(NULL, NULL, window, resultp);
}

CGLContextObj CGLRetainContext(CGLContextObj context) {
    if (context == NULL) {
        return NULL;
    }

    __sync_fetch_and_add(&context->retain_count, 1);
    return context;
}

static void forgetSurfaceBindings(CGLContextObj context);
static void freeProfileStrings(struct cgl_profile_strings *strings);

void CGLReleaseContext(CGLContextObj context) {
    if (context == NULL) {
        return;
    }

    if (__sync_sub_and_fetch(&context->retain_count, 1) != 0) {
        return;
    }

    forgetSurfaceBindings(context);

    pthread_mutex_lock(&g_activeContextsMutex);
    if (context->prev_active != NULL) {
        context->prev_active->next_active = context->next_active;
    } else if (g_activeContexts == context) {
        g_activeContexts = context->next_active;
    }
    if (context->next_active != NULL) {
        context->next_active->prev_active = context->prev_active;
    }
    context->prev_active = NULL;
    context->next_active = NULL;
    pthread_mutex_unlock(&g_activeContextsMutex);

    if (CGLGetCurrentContext() == context) {
        CGLSetCurrentContext(NULL);
    }

    if (context->fallback_pbuffer != EGL_NO_SURFACE) {
        p_eglDestroySurface(context->egl_display, context->fallback_pbuffer);
        context->fallback_pbuffer = EGL_NO_SURFACE;
    }
    destroyFullScreen(context, context->fullscreen_connection, context->fullscreen_window,
                      context->fullscreen_surface);

    pthread_mutex_destroy(&(context->lock));

    if (context->pixel_format != NULL)
        CGLReleasePixelFormat(context->pixel_format);
    freeProfileStrings(context->profile_strings);
    pthread_mutex_destroy(&context->profile_lock);

    p_eglDestroyContext(context->egl_display, context->egl_context);

    free(context);
}

CGL_EXPORT CGLPixelFormatObj CGLGetPixelFormat(CGLContextObj context) {
    if (context == NULL)
        return NULL;
    return context->pixel_format;
}

GLuint CGLGetContextRetainCount(CGLContextObj context) {
    if (context == NULL) {
        return 0;
    }

    return context->retain_count;
}

CGLError CGLDestroyContext(CGLContextObj context) {
    CGLReleaseContext(context);

    return kCGLNoError;
}

CGLError CGLLockContext(CGLContextObj context) {
    if (context == NULL) {
        return kCGLBadContext;
    }
    answerPendingRelease(context);
    if (!pthread_main_np()) {
        pthread_mutex_lock(&g_bindingMutex);
        int nested = context->lock_depth > 0 && pthread_equal(context->lock_owner, pthread_self());
        pthread_mutex_unlock(&g_bindingMutex);
        if (!nested)
            releaseWantedBinding(context);
        // A second thread uses a context the main thread has bound.
        pthread_mutex_lock(&g_bindingMutex);
        int main_bound = context->binding_thread_is_main && context->binding != CGL_BINDING_FREE;
        pthread_mutex_unlock(&g_bindingMutex);
        if (main_bound)
            noteMultiThread(context);
    }
    if (pthread_main_np()) {
        // The thread that holds the lock may ask the main thread to release
        // the binding while this thread waits for the lock: keep answering.
        while (pthread_mutex_trylock(&(context->lock)) != 0) {
            answerPendingRelease(context);
            usleep(1000);
        }
    } else {
        pthread_mutex_lock(&(context->lock));
    }
    if (context->lock_depth++ == 0) {
        pthread_mutex_lock(&g_bindingMutex);
        context->lock_owner = pthread_self();
        pthread_mutex_unlock(&g_bindingMutex);
    }
    // The binding that was handed to another thread or parked is not taken
    // back here: the first GL call of this thread on the context does that (see
    // gl_acquire_binding), so a thread that only locks the context never waits
    // for the thread that holds the binding.
    // Locking the current context precedes drawing with it, so rectangle
    // textures specified from surfaces the CPU has written since get the
    // new contents.
    if (CGLGetCurrentContext() == context)
        refreshRectangleTextures(context);
    return kCGLNoError;
}

CGLError CGLUnlockContext(CGLContextObj context) {
    if (context == NULL) {
        return kCGLBadContext;
    }
    if (context->lock_depth > 0 && --context->lock_depth == 0) {
        pthread_mutex_lock(&g_bindingMutex);
        context->lock_owner = (pthread_t) 0;
        pthread_mutex_unlock(&g_bindingMutex);
        releaseWantedBinding(context);
        // A thread's use of a context other threads use ends with the outermost
        // unlock: park the binding now (it also answers a request
        // that arrived while the lock was held, see releaseRequestedBinding).
        // macOS leaves a context that several threads use unsafe to touch
        // without the lock, and a thread that wants the context while the
        // main thread is blocked outside this framework (on an application
        // lock, say) would otherwise wait for a binding nobody gives up.
        parkMultiThreadBinding(context);
    }
    pthread_mutex_unlock(&(context->lock));
    return kCGLNoError;
}

static CGLError flushDrawable(CGLContextObj context) {
    EGLSurface surf = atomic_load_explicit(&context->egl_surface, memory_order_acquire);
    if (surf == EGL_NO_SURFACE) {
        glFlush();
        return kCGLNoError;
    }
    // Swapping needs the context bound on this thread; a thread without it
    // (for instance one that was refused the context, see the thread
    // hand-off notes at setCurrentContext) must not reach the driver.
    pthread_mutex_lock(&g_bindingMutex);
    int bound_here = threadHoldsBinding(context);
    pthread_mutex_unlock(&g_bindingMutex);
    if (!bound_here)
        return kCGLBadContext;
    glFinish();
    // Re-check after glFinish() in case CGLDestroyWindow cleared the surface concurrently
    EGLSurface surf_after = atomic_load_explicit(&context->egl_surface, memory_order_acquire);
    if (surf_after == EGL_NO_SURFACE || surf_after != surf) {
        return kCGLNoError;
    }
    if (context->paced)
        cgl_swap_pace(&context->pacer, context->swap_interval);
    EGLBoolean ok = p_eglSwapBuffers(context->egl_display, surf);
    if (ok)
        postWindowDidSwap(surf);
    return ok ? kCGLNoError : kCGLBadDrawable;
}

CGLError CGLFlushDrawable(CGLContextObj context) {
    if (context == NULL) {
        return kCGLBadContext;
    }
    // Another thread may hold the host binding of the context that is current
    // here, or the thread's own was parked or given up: get it (see
    // gl_acquire_binding), and give a binding taken for this call up again.
    int acquired = 1;
    if (!GL_BOUND_HERE() && CGLGetCurrentContext() == context)
        acquired = gl_acquire_binding();
    CGLError error = flushDrawable(context);
    if (acquired == 2)
        gl_release_transient();
    return error;
}

// GL_APPLE_flush_render: swaps the drawable of the current context, like
// CGLFlushDrawable on it. The function has no result, and with no current
// context there is nothing to swap.
void glSwapAPPLE(void) {
    CGLContextObj context = CGLGetCurrentContext();
    if (context != NULL)
        CGLFlushDrawable(context);
}

// GL functions this framework exports in place of the host's, so that
// rendering into IOSurface-backed textures reaches the surfaces (see
// "Writing GL rendering back into surfaces" below). The framework's own
// calls go straight to the host's through these.
#define CGL_HOST_GL_FUNCTIONS(X) \
    X(glFlush) \
    X(glFinish) \
    X(glFenceSync) \
    X(glDeleteTextures) \
    X(glTexImage2D) \
    X(glBindFramebuffer) \
    X(glBindFramebufferEXT) \
    X(glDeleteFramebuffers) \
    X(glDeleteFramebuffersEXT) \
    X(glGetFramebufferAttachmentParameteriv)

#define CGL_HOST_GL_DECLARE(name) static __typeof__(name) *p_host_##name;
CGL_HOST_GL_FUNCTIONS(CGL_HOST_GL_DECLARE)
#undef CGL_HOST_GL_DECLARE

// The library this framework re-exports the host's GL functions from.
#define CGL_HOST_GL_LIBRARY GL_HOST_LIBRARY

static void load_host_gl(void) {
    void *library = dlopen(CGL_HOST_GL_LIBRARY, RTLD_LAZY | RTLD_LOCAL);
    if (!library)
        return;
#define CGL_HOST_GL_LOAD(name) p_host_##name = (__typeof__(p_host_##name)) dlsym(library, #name);
    CGL_HOST_GL_FUNCTIONS(CGL_HOST_GL_LOAD)
#undef CGL_HOST_GL_LOAD
}

static void ensure_host_gl(void) {
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    pthread_once(&once, load_host_gl);
}

// host_glFlush() and so on: the host's function, or nothing without one.
#define CGL_HOST_GL_CALL(result, name, params, args) \
    static result host_##name params { \
        ensure_host_gl(); \
        if (p_host_##name) \
            return p_host_##name args; \
        return (result) 0; \
    }
#define CGL_HOST_GL_CALL_VOID(name, params, args) \
    static void host_##name params { \
        ensure_host_gl(); \
        if (p_host_##name) \
            p_host_##name args; \
    }
CGL_HOST_GL_CALL_VOID(glFlush, (void), ())
CGL_HOST_GL_CALL_VOID(glFinish, (void), ())
CGL_HOST_GL_CALL(GLsync, glFenceSync, (GLenum condition, GLbitfield flags), (condition, flags))
CGL_HOST_GL_CALL_VOID(glDeleteTextures, (GLsizei n, const GLuint *textures), (n, textures))
CGL_HOST_GL_CALL_VOID(glTexImage2D, (GLenum target, GLint level, GLint internalFormat, GLsizei width,
                                     GLsizei height, GLint border, GLenum format, GLenum type,
                                     const GLvoid *pixels),
                      (target, level, internalFormat, width, height, border, format, type, pixels))
CGL_HOST_GL_CALL_VOID(glBindFramebuffer, (GLenum target, GLuint framebuffer), (target, framebuffer))
CGL_HOST_GL_CALL_VOID(glBindFramebufferEXT, (GLenum target, GLuint framebuffer), (target, framebuffer))
CGL_HOST_GL_CALL_VOID(glDeleteFramebuffers, (GLsizei n, const GLuint *framebuffers), (n, framebuffers))
CGL_HOST_GL_CALL_VOID(glDeleteFramebuffersEXT, (GLsizei n, const GLuint *framebuffers), (n, framebuffers))
CGL_HOST_GL_CALL_VOID(glGetFramebufferAttachmentParameteriv,
                      (GLenum target, GLenum attachment, GLenum pname, GLint *params),
                      (target, attachment, pname, params))
#undef CGL_HOST_GL_CALL
#undef CGL_HOST_GL_CALL_VOID

// IOSurface-backed textures.
//
// On macOS the IOSurface is the storage of a texture that
// CGLTexImageIOSurface2D binds to it. Here that holds when IOSurface can
// export the surface's memory as a DMA-BUF and the host driver imports it
// (EGL_EXT_image_dma_buf_import) into a GL_TEXTURE_2D texture: the texture
// then uses the plane's memory itself, CPU writes reach it and GL rendering
// reaches the surface without a copy. Before a lock of the surface, or the
// creation of a Mach port for it, this framework waits for the context's
// GL work to finish, so that the CPU and other processes see it.
//
// Otherwise, for other targets and formats or without driver support, the
// plane's pixels are copied into the texture by the call, and
// CGLReadbackIOSurface copies the texture back into the surface. CPU writes
// to the surface after the call reach the texture only through another
// call. GL rendering into the texture reaches the surface when the context
// flushes it, before the texture is deleted or given other contents (see
// "Writing GL rendering back into surfaces"), and through
// CGLReadbackIOSurface, which IOSurface also calls before a lock of the
// surface or the creation of a Mach port for it (see writeBackSurface).
//
// Each texture remembers the surface it was last specified from. The binding
// retains the surface, as the texture would on macOS, and is dropped when
// the same texture is specified from another surface, when the texture is
// deleted, or when the context is destroyed.

struct surface_binding {
    CGLContextObj context;
    GLuint texture;
    GLenum target;
    GLenum format;
    GLenum type;
    GLuint plane;
    IOSurfaceRef surface;
    // The surface's seed when the texture and the surface last held the
    // same contents.
    uint32_t seed;
    // The texture uses the surface's memory (imported as a DMA-BUF) with
    // this internal format and size, instead of holding a copy.
    int imported;
    GLenum internalFormat;
    GLsizei width, height;
    // For a rectangle texture, which cannot use the surface's memory: a 2D
    // texture of the same context that does (the shadow), with the DRM
    // format it was imported as. The rectangle texture holds a copy that
    // GPU copies keep in step with the shadow. Bindings of one context to
    // the same plane in the same format and size share a shadow.
    GLuint shadow;
    uint32_t shadowFormat;
    // The texture may have been rendered into since it was last written
    // back (one that holds a copy) or since the binding was made (one that
    // uses the surface's memory), and when that was last noted (a value of
    // g_renderSerial; 0 if never).
    int dirty;
    uint64_t rendered;
    // A texture that uses the surface's memory has been rendered into since
    // a context that shares it last flushed.
    int unflushed;
    // The texture (imported) or its shadow uses the surface's GPU buffer
    // instead of its memory (see "GPU buffers"), or giving the surface one
    // failed and is not tried again for this binding.
    int gpuBuffer;
    int noGPUBuffer;
};

static struct surface_binding *g_surfaceBindings;
static size_t g_surfaceBindingCount, g_surfaceBindingCapacity;
static pthread_mutex_t g_surfaceBindingsMutex = PTHREAD_MUTEX_INITIALIZER;
// The number of bindings, read without the lock so that GL calls skip the
// writeback work when there are none.
static _Atomic size_t g_bindingCount;
// Orders the notes of rendering. Guarded by g_surfaceBindingsMutex.
static uint64_t g_renderSerial;

// The share group number is the identity of the group (a number is never
// reused, so it stays distinct from the groups of later contexts); the
// group object an application sees is that number.
CGLShareGroupObj CGLGetShareGroup(CGLContextObj ctx) {
    return ctx ? (CGLShareGroupObj)(uintptr_t) ctx->share_group : NULL;
}

// Whether the two contexts share textures.
static int sharesTextures(CGLContextObj a, CGLContextObj b) {
    return a == b || a->share_group == b->share_group;
}

// Rendering into the surface's memory or GPU buffer that no flush has
// reported yet changed the contents all the same. Called with the bindings
// locked.
static void reportUnflushedBinding(const struct surface_binding *binding) {
    if (!binding->unflushed)
        return;
    // The rendering has to be complete before it is reported (IOSurface
    // copies a buffer into the memory, and a reader of the memory has no
    // other way to wait for it); it can only be waited for where the
    // context is current.
    CGLContextObj current = CGLGetCurrentContext();
    if (current != NULL && sharesTextures(current, binding->context)
        && (binding->gpuBuffer || _IOSurfaceIsShared(binding->surface)))
        host_glFinish();
    if (binding->gpuBuffer)
        _IOSurfaceGPUDidWriteBuffer(binding->surface);
    else
        _IOSurfaceGPUDidWriteMemory(binding->surface);
}

// Removes the binding at index i. Called with the bindings locked.
static void removeSurfaceBinding(size_t i) {
    atomic_fetch_sub(&g_bindingCount, 1);
    reportUnflushedBinding(&g_surfaceBindings[i]);
    CFRelease(g_surfaceBindings[i].surface);
    g_surfaceBindings[i] = g_surfaceBindings[--g_surfaceBindingCount];
}

static void forgetSurfaceBindings(CGLContextObj context) {
    pthread_mutex_lock(&g_surfaceBindingsMutex);
    for (size_t i = 0; i < g_surfaceBindingCount; ) {
        if (g_surfaceBindings[i].context == context) {
            removeSurfaceBinding(i);
        } else {
            i++;
        }
    }
    pthread_mutex_unlock(&g_surfaceBindingsMutex);
}

// Looks up the binding of a texture. Returns 0 if it has none.
static int findSurfaceBinding(CGLContextObj context, GLuint texture, struct surface_binding *binding) {
    int found = 0;
    pthread_mutex_lock(&g_surfaceBindingsMutex);
    for (size_t i = 0; i < g_surfaceBindingCount; i++) {
        if (g_surfaceBindings[i].context == context && g_surfaceBindings[i].texture == texture) {
            *binding = g_surfaceBindings[i];
            found = 1;
            break;
        }
    }
    pthread_mutex_unlock(&g_surfaceBindingsMutex);
    return found;
}

// Whether a binding of context other than the one at index skip uses shadow.
// Called with the bindings locked.
static int shadowInUse(CGLContextObj context, GLuint shadow, size_t skip) {
    for (size_t i = 0; i < g_surfaceBindingCount; i++) {
        if (i != skip && g_surfaceBindings[i].context == context && g_surfaceBindings[i].shadow == shadow)
            return 1;
    }
    return 0;
}

static int shadowInUseByAny(CGLContextObj context, GLuint shadow) {
    pthread_mutex_lock(&g_surfaceBindingsMutex);
    int used = shadowInUse(context, shadow, SIZE_MAX);
    pthread_mutex_unlock(&g_surfaceBindingsMutex);
    return used;
}

// Records binding, replacing the texture's previous one. Called with the
// binding's context current, so that a shadow no binding uses any more is
// deleted.
static int rememberSurfaceBinding(const struct surface_binding *binding) {
    int ok = 1;
    GLuint unusedShadow = 0;
    pthread_mutex_lock(&g_surfaceBindingsMutex);
    size_t i;
    for (i = 0; i < g_surfaceBindingCount; i++) {
        if (g_surfaceBindings[i].context == binding->context && g_surfaceBindings[i].texture == binding->texture)
            break;
    }
    if (i == g_surfaceBindingCount && g_surfaceBindingCount == g_surfaceBindingCapacity) {
        size_t capacity = g_surfaceBindingCapacity ? g_surfaceBindingCapacity * 2 : 16;
        struct surface_binding *larger = realloc(g_surfaceBindings, capacity * sizeof(*larger));
        if (larger) {
            g_surfaceBindings = larger;
            g_surfaceBindingCapacity = capacity;
        } else {
            ok = 0;
        }
    }
    if (ok) {
        CFRetain(binding->surface);
        if (i < g_surfaceBindingCount) {
            GLuint shadow = g_surfaceBindings[i].shadow;
            if (shadow != 0 && shadow != binding->shadow && !shadowInUse(binding->context, shadow, i))
                unusedShadow = shadow;
            reportUnflushedBinding(&g_surfaceBindings[i]);
            CFRelease(g_surfaceBindings[i].surface);
        } else {
            g_surfaceBindingCount++;
            atomic_fetch_add(&g_bindingCount, 1);
        }
        g_surfaceBindings[i] = *binding;
    }
    pthread_mutex_unlock(&g_surfaceBindingsMutex);
    if (unusedShadow != 0)
        host_glDeleteTextures(1, &unusedShadow);
    return ok;
}

static GLenum textureBindingQuery(GLenum target) {
    switch (target) {
    case GL_TEXTURE_2D: return GL_TEXTURE_BINDING_2D;
    case GL_TEXTURE_RECTANGLE: return GL_TEXTURE_BINDING_RECTANGLE;
    default: return 0;
    }
}

// The size of one pixel of client memory in format and type, or 0 if the
// combination is not one this implementation knows.
static size_t pixelSize(GLenum format, GLenum type) {
    switch (type) {
    case GL_UNSIGNED_INT_8_8_8_8:
    case GL_UNSIGNED_INT_8_8_8_8_REV:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
    case GL_UNSIGNED_INT_10_10_10_2:
        return 4;
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_4_4_4_4_REV:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_SHORT_1_5_5_5_REV:
    case GL_UNSIGNED_SHORT_8_8_APPLE:
    case GL_UNSIGNED_SHORT_8_8_REV_APPLE:
        return 2;
    }

    size_t component;
    switch (type) {
    case GL_UNSIGNED_BYTE: case GL_BYTE: component = 1; break;
    case GL_UNSIGNED_SHORT: case GL_SHORT: case GL_HALF_FLOAT: component = 2; break;
    case GL_UNSIGNED_INT: case GL_INT: case GL_FLOAT: component = 4; break;
    default: return 0;
    }

    switch (format) {
    case GL_RED: case GL_RED_INTEGER: case GL_ALPHA: case GL_LUMINANCE:
        return component;
    case GL_RG: case GL_RG_INTEGER: case GL_LUMINANCE_ALPHA:
        return 2 * component;
    case GL_RGB: case GL_BGR: case GL_RGB_INTEGER:
        return 3 * component;
    case GL_RGBA: case GL_BGRA: case GL_RGBA_INTEGER: case GL_BGRA_INTEGER:
        return 4 * component;
    default:
        return 0;
    }
}

// Pixel store state that a transfer between a surface plane and a texture
// sets for itself, restored afterwards so the application's state is kept.
struct pixel_store {
    GLint buffer, rowLength, skipRows, skipPixels, alignment;
};

static void savePixelStore(int pack, struct pixel_store *saved) {
    glGetIntegerv(pack ? GL_PIXEL_PACK_BUFFER_BINDING : GL_PIXEL_UNPACK_BUFFER_BINDING, &saved->buffer);
    glGetIntegerv(pack ? GL_PACK_ROW_LENGTH : GL_UNPACK_ROW_LENGTH, &saved->rowLength);
    glGetIntegerv(pack ? GL_PACK_SKIP_ROWS : GL_UNPACK_SKIP_ROWS, &saved->skipRows);
    glGetIntegerv(pack ? GL_PACK_SKIP_PIXELS : GL_UNPACK_SKIP_PIXELS, &saved->skipPixels);
    glGetIntegerv(pack ? GL_PACK_ALIGNMENT : GL_UNPACK_ALIGNMENT, &saved->alignment);
}

static void setPixelStore(int pack, const struct pixel_store *store) {
    glBindBuffer(pack ? GL_PIXEL_PACK_BUFFER : GL_PIXEL_UNPACK_BUFFER, (GLuint) store->buffer);
    glPixelStorei(pack ? GL_PACK_ROW_LENGTH : GL_UNPACK_ROW_LENGTH, store->rowLength);
    glPixelStorei(pack ? GL_PACK_SKIP_ROWS : GL_UNPACK_SKIP_ROWS, store->skipRows);
    glPixelStorei(pack ? GL_PACK_SKIP_PIXELS : GL_UNPACK_SKIP_PIXELS, store->skipPixels);
    glPixelStorei(pack ? GL_PACK_ALIGNMENT : GL_UNPACK_ALIGNMENT, store->alignment);
}

// Checks that width x height pixels of format and type fit in the plane, and
// returns the plane's address, its row stride in pixels, and the stride and
// pixel size in bytes. The row stride in pixels is 0 when the row stride in
// bytes is not a multiple of the pixel size (24-bit pixels in rows aligned to a
// power of two): GL's row length cannot express that, so such planes are
// transferred one row at a time.
static CGLError planeForTransfer(IOSurfaceRef surface, GLuint plane, GLsizei width, GLsizei height,
                                 GLenum format, GLenum type, void **pixels, GLint *rowLength,
                                 size_t *rowBytes, size_t *pixelBytes) {
    size_t bytesPerRow = IOSurfaceGetBytesPerRowOfPlane(surface, plane);
    size_t planeHeight = IOSurfaceGetHeightOfPlane(surface, plane);
    size_t size = pixelSize(format, type);
    if (size == 0)
        size = IOSurfaceGetBytesPerElementOfPlane(surface, plane);
    *pixels = IOSurfaceGetBaseAddressOfPlane(surface, plane);
    if (!*pixels || bytesPerRow == 0 || size == 0)
        return kCGLBadValue;
    if (width <= 0 || height <= 0 || (size_t) width > bytesPerRow / size || (size_t) height > planeHeight)
        return kCGLBadValue;
    if (bytesPerRow / size > INT32_MAX)
        return kCGLBadValue;
    *rowLength = bytesPerRow % size == 0 ? (GLint) (bytesPerRow / size) : 0;
    *rowBytes = bytesPerRow;
    *pixelBytes = size;
    return kCGLNoError;
}

// Importing surface planes as DMA-BUFs.

enum dmabuf_import {
    DMABUF_IMPORT_UNKNOWN = 0,
    DMABUF_IMPORT_UNSUPPORTED,
    DMABUF_IMPORT_SUPPORTED,
    // Also takes an explicit layout modifier
    // (EGL_EXT_image_dma_buf_import_modifiers).
    DMABUF_IMPORT_WITH_MODIFIERS,
};

static PFNEGLCREATEIMAGEKHRPROC p_eglCreateImageKHR;
static PFNEGLDESTROYIMAGEKHRPROC p_eglDestroyImageKHR;
// glEGLImageTargetTexture2DOES (OES_EGL_image), which Apple's GL headers
// do not declare.
typedef void (*cgl_image_target_texture_proc)(GLenum target, void *image);
static cgl_image_target_texture_proc p_glEGLImageTargetTexture2DOES;
// eglQueryDmaBufModifiersEXT (EGL_EXT_image_dma_buf_import_modifiers).
typedef EGLBoolean (*cgl_query_modifiers_proc)(EGLDisplay display, EGLint format, EGLint maxModifiers,
                                               uint64_t *modifiers, EGLBoolean *externalOnly,
                                               EGLint *modifierCount);
static cgl_query_modifiers_proc p_eglQueryDmaBufModifiersEXT;

// GPU copies between textures. Looked up rather than linked, as the host GL
// library need not export them.
typedef void (*cgl_copy_image_proc)(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX,
                                    GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget,
                                    GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ,
                                    GLsizei width, GLsizei height, GLsizei depth);
typedef void (*cgl_blit_framebuffer_proc)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                                          GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                                          GLbitfield mask, GLenum filter);
typedef const GLubyte *(*cgl_get_stringi_proc)(GLenum name, GLuint index);
static cgl_copy_image_proc p_glCopyImageSubData;
static cgl_blit_framebuffer_proc p_glBlitFramebuffer;
static cgl_get_stringi_proc p_glGetStringi;

static void load_image_functions(void) {
    p_glCopyImageSubData = (cgl_copy_image_proc) p_eglGetProcAddress("glCopyImageSubData");
    p_glBlitFramebuffer = (cgl_blit_framebuffer_proc) p_eglGetProcAddress("glBlitFramebuffer");
    p_glGetStringi = (cgl_get_stringi_proc) p_eglGetProcAddress("glGetStringi");
    p_eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC) p_eglGetProcAddress("eglCreateImageKHR");
    p_eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC) p_eglGetProcAddress("eglDestroyImageKHR");
    p_glEGLImageTargetTexture2DOES =
        (cgl_image_target_texture_proc) p_eglGetProcAddress("glEGLImageTargetTexture2DOES");
    p_eglQueryDmaBufModifiersEXT =
        (cgl_query_modifiers_proc) p_eglGetProcAddress("eglQueryDmaBufModifiersEXT");
}

static int has_extension(const char *extensions, const char *name) {
    size_t length = strlen(name);
    for (const char *found = extensions; (found = strstr(found, name)) != NULL; found += length) {
        if ((found == extensions || found[-1] == ' ') && (found[length] == ' ' || found[length] == '\0'))
            return 1;
    }
    return 0;
}


// OpenGL profiles: what a context reports about itself.
//
// A context asks for a profile through kCGLPFAOpenGLProfile. The host's
// context can offer more than that profile (a compatibility context is
// usually the host's newest version, and a core one the newest core
// version), so glGetString, glGetStringi and glGetIntegerv answer for the
// profile instead, with the version, shading language version and
// extensions a Mac reports for it:
//  - a legacy context (the default) is OpenGL 2.1, GLSL 1.20, and lists the
//    extensions of a 2.1 context through glGetString(GL_EXTENSIONS);
//  - a core context (3.2 or 4.1) reports at most 4.1, GLSL 4.10, and lists
//    its extensions through glGetStringi and GL_NUM_EXTENSIONS.
// Only what the host context supports is listed, and nothing is added to
// what the host version has: a host below the cap reports its own version.
// Functions stay resolvable whatever the profile says; only the reports
// change. GL_VENDOR and GL_RENDERER are the host's.
//
// The framework's own questions to the host driver (which copy method it
// has, which renderer it is) use the host_* functions, not these answers.

#ifndef GL_MAJOR_VERSION
#define GL_MAJOR_VERSION 0x821B
#endif
#ifndef GL_MINOR_VERSION
#define GL_MINOR_VERSION 0x821C
#endif
#ifndef GL_NUM_EXTENSIONS
#define GL_NUM_EXTENSIONS 0x821D
#endif

typedef const GLubyte *(*cgl_host_get_string_proc)(GLenum name);
typedef const GLubyte *(*cgl_host_get_stringi_proc)(GLenum name, GLuint index);
typedef void (*cgl_host_get_integerv_proc)(GLenum pname, GLint *params);
static cgl_host_get_string_proc p_hostGetString;
static cgl_host_get_stringi_proc p_hostGetStringi;
static cgl_host_get_integerv_proc p_hostGetIntegerv;

static void load_host_queries(void) {
    p_hostGetString = (cgl_host_get_string_proc) p_eglGetProcAddress("glGetString");
    p_hostGetStringi = (cgl_host_get_stringi_proc) p_eglGetProcAddress("glGetStringi");
    p_hostGetIntegerv = (cgl_host_get_integerv_proc) p_eglGetProcAddress("glGetIntegerv");
}

static void ensure_host_queries(void) {
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    if (native_egl_available())
        pthread_once(&once, load_host_queries);
}

static const GLubyte *host_glGetString(GLenum name) {
    ensure_host_queries();
    return p_hostGetString ? p_hostGetString(name) : NULL;
}

static const GLubyte *host_glGetStringi(GLenum name, GLuint index) {
    ensure_host_queries();
    return p_hostGetStringi ? p_hostGetStringi(name, index) : NULL;
}

static void host_glGetIntegerv(GLenum pname, GLint *params) {
    ensure_host_queries();
    if (p_hostGetIntegerv)
        p_hostGetIntegerv(pname, params);
}

static const char *const legacy_extensions[] = {
    "GL_ARB_color_buffer_float", "GL_ARB_depth_buffer_float", "GL_ARB_depth_clamp", "GL_ARB_depth_texture",
    "GL_ARB_draw_buffers", "GL_ARB_draw_elements_base_vertex", "GL_ARB_draw_instanced",
    "GL_ARB_fragment_program", "GL_ARB_fragment_program_shadow", "GL_ARB_fragment_shader",
    "GL_ARB_framebuffer_object", "GL_ARB_framebuffer_sRGB", "GL_ARB_half_float_pixel",
    "GL_ARB_half_float_vertex", "GL_ARB_imaging", "GL_ARB_instanced_arrays", "GL_ARB_multisample",
    "GL_ARB_multitexture", "GL_ARB_occlusion_query", "GL_ARB_pixel_buffer_object", "GL_ARB_point_parameters",
    "GL_ARB_point_sprite", "GL_ARB_provoking_vertex", "GL_ARB_seamless_cube_map", "GL_ARB_shader_objects",
    "GL_ARB_shader_texture_lod", "GL_ARB_shading_language_100", "GL_ARB_shadow", "GL_ARB_shadow_ambient",
    "GL_ARB_sync", "GL_ARB_texture_border_clamp", "GL_ARB_texture_compression",
    "GL_ARB_texture_compression_rgtc", "GL_ARB_texture_cube_map", "GL_ARB_texture_env_add",
    "GL_ARB_texture_env_combine", "GL_ARB_texture_env_crossbar", "GL_ARB_texture_env_dot3",
    "GL_ARB_texture_float", "GL_ARB_texture_mirrored_repeat", "GL_ARB_texture_non_power_of_two",
    "GL_ARB_texture_rectangle", "GL_ARB_texture_rg", "GL_ARB_transpose_matrix", "GL_ARB_vertex_array_bgra",
    "GL_ARB_vertex_blend", "GL_ARB_vertex_buffer_object", "GL_ARB_vertex_program", "GL_ARB_vertex_shader",
    "GL_ARB_window_pos", "GL_EXT_abgr", "GL_EXT_bgra", "GL_EXT_bindable_uniform", "GL_EXT_blend_color",
    "GL_EXT_blend_equation_separate", "GL_EXT_blend_func_separate", "GL_EXT_blend_minmax",
    "GL_EXT_blend_subtract", "GL_EXT_clip_volume_hint", "GL_EXT_debug_label", "GL_EXT_debug_marker",
    "GL_EXT_depth_bounds_test", "GL_EXT_draw_buffers2", "GL_EXT_draw_range_elements", "GL_EXT_fog_coord",
    "GL_EXT_framebuffer_blit", "GL_EXT_framebuffer_multisample", "GL_EXT_framebuffer_multisample_blit_scaled",
    "GL_EXT_framebuffer_object", "GL_EXT_framebuffer_sRGB", "GL_EXT_geometry_shader4",
    "GL_EXT_gpu_program_parameters", "GL_EXT_gpu_shader4", "GL_EXT_multi_draw_arrays",
    "GL_EXT_packed_depth_stencil", "GL_EXT_packed_float", "GL_EXT_provoking_vertex", "GL_EXT_rescale_normal",
    "GL_EXT_secondary_color", "GL_EXT_separate_specular_color", "GL_EXT_shadow_funcs",
    "GL_EXT_stencil_two_side", "GL_EXT_stencil_wrap", "GL_EXT_texture_array",
    "GL_EXT_texture_compression_dxt1", "GL_EXT_texture_compression_s3tc", "GL_EXT_texture_env_add",
    "GL_EXT_texture_filter_anisotropic", "GL_EXT_texture_integer", "GL_EXT_texture_lod_bias",
    "GL_EXT_texture_mirror_clamp", "GL_EXT_texture_rectangle", "GL_EXT_texture_shared_exponent",
    "GL_EXT_texture_sRGB", "GL_EXT_texture_sRGB_decode", "GL_EXT_timer_query", "GL_EXT_transform_feedback",
    "GL_EXT_vertex_array_bgra", "GL_APPLE_aux_depth_stencil", "GL_APPLE_client_storage",
    "GL_APPLE_element_array", "GL_APPLE_fence", "GL_APPLE_float_pixels", "GL_APPLE_flush_buffer_range",
    "GL_APPLE_flush_render", "GL_APPLE_packed_pixels", "GL_APPLE_pixel_buffer", "GL_APPLE_rgb_422",
    "GL_APPLE_row_bytes", "GL_APPLE_specular_vector", "GL_APPLE_texture_range", "GL_APPLE_transform_hint",
    "GL_APPLE_vertex_array_object", "GL_APPLE_vertex_array_range", "GL_APPLE_vertex_point_size",
    "GL_APPLE_vertex_program_evaluators", "GL_APPLE_ycbcr_422", "GL_ATI_separate_stencil",
    "GL_ATI_texture_compression_3dc", "GL_ATI_texture_env_combine3", "GL_ATI_texture_float",
    "GL_ATI_texture_mirror_once", "GL_IBM_rasterpos_clip", "GL_NV_blend_square", "GL_NV_conditional_render",
    "GL_NV_depth_clamp", "GL_NV_fog_distance", "GL_NV_light_max_exponent", "GL_NV_texgen_reflection",
    "GL_NV_texture_barrier", "GL_SGIS_generate_mipmap", "GL_SGIS_texture_edge_clamp", "GL_SGIS_texture_lod",
};

static const char *const core_extensions[] = {
    "GL_ARB_blend_func_extended", "GL_ARB_draw_buffers_blend", "GL_ARB_draw_indirect",
    "GL_ARB_ES2_compatibility", "GL_ARB_explicit_attrib_location", "GL_ARB_gpu_shader_fp64",
    "GL_ARB_gpu_shader5", "GL_ARB_instanced_arrays", "GL_ARB_internalformat_query", "GL_ARB_occlusion_query2",
    "GL_ARB_sample_shading", "GL_ARB_sampler_objects", "GL_ARB_separate_shader_objects",
    "GL_ARB_shader_bit_encoding", "GL_ARB_shader_subroutine", "GL_ARB_shading_language_include",
    "GL_ARB_tessellation_shader", "GL_ARB_texture_buffer_object_rgb32", "GL_ARB_texture_cube_map_array",
    "GL_ARB_texture_gather", "GL_ARB_texture_query_lod", "GL_ARB_texture_rgb10_a2ui",
    "GL_ARB_texture_storage", "GL_ARB_texture_swizzle", "GL_ARB_timer_query", "GL_ARB_transform_feedback2",
    "GL_ARB_transform_feedback3", "GL_ARB_vertex_attrib_64bit", "GL_ARB_vertex_type_2_10_10_10_rev",
    "GL_ARB_viewport_array", "GL_EXT_debug_label", "GL_EXT_debug_marker", "GL_EXT_depth_bounds_test",
    "GL_EXT_framebuffer_multisample_blit_scaled", "GL_EXT_texture_compression_s3tc",
    "GL_EXT_texture_filter_anisotropic", "GL_EXT_texture_mirror_clamp", "GL_EXT_texture_sRGB_decode",
    "GL_APPLE_client_storage", "GL_APPLE_container_object_shareable", "GL_APPLE_flush_render",
    "GL_APPLE_rgb_422", "GL_APPLE_row_bytes", "GL_APPLE_texture_range", "GL_ATI_texture_mirror_once",
    "GL_NV_texture_barrier",
};

struct cgl_profile_strings {
    int core;
    int major, minor;
    char version[160];
    char shading_language[16];
    // legacy: the extension list as one string
    char *legacy_list;
    // core: the extensions, pointing into core_extensions
    const char **core_list;
    GLint core_count;
};

static void freeProfileStrings(struct cgl_profile_strings *strings) {
    if (!strings)
        return;
    free(strings->legacy_list);
    free(strings->core_list);
    free(strings);
}

// The part of a host version string after the number: what follows
// "major.minor[.release] ", without a "(Core Profile)" kind of note.
static const char *version_vendor_text(const char *version) {
    const char *p = version;
    while ((*p >= '0' && *p <= '9') || *p == '.')
        p++;
    while (*p == ' ')
        p++;
    if (*p == '(') {
        const char *close = strchr(p, ')');
        p = close ? close + 1 : p + strlen(p);
        while (*p == ' ')
            p++;
    }
    return p;
}

static int host_list_has(const char *list, const char *name) {
    return list && has_extension(list, name);
}

// Works out what a context reports. Called with the context current (its
// host context answers the questions), from the thread that has it.
static struct cgl_profile_strings *makeProfileStrings(CGLContextObj context) {
    const char *hostVersion = (const char *) host_glGetString(GL_VERSION);
    int major = 0, minor = 0;
    if (!hostVersion || sscanf(hostVersion, "%d.%d", &major, &minor) != 2)
        return NULL;

    struct cgl_profile_strings *strings = calloc(1, sizeof(*strings));
    if (!strings)
        return NULL;
    strings->core = context->profile != 0;

    // QUIRK: a 3.2 core request reports the same version as a 4.1 core
    // request (the newest core version, capped at 4.1), not 3.2. Not
    // documented. Evidence: black-box observation on macOS 10.14.4 and
    // 26.7.3 (tier 2), probes in the PR's Provenance. Test:
    // cgl_profile_strings ("QUIRK test": 3.2 and 4.1 describe alike).
    int capMajor = strings->core ? 4 : 2, capMinor = 1;
    if (major > capMajor || (major == capMajor && minor > capMinor)) {
        major = capMajor;
        minor = capMinor;
    }
    strings->major = major;
    strings->minor = minor;
    const char *text = version_vendor_text(hostVersion);
    snprintf(strings->version, sizeof(strings->version), "%d.%d%s%s", major, minor, *text ? " " : "", text);

    // GLSL: 1.20 for legacy, 4.10 for core, or the host's number when it is lower.
    int glsl = strings->core ? 410 : 120;
    const char *hostGlsl = (const char *) host_glGetString(GL_SHADING_LANGUAGE_VERSION);
    int glslMajor = 0, glslMinor = 0;
    if (hostGlsl && sscanf(hostGlsl, "%d.%d", &glslMajor, &glslMinor) == 2 && glslMajor * 100 + glslMinor < glsl)
        glsl = glslMajor * 100 + glslMinor;
    snprintf(strings->shading_language, sizeof(strings->shading_language), "%d.%02d", glsl / 100, glsl % 100);

    if (!strings->core) {
        const char *hostList = (const char *) host_glGetString(GL_EXTENSIONS);
        size_t size = 1;
        for (size_t i = 0; i < sizeof(legacy_extensions) / sizeof(legacy_extensions[0]); i++)
            size += strlen(legacy_extensions[i]) + 1;
        strings->legacy_list = malloc(size);
        if (!strings->legacy_list) {
            freeProfileStrings(strings);
            return NULL;
        }
        char *out = strings->legacy_list;
        *out = '\0';
        for (size_t i = 0; i < sizeof(legacy_extensions) / sizeof(legacy_extensions[0]); i++) {
            if (!host_list_has(hostList, legacy_extensions[i]))
                continue;
            size_t length = strlen(legacy_extensions[i]);
            if (out != strings->legacy_list)
                *out++ = ' ';
            memcpy(out, legacy_extensions[i], length + 1);
            out += length;
        }
    } else {
        const size_t count = sizeof(core_extensions) / sizeof(core_extensions[0]);
        strings->core_list = calloc(count, sizeof(char *));
        if (!strings->core_list) {
            freeProfileStrings(strings);
            return NULL;
        }
        GLint hostCount = 0;
        host_glGetIntegerv(GL_NUM_EXTENSIONS, &hostCount);
        for (size_t i = 0; i < count; i++) {
            for (GLint j = 0; j < hostCount; j++) {
                const char *name = (const char *) host_glGetStringi(GL_EXTENSIONS, (GLuint) j);
                if (name && strcmp(name, core_extensions[i]) == 0) {
                    strings->core_list[strings->core_count++] = core_extensions[i];
                    break;
                }
            }
        }
    }
    return strings;
}

// The strings of the current context of this thread, or NULL when there is
// none (the host answers then, with an error or its own values).
static struct cgl_profile_strings *currentProfileStrings(void) {
    CGLContextObj context = CGLGetCurrentContext();
    if (!context)
        return NULL;
    pthread_mutex_lock(&context->profile_lock);
    if (!context->profile_strings)
        context->profile_strings = makeProfileStrings(context);
    struct cgl_profile_strings *strings = context->profile_strings;
    pthread_mutex_unlock(&context->profile_lock);
    return strings;
}

const GLubyte *override_glGetString(GLenum name) {
    if (name == GL_VERSION || name == GL_SHADING_LANGUAGE_VERSION || name == GL_EXTENSIONS) {
        struct cgl_profile_strings *strings = currentProfileStrings();
        if (strings) {
            if (name == GL_VERSION)
                return (const GLubyte *) strings->version;
            if (name == GL_SHADING_LANGUAGE_VERSION)
                return (const GLubyte *) strings->shading_language;
            // a core context has no extension string; the host raises GL_INVALID_ENUM
            if (!strings->core)
                return (const GLubyte *) strings->legacy_list;
        }
    }
    return host_glGetString(name);
}

const GLubyte *override_glGetStringi(GLenum name, GLuint index) {
    if (name == GL_EXTENSIONS) {
        struct cgl_profile_strings *strings = currentProfileStrings();
        if (strings && strings->core)
            return index < (GLuint) strings->core_count ? (const GLubyte *) strings->core_list[index] : host_glGetStringi(name, UINT_MAX);
        // OpenGL 2.1 has no indexed strings: nothing to return, and the host
        // is asked for an index no list has so that it raises an error.
        if (strings)
            return host_glGetStringi(name, UINT_MAX);
    }
    return host_glGetStringi(name, index);
}

void override_glGetIntegerv(GLenum pname, GLint *params) {
    if (pname == GL_MAJOR_VERSION || pname == GL_MINOR_VERSION || pname == GL_NUM_EXTENSIONS) {
        struct cgl_profile_strings *strings = currentProfileStrings();
        if (strings && strings->core) {
            *params = pname == GL_MAJOR_VERSION ? strings->major
                : pname == GL_MINOR_VERSION ? strings->minor : strings->core_count;
            return;
        }
        // These queries start with OpenGL 3.0; a 2.1 context raises
        // GL_INVALID_ENUM. The host would answer, so it is given a pname
        // that no version knows, which raises it.
        if (strings) {
            host_glGetIntegerv(0, params);
            return;
        }
    }
    host_glGetIntegerv(pname, params);
}

static enum dmabuf_import dmabuf_import(CGLContextObj context) {
    int support = atomic_load(&context->dmabuf_import);
    if (support != DMABUF_IMPORT_UNKNOWN)
        return (enum dmabuf_import) support;

    static pthread_once_t once = PTHREAD_ONCE_INIT;
    pthread_once(&once, load_image_functions);

    support = DMABUF_IMPORT_UNSUPPORTED;
    const char *extensions = p_eglQueryString(context->egl_display, EGL_EXTENSIONS);
    if (extensions && has_extension(extensions, "EGL_EXT_image_dma_buf_import")
        && p_eglCreateImageKHR && p_eglDestroyImageKHR && p_glEGLImageTargetTexture2DOES) {
        support = has_extension(extensions, "EGL_EXT_image_dma_buf_import_modifiers")
            ? DMABUF_IMPORT_WITH_MODIFIERS : DMABUF_IMPORT_SUPPORTED;
    }
    atomic_store(&context->dmabuf_import, support);
    return (enum dmabuf_import) support;
}

// The DRM format of a plane whose pixels are in format and type, as a
// texture of internal_format, or 0 if a texture of that format cannot be
// made from the plane's memory as it is. Only formats whose GL meaning does
// not depend on how the texture is stored are taken: luminance, alpha and
// sRGB textures, for instance, are copied.
static uint32_t drmFormat(GLenum internal_format, GLenum format, GLenum type) {
    int alpha;
    switch (internal_format) {
    case GL_RGBA: case GL_RGBA8: case GL_RGBA16F:
        alpha = 1;
        break;
    case GL_RGB: case GL_RGB8: case GL_RGB16F:
        alpha = 0;
        break;
    case GL_RED: case GL_R8:
        return format == GL_RED && type == GL_UNSIGNED_BYTE ? CGL_DRM_FORMAT_R8 : 0;
    case GL_RG: case GL_RG8:
        return format == GL_RG && type == GL_UNSIGNED_BYTE ? CGL_DRM_FORMAT_GR88 : 0;
    default:
        return 0;
    }

    int halfFloat = internal_format == GL_RGBA16F || internal_format == GL_RGB16F;
    if (halfFloat != (type == GL_HALF_FLOAT))
        return 0;

    switch (format) {
    case GL_BGRA:
        switch (type) {
        case GL_UNSIGNED_BYTE:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
            return alpha ? CGL_DRM_FORMAT_ARGB8888 : CGL_DRM_FORMAT_XRGB8888;
        case GL_UNSIGNED_INT_8_8_8_8:
            return alpha ? CGL_DRM_FORMAT_BGRA8888 : CGL_DRM_FORMAT_BGRX8888;
        }
        return 0;
    case GL_RGBA:
        switch (type) {
        case GL_UNSIGNED_BYTE:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
            return alpha ? CGL_DRM_FORMAT_ABGR8888 : CGL_DRM_FORMAT_XBGR8888;
        case GL_UNSIGNED_INT_8_8_8_8:
            return alpha ? CGL_DRM_FORMAT_RGBA8888 : CGL_DRM_FORMAT_RGBX8888;
        case GL_HALF_FLOAT:
            return alpha ? CGL_DRM_FORMAT_ABGR16161616F : CGL_DRM_FORMAT_XBGR16161616F;
        }
        return 0;
    default:
        return 0;
    }
}

// Whether the display can use an image of the plane memory's linear layout
// (a pitch-linear dma-buf, which is what IOSurface exports) as a 2D texture
// of DRM format fourcc that GL can sample, copy from and render into. Some
// drivers import such images but accept them only as external-only
// textures: binding one to GL_TEXTURE_2D fails, and importing one with a
// plane offset or through another route can crash the driver. The
// modifiers the display lists for the format say which case it is, so
// nothing needs to be tried. Answers are kept per display and format. With
// no modifier listing to ask, the import is left to be tried.
struct linear_import_answer {
    EGLDisplay display;
    uint32_t fourcc;
    int usable;
};
#define LINEAR_IMPORT_ANSWERS 32
static struct linear_import_answer g_linearImportAnswers[LINEAR_IMPORT_ANSWERS];
static size_t g_linearImportAnswerCount;
static pthread_mutex_t g_linearImportMutex = PTHREAD_MUTEX_INITIALIZER;

static int queryLinearImportUsable(EGLDisplay display, uint32_t fourcc) {
    if (!p_eglQueryDmaBufModifiersEXT)
        return 1;
    EGLint count = 0;
    if (!p_eglQueryDmaBufModifiersEXT(display, (EGLint) fourcc, 0, NULL, NULL, &count))
        return 1;
    if (count <= 0)
        return 1;
    uint64_t *modifiers = malloc((size_t) count * sizeof(*modifiers));
    EGLBoolean *externalOnly = malloc((size_t) count * sizeof(*externalOnly));
    int usable = 1;
    if (modifiers && externalOnly) {
        EGLint returned = 0;
        if (p_eglQueryDmaBufModifiersEXT(display, (EGLint) fourcc, count, modifiers, externalOnly, &returned)) {
            usable = 0;
            for (EGLint i = 0; i < returned; i++) {
                if (modifiers[i] == CGL_DRM_FORMAT_MOD_LINEAR && !externalOnly[i])
                    usable = 1;
            }
        }
    }
    free(modifiers);
    free(externalOnly);
    return usable;
}

static int linearImportUsable(CGLContextObj context, uint32_t fourcc) {
    EGLDisplay display = context->egl_display;
    pthread_mutex_lock(&g_linearImportMutex);
    for (size_t i = 0; i < g_linearImportAnswerCount; i++) {
        if (g_linearImportAnswers[i].display == display && g_linearImportAnswers[i].fourcc == fourcc) {
            int usable = g_linearImportAnswers[i].usable;
            pthread_mutex_unlock(&g_linearImportMutex);
            return usable;
        }
    }
    pthread_mutex_unlock(&g_linearImportMutex);

    int usable = queryLinearImportUsable(display, fourcc);
    pthread_mutex_lock(&g_linearImportMutex);
    if (g_linearImportAnswerCount < LINEAR_IMPORT_ANSWERS)
        g_linearImportAnswers[g_linearImportAnswerCount++] =
            (struct linear_import_answer) { display, fourcc, usable };
    pthread_mutex_unlock(&g_linearImportMutex);
    return usable;
}

// GL errors that were pending when an import attempt began. The attempt
// may fail in the driver and raise errors of its own, which belong to no
// application call; they are cleared afterwards and the earlier ones are
// raised again, so that glGetError shows the application exactly its own.
// Only the error codes that a harmless call can raise again are kept
// (the others are rare enough that dropping them is the lesser harm).
struct gl_errors {
    unsigned enumError : 1, valueError : 1, operationError : 1;
};

static void saveGLErrors(struct gl_errors *saved) {
    memset(saved, 0, sizeof(*saved));
    // Each code has its own flag, so one call returns at most one per code.
    for (int i = 0; i < 16; i++) {
        GLenum error = glGetError();
        if (error == GL_NO_ERROR)
            break;
        if (error == GL_INVALID_ENUM)
            saved->enumError = 1;
        else if (error == GL_INVALID_VALUE)
            saved->valueError = 1;
        else if (error == GL_INVALID_OPERATION)
            saved->operationError = 1;
    }
}

static void restoreGLErrors(const struct gl_errors *saved) {
    for (int i = 0; i < 16 && glGetError() != GL_NO_ERROR; i++)
        ;
    // None of these calls changes state when it fails. An invalid uniform
    // location other than -1 is an error with or without a current program.
    if (saved->enumError)
        glDisable(0xffff);
    if (saved->valueError)
        glGenTextures(-1, NULL);
    if (saved->operationError)
        glUniform1i(-2, 0);
}

// Makes an EGL image of width x height pixels of the plane's memory in DRM
// format fourcc, or returns EGL_NO_IMAGE_KHR.
static EGLImageKHR createPlaneImage(CGLContextObj context, uint32_t fourcc, GLsizei width, GLsizei height,
                                   IOSurfaceRef surface, GLuint plane) {
    enum dmabuf_import support = dmabuf_import(context);
    if (support == DMABUF_IMPORT_UNSUPPORTED)
        return EGL_NO_IMAGE_KHR;
    if (!linearImportUsable(context, fourcc))
        return EGL_NO_IMAGE_KHR;

    _IOSurfaceDMABuf buffer = { .version = _IOSURFACE_DMABUF_VERSION };
    if (!_IOSurfaceGetDMABuf(surface, plane, &buffer))
        return EGL_NO_IMAGE_KHR;
    if (buffer.offset > INT32_MAX || buffer.bytesPerRow > INT32_MAX)
        return EGL_NO_IMAGE_KHR;

    EGLint attributes[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_LINUX_DRM_FOURCC_EXT, (EGLint) fourcc,
        EGL_DMA_BUF_PLANE0_FD_EXT, buffer.fd,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, (EGLint) buffer.offset,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, (EGLint) buffer.bytesPerRow,
        // Without the modifier extension, the driver assumes a linear
        // layout for memory that has no layout of its own.
        EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, (EGLint) (CGL_DRM_FORMAT_MOD_LINEAR & 0xffffffff),
        EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT, (EGLint) (CGL_DRM_FORMAT_MOD_LINEAR >> 32),
        EGL_NONE,
    };
    if (support != DMABUF_IMPORT_WITH_MODIFIERS)
        attributes[12] = EGL_NONE;

    return p_eglCreateImageKHR(context->egl_display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attributes);
}

#define CGL_MAX_NOTED_ATTACHMENTS 16

// Attaches the texture again where the current context's draw or read
// framebuffer has it as a color attachment, after its storage has been
// replaced with an EGLImage or taken away from one. The framebuffer refers
// to the texture object, so rendering should reach the new storage anyway,
// but a host driver was seen to keep rendering into the old one until the
// texture was attached again.
static void reattachTexture(CGLContextObj context, GLenum target, GLuint texture) {
    static const GLenum framebufferTargets[2] = { GL_DRAW_FRAMEBUFFER, GL_READ_FRAMEBUFFER };
    static const GLenum bindings[2] = { GL_DRAW_FRAMEBUFFER_BINDING, GL_READ_FRAMEBUFFER_BINDING };
    GLint framebuffers[2] = { 0, 0 };
    glGetIntegerv(bindings[0], &framebuffers[0]);
    glGetIntegerv(bindings[1], &framebuffers[1]);
    if (context->max_color_attachments == 0) {
        GLint max = 0;
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max);
        context->max_color_attachments = max < 1 ? 1 : max > CGL_MAX_NOTED_ATTACHMENTS ? CGL_MAX_NOTED_ATTACHMENTS : max;
    }
    for (int f = 0; f < 2; f++) {
        if (framebuffers[f] == 0 || (f == 1 && framebuffers[1] == framebuffers[0]))
            continue;
        for (GLint i = 0; i < context->max_color_attachments; i++) {
            GLint type = GL_NONE, name = 0, level = 0;
            host_glGetFramebufferAttachmentParameteriv(framebufferTargets[f], GL_COLOR_ATTACHMENT0 + i,
                                                       GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
            if (type != GL_TEXTURE)
                continue;
            host_glGetFramebufferAttachmentParameteriv(framebufferTargets[f], GL_COLOR_ATTACHMENT0 + i,
                                                       GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &name);
            if ((GLuint) name != texture)
                continue;
            host_glGetFramebufferAttachmentParameteriv(framebufferTargets[f], GL_COLOR_ATTACHMENT0 + i,
                                                       GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL, &level);
            glFramebufferTexture2D(framebufferTargets[f], GL_COLOR_ATTACHMENT0 + i, target, texture, level);
        }
    }
}

// Makes the 2D texture bound in the current context use image, which it
// destroys. Returns whether the driver took it; if not, the texture may
// have been emptied.
static int importImage(CGLContextObj context, GLenum target, GLenum internal_format,
                       GLsizei width, GLsizei height, GLenum format, GLenum type, EGLImageKHR image) {
    // The texture has the image's size afterwards only if the driver took
    // the image; it is emptied first so that its old size does not count.
    // Errors of the attempt are not the application's.
    struct gl_errors saved;
    saveGLErrors(&saved);
    host_glTexImage2D(target, 0, (GLint) internal_format, 0, 0, 0, format, type, NULL);
    p_glEGLImageTargetTexture2DOES(target, image);
    // The texture keeps the memory after the image is gone.
    p_eglDestroyImageKHR(context->egl_display, image);

    GLint importedWidth = 0, importedHeight = 0;
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &importedWidth);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &importedHeight);
    restoreGLErrors(&saved);
    return importedWidth == width && importedHeight == height;
}

// Makes the texture bound to target in the current context use the plane's
// memory. Returns 0, leaving the texture as it was, if the context, the
// surface, the target or the format do not allow that.
static int importSurfacePlane(CGLContextObj context, GLenum target, GLenum internal_format,
                              GLsizei width, GLsizei height, GLenum format, GLenum type,
                              IOSurfaceRef surface, GLuint plane) {
    // OES_EGL_image defines the import for 2D textures only.
    if (target != GL_TEXTURE_2D)
        return 0;
    uint32_t fourcc = drmFormat(internal_format, format, type);
    if (fourcc == 0)
        return 0;
    EGLImageKHR image = createPlaneImage(context, fourcc, width, height, surface, plane);
    if (image == EGL_NO_IMAGE_KHR)
        return 0;
    return importImage(context, target, internal_format, width, height, format, type, image);
}

// Makes context current on this thread for the duration of a call, with the
// thread holding its host binding: the framework's own calls into the host
// driver (the ones that do not go through the gl* exports) need it, and a
// context that is current here may have been parked, or given up after the
// last call (see gl_acquire_binding).
struct context_scope {
    CGLContextObj previous;
    // The binding was taken for this scope only (see gl_acquire_binding).
    int transient;
};

//
// A thread that has no current context or another one is given the context by
// making it current (waiting for its holder, see setCurrentContext). A thread
// whose current context it is only gets the binding back; may_wait is 0 where
// that is not the thread's own request (the callbacks of IOSurface, which run
// on whatever thread locks a surface, and the refresh when a context is made
// current or locked): it must not wait for another thread's binding, and the
// work is left undone when it cannot have it.
static CGLError enterContext(CGLContextObj context, struct context_scope *scope, int may_wait) {
    scope->previous = CGLGetCurrentContext();
    scope->transient = 0;
    if (scope->previous != context)
        return setCurrentContext(context);
    if (GL_BOUND_HERE())
        return kCGLNoError;
    int acquired = acquireBinding(may_wait);
    if (acquired == 0)
        return kCGLBadContext;
    scope->transient = acquired == 2;
    return kCGLNoError;
}

static void leaveContext(CGLContextObj context, const struct context_scope *scope) {
    if (scope->previous != context) {
        // The thread's own context stays lazily current if it was.
        if (scope->previous == NULL || !setCurrentContextLazily(scope->previous))
            setCurrentContext(scope->previous);
    } else if (scope->transient) {
        gl_release_transient();
    }
}

// Rectangle textures kept in step with a surface through a shadow.

enum gpu_copy {
    GPU_COPY_UNKNOWN = 0,
    GPU_COPY_NONE,
    // glCopyImageSubData (OpenGL 4.3, ARB_copy_image).
    GPU_COPY_IMAGE,
    // glBlitFramebuffer (OpenGL 3.0, EXT_framebuffer_blit).
    GPU_COPY_BLIT,
};

// Whether the current context has an extension, asked in the way its
// version allows, so that no GL error is raised.
static int has_gl_extension(int major, const char *name) {
    if (major >= 3) {
        if (!p_glGetStringi)
            return 0;
        GLint count = 0;
        host_glGetIntegerv(GL_NUM_EXTENSIONS, &count);
        for (GLint i = 0; i < count; i++) {
            const GLubyte *extension = p_glGetStringi(GL_EXTENSIONS, (GLuint) i);
            if (extension && strcmp((const char *) extension, name) == 0)
                return 1;
        }
        return 0;
    }
    const GLubyte *extensions = host_glGetString(GL_EXTENSIONS);
    return extensions && has_extension((const char *) extensions, name);
}

// Whether the EGL device behind the display is a software renderer
// (EGL_EXT_device_query, EGL_MESA_device_software).
static int softwareEGLDevice(EGLDisplay display) {
    const char *client = p_eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (!client || !has_extension(client, "EGL_EXT_device_query"))
        return 0;
    typedef EGLBoolean (*query_display_attrib_proc)(EGLDisplay display, EGLint attribute, EGLAttrib *value);
    typedef const char *(*query_device_string_proc)(void *device, EGLint name);
    query_display_attrib_proc queryDisplayAttrib =
        (query_display_attrib_proc) p_eglGetProcAddress("eglQueryDisplayAttribEXT");
    query_device_string_proc queryDeviceString =
        (query_device_string_proc) p_eglGetProcAddress("eglQueryDeviceStringEXT");
    EGLAttrib device = 0;
    // EGL_DEVICE_EXT
    if (!queryDisplayAttrib || !queryDeviceString || !queryDisplayAttrib(display, 0x322C, &device) || !device)
        return 0;
    const char *extensions = queryDeviceString((void *) device, EGL_EXTENSIONS);
    return extensions && has_extension(extensions, "EGL_MESA_device_software");
}

// Whether the current context renders on the CPU. There a "GPU" copy is a
// CPU copy through the imported memory, which costs more than the plain
// CPU copy (measured on llvmpipe), so rectangle textures keep the latter.
static int softwareRenderer(CGLContextObj context) {
    const GLubyte *renderer = host_glGetString(GL_RENDERER);
    static const char *const names[] = { "llvmpipe", "lavapipe", "softpipe", "swrast", "Software Rasterizer" };
    for (size_t i = 0; renderer && i < sizeof(names) / sizeof(names[0]); i++) {
        if (strstr((const char *) renderer, names[i]))
            return 1;
    }
    return softwareEGLDevice(context->egl_display);
}

// How the current context copies between textures for rectangle textures
// kept in step with a surface; GPU_COPY_NONE on software renderers. Found
// out once per context.
static enum gpu_copy gpu_copy(CGLContextObj context) {
    if (context->gpu_copy != GPU_COPY_UNKNOWN)
        return (enum gpu_copy) context->gpu_copy;
    if (softwareRenderer(context)) {
        context->gpu_copy = GPU_COPY_NONE;
        return GPU_COPY_NONE;
    }

    int major = 0, minor = 0;
    const GLubyte *version = host_glGetString(GL_VERSION);
    if (version)
        sscanf((const char *) version, "%d.%d", &major, &minor);

    enum gpu_copy method = GPU_COPY_NONE;
    if (p_glCopyImageSubData && (major > 4 || (major == 4 && minor >= 3) || has_gl_extension(major, "GL_ARB_copy_image")))
        method = GPU_COPY_IMAGE;
    else if (p_glBlitFramebuffer && (major >= 3 || has_gl_extension(major, "GL_EXT_framebuffer_blit")))
        method = GPU_COPY_BLIT;
    context->gpu_copy = method;
    return method;
}

// Copies width x height texels of level 0 from one texture to another in
// the current context, on the GPU. The textures have formats of the same
// size. With convert, the copy goes by colour components rather than by
// bits (a blit), for textures whose formats order the components
// differently, as a GPU buffer may to the driver. Returns 0 if the context
// has no way to do it.
static int gpuCopy(CGLContextObj context, GLuint source, GLenum sourceTarget, GLuint destination,
                             GLenum destinationTarget, GLsizei width, GLsizei height, int convert) {
    enum gpu_copy method = gpu_copy(context);
    if (convert && method != GPU_COPY_NONE)
        method = p_glBlitFramebuffer ? GPU_COPY_BLIT : GPU_COPY_NONE;
    switch (method) {
    case GPU_COPY_IMAGE:
        p_glCopyImageSubData(source, sourceTarget, 0, 0, 0, 0, destination, destinationTarget, 0, 0, 0, 0,
                             width, height, 1);
        return 1;
    case GPU_COPY_BLIT:
        break;
    default:
        return 0;
    }

    // A blit goes through framebuffers and is subject to the scissor test
    // and sRGB conversion, so the application's state is put aside.
    if (context->copy_framebuffers[0] == 0)
        glGenFramebuffers(2, context->copy_framebuffers);
    GLint readFramebuffer = 0, drawFramebuffer = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFramebuffer);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFramebuffer);
    GLboolean scissor = glIsEnabled(GL_SCISSOR_TEST);
    GLboolean srgb = glIsEnabled(GL_FRAMEBUFFER_SRGB);

    glDisable(GL_SCISSOR_TEST);
    if (srgb)
        glDisable(GL_FRAMEBUFFER_SRGB);
    host_glBindFramebuffer(GL_READ_FRAMEBUFFER, context->copy_framebuffers[0]);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sourceTarget, source, 0);
    host_glBindFramebuffer(GL_DRAW_FRAMEBUFFER, context->copy_framebuffers[1]);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, destinationTarget, destination, 0);
    p_glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    // Detached again, so that the framebuffers keep no texture alive.
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sourceTarget, 0, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, destinationTarget, 0, 0);

    host_glBindFramebuffer(GL_READ_FRAMEBUFFER, (GLuint) readFramebuffer);
    host_glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint) drawFramebuffer);
    if (scissor)
        glEnable(GL_SCISSOR_TEST);
    if (srgb)
        glEnable(GL_FRAMEBUFFER_SRGB);
    return 1;
}

// GPU buffers.
//
// Some drivers can sample a DMA-BUF in the linear layout of a surface's
// memory but not render into it (it is external only to them; see
// linearImportUsable). Where such a driver renders into a surface, the
// surface gets a GPU buffer in a layout the driver renders into, from
// IOSurface (_IOSurfaceGetGPUBuffer in IOSurfacePriv.h), which keeps it in
// step with the memory. That happens the first time rendering into a
// texture specified from the surface is written back (copyBindingToSurface):
// the texture's rendering is copied into the new buffer on the GPU, and the
// texture itself is made to use the buffer, a 2D texture by import and a
// rectangle texture through a shadow imported from it. Later textures
// specified from the surface in this process use the buffer the same way.
// Completed rendering into the buffer is reported at the same points as
// rendering into imported memory, after the context has finished it, and
// IOSurface then copies it into the memory. Drivers that render into the
// linear layout never get here: their textures use the memory itself.
//
// DMA-BUFs (the surface's memory or its GPU buffer) are only ever given to
// the driver through EGL_EXT_image_dma_buf_import, always at offset 0 for
// GPU buffers, and never through glImportMemoryFdEXT: some drivers take
// only their own memory objects there and crash on a DMA-BUF.

// EGL_EXT_device_query and EGL_EXT_device_drm_render_node.
#define CGL_EGL_DEVICE_EXT 0x322C
#define CGL_EGL_DRM_RENDER_NODE_FILE_EXT 0x3377
#define CGL_MAX_MODIFIERS 64

struct render_node_answer {
    EGLDisplay display;
    char path[64];
};
#define RENDER_NODE_ANSWERS 8
static struct render_node_answer g_renderNodeAnswers[RENDER_NODE_ANSWERS];
static size_t g_renderNodeAnswerCount;
static pthread_mutex_t g_renderNodeMutex = PTHREAD_MUTEX_INITIALIZER;

static void queryRenderNode(EGLDisplay display, char *path, size_t size) {
    path[0] = '\0';
    const char *client = p_eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (!client || !has_extension(client, "EGL_EXT_device_query"))
        return;
    typedef EGLBoolean (*query_display_attrib_proc)(EGLDisplay display, EGLint attribute, EGLAttrib *value);
    typedef const char *(*query_device_string_proc)(void *device, EGLint name);
    query_display_attrib_proc queryDisplayAttrib =
        (query_display_attrib_proc) p_eglGetProcAddress("eglQueryDisplayAttribEXT");
    query_device_string_proc queryDeviceString =
        (query_device_string_proc) p_eglGetProcAddress("eglQueryDeviceStringEXT");
    EGLAttrib device = 0;
    if (!queryDisplayAttrib || !queryDeviceString || !queryDisplayAttrib(display, CGL_EGL_DEVICE_EXT, &device)
        || !device)
        return;
    const char *extensions = queryDeviceString((void *) device, EGL_EXTENSIONS);
    if (!extensions || !has_extension(extensions, "EGL_EXT_device_drm_render_node"))
        return;
    const char *node = queryDeviceString((void *) device, CGL_EGL_DRM_RENDER_NODE_FILE_EXT);
    if (node && strlen(node) < size)
        strcpy(path, node);
}

// The host path of the DRM render node of the context's display, or NULL.
// Answers are kept per display.
static const char *renderNode(CGLContextObj context) {
    EGLDisplay display = context->egl_display;
    const char *path = NULL;
    pthread_mutex_lock(&g_renderNodeMutex);
    for (size_t i = 0; i < g_renderNodeAnswerCount && path == NULL; i++) {
        if (g_renderNodeAnswers[i].display == display)
            path = g_renderNodeAnswers[i].path;
    }
    if (path == NULL && g_renderNodeAnswerCount < RENDER_NODE_ANSWERS) {
        struct render_node_answer *answer = &g_renderNodeAnswers[g_renderNodeAnswerCount++];
        answer->display = display;
        queryRenderNode(display, answer->path, sizeof(answer->path));
        path = answer->path;
    }
    pthread_mutex_unlock(&g_renderNodeMutex);
    return path && path[0] ? path : NULL;
}

// The layout modifiers that the display lists for fourcc as usable for
// rendering, the linear layout left out. Returns how many there are.
static size_t renderModifiers(EGLDisplay display, uint32_t fourcc, uint64_t *modifiers, size_t max) {
    if (!p_eglQueryDmaBufModifiersEXT)
        return 0;
    uint64_t listed[CGL_MAX_MODIFIERS];
    EGLBoolean externalOnly[CGL_MAX_MODIFIERS];
    EGLint count = 0;
    if (!p_eglQueryDmaBufModifiersEXT(display, (EGLint) fourcc, CGL_MAX_MODIFIERS, listed, externalOnly, &count))
        return 0;
    size_t usable = 0;
    for (EGLint i = 0; i < count && i < CGL_MAX_MODIFIERS && usable < max; i++) {
        if (!externalOnly[i] && listed[i] != CGL_DRM_FORMAT_MOD_LINEAR)
            modifiers[usable++] = listed[i];
    }
    return usable;
}

// Whether the current context renders into surfaces of DRM format fourcc
// through GPU buffers: its driver cannot render into the linear layout, can
// import other layouts, and can copy between textures on the GPU.
static int gpuBufferUsable(CGLContextObj context, uint32_t fourcc) {
    if (fourcc != CGL_DRM_FORMAT_ARGB8888 && fourcc != CGL_DRM_FORMAT_ABGR8888)
        return 0;
    if (dmabuf_import(context) != DMABUF_IMPORT_WITH_MODIFIERS || linearImportUsable(context, fourcc))
        return 0;
    // Copies to and from the buffer are blits (gpuCopy with convert).
    return gpu_copy(context) != GPU_COPY_NONE && p_glBlitFramebuffer != NULL;
}

// Fills in *buffer with the GPU buffer of the plane, made if create is set
// and the surface has none. Returns 0 if the context does not use GPU
// buffers for the format, or the surface has or can get no buffer of the
// given size that the context can render into.
static int getGPUBuffer(CGLContextObj context, IOSurfaceRef surface, GLuint plane, uint32_t fourcc,
                        GLsizei width, GLsizei height, int create, _IOSurfaceGPUBuffer *buffer) {
    if (!gpuBufferUsable(context, fourcc))
        return 0;
    const char *node = renderNode(context);
    if (node == NULL)
        return 0;
    uint64_t modifiers[CGL_MAX_MODIFIERS];
    size_t count = renderModifiers(context->egl_display, fourcc, modifiers, CGL_MAX_MODIFIERS);
    if (count == 0)
        return 0;
    *buffer = (_IOSurfaceGPUBuffer) { .version = _IOSURFACE_GPU_BUFFER_VERSION };
    if (!_IOSurfaceGetGPUBuffer(surface, plane, node, fourcc, modifiers, count,
                                create ? _IOSURFACE_GPU_BUFFER_CREATE : 0, buffer))
        return 0;
    if (buffer->width != (uint32_t) width || buffer->height != (uint32_t) height || buffer->offset != 0
        || buffer->stride > INT32_MAX)
        return 0;
    for (size_t i = 0; i < count; i++) {
        if (modifiers[i] == buffer->modifier)
            return 1;
    }
    return 0;
}

static EGLImageKHR createGPUBufferImage(CGLContextObj context, const _IOSurfaceGPUBuffer *buffer) {
    EGLint attributes[] = {
        EGL_WIDTH, (EGLint) buffer->width,
        EGL_HEIGHT, (EGLint) buffer->height,
        EGL_LINUX_DRM_FOURCC_EXT, (EGLint) buffer->fourcc,
        EGL_DMA_BUF_PLANE0_FD_EXT, buffer->fd,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, (EGLint) buffer->stride,
        EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, (EGLint) (buffer->modifier & 0xffffffff),
        EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT, (EGLint) (buffer->modifier >> 32),
        EGL_NONE,
    };
    return p_eglCreateImageKHR(context->egl_display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attributes);
}

// Makes the 2D texture bound in the current context use the GPU buffer.
static int importGPUBuffer(CGLContextObj context, GLenum internal_format, GLsizei width, GLsizei height,
                           GLenum format, GLenum type, const _IOSurfaceGPUBuffer *buffer) {
    EGLImageKHR image = createGPUBufferImage(context, buffer);
    if (image == EGL_NO_IMAGE_KHR)
        return 0;
    return importImage(context, GL_TEXTURE_2D, internal_format, width, height, format, type, image);
}

// The shadow for a rectangle texture of the plane: a binding's shadow of
// the same plane, format and size in the context, or a new 2D texture
// imported from the plane's memory, or from its GPU buffer if buffer is not
// NULL. Returns 0 if there is none.
static GLuint shadowTexture(CGLContextObj context, IOSurfaceRef surface, GLuint plane, uint32_t fourcc,
                            GLsizei width, GLsizei height, const _IOSurfaceGPUBuffer *buffer) {
    GLuint shadow = 0;
    pthread_mutex_lock(&g_surfaceBindingsMutex);
    for (size_t i = 0; i < g_surfaceBindingCount && shadow == 0; i++) {
        const struct surface_binding *binding = &g_surfaceBindings[i];
        if (binding->context == context && binding->shadow != 0 && binding->surface == surface
            && binding->plane == plane && binding->shadowFormat == fourcc
            && binding->width == width && binding->height == height
            && binding->gpuBuffer == (buffer != NULL))
            shadow = binding->shadow;
    }
    pthread_mutex_unlock(&g_surfaceBindingsMutex);
    if (shadow != 0)
        return shadow;

    EGLImageKHR image = buffer ? createGPUBufferImage(context, buffer)
                               : createPlaneImage(context, fourcc, width, height, surface, plane);
    if (image == EGL_NO_IMAGE_KHR)
        return 0;
    struct gl_errors saved;
    saveGLErrors(&saved);
    GLint previous = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous);
    glGenTextures(1, &shadow);
    glBindTexture(GL_TEXTURE_2D, shadow);
    p_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
    p_eglDestroyImageKHR(context->egl_display, image);
    // Complete with one level, as copies need.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GLint importedWidth = 0, importedHeight = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &importedWidth);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &importedHeight);
    glBindTexture(GL_TEXTURE_2D, (GLuint) previous);
    if (importedWidth != width || importedHeight != height) {
        host_glDeleteTextures(1, &shadow);
        shadow = 0;
    }
    restoreGLErrors(&saved);
    return shadow;
}

// Gives the rectangle texture bound to target the plane's contents through
// a shadow. Returns the shadow, or 0, leaving the texture to the CPU copy,
// if the format, the driver or the context do not allow it.
static GLuint copySurfacePlaneToRectangle(CGLContextObj context, GLenum target, GLint texture,
                                          GLenum internal_format, GLsizei width, GLsizei height,
                                          GLenum format, GLenum type, IOSurfaceRef surface,
                                          GLuint plane, const _IOSurfaceGPUBuffer *buffer,
                                          uint32_t *shadowFormat) {
    if (target != GL_TEXTURE_RECTANGLE || texture == 0)
        return 0;
    // Formats without alpha are left out: the copy would carry the
    // padding byte of the memory into the texture's alpha.
    uint32_t fourcc = drmFormat(internal_format, format, type);
    if (fourcc == 0 || fourcc == CGL_DRM_FORMAT_XRGB8888 || fourcc == CGL_DRM_FORMAT_XBGR8888
        || fourcc == CGL_DRM_FORMAT_BGRX8888 || fourcc == CGL_DRM_FORMAT_RGBX8888
        || fourcc == CGL_DRM_FORMAT_XBGR16161616F)
        return 0;
    if (dmabuf_import(context) == DMABUF_IMPORT_UNSUPPORTED || gpu_copy(context) == GPU_COPY_NONE)
        return 0;
    GLuint shadow = shadowTexture(context, surface, plane, fourcc, width, height, buffer);
    if (shadow == 0)
        return 0;

    GLint allocatedWidth = 0, allocatedHeight = 0, allocatedFormat = 0;
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &allocatedWidth);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &allocatedHeight);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_INTERNAL_FORMAT, &allocatedFormat);
    if (allocatedWidth != width || allocatedHeight != height || allocatedFormat != (GLint) internal_format) {
        struct pixel_store saved, none = { 0, 0, 0, 0, 4 };
        savePixelStore(0, &saved);
        setPixelStore(0, &none);
        host_glTexImage2D(target, 0, (GLint) internal_format, width, height, 0, format, type, NULL);
        setPixelStore(0, &saved);
    }
    gpuCopy(context, shadow, GL_TEXTURE_2D, (GLuint) texture, target, width, height, buffer != NULL);
    *shadowFormat = fourcc;
    return shadow;
}

// Whether the rectangle texture of binding still has the size and format
// it was specified with, so that a copy from or to its shadow fits. The
// application may have respecified it since.
static int rectangleMatchesShadow(const struct surface_binding *binding) {
    GLint previous = 0, width = 0, height = 0, internalFormat = 0;
    glGetIntegerv(textureBindingQuery(binding->target), &previous);
    glBindTexture(binding->target, binding->texture);
    glGetTexLevelParameteriv(binding->target, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(binding->target, 0, GL_TEXTURE_HEIGHT, &height);
    glGetTexLevelParameteriv(binding->target, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
    glBindTexture(binding->target, (GLuint) previous);
    return width == binding->width && height == binding->height
        && internalFormat == (GLint) binding->internalFormat;
}

static void refreshRectangleTextures(CGLContextObj context) {
    for (;;) {
        // One binding at a time, as the copy runs without the lock.
        struct surface_binding binding = { 0 };
        uint32_t seed = 0;
        pthread_mutex_lock(&g_surfaceBindingsMutex);
        for (size_t i = 0; i < g_surfaceBindingCount; i++) {
            struct surface_binding *candidate = &g_surfaceBindings[i];
            if (candidate->context != context || (candidate->shadow == 0 && !candidate->gpuBuffer))
                continue;
            seed = IOSurfaceGetSeed(candidate->surface);
            if (seed != candidate->seed) {
                candidate->seed = seed;
                binding = *candidate;
                break;
            }
        }
        pthread_mutex_unlock(&g_surfaceBindingsMutex);
        if (binding.context == NULL)
            return;
        // A GPU buffer gets what the CPU wrote in another process first.
        if (binding.gpuBuffer)
            _IOSurfaceGPUWillUse(binding.surface);
        // The copy goes to the host driver directly, so the thread must hold
        // the context's binding while it runs (a thread that only made the
        // context current, or locked it, may not yet).
        struct context_scope scope;
        if (binding.shadow != 0 && enterContext(context, &scope, 0) == kCGLNoError) {
            if (glIsTexture(binding.texture) && rectangleMatchesShadow(&binding))
                gpuCopy(context, binding.shadow, GL_TEXTURE_2D, binding.texture, binding.target,
                    binding.width, binding.height, binding.gpuBuffer);
            leaveContext(context, &scope);
        }
    }
}

static void writeBackTexture(CGLContextObj context, GLuint texture);

CGLError CGLTexImageIOSurface2D(CGLContextObj ctx, GLenum target, GLenum internal_format,
                                GLsizei width, GLsizei height, GLenum format, GLenum type,
                                IOSurfaceRef ioSurface, GLuint plane) {
    if (!ctx)
        return kCGLBadContext;
    if (!ioSurface)
        return kCGLBadAddress;
    GLenum bindingQuery = textureBindingQuery(target);
    if (!bindingQuery)
        return kCGLBadEnumeration;

    void *pixels;
    GLint rowLength;
    size_t rowBytes, pixelBytes;
    CGLError error = planeForTransfer(ioSurface, plane, width, height, format, type, &pixels, &rowLength,
                                      &rowBytes, &pixelBytes);
    if (error != kCGLNoError)
        return error;

    struct context_scope scope;
    error = enterContext(ctx, &scope, 1);
    if (error != kCGLNoError)
        return error;

    // GL errors from the upload are left for the application to query, as
    // are errors that were pending before this call.
    GLint texture = 0;
    glGetIntegerv(bindingQuery, &texture);

    // Rendering into the texture since its last copy belongs to the surface
    // it was specified from before.
    if (texture != 0)
        writeBackTexture(ctx, (GLuint) texture);
    // A texture that used another surface's memory must have finished
    // rendering into it before it uses this one's: nothing else waits for
    // that rendering once the binding is gone.
    struct surface_binding previousBinding;
    if (texture != 0 && findSurfaceBinding(ctx, (GLuint) texture, &previousBinding) && previousBinding.imported
        && previousBinding.dirty && (previousBinding.surface != ioSurface || previousBinding.plane != plane))
        host_glFinish();

    // A texture that already uses this plane's memory in the same way is
    // left alone; it has the surface's current contents.
    GLint allocatedWidth = 0, allocatedHeight = 0, allocatedFormat = 0;
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &allocatedWidth);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &allocatedHeight);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_INTERNAL_FORMAT, &allocatedFormat);
    int hadBinding = texture != 0 && findSurfaceBinding(ctx, (GLuint) texture, &previousBinding);
    int imported = hadBinding
        && previousBinding.imported && previousBinding.surface == ioSurface && previousBinding.plane == plane
        && previousBinding.target == target && previousBinding.internalFormat == internal_format
        && previousBinding.format == format && previousBinding.type == type
        && previousBinding.width == width && previousBinding.height == height
        && allocatedWidth == width && allocatedHeight == height;
    int gpuBuffer = imported && previousBinding.gpuBuffer;
    // The GPU buffer gets writes the CPU made in other processes.
    if (gpuBuffer)
        _IOSurfaceGPUWillUse(ioSurface);

    if (!imported && texture != 0)
        imported = importSurfacePlane(ctx, target, internal_format, width, height, format, type, ioSurface, plane);

    uint32_t shadowFormat = 0;
    GLuint shadow = 0;
    if (!imported)
        shadow = copySurfacePlaneToRectangle(ctx, target, texture, internal_format, width, height, format, type,
                                             ioSurface, plane, NULL, &shadowFormat);

    // A surface that has a GPU buffer is used through it where its memory
    // cannot be.
    _IOSurfaceGPUBuffer buffer;
    if (!imported && shadow == 0 && texture != 0
        && getGPUBuffer(ctx, ioSurface, plane, drmFormat(internal_format, format, type), width, height, 0, &buffer)) {
        _IOSurfaceGPUWillUse(ioSurface);
        if (target == GL_TEXTURE_2D)
            imported = gpuBuffer = importGPUBuffer(ctx, internal_format, width, height, format, type, &buffer);
        else
            shadow = copySurfacePlaneToRectangle(ctx, target, texture, internal_format, width, height, format,
                                                 type, ioSurface, plane, &buffer, &shadowFormat);
        gpuBuffer = imported || shadow != 0;
    }

    if (!imported && shadow == 0) {
        // A failed import may have emptied the texture. A texture that used
        // other memory gets storage of its own, so that the upload does not
        // write into that memory.
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &allocatedWidth);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &allocatedHeight);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_INTERNAL_FORMAT, &allocatedFormat);
        if (hadBinding && previousBinding.imported)
            allocatedWidth = allocatedHeight = 0;

        struct pixel_store saved, transfer = { 0, rowLength, 0, 0, 1 };
        savePixelStore(0, &saved);
        setPixelStore(0, &transfer);

        if (rowLength == 0) {
            // Rows that are not a whole number of pixels apart go up one by one.
            if (allocatedWidth != width || allocatedHeight != height || allocatedFormat != (GLint) internal_format)
                host_glTexImage2D(target, 0, (GLint) internal_format, width, height, 0, format, type, NULL);
            for (GLsizei row = 0; row < height; row++)
                glTexSubImage2D(target, 0, 0, row, width, 1, format, type, (const char *) pixels + row * rowBytes);
        } else if (allocatedWidth == width && allocatedHeight == height && allocatedFormat == (GLint) internal_format)
            glTexSubImage2D(target, 0, 0, 0, width, height, format, type, pixels);
        else
            host_glTexImage2D(target, 0, (GLint) internal_format, width, height, 0, format, type, pixels);

        setPixelStore(0, &saved);
    }

    // The texture's storage moved to or away from a GPU buffer.
    if (texture != 0 && target == GL_TEXTURE_2D && (gpuBuffer || (hadBinding && previousBinding.gpuBuffer)))
        reattachTexture(ctx, target, (GLuint) texture);

    if (texture != 0) {
        struct surface_binding binding = {
            .context = ctx, .texture = (GLuint) texture, .target = target,
            .format = format, .type = type, .plane = plane, .surface = ioSurface,
            .seed = IOSurfaceGetSeed(ioSurface),
            .imported = imported, .internalFormat = internal_format,
            .width = width, .height = height,
            .shadow = shadow, .shadowFormat = shadowFormat,
            .gpuBuffer = gpuBuffer,
        };
        if (!rememberSurfaceBinding(&binding)) {
            if (shadow != 0 && !shadowInUseByAny(ctx, shadow))
                host_glDeleteTextures(1, &shadow);
            error = kCGLBadAlloc;
        }
    }

    leaveContext(ctx, &scope);
    return error;
}

// Set while this thread copies a texture into a surface.
static __thread int t_readingBack;

// Records that the texture of binding matches its surface at seed, so it
// has nothing left to write back.
static void setBindingSeed(const struct surface_binding *binding, uint32_t seed) {
    pthread_mutex_lock(&g_surfaceBindingsMutex);
    for (size_t i = 0; i < g_surfaceBindingCount; i++) {
        if (g_surfaceBindings[i].context == binding->context
            && g_surfaceBindings[i].texture == binding->texture
            && g_surfaceBindings[i].surface == binding->surface) {
            g_surfaceBindings[i].seed = seed;
            g_surfaceBindings[i].dirty = 0;
        }
    }
    pthread_mutex_unlock(&g_surfaceBindingsMutex);
}

// Records how the texture of binding now uses its surface (the fields
// that moveToGPUBuffer changes).
static void updateBinding(const struct surface_binding *binding) {
    pthread_mutex_lock(&g_surfaceBindingsMutex);
    for (size_t i = 0; i < g_surfaceBindingCount; i++) {
        struct surface_binding *recorded = &g_surfaceBindings[i];
        if (recorded->context == binding->context && recorded->texture == binding->texture
            && recorded->surface == binding->surface) {
            recorded->imported = binding->imported;
            recorded->gpuBuffer = binding->gpuBuffer;
            recorded->noGPUBuffer = binding->noGPUBuffer;
            recorded->shadow = binding->shadow;
            recorded->shadowFormat = binding->shadowFormat;
            recorded->seed = binding->seed;
            recorded->dirty = binding->dirty;
            break;
        }
    }
    pthread_mutex_unlock(&g_surfaceBindingsMutex);
}

// Gives the surface of binding, whose texture holds a copy that has been
// rendered into, a GPU buffer (see "GPU buffers"): the texture's contents
// are copied into the buffer, and the texture uses the buffer from then on,
// directly (a 2D texture) or through a shadow. The rendering is then
// reported to IOSurface. In context, which is current and shares the
// texture. Returns 0, changing nothing, if the context does not use GPU
// buffers or the surface cannot have one.
static int moveToGPUBuffer(CGLContextObj context, const struct surface_binding *binding) {
    if (binding->imported || binding->shadow != 0 || binding->rendered == 0 || binding->noGPUBuffer)
        return 0;
    uint32_t fourcc = drmFormat(binding->internalFormat, binding->format, binding->type);
    if (!gpuBufferUsable(context, fourcc))
        return 0;
    if (!glIsTexture(binding->texture) || !rectangleMatchesShadow(binding))
        return 0;

    struct surface_binding updated = *binding;
    _IOSurfaceGPUBuffer buffer;
    GLuint shadow = 0;
    if (getGPUBuffer(context, binding->surface, binding->plane, fourcc, binding->width, binding->height, 1,
                     &buffer))
        shadow = shadowTexture(context, binding->surface, binding->plane, fourcc, binding->width,
                               binding->height, &buffer);
    if (shadow == 0) {
        updated.noGPUBuffer = 1;
        updateBinding(&updated);
        return 0;
    }
    gpuCopy(context, binding->texture, binding->target, shadow, GL_TEXTURE_2D, binding->width,
                      binding->height, 1);

    updated.gpuBuffer = 1;
    updated.dirty = 0;
    updated.shadow = shadow;
    updated.shadowFormat = fourcc;
    if (binding->target == GL_TEXTURE_2D) {
        // The texture becomes the buffer, as a texture specified from a
        // surface that has one would; the temporary shadow goes.
        GLint previous = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous);
        glBindTexture(GL_TEXTURE_2D, binding->texture);
        if (importGPUBuffer(context, binding->internalFormat, binding->width, binding->height, binding->format,
                            binding->type, &buffer)) {
            updated.imported = 1;
            updated.shadow = 0;
            updated.shadowFormat = 0;
        } else {
            // The texture may have been emptied; it keeps a copy of the
            // buffer through the shadow instead.
            struct pixel_store saved, none = { 0, 0, 0, 0, 4 };
            savePixelStore(0, &saved);
            setPixelStore(0, &none);
            host_glTexImage2D(GL_TEXTURE_2D, 0, (GLint) binding->internalFormat, binding->width, binding->height, 0,
                              binding->format, binding->type, NULL);
            setPixelStore(0, &saved);
            gpuCopy(context, shadow, GL_TEXTURE_2D, binding->texture, GL_TEXTURE_2D, binding->width,
                              binding->height, 1);
        }
        glBindTexture(GL_TEXTURE_2D, (GLuint) previous);
        reattachTexture(context, GL_TEXTURE_2D, binding->texture);
        if (updated.imported && !shadowInUseByAny(context, shadow))
            host_glDeleteTextures(1, &shadow);
    }

    // The rendering is in the buffer once the GPU is done; IOSurface then
    // copies it into the memory and changes the seed.
    host_glFinish();
    _IOSurfaceGPUDidWriteBuffer(binding->surface);
    updated.seed = IOSurfaceGetSeed(binding->surface);
    updateBinding(&updated);
    return 1;
}

// The most contexts whose work finishImportedSurface waits for at once.
#define MAX_FINISHED_CONTEXTS 16

// Rendering into the surface's GPU buffer through textures that context
// shares, which the context has finished, is reported to IOSurface, which
// copies it into the memory.
static void reportFinishedBufferWrites(CGLContextObj context, IOSurfaceRef ioSurface) {
    pthread_mutex_lock(&g_surfaceBindingsMutex);
    int report = 0;
    for (size_t i = 0; i < g_surfaceBindingCount; i++) {
        struct surface_binding *binding = &g_surfaceBindings[i];
        if (binding->surface == ioSurface && binding->gpuBuffer && binding->unflushed
            && sharesTextures(binding->context, context)) {
            binding->unflushed = 0;
            report = 1;
        }
    }
    if (report)
        _IOSurfaceGPUDidWriteBuffer(ioSurface);
    pthread_mutex_unlock(&g_surfaceBindingsMutex);
}

// Waits for the GL work of the contexts that have textures using the
// surface's memory or GPU buffer, so that it holds their rendering, and with
// reportBuffers, has the rendering in the GPU buffer copied into the memory.
// A context current on another thread cannot be made current here and is
// skipped; its application flushes it before handing the surface on, as on
// macOS. Returns whether the surface has any such texture.
static int finishImportedSurface(IOSurfaceRef ioSurface, int reportBuffers) {
    CGLContextObj contexts[MAX_FINISHED_CONTEXTS];
    size_t count = 0;
    int found = 0;

    pthread_mutex_lock(&g_surfaceBindingsMutex);
    for (size_t i = 0; i < g_surfaceBindingCount; i++) {
        if (g_surfaceBindings[i].surface != ioSurface || !g_surfaceBindings[i].imported)
            continue;
        found = 1;
        size_t j;
        for (j = 0; j < count && contexts[j] != g_surfaceBindings[i].context; j++)
            ;
        if (j == count && count < MAX_FINISHED_CONTEXTS)
            contexts[count++] = CGLRetainContext(g_surfaceBindings[i].context);
    }
    pthread_mutex_unlock(&g_surfaceBindingsMutex);

    for (size_t i = 0; i < count; i++) {
        struct context_scope scope;
        if (enterContext(contexts[i], &scope, 0) == kCGLNoError) {
            host_glFinish();
            leaveContext(contexts[i], &scope);
            if (reportBuffers)
                reportFinishedBufferWrites(contexts[i], ioSurface);
        }
        CGLReleaseContext(contexts[i]);
    }
    return found;
}

// Copies the texture of binding, which holds a copy of its surface, into
// the surface, in context, which is current and shares the texture. On the
// GPU through the shadow if there is one, else through the CPU.
static CGLError copyBindingToSurface(CGLContextObj context, const struct surface_binding *binding) {
    IOSurfaceRef ioSurface = binding->surface;
    if (moveToGPUBuffer(context, binding))
        return kCGLNoError;
    if (binding->shadow != 0) {
        // The shadow is the surface's memory: the copy lands there once the
        // GPU is done, and a lock marks the surface modified, as the CPU
        // copy does. A texture respecified since is left alone.
        if (!glIsTexture(binding->texture) || !rectangleMatchesShadow(binding))
            return kCGLBadValue;
        gpuCopy(context, binding->texture, binding->target, binding->shadow, GL_TEXTURE_2D,
                          binding->width, binding->height, binding->gpuBuffer);
        host_glFinish();
        if (binding->gpuBuffer) {
            // The shadow is the GPU buffer, which IOSurface copies into the
            // memory.
            _IOSurfaceGPUDidWriteBuffer(ioSurface);
            setBindingSeed(binding, IOSurfaceGetSeed(ioSurface));
            return kCGLNoError;
        }
        uint32_t seed = 0;
        t_readingBack = 1;
        IOSurfaceLock(ioSurface, 0, NULL);
        IOSurfaceUnlock(ioSurface, 0, &seed);
        t_readingBack = 0;
        setBindingSeed(binding, seed);
        return kCGLNoError;
    }

    if (!glIsTexture(binding->texture))
        return kCGLBadValue;
    GLenum bindingQuery = textureBindingQuery(binding->target);
    GLint oldTexture = 0, width = 0, height = 0;
    glGetIntegerv(bindingQuery, &oldTexture);
    glBindTexture(binding->target, binding->texture);
    glGetTexLevelParameteriv(binding->target, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(binding->target, 0, GL_TEXTURE_HEIGHT, &height);

    // The texture may have been respecified since; it must still fit.
    void *pixels;
    GLint rowLength;
    size_t rowBytes, pixelBytes;
    CGLError error = planeForTransfer(ioSurface, binding->plane, width, height, binding->format, binding->type,
                                      &pixels, &rowLength, &rowBytes, &pixelBytes);
    if (error == kCGLNoError) {
        struct pixel_store saved, transfer = { 0, rowLength, 0, 0, 1 };
        savePixelStore(1, &saved);
        setPixelStore(1, &transfer);
        uint32_t seed = 0;
        t_readingBack = 1;
        IOSurfaceLock(ioSurface, 0, NULL);
        if (rowLength == 0) {
            // Rows that are not a whole number of pixels apart: read the
            // texture packed and copy the rows to where they belong.
            size_t packedRow = (size_t) width * pixelBytes;
            uint8_t *packed = malloc(packedRow * (size_t) height);
            if (packed != NULL) {
                glGetTexImage(binding->target, 0, binding->format, binding->type, packed);
                for (GLint row = 0; row < height; row++)
                    memcpy((uint8_t *) pixels + row * rowBytes, packed + row * packedRow, packedRow);
                free(packed);
            }
        } else {
            glGetTexImage(binding->target, 0, binding->format, binding->type, pixels);
        }
        IOSurfaceUnlock(ioSurface, 0, &seed);
        t_readingBack = 0;
        setPixelStore(1, &saved);
        setBindingSeed(binding, seed);
    }

    glBindTexture(binding->target, (GLuint) oldTexture);
    return error;
}

// Brings GL rendering on ioSurface into the surface's memory. Textures that
// use the memory itself only need their contexts' work to finish. A texture
// that holds a copy is copied into the surface: the one last rendered into
// (see "Writing GL rendering back into surfaces"), or, if none has been and
// no texture uses the memory, the one last specified from the surface.
// With onlyIfUnmodified, not if
// the CPU has written the surface since the texture last matched it, as
// its contents are newer then.
static CGLError readBackSurface(IOSurfaceRef ioSurface, int onlyIfUnmodified) {
    int imported = finishImportedSurface(ioSurface, 1);

    struct surface_binding binding = { 0 };
    pthread_mutex_lock(&g_surfaceBindingsMutex);
    size_t chosen = SIZE_MAX;
    for (size_t i = g_surfaceBindingCount; i-- > 0; ) {
        if (g_surfaceBindings[i].surface != ioSurface || g_surfaceBindings[i].imported)
            continue;
        if (chosen == SIZE_MAX || g_surfaceBindings[i].rendered > g_surfaceBindings[chosen].rendered)
            chosen = i;
    }
    if (chosen != SIZE_MAX)
        binding = g_surfaceBindings[chosen];
    if (binding.context && onlyIfUnmodified && IOSurfaceGetSeed(ioSurface) != binding.seed)
        binding.context = NULL;
    // Where textures use the surface's memory, the memory has their
    // rendering, and a copy not rendered into since it was last copied
    // holds nothing newer.
    if (binding.context && imported && !binding.dirty)
        binding.context = NULL;
    // A texture whose rendering has been noted and written back since holds
    // nothing newer either. Other textures are copied, so that changes made
    // in other ways (glTexSubImage2D, glCopyTexSubImage2D) reach the surface.
    if (binding.context && onlyIfUnmodified && binding.rendered != 0 && !binding.dirty)
        binding.context = NULL;
    if (binding.context)
        CGLRetainContext(binding.context);
    pthread_mutex_unlock(&g_surfaceBindingsMutex);
    if (!binding.context) {
        if (!imported)
            return kCGLBadValue;
        // An explicit readback marks the surface modified, as a copy does.
        if (!onlyIfUnmodified) {
            t_readingBack = 1;
            IOSurfaceLock(ioSurface, 0, NULL);
            IOSurfaceUnlock(ioSurface, 0, NULL);
            t_readingBack = 0;
        }
        return kCGLNoError;
    }

    // A context current here that shares the texture copies it without a
    // switch; otherwise the binding's own context is made current.
    CGLContextObj current = CGLGetCurrentContext();
    CGLContextObj copier = current && sharesTextures(current, binding.context) ? current : binding.context;
    struct context_scope scope;
    CGLError error = enterContext(copier, &scope, 0);
    if (error == kCGLNoError) {
        error = copyBindingToSurface(copier, &binding);
        leaveContext(copier, &scope);
    }
    CGLReleaseContext(binding.context);
    return error;
}

CGLError CGLReadbackIOSurface(IOSurfaceRef ioSurface) {
    if (!ioSurface)
        return kCGLBadAddress;
    return readBackSurface(ioSurface, 0);
}

// IOSurface's writeback callback. Rendering into textures that use the
// surface's memory is waited for. A texture that holds a copy has the GPU's
// rendering, so it is copied into the surface unless the CPU has written
// the surface since. A surface with no texture, or one this thread is
// already copying into, is left alone.
static void noteDrawFramebuffer(CGLContextObj context);

static void writeBackSurface(IOSurfaceRef ioSurface) {
    if (t_readingBack)
        return;
    // Rendering of this thread's context is noted first, so that the
    // texture last rendered into is the one copied.
    // (The query goes to the host driver directly: the thread must hold the
    // binding of its current context, which a lock of the surface may find
    // given up.)
    CGLContextObj current = CGLGetCurrentContext();
    struct context_scope scope;
    if (current != NULL && enterContext(current, &scope, 0) == kCGLNoError) {
        noteDrawFramebuffer(current);
        leaveContext(current, &scope);
    }
    readBackSurface(ioSurface, 1);
}

// IOSurface's finish callback, before a lock with kIOSurfaceLockAvoidSync:
// rendering into textures that use the surface's memory is waited for, and
// nothing is copied.
static void finishSurface(IOSurfaceRef ioSurface) {
    if (!t_readingBack)
        finishImportedSurface(ioSurface, 0);
}

// Writing GL rendering back into surfaces.
//
// On macOS the surface is the storage of a texture specified from it, so
// rendering into the texture is in the surface for the CPU and for other
// contexts and processes once the producer has flushed it, whatever happens
// to the texture afterwards. A texture that holds a copy of its surface has
// the rendering only in the texture, so this framework copies it into the
// surface:
//  - when a context that shares the texture calls glFlush, glFinish,
//    glFenceSync or CGLFlushDrawable;
//  - before the texture is deleted (glDeleteTextures), or its level 0 is
//    given other contents (glTexImage2D, which the host carries out, while
//    on macOS the texture keeps the surface) or specified from another
//    surface (CGLTexImageIOSurface2D).
// Only textures that may have been rendered into since their last copy are
// copied, once each. Rendering is noted, without following individual
// draws, when the texture is a color attachment of the context's draw
// framebuffer at one of these calls, when another draw framebuffer is
// bound, when the framebuffer is deleted, and before a lock of the
// surface. The copy keeps to the rule of the copy before a lock: a CPU write
// made since the texture last matched the surface is newer and is kept.
//
// The copy is made in the calling context, which shares the texture, so
// that no other context has to be made current.
//
// IOSurface is told about the rendering as it is noted. Rendering into a
// copy is reported with _IOSurfaceGPUDidWrite, so that a lock with
// kIOSurfaceLockAvoidSync fails until the copy has been written back.
// Rendering into a surface's memory is reported with
// _IOSurfaceGPUDidWriteMemory at the next of these calls, so that the seed
// changes, as it does on macOS when a flush makes rendering into a surface
// visible. A lock with kIOSurfaceLockAvoidSync waits for that rendering
// (the finish callback) but copies nothing.

// The most color attachments of one framebuffer that are looked at.

// Whether a context that shares textures with context has a texture
// specified from a surface.
static int sharesBinding(CGLContextObj context) {
    int found = 0;
    pthread_mutex_lock(&g_surfaceBindingsMutex);
    for (size_t i = 0; i < g_surfaceBindingCount && !found; i++)
        found = sharesTextures(g_surfaceBindings[i].context, context);
    pthread_mutex_unlock(&g_surfaceBindingsMutex);
    return found;
}

// The current context, if there may be anything for it to write back.
static CGLContextObj contextWithCopies(void) {
    if (atomic_load_explicit(&g_bindingCount, memory_order_relaxed) == 0 || t_readingBack)
        return NULL;
    return CGLGetCurrentContext();
}

// Notes rendering into the textures specified from surfaces that are color
// attachments of the draw framebuffer of context, which is current.
static void noteDrawFramebuffer(CGLContextObj context) {
    if (atomic_load_explicit(&g_bindingCount, memory_order_relaxed) == 0 || t_readingBack
        || !sharesBinding(context))
        return;
    GLint framebuffer = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &framebuffer);
    if (framebuffer == 0)
        return;
    if (context->max_color_attachments == 0) {
        GLint max = 0;
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max);
        context->max_color_attachments = max < 1 ? 1 : max > CGL_MAX_NOTED_ATTACHMENTS ? CGL_MAX_NOTED_ATTACHMENTS : max;
    }
    GLuint textures[CGL_MAX_NOTED_ATTACHMENTS];
    int count = 0;
    for (GLint i = 0; i < context->max_color_attachments; i++) {
        GLint type = GL_NONE, name = 0;
        host_glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                                                   GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
        if (type != GL_TEXTURE)
            continue;
        host_glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                                                   GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &name);
        if (name != 0)
            textures[count++] = (GLuint) name;
    }
    if (count == 0)
        return;
    pthread_mutex_lock(&g_surfaceBindingsMutex);
    for (size_t i = 0; i < g_surfaceBindingCount; i++) {
        struct surface_binding *binding = &g_surfaceBindings[i];
        if (!sharesTextures(binding->context, context))
            continue;
        for (int j = 0; j < count; j++) {
            if (binding->texture == textures[j]) {
                binding->dirty = 1;
                binding->rendered = ++g_renderSerial;
                // A copy is newer than the surface until it is written
                // back, which it is unless the CPU has written the surface
                // since the copy last matched it.
                if (binding->imported)
                    binding->unflushed = 1;
                else if (IOSurfaceGetSeed(binding->surface) == binding->seed)
                    _IOSurfaceGPUDidWrite(binding->surface);
                break;
            }
        }
    }
    pthread_mutex_unlock(&g_surfaceBindingsMutex);
}

// Copies the textures that context shares and that may have been rendered
// into since their last copy (only texture, unless it is 0) into their
// surfaces. context is current.
static void writeBackRendered(CGLContextObj context, GLuint texture) {
    for (;;) {
        struct surface_binding binding;
        int found = 0;
        pthread_mutex_lock(&g_surfaceBindingsMutex);
        for (size_t i = 0; i < g_surfaceBindingCount && !found; i++) {
            struct surface_binding *candidate = &g_surfaceBindings[i];
            if (candidate->imported || !candidate->dirty || !sharesTextures(candidate->context, context)
                || (texture != 0 && candidate->texture != texture))
                continue;
            candidate->dirty = 0;
            binding = *candidate;
            CFRetain(binding.surface);
            found = 1;
        }
        pthread_mutex_unlock(&g_surfaceBindingsMutex);
        if (!found)
            return;
        if (IOSurfaceGetSeed(binding.surface) == binding.seed)
            copyBindingToSurface(context, &binding);
        CFRelease(binding.surface);
    }
}

// Rendering into the memory or the GPU buffer of surfaces through textures
// that context, which is current, shares has been submitted: the surfaces'
// contents have changed. The rendering is finished first: IOSurface copies a
// GPU buffer into the memory, and readers of the memory cannot wait for it.
static void reportMemoryWrites(CGLContextObj context) {
    pthread_mutex_lock(&g_surfaceBindingsMutex);
    int finished = 0;
    for (size_t i = 0; i < g_surfaceBindingCount; i++) {
        struct surface_binding *binding = &g_surfaceBindings[i];
        if (!binding->unflushed || !sharesTextures(binding->context, context))
            continue;
        // Bindings of the same surface are reported once.
        for (size_t j = i; j < g_surfaceBindingCount; j++) {
            if (g_surfaceBindings[j].surface == binding->surface
                && g_surfaceBindings[j].gpuBuffer == binding->gpuBuffer
                && sharesTextures(g_surfaceBindings[j].context, context))
                g_surfaceBindings[j].unflushed = 0;
        }
        // glFlush only submits the work, but on macOS the memory holds the
        // rendering once the producer has flushed. A reader in this process
        // waits for it in its lock; one in another process cannot wait for
        // this context's work, so for a surface that another process may
        // map the work is finished (once per call) before the report.
        if (!finished && (binding->gpuBuffer || _IOSurfaceIsShared(binding->surface))) {
            host_glFinish();
            finished = 1;
        }
        if (binding->gpuBuffer)
            _IOSurfaceGPUDidWriteBuffer(binding->surface);
        else
            _IOSurfaceGPUDidWriteMemory(binding->surface);
    }
    pthread_mutex_unlock(&g_surfaceBindingsMutex);
}

static void writeBackCurrentContext(void) {
    CGLContextObj context = contextWithCopies();
    if (context == NULL)
        return;
    noteDrawFramebuffer(context);
    writeBackRendered(context, 0);
    reportMemoryWrites(context);
}

// Set while glTexImage2D specifies a texture from its surface again, whose
// contents are then not the rendering's.
static __thread int t_respecifying;

static void writeBackTexture(CGLContextObj context, GLuint texture) {
    if (atomic_load_explicit(&g_bindingCount, memory_order_relaxed) == 0 || t_readingBack || t_respecifying)
        return;
    noteDrawFramebuffer(context);
    writeBackRendered(context, texture);
}

// Whether a texture that context shares uses its surface's memory or GPU
// buffer.
static int textureImported(CGLContextObj context, GLuint texture) {
    int imported = 0;
    pthread_mutex_lock(&g_surfaceBindingsMutex);
    for (size_t i = 0; i < g_surfaceBindingCount && !imported; i++) {
        imported = g_surfaceBindings[i].imported && g_surfaceBindings[i].texture == texture
            && sharesTextures(g_surfaceBindings[i].context, context);
    }
    pthread_mutex_unlock(&g_surfaceBindingsMutex);
    return imported;
}

// Writes the texture, which context shares and which is current, back into
// its surface and drops its bindings, as it is deleted, so that the surface
// is released as on macOS. A texture that uses the surface's memory or GPU
// buffer has its rendering finished first: no later lock of the surface
// waits for a context without textures on it.
static void releaseTextureBindings(CGLContextObj context, GLuint texture) {
    writeBackRendered(context, texture);
    if (textureImported(context, texture))
        host_glFinish();
    GLuint unused[8];
    int unusedCount = 0;
    pthread_mutex_lock(&g_surfaceBindingsMutex);
    for (size_t i = 0; i < g_surfaceBindingCount; ) {
        struct surface_binding *binding = &g_surfaceBindings[i];
        if (binding->texture != texture || !sharesTextures(binding->context, context)) {
            i++;
            continue;
        }
        GLuint shadow = binding->shadow;
        removeSurfaceBinding(i);
        int used = 0;
        for (size_t j = 0; j < g_surfaceBindingCount && !used; j++)
            used = g_surfaceBindings[j].shadow == shadow && sharesTextures(g_surfaceBindings[j].context, context);
        for (int j = 0; j < unusedCount && !used; j++)
            used = unused[j] == shadow;
        if (shadow != 0 && !used && unusedCount < (int) (sizeof(unused) / sizeof(unused[0])))
            unused[unusedCount++] = shadow;
    }
    pthread_mutex_unlock(&g_surfaceBindingsMutex);
    if (unusedCount > 0)
        host_glDeleteTextures(unusedCount, unused);
}

void override_glFlush(void) {
    writeBackCurrentContext();
    host_glFlush();
}

void override_glFinish(void) {
    writeBackCurrentContext();
    host_glFinish();
}

// The copies are made before the fence, so waiting for it covers them.
GLsync override_glFenceSync(GLenum condition, GLbitfield flags) {
    writeBackCurrentContext();
    return host_glFenceSync(condition, flags);
}

static void noteCurrentDrawFramebuffer(GLenum target) {
    CGLContextObj context = contextWithCopies();
    if (context != NULL && target != GL_READ_FRAMEBUFFER)
        noteDrawFramebuffer(context);
}

void override_glBindFramebuffer(GLenum target, GLuint framebuffer) {
    noteCurrentDrawFramebuffer(target);
    host_glBindFramebuffer(target, framebuffer);
}

void override_glBindFramebufferEXT(GLenum target, GLuint framebuffer) {
    noteCurrentDrawFramebuffer(target);
    host_glBindFramebufferEXT(target, framebuffer);
}

void override_glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers) {
    noteCurrentDrawFramebuffer(GL_DRAW_FRAMEBUFFER);
    host_glDeleteFramebuffers(n, framebuffers);
}

void override_glDeleteFramebuffersEXT(GLsizei n, const GLuint *framebuffers) {
    noteCurrentDrawFramebuffer(GL_DRAW_FRAMEBUFFER);
    host_glDeleteFramebuffersEXT(n, framebuffers);
}

void override_glDeleteTextures(GLsizei n, const GLuint *textures) {
    CGLContextObj context = contextWithCopies();
    if (context != NULL && n > 0 && textures != NULL) {
        noteDrawFramebuffer(context);
        for (GLsizei i = 0; i < n; i++) {
            if (textures[i] != 0)
                releaseTextureBindings(context, textures[i]);
        }
    }
    host_glDeleteTextures(n, textures);
}

// QUIRK: glTexImage2D on level 0 of a texture specified from a surface
// leaves the texture the surface's, with its size and contents: rendering
// before the call stays in the surface and rendering after it reaches the
// surface. Not documented. Evidence: black-box observation on macOS 26.7.3
// (tier 2), Apple Software Renderer, rectangle textures, with and without
// pixel data and with another size; the call raises GL_INVALID_OPERATION
// there, which is not reproduced here. Test: the respecify case of
// tests/src/opengl_iosurface_writeback.m.
// The host carries the call out, so what was rendered before is written
// back first and the texture is then specified from the surface again.
void override_glTexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height,
                             GLint border, GLenum format, GLenum type, const GLvoid *pixels) {
    CGLContextObj context = NULL;
    GLint texture = 0;
    struct surface_binding binding;
    int bound = 0;
    if (level == 0 && (target == GL_TEXTURE_2D || target == GL_TEXTURE_RECTANGLE)
        && (context = contextWithCopies()) != NULL) {
        glGetIntegerv(textureBindingQuery(target), &texture);
        if (texture != 0) {
            writeBackTexture(context, (GLuint) texture);
            bound = findSurfaceBinding(context, (GLuint) texture, &binding) && binding.target == target;
            if (bound)
                CFRetain(binding.surface);
        }
    }
    host_glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
    if (bound) {
        // A texture that used the surface's memory does not any more; it is
        // imported again.
        if (binding.imported) {
            pthread_mutex_lock(&g_surfaceBindingsMutex);
            for (size_t i = 0; i < g_surfaceBindingCount; i++) {
                if (g_surfaceBindings[i].context == context && g_surfaceBindings[i].texture == (GLuint) texture) {
                    removeSurfaceBinding(i);
                    break;
                }
            }
            pthread_mutex_unlock(&g_surfaceBindingsMutex);
        }
        t_respecifying = 1;
        CGLTexImageIOSurface2D(context, target, binding.internalFormat, binding.width, binding.height,
                               binding.format, binding.type, binding.surface, binding.plane);
        t_respecifying = 0;
        CFRelease(binding.surface);
    }
}

CGLError CGLSetDrawableRefreshRate(CGLContextObj context, uint32_t millihertz) {
    if (context == NULL)
        return kCGLBadContext;
    context->pacer.period_ns = cgl_period_ns_from_millihertz(millihertz);
    return kCGLNoError;
}

CGLError CGLSetParameter(CGLContextObj context, CGLContextParameter parameter, const GLint *value) {
    if (!value)
        return kCGLBadAddress;

    if (!context)
        return kCGLBadContext;

    if (parameter == kCGLCPSwapInterval)
    {
        GLint v = *value;
        EGLBoolean success = EGL_TRUE;
        // EGL sets the interval of the surface current on the calling thread:
        // a context this thread holds. A context that is current here but not
        // held (parked, or given up after its last call) gets the interval
        // when it is bound again (see bindCurrentContext).
        pthread_mutex_lock(&g_bindingMutex);
        int held = threadHoldsBinding(context);
        pthread_mutex_unlock(&g_bindingMutex);
        if (held && atomic_load_explicit(&context->egl_surface, memory_order_acquire) != EGL_NO_SURFACE)
            success = p_eglSwapInterval(context->egl_display, context->paced ? 0 : v);
        if (success)
            context->swap_interval = v;
        return success ? kCGLNoError : kCGLBadValue;
    }
    // 280 and 284 size internal function caches of the original implementation
    // (set by SDL 1.2, among others); they have no observable effect here.
    if (parameter == 280 || parameter == 284)
        return kCGLNoError;
    fprintf(stderr, "CGLSetParameter unimplemented for parameter %d\n", parameter);
    return kCGLNoError;
}

CGLError CGLGetParameter(CGLContextObj context, CGLContextParameter parameter, GLint *value) {
    if (!value)
        return kCGLBadAddress;
    if (!context)
        return kCGLBadContext;

    if (parameter == kCGLCPSwapInterval)
    {
        *value = context->swap_interval;
        return kCGLNoError;
    }
    fprintf(stderr, "CGLGetParameter unimplemented for parameter %d\n", parameter);
    return kCGLNoError;
}

// Renderer information.
//
// As on macOS there are two renderers: the host's GL implementation, described from what the host
// reports about the GL context an application would get (the GL strings and the EGL device behind the
// display, with the video memory from the IORegistry), and a software renderer that is never accelerated.

struct cgl_renderer {
    GLint renderer_id;
    GLint accelerated;
    // video memory in bytes (0 if it could not be found out)
    uint64_t video_memory;
    GLint max_samples;
    GLint gl_major;
};

struct _CGLRendererInfoObj {
    GLint count;
    GLuint display_mask;
    struct cgl_renderer renderers[2];
};

// GL_NVX_gpu_memory_info
#define CGL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX 0x9047

// The renderer ID that describes a renderer in the vocabulary of CGLRenderers.h: the family of the
// GPU vendor, or the software renderer.
static GLint rendererIDFor(const char *vendor, const char *renderer, int accelerated) {
    if (!accelerated)
        return kCGLRendererGenericFloatID;
    if (vendor && (strstr(vendor, "NVIDIA") || strstr(vendor, "Nouveau")))
        return kCGLRendererGeForceID;
    if (vendor && (strstr(vendor, "AMD") || strstr(vendor, "ATI")))
        return kCGLRendererATIRadeonX4000ID;
    if (vendor && strstr(vendor, "Intel"))
        return kCGLRendererIntelHD5000ID;
    if (renderer && strstr(renderer, "NVIDIA"))
        return kCGLRendererGeForceID;
    if (renderer && (strstr(renderer, "AMD") || strstr(renderer, "Radeon")))
        return kCGLRendererATIRadeonX4000ID;
    if (renderer && strstr(renderer, "Intel"))
        return kCGLRendererIntelHD5000ID;
    // a GPU from another vendor: the closest family that stands for "a recent GPU"
    return kCGLRendererGeForceID;
}

// The video memory of the GPU in megabytes, from the IORegistry: the IOAccelerator service's
// "VRAM,totalMB", which is where macOS publishes it. The registry (iokitd) also decides what is
// reported for a GPU that has no memory of its own. With several GPUs the first one that has a
// figure is taken; the registry does not say which of them the host's GL context is on.
// 0 if the registry has none.
static uint64_t registryVideoMegabytes(void) {
    io_iterator_t iterator = IO_OBJECT_NULL;
    if (IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("IOAccelerator"), &iterator) != KERN_SUCCESS)
        return 0;
    uint64_t megabytes = 0;
    CFStringRef key = CFStringCreateWithCString(NULL, "VRAM,totalMB", kCFStringEncodingUTF8);
    io_registry_entry_t service;
    while (megabytes == 0 && (service = IOIteratorNext(iterator)) != IO_OBJECT_NULL) {
        CFTypeRef value = IORegistryEntryCreateCFProperty(service, key, NULL, 0);
        if (value) {
            int64_t number = 0;
            if (CFGetTypeID(value) == CFNumberGetTypeID() && CFNumberGetValue(value, kCFNumberSInt64Type, &number) && number > 0)
                megabytes = (uint64_t) number;
            CFRelease(value);
        }
        IOObjectRelease(service);
    }
    CFRelease(key);
    IOObjectRelease(iterator);
    return megabytes;
}

// Describes the renderer of the context that is current, which has the given display.
static void describeCurrentRenderer(EGLDisplay egl_display, struct cgl_renderer *out) {
    const char *vendor = (const char *) host_glGetString(GL_VENDOR);
    const char *renderer = (const char *) host_glGetString(GL_RENDERER);
    const char *version = (const char *) host_glGetString(GL_VERSION);
    int major = 0, minor = 0;
    if (version)
        sscanf(version, "%d.%d", &major, &minor);

    static const char *const software_names[] = { "llvmpipe", "lavapipe", "softpipe", "swrast", "Software Rasterizer" };
    int accelerated = 1;
    for (size_t i = 0; renderer && i < sizeof(software_names) / sizeof(software_names[0]); i++) {
        if (strstr(renderer, software_names[i]))
            accelerated = 0;
    }
    if (accelerated && softwareEGLDevice(egl_display))
        accelerated = 0;

    // the registry's figure, else what the GL driver says about its dedicated memory
    uint64_t video_memory = registryVideoMegabytes() * 1024 * 1024;
    if (video_memory == 0 && accelerated) {
        load_image_functions();
        if (has_gl_extension(major, "GL_NVX_gpu_memory_info")) {
            GLint kilobytes = 0;
            glGetIntegerv(CGL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &kilobytes);
            if (kilobytes > 0)
                video_memory = (uint64_t) kilobytes * 1024;
        }
    }

    GLint max_samples = 0;
    if (major >= 3)
        glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
    glGetError();

    out->renderer_id = rendererIDFor(vendor, renderer, accelerated);
    out->accelerated = accelerated;
    out->video_memory = video_memory;
    out->max_samples = max_samples > 0 ? max_samples : 0;
    out->gl_major = major;
}

// Makes a context just to ask it, and puts back whatever was current. The answer for a display is kept:
// it cannot change while the display stays.
static int describeHostRenderer(struct cgl_renderer *out) {
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    static struct cgl_renderer cached;
    static EGLDisplay cached_display = EGL_NO_DISPLAY;
    static int cached_valid;

    pthread_mutex_lock(&registration_lock);
    EGLDisplay wanted = display;
    pthread_mutex_unlock(&registration_lock);

    pthread_mutex_lock(&lock);
    if (cached_valid && cached_display == wanted) {
        *out = cached;
        pthread_mutex_unlock(&lock);
        return 1;
    }

    int ok = 0;
    CGLContextObj context = NULL;
    CGLContextObj previous = CGLGetCurrentContext();
    if (createContext(NULL, NULL, NULL, &context) == kCGLNoError) {
        CGLRetainContext(previous);
        if (CGLSetCurrentContext(context) == kCGLNoError) {
            describeCurrentRenderer(context->egl_display, out);
            ok = 1;
            CGLSetCurrentContext(previous);
        }
        CGLReleaseContext(previous);
        CGLReleaseContext(context);
    }
    if (ok) {
        cached = *out;
        cached_display = wanted;
        cached_valid = 1;
    }
    pthread_mutex_unlock(&lock);
    return ok;
}

CGLError CGLQueryRendererInfo(GLuint display_mask, CGLRendererInfoObj *rend, GLint *nrend) {
    if (!rend || !nrend)
        return kCGLBadAddress;
    *rend = NULL;
    *nrend = 0;
    // a mask without a display selects no renderer
    if (display_mask == 0)
        return kCGLBadDisplay;
    if (!native_egl_available())
        return kCGLBadRendererInfo;

    struct _CGLRendererInfoObj *info = calloc(1, sizeof(*info));
    if (!info)
        return kCGLBadAlloc;
    if (!describeHostRenderer(&info->renderers[0])) {
        free(info);
        return kCGLBadRendererInfo;
    }
    // the software renderer: no video memory, the GL version of the host
    info->renderers[1].renderer_id = kCGLRendererGenericFloatID;
    info->renderers[1].accelerated = 0;
    info->renderers[1].video_memory = 0;
    info->renderers[1].max_samples = 0;
    info->renderers[1].gl_major = info->renderers[0].gl_major;
    info->count = 2;
    // the renderer draws to whichever of the requested displays there are
    info->display_mask = display_mask;
    *rend = info;
    *nrend = 2;
    return kCGLNoError;
}

CGLError CGLDescribeRenderer(CGLRendererInfoObj rend, GLint rend_num, CGLRendererProperty prop, GLint *value) {
    if (!value)
        return kCGLBadAddress;
    if (!rend)
        return kCGLBadRendererInfo;
    if (prop == kCGLRPRendererCount) {
        *value = rend->count;
        return kCGLNoError;
    }
    if (rend_num < 0 || rend_num >= rend->count)
        return kCGLBadValue;

    const struct cgl_renderer *renderer = &rend->renderers[rend_num];
    uint64_t megabytes = renderer->video_memory / (1024 * 1024);
    switch ((int) prop) {
    case kCGLRPRendererID: *value = renderer->renderer_id; break;
    case kCGLRPAccelerated: *value = renderer->accelerated; break;
    case kCGLRPDisplayMask: *value = (GLint) rend->display_mask; break;
    case kCGLRPWindow: *value = 1; break;
    case kCGLRPOffScreen: *value = 0; break;
    case kCGLRPFullScreen: *value = renderer->accelerated; break;
    case kCGLRPRobust: *value = 0; break;
    case kCGLRPMPSafe: *value = 1; break;
    case kCGLRPMultiScreen: *value = 1; break;
    case kCGLRPBackingStore: *value = 1; break;
    case kCGLRPCompliant: *value = 1; break;
    case kCGLRPOnline: *value = 1; break;
    case kCGLRPGPUVertProcCapable:
    case kCGLRPGPUFragProcCapable:
    case kCGLRPAcceleratedCompute:
        *value = renderer->accelerated;
        break;
    case kCGLRPVideoMemoryMegabytes:
    case kCGLRPTextureMemoryMegabytes:
        *value = megabytes > (uint64_t) INT_MAX ? INT_MAX : (GLint) megabytes;
        break;
    case kCGLRPVideoMemory:
    case kCGLRPTextureMemory:
        // the figure in bytes, as far as an int can say it
        *value = renderer->video_memory > (uint64_t) INT_MAX ? INT_MAX : (GLint) renderer->video_memory;
        break;
    case kCGLRPMaxSampleBuffers: *value = renderer->max_samples > 0; break;
    case kCGLRPMaxSamples: *value = renderer->max_samples; break;
    case kCGLRPMajorGLVersion: *value = renderer->gl_major; break;
    default:
        *value = 0;
        return kCGLBadProperty;
    }
    return kCGLNoError;
}

CGLError CGLDestroyRendererInfo(CGLRendererInfoObj rend) {
    free(rend);
    return kCGLNoError;
}

void CGLGetVersion(GLint *majorvers, GLint *minorvers) {
    if (majorvers)
        *majorvers = 1;
    if (minorvers)
        *minorvers = 0;
}
