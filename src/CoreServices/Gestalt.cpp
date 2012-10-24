#include "MacErrors.h"
#include "../libSystem/libc/sysctl.h"
#include <cstring>
#include <cstdio>
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
		
		__darwin_sysctl(name, 2, version, &version_len, nullptr, 0);
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
