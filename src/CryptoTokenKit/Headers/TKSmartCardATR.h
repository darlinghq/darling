#import <Foundation/Foundation.h>

#import "TKTLVRecord.h"

NS_ASSUME_NONNULL_BEGIN

typedef NS_OPTIONS(NSUInteger, TKSmartCardProtocol) {
    TKSmartCardProtocolNone  = 0,
    TKSmartCardProtocolT0    = (1L << 0),
    TKSmartCardProtocolT1    = (1L << 1),
    TKSmartCardProtocolT15   = (1L << 15),
    TKSmartCardProtocolAny   = (1L << 16) - 1,
} NS_ENUM_AVAILABLE(10_10, 9_0);

NS_CLASS_AVAILABLE(10_10, 9_0)
@interface TKSmartCardATRInterfaceGroup : NSObject

@property (nonatomic, readonly, nullable) NSNumber *TA;

@property (nonatomic, readonly, nullable) NSNumber *TB;

@property (nonatomic, readonly, nullable) NSNumber *TC;

@property (nonatomic, readonly, nullable) NSNumber *protocol;

@end

NS_CLASS_AVAILABLE(10_10, 9_0)
@interface TKSmartCardATR : NSObject

- (nullable instancetype)initWithBytes:(NSData *)bytes;

- (nullable instancetype)initWithSource:(int(^)())source;

@property (nonatomic, readonly) NSData *bytes;

@property (nonatomic, readonly) NSArray<NSNumber *> *protocols;

- (nullable TKSmartCardATRInterfaceGroup *)interfaceGroupAtIndex:(NSInteger)index;

- (nullable TKSmartCardATRInterfaceGroup *)interfaceGroupForProtocol:(TKSmartCardProtocol)protocol;

@property (nonatomic, readonly) NSData *historicalBytes;

@property (nonatomic, readonly, nullable) NSArray<TKCompactTLVRecord *> *historicalRecords
__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END
