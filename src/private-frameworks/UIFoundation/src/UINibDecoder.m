/*
 This file is part of Darling.

 Copyright (C) 2025 Darling Developers

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

#import <UIFoundation/UINibDecoder.h>

@implementation UINibDecoder

- (instancetype) initForReadingWithData: (NSData *) data
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return self;
}

- (BOOL) containsValueForKey: (NSString *) key
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return NO;
}

- (void) decodeValueOfObjCType: (const char *) type at: (void *) data
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
}

- (NSData *) decodeDataObject
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return nil;
}

- (id) decodeObjectForKey: (NSString *) key
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return nil;
}

- (BOOL) decodeBoolForKey: (NSString *) key
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return NO;
}

- (int) decodeIntForKey:(NSString *) key
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return 0;
}

- (int32_t) decodeInt32ForKey:(NSString *) key
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return 0;
}

- (int64_t) decodeInt64ForKey:(NSString *) key
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return 0;
}

- (float) decodeFloatForKey:(NSString *) key
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return 0.0f;
}

- (double) decodeDoubleForKey:(NSString *) key
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return 0.0;
}

- (const uint8_t *) decodeBytesForKey: (NSString *) key returnedLength: (NSUInteger *) len
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return NULL;
}

- (NSInteger) versionForClassName: (NSString *) className
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return 0;
}

@end
