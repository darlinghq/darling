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

#ifndef _BADOWNLOAD_H_
#define _BADOWNLOAD_H_

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NSInteger BADownloaderPriority API_AVAILABLE(macos(13.0));

// The documentation names the three constants and says only that a priority lies between the minimum and the
// maximum. GUESS: 0, 1 and 2.
extern const BADownloaderPriority BADownloaderPriorityDefault API_AVAILABLE(macos(13.0));
extern const BADownloaderPriority BADownloaderPriorityMin API_AVAILABLE(macos(13.0));
extern const BADownloaderPriority BADownloaderPriorityMax API_AVAILABLE(macos(13.0));

// The documentation shows no numeric values.
// GUESS: values 0 to 4 in the order the documentation lists them.
typedef NS_ENUM(NSInteger, BADownloadState) {
    BADownloadStateCreated = 0,
    BADownloadStateWaiting = 1,
    BADownloadStateDownloading = 2,
    BADownloadStateFinished = 3,
    BADownloadStateFailed = 4,
};

API_AVAILABLE(macos(13.0))
@interface BADownload : NSObject <NSCoding, NSCopying, NSSecureCoding>

@property (nonatomic, readonly) NSString *identifier;
@property (nonatomic, readonly) NSString *uniqueIdentifier;
@property (readonly) BOOL isEssential API_AVAILABLE(macos(13.3));
@property (readonly) BADownloaderPriority priority;
@property (nonatomic, readonly) BADownloadState state;

- (instancetype)copyAsNonEssential API_AVAILABLE(macos(13.3));

@end

NS_ASSUME_NONNULL_END

#endif
