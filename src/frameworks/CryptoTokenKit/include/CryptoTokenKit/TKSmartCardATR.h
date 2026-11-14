/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <Foundation/Foundation.h>
#import <CryptoTokenKit/TKTLVRecord.h>

NS_ASSUME_NONNULL_BEGIN

@class TKSmartCardATR;

// Transmission protocols a card can speak; a set of them is used for allowedProtocols.
typedef NS_OPTIONS(NSUInteger, TKSmartCardProtocol) {
    TKSmartCardProtocolNone = 0,
    TKSmartCardProtocolT0 NS_SWIFT_NAME(t0) = 1 << 0,
    TKSmartCardProtocolT1 NS_SWIFT_NAME(t1) = 1 << 1,
    TKSmartCardProtocolT15 NS_SWIFT_NAME(t15) = 1 << 15,
    TKSmartCardProtocolAny = 0xffff,
} API_AVAILABLE(macos(10.10));

// One interface-bytes group of an answer-to-reset: the TA/TB/TC bytes of the group and the protocol they announce.
API_AVAILABLE(macos(10.10))
@interface TKSmartCardATRInterfaceGroup : NSObject

@property (nonatomic, readonly, nullable) NSNumber *TA;
@property (nonatomic, readonly, nullable) NSNumber *TB;
@property (nonatomic, readonly, nullable) NSNumber *TC;
@property (nonatomic, readonly, nullable) NSNumber *protocol;

@end

// The ISO 7816 answer-to-reset a card sends when powered up.
API_AVAILABLE(macos(10.10))
@interface TKSmartCardATR : NSObject

// The whole ATR, and its historical bytes part.
@property (nonatomic, readonly) NSData *bytes;
@property (nonatomic, readonly) NSData *historicalBytes;

// The historical bytes parsed as compact-TLV records.
@property (nonatomic, readonly) NSArray<TKCompactTLVRecord *> *historicalRecords API_AVAILABLE(macos(10.12));

// Protocols announced by the card, as NSNumber-wrapped protocol numbers.
@property (nonatomic, readonly) NSArray<NSNumber *> *protocols;

- (nullable instancetype)initWithBytes:(NSData *)bytes;
- (nullable instancetype)initWithSource:(int (^)(void))source;

- (nullable TKSmartCardATRInterfaceGroup *)interfaceGroupAtIndex:(NSInteger)index;
- (nullable TKSmartCardATRInterfaceGroup *)interfaceGroupForProtocol:(TKSmartCardProtocol)protocol;

@end

NS_ASSUME_NONNULL_END
