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

#ifndef _CBCharacteristic_H_
#define _CBCharacteristic_H_

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CBAttribute.h>

typedef NS_OPTIONS(NSUInteger, CBCharacteristicProperties) {
    CBCharacteristicPropertyBroadcast = 0x01,
    CBCharacteristicPropertyRead = 0x02,
    CBCharacteristicPropertyWriteWithoutResponse = 0x04,
    CBCharacteristicPropertyWrite = 0x08,
    CBCharacteristicPropertyNotify = 0x10,
    CBCharacteristicPropertyIndicate = 0x20,
    CBCharacteristicPropertyAuthenticatedSignedWrites = 0x40,
    CBCharacteristicPropertyExtendedProperties = 0x80,
    CBCharacteristicPropertyNotifyEncryptionRequired = 0x100,
    CBCharacteristicPropertyIndicateEncryptionRequired = 0x200,
};

typedef NS_ENUM(NSInteger, CBCharacteristicWriteType) {
    CBCharacteristicWriteWithResponse = 0,
    CBCharacteristicWriteWithoutResponse = 1,
};

@class CBService;
@class CBDescriptor;

@interface CBCharacteristic : CBAttribute

@property (nonatomic, assign) CBService *service;
@property (nonatomic, copy) NSArray<CBDescriptor *> *descriptors;
@property (nonatomic, copy) NSData *value;
@property (nonatomic, readonly) CBCharacteristicProperties properties;
@property (nonatomic, readonly) BOOL isNotifying;
@property (nonatomic, readonly) BOOL isBroadcasted;

@end

#endif
