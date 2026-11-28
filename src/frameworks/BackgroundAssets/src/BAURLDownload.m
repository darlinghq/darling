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

#import <BackgroundAssets/BAURLDownload.h>
#import "BADownloadInternal.h"

// Placeholder: the request, size and App Group are stored and archived, and never used; no request is made.
//
// GUESS: the documentation says requests must use HTTPS and that the size must match the file, but a download
// that is never started cannot fail on either, so neither is checked. The initializers without an essential
// parameter make a non-essential download, as the documentation says is the default for the one that has it. The
// documentation states the default priority only for the initializer that has neither a file size nor a priority;
// the one with a file size and no priority uses it too. Initializers without a file size use 0.
@implementation BAURLDownload {
    NSURLRequest *_request;
    NSUInteger _fileSize;
    NSString *_applicationGroupIdentifier;
}

- (instancetype)initWithIdentifier:(NSString *)identifier
                           request:(NSURLRequest *)request
                         essential:(BOOL)essential
                          fileSize:(NSUInteger)fileSize
        applicationGroupIdentifier:(NSString *)applicationGroupIdentifier
                          priority:(BADownloaderPriority)priority
{
    self = [super initPlaceholderWithIdentifier:identifier essential:essential priority:priority];
    if (self) {
        _request = [request copy];
        _fileSize = fileSize;
        _applicationGroupIdentifier = [applicationGroupIdentifier copy];
    }
    return self;
}

- (instancetype)initWithIdentifier:(NSString *)identifier
                           request:(NSURLRequest *)request
                          fileSize:(NSUInteger)fileSize
        applicationGroupIdentifier:(NSString *)applicationGroupIdentifier
{
    return [self initWithIdentifier:identifier
                            request:request
                          essential:NO
                           fileSize:fileSize
         applicationGroupIdentifier:applicationGroupIdentifier
                           priority:BADownloaderPriorityDefault];
}

- (instancetype)initWithIdentifier:(NSString *)identifier
                           request:(NSURLRequest *)request
        applicationGroupIdentifier:(NSString *)applicationGroupIdentifier
{
    return [self initWithIdentifier:identifier
                            request:request
                          essential:NO
                           fileSize:0
         applicationGroupIdentifier:applicationGroupIdentifier
                           priority:BADownloaderPriorityDefault];
}

- (instancetype)initWithIdentifier:(NSString *)identifier
                           request:(NSURLRequest *)request
        applicationGroupIdentifier:(NSString *)applicationGroupIdentifier
                          priority:(BADownloaderPriority)priority
{
    return [self initWithIdentifier:identifier
                            request:request
                          essential:NO
                           fileSize:0
         applicationGroupIdentifier:applicationGroupIdentifier
                           priority:priority];
}

- (id)copyWithZone:(NSZone *)zone
{
    BAURLDownload *copy = [super copyWithZone:zone];
    copy->_request = [_request copy];
    copy->_fileSize = _fileSize;
    copy->_applicationGroupIdentifier = [_applicationGroupIdentifier copy];
    return copy;
}

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [super encodeWithCoder:coder];
    [coder encodeObject:_request forKey:@"request"];
    [coder encodeInt64:(int64_t)_fileSize forKey:@"fileSize"];
    [coder encodeObject:_applicationGroupIdentifier forKey:@"applicationGroupIdentifier"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    NSURLRequest *request = [coder decodeObjectOfClass:[NSURLRequest class] forKey:@"request"];
    NSString *group = [coder decodeObjectOfClass:[NSString class] forKey:@"applicationGroupIdentifier"];
    if (!request || !group) {
        return nil;
    }
    self = [super initWithCoder:coder];
    if (self) {
        _request = [request copy];
        _fileSize = (NSUInteger)[coder decodeInt64ForKey:@"fileSize"];
        _applicationGroupIdentifier = [group copy];
    }
    return self;
}

@end
