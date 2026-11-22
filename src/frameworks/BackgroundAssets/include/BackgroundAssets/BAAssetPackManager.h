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

#ifndef _BAASSETPACKMANAGER_H_
#define _BAASSETPACKMANAGER_H_

#import <Foundation/Foundation.h>
#import <BackgroundAssets/BAAssetPack.h>
#import <BackgroundAssets/BAAssetPackManifest.h>
#import <BackgroundAssets/BAAssetPackStatus.h>
#import <BackgroundAssets/BAErrors.h>

NS_ASSUME_NONNULL_BEGIN

// The documentation shows no numeric values.
// GUESS: values 0 and 1 in the order the documentation lists the cases.
typedef NS_ENUM(NSInteger, BASizeCalculationMethod) {
    BASizeCalculationMethodAllocated = 0,
    BASizeCalculationMethodLogical = 1,
} API_AVAILABLE(macos(27.2));

// GUESS: the documentation does not show which methods are required, so all are declared optional.
API_AVAILABLE(macos(26.0))
@protocol BAManagedAssetPackDownloadDelegate <NSObject>

@optional
- (void)downloadOfAssetPackBegan:(BAAssetPack *)assetPack;
- (void)downloadOfAssetPack:(BAAssetPack *)assetPack hasProgress:(NSProgress *)progress;
- (void)downloadOfAssetPackPaused:(BAAssetPack *)assetPack;
- (void)downloadOfAssetPackFinished:(BAAssetPack *)assetPack;
- (void)downloadOfAssetPack:(BAAssetPack *)assetPack failedWithError:(NSError *)error;

@end

// GUESS: the documentation's declarations show no nullability on the parameters of completion handlers. A
// parameter that the documentation says is delivered only on success, or only on failure, is declared nullable.
API_AVAILABLE(macos(26.0))
@interface BAAssetPackManager : NSObject

@property (class, strong, readonly) BAAssetPackManager *sharedManager;

@property (weak, readwrite, nullable) id<BAManagedAssetPackDownloadDelegate> delegate;

- (void)getManifestWithCompletionHandler:(void (^)(BAAssetPackManifest *_Nullable manifest, NSError *_Nullable error))completionHandler API_AVAILABLE(macos(27.0));
- (void)getAllAssetPacksWithCompletionHandler:(void (^)(NSSet<BAAssetPack *> *_Nullable assetPacks, NSError *_Nullable error))completionHandler API_DEPRECATED("", macos(26.0, 27.0));
- (void)getAssetPackWithIdentifier:(NSString *)assetPackIdentifier
                 completionHandler:(void (^)(BAAssetPack *_Nullable assetPack, NSError *_Nullable error))completionHandler API_DEPRECATED("", macos(26.0, 27.0));

- (nullable NSData *)contentsAtPath:(NSString *)path
    searchingInAssetPackWithIdentifier:(nullable NSString *)assetPackIdentifier
                               options:(NSDataReadingOptions)options
                                 error:(NSError **)error;
- (nullable NSData *)contentsAtPath:(NSString *)path
              asLocalizedForLanguage:(NSString *)languageIdentifier
                             options:(NSDataReadingOptions)options
                               error:(NSError **)error API_AVAILABLE(macos(27.0));
- (int)fileDescriptorForPath:(NSString *)path
    searchingInAssetPackWithIdentifier:(nullable NSString *)assetPackIdentifier
                                 error:(NSError **)error;
- (int)fileDescriptorForPath:(NSString *)path
       asLocalizedForLanguage:(NSString *)languageIdentifier
                        error:(NSError **)error API_AVAILABLE(macos(27.0));
- (nullable NSURL *)URLForPath:(NSString *)path error:(NSError **)error;
- (nullable NSURL *)URLForPath:(NSString *)path
         asLocalizedForLanguage:(NSString *)languageIdentifier
                          error:(NSError **)error API_AVAILABLE(macos(27.0));

- (void)checkForUpdatesWithCompletionHandler:(void (^)(NSSet<NSString *> *_Nullable updatingIdentifiers, NSSet<NSString *> *_Nullable removedIdentifiers, NSError *_Nullable error))completionHandler;
- (void)ensureLocalAvailabilityOfAssetPack:(BAAssetPack *)assetPack
                         completionHandler:(void (^)(NSError *_Nullable error))completionHandler;
- (void)ensureLocalAvailabilityOfAssetPacks:(NSSet<BAAssetPack *> *)assetPacks
                          completionHandler:(void (^)(NSError *_Nullable error))completionHandler API_AVAILABLE(macos(27.0));
- (void)ensureLocalAvailabilityOfAssetPack:(BAAssetPack *)assetPack
                      requireLatestVersion:(BOOL)shouldUpdate
                         completionHandler:(void (^)(NSError *_Nullable error))completionHandler API_AVAILABLE(macos(26.4));
- (void)ensureLocalAvailabilityOfAssetPacks:(NSSet<BAAssetPack *> *)assetPacks
                      requireLatestVersions:(BOOL)shouldUpdate
                          completionHandler:(void (^)(NSError *_Nullable error))completionHandler API_AVAILABLE(macos(27.0));
- (BOOL)assetPackIsAvailableLocallyWithIdentifier:(NSString *)assetPackIdentifier API_AVAILABLE(macos(26.4));
- (void)getStatusRelativeToAssetPack:(BAAssetPack *)assetPack
                   completionHandler:(void (^)(BAAssetPackStatus status, NSError *_Nullable error))completionHandler API_AVAILABLE(macos(26.4));
- (void)getLocalStatusOfAssetPackWithIdentifier:(NSString *)assetPackIdentifier
                              completionHandler:(void (^)(BAAssetPackStatus status))completionHandler API_AVAILABLE(macos(26.4));
- (void)removeAssetPackWithIdentifier:(NSString *)assetPackIdentifier
                    completionHandler:(void (^)(NSError *_Nullable error))completionHandler;
- (void)getStatusOfAssetPackWithIdentifier:(NSString *)assetPackIdentifier
                         completionHandler:(void (^)(BAAssetPackStatus status, NSError *_Nullable error))completionHandler API_DEPRECATED("", macos(26.0, 26.4));

- (void)getLocallyAvailableLanguagesWithCompletionHandler:(void (^)(NSArray<NSString *> *languageIdentifiers))completionHandler API_AVAILABLE(macos(27.0));
@property (copy, readwrite, nullable) NSString *resolvedLanguage API_AVAILABLE(macos(27.0));
- (void)reconcilePreferredLanguagesWithCompletionHandler:(void (^)(NSError *_Nullable error))completionHandler API_AVAILABLE(macos(27.0));

- (void)getLocalSizeOfAssetPackWithIdentifier:(NSString *)assetPackIdentifier
                            calculationMethod:(BASizeCalculationMethod)calculationMethod
                            completionHandler:(void (^)(long long size, NSError *_Nullable error))completionHandler API_AVAILABLE(macos(27.2));
- (NSInteger)localVersionOfAssetPackWithIdentifier:(NSString *)assetPackIdentifier
                                              error:(NSError **)error API_AVAILABLE(macos(27.2));

@end

NS_ASSUME_NONNULL_END

#endif
