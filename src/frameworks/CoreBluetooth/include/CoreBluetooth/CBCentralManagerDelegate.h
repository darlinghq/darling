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

#ifndef _CBCentralManagerDelegate_H_
#define _CBCentralManagerDelegate_H_

#import <Foundation/Foundation.h>

@class CBCentralManager;
@class CBPeripheral;

@protocol CBCentralManagerDelegate <NSObject>

- (void)centralManagerDidUpdateState:(CBCentralManager *)central;

@optional
- (void)centralManager:(CBCentralManager *)central
    didDiscoverPeripheral:(CBPeripheral *)peripheral
        advertisementData:(NSDictionary<NSString *, id> *)advertisementData
                     RSSI:(NSNumber *)RSSI;
- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral;
- (void)centralManager:(CBCentralManager *)central
    didFailToConnectPeripheral:(CBPeripheral *)peripheral
                         error:(NSError *)error;
- (void)centralManager:(CBCentralManager *)central
    didDisconnectPeripheral:(CBPeripheral *)peripheral
                      error:(NSError *)error;
- (void)centralManager:(CBCentralManager *)central
    willRestoreState:(NSDictionary<NSString *, id> *)dict;

@end

#endif
