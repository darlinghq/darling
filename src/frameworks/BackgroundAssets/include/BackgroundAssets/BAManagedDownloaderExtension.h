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

#ifndef _BAMANAGEDDOWNLOADEREXTENSION_H_
#define _BAMANAGEDDOWNLOADEREXTENSION_H_

#import <Foundation/Foundation.h>
#import <BackgroundAssets/BAAssetPack.h>
#import <BackgroundAssets/BADownloaderExtension.h>

NS_ASSUME_NONNULL_BEGIN

API_AVAILABLE(macos(26.0))
@protocol BAManagedDownloaderExtension <BADownloaderExtension>

// The documentation says the method may be omitted.
@optional
- (BOOL)shouldDownloadAssetPack:(BAAssetPack *)assetPack;

@end

NS_ASSUME_NONNULL_END

#endif
