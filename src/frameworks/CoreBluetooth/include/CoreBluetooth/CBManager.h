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

#ifndef _CBManager_H_
#define _CBManager_H_

#import <Foundation/Foundation.h>

typedef NS_ENUM(NSInteger, CBManagerState) {
    CBManagerStateUnknown = 0,
    CBManagerStatePoweredOff = 1,
    CBManagerStatePoweredOn = 2,
    CBManagerStateResetting = 3,
    CBManagerStateUnauthorized = 4,
    CBManagerStateUnsupported = 5,
};

typedef NS_ENUM(NSInteger, CBManagerAuthorization) {
    CBManagerAuthorizationNotDetermined = 0,
    CBManagerAuthorizationRestricted = 1,
    CBManagerAuthorizationDenied = 2,
    CBManagerAuthorizationAllowedAlways = 3,
};

@interface CBManager : NSObject

@property (nonatomic, readonly) CBManagerState state;
@property (nonatomic, class, readonly) CBManagerAuthorization authorization;
@property (nonatomic, readonly) CBManagerAuthorization authorization;

@end

#endif
