#ifndef CGLAYER_H_
#define CGLAYER_H_
#include <CoreFoundation/CFDictionary.h>
#include "CGContext.h"
#include "CGGeometry.h"

BEGIN_EXTERN_C

struct __CGLayer;
typedef __CGLayer *CGLayerRef;

void CGContextDrawLayerAtPoint(CGContextRef context, CGPoint point, CGLayerRef layer);
void CGContextDrawLayerInRect(CGContextRef context, CGRect rect, CGLayerRef layer);
CGLayerRef CGLayerCreateWithContext(CGContextRef context, CGSize size, CFDictionaryRef auxiliaryInfo);
CGContextRef CGLayerGetContext(CGLayerRef layer);
CGSize CGLayerGetSize(CGLayerRef layer);
CFTypeID CGLayerGetTypeID(void);
void CGLayerRelease(CGLayerRef layer);
CGLayerRef CGLayerRetain(CGLayerRef layer);

END_EXTERN_C

#endif
