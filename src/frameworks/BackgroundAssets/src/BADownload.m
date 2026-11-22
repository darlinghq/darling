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

#import <BackgroundAssets/BADownload.h>
#import "BADownloadInternal.h"

// Placeholder: a download only records its identifier, essential flag and priority. Nothing ever schedules it, so
// it stays in BADownloadStateCreated.
//
// GUESS: the keys used to archive a download, and a UUID string for the system-provided uniqueIdentifier of a new
// download or of a copy (the documentation says only that it identifies the download object).
@implementation BADownload

- (instancetype)initPlaceholderWithIdentifier:(NSString *)identifier
                                    essential:(BOOL)essential
                                     priority:(BADownloaderPriority)priority
{
    self = [super init];
    if (self) {
        _identifier = [identifier copy];
        _uniqueIdentifier = [[NSUUID UUID] UUIDString];
        _isEssential = essential;
        _priority = priority;
        _state = BADownloadStateCreated;
    }
    return self;
}

- (instancetype)copyAsNonEssential
{
    BADownload *copy = [self copy];
    copy->_isEssential = NO;
    return copy;
}

- (id)copyWithZone:(NSZone *)zone
{
    BADownload *copy = [[[self class] allocWithZone:zone] initPlaceholderWithIdentifier:_identifier
                                                                              essential:_isEssential
                                                                               priority:_priority];
    copy->_state = _state;
    return copy;
}

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_identifier forKey:@"identifier"];
    [coder encodeObject:_uniqueIdentifier forKey:@"uniqueIdentifier"];
    [coder encodeBool:_isEssential forKey:@"isEssential"];
    [coder encodeInteger:_priority forKey:@"priority"];
    [coder encodeInteger:_state forKey:@"state"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    NSString *identifier = [coder decodeObjectOfClass:[NSString class] forKey:@"identifier"];
    NSString *uniqueIdentifier = [coder decodeObjectOfClass:[NSString class] forKey:@"uniqueIdentifier"];
    NSInteger state = [coder decodeIntegerForKey:@"state"];
    if (!identifier || !uniqueIdentifier || state < BADownloadStateCreated || state > BADownloadStateFailed) {
        return nil;
    }
    self = [self initPlaceholderWithIdentifier:identifier
                                     essential:[coder decodeBoolForKey:@"isEssential"]
                                      priority:[coder decodeIntegerForKey:@"priority"]];
    if (self) {
        _uniqueIdentifier = [uniqueIdentifier copy];
        _state = (BADownloadState)state;
    }
    return self;
}

@end
