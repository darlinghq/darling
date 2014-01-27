#import <Foundation/NSObject.h>
#import <AppKit/AppKitDefines.h>

@class NSData, NSArray;



enum {
    NSUnknownColorSpaceModel = -1,
    NSGrayColorSpaceModel,
    NSRGBColorSpaceModel,
    NSCMYKColorSpaceModel,
    NSLABColorSpaceModel,
    NSDeviceNColorSpaceModel,
    NSIndexedColorSpaceModel,
    NSPatternColorSpaceModel
};
typedef NSInteger NSColorSpaceModel;



@interface NSColorSpace : NSObject <NSCoding> {
    id _profile;
    struct {
 unsigned int colorSpaceID:8;
 unsigned int storageType:3;
 unsigned int replacedDuringUnarchiving:1;
 unsigned int :20;
#if __LP64__
 unsigned int :32;
#endif
    } _flags;
    void *__cgColorSpace;
    void *__cgColorTransform;
    void *_reserved[4];
}

- (id)initWithICCProfileData:(NSData *)iccData;
- (NSData *)ICCProfileData;

- (id)initWithColorSyncProfile:(void * )prof;
- (void * )colorSyncProfile ;



- (id)initWithCGColorSpace:(CGColorSpaceRef)cgColorSpace;



- (CGColorSpaceRef)CGColorSpace;

- (NSInteger)numberOfColorComponents;
- (NSColorSpaceModel)colorSpaceModel;
- (NSString *)localizedName;

+ (NSColorSpace *)genericRGBColorSpace;
+ (NSColorSpace *)genericGrayColorSpace;
+ (NSColorSpace *)genericCMYKColorSpace;
+ (NSColorSpace *)deviceRGBColorSpace;
+ (NSColorSpace *)deviceGrayColorSpace;
+ (NSColorSpace *)deviceCMYKColorSpace;

+ (NSColorSpace *)sRGBColorSpace;
+ (NSColorSpace *)genericGamma22GrayColorSpace;

+ (NSColorSpace *)adobeRGB1998ColorSpace;



+ (NSArray *)availableColorSpacesWithModel:(NSColorSpaceModel)model;

@end
