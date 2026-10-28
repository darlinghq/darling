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

#import <AddressBook/ABAddressBook.h>
#import <AddressBook/ABPerson.h>

@implementation ABAddressBook

+ (ABAddressBook *) sharedAddressBook
{
    static ABAddressBook *shared = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        shared = [[ABAddressBook alloc] init];
    });
    return shared;
}

- (void) dealloc
{
    [_me release];
    [super dealloc];
}

- (ABPerson *) me
{
    @synchronized(self) {
        if (_me == nil) {
            _me = [[ABPerson alloc] init];
        }
        return [[_me retain] autorelease];
    }
}

- (void) setMe: (ABPerson *) person
{
    @synchronized(self) {
        if (_me != person) {
            [_me release];
            _me = [person retain];
        }
    }
}

- (NSArray *) people
{
    return [NSArray array];
}

- (NSArray *) groups
{
    return [NSArray array];
}

- (ABRecord *) recordForUniqueId: (NSString *) uniqueId
{
    return nil;
}

- (BOOL) save
{
    return YES;
}

- (BOOL) hasUnsavedChanges
{
    return NO;
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
