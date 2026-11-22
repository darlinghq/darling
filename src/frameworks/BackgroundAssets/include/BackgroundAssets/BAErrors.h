/*
 This file is part of Darling.

 Copyright (C) 2026 Darling developers

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

#ifndef _BAERRORS_H_
#define _BAERRORS_H_

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

extern NSString * const BAErrorDomain API_AVAILABLE(macos(14.0));

// The documentation lists the cases, alphabetically except for DownloadDoesNotExist, and shows no numeric values.
// GUESS: values 0 to 16 in the order the documentation lists them.
typedef NS_ENUM(NSInteger, BAErrorCode) {
    BAErrorCodeCallFromExtensionNotAllowed = 0,
    BAErrorCodeCallFromInactiveProcessNotAllowed = 1,
    BAErrorCodeCallerConnectionInvalid = 2,
    BAErrorCodeCallerConnectionNotAccepted = 3,
    BAErrorCodeDownloadAlreadyFailed = 4,
    BAErrorCodeDownloadAlreadyScheduled = 5,
    BAErrorCodeDownloadBackgroundActivityProhibited = 6,
    BAErrorCodeDownloadEssentialDownloadNotPermitted = 7,
    BAErrorCodeDownloadFailedToStart = 8,
    BAErrorCodeDownloadInvalid = 9,
    BAErrorCodeDownloadNotScheduled = 10,
    BAErrorCodeDownloadWouldExceedAllowance = 11,
    BAErrorCodeSessionDownloadAllowanceExceeded = 12,
    BAErrorCodeSessionDownloadDisallowedByAllowance = 13,
    BAErrorCodeSessionDownloadDisallowedByDomain = 14,
    BAErrorCodeSessionDownloadNotPermittedBeforeAppLaunch = 15,
    BAErrorCodeDownloadDoesNotExist = 16,
} API_AVAILABLE(macos(14.0));

extern NSErrorDomain const BAManagedErrorDomain API_AVAILABLE(macos(26.0));

// The documentation shows no numeric values.
// GUESS: values 0 to 2 in the order the documentation lists them.
typedef NS_ENUM(NSInteger, BAManagedErrorCode) {
    BAManagedErrorCodeAssetPackNotFound = 0,
    BAManagedErrorCodeFileNotFound = 1,
    BAManagedErrorCodeLocalAvailabilityFailure = 2,
} API_AVAILABLE(macos(26.0));

// userInfo key (an NSString) for the identifier of the asset pack in a BAManagedErrorCodeAssetPackNotFound error.
extern NSErrorUserInfoKey const BAAssetPackIdentifierErrorKey API_AVAILABLE(macos(26.0));

// userInfo keys of a BAManagedErrorCodeLocalAvailabilityFailure error: the asset packs that could not be made
// available, as an NSDictionary from BAAssetPack to NSError, and the ones that were, as an NSSet of BAAssetPack.
extern NSErrorUserInfoKey const BAFailuresErrorKey API_AVAILABLE(macos(27.0));
extern NSErrorUserInfoKey const BASuccessesErrorKey API_AVAILABLE(macos(27.0));

NS_ASSUME_NONNULL_END

#endif
