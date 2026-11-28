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

#import <BackgroundAssets/BAAssetPack.h>
#import "BADownloadInternal.h"

// Placeholder: this framework never has an asset pack to hand out, so an instance can only come from alloc and
// init, and describes an empty pack: an empty identifier (the property is not nullable), version 0, no user info,
// no language and a download size of 0. A download made from it records the empty identifier and is never
// scheduled.
//
// GUESS: the download of a pack is not essential and has the default priority; the documentation says neither.
@implementation BAAssetPack

- (instancetype)init
{
    self = [super init];
    if (self) {
        _identifier = @"";
    }
    return self;
}

// NSDictionary copies its keys, and BAManagedErrorCodeLocalAvailabilityFailure errors key a dictionary by asset
// pack. The class is immutable, so a copy is the instance itself. The documentation lists no NSCopying conformance,
// so none is declared.
- (id)copyWithZone:(NSZone *)zone
{
    return self;
}

- (BADownload *)download
{
    return [[BADownload alloc] initPlaceholderWithIdentifier:self.identifier
                                                   essential:NO
                                                    priority:BADownloaderPriorityDefault];
}

- (BADownload *)downloadForContentRequest:(BAContentRequest)contentRequest
{
    return [self download];
}

@end
