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

@implementation CKUserIdentity

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)_initWithUserRecordID:(CKRecordID *)userRecordID lookupInfo:(CKUserIdentityLookupInfo *)lookupInfo nameComponents:(NSPersonNameComponents *)nameComponents hasiCloudAccount:(BOOL)hasiCloudAccount
{
    if ((self = [super init])) {
        _userRecordID = [userRecordID copy];
        _lookupInfo = [lookupInfo copy];
        _nameComponents = [nameComponents copy];
        _hasiCloudAccount = hasiCloudAccount;
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    return [self _initWithUserRecordID:[coder decodeObjectOfClass:[CKRecordID class] forKey:@"UserRecordID"]
                            lookupInfo:[coder decodeObjectOfClass:[CKUserIdentityLookupInfo class] forKey:@"LookupInfo"]
                        nameComponents:[coder decodeObjectOfClass:[NSPersonNameComponents class] forKey:@"NameComponents"]
                      hasiCloudAccount:[coder decodeBoolForKey:@"HasiCloudAccount"]];
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_userRecordID forKey:@"UserRecordID"];
    [coder encodeObject:_lookupInfo forKey:@"LookupInfo"];
    [coder encodeObject:_nameComponents forKey:@"NameComponents"];
    [coder encodeBool:_hasiCloudAccount forKey:@"HasiCloudAccount"];
}

- (id)copyWithZone:(NSZone *)zone
{
    // Immutable.
    return self;
}

- (CKUserIdentityLookupInfo *)lookupInfo
{
    return _lookupInfo;
}

- (NSPersonNameComponents *)nameComponents
{
    return _nameComponents;
}

- (CKRecordID *)userRecordID
{
    return _userRecordID;
}

- (BOOL)hasiCloudAccount
{
    return _hasiCloudAccount;
}

- (BOOL)isEqual:(id)object
{
    if (self == object)
        return YES;
    if (![object isKindOfClass:[CKUserIdentity class]])
        return NO;
    CKUserIdentity *other = object;
    return _hasiCloudAccount == other->_hasiCloudAccount
        && (_userRecordID == other->_userRecordID || [_userRecordID isEqual:other->_userRecordID])
        && (_lookupInfo == other->_lookupInfo || [_lookupInfo isEqual:other->_lookupInfo]);
}

- (NSUInteger)hash
{
    return _userRecordID.hash ^ _lookupInfo.hash;
}

@end
