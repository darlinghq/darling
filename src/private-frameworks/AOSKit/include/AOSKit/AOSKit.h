/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#ifndef _AOSKit_H_
#define _AOSKit_H_

#include <CoreFoundation/CoreFoundation.h>

extern const CFStringRef kAOSAppleAccountInfoKey;
extern const CFStringRef kAOSErrorDomain;
extern const CFStringRef kAOSMMeInfoKey;
extern const CFStringRef kAOSPersonIDKey;
extern const CFStringRef kAOSTokensKey;
extern const CFStringRef kAOSURLKey;

/* The argument lists are not known; the implementations ignore their arguments. */
CFTypeRef AOSAccountCreate();
CFTypeRef _AOSAccountRetrieveInfo();
Boolean AOSTransactionSuccessful();
CFTypeRef AOSTransactionGetResult();
CFTypeRef AOSTransactionGetError();

#endif
