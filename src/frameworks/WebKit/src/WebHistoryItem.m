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

#import <WebKit/WebHistoryItem.h>
#import <Foundation/NSString.h>
#import <Foundation/NSMethodSignature.h>
#import <Foundation/NSInvocation.h>

@implementation WebHistoryItem

- (id)initWithURLString:(NSString *)urlString title:(NSString *)title lastVisitedTimeInterval:(NSTimeInterval)time
{
    self = [super init];
    if (self) {
        _urlString = [urlString copy];
        _title = [title copy];
        _time = time;
    }
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    return [self retain];
}

- (void)dealloc
{
    [_urlString release];
    [_title release];
    [super dealloc];
}

- (NSString *)originalURLString
{
    return _urlString;
}

- (NSString *)URLString
{
    return _urlString;
}

- (NSString *)title
{
    return _title;
}

- (NSTimeInterval)lastVisitedTimeInterval
{
    return _time;
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
