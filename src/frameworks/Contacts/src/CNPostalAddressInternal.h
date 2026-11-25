/*
 This file is part of Darling.

 Copyright (C) 2026 Darling developers

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

#ifndef CONTACTS_CNPOSTALADDRESSINTERNAL_H
#define CONTACTS_CNPOSTALADDRESSINTERNAL_H

// Internal to CNPostalAddress.m and CNMutablePostalAddress.m; not installed.

#import <Contacts/CNPostalAddress.h>

#if __LP64__

// The instance variables are declared here, not in the public header (Apple's declares none). They are
// @protected because CNMutablePostalAddress sets them. The 32-bit slice keeps the forwarding stub: its legacy
// runtime cannot declare instance variables in a class extension.
@interface CNPostalAddress () {
@protected
	NSString *_street;
	NSString *_subLocality;
	NSString *_city;
	NSString *_subAdministrativeArea;
	NSString *_state;
	NSString *_postalCode;
	NSString *_country;
	NSString *_ISOCountryCode;
}
@end

// Stores a copy of a part of a postal address in its slot. The header declares every part non-nil and the
// pages say an empty string means "not set"; macOS stores a nil part as an empty string (observed on 26.7.3 and 10.14.4).
static inline void CNPostalAddressSetPart(NSString **slot, NSString *value)
{
	NSString *copy = [(value ?: @"") copy];
	[*slot release];
	*slot = copy;
}

#endif

#endif
