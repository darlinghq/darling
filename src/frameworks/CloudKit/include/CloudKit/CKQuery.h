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

// A search of the records of one type.
@interface CKQuery : NSObject <NSSecureCoding, NSCopying> {
@private
    CKRecordType _recordType;
    NSPredicate *_predicate;
    NSArray<NSSortDescriptor *> *_sortDescriptors;
}

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

- (instancetype)initWithRecordType:(CKRecordType)recordType predicate:(NSPredicate *)predicate NS_REFINED_FOR_SWIFT;

@property (nonatomic, readonly, copy) CKRecordType recordType NS_REFINED_FOR_SWIFT;
@property (nonatomic, readonly, copy) NSPredicate *predicate;
@property (nonatomic, copy, nullable) NSArray<NSSortDescriptor *> *sortDescriptors;

@end

NS_ASSUME_NONNULL_END
