#import <Metal/MTLTypes.h>

MTLSize MTLSizeMake(NSUInteger width, NSUInteger height, NSUInteger depth) {
    MTLSize item = { width, height, depth };
    return item;
}

MTLOrigin MTLOriginMake(NSUInteger x, NSUInteger y, NSUInteger z) {
    MTLOrigin item = { x, y, z };
    return item;
}
