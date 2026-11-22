// Copyright (C) 2026 Lubos Dolezel
//
// What the generated gl* exports (gl_exports.c) share with OpenGL.c.
#ifndef GL_EXPORTS_H
#define GL_EXPORTS_H

#include <stddef.h>

// The wrapper of the host's libGL, which holds the real functions (OpenGL.framework re-exports
// it). Never the public libGL.dylib in the framework's Libraries directory: that library
// re-exports this framework, so a lookup there would end in the framework's own entry points.
#define GL_HOST_LIBRARY "/usr/lib/native/libGL.dylib"

// The context whose host binding the calling thread holds, or NULL. Only the
// thread itself changes it (when it binds, or gives its binding up).
extern __thread void *gl_tls_bound;
#define GL_BOUND_HERE() (gl_tls_bound != NULL)

// gl_acquire_binding returns 2 when the binding was taken for a single call
// (the thread is not inside CGLLockContext on a context other threads use):
// the caller gives it up again after the call with gl_release_transient, so a
// thread that goes on to wait somewhere never keeps the others from the context.
void gl_release_transient(void);

// Makes the calling thread hold the host binding of its current context (the
// binding may have been given to another thread, or released). Returns 0 when
// the thread has no current context or could not get the binding: the call
// then does nothing, as OpenGL calls without a context do on macOS.
int gl_acquire_binding(void);  // 0: no binding, 1: held, 2: held for this call only

// The entry points of gen/generate-gl-trampolines.py: the host function of
// index, after the calling thread was given the binding of its current context
// (kept, as no call is left to give it up after).
void *gl_trampoline_target(unsigned index);
extern const char *const gl_trampoline_names[];
extern void *gl_trampoline_functions[];

extern void *gl_real_functions[];
void *gl_resolve_real(unsigned index);
#define GL_REAL(index) (gl_real_functions[index] != NULL ? gl_real_functions[index] : gl_resolve_real(index))

#endif
