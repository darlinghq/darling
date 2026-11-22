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

#ifndef _BAAPPEXTENSIONINFO_H_
#define _BAAPPEXTENSIONINFO_H_

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

API_AVAILABLE(macos(13.0))
@interface BAAppExtensionInfo : NSObject <NSSecureCoding>

@property (strong, readonly, nullable) NSNumber *restrictedDownloadSizeRemaining;
@property (strong, readonly, nullable) NSNumber *restrictedEssentialDownloadSizeRemaining API_AVAILABLE(macos(13.3));

@end

NS_ASSUME_NONNULL_END

#endif
