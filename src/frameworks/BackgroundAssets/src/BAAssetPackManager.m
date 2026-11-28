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

#import <BackgroundAssets/BAAssetPackManager.h>
#import "BAPlaceholder.h"

// Placeholder: no asset pack exists, so nothing is ever downloaded, found, updated or removed, and nothing touches
// the network or the file system. A request that names an asset pack gets the documented "not found" result where
// the documentation gives one: BAManagedErrorCodeAssetPackNotFound with BAAssetPackIdentifierErrorKey for an
// identifier that is not known, and BAManagedErrorCodeFileNotFound with NSFilePathErrorKey for a file that is not
// there. Each completion handler is called exactly once, asynchronously.
//
// GUESS: the documentation does not say on which queue the handlers of this class run; a global queue is used,
// which is what it says for the handlers of BADownloadManager (any queue).
// GUESS: the manifest and the list of all asset packs are empty and succeed, since the documentation gives no error
// for having no asset packs; checkForUpdates reports nothing updating and nothing removed.
// GUESS: removeAssetPackWithIdentifier:, getStatusRelativeToAssetPack: and the single-pack ensureLocalAvailability
// methods report BAManagedErrorCodeAssetPackNotFound, which the documentation states for the other lookups but not
// for these. The size delivered with the not-found error of getLocalSizeOfAssetPackWithIdentifier: is -1, as
// the documentation gives -1 for the failed results of the sibling methods that return a number.
// GUESS: an empty set of asset packs is trivially available, and reconciling the languages has nothing to
// download or remove, so both succeed. A non-empty set fails as a whole with
// BAManagedErrorCodeLocalAvailabilityFailure, an empty BASuccessesErrorKey set and a not-found error for each
// pack under BAFailuresErrorKey.
// GUESS: URLForPath:error: returns a file URL for the path under a directory that does not exist, since the
// documentation says a URL comes back even when no item exists and gives no root. A path with a ".." component
// could leave that directory, so it fails with BAManagedErrorCodeFileNotFound instead.
// GUESS: resolvedLanguage only stores what is set; the documentation says setting it also changes the app's
// display language, which is not done.

static NSError *BAAssetPackNotFoundError(NSString *identifier)
{
    NSDictionary *info = identifier ? @{BAAssetPackIdentifierErrorKey: identifier} : nil;
    return [NSError errorWithDomain:BAManagedErrorDomain code:BAManagedErrorCodeAssetPackNotFound userInfo:info];
}

static NSError *BAFileNotFoundError(NSString *path)
{
    NSDictionary *info = path ? @{NSFilePathErrorKey: path} : nil;
    return [NSError errorWithDomain:BAManagedErrorDomain code:BAManagedErrorCodeFileNotFound userInfo:info];
}

static NSError *BAAssetPacksUnavailableError(NSSet<BAAssetPack *> *assetPacks)
{
    NSMutableDictionary *failures = [NSMutableDictionary dictionary];
    for (BAAssetPack *assetPack in assetPacks) {
        [failures setObject:BAAssetPackNotFoundError(assetPack.identifier) forKey:(id<NSCopying>)assetPack];
    }
    return [NSError errorWithDomain:BAManagedErrorDomain
                               code:BAManagedErrorCodeLocalAvailabilityFailure
                           userInfo:@{BASuccessesErrorKey: [NSSet set], BAFailuresErrorKey: failures}];
}

static NSURL *BAEmptyAssetPackURL(NSString *path, NSError **error)
{
    if ([[path pathComponents] containsObject:@".."]) {
        if (error) {
            *error = BAFileNotFoundError(path);
        }
        return nil;
    }
    NSURL *root = [NSURL fileURLWithPath:@"/nonexistent/BackgroundAssets" isDirectory:YES];
    return [root URLByAppendingPathComponent:path];
}

@implementation BAAssetPackManager

+ (BAAssetPackManager *)sharedManager
{
    static BAAssetPackManager *manager;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        manager = [[BAAssetPackManager alloc] init];
    });
    return manager;
}

- (void)getManifestWithCompletionHandler:(void (^)(BAAssetPackManifest *manifest, NSError *error))completionHandler
{
    BA_STUB_CALLED();
    if (!completionHandler) {
        return;
    }
    BAAssetPackManifest *manifest = [[BAAssetPackManifest alloc] init];
    BAPlaceholderAsync(^{
        completionHandler(manifest, nil);
    });
}

- (void)getAllAssetPacksWithCompletionHandler:(void (^)(NSSet<BAAssetPack *> *assetPacks, NSError *error))completionHandler
{
    BA_STUB_CALLED();
    if (!completionHandler) {
        return;
    }
    BAPlaceholderAsync(^{
        completionHandler([NSSet set], nil);
    });
}

- (void)getAssetPackWithIdentifier:(NSString *)assetPackIdentifier
                 completionHandler:(void (^)(BAAssetPack *assetPack, NSError *error))completionHandler
{
    BA_STUB_CALLED();
    if (!completionHandler) {
        return;
    }
    NSError *error = BAAssetPackNotFoundError(assetPackIdentifier);
    BAPlaceholderAsync(^{
        completionHandler(nil, error);
    });
}

- (NSData *)contentsAtPath:(NSString *)path
    searchingInAssetPackWithIdentifier:(NSString *)assetPackIdentifier
                               options:(NSDataReadingOptions)options
                                 error:(NSError **)error
{
    BA_STUB_CALLED();
    if (error) {
        *error = BAFileNotFoundError(path);
    }
    return nil;
}

- (NSData *)contentsAtPath:(NSString *)path
     asLocalizedForLanguage:(NSString *)languageIdentifier
                    options:(NSDataReadingOptions)options
                      error:(NSError **)error
{
    BA_STUB_CALLED();
    if (error) {
        *error = BAFileNotFoundError(path);
    }
    return nil;
}

- (int)fileDescriptorForPath:(NSString *)path
    searchingInAssetPackWithIdentifier:(NSString *)assetPackIdentifier
                                 error:(NSError **)error
{
    BA_STUB_CALLED();
    if (error) {
        *error = BAFileNotFoundError(path);
    }
    return -1;
}

- (int)fileDescriptorForPath:(NSString *)path
       asLocalizedForLanguage:(NSString *)languageIdentifier
                        error:(NSError **)error
{
    BA_STUB_CALLED();
    if (error) {
        *error = BAFileNotFoundError(path);
    }
    return -1;
}

- (NSURL *)URLForPath:(NSString *)path error:(NSError **)error
{
    BA_STUB_CALLED();
    return BAEmptyAssetPackURL(path, error);
}

- (NSURL *)URLForPath:(NSString *)path
    asLocalizedForLanguage:(NSString *)languageIdentifier
                     error:(NSError **)error
{
    BA_STUB_CALLED();
    return BAEmptyAssetPackURL(path, error);
}

- (void)checkForUpdatesWithCompletionHandler:(void (^)(NSSet<NSString *> *updatingIdentifiers, NSSet<NSString *> *removedIdentifiers, NSError *error))completionHandler
{
    BA_STUB_CALLED();
    if (!completionHandler) {
        return;
    }
    BAPlaceholderAsync(^{
        completionHandler([NSSet set], [NSSet set], nil);
    });
}

- (void)ensureLocalAvailabilityOfAssetPack:(BAAssetPack *)assetPack
                         completionHandler:(void (^)(NSError *error))completionHandler
{
    BA_STUB_CALLED();
    if (!completionHandler) {
        return;
    }
    NSError *error = BAAssetPackNotFoundError(assetPack.identifier);
    BAPlaceholderAsync(^{
        completionHandler(error);
    });
}

- (void)ensureLocalAvailabilityOfAssetPacks:(NSSet<BAAssetPack *> *)assetPacks
                          completionHandler:(void (^)(NSError *error))completionHandler
{
    BA_STUB_CALLED();
    if (!completionHandler) {
        return;
    }
    NSError *error = assetPacks.count ? BAAssetPacksUnavailableError(assetPacks) : nil;
    BAPlaceholderAsync(^{
        completionHandler(error);
    });
}

- (void)ensureLocalAvailabilityOfAssetPack:(BAAssetPack *)assetPack
                      requireLatestVersion:(BOOL)shouldUpdate
                         completionHandler:(void (^)(NSError *error))completionHandler
{
    [self ensureLocalAvailabilityOfAssetPack:assetPack completionHandler:completionHandler];
}

- (void)ensureLocalAvailabilityOfAssetPacks:(NSSet<BAAssetPack *> *)assetPacks
                      requireLatestVersions:(BOOL)shouldUpdate
                          completionHandler:(void (^)(NSError *error))completionHandler
{
    [self ensureLocalAvailabilityOfAssetPacks:assetPacks completionHandler:completionHandler];
}

- (BOOL)assetPackIsAvailableLocallyWithIdentifier:(NSString *)assetPackIdentifier
{
    return NO;
}

- (void)getStatusRelativeToAssetPack:(BAAssetPack *)assetPack
                   completionHandler:(void (^)(BAAssetPackStatus status, NSError *error))completionHandler
{
    BA_STUB_CALLED();
    if (!completionHandler) {
        return;
    }
    NSError *error = BAAssetPackNotFoundError(assetPack.identifier);
    BAPlaceholderAsync(^{
        completionHandler(0, error);
    });
}

- (void)getLocalStatusOfAssetPackWithIdentifier:(NSString *)assetPackIdentifier
                              completionHandler:(void (^)(BAAssetPackStatus status))completionHandler
{
    BA_STUB_CALLED();
    if (!completionHandler) {
        return;
    }
    BAPlaceholderAsync(^{
        completionHandler(0);
    });
}

- (void)removeAssetPackWithIdentifier:(NSString *)assetPackIdentifier
                    completionHandler:(void (^)(NSError *error))completionHandler
{
    BA_STUB_CALLED();
    if (!completionHandler) {
        return;
    }
    NSError *error = BAAssetPackNotFoundError(assetPackIdentifier);
    BAPlaceholderAsync(^{
        completionHandler(error);
    });
}

- (void)getStatusOfAssetPackWithIdentifier:(NSString *)assetPackIdentifier
                         completionHandler:(void (^)(BAAssetPackStatus status, NSError *error))completionHandler
{
    BA_STUB_CALLED();
    if (!completionHandler) {
        return;
    }
    NSError *error = BAAssetPackNotFoundError(assetPackIdentifier);
    BAPlaceholderAsync(^{
        completionHandler(0, error);
    });
}

- (void)getLocallyAvailableLanguagesWithCompletionHandler:(void (^)(NSArray<NSString *> *languageIdentifiers))completionHandler
{
    BA_STUB_CALLED();
    if (!completionHandler) {
        return;
    }
    BAPlaceholderAsync(^{
        completionHandler(@[]);
    });
}

- (void)reconcilePreferredLanguagesWithCompletionHandler:(void (^)(NSError *error))completionHandler
{
    BA_STUB_CALLED();
    if (!completionHandler) {
        return;
    }
    BAPlaceholderAsync(^{
        completionHandler(nil);
    });
}

- (void)getLocalSizeOfAssetPackWithIdentifier:(NSString *)assetPackIdentifier
                            calculationMethod:(BASizeCalculationMethod)calculationMethod
                            completionHandler:(void (^)(long long size, NSError *error))completionHandler
{
    BA_STUB_CALLED();
    if (!completionHandler) {
        return;
    }
    NSError *error = BAAssetPackNotFoundError(assetPackIdentifier);
    BAPlaceholderAsync(^{
        completionHandler(-1, error);
    });
}

- (NSInteger)localVersionOfAssetPackWithIdentifier:(NSString *)assetPackIdentifier error:(NSError **)error
{
    BA_STUB_CALLED();
    if (error) {
        *error = BAAssetPackNotFoundError(assetPackIdentifier);
    }
    return -1;
}

@end
