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

#ifndef _QLTHUMBNAILERROR_H_
#define _QLTHUMBNAILERROR_H_

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

extern NSErrorDomain const QLThumbnailErrorDomain API_AVAILABLE(macos(10.15));

// The documentation lists the cases, in no stated order, and gives no numeric values.
// GUESS: values 0 to 5 in the order generationFailed, savingToURLFailed, requestInvalid,
// requestCancelled, noCachedThumbnail, noCloudThumbnail.
typedef NS_ERROR_ENUM(QLThumbnailErrorDomain, QLThumbnailError) {
    QLThumbnailErrorGenerationFailed = 0,
    QLThumbnailErrorSavingToURLFailed = 1,
    QLThumbnailErrorRequestInvalid = 2,
    QLThumbnailErrorRequestCancelled = 3,
    QLThumbnailErrorNoCachedThumbnail = 4,
    QLThumbnailErrorNoCloudThumbnail = 5,
} API_AVAILABLE(macos(10.15));

NS_ASSUME_NONNULL_END

#endif
