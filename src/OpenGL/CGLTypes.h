#ifndef CGLTYPES_H
#define CGLTYPES_H
#include "CGLContext.h"

typedef CGLContext* CGLContextObj;
typedef struct CGLPixelFormat* CGLPixelFormatObj;

enum CGLError
{
	kCGLNoError = 0,
	kCGLBadAttribute = 10000,
	kCGLBadProperty = 10001,
	kCGLBadPixelFormat = 10002,
	kCGLBadRendererInfo = 10003,
	kCGLBadContext = 10004,
	kCGLBadDrawable = 10005,
	kCGLBadDisplay = 10006,
	kCGLBadState = 10007,
	kCGLBadValue = 10008,
	kCGLBadMatch = 10009,
	kCGLBadEnumeration = 10010,
	kCGLBadOffScreen = 10011,
	kCGLBadFullScreen = 10012,
	kCGLBadWindow = 10013,
	kCGLBadAddress = 10014,
	kCGLBadCodeModule = 10015,
	kCGLBadAlloc = 10016,
	kCGLBadConnection = 10017
};


#endif

