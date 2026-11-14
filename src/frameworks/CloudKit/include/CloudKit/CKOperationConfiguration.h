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

#import <Foundation/Foundation.h>
#import <CloudKit/CKDefines.h>

NS_ASSUME_NONNULL_BEGIN

@class CKContainer;

// Network and scheduling settings of an operation. Defaults match an operation created without a configuration.
NS_SWIFT_NAME(CKOperation.Configuration)
@interface CKOperationConfiguration : NSObject <NSCopying> {
@private
    CKContainer *_container;
    NSQualityOfService _qualityOfService;
    BOOL _allowsCellularAccess;
    BOOL _longLived;
    NSTimeInterval _timeoutIntervalForRequest;
    NSTimeInterval _timeoutIntervalForResource;
}

// The container to run against; nil stands for the container the operation is added through.
@property (nonatomic, strong, nullable) CKContainer *container;
@property (nonatomic, assign) NSQualityOfService qualityOfService;
@property (nonatomic, assign) BOOL allowsCellularAccess;
@property (nonatomic, assign, getter=isLongLived) BOOL longLived;
// Seconds; a request that makes no progress for this long fails.
@property (nonatomic, assign) NSTimeInterval timeoutIntervalForRequest;
// Seconds; -1 imposes no limit on the whole transfer.
@property (nonatomic, assign) NSTimeInterval timeoutIntervalForResource;

- (instancetype)init;
@end

NS_ASSUME_NONNULL_END
