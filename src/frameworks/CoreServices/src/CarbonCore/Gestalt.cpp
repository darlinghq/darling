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

// Gestalt.h documents the packed form for 10.x as BCD digits: the major
// version in the high byte, then minor and bug-fix in one nibble each, both
// saturated at 9 (10.9.5 is 0x1095, 10.15.7 is 0x1097).
static int packedSystemVersion(int maj, int min, int patch)
{
	// QUIRK: for macOS 11 and later the packed value keeps the same layout,
	// with the major version as two BCD digits in the high byte. macOS
	// 27.0.1 returns 0x2701 for 27.0.1, with noErr. Not documented.
	// Evidence: black-box observation on macOS 27.0.1 arm64, native, under
	// Rosetta, with an SDK 10.15 binary and with SYSTEM_VERSION_COMPAT=1,
	// all identical (tier 2). Saturating minor and bug-fix at 9 for these
	// majors is carried over from the 10.x rule by inference; no such
	// release was available to observe.
	// Test: tests/gestalt-system-version.rb (27.0.1 packs to 0x2701) and
	// tests/src/gestalt_version.c.
	int packedMajor = ((maj / 10) << 4) | (maj % 10);
	return (packedMajor << 8) | ((min > 9 ? 9 : min) << 4) | (patch > 9 ? 9 : patch);
}

OSStatus Gestalt(uint32_t type, int* value)
{
	// No response pointer is not a supported call. macOS does not define
	// what happens (it usually returns noErr but has been seen to fault);
	// reporting paramErr is the conservative choice.
	if (!value)
		return paramErr;
	*value = 0;
	switch (type)
	{
	case gestaltSystemVersion:
	case gestaltSystemVersionMajor:
	case gestaltSystemVersionMinor:
	case gestaltSystemVersionBugFix:
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
			*value = packedSystemVersion(maj, min, patch);
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
		return gestaltUndefSelectorErr;
	}
	return 0;
}
