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

#import <BackgroundAssets/BAAssetPackManifest.h>
#import "BAPlaceholder.h"

// Placeholder: a manifest has no asset packs, no downloads and no languages. An instance made with alloc and init
// is that empty manifest, which is what BAAssetPackManager hands out. The initializers that take a manifest file
// or its data cannot produce a manifest, so they return nil with an error.
//
// GUESS: the error is NSFeatureUnsupportedError in NSCocoaErrorDomain; the documentation names no code.
@implementation BAAssetPackManifest

- (instancetype)initWithContentsOfURL:(NSURL *)URL
           applicationGroupIdentifier:(NSString *)applicationGroupIdentifier
                                error:(NSError **)error
{
    BA_STUB_CALLED();
    if (error) {
        *error = [NSError errorWithDomain:NSCocoaErrorDomain code:NSFeatureUnsupportedError userInfo:nil];
    }
    return nil;
}

- (instancetype)initFromData:(NSData *)data
  applicationGroupIdentifier:(NSString *)applicationGroupIdentifier
                       error:(NSError **)error
{
    BA_STUB_CALLED();
    if (error) {
        *error = [NSError errorWithDomain:NSCocoaErrorDomain code:NSFeatureUnsupportedError userInfo:nil];
    }
    return nil;
}

- (NSSet<BADownload *> *)allDownloads
{
    return [NSSet set];
}

- (NSSet<BADownload *> *)allDownloadsForContentRequest:(BAContentRequest)contentRequest
{
    return [NSSet set];
}

- (NSSet<BAAssetPack *> *)assetPacks
{
    return [NSSet set];
}

- (BAAssetPack *)assetPackWithIdentifier:(NSString *)assetPackIdentifier
{
    return nil;
}

- (NSSet<BAAssetPack *> *)localizedAssetPacks
{
    return [NSSet set];
}

- (NSSet<BAAssetPack *> *)localizedAssetPacksForLanguage:(NSString *)languageIdentifier
{
    return [NSSet set];
}

- (NSString *)primaryLanguage
{
    return nil;
}

- (NSArray<NSString *> *)availableLanguages
{
    return @[];
}

- (NSString *)resolvedLanguage
{
    return nil;
}

@end
