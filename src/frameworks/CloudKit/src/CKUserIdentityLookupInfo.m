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

#import "CKInternal.h"

@implementation CKUserIdentityLookupInfo

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)initWithEmailAddress:(NSString *)emailAddress
{
    if (emailAddress == nil)
        CKRaiseInvalid(@"CKException", @"emailAddress can not be nil");
    if ((self = [super init]))
        _emailAddress = [emailAddress copy];
    return self;
}

- (instancetype)initWithPhoneNumber:(NSString *)phoneNumber
{
    if (phoneNumber == nil)
        CKRaiseInvalid(@"CKException", @"phoneNumber can not be nil");
    if ((self = [super init]))
        _phoneNumber = [phoneNumber copy];
    return self;
}

- (instancetype)initWithUserRecordID:(CKRecordID *)userRecordID
{
    if (userRecordID == nil)
        CKRaiseInvalid(@"CKException", @"userRecordID can not be nil");
    if ((self = [super init]))
        _userRecordID = [userRecordID copy];
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    NSString *email = [coder decodeObjectOfClass:[NSString class] forKey:@"EmailAddress"];
    NSString *phone = [coder decodeObjectOfClass:[NSString class] forKey:@"PhoneNumber"];
    CKRecordID *recordID = [coder decodeObjectOfClass:[CKRecordID class] forKey:@"UserRecordID"];
    if (email != nil)
        return [self initWithEmailAddress:email];
    if (phone != nil)
        return [self initWithPhoneNumber:phone];
    if (recordID != nil)
        return [self initWithUserRecordID:recordID];
    return nil;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_emailAddress forKey:@"EmailAddress"];
    [coder encodeObject:_phoneNumber forKey:@"PhoneNumber"];
    [coder encodeObject:_userRecordID forKey:@"UserRecordID"];
}

- (id)copyWithZone:(NSZone *)zone
{
    // Immutable.
    return self;
}

- (NSString *)emailAddress
{
    return _emailAddress;
}

- (NSString *)phoneNumber
{
    return _phoneNumber;
}

- (CKRecordID *)userRecordID
{
    return _userRecordID;
}

+ (NSArray<CKUserIdentityLookupInfo *> *)lookupInfosWithEmails:(NSArray<NSString *> *)emails
{
    NSMutableArray *result = [NSMutableArray arrayWithCapacity:emails.count];
    for (NSString *email in emails)
        [result addObject:[[CKUserIdentityLookupInfo alloc] initWithEmailAddress:email]];
    return result;
}

+ (NSArray<CKUserIdentityLookupInfo *> *)lookupInfosWithPhoneNumbers:(NSArray<NSString *> *)phoneNumbers
{
    NSMutableArray *result = [NSMutableArray arrayWithCapacity:phoneNumbers.count];
    for (NSString *number in phoneNumbers)
        [result addObject:[[CKUserIdentityLookupInfo alloc] initWithPhoneNumber:number]];
    return result;
}

+ (NSArray<CKUserIdentityLookupInfo *> *)lookupInfosWithRecordIDs:(NSArray<CKRecordID *> *)recordIDs
{
    NSMutableArray *result = [NSMutableArray arrayWithCapacity:recordIDs.count];
    for (CKRecordID *recordID in recordIDs)
        [result addObject:[[CKUserIdentityLookupInfo alloc] initWithUserRecordID:recordID]];
    return result;
}

- (BOOL)isEqual:(id)object
{
    if (self == object)
        return YES;
    if (![object isKindOfClass:[CKUserIdentityLookupInfo class]])
        return NO;
    CKUserIdentityLookupInfo *other = object;
    return (_emailAddress == other->_emailAddress || [_emailAddress isEqual:other->_emailAddress])
        && (_phoneNumber == other->_phoneNumber || [_phoneNumber isEqual:other->_phoneNumber])
        && (_userRecordID == other->_userRecordID || [_userRecordID isEqual:other->_userRecordID]);
}

- (NSUInteger)hash
{
    return _emailAddress.hash ^ _phoneNumber.hash ^ _userRecordID.hash;
}

@end
