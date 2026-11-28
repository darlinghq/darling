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

#ifndef _BAASSETPACK_H_
#define _BAASSETPACK_H_

#import <Foundation/Foundation.h>
#import <BackgroundAssets/BAContentRequest.h>
#import <BackgroundAssets/BADownload.h>

NS_ASSUME_NONNULL_BEGIN

API_AVAILABLE(macos(26.0))
@interface BAAssetPack : NSObject

@property (copy, readonly) NSString *identifier;
@property (assign, readonly) NSInteger version;
@property (copy, readonly, nullable) NSData *userInfo;
@property (copy, readonly, nullable) NSString *language API_AVAILABLE(macos(27.0));
@property (assign, readonly) NSInteger downloadSize;

- (BADownload *)download;
- (BADownload *)downloadForContentRequest:(BAContentRequest)contentRequest;

@end

NS_ASSUME_NONNULL_END

#endif
