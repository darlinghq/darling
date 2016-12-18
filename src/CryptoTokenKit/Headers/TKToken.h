#import <Foundation/Foundation.h>
#import <Security/Security.h>

#import <CryptoTokenKit/TKSmartCard.h>

NS_ASSUME_NONNULL_BEGIN

@protocol TKTokenSessionDelegate;
@class TKTokenSession;
@class TKTokenKeyAlgorithm;
@class TKTokenKeyExchangeParameters;
@protocol TKTokenDelegate;
@class TKToken;
@protocol TKTokenDriverDelegate;
@class TKTokenDriver;
@class TKTokenKeychainContents;
@class TKTokenAuthOperation;

typedef id TKTokenObjectID
__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE;

typedef NS_ENUM(NSInteger, TKTokenOperation) {
    TKTokenOperationNone                = 0,

    TKTokenOperationReadData            = 1,

    TKTokenOperationSignData            = 2,

    TKTokenOperationDecryptData         = 3,

    TKTokenOperationPerformKeyExchange  = 4,
} __OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE;

typedef id TKTokenOperationConstraint
__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE;

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@interface TKTokenKeyAlgorithm : NSObject

- (instancetype)init NS_UNAVAILABLE;

- (BOOL)isAlgorithm:(SecKeyAlgorithm)algorithm;

- (BOOL)supportsAlgorithm:(SecKeyAlgorithm)algorithm;

@end

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@interface TKTokenKeyExchangeParameters : NSObject

@property (readonly) NSInteger requestedSize;

@property (readonly, nullable, copy) NSData *sharedInfo;

@end

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@interface TKTokenSession : NSObject

- (instancetype)initWithToken:(TKToken *)token NS_DESIGNATED_INITIALIZER;
@property (readonly) TKToken *token;
@property (weak, nullable) id<TKTokenSessionDelegate> delegate;

- (instancetype)init NS_UNAVAILABLE;

@end

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@protocol TKTokenSessionDelegate<NSObject>

@optional

- (nullable TKTokenAuthOperation *)tokenSession:(TKTokenSession *)session beginAuthForOperation:(TKTokenOperation)operation constraint:(TKTokenOperationConstraint)constraint error:(NSError **)error;

- (BOOL)tokenSession:(TKTokenSession *)session supportsOperation:(TKTokenOperation)operation usingKey:(TKTokenObjectID)keyObjectID algorithm:(TKTokenKeyAlgorithm *)algorithm;

- (nullable NSData *)tokenSession:(TKTokenSession *)session signData:(NSData *)dataToSign usingKey:(TKTokenObjectID)keyObjectID algorithm:(TKTokenKeyAlgorithm *)algorithm error:(NSError **)error;

- (nullable NSData *)tokenSession:(TKTokenSession *)session decryptData:(NSData *)ciphertext usingKey:(TKTokenObjectID)keyObjectID algorithm:(TKTokenKeyAlgorithm *)algorithm error:(NSError **)error;

- (nullable NSData *)tokenSession:(TKTokenSession *)session performKeyExchangeWithPublicKey:(NSData *)otherPartyPublicKeyData usingKey:(TKTokenObjectID)objectID algorithm:(TKTokenKeyAlgorithm *)algorithm parameters:(TKTokenKeyExchangeParameters *)parameters error:(NSError **)error;

@end

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@interface TKToken : NSObject

- (instancetype)initWithTokenDriver:(TKTokenDriver *)tokenDriver instanceID:(NSString *)instanceID NS_DESIGNATED_INITIALIZER;
@property (readonly) TKTokenDriver *tokenDriver;
@property (weak, nullable) id<TKTokenDelegate> delegate;

@property (nullable, readonly) TKTokenKeychainContents *keychainContents;

- (instancetype)init NS_UNAVAILABLE;

@end

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@protocol TKTokenDelegate<NSObject>

@required

- (nullable TKTokenSession *)token:(TKToken *)token createSessionWithError:(NSError **)error;

@optional

- (void)token:(TKToken *)token terminateSession:(TKTokenSession *)session;

@end

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@interface TKTokenDriver : NSObject

@property (weak, nullable) id<TKTokenDriverDelegate> delegate;

@end

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@protocol TKTokenDriverDelegate<NSObject>

@optional

- (void)tokenDriver:(TKTokenDriver *)driver terminateToken:(TKToken *)token;

@end

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@interface TKTokenAuthOperation : NSObject<NSSecureCoding>

- (BOOL)finishWithError:(NSError **)error;

@end

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@interface TKTokenPasswordAuthOperation : TKTokenAuthOperation

@property (nullable, copy) NSString *password;

@end

NS_ASSUME_NONNULL_END
