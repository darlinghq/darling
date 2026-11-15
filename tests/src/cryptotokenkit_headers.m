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

// CryptoTokenKit headers: they parse as one umbrella, the enums have the documented values and the class
// graph and method signatures match the public API. Most classes have no implementation in Darling yet,
// so use_api() is only compiled, never called: it exercises every declared method with its documented
// argument types, which is the contract check (the same file compiles against the real SDK).
// Prints nothing on success; the exit code is the number of failed checks.

#import <Foundation/Foundation.h>
#import <CryptoTokenKit/CryptoTokenKit.h>
#import <objc/runtime.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

__attribute__((unused)) static void use_api(void)
{
    TKSmartCardSlotManager *manager = TKSmartCardSlotManager.defaultManager;
    NSArray<NSString *> *names = manager.slotNames;
    [manager getSlotWithName:names.firstObject reply:^(TKSmartCardSlot *_Nullable slot) { (void)slot; }];
    TKSmartCardSlot *slot = [manager slotNamed:@"reader"];
    TKSmartCardSlotState state = slot.state;
    TKSmartCardATR *atr = slot.ATR;
    NSString *name = slot.name;
    NSInteger maxIn = slot.maxInputLength, maxOut = slot.maxOutputLength;
    TKSmartCard *card = [slot makeSmartCard];
    (void)state; (void)name; (void)maxIn; (void)maxOut;

    BOOL valid = card.valid;
    card.allowedProtocols = TKSmartCardProtocolT0 | TKSmartCardProtocolT1;
    TKSmartCardProtocol current = card.currentProtocol;
    card.sensitive = YES;
    card.context = @"x";
    card.cla = 0x80;
    card.useExtendedLength = YES;
    card.useCommandChaining = YES;
    (void)valid; (void)current;
    [card beginSessionWithReply:^(BOOL success, NSError *error) { (void)success; (void)error; }];
    [card transmitRequest:[NSData data] reply:^(NSData *response, NSError *error) { (void)response; (void)error; }];
    [card sendIns:0xA4 p1:4 p2:0 data:nil le:@256 reply:^(NSData *replyData, UInt16 sw, NSError *error) {
        (void)replyData; (void)sw; (void)error;
    }];
    UInt16 sw = 0;
    NSError *error = nil;
    NSData *response = [card sendIns:0xA4 p1:4 p2:0 data:[NSData data] le:nil sw:&sw error:&error];
    BOOL ok = [card inSessionWithError:&error executeBlock:^BOOL(NSError **blockError) { (void)blockError; return YES; }];
    [card endSession];
    (void)response; (void)ok;

    NSData *bytes = atr.bytes, *historical = atr.historicalBytes;
    NSArray<TKCompactTLVRecord *> *records = atr.historicalRecords;
    NSArray<NSNumber *> *protocols = atr.protocols;
    TKSmartCardATR *parsed = [[TKSmartCardATR alloc] initWithBytes:bytes];
    TKSmartCardATR *streamed = [[TKSmartCardATR alloc] initWithSource:^int { return -1; }];
    TKSmartCardATRInterfaceGroup *group = [atr interfaceGroupAtIndex:1];
    TKSmartCardATRInterfaceGroup *byProtocol = [atr interfaceGroupForProtocol:TKSmartCardProtocolT1];
    NSNumber *ta = group.TA, *tb = group.TB, *tc = group.TC, *proto = byProtocol.protocol;
    (void)historical; (void)records; (void)protocols; (void)parsed; (void)streamed; (void)ta; (void)tb; (void)tc; (void)proto;

    TKTLVRecord *record = [TKTLVRecord recordFromData:bytes];
    NSArray<TKTLVRecord *> *sequence = [TKTLVRecord sequenceOfRecordsFromData:bytes];
    TKTLVTag tag = record.tag;
    NSData *value = record.value, *encoded = record.data;
    TKBERTLVRecord *ber = [[TKBERTLVRecord alloc] initWithTag:0x61 value:value];
    TKBERTLVRecord *berNested = [[TKBERTLVRecord alloc] initWithTag:0x61 records:sequence];
    NSData *tagBytes = [TKBERTLVRecord dataForTag:0x7f21];
    TKSimpleTLVRecord *simple = [[TKSimpleTLVRecord alloc] initWithTag:1 value:value];
    TKCompactTLVRecord *compact = [[TKCompactTLVRecord alloc] initWithTag:1 value:value];
    (void)tag; (void)encoded; (void)ber; (void)berNested; (void)tagBytes; (void)simple; (void)compact;

    TKTokenKeychainItem *item = [[TKTokenKeychainItem alloc] initWithObjectID:@"id"];
    item.label = @"label";
    item.constraints = @{@(TKTokenOperationSignData): @"constraint"};
    TKTokenObjectID objectID = item.objectID;
    (void)objectID;
}

int main(void)
{
    CHECK(TKSmartCardProtocolNone == 0 && TKSmartCardProtocolT0 == 1 && TKSmartCardProtocolT1 == 2 && TKSmartCardProtocolT15 == 0x8000);
    CHECK(TKSmartCardProtocolAny == 0xffff);
    CHECK(TKSmartCardSlotStateMissing == 0 && TKSmartCardSlotStateEmpty == 1 && TKSmartCardSlotStateProbing == 2);
    CHECK(TKSmartCardSlotStateMuteCard == 3 && TKSmartCardSlotStateValidCard == 4);
    CHECK(TKTokenOperationNone == 0 && TKTokenOperationReadData == 1 && TKTokenOperationSignData == 2);
    CHECK(TKTokenOperationDecryptData == 3 && TKTokenOperationPerformKeyExchange == 4);
    CHECK(class_getSuperclass([TKBERTLVRecord class]) == [TKTLVRecord class]);
    CHECK(class_getSuperclass([TKSmartCardToken class]) == [TKToken class]);
    return failures;
}
