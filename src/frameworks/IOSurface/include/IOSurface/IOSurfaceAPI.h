#ifndef IOSURFACE_API_H
#define IOSURFACE_API_H

#include <IOKit/IOKitLib.h>
#include <xpc/xpc.h>
#include <IOSurface/IOSurfaceBase.h>
#include <IOSurface/IOSurfaceTypes.h>
#include <IOSurface/IOSurfaceRef.h>

__BEGIN_DECLS

xpc_object_t IOSurfaceCreateXPCObject(IOSurfaceRef surface);
IOSurfaceRef IOSurfaceLookupFromXPCObject(xpc_object_t obj);

__END_DECLS

#endif
