#import <Foundation/Foundation.h>

@interface AKDeviceListRequestContext : NSObject

@property(copy) NSArray<NSString*>* services;
@property(copy) NSString* altDSID;

@end
