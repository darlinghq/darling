/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

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


#ifndef _CS_FILES_H
#define _CS_FILES_H
#include <CoreServices/MacTypes.h>
#include <CoreServices/FileManager.h>

#ifdef __cplusplus
extern "C" {
#endif

#if __LP64__
	typedef int FSIORefNum;
#else
	typedef SInt16 FSIORefNum;
#endif

enum {
	fsCurPerm = 0,
	fsRdPerm = 1,
	fsWrPerm = 2,
	fsRdWrPerm = 3,
	dsRdWrShPerm = 4,
};

OSErr FSGetDataForkName(HFSUniStr255* dataForkName);
OSErr FSGetResourceForkName(HFSUniStr255* rsrcForkName);

#ifdef __cplusplus
}
#endif

#endif
