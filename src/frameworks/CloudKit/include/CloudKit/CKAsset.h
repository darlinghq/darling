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
#import <CloudKit/CKRecordValue.h>

NS_ASSUME_NONNULL_BEGIN

// A file attached to a record.
@interface CKAsset : NSObject <NSSecureCoding> {
@private
    NSURL *_fileURL;
}

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

// Raises for a nil or non-file URL.
- (instancetype)initWithFileURL:(NSURL *)fileURL;

@property (nonatomic, readonly, copy, nullable) NSURL *fileURL;

@end

@interface CKAsset (CKRecordValue) <CKRecordValue>
@end

NS_ASSUME_NONNULL_END
