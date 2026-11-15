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
#import <CloudKit/CKError.h>

NSErrorDomain const CKErrorDomain = @"CKErrorDomain";

NSString * const CKPartialErrorsByItemIDKey = @"CKPartialErrors";
NSString * const CKRecordChangedErrorClientRecordKey = @"ClientRecord";
NSString * const CKRecordChangedErrorServerRecordKey = @"ServerRecord";
NSString * const CKRecordChangedErrorAncestorRecordKey = @"AncestorRecord";
NSString * const CKErrorRetryAfterKey = @"CKRetryAfter";
NSString * const CKErrorUserDidResetEncryptedDataKey = @"CKUserDidResetEncryptedData";
