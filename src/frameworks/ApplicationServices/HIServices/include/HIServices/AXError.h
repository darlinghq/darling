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

#ifndef _HISERVICES_AXERROR_H_
#define _HISERVICES_AXERROR_H_

#include <CoreFoundation/CoreFoundation.h>

// Result codes of the accessibility API (Apple's AXError.h)
typedef CF_ENUM(int32_t, AXError) {
	kAXErrorSuccess                             = 0,
	kAXErrorFailure                             = -25200,
	kAXErrorIllegalArgument                     = -25201,
	kAXErrorInvalidUIElement                    = -25202,
	kAXErrorInvalidUIElementObserver            = -25203,
	kAXErrorCannotComplete                      = -25204,
	kAXErrorAttributeUnsupported                = -25205,
	kAXErrorActionUnsupported                   = -25206,
	kAXErrorNotificationUnsupported             = -25207,
	kAXErrorNotImplemented                      = -25208,
	kAXErrorNotificationAlreadyRegistered       = -25209,
	kAXErrorNotificationNotRegistered           = -25210,
	kAXErrorAPIDisabled                         = -25211,
	kAXErrorNoValue                             = -25212,
	kAXErrorParameterizedAttributeUnsupported   = -25213,
	kAXErrorNotEnoughPrecision                  = -25214,
};

#endif
