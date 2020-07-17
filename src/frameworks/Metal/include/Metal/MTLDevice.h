#import <Foundation/Foundation.h>
#import <Metal/MTLTypes.h>

typedef NS_ENUM(NSUInteger, MTLDeviceLocation) {
    MTLDeviceLocationBuiltIn        = 0,
    MTLDeviceLocationSlot           = 1,
    MTLDeviceLocationExternal       = 2,
    MTLDeviceLocationUnspecified    = NSUIntegerMax,
};

typedef NS_ENUM(NSInteger, MTLGPUFamily) {
    MTLGPUFamilyCommon1         = 3001,
    MTLGPUFamilyCommon2         = 3002,
    MTLGPUFamilyCommon3         = 3003,
    MTLGPUFamilyApple1          = 1001,
    MTLGPUFamilyApple2          = 1002,
    MTLGPUFamilyApple3          = 1003,
    MTLGPUFamilyApple4          = 1004,
    MTLGPUFamilyApple5          = 1005,
    MTLGPUFamilyApple6          = 1006,
    MTLGPUFamilyMac1            = 2001,
    MTLGPUFamilyMac2            = 2002,
    MTLGPUFamilyMacCatalyst1    = 4001,
    MTLGPUFamilyMacCatalyst2    = 4002,
};

typedef NS_ENUM(NSUInteger, MTLFeatureSet) {
    MTLFeatureSet_iOS_GPUFamily1_v1             = 0,
    MTLFeatureSet_iOS_GPUFamily1_v2             = 2,
    MTLFeatureSet_iOS_GPUFamily1_v3             = 5,
    MTLFeatureSet_iOS_GPUFamily1_v4             = 8,
    MTLFeatureSet_iOS_GPUFamily1_v5             = 12,
    MTLFeatureSet_iOS_GPUFamily2_v1             = 1,
    MTLFeatureSet_iOS_GPUFamily2_v2             = 3,
    MTLFeatureSet_iOS_GPUFamily2_v3             = 6,
    MTLFeatureSet_iOS_GPUFamily2_v4             = 9,
    MTLFeatureSet_iOS_GPUFamily2_v5             = 13,
    MTLFeatureSet_iOS_GPUFamily3_v1             = 4,
    MTLFeatureSet_iOS_GPUFamily3_v2             = 7,
    MTLFeatureSet_iOS_GPUFamily3_v3             = 10,
    MTLFeatureSet_iOS_GPUFamily3_v4             = 14,
    MTLFeatureSet_iOS_GPUFamily4_v1             = 11,
    MTLFeatureSet_iOS_GPUFamily4_v2             = 15,
    MTLFeatureSet_iOS_GPUFamily5_v1             = 16,

    MTLFeatureSet_macOS_GPUFamily1_v1           = 10000,
    MTLFeatureSet_macOS_GPUFamily1_v2           = 10001,
    MTLFeatureSet_macOS_GPUFamily1_v3           = 10003,
    MTLFeatureSet_macOS_GPUFamily1_v4           = 10004,
    MTLFeatureSet_macOS_GPUFamily2_v1           = 10005,
    MTLFeatureSet_macOS_ReadWriteTextureTier2   = 10002,

    MTLFeatureSet_tvOS_GPUFamily1_v1            = 30000,
    MTLFeatureSet_tvOS_GPUFamily1_v2            = 30001,
    MTLFeatureSet_tvOS_GPUFamily1_v3            = 30002,
    MTLFeatureSet_tvOS_GPUFamily1_v4            = 30004,
    MTLFeatureSet_tvOS_GPUFamily2_v1            = 30003,
    MTLFeatureSet_tvOS_GPUFamily2_v2            = 30005,
};


@protocol MTLDevice<NSObject>

@property(readonly) NSString *name;
@property(readonly, getter=isHeadless) BOOL headless;
@property(readonly, getter=isLowPower) BOOL lowPower;
@property(readonly, getter=isRemovable) BOOL removable;
@property(readonly) uint64_t registryID;
@property(readonly) MTLDeviceLocation location;
@property(readonly) NSUInteger locationNumber;
@property(readonly) uint64_t maxTransferRate;
@property(readonly) BOOL hasUnifiedMemory;
@property(readonly) uint64_t peerGroupID;
@property(readonly) uint32_t peerCount;
@property(readonly) uint32_t peerIndex;
@property(readonly) uint64_t recommendedMaxWorkingSetSize;
@property(readonly) NSUInteger currentAllocatedSize;
@property(readonly) NSUInteger maxThreadgroupMemoryLength;
@property(readonly) MTLSize maxThreadsPerThreadgroup;
@property(readonly, getter=areProgrammableSamplePositionsSupported) BOOL programmableSamplePositionsSupported;
@property(readonly, getter=areRasterOrderGroupsSupported) BOOL rasterOrderGroupsSupported;
@property(readonly, getter=isDepth24Stencil8PixelFormatSupported) BOOL depth24Stencil8PixelFormatSupported;
@property(readonly, getter=areBarycentricCoordsSupported) BOOL barycentricCoordsSupported;

- (BOOL)supportsFamily:(MTLGPUFamily)gpuFamily;
- (BOOL)supportsFeatureSet:(MTLFeatureSet)featureSet;
- (void)getDefaultSamplePositions:(MTLSamplePosition *)positions count:(NSUInteger)count;

@end
