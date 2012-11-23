#include "bsd.h"
#include "service.h"
#include <cstring>
#include <cstdio>
#include <sys/types.h>
#include <dirent.h>
#include "constants.h"
extern "C" {
#include <CoreFoundation/CFString.h>
}

std::string DarlingTranslateBSDName(const char* name);
bool DarlingFindBSDName(const char* name, char* path);

/*
CFMutableDictionaryRef IOBSDNameMatching(void* iokitPort, unsigned int options, const char* bsdName)
{
	// Search /sys/class/* for a device called bsdName
	bsdName = DarlingTranslateBSDName(bsdName);

	char path[PATH_MAX];

	if (DarlingFindBSDName(bsdName, path))
		return IOServiceSysPath(path);	
	else
		return NULL;
}
*/


std::string DarlingTranslateBSDName(const char* name)
{
	int num;
	char linuxName[100];

	// Macbooks have en0 = eth0, en1 = wlan0
	// We don't handle that yet
	if (sscanf(name, "en%d", &num) == 1)
	{
		sprintf(linuxName, "eth%d", num);
		return linuxName;
	}
	if (sscanf(name, "lo%d", &num) == 1)
	{
		return "lo";
	}
	if (sscanf(name, "stf%d", &num) == 1)
	{
		sprintf(linuxName, "sit%d", num);
		return linuxName;
	}

	return name;
}


