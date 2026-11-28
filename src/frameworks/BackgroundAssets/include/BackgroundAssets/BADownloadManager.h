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

#ifndef _BADOWNLOADMANAGER_H_
#define _BADOWNLOADMANAGER_H_

#import <Foundation/Foundation.h>
#import <BackgroundAssets/BADownload.h>
#import <BackgroundAssets/BAErrors.h>

NS_ASSUME_NONNULL_BEGIN

// GUESS: the documentation does not show which methods are required, so all are declared optional.
API_AVAILABLE(macos(13.0))
@protocol BADownloadManagerDelegate <NSObject>

@optional
- (void)downloadDidBegin:(BADownload *)download;
- (void)download:(BADownload *)download
    didReceiveChallenge:(NSURLAuthenticationChallenge *)challenge
      completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition disposition, NSURLCredential *_Nullable credential))completionHandler;
- (void)download:(BADownload *)download
              didWriteBytes:(int64_t)bytesWritten
          totalBytesWritten:(int64_t)totalBytesWritten
  totalBytesExpectedToWrite:(int64_t)totalExpectedBytes;
- (void)downloadDidPause:(BADownload *)download;
- (void)download:(BADownload *)download finishedWithFileURL:(NSURL *)fileURL;
- (void)download:(BADownload *)download failedWithError:(NSError *)error;

@end

API_AVAILABLE(macos(13.0))
@interface BADownloadManager : NSObject

@property (class, strong, readonly) BADownloadManager *sharedManager;

// GUESS: nullable, by analogy with the documented delegate of BAAssetPackManager.
@property (weak, nullable) id<BADownloadManagerDelegate> delegate;

- (BOOL)scheduleDownload:(BADownload *)download error:(NSError **)error;
- (BOOL)startForegroundDownload:(BADownload *)download error:(NSError **)error;
- (BOOL)cancelDownload:(BADownload *)download error:(NSError **)error;

// GUESS: the documentation shows no nullability for the downloads or the error of the completion handler and
// the error out-parameter; the result is nonnull on success and the error is nullable.
- (void)fetchCurrentDownloadsWithCompletionHandler:(void (^)(NSArray<BADownload *> *_Nullable downloads, NSError *_Nullable error))completionHandler;
- (nullable NSArray<BADownload *> *)fetchCurrentDownloads:(NSError **)error API_AVAILABLE(macos(13.3));

- (void)performWithExclusiveControl:(void (^)(BOOL acquiredLock, NSError *_Nullable error))performHandler;
- (void)performWithExclusiveControlBeforeDate:(NSDate *)date
                               performHandler:(void (^)(BOOL acquiredLock, NSError *_Nullable error))performHandler;

@end

NS_ASSUME_NONNULL_END

#endif
