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

#include <CarbonCore/MacErrors.h>
#include <cstring>
#include <cstdio>
#include <sys/sysctl.h>
#include <CarbonCore/Gestalt.h>

OSStatus Gestalt(uint32_t type, int* value)
{
	if (!value)
		return paramErr;
	*value = 0;
	switch (type)
	{
	case gestaltSystemVersion:
	case gestaltSystemVersionMajor:
	case gestaltSystemVersionMinor:
	case gestaltSystemVersionPatch:
	{
		// The selectors report the macOS product version, the same string
		// NSProcessInfo and SystemVersion.plist report, never the Darwin
		// kernel release (KERN_OSRELEASE).
		char version[64] = {};
		size_t version_len = sizeof(version) - 1;
		int maj = 0, min = 0, patch = 0;

		if (sysctlbyname("kern.osproductversion", version, &version_len, nullptr, 0) != 0)
			return gestaltUnknownErr;
		if (sscanf(version, "%d.%d.%d", &maj, &min, &patch) < 1 || maj < 0 || maj > 99 || min < 0 || patch < 0)
			return gestaltUnknownErr;

		if (type == gestaltSystemVersion)
		{
			// Legacy packed BCD (10.9.5 is 0x1095); minor and bug-fix saturate at 9.
			int packedMajor = ((maj / 10) << 4) | (maj % 10);
			*value = (packedMajor << 8) | ((min > 9 ? 9 : min) << 4) | (patch > 9 ? 9 : patch);
		}
		else if (type == gestaltSystemVersionMajor)
			*value = maj;
		else if (type == gestaltSystemVersionMinor)
			*value = min;
		else
			*value = patch;
		// Without this the version selectors fall into the architecture case below and are overwritten.
		break;
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
