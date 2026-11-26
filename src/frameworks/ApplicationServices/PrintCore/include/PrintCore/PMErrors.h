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

#ifndef _PrintCore_PMErrors_H_
#define _PrintCore_PMErrors_H_

#include <CoreServices/CoreServices.h>

enum {
	kPMNoError = noErr,
	kPMGeneralError = -30870,
	kPMOutOfScope = -30871,
	kPMInvalidParameter = paramErr,
	kPMNoDefaultPrinter = -30872,
	kPMNotImplemented = -30873,
	kPMNoSuchEntry = -30874,
	kPMInvalidPrintSettings = -30875,
	kPMInvalidPageFormat = -30876,
	kPMValueOutOfRange = -30877,
	kPMLockIgnored = -30878,
	kPMInvalidPrintSession = -30879,
	kPMObjectInUse = -30880,
	kPMInvalidPrinter = -30881,
	kPMInvalidPaper = -30893,
	kPMInvalidObject = -30896
};

#endif
