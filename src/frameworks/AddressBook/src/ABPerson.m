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

#import <AddressBook/ABPerson.h>
#import <AddressBook/ABGlobals.h>
#import <Foundation/NSMethodSignature.h>
#import <Foundation/NSInvocation.h>

@implementation ABPerson

- (id) valueForProperty: (NSString *) property
{
    if ([property isEqual: kABFirstNameProperty]) {
        NSString *name = NSFullUserName();
        NSArray *parts = [name componentsSeparatedByString: @" "];
        return [parts count] > 0 ? parts[0] : (NSUserName() ?: @"Developer");
    }
    if ([property isEqual: kABLastNameProperty]) {
        NSString *name = NSFullUserName();
        NSArray *parts = [name componentsSeparatedByString: @" "];
        return [parts count] > 1 ? parts[1] : @"";
    }
    if ([property isEqual: kABOrganizationProperty]) {
        return @"Darling Developer";
    }
    return [super valueForProperty: property];
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
