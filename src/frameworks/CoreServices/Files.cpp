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
#include <CoreServices/Files.h>
#include <CoreServices/MacErrors.h>

OSErr FSGetDataForkName(HFSUniStr255* dataForkName)
{
	dataForkName->length = 0;
	return noErr;
}

OSErr FSGetResourceForkName(HFSUniStr255* rsrcForkName)
{
	const char name[] = "RESOURCE_FORK";
	rsrcForkName->length = sizeof(name) - 1;

	for (int i = 0; i < sizeof(name) - 1; i++)
		rsrcForkName->unicode[i] = name[i];

	return noErr;
}
