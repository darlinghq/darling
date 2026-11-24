/* Copyright (C) 2026 Lubos Dolezel */
#ifndef _CGLCONTEXT_H
#define _CGLCONTEXT_H

#include <OpenGL/CGLTypes.h>
#include <OpenGL/gliContext.h>
#include <OpenGL/gliDispatch.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CGLPrivateObject *CGLPrivateObj;

/* What a CGLContextObj points to. The members up to "stak" are the public
   layout that code compiled with CGLMacro.h relies on (it calls the context's
   dispatch table directly); the framework may keep more state behind them. */
struct _CGLContextObject {
    GLIContext rend;
    GLIFunctionDispatch disp;
    CGLPrivateObj priv;
    void *stak;
};

#ifdef __cplusplus
}
#endif

#endif /* _CGLCONTEXT_H */
