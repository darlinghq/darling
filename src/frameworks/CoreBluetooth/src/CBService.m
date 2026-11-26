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

#import <CoreBluetooth/CBService.h>
#import "CBInternal.h"

#if __LP64__

@implementation CBService
{
    CBPeripheral *_peripheral;
    BOOL _isPrimary;
    NSArray<CBCharacteristic *> *_characteristics;
    NSArray<CBService *> *_includedServices;
    NSString *_bluezPath;
    NSArray<NSString *> *_includedPaths;
}

@synthesize peripheral = _peripheral;
@synthesize isPrimary = _isPrimary;
@synthesize characteristics = _characteristics;
@synthesize includedServices = _includedServices;
@synthesize bluezPath = _bluezPath;
@synthesize includedPaths = _includedPaths;

@end

#else // __LP64__

#import <darling-stub-forwarding.h>

#pragma clang diagnostic ignored "-Wobjc-property-implementation"
#pragma clang diagnostic ignored "-Wincomplete-implementation"

@implementation CBService

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
