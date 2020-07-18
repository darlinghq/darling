#import <Metal/MTLTypes.h>

MTLSize MTLSizeMake(NSUInteger width, NSUInteger height, NSUInteger depth) {
    return { width, height, depth };
}

MTLOrigin MTLOriginMake(NSUInteger x, NSUInteger y, NSUInteger z) {
    return {x, y, z};
}
