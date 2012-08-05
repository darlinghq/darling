// Copyright 2011 Shinichiro Hamaji. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   1. Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
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

// A command line tool to extract a Mach-O binary from a fat binary.

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <cstdio>

#include <map>
#include <string>
#include <iostream>

#include <mach-o/fat.h>

#include "FatMachO.h"

static std::string getFileName(std::string path);

int main(int argc, char** argv)
{
	if (argc < 2 || argc > 4)
	{
		std::cerr << "Usage: " << argv[0] << " <file> [arch-out]\n";
		return 1;
	}

	int fd = ::open(argv[1], O_RDONLY);
	if (fd < 0)
	{
		perror("open");
		return 1;
	}

	off_t len = ::lseek(fd, 0, SEEK_END);
	const char* bin = reinterpret_cast<char*>(::mmap(NULL, len, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd, 0));
	
	if (bin == MAP_FAILED)
	{
		perror("mmap");
		return -errno;
	}

	std::map<std::string, fat_arch> archs;
	if (!FatMachO::readFatInfo(fd, &archs))
	{
		std::cerr << "Failed to read the Fat Mach-O header\n";
		return 1;
	}
	
	std::cout << archs.size() << " archs:\n";

	for (std::map<std::string, fat_arch>::const_iterator iter = archs.begin(); iter != archs.end(); ++iter)
	{
		const fat_arch& arch = iter->second;
		std::cout << "cputype=" << arch.cputype << " (" << iter->first << ") "
			"cpusubtype=" << arch.cpusubtype << " offset=" << arch.offset << " size=" << arch.size
			<< " align=" << arch.align << std::endl;
	}

	for (int i = 2; i < argc; i++)
	{
		const char* arch_name = argv[i];
		std::map<std::string, fat_arch>::const_iterator found = archs.find(arch_name);
		if (found == archs.end())
		{
			std::cerr << "Architecture not present: " << arch_name << std::endl;
			continue;
		}

		const fat_arch& arch = found->second;
		std::string outname = getFileName(argv[0]) + "." + arch_name;
		int out = ::open(outname.c_str(), O_WRONLY | O_CREAT, 0777);
		
		if (::write(out, bin + arch.offset, arch.size) != arch.size)
		{
			perror("write");
			return -errno;
		}
		
		::close(out);
	}
}

std::string getFileName(std::string path)
{
	size_t pos = path.rfind('/');
	if (pos == std::string::npos)
		return path;
	
	return path.substr(pos+1);
}

