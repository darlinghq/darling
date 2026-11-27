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

#import <CoreBluetooth/CBCentralManager.h>
#import "CBHostSession.h"
#import "CBInternal.h"

NSString *const CBCentralManagerScanOptionAllowDuplicatesKey = @"kCBScanOptionAllowDuplicates";
NSString *const CBCentralManagerOptionShowPowerAlertKey = @"kCBInitOptionShowPowerAlert"; // value as observed on macOS 26.7.3

#if __LP64__

@implementation CBCentralManager
{
    id<CBCentralManagerDelegate> _delegate;
    CBHostSession *_hostSession;
}

@synthesize delegate = _delegate;

- (instancetype)initWithDelegate:(id<CBCentralManagerDelegate>)delegate queue:(dispatch_queue_t)queue
{
    return [self initWithDelegate:delegate queue:queue options:nil];
}

- (instancetype)initWithDelegate:(id<CBCentralManagerDelegate>)aDelegate
                           queue:(dispatch_queue_t)queue
                         options:(NSDictionary<NSString *, id> *)options
{
    self = [super init];
    if (self == nil)
        return nil;

    _delegate = aDelegate;
    _hostSession = [[CBHostSession alloc] initWithManager:self queue:queue];
    (void) options;
    return self;
}

- (void)dealloc
{
    [_hostSession close];
    [_hostSession release];
    [super dealloc];
}

- (BOOL)isScanning
{
    return [_hostSession isScanning];
}

- (void)scanForPeripheralsWithServices:(NSArray<CBUUID *> *)serviceUUIDs
                               options:(NSDictionary<NSString *, id> *)options
{
    const BOOL allowDuplicates =
        [[options objectForKey:CBCentralManagerScanOptionAllowDuplicatesKey] boolValue];
    [_hostSession scanForServices:serviceUUIDs allowDuplicates:allowDuplicates];
}

- (void)stopScan
{
    [_hostSession stopScan];
}

- (void)connectPeripheral:(CBPeripheral *)peripheral
                  options:(NSDictionary<NSString *, id> *)options
{
    (void) options;
    [_hostSession connectPeripheral:peripheral];
}

- (void)cancelPeripheralConnection:(CBPeripheral *)peripheral
{
    [_hostSession cancelPeripheralConnection:peripheral];
}

- (NSArray<CBPeripheral *> *)retrievePeripheralsWithIdentifiers:(NSArray<NSUUID *> *)identifiers
{
    return [_hostSession peripheralsWithIdentifiers:identifiers];
}

- (NSArray<CBPeripheral *> *)retrieveConnectedPeripheralsWithServices:(NSArray<CBUUID *> *)serviceUUIDs
{
    return [_hostSession connectedPeripheralsWithServices:serviceUUIDs];
}

@end

#else // __LP64__

#import <darling-stub-forwarding.h>

#pragma clang diagnostic ignored "-Wobjc-property-implementation"
#pragma clang diagnostic ignored "-Wincomplete-implementation"
#pragma clang diagnostic ignored "-Wprotocol"

@implementation CBCentralManager

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
