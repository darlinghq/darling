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
#import <CloudKit/CKOperationConfiguration.h>
#import <CloudKit/CKOperationGroup.h>

NS_ASSUME_NONNULL_BEGIN

@class CKContainer;

// Base class of the operations that talk to CloudKit. An operation reports its outcome through the
// completion blocks of the subclass; it does not throw. Without an iCloud account every operation
// completes with CKErrorNotAuthenticated.
@interface CKOperation : NSOperation {
@private
    CKOperationGroup *_group;
    CKOperationID _operationID;
    void (^_longLivedOperationWasPersistedBlock)(void);
    BOOL _usesBackgroundSession;
    CKOperationConfiguration *_configuration;
    BOOL _executing;
    BOOL _finished;
}

- (instancetype)init;

// Never nil: an operation without a configuration returns a default one. Setting a configuration
// copies it, and the quality of service of the operation follows it.
@property (nonatomic, copy, null_resettable) CKOperationConfiguration *configuration;
@property (nonatomic, strong, nullable) CKOperationGroup *group;
@property (nonatomic, readonly, copy) CKOperationID operationID NS_REFINED_FOR_SWIFT;
@property (nonatomic, copy, nullable) void (^longLivedOperationWasPersistedBlock)(void);

// Settings that live in the configuration; kept for source compatibility.
@property (nonatomic, strong, nullable) CKContainer *container;
@property (nonatomic, assign) BOOL allowsCellularAccess;
@property (nonatomic, assign, getter=isLongLived) BOOL longLived;
@property (nonatomic, assign) NSTimeInterval timeoutIntervalForRequest;
@property (nonatomic, assign) NSTimeInterval timeoutIntervalForResource;
@property (nonatomic, assign) BOOL usesBackgroundSession;
@end

NS_ASSUME_NONNULL_END
