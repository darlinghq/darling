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

#import <AddressBook/ABRecord.h>
#import <CoreFoundation/CoreFoundation.h>
#import <objc/runtime.h>

static const char *kABRecordUniqueIdKey = "kABRecordUniqueIdKey";

@implementation ABRecord

- (id) valueForProperty: (NSString *) property
{
    return nil;
}

- (BOOL) setValue: (id) value forProperty: (NSString *) property
{
    return YES;
}

- (BOOL) removeValueForProperty: (NSString *) property
{
    return YES;
}

- (NSString *) uniqueId
{
    @synchronized(self) {
        NSString *uid = objc_getAssociatedObject(self, kABRecordUniqueIdKey);
        if (uid == nil) {
            CFUUIDRef uuid = CFUUIDCreate(kCFAllocatorDefault);
            uid = [(NSString *)CFUUIDCreateString(kCFAllocatorDefault, uuid) autorelease];
            CFRelease(uuid);
            objc_setAssociatedObject(self, kABRecordUniqueIdKey, uid, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        }
        return uid;
    }
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
