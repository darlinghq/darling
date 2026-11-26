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

#ifndef _CBCentralManager_H_
#define _CBCentralManager_H_

#import <Foundation/Foundation.h>
#import <dispatch/dispatch.h>
#import <CoreBluetooth/CBManager.h>
#import <CoreBluetooth/CBCentralManagerConstants.h>
#import <CoreBluetooth/CBCentralManagerDelegate.h>

@class CBPeripheral;
@class CBUUID;

@interface CBCentralManager : CBManager

@property (nonatomic, assign) id<CBCentralManagerDelegate> delegate;
@property (nonatomic, readonly) BOOL isScanning;

- (instancetype)initWithDelegate:(id<CBCentralManagerDelegate>)delegate
                           queue:(dispatch_queue_t)queue;
- (instancetype)initWithDelegate:(id<CBCentralManagerDelegate>)delegate
                           queue:(dispatch_queue_t)queue
                         options:(NSDictionary<NSString *, id> *)options;

- (void)scanForPeripheralsWithServices:(NSArray<CBUUID *> *)serviceUUIDs
                               options:(NSDictionary<NSString *, id> *)options;
- (void)stopScan;

- (void)connectPeripheral:(CBPeripheral *)peripheral
                  options:(NSDictionary<NSString *, id> *)options;
- (void)cancelPeripheralConnection:(CBPeripheral *)peripheral;

- (NSArray<CBPeripheral *> *)retrievePeripheralsWithIdentifiers:(NSArray<NSUUID *> *)identifiers;
- (NSArray<CBPeripheral *> *)retrieveConnectedPeripheralsWithServices:(NSArray<CBUUID *> *)serviceUUIDs;

@end

#endif
