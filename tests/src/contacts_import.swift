// Copyright (C) 2026 Lubos Dolezel
//
// Typecheck of what the Swift Contacts overlay (apple/swift 5.2.2, stdlib/public/Darwin/Contacts/CNError.swift)
// needs from the Contacts Clang module: CNError as an imported NS_ERROR_ENUM (an error struct with a
// nested Code enum and userInfo) and the three userInfo key constants.
// Build: swiftc -typecheck contacts_import.swift -target x86_64-apple-macosx26.0 -sdk <SDK>

import Contacts
import Foundation

// The overlay's extension, spelled out.
extension CNError {
    var recordsFromUserInfo: [AnyObject]? {
        return userInfo[CNErrorUserInfoAffectedRecordsKey] as? [AnyObject]
    }
    var identifiersFromUserInfo: [String]? {
        return userInfo[CNErrorUserInfoAffectedRecordIdentifiersKey] as? [String]
    }
    var keyPathsFromUserInfo: [String]? {
        return userInfo[CNErrorUserInfoKeyPathsKey] as? [String]
    }
}

func use(_ error: Error) {
    if let cn = error as? CNError {
        switch cn.code {
        case .communicationError, .authorizationDenied, .recordDoesNotExist, .vCardMalformed:
            break
        default:
            break
        }
        _ = (cn.recordsFromUserInfo, cn.identifiersFromUserInfo, cn.keyPathsFromUserInfo)
    }
    let code: CNError.Code = .validationMultipleErrors
    _ = code.rawValue
    _ = CNError.errorDomain
}
