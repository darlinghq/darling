#import <Foundation/Foundation.h>

#import <CryptoTokenKit/TKToken.h>

NS_ASSUME_NONNULL_BEGIN

@class TKTokenKeychainItem;
@class TKTokenKeychainCertificate;
@class TKTokenKeychainKey;
@class TKTokenKeychainState;

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@interface TKTokenKeychainItem : NSObject

- (instancetype)initWithObjectID:(TKTokenObjectID)objectID NS_DESIGNATED_INITIALIZER;
- (instancetype)init NS_UNAVAILABLE;

@property (readonly, copy) TKTokenObjectID objectID;

@property (nullable, copy) NSString *label;

@property (nullable, copy) NSDictionary<NSNumber *, TKTokenOperationConstraint> *constraints;

@end

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@interface TKTokenKeychainCertificate : TKTokenKeychainItem

- (nullable instancetype)initWithCertificate:(SecCertificateRef)certificateRef objectID:(TKTokenObjectID)objectID NS_DESIGNATED_INITIALIZER;
- (instancetype)initWithObjectID:(TKTokenObjectID)objectID NS_UNAVAILABLE;

@property (copy, readonly) NSData *data;

@end

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@interface TKTokenKeychainKey : TKTokenKeychainItem

- (nullable instancetype)initWithCertificate:(nullable SecCertificateRef)certificateRef objectID:(TKTokenObjectID)objectID NS_DESIGNATED_INITIALIZER;
- (instancetype)initWithObjectID:(TKTokenObjectID)objectID NS_UNAVAILABLE;

@property (copy) NSString *keyType;

@property (copy, nullable) NSData *applicationTag;

@property NSInteger keySizeInBits;

@property (copy, nullable) NSData *publicKeyData;

@property (copy, nullable) NSData *publicKeyHash;

@property BOOL canDecrypt;

@property BOOL canSign;

@property BOOL canPerformKeyExchange;

@property (getter=isSuitableForLogin) BOOL suitableForLogin;

@end

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@interface TKTokenKeychainContents : NSObject

- (void)fillWithItems:(NSArray<TKTokenKeychainItem *> *)items;

@property (readonly, copy) NSArray<TKTokenKeychainItem *> *items;

- (nullable TKTokenKeychainKey *)keyForObjectID:(TKTokenObjectID)objectID error:(NSError **)error;

- (nullable TKTokenKeychainCertificate *)certificateForObjectID:(TKTokenObjectID)objectID error:(NSError **)error;

- (instancetype)init NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END
