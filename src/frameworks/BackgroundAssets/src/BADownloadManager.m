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

#import <BackgroundAssets/BADownloadManager.h>
#import "BAPlaceholder.h"

// Placeholder: there is no download service behind this class. The queue of downloads is always empty, no download
// can be scheduled, started or cancelled, and exclusive control is granted without waiting, since nothing else can
// hold it. Nothing touches the network or the file system. Each completion handler is called exactly once,
// asynchronously, on a global queue; the documentation says the handlers of this class may run on any queue.
//
// A download that cannot start reports an error in BAErrorDomain whose NSUnderlyingErrorKey holds
// NSFeatureUnsupportedError in NSCocoaErrorDomain, so the real reason stays visible.
// GUESS: the codes. The documentation names the cases but gives none a meaning: DownloadFailedToStart is used for
// scheduling and for starting a download, DownloadNotScheduled for cancelling one.
// GUESS: the documentation does not say whether the delegate is retained; it is held weakly, as declared.

static NSError *BAUnsupportedDownloadError(BAErrorCode code)
{
    NSError *reason = [NSError errorWithDomain:NSCocoaErrorDomain code:NSFeatureUnsupportedError userInfo:nil];
    return [NSError errorWithDomain:BAErrorDomain code:code userInfo:@{NSUnderlyingErrorKey: reason}];
}

@implementation BADownloadManager

+ (BADownloadManager *)sharedManager
{
    static BADownloadManager *manager;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        manager = [[BADownloadManager alloc] init];
    });
    return manager;
}

- (BOOL)scheduleDownload:(BADownload *)download error:(NSError **)error
{
    BA_STUB_CALLED();
    if (error) {
        *error = BAUnsupportedDownloadError(BAErrorCodeDownloadFailedToStart);
    }
    return NO;
}

- (BOOL)startForegroundDownload:(BADownload *)download error:(NSError **)error
{
    BA_STUB_CALLED();
    if (error) {
        *error = BAUnsupportedDownloadError(BAErrorCodeDownloadFailedToStart);
    }
    return NO;
}

- (BOOL)cancelDownload:(BADownload *)download error:(NSError **)error
{
    BA_STUB_CALLED();
    if (error) {
        *error = BAUnsupportedDownloadError(BAErrorCodeDownloadNotScheduled);
    }
    return NO;
}

- (void)fetchCurrentDownloadsWithCompletionHandler:(void (^)(NSArray<BADownload *> *downloads, NSError *error))completionHandler
{
    BA_STUB_CALLED();
    if (!completionHandler) {
        return;
    }
    BAPlaceholderAsync(^{
        completionHandler(@[], nil);
    });
}

- (NSArray<BADownload *> *)fetchCurrentDownloads:(NSError **)error
{
    BA_STUB_CALLED();
    return @[];
}

- (void)performWithExclusiveControl:(void (^)(BOOL acquiredLock, NSError *error))performHandler
{
    BA_STUB_CALLED();
    if (!performHandler) {
        return;
    }
    BAPlaceholderAsync(^{
        performHandler(YES, nil);
    });
}

- (void)performWithExclusiveControlBeforeDate:(NSDate *)date
                               performHandler:(void (^)(BOOL acquiredLock, NSError *error))performHandler
{
    BA_STUB_CALLED();
    if (!performHandler) {
        return;
    }
    BAPlaceholderAsync(^{
        performHandler(YES, nil);
    });
}

@end
