#include <stdlib.h>
#include <stdio.h>  // for fprintf(stderr, "unimplemented")
#include <string.h>
#include <stdint.h>

#include <pthread.h>
#include <stdatomic.h>
#include <dlfcn.h>

#define GL_GLEXT_PROTOTYPES 1
#include <OpenGL/OpenGL.h>
#include <OpenGL/glext.h>
#include <OpenGL/CGLInternal.h>
#include <CoreFoundation/CFDictionary.h>
#include <IOSurface/IOSurface.h>
#include <IOSurface/IOSurfacePriv.h>
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

static void load_native_egl(void) {
    void *handle = dlopen("/usr/lib/native/libEGL.dylib", RTLD_NOW | RTLD_LOCAL);
    if (!handle)
        return;
#define CGL_EGL_RESOLVE(name) \
    if (!(p_##name = (__typeof__(p_##name)) dlsym(handle, #name))) { \
        dlclose(handle); \
        return; \
    }
    CGL_EGL_FUNCTIONS(CGL_EGL_RESOLVE)
#undef CGL_EGL_RESOLVE
    native_egl_loaded = 1;
}

// Every entry point that can reach EGL before a context or a window exists
// checks this; contexts and windows only exist once EGL has been loaded.
static int native_egl_available(void) {
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    pthread_once(&once, load_native_egl);
    return native_egl_loaded;
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

struct _CGLContextObj {
    GLuint retain_count;
    pthread_mutex_t lock;
    EGLDisplay egl_display;
    EGLConfig egl_config;
    EGLContext egl_context;
    _Atomic EGLSurface egl_surface;
    EGLSurface fallback_pbuffer;
    // EGL has no function for getting the current swap interval,
    // so we need to save the last set value. The default is 0.
    // This is the application's value; a paced context never gives it to EGL.
    int swap_interval;
    int paced;
    struct cgl_swap_pacer pacer;
    CGLPixelFormatObj pixel_format;
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
    struct _CGLContextObj *prev_active;
    struct _CGLContextObj *next_active;
};

static struct _CGLContextObj *g_activeContexts = NULL;
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

__attribute__((constructor))
static void _CGLInitialize(void)
{
    g_displays = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);

    // A binding retains its surface, so a surface is never freed while
    // this framework has state for it and no forget callback is needed.
    static const _IOSurfaceGPUCallbacks callbacks = {
        .version = _IOSURFACE_GPU_CALLBACKS_VERSION,
        .writeback = writeBackSurface,
        .forget = NULL,
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
    if (!native_display || !native_egl_available())
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
};

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

CGL_EXPORT void CGLDestroyWindow(CGLWindowRef ref) {
    struct _CGLWindow *window = ref;
    if (!window)
        return;

    CGLContextObj current = CGLGetCurrentContext();
    int current_cleared = 0;

    if (window->surface != EGL_NO_SURFACE) {
        pthread_mutex_lock(&g_activeContextsMutex);
        for (struct _CGLContextObj *ctx = g_activeContexts; ctx != NULL; ctx = ctx->next_active) {
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

    if (current_cleared && current && current->egl_display != EGL_NO_DISPLAY) {
        // Disassociate the destroyed window surface from the current EGL thread
        p_eglMakeCurrent(current->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, current->egl_context);
    }

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

// Makes context current without bringing IOSurface contents into its
// rectangle textures; used inside this framework.
static CGLError setCurrentContext(CGLContextObj context) {
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

static void refreshRectangleTextures(CGLContextObj context);

CGLError CGLSetCurrentContext(CGLContextObj context) {
    CGLError error = setCurrentContext(context);
    // A context that becomes current is about to be used, so rectangle
    // textures specified from surfaces the CPU has written since get the
    // new contents.
    if (error == kCGLNoError && context != NULL)
        refreshRectangleTextures(context);
    return error;
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

CGLError CGLSetFullScreen(CGLContextObj ctx) {
    printf("STUB: CGLSetFullScreen\n");

    return kCGLNoError;
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
    atomic_store_explicit(&ctx->egl_surface, (EGLSurface)EGL_NO_SURFACE, memory_order_release);
    int is_current = (CGLGetCurrentContext() == ctx);
    pthread_mutex_unlock(&ctx->lock);

    /* egl_display and egl_context are immutable after CGLCreateContext */
    if (ctx->egl_display != EGL_NO_DISPLAY) {
        if (old_fallback != EGL_NO_SURFACE) {
            p_eglDestroySurface(ctx->egl_display, old_fallback);
        }
        if (is_current) {
            // Disassociate any surface from the current thread's EGL context
            p_eglMakeCurrent(ctx->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx->egl_context);
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

    CGLContextObj context = malloc(sizeof(struct _CGLContextObj));

    if (context == NULL) {
        p_eglDestroyContext(owner, egl_context);
        return kCGLBadAlloc;
    }

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
    context->swap_interval = interval;
    context->paced = paced;
    context->pacer.period_ns = cgl_period_ns_from_millihertz(0);
    context->pacer.last_ns = 0;
    context->pixel_format = pixelFormat ? CGLRetainPixelFormat(pixelFormat) : NULL;
    context->enables = 0;
    atomic_init(&context->dmabuf_import, 0);
    context->gpu_copy = 0;
    context->copy_framebuffers[0] = context->copy_framebuffers[1] = 0;

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

    pthread_mutex_destroy(&(context->lock));

    if (context->pixel_format != NULL)
        CGLReleasePixelFormat(context->pixel_format);

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
    pthread_mutex_lock(&(context->lock));
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
    pthread_mutex_unlock(&(context->lock));
    return kCGLNoError;
}

CGLError CGLFlushDrawable(CGLContextObj context) {
    if (context == NULL) {
        return kCGLBadContext;
    }
    EGLSurface surf = atomic_load_explicit(&context->egl_surface, memory_order_acquire);
    if (surf == EGL_NO_SURFACE) {
        glFlush();
        return kCGLNoError;
    }
    glFinish();
    // Re-check after glFinish() in case CGLDestroyWindow cleared the surface concurrently
    EGLSurface surf_after = atomic_load_explicit(&context->egl_surface, memory_order_acquire);
    if (surf_after == EGL_NO_SURFACE || surf_after != surf) {
        return kCGLNoError;
    }
    if (context->paced)
        cgl_swap_pace(&context->pacer, context->swap_interval);
    EGLBoolean ok = p_eglSwapBuffers(context->egl_display, surf);
    return ok ? kCGLNoError : kCGLBadDrawable;
}

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
// call. GL rendering into the texture reaches the surface through
// CGLReadbackIOSurface, which IOSurface also calls before a lock of the
// surface or the creation of a Mach port for it (see writeBackSurface).
//
// Each texture remembers the surface it was last specified from. The binding
// retains the surface, as the texture would on macOS, and is dropped when
// the same texture is specified from another surface or when the context is
// destroyed.

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
};

static struct surface_binding *g_surfaceBindings;
static size_t g_surfaceBindingCount, g_surfaceBindingCapacity;
static pthread_mutex_t g_surfaceBindingsMutex = PTHREAD_MUTEX_INITIALIZER;

static void forgetSurfaceBindings(CGLContextObj context) {
    pthread_mutex_lock(&g_surfaceBindingsMutex);
    for (size_t i = 0; i < g_surfaceBindingCount; ) {
        if (g_surfaceBindings[i].context == context) {
            CFRelease(g_surfaceBindings[i].surface);
            g_surfaceBindings[i] = g_surfaceBindings[--g_surfaceBindingCount];
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
            CFRelease(g_surfaceBindings[i].surface);
        } else {
            g_surfaceBindingCount++;
        }
        g_surfaceBindings[i] = *binding;
    }
    pthread_mutex_unlock(&g_surfaceBindingsMutex);
    if (unusedShadow != 0)
        glDeleteTextures(1, &unusedShadow);
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
// returns the plane's address and its row stride in pixels.
static CGLError planeForTransfer(IOSurfaceRef surface, GLuint plane, GLsizei width, GLsizei height,
                                 GLenum format, GLenum type, void **pixels, GLint *rowLength) {
    size_t bytesPerRow = IOSurfaceGetBytesPerRowOfPlane(surface, plane);
    size_t planeHeight = IOSurfaceGetHeightOfPlane(surface, plane);
    size_t size = pixelSize(format, type);
    if (size == 0)
        size = IOSurfaceGetBytesPerElementOfPlane(surface, plane);
    *pixels = IOSurfaceGetBaseAddressOfPlane(surface, plane);
    if (!*pixels || bytesPerRow == 0 || size == 0 || bytesPerRow % size != 0)
        return kCGLBadValue;
    if (width <= 0 || height <= 0 || (size_t) width > bytesPerRow / size || (size_t) height > planeHeight)
        return kCGLBadValue;
    if (bytesPerRow / size > INT32_MAX)
        return kCGLBadValue;
    *rowLength = (GLint) (bytesPerRow / size);
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
}

static int has_extension(const char *extensions, const char *name) {
    size_t length = strlen(name);
    for (const char *found = extensions; (found = strstr(found, name)) != NULL; found += length) {
        if ((found == extensions || found[-1] == ' ') && (found[length] == ' ' || found[length] == '\0'))
            return 1;
    }
    return 0;
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

// Makes an EGL image of width x height pixels of the plane's memory in DRM
// format fourcc, or returns EGL_NO_IMAGE_KHR.
static EGLImageKHR createPlaneImage(CGLContextObj context, uint32_t fourcc, GLsizei width, GLsizei height,
                                   IOSurfaceRef surface, GLuint plane) {
    enum dmabuf_import support = dmabuf_import(context);
    if (support == DMABUF_IMPORT_UNSUPPORTED)
        return 0;

    if (support == DMABUF_IMPORT_UNSUPPORTED)
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
    // The texture has the image's size afterwards only if the driver took
    // the image; it is emptied first so that its old size does not count.
    // Checking it so leaves the GL error state to the application.
    glTexImage2D(target, 0, (GLint) internal_format, 0, 0, 0, format, type, NULL);
    p_glEGLImageTargetTexture2DOES(target, image);
    // The texture keeps the memory after the image is gone.
    p_eglDestroyImageKHR(context->egl_display, image);

    GLint importedWidth = 0, importedHeight = 0;
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &importedWidth);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &importedHeight);
    return importedWidth == width && importedHeight == height;
}

// Makes context current on this thread for the duration of a call.
static CGLError enterContext(CGLContextObj context, CGLContextObj *previous) {
    *previous = CGLGetCurrentContext();
    if (*previous == context)
        return kCGLNoError;
    return setCurrentContext(context);
}

static void leaveContext(CGLContextObj context, CGLContextObj previous) {
    if (previous != context)
        setCurrentContext(previous);
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
        glGetIntegerv(GL_NUM_EXTENSIONS, &count);
        for (GLint i = 0; i < count; i++) {
            const GLubyte *extension = p_glGetStringi(GL_EXTENSIONS, (GLuint) i);
            if (extension && strcmp((const char *) extension, name) == 0)
                return 1;
        }
        return 0;
    }
    const GLubyte *extensions = glGetString(GL_EXTENSIONS);
    return extensions && has_extension((const char *) extensions, name);
}

// How the current context copies between textures.
static enum gpu_copy gpu_copy(CGLContextObj context) {
    if (context->gpu_copy != GPU_COPY_UNKNOWN)
        return (enum gpu_copy) context->gpu_copy;

    int major = 0, minor = 0;
    const GLubyte *version = glGetString(GL_VERSION);
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
// size. Returns 0 if the context has no way to do it.
static int gpuCopy(CGLContextObj context, GLuint source, GLenum sourceTarget,
                   GLuint destination, GLenum destinationTarget, GLsizei width, GLsizei height) {
    switch (gpu_copy(context)) {
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
    glBindFramebuffer(GL_READ_FRAMEBUFFER, context->copy_framebuffers[0]);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sourceTarget, source, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, context->copy_framebuffers[1]);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, destinationTarget, destination, 0);
    p_glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    // Detached again, so that the framebuffers keep no texture alive.
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sourceTarget, 0, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, destinationTarget, 0, 0);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, (GLuint) readFramebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint) drawFramebuffer);
    if (scissor)
        glEnable(GL_SCISSOR_TEST);
    if (srgb)
        glEnable(GL_FRAMEBUFFER_SRGB);
    return 1;
}

// The shadow for a rectangle texture of the plane: a binding's shadow of
// the same plane, format and size in the context, or a new 2D texture
// imported from the plane. Returns 0 if there is none.
static GLuint shadowTexture(CGLContextObj context, IOSurfaceRef surface, GLuint plane, uint32_t fourcc,
                            GLsizei width, GLsizei height) {
    GLuint shadow = 0;
    pthread_mutex_lock(&g_surfaceBindingsMutex);
    for (size_t i = 0; i < g_surfaceBindingCount && shadow == 0; i++) {
        const struct surface_binding *binding = &g_surfaceBindings[i];
        if (binding->context == context && binding->shadow != 0 && binding->surface == surface
            && binding->plane == plane && binding->shadowFormat == fourcc
            && binding->width == width && binding->height == height)
            shadow = binding->shadow;
    }
    pthread_mutex_unlock(&g_surfaceBindingsMutex);
    if (shadow != 0)
        return shadow;

    EGLImageKHR image = createPlaneImage(context, fourcc, width, height, surface, plane);
    if (image == EGL_NO_IMAGE_KHR)
        return 0;
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
        glDeleteTextures(1, &shadow);
        return 0;
    }
    return shadow;
}

// Gives the rectangle texture bound to target the plane's contents through
// a shadow. Returns the shadow, or 0, leaving the texture to the CPU copy,
// if the format, the driver or the context do not allow it.
static GLuint copySurfacePlaneToRectangle(CGLContextObj context, GLenum target, GLint texture,
                                          GLenum internal_format, GLsizei width, GLsizei height,
                                          GLenum format, GLenum type, IOSurfaceRef surface,
                                          GLuint plane, uint32_t *shadowFormat) {
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
    GLuint shadow = shadowTexture(context, surface, plane, fourcc, width, height);
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
        glTexImage2D(target, 0, (GLint) internal_format, width, height, 0, format, type, NULL);
        setPixelStore(0, &saved);
    }
    gpuCopy(context, shadow, GL_TEXTURE_2D, (GLuint) texture, target, width, height);
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
            if (candidate->context != context || candidate->shadow == 0)
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
        if (glIsTexture(binding.texture) && rectangleMatchesShadow(&binding))
            gpuCopy(context, binding.shadow, GL_TEXTURE_2D, binding.texture, binding.target,
                binding.width, binding.height);
    }
}

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
    CGLError error = planeForTransfer(ioSurface, plane, width, height, format, type, &pixels, &rowLength);
    if (error != kCGLNoError)
        return error;

    CGLContextObj previous;
    error = enterContext(ctx, &previous);
    if (error != kCGLNoError)
        return error;

    // GL errors from the upload are left for the application to query, as
    // are errors that were pending before this call.
    GLint texture = 0;
    glGetIntegerv(bindingQuery, &texture);

    // A texture that already uses this plane's memory in the same way is
    // left alone; it has the surface's current contents.
    struct surface_binding previousBinding;
    GLint allocatedWidth = 0, allocatedHeight = 0, allocatedFormat = 0;
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &allocatedWidth);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &allocatedHeight);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_INTERNAL_FORMAT, &allocatedFormat);
    int imported = texture != 0 && findSurfaceBinding(ctx, (GLuint) texture, &previousBinding)
        && previousBinding.imported && previousBinding.surface == ioSurface && previousBinding.plane == plane
        && previousBinding.target == target && previousBinding.internalFormat == internal_format
        && previousBinding.format == format && previousBinding.type == type
        && previousBinding.width == width && previousBinding.height == height
        && allocatedWidth == width && allocatedHeight == height;

    if (!imported && texture != 0)
        imported = importSurfacePlane(ctx, target, internal_format, width, height, format, type, ioSurface, plane);

    uint32_t shadowFormat = 0;
    GLuint shadow = 0;
    if (!imported)
        shadow = copySurfacePlaneToRectangle(ctx, target, texture, internal_format, width, height, format, type,
                                             ioSurface, plane, &shadowFormat);

    if (!imported && shadow == 0) {
        // A failed import may have emptied the texture.
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &allocatedWidth);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &allocatedHeight);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_INTERNAL_FORMAT, &allocatedFormat);

        struct pixel_store saved, transfer = { 0, rowLength, 0, 0, 1 };
        savePixelStore(0, &saved);
        setPixelStore(0, &transfer);

        if (allocatedWidth == width && allocatedHeight == height && allocatedFormat == (GLint) internal_format)
            glTexSubImage2D(target, 0, 0, 0, width, height, format, type, pixels);
        else
            glTexImage2D(target, 0, (GLint) internal_format, width, height, 0, format, type, pixels);

        setPixelStore(0, &saved);
    }

    if (texture != 0) {
        struct surface_binding binding = {
            .context = ctx, .texture = (GLuint) texture, .target = target,
            .format = format, .type = type, .plane = plane, .surface = ioSurface,
            .seed = IOSurfaceGetSeed(ioSurface),
            .imported = imported, .internalFormat = internal_format,
            .width = width, .height = height,
            .shadow = shadow, .shadowFormat = shadowFormat,
        };
        if (!rememberSurfaceBinding(&binding)) {
            if (shadow != 0 && !shadowInUseByAny(ctx, shadow))
                glDeleteTextures(1, &shadow);
            error = kCGLBadAlloc;
        }
    }

    leaveContext(ctx, previous);
    return error;
}

// Set while this thread copies a texture into a surface.
static __thread int t_readingBack;

// Records that the texture of binding matches its surface at seed.
static void setBindingSeed(const struct surface_binding *binding, uint32_t seed) {
    pthread_mutex_lock(&g_surfaceBindingsMutex);
    for (size_t i = 0; i < g_surfaceBindingCount; i++) {
        if (g_surfaceBindings[i].context == binding->context
            && g_surfaceBindings[i].texture == binding->texture
            && g_surfaceBindings[i].surface == binding->surface)
            g_surfaceBindings[i].seed = seed;
    }
    pthread_mutex_unlock(&g_surfaceBindingsMutex);
}

// The most contexts whose work finishImportedSurface waits for at once.
#define MAX_FINISHED_CONTEXTS 16

// Waits for the GL work of the contexts that have textures using the
// surface's memory, so that the memory holds their rendering. A context
// current on another thread cannot be made current here and is skipped; its
// application flushes it before handing the surface on, as on macOS.
// Returns whether the surface has any such texture.
static int finishImportedSurface(IOSurfaceRef ioSurface) {
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
        CGLContextObj previous;
        if (enterContext(contexts[i], &previous) == kCGLNoError) {
            glFinish();
            leaveContext(contexts[i], previous);
        }
        CGLReleaseContext(contexts[i]);
    }
    return found;
}

// Brings GL rendering on ioSurface into the surface's memory. Textures that
// use the memory itself only need their contexts' work to finish. A texture
// that holds a copy, the one last specified from the surface, is copied into
// the surface, on the GPU through its shadow if it has one; with
// onlyIfUnmodified, not if the CPU has written the surface since the texture
// last matched it, as its contents are newer then.
static CGLError readBackSurface(IOSurfaceRef ioSurface, int onlyIfUnmodified) {
    int imported = finishImportedSurface(ioSurface);

    struct surface_binding binding = { 0 };
    pthread_mutex_lock(&g_surfaceBindingsMutex);
    for (size_t i = g_surfaceBindingCount; i-- > 0; ) {
        if (g_surfaceBindings[i].surface == ioSurface && !g_surfaceBindings[i].imported) {
            binding = g_surfaceBindings[i];
            break;
        }
    }
    if (binding.context && onlyIfUnmodified && IOSurfaceGetSeed(ioSurface) != binding.seed)
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

    CGLContextObj previous;
    CGLError error = enterContext(binding.context, &previous);
    if (error != kCGLNoError) {
        CGLReleaseContext(binding.context);
        return error;
    }

    if (binding.shadow != 0) {
        // The shadow is the surface's memory: the copy lands there once the
        // GPU is done, and a lock marks the surface modified, as the CPU
        // copy does. A texture respecified since is left alone.
        if (!glIsTexture(binding.texture) || !rectangleMatchesShadow(&binding)) {
            leaveContext(binding.context, previous);
            CGLReleaseContext(binding.context);
            return kCGLBadValue;
        }
        gpuCopy(binding.context, binding.texture, binding.target, binding.shadow, GL_TEXTURE_2D,
                binding.width, binding.height);
        glFinish();
        uint32_t seed = 0;
        t_readingBack = 1;
        IOSurfaceLock(ioSurface, 0, NULL);
        IOSurfaceUnlock(ioSurface, 0, &seed);
        t_readingBack = 0;
        setBindingSeed(&binding, seed);
        leaveContext(binding.context, previous);
        CGLReleaseContext(binding.context);
        return kCGLNoError;
    }

    GLenum bindingQuery = textureBindingQuery(binding.target);
    GLint oldTexture = 0, width = 0, height = 0;
    glGetIntegerv(bindingQuery, &oldTexture);
    glBindTexture(binding.target, binding.texture);
    glGetTexLevelParameteriv(binding.target, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(binding.target, 0, GL_TEXTURE_HEIGHT, &height);

    // The texture may have been respecified since; it must still fit.
    void *pixels;
    GLint rowLength;
    error = planeForTransfer(ioSurface, binding.plane, width, height, binding.format, binding.type,
                             &pixels, &rowLength);
    if (error == kCGLNoError) {
        struct pixel_store saved, transfer = { 0, rowLength, 0, 0, 1 };
        savePixelStore(1, &saved);
        setPixelStore(1, &transfer);
        uint32_t seed = 0;
        t_readingBack = 1;
        IOSurfaceLock(ioSurface, 0, NULL);
        glGetTexImage(binding.target, 0, binding.format, binding.type, pixels);
        IOSurfaceUnlock(ioSurface, 0, &seed);
        t_readingBack = 0;
        setPixelStore(1, &saved);
        setBindingSeed(&binding, seed);
    }

    glBindTexture(binding.target, (GLuint) oldTexture);
    leaveContext(binding.context, previous);
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
static void writeBackSurface(IOSurfaceRef ioSurface) {
    if (!t_readingBack)
        readBackSurface(ioSurface, 1);
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
        if (CGLGetCurrentContext() == context && atomic_load_explicit(&context->egl_surface, memory_order_acquire) != EGL_NO_SURFACE)
            success = p_eglSwapInterval(context->egl_display, context->paced ? 0 : v);
        if (success)
            context->swap_interval = v;
        return success ? kCGLNoError : kCGLBadValue;
    }
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

// No renderer information is available: the query reports no renderers
// instead of leaving the outputs unset and claiming success.
CGLError CGLDescribeRenderer(CGLRendererInfoObj rend, GLint rend_num, CGLRendererProperty prop, GLint *value) {
    if (!value)
        return kCGLBadAddress;
    *value = 0;
    return kCGLBadRendererInfo;
}

CGLError CGLQueryRendererInfo(GLuint display_mask, CGLRendererInfoObj *rend, GLint *nrend) {
    if (!rend || !nrend)
        return kCGLBadAddress;
    *rend = NULL;
    *nrend = 0;
    return kCGLBadRendererInfo;
}

CGLError CGLDestroyRendererInfo(CGLRendererInfoObj rend) {
    return kCGLNoError;
}

void CGLGetVersion(GLint *majorvers, GLint *minorvers) {
    if (majorvers)
        *majorvers = 1;
    if (minorvers)
        *minorvers = 0;
}
