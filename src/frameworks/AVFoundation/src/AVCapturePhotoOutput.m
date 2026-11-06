/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

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

#import <Foundation/Foundation.h>

@interface AVCapturePhotoOutput : NSObject
@end

@implementation AVCapturePhotoOutput

// Every selector is accepted and logged; messages return nil/0 to the caller.
+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [NSMethodSignature signatureWithObjCTypes: "@@:"];
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
    id result = nil;
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), self);
    [anInvocation setReturnValue: &result];
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [NSMethodSignature signatureWithObjCTypes: "@@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    id result = nil;
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
    [anInvocation setReturnValue: &result];
}

@end
