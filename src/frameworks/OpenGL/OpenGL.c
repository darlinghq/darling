#include <stdlib.h>
#include <stdio.h>  // for fprintf(stderr, "unimplemented")

#include <pthread.h>
#include <stdatomic.h>
#include <dlfcn.h>

#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLInternal.h>
#include <CoreFoundation/CFDictionary.h>
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
    X(eglInitialize) \
    X(eglMakeCurrent) \
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
static int default_swap_interval = 1;
// Set while the registered display is Wayland, where the application's swap
// interval is kept in software (swap_pacing.h) and EGL stays at interval 0.
static int default_paced = 0;
// The EGL platform of the registered window-system display. The CoreGraphics
// connection belongs to the same window-system backend, so its native display
// is of this platform too. Until a backend registers, assume X11, the
// platform of the X11 backend's connection.
static EGLenum registered_platform = EGL_PLATFORM_X11_KHR;
static pthread_mutex_t registration_lock = PTHREAD_MUTEX_INITIALIZER;
static EGLContext default_share_context = EGL_NO_CONTEXT;

static EGLBoolean choose_config(EGLDisplay dpy, EGLint surface_type, EGLConfig *out_config, int *out_count) {
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

    EGLint count = 0;
    if (p_eglChooseConfig(dpy, attrs_24, out_config, 1, &count) && count > 0) {
        if (out_count) *out_count = count;
        return EGL_TRUE;
    }
    if (p_eglChooseConfig(dpy, attrs_16, out_config, 1, &count) && count > 0) {
        if (out_count) *out_count = count;
        return EGL_TRUE;
    }
    if (p_eglChooseConfig(dpy, attrs_fallback, out_config, 1, &count) && count > 0) {
        if (out_count) *out_count = count;
        return EGL_TRUE;
    }
    return EGL_FALSE;
}

static EGLBoolean choose_registered_config(EGLDisplay dpy, EGLConfig *out_config, int *out_count) {
    return choose_config(dpy, EGL_WINDOW_BIT | EGL_PBUFFER_BIT, out_config, out_count);
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
        if (!choose_config(candidates[i], EGL_PBUFFER_BIT, &candidate_config, NULL))
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
    // so we need to save the last set value. The default is 1.
    // This is the application's value; a paced context never gives it to EGL.
    int swap_interval;
    int paced;
    struct cgl_swap_pacer pacer;
    CGLPixelFormatObj pixel_format;
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
        return 1;
    default:
        return 0;
   }
}

__attribute__((constructor))
static void _CGLInitialize(void)
{
    g_displays = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
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

CGLError CGLRegisterNativeDisplay(void *native_display) {
    if (!native_display || !native_egl_available())
        return kCGLBadConnection;
    EGLDisplay candidate = p_eglGetPlatformDisplay(EGL_PLATFORM_X11_KHR, native_display, NULL);
    EGLConfig candidate_config;
    EGLint count;
    if (candidate == EGL_NO_DISPLAY || !p_eglInitialize(candidate, NULL, NULL))
        return kCGLBadConnection;
    if (!choose_registered_config(candidate, &candidate_config, &count))
        return kCGLBadPixelFormat;
    if (!p_eglBindAPI(EGL_OPENGL_API))
        return kCGLBadState;
    pthread_mutex_lock(&registration_lock);
    display = candidate;
    config = candidate_config;
    num_config = count;
    registered_platform = EGL_PLATFORM_X11_KHR;
    default_swap_interval = 1;
    default_paced = 0;
    pthread_mutex_unlock(&registration_lock);
    return kCGLNoError;
}

// Explicit-platform counterpart for backends whose native handles must not be
// interpreted using EGL's process default platform. Keep the X11 entry unchanged.
CGLError CGLRegisterNativeDisplayForPlatform(void *native_display, unsigned int platform) {
    if (!native_display || !native_egl_available())
        return kCGLBadConnection;
    EGLDisplay candidate = p_eglGetPlatformDisplay(platform, native_display, NULL);
    if (candidate == EGL_NO_DISPLAY)
        return kCGLBadConnection;
    EGLBoolean initialized = p_eglInitialize(candidate, NULL, NULL);
    if (!initialized)
        return kCGLBadConnection;
    CGLError error = kCGLBadConnection;
    const EGLint attributes[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };
    EGLint count = 0;
    if (!p_eglChooseConfig(candidate, attributes, NULL, 0, &count) || count <= 0) {
        error = kCGLBadPixelFormat;
        goto reject;
    }
    EGLConfig *configs = calloc((size_t)count, sizeof(*configs));
    if (!configs) {
        error = kCGLBadAlloc;
        goto reject;
    }
    EGLConfig candidate_config = NULL;
    if (p_eglChooseConfig(candidate, attributes, configs, count, &count)) {
        for (EGLint i = 0; i < count; ++i) {
            EGLint minimum_interval;
            if (p_eglGetConfigAttrib(candidate, configs[i], EGL_MIN_SWAP_INTERVAL, &minimum_interval) &&
                minimum_interval == 0) {
                candidate_config = configs[i];
                break;
            }
        }
    }
    free(configs);
    if (!candidate_config) {
        error = kCGLBadPixelFormat;
        goto reject;
    }
    if (!p_eglBindAPI(EGL_OPENGL_API)) {
        error = kCGLBadState;
        goto reject;
    }
    // Publish only a fully initialized display/config pair. A rejected platform
    // must not corrupt an already working backend.
    pthread_mutex_lock(&registration_lock);
    display = candidate;
    config = candidate_config;
    num_config = count;
    // Layer animations may keep drawing while their parent is hidden. Waiting
    // for a Wayland frame callback on an unmapped surface can block forever.
    registered_platform = platform;
    default_swap_interval = 0;
    default_paced = platform == EGL_PLATFORM_WAYLAND_KHR;
    pthread_mutex_unlock(&registration_lock);
    return kCGLNoError;

reject:
    // EGL may return an existing handle still owned by a live context, even
    // after another backend registration changed the default display.
    // A failed probe must not terminate that shared display.
    return error;
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
    EGLSurface surface;
};

static CGLWindowRef createWindow(EGLDisplay owner, EGLConfig owner_config, void *native_window) {
    if (!native_window || owner == EGL_NO_DISPLAY)
        return NULL;
    struct _CGLWindow *window = malloc(sizeof(*window));
    if (!window)
        return NULL;
    window->display = owner;
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

CGLError CGLSetCurrentContext(CGLContextObj context) {
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

CGLError CGLEnable(CGLContextObj ctx, CGLContextEnable pname) {
    return kCGLNoError;
}

CGLError CGLDisable(CGLContextObj ctx, CGLContextEnable pname) {
    return kCGLNoError;
}

CGLError CGLIsEnabled(CGLContextObj ctx, CGLContextEnable pname, GLint *enable) {
    if (enable) *enable = 0;
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

CGLError CGLCreateContext(CGLPixelFormatObj pixelFormat, CGLContextObj share, CGLContextObj *resultp) {

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
    if (owner == EGL_NO_DISPLAY) {
        owner = get_default_display(&owner_config);
        if (owner == EGL_NO_DISPLAY)
            return kCGLBadConnection;
    }
    if (!p_eglBindAPI(EGL_OPENGL_API))
        return kCGLBadState;
    EGLContext egl_share = EGL_NO_CONTEXT;
    if (share != NULL) {
        egl_share = share->egl_context;
    } else {
        pthread_mutex_lock(&registration_lock);
        egl_share = default_share_context;
        pthread_mutex_unlock(&registration_lock);
    }
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
        }
        /* Note: kCGLOGLPVersion_Legacy (0x1000) passes NULL attributes to request a compatibility context */
    }
    context_attribs[attr_idx] = EGL_NONE;
    EGLContext egl_context = p_eglCreateContext(owner, owner_config, egl_share, attr_idx > 0 ? context_attribs : NULL);
    if (egl_context == EGL_NO_CONTEXT && egl_share != EGL_NO_CONTEXT) {
        egl_context = p_eglCreateContext(owner, owner_config, EGL_NO_CONTEXT, attr_idx > 0 ? context_attribs : NULL);
    }
    if (egl_context == EGL_NO_CONTEXT && attr_idx > 0) {
        /* Core Profile creation failed on host driver; log warning and attempt best-effort fallback to compatibility context */
        fprintf(stderr, "CGLCreateContext: Core Profile request failed, falling back to compatibility context\n");
        egl_context = p_eglCreateContext(owner, owner_config, EGL_NO_CONTEXT, NULL);
    }

    if (egl_context == EGL_NO_CONTEXT) {
        return kCGLBadContext;
    }

    pthread_mutex_lock(&registration_lock);
    if (default_share_context == EGL_NO_CONTEXT && attr_idx == 0) {
        default_share_context = egl_context;
    }
    pthread_mutex_unlock(&registration_lock);

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

CGLContextObj CGLRetainContext(CGLContextObj context) {
    if (context == NULL) {
        return NULL;
    }

    __sync_fetch_and_add(&context->retain_count, 1);
    return context;
}

void CGLReleaseContext(CGLContextObj context) {
    if (context == NULL) {
        return;
    }

    if (__sync_sub_and_fetch(&context->retain_count, 1) != 0) {
        return;
    }

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

    pthread_mutex_lock(&registration_lock);
    if (default_share_context == context->egl_context) {
        default_share_context = EGL_NO_CONTEXT;
    }
    pthread_mutex_unlock(&registration_lock);

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

CGLError CGLSetDrawableRefreshRate(CGLContextObj context, uint32_t millihertz) {
    if (context == NULL)
        return kCGLBadContext;
    context->pacer.period_ns = cgl_period_ns_from_millihertz(millihertz);
    return kCGLNoError;
}

CGLError CGLSetParameter(CGLContextObj context, CGLContextParameter parameter, const GLint *value) {
    if (!value)
        return kCGLBadAddress;

    if (parameter == kCGLCPSwapInterval)
    {
        GLint v = *value;
        if (!context)
            return kCGLBadContext;
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

    if (parameter == kCGLCPSwapInterval)
    {
        *value = context->swap_interval;
        return kCGLNoError;
    }
    fprintf(stderr, "CGLGetParameter unimplemented for parameter %d\n", parameter);
    return kCGLNoError;
}

CGLError CGLDescribeRenderer(CGLRendererInfoObj rend, long rend_num, CGLRendererProperty prop, long *value) {
    return kCGLNoError;
}

CGLError CGLQueryRendererInfo(unsigned long display_mask, CGLRendererInfoObj *rend, long *nrend) {
    return kCGLNoError;
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
