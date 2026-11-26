/*
 * This file is part of Darling.
 *
 * Copyright (C) 2025 Darling Developers
 * Copyright (C) 2026 Darling contributors
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _CBError_H_
#define _CBError_H_

#import <Foundation/Foundation.h>

extern NSString *const CBErrorDomain;
extern NSString *const CBATTErrorDomain;

typedef NS_ENUM(NSInteger, CBError) {
    CBErrorUnknown = 0,
    CBErrorInvalidParameters = 1,
    CBErrorNotConnected = 2,
    CBErrorOutOfSpace = 3,
    CBErrorTimeout = 4,
    CBErrorConnectionFailed = 5,
    CBErrorConnectionRejected = 6,
    CBErrorConnectionCancelled = 7,
    CBErrorDisconnected = 8,
    CBErrorOperationNotSupported = 9,
};

typedef NS_ENUM(NSInteger, CBATTError) {
    CBATTErrorSuccess = 0x00,
    CBATTErrorInvalidHandle = 0x01,
    CBATTErrorReadNotPermitted = 0x02,
    CBATTErrorWriteNotPermitted = 0x03,
    CBATTErrorInvalidPdu = 0x04,
    CBATTErrorInsufficientAuthentication = 0x05,
    CBATTErrorRequestNotSupported = 0x06,
    CBATTErrorInvalidOffset = 0x07,
    CBATTErrorInsufficientAuthorization = 0x08,
    CBATTErrorPrepareQueueFull = 0x09,
    CBATTErrorAttributeNotFound = 0x0a,
    CBATTErrorAttributeNotLong = 0x0b,
    CBATTErrorInsufficientEncryptionKeySize = 0x0c,
    CBATTErrorInvalidAttributeValueLength = 0x0d,
    CBATTErrorUnlikelyError = 0x0e,
    CBATTErrorInsufficientEncryption = 0x0f,
    CBATTErrorUnsupportedGroupType = 0x10,
    CBATTErrorInsufficientResources = 0x11,
};

#endif
