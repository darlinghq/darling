// Copyright 2011 Shinichiro Hamaji. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   1. Redistributions of source code must retain the above copyright
//      notice, this list of  conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above
//      copyright notice, this list of conditions and the following
//      disclaimer in the documentation and/or other materials
//      provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY Shinichiro Hamaji ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Shinichiro Hamaji OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
// USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.


#include "MachO.h"
#include "FatMachO.h"
#include "MachOImpl.h"
#include <cstdio>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <string>
#include <cstring>
#include <map>

MachO* MachO::readFile(std::string path, const char* arch, bool need_exports)
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
		std::map<std::string, fat_arch>::const_iterator found = archs.find(arch);
		if (found == archs.end())
		{
			std::vector<std::string> varchs;
			for (auto elem : archs)
				varchs.push_back(elem.first);

			throw fat_architecture_not_supported(varchs);
		}
    
		offset = found->second.offset;
		len = found->second.size;
		// LOGF("fat offset=%lu, len=%lu\n", (unsigned long)offset, (unsigned long)len);
	}

	return new MachOImpl(path.c_str(), fd, offset, len, need_exports);
}

bool MachO::isMachO(const char* path)
{
	bool is_macho = false;
	int fd = ::open(path, O_RDONLY);
	
	if (fd != -1)
	{
		uint32_t magic;
		if (::read(fd, &magic, 4) == 4)
		{
			is_macho = ( magic == MH_MAGIC_64 || magic == MH_MAGIC || magic == FAT_CIGAM || magic == FAT_MAGIC );
		}
		::close(fd);
	}
	return is_macho;
}
