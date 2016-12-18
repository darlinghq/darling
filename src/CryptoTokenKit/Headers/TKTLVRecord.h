#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef UInt64 TKTLVTag
__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE;

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@interface TKTLVRecord : NSObject

@property (nonatomic, readonly) TKTLVTag tag;

@property (nonatomic, readonly) NSData *value;

@property (nonatomic, readonly) NSData *data;

+ (nullable instancetype)recordFromData:(NSData *)data;

+ (nullable NSArray<TKTLVRecord *> *)sequenceOfRecordsFromData:(NSData *)data;

- (instancetype)init NS_UNAVAILABLE;

@end


__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@interface TKBERTLVRecord : TKTLVRecord

+ (NSData *)dataForTag:(TKTLVTag)tag;

- (instancetype)initWithTag:(TKTLVTag)tag value:(NSData *)value;

- (instancetype)initWithTag:(TKTLVTag)tag records:(NSArray<TKTLVRecord *> *)records;

@end

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@interface TKSimpleTLVRecord : TKTLVRecord

- (instancetype)initWithTag:(UInt8)tag value:(NSData *)value;

@end

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE
@interface TKCompactTLVRecord : TKTLVRecord

- (instancetype)initWithTag:(UInt8)tag value:(NSData *)value;

@end

NS_ASSUME_NONNULL_END
