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
