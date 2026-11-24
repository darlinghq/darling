/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _GCDevice_H_
#define _GCDevice_H_

#import <Foundation/Foundation.h>
#import <GameController/GCProductCategories.h>

@class GCPhysicalInputProfile;

NS_ASSUME_NONNULL_BEGIN

@protocol GCDevice <NSObject>

@property (nonatomic, strong, null_resettable) dispatch_queue_t handlerQueue;
@property (nonatomic, readonly, copy, nullable) NSString *vendorName;
@property (nonatomic, readonly) NSString *productCategory;
@property (nonatomic, strong, readonly, nullable) GCPhysicalInputProfile *physicalInputProfile;

@end

NS_ASSUME_NONNULL_END

#endif
