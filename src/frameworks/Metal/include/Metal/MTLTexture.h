#import <Foundation/Foundation.h>

typedef NS_ENUM(NSUInteger, MTLTextureType) {
    MTLTextureType1D                    = 0,
    MTLTextureType1DArray               = 1,
    MTLTextureType2D                    = 2,
    MTLTextureType2DArray               = 3,
    MTLTextureType2DMultisample         = 4,
    MTLTextureTypeCube                  = 5,
    MTLTextureTypeCubeArray             = 6,
    MTLTextureType3D                    = 7,
    MTLTextureType2DMultisampleArray    = 8,
    MTLTextureTypeTextureBuffer         = 9
};
