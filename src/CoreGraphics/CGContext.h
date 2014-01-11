#ifndef CGCONTEXT_H_
#define CGCONTEXT_H_
#include "CGGeometry.h"
#include <CoreFoundation/CFBase.h>

BEGIN_EXTERN_C

struct __CGContext;
typedef struct __CGContext *CGContextRef;

CGContextRef CGContextRetain(CGContextRef c);
void CGContextRelease(CGContextRef c);
CFTypeID CGContextGetTypeID(void);

void CGContextSaveGState(CGContextRef c);
void CGContextRestoreGState(CGContextRef c);
void CGContextFlush(CGContextRef c);

END_EXTERN_C

#endif
