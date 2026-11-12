/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Renderer IDs, the values of kCGLRPRendererID.
 */
#ifndef _CGLRENDERERS_H
#define _CGLRENDERERS_H

enum {
    kCGLRendererGenericID = 0x00020200,
    kCGLRendererGenericFloatID = 0x00020400, // the software renderer
    kCGLRendererAppleSWID = 0x00020600,

    kCGLRendererATIRadeonX2000ID = 0x00021A00,
    kCGLRendererATIRadeonX3000ID = 0x00021B00,
    kCGLRendererATIRadeonX4000ID = 0x00021C00,

    kCGLRendererGeForce8xxxID = 0x00022600,
    kCGLRendererGeForceID = 0x00022700,

    kCGLRendererIntelHDID = 0x00024300,
    kCGLRendererIntelHD4000ID = 0x00024400,
    kCGLRendererIntelHD5000ID = 0x00024500
};

// The bits of a renderer ID that identify the kind of renderer.
#define kCGLRendererIDMatchingMask 0x00FE7F00

#endif /* _CGLRENDERERS_H */
