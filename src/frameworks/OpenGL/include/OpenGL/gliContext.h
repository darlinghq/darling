/* Copyright (C) 2026 Lubos Dolezel */
#ifndef _GLICONTEXT_H
#define _GLICONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

/* A context's renderer, as an opaque handle (the "rend" member of its
   CGLContextObj). */
typedef struct __GLIContextRec *GLIContext;
typedef struct __GLISharedRec *GLIShared;

#ifdef __cplusplus
}
#endif

#endif /* _GLICONTEXT_H */
