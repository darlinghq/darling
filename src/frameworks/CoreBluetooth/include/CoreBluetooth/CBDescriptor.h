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

#ifndef _CBDescriptor_H_
#define _CBDescriptor_H_

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CBAttribute.h>

@class CBCharacteristic;

@interface CBDescriptor : CBAttribute

@property (nonatomic, assign) CBCharacteristic *characteristic;
@property (nonatomic, retain) id value;

@end

#endif
