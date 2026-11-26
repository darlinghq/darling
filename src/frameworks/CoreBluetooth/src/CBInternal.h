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

#ifndef CB_INTERNAL_H
#define CB_INTERNAL_H

#import <CoreBluetooth/CoreBluetooth.h>

@class CBHostSession;

// Private write access for the BlueZ-backed objects. These declarations are
// not part of the public SDK surface. A class extension may only redeclare a
// property as readwrite when the public property is readonly; properties that
// are already readwrite are omitted here.
@interface CBManager ()
@property (nonatomic, readwrite) CBManagerState state;
@end

@interface CBPeer ()
@property (nonatomic, copy, readwrite) NSUUID *identifier;
@end

@interface CBAttribute ()
@property (nonatomic, retain, readwrite) CBUUID *UUID;
@end

@interface CBService ()
@property (nonatomic, readwrite) BOOL isPrimary;
@property (nonatomic, copy) NSString *bluezPath;
@property (nonatomic, copy) NSArray<NSString *> *includedPaths;
@end

@interface CBCharacteristic ()
@property (nonatomic, readwrite) CBCharacteristicProperties properties;
@property (nonatomic, readwrite) BOOL isNotifying;
@property (nonatomic, readwrite) BOOL isBroadcasted;
@property (nonatomic, copy) NSString *bluezPath;
@end

@interface CBDescriptor ()
@property (nonatomic, copy) NSString *bluezPath;
@end

@interface CBPeripheral ()
@property (nonatomic, assign, readwrite) CBPeripheralState state;
@property (nonatomic, copy, readwrite) NSString *name;
@property (nonatomic, retain, readwrite) NSNumber *RSSI;
@property (nonatomic, copy, readwrite) NSArray<CBService *> *services;
@property (nonatomic, assign, readwrite) BOOL canSendWriteWithoutResponse;
@property (nonatomic, copy) NSString *bluezPath;
@property (nonatomic, retain) NSDictionary *advertisementData;
@property (nonatomic, assign) CBHostSession *hostSession;
@end

#endif
