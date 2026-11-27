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

#ifndef _CBService_H_
#define _CBService_H_

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CBAttribute.h>

@class CBPeripheral;
@class CBCharacteristic;

@interface CBService : CBAttribute

@property (nonatomic, assign) CBPeripheral *peripheral;
@property (nonatomic, readonly) BOOL isPrimary;
@property (nonatomic, copy) NSArray<CBCharacteristic *> *characteristics;
@property (nonatomic, copy) NSArray<CBService *> *includedServices;

@end

#endif
