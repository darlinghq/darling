/*
 * This file is part of Darling.
 *
 * Copyright (C) 2019 Lubos Dolezel
 * Copyright (C) 2026 Darling contributors
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _CBPeripheral_H_
#define _CBPeripheral_H_

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CBPeer.h>
#import <CoreBluetooth/CBCharacteristic.h>
#import <CoreBluetooth/CBPeripheralDelegate.h>

@class CBService;
@class CBCharacteristic;
@class CBDescriptor;
@class CBUUID;

typedef NS_ENUM(NSInteger, CBPeripheralState) {
    CBPeripheralStateDisconnected = 0,
    CBPeripheralStateConnecting = 1,
    CBPeripheralStateConnected = 2,
    CBPeripheralStateDisconnecting = 3,
};

@interface CBPeripheral : CBPeer

@property (nonatomic, assign) id<CBPeripheralDelegate> delegate;
@property (nonatomic, readonly, copy) NSString *name;
@property (nonatomic, readonly) CBPeripheralState state;
@property (nonatomic, readonly) NSNumber *RSSI;
@property (nonatomic, readonly, copy) NSArray<CBService *> *services;
@property (nonatomic, readonly) BOOL canSendWriteWithoutResponse;

- (void)readRSSI;
- (void)discoverServices:(NSArray<CBUUID *> *)serviceUUIDs;
- (void)discoverIncludedServices:(NSArray<CBUUID *> *)includedServiceUUIDs
                       forService:(CBService *)service;
- (void)discoverCharacteristics:(NSArray<CBUUID *> *)characteristicUUIDs
                       forService:(CBService *)service;
- (void)discoverDescriptorsForCharacteristic:(CBCharacteristic *)characteristic;
- (void)readValueForCharacteristic:(CBCharacteristic *)characteristic;
- (void)readValueForDescriptor:(CBDescriptor *)descriptor;
- (void)writeValue:(NSData *)data
    forCharacteristic:(CBCharacteristic *)characteristic
                  type:(CBCharacteristicWriteType)type;
- (void)writeValue:(NSData *)data forDescriptor:(CBDescriptor *)descriptor;
- (void)setNotifyValue:(BOOL)enabled forCharacteristic:(CBCharacteristic *)characteristic;
- (NSInteger)maximumWriteValueLengthForType:(CBCharacteristicWriteType)type;

@end

#endif
