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
#import <CryptoTokenKit/TKSmartCardATR.h>

NS_ASSUME_NONNULL_BEGIN

@class TKSmartCard, TKSmartCardSlot;

typedef NS_ENUM(NSInteger, TKSmartCardSlotState) {
    TKSmartCardSlotStateMissing = 0,
    TKSmartCardSlotStateEmpty = 1,
    TKSmartCardSlotStateProbing = 2,
    TKSmartCardSlotStateMuteCard = 3,
    TKSmartCardSlotStateValidCard = 4,
} NS_SWIFT_NAME(TKSmartCardSlot.State) API_AVAILABLE(macos(10.10));

// A card reader slot.
API_AVAILABLE(macos(10.10))
@interface TKSmartCardSlot : NSObject

@property (nonatomic, readonly) TKSmartCardSlotState state;
// The ATR of the inserted card (nil unless the state is MuteCard or ValidCard).
@property (nonatomic, readonly, nullable) TKSmartCardATR *ATR;
@property (nonatomic, readonly, copy) NSString *name;
@property (nonatomic, readonly) NSInteger maxInputLength;
@property (nonatomic, readonly) NSInteger maxOutputLength;

// A card object for the inserted card; nil when the slot holds no valid card.
- (nullable TKSmartCard *)makeSmartCard;

@end

// Finds the card reader slots of the system.
API_AVAILABLE(macos(10.10))
@interface TKSmartCardSlotManager : NSObject

@property (class, nonatomic, readonly, nullable) TKSmartCardSlotManager *defaultManager;
@property (nonatomic, readonly, copy) NSArray<NSString *> *slotNames;

- (void)getSlotWithName:(NSString *)name reply:(void (^)(TKSmartCardSlot *_Nullable slot))reply;
- (nullable TKSmartCardSlot *)slotNamed:(NSString *)name;

@end

// A smart card in a slot. Commands are exchanged inside a session.
API_AVAILABLE(macos(10.10))
@interface TKSmartCard : NSObject

@property (nonatomic, readonly) TKSmartCardSlot *slot;
// NO once the card was removed or reset, after which the object is no longer usable.
@property (nonatomic, readonly, getter=isValid) BOOL valid;
@property (nonatomic) TKSmartCardProtocol allowedProtocols;
@property (nonatomic, readonly) TKSmartCardProtocol currentProtocol;
// Whether the card handles sensitive data; sensitive cards are reset when a session ends.
@property (nonatomic) BOOL sensitive;
// Arbitrary client object kept with the card.
@property (nonatomic, nullable, strong) id context;

// Instruction class byte used by the sendIns methods.
@property (nonatomic) UInt8 cla;
@property (nonatomic) BOOL useExtendedLength;
@property (nonatomic) BOOL useCommandChaining;

- (void)beginSessionWithReply:(void (^)(BOOL success, NSError *_Nullable error))reply;
- (void)endSession;

// Sends a raw APDU.
- (void)transmitRequest:(NSData *)request reply:(void (^)(NSData *_Nullable response, NSError *_Nullable error))reply;

// ISO 7816 command APDU helpers; the Swift overlay wraps them with optional-friendly versions.
- (void)sendIns:(UInt8)ins p1:(UInt8)p1 p2:(UInt8)p2 data:(nullable NSData *)requestData le:(nullable NSNumber *)le
          reply:(void (^)(NSData *_Nullable replyData, UInt16 sw, NSError *_Nullable error))reply NS_REFINED_FOR_SWIFT;

- (nullable NSData *)sendIns:(UInt8)ins p1:(UInt8)p1 p2:(UInt8)p2 data:(nullable NSData *)requestData le:(nullable NSNumber *)le
                          sw:(UInt16 *)sw error:(NSError **)error NS_REFINED_FOR_SWIFT API_AVAILABLE(macos(10.12));

// Runs the block inside a session, beginning one if needed; returns the block's result.
- (BOOL)inSessionWithError:(NSError **)error executeBlock:(BOOL (^)(NSError **error))block NS_REFINED_FOR_SWIFT
    API_AVAILABLE(macos(10.12));

@end

NS_ASSUME_NONNULL_END
