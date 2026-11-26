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

#import <LinkPresentation/LPLinkMetadata.h>
#import <LinkPresentation/LPError.h>

NSErrorDomain const LPErrorDomain = @"LPErrorDomain";

// The URLs and the title are what is encoded; the providers are not.
static NSString *const originalURLKey = @"originalURL";
static NSString *const URLKey = @"URL";
static NSString *const titleKey = @"title";
static NSString *const remoteVideoURLKey = @"remoteVideoURL";

@implementation LPLinkMetadata

@synthesize originalURL = _originalURL, URL = _URL, title = _title;
@synthesize iconProvider = _iconProvider, imageProvider = _imageProvider, videoProvider = _videoProvider;
@synthesize remoteVideoURL = _remoteVideoURL;

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (void)dealloc
{
    [_originalURL release];
    [_URL release];
    [_title release];
    [_iconProvider release];
    [_imageProvider release];
    [_videoProvider release];
    [_remoteVideoURL release];
    [super dealloc];
}

- (id)copyWithZone:(NSZone *)zone
{
    LPLinkMetadata *copy = [[[self class] allocWithZone: zone] init];
    copy.originalURL = _originalURL;
    copy.URL = _URL;
    copy.title = _title;
    copy.iconProvider = _iconProvider;
    copy.imageProvider = _imageProvider;
    copy.videoProvider = _videoProvider;
    copy.remoteVideoURL = _remoteVideoURL;
    return copy;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject: _originalURL forKey: originalURLKey];
    [coder encodeObject: _URL forKey: URLKey];
    [coder encodeObject: _title forKey: titleKey];
    [coder encodeObject: _remoteVideoURL forKey: remoteVideoURLKey];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super init];
    if (self) {
        _originalURL = [[coder decodeObjectOfClass: [NSURL class] forKey: originalURLKey] retain];
        _URL = [[coder decodeObjectOfClass: [NSURL class] forKey: URLKey] retain];
        _title = [[coder decodeObjectOfClass: [NSString class] forKey: titleKey] copy];
        _remoteVideoURL = [[coder decodeObjectOfClass: [NSURL class] forKey: remoteVideoURLKey] retain];
    }
    return self;
}

@end
