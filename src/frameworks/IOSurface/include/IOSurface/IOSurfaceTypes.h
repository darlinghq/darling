#ifndef IOSURFACE_TYPES_H
#define IOSURFACE_TYPES_H

#include <IOSurface/IOSurfaceBase.h>

#define kIOSurfaceSuccess KERN_SUCCESS

typedef uint32_t IOSurfaceID;

typedef CF_OPTIONS(uint32_t, IOSurfaceLockOptions)
{
	kIOSurfaceLockReadOnly = 1,
	kIOSurfaceLockAvoidSync = 2,
};

typedef CF_OPTIONS(uint32_t, IOSurfacePurgeabilityState)
{
	kIOSurfacePurgeableNonVolatile = 0,
	kIOSurfacePurgeableVolatile = 1,
	kIOSurfacePurgeableEmpty = 2,
	kIOSurfacePurgeableKeepCurrent = 3,
};

enum
{
	kIOSurfaceDefaultCache = 0,
	kIOSurfaceInhibitCache = 1,
	kIOSurfaceWriteThruCache = 2,
	kIOSurfaceCopybackCache = 3,
	kIOSurfaceWriteCombineCache = 4,
	kIOSurfaceCopybackInnerCache = 5,
};

enum
{
	kIOSurfaceMapCacheShift = 8,
	kIOSurfaceMapDefaultCache = kIOSurfaceDefaultCache << kIOSurfaceMapCacheShift,
	kIOSurfaceMapInhibitCache = kIOSurfaceInhibitCache << kIOSurfaceMapCacheShift,
	kIOSurfaceMapWriteThruCache = kIOSurfaceWriteThruCache << kIOSurfaceMapCacheShift,
	kIOSurfaceMapCopybackCache = kIOSurfaceCopybackCache << kIOSurfaceMapCacheShift,
	kIOSurfaceMapWriteCombineCache = kIOSurfaceWriteCombineCache << kIOSurfaceMapCacheShift,
	kIOSurfaceMapCopybackInnerCache = kIOSurfaceCopybackInnerCache << kIOSurfaceMapCacheShift,
};

#endif
