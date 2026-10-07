#include <stdlib.h>
#include <stdio.h>  // for fprintf(stderr, "unimplemented")

#include <pthread.h>

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

#define APPLE
#define __APPLE__

static EGLDisplay display;

static EGLConfig config;
static int num_config;
static int default_swap_interval = 1;
static pthread_mutex_t registration_lock = PTHREAD_MUTEX_INITIALIZER;

static EGLint const attribute_list[] = {
    EGL_RED_SIZE, 1,
    EGL_GREEN_SIZE, 1,
    EGL_BLUE_SIZE, 1,
    EGL_NONE
};

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
    EGLSurface egl_surface;
    // EGL has no function for getting the current swap interval,
    // so we need to save the last set value. The default is 1.
    int swap_interval;
};

struct _CGLPixelFormatObj {
    GLuint retain_count;
    CGLPixelFormatAttribute *attributes;
    EGLDisplay egl_display;
    EGLConfig egl_config;
    int swap_interval;
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
    EGLDisplay candidate = eglGetDisplay(native_display);
    EGLConfig candidate_config;
    EGLint count;
    if (candidate == EGL_NO_DISPLAY || !eglInitialize(candidate, NULL, NULL))
        return kCGLBadConnection;
    if (!eglChooseConfig(candidate, attribute_list, &candidate_config, 1, &count) || count == 0)
        return kCGLBadPixelFormat;
    if (!eglBindAPI(EGL_OPENGL_API))
        return kCGLBadState;
    pthread_mutex_lock(&registration_lock);
    display = candidate;
    config = candidate_config;
    num_config = count;
    default_swap_interval = 1;
    pthread_mutex_unlock(&registration_lock);
    return kCGLNoError;
}

// Explicit-platform counterpart for backends whose native handles must not be
// interpreted using EGL's process default platform. Keep the X11 entry unchanged.
CGLError CGLRegisterNativeDisplayForPlatform(void *native_display, unsigned int platform) {
    if (!native_display)
        return kCGLBadConnection;
    EGLDisplay candidate = eglGetPlatformDisplay(platform, native_display, NULL);
    if (candidate == EGL_NO_DISPLAY)
        return kCGLBadConnection;
    EGLBoolean initialized = eglInitialize(candidate, NULL, NULL);
    if (!initialized)
        return kCGLBadConnection;
    CGLError error = kCGLBadConnection;
    const EGLint attributes[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };
    EGLint count = 0;
    if (!eglChooseConfig(candidate, attributes, NULL, 0, &count) || count <= 0) {
        error = kCGLBadPixelFormat;
        goto reject;
    }
    EGLConfig *configs = calloc((size_t)count, sizeof(*configs));
    if (!configs) {
        error = kCGLBadAlloc;
        goto reject;
    }
    EGLConfig candidate_config = NULL;
    if (eglChooseConfig(candidate, attributes, configs, count, &count)) {
        for (EGLint i = 0; i < count; ++i) {
            EGLint minimum_interval;
            if (eglGetConfigAttrib(candidate, configs[i], EGL_MIN_SWAP_INTERVAL, &minimum_interval) &&
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
    if (!eglBindAPI(EGL_OPENGL_API)) {
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
    default_swap_interval = 0;
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

    pthread_mutex_lock(&g_displaysMutex);
    rv = (struct _CGLDisplay*) CFDictionaryGetValue(g_displays, (const void*)(unsigned long) cid);
    pthread_mutex_unlock(&g_displaysMutex);

    if (!rv)
    {
        EGLDisplay disp = eglGetDisplay(_CGSNativeDisplay(cid));
        if (disp == EGL_NO_DISPLAY)
            return NULL;

        rv = (struct _CGLDisplay*) malloc(sizeof(*rv));
        rv->display = disp;

        eglInitialize(rv->display, NULL, NULL);
        eglChooseConfig(rv->display, attribute_list, &rv->config, 1, &rv->num_config);

        eglBindAPI(EGL_OPENGL_API);

        pthread_mutex_lock(&g_displaysMutex);
        CFDictionaryAddValue(g_displays, (const void*)(unsigned long) cid, rv);
        pthread_mutex_unlock(&g_displaysMutex);
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

    gl->egl_surface = eglCreateWindowSurface(gl->egl_display, gl->egl_config, window, NULL);
    if (gl->egl_surface == EGL_NO_SURFACE)
        return kCGLBadState;
    return kCGLNoError;
}

// CGLWindowRef is opaque: retain the display that owns its EGL surface.
struct _CGLWindow {
    EGLDisplay display;
    EGLSurface surface;
};

static CGLWindowRef createWindow(EGLDisplay owner, EGLConfig owner_config, void *native_window) {
    struct _CGLWindow *window = malloc(sizeof(*window));
    if (!window)
        return NULL;
    window->display = owner;
    window->surface = eglCreateWindowSurface(owner, owner_config,
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
    eglDestroySurface(window->display, window->surface);
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
    EGLSurface previous_surface = context->egl_surface;
    context->egl_surface = drawable->surface;
    CGLError error = CGLSetCurrentContext(context);
    if (error != kCGLNoError)
        context->egl_surface = previous_surface;
    return error;
}

static pthread_key_t current_context_key;

static void make_key() {
    pthread_key_create(&current_context_key, NULL);
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
    if (!eglBindAPI(EGL_OPENGL_API))
        return kCGLBadState;
    if (context != NULL) {
        EGLSurface surface = context->egl_surface;
        if (!eglMakeCurrent(context->egl_display, surface, surface, context->egl_context)) {
            return kCGLBadContext;
        }
        pthread_setspecific(get_current_context_key(), context);
        if (surface != EGL_NO_SURFACE && !eglSwapInterval(context->egl_display, context->swap_interval))
            return kCGLBadValue;
    } else {
        CGLContextObj current = CGLGetCurrentContext();
        if (!current)
            return kCGLNoError;
        if (!eglMakeCurrent(current->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
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
    CGLPixelFormatObj format = malloc(sizeof(struct _CGLPixelFormatObj));
    int count = attributes_count(attrs);

    pthread_mutex_lock(&registration_lock);
    format->egl_display = display;
    format->egl_config = config;
    format->swap_interval = default_swap_interval;
    pthread_mutex_unlock(&registration_lock);
    format->retain_count = 1;
    format->attributes = malloc(sizeof(CGLPixelFormatAttribute) * count);
    for (int i = 0; i < count; i++) {
        format->attributes[i] = attrs[i];
    }

    *result = format;
    *number_of_screens = 1;

    return kCGLNoError;
}

CGLError CGLClearDrawable(CGLContextObj ctx)
{
    printf("STUB: CGLClearDrawable\n");

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

    format->retain_count++;
    return format;
}

void CGLReleasePixelFormat(CGLPixelFormatObj format) {
    if (format == NULL) {
        return;
    }

    format->retain_count--;

    if (format->retain_count == 0) {
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

    pthread_mutex_lock(&registration_lock);
    EGLDisplay owner = share ? share->egl_display : (pixelFormat ? pixelFormat->egl_display : display);
    EGLConfig owner_config = share ? share->egl_config : (pixelFormat ? pixelFormat->egl_config : config);
    int interval = share ? share->swap_interval : (pixelFormat ? pixelFormat->swap_interval : default_swap_interval);
    pthread_mutex_unlock(&registration_lock);
    if (!eglBindAPI(EGL_OPENGL_API))
        return kCGLBadState;
    EGLContext egl_share = EGL_NO_CONTEXT;
    if (share != NULL) {
        egl_share = share->egl_context;
    }
    EGLContext egl_context = eglCreateContext(owner, owner_config, egl_share, NULL);

    if (egl_context == EGL_NO_CONTEXT) {
        return kCGLBadContext;
    }

    CGLContextObj context = malloc(sizeof(struct _CGLContextObj));

    if (context == NULL) {
        eglDestroyContext(owner, egl_context);
        return kCGLBadAlloc;
    }

    context->retain_count = 1;
    pthread_mutex_init(&(context->lock), NULL);
    context->egl_display = owner;
    context->egl_config = owner_config;
    context->egl_context = egl_context;
    context->egl_surface = NULL;
    context->swap_interval = interval;

    *resultp = context;

    return kCGLNoError;
}

CGLContextObj CGLRetainContext(CGLContextObj context) {
    if (context == NULL) {
        return NULL;
    }

    context->retain_count++;
    return context;
}

void CGLReleaseContext(CGLContextObj context) {
    if (context == NULL) {
        return;
    }

    if (CGLGetCurrentContext() == context) {
        // Do not free a context while EGL/TLS still identify it as current.
        // A failed unbind leaves ownership with the caller for a later retry.
        if (CGLSetCurrentContext(NULL) != kCGLNoError)
            return;
    }

    context->retain_count--;

    if (context->retain_count != 0) {
        return;
    }

    pthread_mutex_destroy(&(context->lock));

    eglDestroyContext(context->egl_display, context->egl_context);

    free(context);
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
    pthread_mutex_lock(&(context->lock));
    return kCGLNoError;
}

CGLError CGLUnlockContext(CGLContextObj context) {
    pthread_mutex_unlock(&(context->lock));
    return kCGLNoError;
}

CGLError CGLFlushDrawable(CGLContextObj context) {
    if (context == NULL)
        return kCGLBadContext;
    if (context->egl_surface == EGL_NO_SURFACE)
        return kCGLBadDrawable;
    return eglSwapBuffers(context->egl_display, context->egl_surface) ? kCGLNoError : kCGLBadDrawable;
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
        if (CGLGetCurrentContext() == context && context->egl_surface != EGL_NO_SURFACE)
            success = eglSwapInterval(context->egl_display, v);
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
