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
#import <CloudKit/CKNotification.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, CKQueryNotificationReason) {
    CKQueryNotificationReasonRecordCreated = 1,
    CKQueryNotificationReasonRecordUpdated = 2,
    CKQueryNotificationReasonRecordDeleted = 3,
} NS_SWIFT_NAME(CKQueryNotification.Reason);

// Sent for a query subscription when a matching record changed.
@interface CKQueryNotification : CKNotification {
@private
    CKQueryNotificationReason _queryNotificationReason;
    NSDictionary<CKRecordFieldKey, id> *_recordFields;
    CKRecordID *_recordID;
    CKDatabaseScope _databaseScope;
}

@property (nonatomic, readonly, assign) CKQueryNotificationReason queryNotificationReason;
// Fields of the record that were requested through the subscription's desired keys.
@property (nonatomic, readonly, copy, nullable) NSDictionary<CKRecordFieldKey, id> *recordFields;
@property (nonatomic, readonly, copy, nullable) CKRecordID *recordID;
@property (nonatomic, readonly, assign) CKDatabaseScope databaseScope;

@end

NS_ASSUME_NONNULL_END
