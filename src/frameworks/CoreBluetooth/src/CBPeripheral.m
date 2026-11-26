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

#import <CoreBluetooth/CBPeripheral.h>
#import "CBHostSession.h"
#import "CBInternal.h"

#if __LP64__

@implementation CBPeripheral
{
    id<CBPeripheralDelegate> _delegate;
    NSString *_name;
    CBPeripheralState _state;
    NSNumber *_RSSI;
    NSArray<CBService *> *_services;
    BOOL _canSendWriteWithoutResponse;
    NSString *_bluezPath;
    NSDictionary *_advertisementData;
    CBHostSession *_hostSession;
}

@synthesize delegate = _delegate;
@synthesize name = _name;
@synthesize state = _state;
@synthesize RSSI = _RSSI;
@synthesize services = _services;
@synthesize canSendWriteWithoutResponse = _canSendWriteWithoutResponse;
@synthesize bluezPath = _bluezPath;
@synthesize advertisementData = _advertisementData;
@synthesize hostSession = _hostSession;

- (void)readRSSI
{
    [_hostSession readRSSIForPeripheral:self];
}

- (void)discoverServices:(NSArray<CBUUID *> *)serviceUUIDs
{
    [_hostSession peripheral:self discoverServices:serviceUUIDs];
}

- (void)discoverIncludedServices:(NSArray<CBUUID *> *)includedServiceUUIDs
                       forService:(CBService *)service
{
    [_hostSession peripheral:self discoverIncludedServices:includedServiceUUIDs forService:service];
}

- (void)discoverCharacteristics:(NSArray<CBUUID *> *)characteristicUUIDs
                       forService:(CBService *)service
{
    [_hostSession peripheral:self discoverCharacteristics:characteristicUUIDs forService:service];
}

- (void)discoverDescriptorsForCharacteristic:(CBCharacteristic *)characteristic
{
    [_hostSession peripheral:self discoverDescriptorsForCharacteristic:characteristic];
}

- (void)readValueForCharacteristic:(CBCharacteristic *)characteristic
{
    [_hostSession peripheral:self readValueForCharacteristic:characteristic];
}

- (void)readValueForDescriptor:(CBDescriptor *)descriptor
{
    [_hostSession peripheral:self readValueForDescriptor:descriptor];
}

- (void)writeValue:(NSData *)data
    forCharacteristic:(CBCharacteristic *)characteristic
                  type:(CBCharacteristicWriteType)type
{
    [_hostSession peripheral:self writeValue:data forCharacteristic:characteristic type:type];
}

- (void)writeValue:(NSData *)data forDescriptor:(CBDescriptor *)descriptor
{
    [_hostSession peripheral:self writeValue:data forDescriptor:descriptor];
}

- (void)setNotifyValue:(BOOL)enabled forCharacteristic:(CBCharacteristic *)characteristic
{
    [_hostSession peripheral:self setNotifyValue:enabled forCharacteristic:characteristic];
}

- (NSInteger)maximumWriteValueLengthForType:(CBCharacteristicWriteType)type
{
    (void) type;
    // ATT_MTU is negotiated by BlueZ below this layer. 512 is the largest
    // value CoreBluetooth clients expect from a modern macOS stack.
    return 512;
}

@end

#else // __LP64__

#import <darling-stub-forwarding.h>

#pragma clang diagnostic ignored "-Wobjc-property-implementation"
#pragma clang diagnostic ignored "-Wincomplete-implementation"
#pragma clang diagnostic ignored "-Wprotocol"

@implementation CBPeripheral

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
