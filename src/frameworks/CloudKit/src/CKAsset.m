/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

#import "CKInternal.h"

@implementation CKAsset

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)initWithFileURL:(NSURL *)fileURL
{
    if (fileURL == nil)
        CKRaiseInvalid(NSInvalidArgumentException, @"Null fileURL");
    if (![fileURL isFileURL])
        CKRaiseInvalid(NSInvalidArgumentException, @"Non-file URL");
    if ((self = [super init]))
        _fileURL = [fileURL copy];
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    NSURL *url = [coder decodeObjectOfClass:[NSURL class] forKey:@"FileURL"];
    if (url == nil || ![url isFileURL]) {
        return nil;
    }
    return [self initWithFileURL:url];
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_fileURL forKey:@"FileURL"];
}

- (NSURL *)fileURL
{
    return _fileURL;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p; path=%@>", [self class], self, _fileURL.path];
}

@end
