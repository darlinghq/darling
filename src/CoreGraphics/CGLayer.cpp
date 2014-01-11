#include "CGLayer.h"
#include <QPicture>
#include <QPainter>
#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFString.h>

static CFTypeID _kCGLayerTypeID;
__attribute__((constructor)) void CGLayerInitialize();

// in CGContext.cpp
CGContextRef CGContextForPaintDevice(QPaintDevice* paintDevice);
QPainter* CGContextGetPainter(CGContextRef ref);

struct __CGLayer
{
	CFRuntimeBase _parent;
	QPicture* picture;
	CGContextRef context;
};

void CGContextDrawLayerAtPoint(CGContextRef context, CGPoint point, CGLayerRef layer)
{
	QPainter* painter = CGContextGetPainter(context);
	painter->drawPicture(QPointF(point.x, point.y), *layer->picture);
}

void CGContextDrawLayerInRect(CGContextRef context, CGRect rect, CGLayerRef layer)
{
	QPainter* painter = CGContextGetPainter(context);
	QTransform tf = painter->worldTransform();
	qreal sx, sy;
	CGSize origSize = CGLayerGetSize(layer);
	
	sx = CGRectGetWidth(rect) / origSize.width;
	sy = CGRectGetHeight(rect) / origSize.height;
	
	painter->scale(sx, sy);
	
	painter->drawPicture(QPointF(rect.origin.x / sx, rect.origin.y / sy), *layer->picture);
	
	painter->setWorldTransform(tf);
}

CGLayerRef CGLayerCreateWithContext(CGContextRef context, CGSize size, CFDictionaryRef auxiliaryInfo)
{
	CGLayerRef layer = (CGLayerRef)_CFRuntimeCreateInstance (nullptr, _kCGLayerTypeID,
		sizeof(struct __CGLayer) - sizeof(CFRuntimeBase), nullptr);
	
	layer->picture = new QPicture;
	layer->picture->setBoundingRect(QRect(0, 0, size.width, size.height));
	layer->context = CGContextForPaintDevice(layer->picture);
	
	// TODO: copy over settings from "context"
	
	return layer;
}

CGContextRef CGLayerGetContext(CGLayerRef layer)
{
	return layer->context;
}

CGSize CGLayerGetSize(CGLayerRef layer)
{
	QRect rect = layer->picture->boundingRect();
	return CGSize { CGFloat(rect.width()), CGFloat(rect.height()) };
}

static CFHashCode CGLayerHash (CFTypeRef cf)
{
	return CFHash (cf);
}

static CFStringRef CGLayerCopyFormattingDesc (CFTypeRef cf, CFDictionaryRef formatOptions)
{
	return CFSTR("CGLayer");
}

static Boolean CGLayerEqual (CFTypeRef cf1, CFTypeRef cf2)
{
	return cf1 == cf2;
}

static void CGLayerDealloc(CFTypeRef cf)
{
	CGLayerRef ref = CGLayerRef(cf);
	CFRelease(ref->context);
	delete ref->picture;
}

void CGLayerInitialize()
{
	static const CFRuntimeClass CGLayerClass =
	{
		0,
		"CGLayer",
		NULL,
		NULL,
		CGLayerDealloc,
		NULL,
		CGLayerHash,
		CGLayerCopyFormattingDesc,
		NULL
	};
	_kCGLayerTypeID = _CFRuntimeRegisterClass (&CGLayerClass);
}

CFTypeID CGLayerGetTypeID(void)
{
	return _kCGLayerTypeID;
}


void CGLayerRelease(CGLayerRef layer)
{
	if (layer)
		CFRelease(layer);
}

CGLayerRef CGLayerRetain(CGLayerRef layer)
{
	if (layer)
		CFRetain(layer);
	return layer;
}
