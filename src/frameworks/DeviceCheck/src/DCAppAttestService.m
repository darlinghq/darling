#import <Foundation/Foundation.h>

@interface DCAppAttestService : NSObject
+ (instancetype)sharedService;
@property (readonly, getter=isSupported) BOOL supported;
@end

@implementation DCAppAttestService

+ (instancetype)sharedService {
    return nil;
}

- (BOOL)isSupported {
    return NO;
}

@end
