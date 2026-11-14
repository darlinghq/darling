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
#import <CloudKit/CKOperation.h>
#import <CloudKit/CKShareMetadata.h>

NS_ASSUME_NONNULL_BEGIN

// Fetches what share URLs point at, before the shares are accepted.
@interface CKFetchShareMetadataOperation : CKOperation {
@private
    NSArray<NSURL *> *_shareURLs;
    BOOL _shouldFetchRootRecord;
    NSArray<CKRecordFieldKey> *_rootRecordDesiredKeys;
    void (^_perShareMetadataBlock)(NSURL *shareURL, CKShareMetadata * _Nullable shareMetadata, NSError * _Nullable error);
    void (^_fetchShareMetadataCompletionBlock)(NSError * _Nullable operationError);
}

- (instancetype)initWithShareURLs:(NSArray<NSURL *> *)shareURLs;

@property (nonatomic, copy, nullable) NSArray<NSURL *> *shareURLs;
// With YES the metadata includes the root record of the share.
@property (nonatomic, assign) BOOL shouldFetchRootRecord;
// Names of the root record fields to fetch; nil fetches all, an empty array fetches none.
@property (nonatomic, copy, nullable) NSArray<CKRecordFieldKey> *rootRecordDesiredKeys NS_REFINED_FOR_SWIFT;
@property (nonatomic, copy, nullable) void (^perShareMetadataBlock)(NSURL *shareURL, CKShareMetadata * _Nullable shareMetadata, NSError * _Nullable error);
@property (nonatomic, copy, nullable) void (^fetchShareMetadataCompletionBlock)(NSError * _Nullable operationError);
@end

NS_ASSUME_NONNULL_END
