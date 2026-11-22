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

#import <BackgroundAssets/BAAppExtensionInfo.h>

// Placeholder: nothing restricts the downloads of an extension here, and the system never creates one of these
// objects, so both values are nil.
//
// GUESS: the keys used to archive an instance.
@implementation BAAppExtensionInfo

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_restrictedDownloadSizeRemaining forKey:@"restrictedDownloadSizeRemaining"];
    [coder encodeObject:_restrictedEssentialDownloadSizeRemaining forKey:@"restrictedEssentialDownloadSizeRemaining"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super init];
    if (self) {
        _restrictedDownloadSizeRemaining = [coder decodeObjectOfClass:[NSNumber class] forKey:@"restrictedDownloadSizeRemaining"];
        _restrictedEssentialDownloadSizeRemaining = [coder decodeObjectOfClass:[NSNumber class] forKey:@"restrictedEssentialDownloadSizeRemaining"];
    }
    return self;
}

@end
