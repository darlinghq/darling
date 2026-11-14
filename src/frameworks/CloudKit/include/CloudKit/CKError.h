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

NS_ASSUME_NONNULL_BEGIN

// Error domain of the CloudKit framework.
extern NSErrorDomain const CKErrorDomain API_AVAILABLE(macos(10.10));

typedef NS_ERROR_ENUM(CKErrorDomain, CKErrorCode) {
    CKErrorInternalError = 1,
    CKErrorPartialFailure = 2,
    CKErrorNetworkUnavailable = 3,
    CKErrorNetworkFailure = 4,
    CKErrorBadContainer = 5,
    CKErrorServiceUnavailable = 6,
    CKErrorRequestRateLimited = 7,
    CKErrorMissingEntitlement = 8,
    CKErrorNotAuthenticated = 9,
    CKErrorPermissionFailure = 10,
    CKErrorUnknownItem = 11,
    CKErrorInvalidArguments = 12,
    CKErrorResultsTruncated API_DEPRECATED("Will not be returned", macos(10.10, 10.12)) = 13,
    CKErrorServerRecordChanged = 14,
    CKErrorServerRejectedRequest = 15,
    CKErrorAssetFileNotFound = 16,
    CKErrorAssetFileModified = 17,
    CKErrorIncompatibleVersion = 18,
    CKErrorConstraintViolation = 19,
    CKErrorOperationCancelled = 20,
    CKErrorChangeTokenExpired = 21,
    CKErrorBatchRequestFailed = 22,
    CKErrorZoneBusy = 23,
    CKErrorBadDatabase = 24,
    CKErrorQuotaExceeded = 25,
    CKErrorZoneNotFound = 26,
    CKErrorLimitExceeded = 27,
    CKErrorUserDeletedZone = 28,
    CKErrorTooManyParticipants = 29,
    CKErrorAlreadyShared = 30,
    CKErrorReferenceViolation = 31,
    CKErrorManagedAccountRestricted = 32,
    CKErrorParticipantMayNeedVerification = 33,
    CKErrorServerResponseLost = 34,
    CKErrorAssetNotAvailable = 35,
    CKErrorAccountTemporarilyUnavailable = 36,
} API_AVAILABLE(macos(10.10));

// Keys of the userInfo dictionary of a CKErrorDomain error.
// For CKErrorPartialFailure: a dictionary of item identifier to the NSError of that item.
extern NSString * const CKPartialErrorsByItemIDKey API_AVAILABLE(macos(10.10));
// For CKErrorServerRecordChanged: the record the client tried to save, the record on the server and the
// last common ancestor.
extern NSString * const CKRecordChangedErrorClientRecordKey API_AVAILABLE(macos(10.10));
extern NSString * const CKRecordChangedErrorServerRecordKey API_AVAILABLE(macos(10.10));
extern NSString * const CKRecordChangedErrorAncestorRecordKey API_AVAILABLE(macos(10.10));
// Seconds (NSNumber) after which a failed request may be retried.
extern NSString * const CKErrorRetryAfterKey API_AVAILABLE(macos(10.10));
// Set when the user reset their encrypted data.
extern NSString * const CKErrorUserDidResetEncryptedDataKey API_AVAILABLE(macos(10.13));

NS_ASSUME_NONNULL_END
