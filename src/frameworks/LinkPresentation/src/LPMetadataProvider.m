/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#import <LinkPresentation/LPMetadataProvider.h>
#import <LinkPresentation/LPError.h>
#include <dispatch/dispatch.h>

// Darling does not fetch link metadata: a fetch fails with
// LPErrorMetadataFetchFailed, or LPErrorMetadataFetchCancelled once the fetch
// is cancelled. As documented, a provider fetches once; a second
// fetch is a programming error.
@implementation LPMetadataProvider {
    BOOL _started;
    BOOL _cancelled;
}

@synthesize shouldFetchSubresources = _shouldFetchSubresources, timeout = _timeout;

- (instancetype)init
{
    self = [super init];
    if (self) {
        _shouldFetchSubresources = YES;
        _timeout = 30;
    }
    return self;
}

- (void)startFetchingMetadataForURL:(NSURL *)URL completionHandler:(void (^)(LPLinkMetadata *, NSError *))completionHandler
{
    if (_started)
        [NSException raise: NSInternalInconsistencyException format: @"-[LPMetadataProvider startFetchingMetadataForURL:completionHandler:]: a provider fetches only once"];
    _started = YES;
    (void) URL;
    void (^handler)(LPLinkMetadata *, NSError *) = [completionHandler copy];
    // the block keeps the provider, so that a -cancel before it runs is seen
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), ^{
        NSInteger code = self->_cancelled ? LPErrorMetadataFetchCancelled : LPErrorMetadataFetchFailed;
        handler(nil, [NSError errorWithDomain: LPErrorDomain code: code userInfo: nil]);
        [handler release];
    });
}

- (void)startFetchingMetadataForRequest:(NSURLRequest *)request completionHandler:(void (^)(LPLinkMetadata *, NSError *))completionHandler
{
    [self startFetchingMetadataForURL: request.URL completionHandler: completionHandler];
}

- (void)cancel
{
    _cancelled = YES;
}

@end
