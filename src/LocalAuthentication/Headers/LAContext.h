#import <Foundation/Foundation.h>
#import <LocalAuthentication/LAPublicDefines.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, LAPolicy)
{
    LAPolicyDeviceOwnerAuthenticationWithBiometrics NS_ENUM_AVAILABLE(10_12, 8_0) __WATCHOS_AVAILABLE(3.0) __TVOS_AVAILABLE(10.0) = kLAPolicyDeviceOwnerAuthenticationWithBiometrics,

    LAPolicyDeviceOwnerAuthentication NS_ENUM_AVAILABLE(10_11, 9_0) = kLAPolicyDeviceOwnerAuthentication

} NS_ENUM_AVAILABLE(10_10, 8_0) __WATCHOS_AVAILABLE(3.0) __TVOS_AVAILABLE(10.0);

extern const NSTimeInterval LATouchIDAuthenticationMaximumAllowableReuseDuration NS_AVAILABLE(10_12, 9_0) __WATCHOS_UNAVAILABLE __TVOS_UNAVAILABLE;

NS_CLASS_AVAILABLE(10_10, 8_0) __WATCHOS_AVAILABLE(3.0) __TVOS_AVAILABLE(10.0)
@interface LAContext : NSObject

- (BOOL)canEvaluatePolicy:(LAPolicy)policy error:(NSError * __autoreleasing *)error __attribute__((swift_error(none)));

- (void)evaluatePolicy:(LAPolicy)policy
       localizedReason:(NSString *)localizedReason
                 reply:(void(^)(BOOL success, NSError * __nullable error))reply;

- (void)invalidate NS_AVAILABLE(10_11, 9_0);

typedef NS_ENUM(NSInteger, LACredentialType)
{
    LACredentialTypeApplicationPassword __TVOS_UNAVAILABLE = 0,
} NS_ENUM_AVAILABLE(10_11, 9_0) __WATCHOS_AVAILABLE(3.0) __TVOS_AVAILABLE(10.0);


- (BOOL)setCredential:(nullable NSData *)credential
                 type:(LACredentialType)type NS_AVAILABLE(10_11, 9_0) __WATCHOS_AVAILABLE(3.0) __TVOS_UNAVAILABLE;


- (BOOL)isCredentialSet:(LACredentialType)type NS_AVAILABLE(10_11, 9_0) __WATCHOS_AVAILABLE(3.0) __TVOS_UNAVAILABLE;

typedef NS_ENUM(NSInteger, LAAccessControlOperation)
{

    LAAccessControlOperationCreateItem,


    LAAccessControlOperationUseItem,


    LAAccessControlOperationCreateKey,


    LAAccessControlOperationUseKeySign,
    

    LAAccessControlOperationUseKeyDecrypt NS_ENUM_AVAILABLE(10_12, 10_0),


    LAAccessControlOperationUseKeyKeyExchange NS_ENUM_AVAILABLE(10_12, 10_0),
} NS_ENUM_AVAILABLE(10_11, 9_0) __WATCHOS_AVAILABLE(3.0) __TVOS_AVAILABLE(10.0);


- (void)evaluateAccessControl:(SecAccessControlRef)accessControl
                    operation:(LAAccessControlOperation)operation
              localizedReason:(NSString *)localizedReason
                        reply:(void(^)(BOOL success, NSError * __nullable error))reply
                        NS_AVAILABLE(10_11, 9_0) __WATCHOS_AVAILABLE(3.0) __TVOS_UNAVAILABLE;


@property (nonatomic, nullable, copy) NSString *localizedFallbackTitle;


@property (nonatomic, nullable, copy) NSString *localizedCancelTitle NS_AVAILABLE(10_12, 10_0);


@property (nonatomic, nullable) NSNumber *maxBiometryFailures NS_DEPRECATED_IOS(8_3, 9_0) __WATCHOS_UNAVAILABLE __TVOS_UNAVAILABLE;

@property (nonatomic, nullable, readonly) NSData *evaluatedPolicyDomainState NS_AVAILABLE(10_11, 9_0) __WATCHOS_UNAVAILABLE __TVOS_UNAVAILABLE;

@property (nonatomic) NSTimeInterval touchIDAuthenticationAllowableReuseDuration NS_AVAILABLE(10_12, 9_0) __WATCHOS_UNAVAILABLE __TVOS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END
