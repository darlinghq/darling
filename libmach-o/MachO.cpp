#include "MachO.h"
#include "FatMachO.h"
#include <cstdio>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <string>
#include <map>

MachO* MachO::read(std::string path, const char* arch, bool need_exports)
{
	int fd = ::open(path.c_str(), O_RDONLY);
	if (fd < 0)
	{
		std::cerr << path << ": " << strerror(errno) << std::endl;
		return 0;
	}

	size_t offset = 0, len = 0;
	std::map<std::string, fat_arch> archs;
	if (FatMachO::readFatInfo(fd, &archs))
	{
		std::map<string, fat_arch>::const_iterator found = archs.find(arch);
		if (found == archs.end())
		{
			std::cerr << path << " is a fat binary, but doesn't support the following architecture: " << arch << std::endl;
			return 0;
		}
    
		offset = found->second.offset;
		len = found->second.size;
		LOGF("fat offset=%lu, len=%lu\n", (unsigned long)offset, (unsigned long)len);
	}

	return new MachOImpl(path, fd, offset, len, need_exports);
}
