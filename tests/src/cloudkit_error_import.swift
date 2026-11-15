// Copyright (C) 2026 Lubos Dolezel
//
// Typecheck of what the Swift CloudKit overlay's CKError.swift (apple/swift 5.2.2, stdlib/public/Darwin/CloudKit)
// needs from the CloudKit Clang module: CKError as an imported NS_ERROR_ENUM (CKErrorCode becomes CKError with a
// nested Code) and the error userInfo keys. The rest of the overlay (CKRecord.ID and the other nested types, record
// values, operations) needs the declarations of the later CloudKit header work.
// Build: swiftc -typecheck cloudkit_error_import.swift -target x86_64-apple-macosx26.0 -sdk <SDK>

import CloudKit
import Foundation

// The overlay's extension, spelled out.
extension CKError {
    var partialErrorsFromUserInfo: [AnyHashable: Error]? {
        return userInfo[CKPartialErrorsByItemIDKey] as? [AnyHashable: NSError] as [AnyHashable: Error]?
    }
    var ancestorFromUserInfo: CKRecord? {
        return userInfo[CKRecordChangedErrorAncestorRecordKey] as? CKRecord
    }
    var serverFromUserInfo: CKRecord? {
        return userInfo[CKRecordChangedErrorServerRecordKey] as? CKRecord
    }
    var clientFromUserInfo: CKRecord? {
        return userInfo[CKRecordChangedErrorClientRecordKey] as? CKRecord
    }
    var retryAfterFromUserInfo: Double? {
        return userInfo[CKErrorRetryAfterKey] as? Double
    }
}

func use(_ error: Error) {
    if let ck = error as? CKError {
        switch ck.code {
        case .internalError, .partialFailure, .networkUnavailable, .serverRecordChanged, .zoneNotFound,
             .accountTemporarilyUnavailable:
            break
        default:
            break
        }
        _ = (ck.partialErrorsFromUserInfo, ck.ancestorFromUserInfo, ck.serverFromUserInfo,
             ck.clientFromUserInfo, ck.retryAfterFromUserInfo)
    }
    let code: CKError.Code = .operationCancelled
    _ = (code.rawValue, CKError.errorDomain, CKErrorUserDidResetEncryptedDataKey)
}
