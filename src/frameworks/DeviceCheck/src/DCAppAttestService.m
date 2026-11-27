#import <DeviceCheck/DCAppAttestService.h>
#import <dispatch/dispatch.h>

@implementation DCAppAttestService

+ (DCAppAttestService *)sharedService {
    static DCAppAttestService *service = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        service = [[DCAppAttestService alloc] init];
    });
    return service;
}

- (BOOL)isSupported {
    return NO;
}

@end
