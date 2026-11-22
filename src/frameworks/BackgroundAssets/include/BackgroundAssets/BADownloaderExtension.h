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

#ifndef _BADOWNLOADEREXTENSION_H_
#define _BADOWNLOADEREXTENSION_H_

#import <Foundation/Foundation.h>
#import <BackgroundAssets/BAAppExtensionInfo.h>
#import <BackgroundAssets/BAContentRequest.h>
#import <BackgroundAssets/BADownload.h>

NS_ASSUME_NONNULL_BEGIN

// GUESS: the documentation does not show which methods are required. All are declared optional, which also
// keeps an Objective-C class that adopts BAManagedDownloaderExtension free of "not implemented" warnings, as the
// documentation's example expects.
API_AVAILABLE(macos(13.0))
@protocol BADownloaderExtension <NSObject>

@optional
- (void)backgroundDownload:(BADownload *)download
       didReceiveChallenge:(NSURLAuthenticationChallenge *)challenge
         completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition disposition, NSURLCredential *_Nullable credential))completionHandler;
- (void)backgroundDownload:(BADownload *)download finishedWithFileURL:(NSURL *)fileURL;
- (void)backgroundDownload:(BADownload *)download failedWithError:(NSError *)error;
- (NSSet<BADownload *> *)downloadsForRequest:(BAContentRequest)contentRequest
                                  manifestURL:(NSURL *)manifestURL
                                extensionInfo:(BAAppExtensionInfo *)extensionInfo;
- (void)extensionWillTerminate API_DEPRECATED("", macos(13.0, 13.3));

@end

NS_ASSUME_NONNULL_END

#endif
