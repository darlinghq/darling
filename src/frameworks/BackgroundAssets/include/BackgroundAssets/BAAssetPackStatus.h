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

#ifndef _BAASSETPACKSTATUS_H_
#define _BAASSETPACKSTATUS_H_

#import <Foundation/Foundation.h>

// The documentation describes a status value that includes several of these cases and one that includes none, and
// the Swift counterpart of the type is an OptionSet, so the type is an option set, but the documentation shows
// no bit values.
// GUESS: bits 0 to 6 in the order the documentation lists the cases.
typedef NS_OPTIONS(NSUInteger, BAAssetPackStatus) {
    BAAssetPackStatusDownloadAvailable = 1 << 0,
    BAAssetPackStatusDownloading = 1 << 1,
    BAAssetPackStatusDownloaded = 1 << 2,
    BAAssetPackStatusUpToDate = 1 << 3,
    BAAssetPackStatusOutOfDate = 1 << 4,
    BAAssetPackStatusObsolete = 1 << 5,
    BAAssetPackStatusUpdateAvailable = 1 << 6,
} API_AVAILABLE(macos(26.0));

#endif
