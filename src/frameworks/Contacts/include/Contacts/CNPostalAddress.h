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

#ifndef CONTACTS_CNPOSTALADDRESS_H
#define CONTACTS_CNPOSTALADDRESS_H

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

// An immutable postal address. The parts that an address does not have read as empty strings.
API_AVAILABLE(macos(10.11))
@interface CNPostalAddress : NSObject <NSCopying, NSMutableCopying, NSSecureCoding>

// A street address of several lines is delimited with "\n".
@property (readonly, copy, NS_NONATOMIC_IOSONLY) NSString *street;
@property (readonly, copy, NS_NONATOMIC_IOSONLY) NSString *subLocality API_AVAILABLE(macos(10.12.4));
@property (readonly, copy, NS_NONATOMIC_IOSONLY) NSString *city;
@property (readonly, copy, NS_NONATOMIC_IOSONLY) NSString *subAdministrativeArea API_AVAILABLE(macos(10.12.4));
@property (readonly, copy, NS_NONATOMIC_IOSONLY) NSString *state;
@property (readonly, copy, NS_NONATOMIC_IOSONLY) NSString *postalCode;
@property (readonly, copy, NS_NONATOMIC_IOSONLY) NSString *country;
// ISO 3166-1 alpha-2.
@property (readonly, copy, NS_NONATOMIC_IOSONLY) NSString *ISOCountryCode;

+ (NSString *)localizedStringForKey:(NSString *)key;

@end

NS_ASSUME_NONNULL_END

#endif
