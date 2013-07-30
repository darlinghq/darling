#include "CGLTypes.h"
#include "CGLContext.h"
#include "CGLContextImpl.h"

CGLError CGLLockContext(CGLContextObj context)
{
	if (!context)
		return kCGLBadContext;

	context->priv->lock();

	return kCGLNoError;
}

CGLError CGLUnlockContext(CGLContextObj context)
{
	if (!context)
		return kCGLBadContext;

	context->priv->unlock();

	return kCGLNoError;
}

