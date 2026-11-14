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
#import <CoreLocation/CLLocation.h>

NS_ASSUME_NONNULL_BEGIN

@class CKReference;
@class CKAsset;

// Marker protocol of the classes a record field may hold.
NS_SWIFT_NAME(__CKRecordObjCValue)
@protocol CKRecordValue <NSObject>
@end

@interface NSString (CKRecordValue) <CKRecordValue>
@end
@interface NSNumber (CKRecordValue) <CKRecordValue>
@end
@interface NSArray (CKRecordValue) <CKRecordValue>
@end
@interface NSDate (CKRecordValue) <CKRecordValue>
@end
@interface NSData (CKRecordValue) <CKRecordValue>
@end
@interface CLLocation (CKRecordValue) <CKRecordValue>
@end

NS_ASSUME_NONNULL_END
