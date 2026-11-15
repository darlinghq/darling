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

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, CKOperationGroupTransferSize) {
    CKOperationGroupTransferSizeUnknown = 0,
    CKOperationGroupTransferSizeKilobytes = 1,
    CKOperationGroupTransferSizeMegabytes = 2,
    CKOperationGroupTransferSizeTensOfMegabytes = 3,
    CKOperationGroupTransferSizeHundredsOfMegabytes = 4,
    CKOperationGroupTransferSizeGigabytes = 5,
    CKOperationGroupTransferSizeTensOfGigabytes = 6,
    CKOperationGroupTransferSizeHundredsOfGigabytes = 7,
};

// Ties operations together for scheduling and statistics.
@interface CKOperationGroup : NSObject <NSSecureCoding> {
@private
    NSString *_operationGroupID;
    CKOperationConfiguration *_defaultConfiguration;
    NSString *_name;
    NSUInteger _quantity;
    CKOperationGroupTransferSize _expectedSendSize;
    CKOperationGroupTransferSize _expectedReceiveSize;
}

- (instancetype)init;

@property (nonatomic, readonly, copy) NSString *operationGroupID;
// Applies to the operations of the group that have no configuration of their own.
@property (nonatomic, copy) CKOperationConfiguration *defaultConfiguration;
@property (nonatomic, copy, nullable) NSString *name;
// How many operations the group is expected to contain.
@property (nonatomic, assign) NSUInteger quantity;
@property (nonatomic, assign) CKOperationGroupTransferSize expectedSendSize;
@property (nonatomic, assign) CKOperationGroupTransferSize expectedReceiveSize;
@end

NS_ASSUME_NONNULL_END
