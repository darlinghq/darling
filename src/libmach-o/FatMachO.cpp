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

#include "FatMachO.h"

#ifndef _LARGEFILE64_SOURCE
#	define _LARGEFILE64_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <map>
#include <sstream>

static void fixEndian(uint32_t& p)
{
	uint32_t v = p;
	p = (v << 24) | ((v << 8) & 0x00ff0000) | ((v >> 8) & 0xff00) | (v >> 24);
}

static std::string getArchName(uint32_t a)
{
	switch (a)
	{
	case CPU_TYPE_X86:
		return "i386";
	case CPU_TYPE_X86_64:
		return "x86-64";
	case CPU_TYPE_POWERPC:
		return "ppc";
	case CPU_TYPE_POWERPC64:
		return "ppc64";
	default:
		std::stringstream buf;
		buf << "??? (" << a << ")";
		return buf.str();
    }
}

bool FatMachO::readFatInfo(int fd, std::map<std::string, fat_arch>* fat)
{
	fat_header header;
	ssize_t len;
	off_t maxLen;
	
	if (!fat)
		return false;
	
	maxLen = ::lseek(fd, 0, SEEK_END);
	::lseek(fd, 0, SEEK_SET);
	
	len = ::read(fd, &header, sizeof(header));

	if (len != sizeof(header))
		return false;

	bool be = header.magic == FAT_CIGAM;

	if (be)
		fixEndian(header.nfat_arch);
	
	for (uint32_t i = 0; i < header.nfat_arch; i++)
	{
		fat_arch arch;
		len = ::read(fd, &arch, sizeof(arch));
		if (len != sizeof(arch))
			return false;

		if (be)
		{
			fixEndian(arch.cputype);
			fixEndian(arch.cpusubtype);
			fixEndian(arch.offset);
			fixEndian(arch.size);
			fixEndian(arch.align);
		}
		
		// check size/offset validity
		if (off_t(arch.size) + arch.offset > maxLen)
			continue;
		
		(*fat)[ getArchName(arch.cputype) ] = arch;
    }

	return true;
}
