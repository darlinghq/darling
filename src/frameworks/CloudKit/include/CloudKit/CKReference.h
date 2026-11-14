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
#import <CloudKit/CKRecordValue.h>

NS_ASSUME_NONNULL_BEGIN

@class CKRecord;
@class CKRecordID;

typedef NS_ENUM(NSUInteger, CKReferenceAction) {
    CKReferenceActionNone = 0,
    CKReferenceActionDeleteSelf = 1,
} NS_SWIFT_NAME(CKRecord_Reference_Action);

// A link from one record to another.
NS_SWIFT_NAME(CKRecord.Reference)
@interface CKReference : NSObject <NSSecureCoding, NSCopying> {
@private
    CKRecordID *_recordID;
    CKReferenceAction _referenceAction;
}

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

- (instancetype)initWithRecordID:(CKRecordID *)recordID action:(CKReferenceAction)action;
- (instancetype)initWithRecord:(CKRecord *)record action:(CKReferenceAction)action;

@property (nonatomic, readonly, assign) CKReferenceAction referenceAction NS_SWIFT_NAME(action);
@property (nonatomic, readonly, copy) CKRecordID *recordID;

@end

@interface CKReference (CKRecordValue) <CKRecordValue>
@end

NS_ASSUME_NONNULL_END
