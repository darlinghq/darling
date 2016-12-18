#import <Foundation/Foundation.h>
#import <CryptoTokenKit/TKSmartCardATR.h>

NS_ASSUME_NONNULL_BEGIN

@class TKSmartCard;
@class TKSmartCardSlot;
@class TKSmartCardUserInteraction;

NS_CLASS_AVAILABLE(10_10, 9_0)
@interface TKSmartCardSlotManager : NSObject

@property (class, nullable, readonly) TKSmartCardSlotManager *defaultManager;

@property (readonly) NSArray<NSString *> *slotNames;

- (void)getSlotWithName:(NSString *)name reply:(void(^)(TKSmartCardSlot *__nullable slot))reply;

@end

typedef NS_ENUM(NSInteger, TKSmartCardSlotState) {

    TKSmartCardSlotStateMissing        = 0,


    TKSmartCardSlotStateEmpty          = 1,


    TKSmartCardSlotStateProbing        = 2,


    TKSmartCardSlotStateMuteCard       = 3,


    TKSmartCardSlotStateValidCard      = 4,
} NS_ENUM_AVAILABLE(10_10, 9_0);


typedef NS_ENUM(NSInteger, TKSmartCardPINCharset) {

    TKSmartCardPINCharsetNumeric             = 0,


    TKSmartCardPINCharsetAlphanumeric        = 1,


    TKSmartCardPINCharsetUpperAlphanumeric   = 2,
} NS_ENUM_AVAILABLE(10_11, 9_0);


typedef NS_ENUM(NSInteger, TKSmartCardPINEncoding) {

    TKSmartCardPINEncodingBinary    = 0,


    TKSmartCardPINEncodingASCII     = 1,


    TKSmartCardPINEncodingBCD       = 2,
} NS_ENUM_AVAILABLE(10_11, 9_0);


typedef NS_ENUM(NSInteger, TKSmartCardPINJustification) {

    TKSmartCardPINJustificationLeft     = 0,


    TKSmartCardPINJustificationRight    = 1,
} NS_ENUM_AVAILABLE(10_11, 9_0);


typedef NS_OPTIONS(NSUInteger, TKSmartCardPINCompletion) {

    TKSmartCardPINCompletionMaxLength  = (1L << 0),


    TKSmartCardPINCompletionKey        = (1L << 1),


    TKSmartCardPINCompletionTimeout    = (1L << 2),
} NS_ENUM_AVAILABLE(10_11, 9_0);


typedef NS_OPTIONS(NSUInteger, TKSmartCardPINConfirmation) {

    TKSmartCardPINConfirmationNone      = 0,


    TKSmartCardPINConfirmationNew       = (1L << 0),


    TKSmartCardPINConfirmationCurrent   = (1L << 1),
} NS_ENUM_AVAILABLE(10_11, 9_0);


NS_CLASS_AVAILABLE(10_11, 9_0)
@interface TKSmartCardPINFormat : NSObject


@property TKSmartCardPINCharset charset;

@property TKSmartCardPINEncoding encoding;


@property NSInteger minPINLength;


@property NSInteger maxPINLength;


@property NSInteger PINBlockByteLength;

@property TKSmartCardPINJustification PINJustification;

@property NSInteger PINBitOffset;

@property NSInteger PINLengthBitOffset;

@property NSInteger PINLengthBitSize;

@end


NS_CLASS_AVAILABLE(10_11, 9_0)
@protocol TKSmartCardUserInteractionDelegate
@optional

- (void)characterEnteredInUserInteraction:(TKSmartCardUserInteraction *)interaction;

- (void)correctionKeyPressedInUserInteraction:(TKSmartCardUserInteraction *)interaction;

- (void)validationKeyPressedInUserInteraction:(TKSmartCardUserInteraction *)interaction;

- (void)invalidCharacterEnteredInUserInteraction:(TKSmartCardUserInteraction *)interaction;

- (void)oldPINRequestedInUserInteraction:(TKSmartCardUserInteraction *)interaction;

- (void)newPINRequestedInUserInteraction:(TKSmartCardUserInteraction *)interaction;

- (void)newPINConfirmationRequestedInUserInteraction:(TKSmartCardUserInteraction *)interaction;

@end

NS_CLASS_AVAILABLE(10_11, 9_0)
@interface TKSmartCardUserInteraction : NSObject

@property (weak, nullable) id<TKSmartCardUserInteractionDelegate> delegate;

@property NSTimeInterval initialTimeout;

@property NSTimeInterval interactionTimeout;

- (void)runWithReply: (void(^)(BOOL success, NSError *__nullable error))reply;

- (BOOL)cancel;

@end

NS_CLASS_AVAILABLE(10_11, 9_0)
@interface TKSmartCardUserInteractionForPINOperation : TKSmartCardUserInteraction

@property TKSmartCardPINCompletion PINCompletion;

@property (nullable) NSArray<NSNumber *> *PINMessageIndices;

@property (null_resettable) NSLocale *locale;

@property UInt16 resultSW;

@property (nullable) NSData *resultData;

@end

NS_CLASS_AVAILABLE(10_11, 9_0)
@interface TKSmartCardUserInteractionForSecurePINVerification : TKSmartCardUserInteractionForPINOperation

@end

NS_CLASS_AVAILABLE(10_11, 9_0)
@interface TKSmartCardUserInteractionForSecurePINChange : TKSmartCardUserInteractionForPINOperation

@property TKSmartCardPINConfirmation PINConfirmation;

@end

NS_CLASS_AVAILABLE(10_10, 9_0)
@interface TKSmartCardSlot : NSObject

@property (readonly) TKSmartCardSlotState state;

@property (readonly, nullable) TKSmartCardATR *ATR;

@property (nonatomic, readonly) NSString *name;

@property (nonatomic, readonly) NSInteger maxInputLength;

@property (nonatomic, readonly) NSInteger maxOutputLength;

- (nullable TKSmartCard *)makeSmartCard;

@end

NS_CLASS_AVAILABLE(10_10, 9_0)
@interface TKSmartCard : NSObject

@property (nonatomic, readonly) TKSmartCardSlot *slot;

@property (readonly) BOOL valid;

@property TKSmartCardProtocol allowedProtocols;

@property (readonly) TKSmartCardProtocol currentProtocol;

@property BOOL sensitive;

@property (nullable) id context;

- (void)beginSessionWithReply:(void(^)(BOOL success, NSError *__nullable error))reply;

- (void)transmitRequest:(NSData *)request reply:(void(^)(NSData *__nullable response, NSError *__nullable error))reply;


- (void)endSession;

- (nullable TKSmartCardUserInteractionForSecurePINVerification *)userInteractionForSecurePINVerificationWithPINFormat:(TKSmartCardPINFormat *)PINFormat
                                                                                                                  APDU:(NSData *)APDU
                                                                                                         PINByteOffset:(NSInteger)PINByteOffset
                                                                                                                NS_AVAILABLE(10_11, 9_0);

- (nullable TKSmartCardUserInteractionForSecurePINChange *)userInteractionForSecurePINChangeWithPINFormat:(TKSmartCardPINFormat *)PINFormat
                                                                                                     APDU:(NSData *)APDU
                                                                                     currentPINByteOffset:(NSInteger)currentPINByteOffset
                                                                                         newPINByteOffset:(NSInteger)newPINByteOffset
                                                                                                NS_AVAILABLE(10_11, 9_0);

@end


@interface TKSmartCard (APDULevelTransmit)

@property UInt8 cla
__OSX_AVAILABLE(10.10) __IOS_AVAILABLE(9.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE;

@property BOOL useExtendedLength
__OSX_AVAILABLE(10.10) __IOS_AVAILABLE(9.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE;

@property BOOL useCommandChaining
__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE;

- (void)sendIns:(UInt8)ins p1:(UInt8)p1 p2:(UInt8)p2 data:(nullable NSData *)requestData le:(nullable NSNumber *)le
          reply:(void(^)(NSData *__nullable replyData, UInt16 sw, NSError *__nullable error))reply
__OSX_AVAILABLE(10.10) __IOS_AVAILABLE(9.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE;

- (BOOL)inSessionWithError:(NSError **)error executeBlock:(BOOL(^)(NSError **error))block
__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE;

- (nullable NSData *)sendIns:(UInt8)ins p1:(UInt8)p1 p2:(UInt8)p2 data:(nullable NSData *)requestData
                          le:(nullable NSNumber *)le sw:(UInt16 *)sw error:(NSError **)error
__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END
