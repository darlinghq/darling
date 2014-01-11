#include "CGContext.h"
#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFString.h>
#include <QPainter>

static CFTypeID _kCGContextTypeID;
__attribute__((constructor)) void CGContextInitialize();

struct __CGContext
{
	CFRuntimeBase _parent;
	QPainter* painter;
};

CGContextRef CGContextForPaintDevice(QPaintDevice* paintDevice)
{
	CGContextRef context = (CGContextRef)_CFRuntimeCreateInstance (nullptr, _kCGContextTypeID,
		sizeof(struct __CGContext) - sizeof(CFRuntimeBase), nullptr);
	
	// TODO: mutual reference counting?
	context->painter = new QPainter(paintDevice);
	
	return context;
}

QPainter* CGContextGetPainter(CGContextRef ref)
{
	return ref->painter;
}

CGContextRef CGContextRetain(CGContextRef c)
{
	if (c)
		CFRetain(c);
	return c;
}

void CGContextRelease(CGContextRef c)
{
	if (c)
		CFRelease(c);
}

static CFHashCode CGContextHash (CFTypeRef cf)
{
	return CFHash (cf);
}

static CFStringRef CGContextCopyFormattingDesc (CFTypeRef cf, CFDictionaryRef formatOptions)
{
	return CFSTR("CGContext");
}

static Boolean CGContextEqual (CFTypeRef cf1, CFTypeRef cf2)
{
	return cf1 == cf2;
}

static void CGContextDealloc(CFTypeRef cf)
{
	CGContextRef ref = CGContextRef(cf);
	delete ref->painter;
}

void CGContextInitialize()
{
	static const CFRuntimeClass CGContextClass =
	{
		0,
		"CGContext",
		NULL,
		NULL,
		CGContextDealloc,
		NULL,
		CGContextHash,
		CGContextCopyFormattingDesc,
		NULL
	};
	_kCGContextTypeID = _CFRuntimeRegisterClass (&CGContextClass);
}

CFTypeID CGContextGetTypeID(void)
{
	return _kCGContextTypeID;
}

void CGContextSaveGState(CGContextRef c)
{
	c->painter->save();
}

void CGContextRestoreGState(CGContextRef c)
{
	c->painter->restore();
}

void CGContextFlush(CGContextRef c)
{
}
