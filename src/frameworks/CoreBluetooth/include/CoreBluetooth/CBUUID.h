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

#ifndef _CBUUID_H_
#define _CBUUID_H_

#import <Foundation/Foundation.h>

@interface CBUUID : NSObject <NSCopying>

+ (CBUUID *)UUIDWithString:(NSString *)theString;
+ (CBUUID *)UUIDWithData:(NSData *)theData;
+ (CBUUID *)UUIDWithNSUUID:(NSUUID *)theNSUUID;

@property (nonatomic, readonly, copy) NSData *data;
@property (nonatomic, readonly, copy) NSString *UUIDString;

@end

#endif
