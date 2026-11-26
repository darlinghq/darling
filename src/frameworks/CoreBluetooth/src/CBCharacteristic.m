/*
  This file is part of Darling.

  Copyright (C) 2019 Lubos Dolezel
  Copyright (C) 2026 Darling contributors

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

#import <CoreBluetooth/CBCharacteristic.h>
#import "CBInternal.h"

#if __LP64__

@implementation CBCharacteristic
{
    CBService *_service;
    NSArray<CBDescriptor *> *_descriptors;
    NSData *_value;
    CBCharacteristicProperties _properties;
    BOOL _isNotifying;
    BOOL _isBroadcasted;
    NSString *_bluezPath;
}

@synthesize service = _service;
@synthesize descriptors = _descriptors;
@synthesize value = _value;
@synthesize properties = _properties;
@synthesize isNotifying = _isNotifying;
@synthesize isBroadcasted = _isBroadcasted;
@synthesize bluezPath = _bluezPath;

@end

#else // __LP64__

// The 32-bit slice uses the legacy Objective-C runtime, which cannot synthesize
// the properties backing the BlueZ implementation. Keep the original stub
// behavior there.
#import <darling-stub-forwarding.h>

#pragma clang diagnostic ignored "-Wobjc-property-implementation"
#pragma clang diagnostic ignored "-Wincomplete-implementation"

@implementation CBCharacteristic

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

@end

#endif // __LP64__
