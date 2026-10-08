/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#import <GameController/GCController.h>
#import <GameController/GCMicroGamepad.h>
#import <GameController/GCPhysicalInputProfile.h>

@implementation GCController

+ (NSArray *)controllers {
	return @[];
}

+ (NSArray *)extendedGamepads {
	return @[];
}

- (GCExtendedGamepad *)extendedGamepad {
	return nil;
}

- (GCMicroGamepad *)microGamepad {
	return nil;
}

- (GCPhysicalInputProfile *)physicalInputProfile {
	return nil;
}

- (NSString *)vendorName {
	return nil;
}

- (NSString *)productCategory {
	return @"";
}

- (GCControllerPlayerIndex)playerIndex {
	return GCControllerPlayerIndexUnset;
}

- (void)setPlayerIndex:(GCControllerPlayerIndex)playerIndex {
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
    NSLog(@"Stub called: %@ in %@ (class method)", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
