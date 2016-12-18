#import <CryptoTokenKit/TKToken.h>
#import <CryptoTokenKit/TKSmartCard.h>

NS_ASSUME_NONNULL_BEGIN

@class TKSmartCardTokenSession;
@class TKSmartCardToken;
@protocol TKSmartCardTokenDriverDelegate;
@class TKSmartCardTokenDriver;

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@interface TKTokenSmartCardPINAuthOperation : TKTokenAuthOperation

@property TKSmartCardPINFormat *PINFormat;

@property (nullable, copy) NSData *APDUTemplate;

@property NSInteger PINByteOffset;

@property (nullable) TKSmartCard *smartCard;

@property (nullable, copy) NSString *PIN;

@end

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@interface TKSmartCardTokenSession : TKTokenSession

@property (readonly) TKSmartCard *smartCard;

@end

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@interface TKSmartCardToken : TKToken

- (instancetype)initWithSmartCard:(TKSmartCard *)smartCard AID:(nullable NSData *)AID instanceID:(NSString *)instanceID tokenDriver:(TKSmartCardTokenDriver *)tokenDriver NS_DESIGNATED_INITIALIZER;

@property (readonly, nullable) NSData *AID;

- (instancetype)initWithTokenDriver:(TKTokenDriver *)tokenDriver instanceID:(NSString *)instanceID NS_UNAVAILABLE;

@end

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@interface TKSmartCardTokenDriver : TKTokenDriver

@end

@protocol TKSmartCardTokenDriverDelegate<TKTokenDriverDelegate>

- (nullable TKSmartCardToken *)tokenDriver:(TKSmartCardTokenDriver *)driver createTokenForSmartCard:(TKSmartCard *)smartCard AID:(nullable NSData *)AID error:(NSError **)error;

@end

NS_ASSUME_NONNULL_END
