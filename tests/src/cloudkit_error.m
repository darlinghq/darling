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

// CloudKit errors: the CKErrorDomain string, the CKErrorCode values and the error userInfo keys.
// Prints PASS when everything holds; the exit code is the number of failed checks.

#import <Foundation/Foundation.h>
#import <CloudKit/CloudKit.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

int main(void)
{
    @autoreleasepool {
        CHECK([CKErrorDomain isEqualToString:@"CKErrorDomain"]);
        CHECK([CKPartialErrorsByItemIDKey isEqualToString:@"CKPartialErrors"]);
        CHECK([CKRecordChangedErrorClientRecordKey isEqualToString:@"ClientRecord"]);
        CHECK([CKRecordChangedErrorServerRecordKey isEqualToString:@"ServerRecord"]);
        CHECK([CKRecordChangedErrorAncestorRecordKey isEqualToString:@"AncestorRecord"]);
        CHECK([CKErrorRetryAfterKey isEqualToString:@"CKRetryAfter"]);
        CHECK([CKErrorUserDidResetEncryptedDataKey isEqualToString:@"CKUserDidResetEncryptedData"]);

        CHECK(sizeof(CKErrorCode) == sizeof(NSInteger));
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        CHECK(CKErrorResultsTruncated == 13);
#pragma clang diagnostic pop
        CHECK(CKErrorInternalError == 1);
        CHECK(CKErrorPartialFailure == 2);
        CHECK(CKErrorNetworkUnavailable == 3);
        CHECK(CKErrorNetworkFailure == 4);
        CHECK(CKErrorBadContainer == 5);
        CHECK(CKErrorServiceUnavailable == 6);
        CHECK(CKErrorRequestRateLimited == 7);
        CHECK(CKErrorMissingEntitlement == 8);
        CHECK(CKErrorNotAuthenticated == 9);
        CHECK(CKErrorPermissionFailure == 10);
        CHECK(CKErrorUnknownItem == 11);
        CHECK(CKErrorInvalidArguments == 12);
        CHECK(CKErrorServerRecordChanged == 14);
        CHECK(CKErrorServerRejectedRequest == 15);
        CHECK(CKErrorAssetFileNotFound == 16);
        CHECK(CKErrorAssetFileModified == 17);
        CHECK(CKErrorIncompatibleVersion == 18);
        CHECK(CKErrorConstraintViolation == 19);
        CHECK(CKErrorOperationCancelled == 20);
        CHECK(CKErrorChangeTokenExpired == 21);
        CHECK(CKErrorBatchRequestFailed == 22);
        CHECK(CKErrorZoneBusy == 23);
        CHECK(CKErrorBadDatabase == 24);
        CHECK(CKErrorQuotaExceeded == 25);
        CHECK(CKErrorZoneNotFound == 26);
        CHECK(CKErrorLimitExceeded == 27);
        CHECK(CKErrorUserDeletedZone == 28);
        CHECK(CKErrorTooManyParticipants == 29);
        CHECK(CKErrorAlreadyShared == 30);
        CHECK(CKErrorReferenceViolation == 31);
        CHECK(CKErrorManagedAccountRestricted == 32);
        CHECK(CKErrorParticipantMayNeedVerification == 33);
        CHECK(CKErrorServerResponseLost == 34);
        CHECK(CKErrorAssetNotAvailable == 35);
        CHECK(CKErrorAccountTemporarilyUnavailable == 36);

        NSError *e = [NSError errorWithDomain:CKErrorDomain
                                         code:CKErrorPartialFailure
                                     userInfo:@{CKErrorRetryAfterKey: @3.5}];
        CHECK(e.code == 2);
        CHECK([e.userInfo[CKErrorRetryAfterKey] doubleValue] == 3.5);
    }
    if (!failures) printf("PASS\n");
    return failures;
}
