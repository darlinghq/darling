#ifndef _DCAPPATTESTSERVICE_H_
#define _DCAPPATTESTSERVICE_H_

#import <Foundation/Foundation.h>
#import <DeviceCheck/DCError.h>

NS_ASSUME_NONNULL_BEGIN

// App Attest needs hardware this system does not have: isSupported is NO, key
// generation fails with DCErrorFeatureUnsupported and no key is ever attested.
@interface DCAppAttestService : NSObject
@property (class, readonly) DCAppAttestService *sharedService;
@property (readonly, getter=isSupported) BOOL supported;
- (void)generateKeyWithCompletionHandler:(void (^)(NSString * _Nullable keyId, NSError * _Nullable error))completionHandler;
- (void)attestKey:(NSString *)keyId clientDataHash:(NSData *)clientDataHash completionHandler:(void (^)(NSData * _Nullable attestationObject, NSError * _Nullable error))completionHandler;
- (void)generateAssertion:(NSString *)keyId clientDataHash:(NSData *)clientDataHash completionHandler:(void (^)(NSData * _Nullable assertionObject, NSError * _Nullable error))completionHandler;
@end

NS_ASSUME_NONNULL_END

#endif
