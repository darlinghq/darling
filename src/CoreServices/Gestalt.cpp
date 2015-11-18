/*
This file is part of Darling.

Copyright (C) 2012-2013 Lubos Dolezel

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

#include "MacErrors.h"
#include <cstring>
#include <cstdio>
#include <sys/sysctl.h>
#include "Gestalt.h"

OSStatus Gestalt(uint32_t type, int* value)
{
	switch (type)
	{
	case gestaltSystemVersion:
	case gestaltSystemVersionMajor:
	case gestaltSystemVersionMinor:
	case gestaltSystemVersionPatch:
	{
		// We may be mixing the Darwin and OS version, but this API is deprecated anyway
		int name[2] = { CTL_KERN, KERN_OSRELEASE };
		char version[32];
		size_t version_len = sizeof(version) - 1;
		int maj, min, patch;
		
		sysctl(name, 2, version, &version_len, nullptr, 0);
		sscanf(version, "%d.%d.%d", &maj, &min, &patch);
		
		if (type == gestaltSystemVersion)
			*value = 0x100 * maj + 0x10 * min;
		else if (type == gestaltSystemVersionMajor)
			*value = maj;
		else if (type == gestaltSystemVersionMinor)
			*value = min;
		else if (type == gestaltSystemVersionPatch)
			*value = patch;
	}
	case gestaltSysArchitecture:
#ifdef __powerpc__
		*value = gestaltPowerPC;
#elif defined(__x86_64__) || defined(__i386__)
		*value = gestaltIntel;
#endif
		break;
	default:
		*value = 0;
		return unimpErr;
	}
	return 0;
}
