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

#import <FinderSync/FinderSync.h>

@implementation FIFinderSyncController

+ (instancetype)defaultController
{
    static FIFinderSyncController *sController = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sController = [[FIFinderSyncController alloc] init];
    });
    return sController;
}

+ (BOOL)isExtensionEnabled
{
    return NO;
}

+ (void)showExtensionManagementInterface
{
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        _directoryURLs = [[NSSet alloc] init];
        _badges = [[NSMutableDictionary alloc] init];
    }
    return self;
}

- (void)dealloc
{
    [_directoryURLs release];
    [_badges release];
    [super dealloc];
}

- (NSSet<NSURL *> *)directoryURLs
{
    @synchronized (self) {
        return [[_directoryURLs retain] autorelease];
    }
}

- (void)setDirectoryURLs:(NSSet<NSURL *> *)directoryURLs
{
    @synchronized (self) {
        if (_directoryURLs != directoryURLs) {
            [_directoryURLs release];
            _directoryURLs = [directoryURLs copy];
        }
    }
}

- (void)setBadgeImage:(NSImage *)image label:(NSString *)label forBadgeIdentifier:(FIBadgeIdentifier)badgeID
{
    // Badge images are Finder-specific overlays on desktop icons
}

- (void)setBadgeIdentifier:(FIBadgeIdentifier)badgeID forURL:(NSURL *)url
{
    if (!url) return;
    @synchronized (self) {
        if (badgeID)
            [_badges setObject:badgeID forKey:url];
        else
            [_badges removeObjectForKey:url];
    }
}

- (FIBadgeIdentifier)badgeIdentifierForURL:(NSURL *)url
{
    if (!url) return nil;
    @synchronized (self) {
        return [[[_badges objectForKey:url] retain] autorelease];
    }
}

@end

@implementation FIFinderSync
@end
