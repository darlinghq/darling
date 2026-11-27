/*
 * This file is part of Darling.
 *
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

#ifndef CB_HOST_SESSION_H
#define CB_HOST_SESSION_H

#import <CoreBluetooth/CoreBluetooth.h>

// Bridges one CBCentralManager to the host's BlueZ D-Bus objects. Delegate
// callbacks are always delivered on the manager's dispatch queue.
__attribute__((visibility("hidden")))
@interface CBHostSession : NSObject

- (instancetype)initWithManager:(CBCentralManager *)manager queue:(dispatch_queue_t)queue;
- (void)close;

@property (nonatomic, readonly) int state;
@property (nonatomic, readonly) BOOL isScanning;

- (void)scanForServices:(NSArray<CBUUID *> *)services allowDuplicates:(BOOL)allowDuplicates;
- (void)stopScan;
- (void)connectPeripheral:(CBPeripheral *)peripheral;
- (void)cancelPeripheralConnection:(CBPeripheral *)peripheral;
- (NSArray<CBPeripheral *> *)peripheralsWithIdentifiers:(NSArray<NSUUID *> *)identifiers;
- (NSArray<CBPeripheral *> *)connectedPeripheralsWithServices:(NSArray<CBUUID *> *)services;

- (void)peripheral:(CBPeripheral *)peripheral discoverServices:(NSArray<CBUUID *> *)services;
- (void)peripheral:(CBPeripheral *)peripheral
    discoverIncludedServices:(NSArray<CBUUID *> *)services
                  forService:(CBService *)service;
- (void)peripheral:(CBPeripheral *)peripheral
    discoverCharacteristics:(NSArray<CBUUID *> *)characteristics
                 forService:(CBService *)service;
- (void)peripheral:(CBPeripheral *)peripheral
    discoverDescriptorsForCharacteristic:(CBCharacteristic *)characteristic;
- (void)readRSSIForPeripheral:(CBPeripheral *)peripheral;
- (void)peripheral:(CBPeripheral *)peripheral readValueForCharacteristic:(CBCharacteristic *)characteristic;
- (void)peripheral:(CBPeripheral *)peripheral readValueForDescriptor:(CBDescriptor *)descriptor;
- (void)peripheral:(CBPeripheral *)peripheral
       writeValue:(NSData *)data
    forCharacteristic:(CBCharacteristic *)characteristic
                 type:(CBCharacteristicWriteType)type;
- (void)peripheral:(CBPeripheral *)peripheral
       writeValue:(NSData *)data
      forDescriptor:(CBDescriptor *)descriptor;
- (void)peripheral:(CBPeripheral *)peripheral
      setNotifyValue:(BOOL)enabled
   forCharacteristic:(CBCharacteristic *)characteristic;

@end

#endif
