#import <Foundation/Foundation.h>

typedef struct {
    float x;
    float y;
} MTLSamplePosition;

typedef struct {
    NSUInteger width;
    NSUInteger height;
    NSUInteger depth;
} MTLSize;

MTLSize MTLSizeMake(NSUInteger width, NSUInteger height, NSUInteger depth);

typedef struct {
    NSUInteger x;
    NSUInteger y;
    NSUInteger z;
} MTLOrigin;

MTLOrigin MTLOriginMake(NSUInteger x, NSUInteger y, NSUInteger z);

typedef struct {
    MTLOrigin origin;
    MTLSize size;
} MTLRegion;

MTLRegion MTLRegionMake1D(NSUInteger x, NSUInteger width);
MTLRegion MTLRegionMake2D(NSUInteger x, NSUInteger y, NSUInteger width, NSUInteger height);
MTLRegion MTLRegionMake3D(NSUInteger x, NSUInteger y, NSUInteger z, NSUInteger width, NSUInteger height, NSUInteger depth);
