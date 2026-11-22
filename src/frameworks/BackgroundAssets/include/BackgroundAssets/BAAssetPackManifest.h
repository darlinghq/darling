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

#ifndef _BAASSETPACKMANIFEST_H_
#define _BAASSETPACKMANIFEST_H_

#import <Foundation/Foundation.h>
#import <BackgroundAssets/BAAssetPack.h>
#import <BackgroundAssets/BAContentRequest.h>
#import <BackgroundAssets/BADownload.h>

NS_ASSUME_NONNULL_BEGIN

API_AVAILABLE(macos(26.0))
@interface BAAssetPackManifest : NSObject

// GUESS: the documentation shows these two as returning a non-nullable instancetype but says only that an error is
// reported through the out-parameter; a nil result on failure is assumed.
- (nullable instancetype)initWithContentsOfURL:(NSURL *)URL
                    applicationGroupIdentifier:(NSString *)applicationGroupIdentifier
                                         error:(NSError **)error;
- (nullable instancetype)initFromData:(NSData *)data
           applicationGroupIdentifier:(NSString *)applicationGroupIdentifier
                                error:(NSError **)error;

- (NSSet<BADownload *> *)allDownloads;
- (NSSet<BADownload *> *)allDownloadsForContentRequest:(BAContentRequest)contentRequest;

@property (copy, readonly) NSSet<BAAssetPack *> *assetPacks;
- (nullable BAAssetPack *)assetPackWithIdentifier:(NSString *)assetPackIdentifier API_AVAILABLE(macos(27.0));

@property (copy, readonly) NSSet<BAAssetPack *> *localizedAssetPacks API_AVAILABLE(macos(27.0));
- (NSSet<BAAssetPack *> *)localizedAssetPacksForLanguage:(NSString *)languageIdentifier API_AVAILABLE(macos(27.0));

@property (copy, readonly, nullable) NSString *primaryLanguage API_AVAILABLE(macos(27.0));
@property (copy, readonly) NSArray<NSString *> *availableLanguages API_AVAILABLE(macos(27.0));
@property (copy, readonly, nullable) NSString *resolvedLanguage API_AVAILABLE(macos(27.0));

@end

NS_ASSUME_NONNULL_END

#endif
