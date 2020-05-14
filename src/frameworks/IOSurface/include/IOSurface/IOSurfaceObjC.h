#ifndef IOSURFACE_OBJC_H
#define IOSURFACE_OBJC_H

#ifdef __OBJC__

#import <IOSurface/IOSurfaceTypes.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSDictionary.h>

typedef NSString *IOSurfacePropertyKey;

__BEGIN_DECLS


extern IOSurfacePropertyKey IOSurfacePropertyKeyAllocSize;
extern IOSurfacePropertyKey IOSurfacePropertyKeyWidth;
extern IOSurfacePropertyKey IOSurfacePropertyKeyHeight;
extern IOSurfacePropertyKey IOSurfacePropertyKeyBytesPerRow;
extern IOSurfacePropertyKey IOSurfacePropertyKeyBytesPerElement;
extern IOSurfacePropertyKey IOSurfacePropertyKeyElementWidth;
extern IOSurfacePropertyKey IOSurfacePropertyKeyElementHeight;
extern IOSurfacePropertyKey IOSurfacePropertyKeyOffset;
extern IOSurfacePropertyKey IOSurfacePropertyKeyPlaneInfo;
extern IOSurfacePropertyKey IOSurfacePropertyKeyPlaneWidth;
extern IOSurfacePropertyKey IOSurfacePropertyKeyPlaneHeight;
extern IOSurfacePropertyKey IOSurfacePropertyKeyPlaneBytesPerRow;
extern IOSurfacePropertyKey IOSurfacePropertyKeyPlaneOffset;
extern IOSurfacePropertyKey IOSurfacePropertyKeyPlaneSize;
extern IOSurfacePropertyKey IOSurfacePropertyKeyPlaneBase;
extern IOSurfacePropertyKey IOSurfacePropertyKeyPlaneBytesPerElement;
extern IOSurfacePropertyKey IOSurfacePropertyKeyPlaneElementWidth;
extern IOSurfacePropertyKey IOSurfacePropertyKeyPlaneElementHeight;
extern IOSurfacePropertyKey IOSurfacePropertyKeyCacheMode;
extern IOSurfacePropertyKey IOSurfacePropertyKeyPixelFormat;
extern IOSurfacePropertyKey IOSurfacePropertyKeyPixelSizeCastingAllowed;


@interface IOSurface : NSObject <NSSecureCoding>
{
@package
    void * _impl;
}

- (nullable instancetype)initWithProperties:(NSDictionary <IOSurfacePropertyKey, id> *)properties;
- (nullable instancetype)initWithMachPort:(mach_port_t)port;
- (nullable instancetype)initWithSurfaceID:(IOSurfaceID)surfaceID;
- (kern_return_t)lockWithOptions:(IOSurfaceLockOptions)options seed:(nullable uint32_t *)seed;
- (kern_return_t)unlockWithOptions:(IOSurfaceLockOptions)options seed:(nullable uint32_t *)seed;


@property (readonly) NSInteger allocationSize;

@property (readonly) NSInteger width;
@property (readonly) NSInteger height;
@property (readonly) void *baseAddress;
@property (readonly) OSType pixelFormat;

@property (readonly) NSInteger bytesPerRow;
@property (readonly) NSInteger bytesPerElement;
@property (readonly) NSInteger elementWidth;
@property (readonly) NSInteger elementHeight;
@property (readonly) uint32_t seed;
@property (readonly) NSUInteger planeCount;

- (NSInteger)widthOfPlaneAtIndex:(NSUInteger)planeIndex;
- (NSInteger)heightOfPlaneAtIndex:(NSUInteger)planeIndex;
- (NSInteger)bytesPerRowOfPlaneAtIndex:(NSUInteger)planeIndex;
- (NSInteger)bytesPerElementOfPlaneAtIndex:(NSUInteger)planeIndex;
- (NSInteger)elementWidthOfPlaneAtIndex:(NSUInteger)planeIndex;
- (NSInteger)elementHeightOfPlaneAtIndex:(NSUInteger)planeIndex;
- (void *)baseAddressOfPlaneAtIndex:(NSUInteger)planeIndex;

- (void)setAttachment:(id)anObject forKey:(NSString *)key;
- (nullable id)attachmentForKey:(NSString *)key;
- (void)removeAttachmentForKey:(NSString *)key;
- (void)setAllAttachments:(NSDictionary<NSString *, id> *)dict;
- (nullable NSDictionary<NSString *, id> *)allAttachments;
- (void)removeAllAttachments;
@property (readonly, getter = isInUse) BOOL inUse;
- (void)incrementUseCount;
- (void)decrementUseCount;

@property (readonly ) int32_t localUseCount;
@property (readonly) BOOL allowsPixelSizeCasting;

- (kern_return_t)setPurgeable:(IOSurfacePurgeabilityState)newState oldState:(IOSurfacePurgeabilityState * _Nullable)oldState;

@end


extern IOSurfacePropertyKey IOSurfacePropertyAllocSizeKey;

__END_DECLS

#endif // __OBJC__
#endif
