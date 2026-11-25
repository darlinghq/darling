/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

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

#ifndef CONTACTS_CNMUTABLEPOSTALADDRESS_H
#define CONTACTS_CNMUTABLEPOSTALADDRESS_H

#import <Contacts/CNPostalAddress.h>

NS_ASSUME_NONNULL_BEGIN

// A postal address whose parts can be changed. Not thread safe. To remove a part when the address is saved,
// set it to an empty string.
API_AVAILABLE(macos(10.11))
@interface CNMutablePostalAddress : CNPostalAddress

@property (copy, NS_NONATOMIC_IOSONLY) NSString *street;
@property (copy, NS_NONATOMIC_IOSONLY) NSString *subLocality API_AVAILABLE(macos(10.12.4));
@property (copy, NS_NONATOMIC_IOSONLY) NSString *city;
@property (copy, NS_NONATOMIC_IOSONLY) NSString *subAdministrativeArea API_AVAILABLE(macos(10.12.4));
@property (copy, NS_NONATOMIC_IOSONLY) NSString *state;
@property (copy, NS_NONATOMIC_IOSONLY) NSString *postalCode;
@property (copy, NS_NONATOMIC_IOSONLY) NSString *country;
@property (copy, NS_NONATOMIC_IOSONLY) NSString *ISOCountryCode;

@end

NS_ASSUME_NONNULL_END

#endif
