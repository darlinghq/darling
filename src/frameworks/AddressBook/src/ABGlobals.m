/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#import <AddressBook/ABGlobals.h>

NSString *const kABDatabaseChangedNotification=@"ABDatabaseChangedNotification";
NSString *const kABDatabaseChangedExternallyNotification=@"ABDatabaseChangedExternallyNotification";

NSString *const kABAddressProperty=@"ABAddressProperty";
NSString *const kABBirthdayProperty=@"ABBirthdayProperty";
NSString *const kABDepartmentProperty=@"ABDepartmentProperty";
NSString *const kABEmailProperty=@"ABEmailProperty";
NSString *const kABFirstNameProperty=@"ABFirstNameProperty";
NSString *const kABInstantMessageProperty=@"ABInstantMessageProperty";
NSString *const kABJobTitleProperty=@"ABJobTitleProperty";
NSString *const kABLastNameProperty=@"ABLastNameProperty";
NSString *const kABMaidenNameProperty=@"ABMaidenNameProperty";
NSString *const kABMiddleNameProperty=@"ABMiddleNameProperty";
NSString *const kABNicknameProperty=@"ABNicknameProperty";
NSString *const kABOrganizationProperty=@"ABOrganizationProperty";
NSString *const kABOtherDatesProperty=@"ABOtherDatesProperty";
NSString *const kABPhoneProperty=@"ABPhoneProperty";
NSString *const kABTitleProperty=@"ABTitleProperty";
NSString *const kABURLsProperty=@"ABURLsProperty";

NSString *const kABPersonFlags=@"ABPersonFlags";

NSString *const kABDeletedRecords=@"ABDeletedRecords";
NSString *const kABInsertedRecords=@"ABInsertedRecords";
NSString *const kABUpdatedRecords=@"ABUpdatedRecords";

// This one is not exported by any header but is required by libraries
NSString *const kABRestoreFromBackup=@"ABRestoreFromBackup";
