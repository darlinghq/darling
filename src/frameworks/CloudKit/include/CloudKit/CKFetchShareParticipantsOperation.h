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
#import <CloudKit/CKShareParticipant.h>
#import <CloudKit/CKUserIdentityLookupInfo.h>

NS_ASSUME_NONNULL_BEGIN

// Looks up the people a share can be offered to.
@interface CKFetchShareParticipantsOperation : CKOperation {
@private
    NSArray<CKUserIdentityLookupInfo *> *_userIdentityLookupInfos;
    void (^_shareParticipantFetchedBlock)(CKShareParticipant *participant);
    void (^_fetchShareParticipantsCompletionBlock)(NSError * _Nullable operationError);
}

- (instancetype)initWithUserIdentityLookupInfos:(NSArray<CKUserIdentityLookupInfo *> *)userIdentityLookupInfos;

@property (nonatomic, copy, nullable) NSArray<CKUserIdentityLookupInfo *> *userIdentityLookupInfos;
@property (nonatomic, copy, nullable) void (^shareParticipantFetchedBlock)(CKShareParticipant *participant);
@property (nonatomic, copy, nullable) void (^fetchShareParticipantsCompletionBlock)(NSError * _Nullable operationError);
@end

NS_ASSUME_NONNULL_END
