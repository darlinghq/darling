#import <Foundation/Foundation.h>

@interface AKRemoteDevice : NSObject

@property(readonly) NSString* machineId;
@property(readonly) NSString* serialNumber;

@end
