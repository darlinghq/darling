#ifndef _DCAPPATTESTSERVICE_H_
#define _DCAPPATTESTSERVICE_H_

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface DCAppAttestService : NSObject
@property (class, readonly) DCAppAttestService *sharedService;
@property (readonly, getter=isSupported) BOOL supported;
@end

NS_ASSUME_NONNULL_END

#endif
