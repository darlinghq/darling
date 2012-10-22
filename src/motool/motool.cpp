/*
 * This file is part of Darling.
 *
 * Copyright (C) 2012 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 * */

#include "MachO.h"
#include "FatMachO.h"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <map>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <iomanip>

enum OpMode { ModeDylibs, ModeSymbols, ModeExports, ModeBinds, ModeSegments };

void printBinInfo(const char* path, const char* arch, const char* opt);
OpMode getOpMode(const char* opt);
std::string protString(int prot);

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "Mach Object File Format (Mach-O) Tool\n";
		std::cerr << "Usage: " << argv[0] << " <file> [arch] [option]\n";
		std::cerr << "\nOptions:\n"
			"\t-d --dylibs\tList dylibs (default)\n"
			"\t-s --symbols\tList symbols\n"
			"\t-e --exports\tList exports\n"
			"\t-b --binds\tList binds\n"
			"\t-g --segments\tList segments and sections\n"
			"\n";
		return 1;
	}
	
	try
	{
		int fd = ::open(argv[1], O_RDONLY);
		if (fd == -1)
			throw std::runtime_error("Cannot open file");
		
		std::cout << "Processing file: " << argv[1] << std::endl;
		
		std::map<std::string, fat_arch> archs;
		if (FatMachO::readFatInfo(fd, &archs))
		{
			if (argc >= 3)
			{
				const char* opt = 0;
				if (argc >= 4)
					opt = argv[3];
				printBinInfo(argv[1], argv[2], opt);
			}
			else
			{
				// print fat info
				std::cout << "Fat file architectures:\n";
				
				for (std::map<std::string, fat_arch>::iterator it = archs.begin(); it != archs.end(); it++)
				{
					std::cout << "\t" << it->first << std::endl;
				}
				std::cout << "\nPass the architecture name as the second argument to get more info on that subfile.\n";
			}
		}
		else
		{
			const char* opt = 0;
			if (argc >= 3)
				opt = argv[2];
			printBinInfo(argv[1], 0, opt);
		}
		
		::close(fd);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 2;
	}
}

template<typename SegType> void printSegments(std::vector<SegType*> segs)
{
	for (auto* seg : segs)
	{
		std::cout << "Name: " << seg->segname << std::endl;
		std::cout << "Addr: 0x" << seg->vmaddr << std::endl;
		std::cout << "Offset in file: 0x" << seg->fileoff << std::endl;
		std::cout << "Size in file: 0x" << seg->filesize << std::endl;
		std::cout << "Initial protection: " << protString(seg->initprot) << std::endl;
		std::cout << "Max protection: " << protString(seg->maxprot) << std::endl;
		std::cout << std::endl;
	}
}

void printBinInfo(const char* path, const char* arch, const char* opt)
{
	MachO* macho = MachO::readFile(path, arch, true);
	if (!macho)
		throw std::runtime_error("Load failed");
	
	OpMode opmode = getOpMode(opt);
	
	switch (opmode)
	{
		case ModeDylibs:
		{
			std::cout << "Dylibs:\n";
			for (const char* name : macho->dylibs())
			{
				std::cout << "\t" << name << std::endl;
			}
			break;
		}
		case ModeSymbols:
		{
			std::cout << "Symbols:\n";
			for (const MachO::Symbol& s : macho->symbols())
			{
				if (!s.addr)
					continue;
				
				std::cout << '\t' << s.name << " at " << "[0x" << std::hex << std::setfill('0');
				
				if (macho->is64())
					std::cout << std::setw(16);
				else
					std::cout << std::setw(8);
				
				std::cout << s.addr << std::setw(0) << std::dec << ']' << std::endl;
			}
			break;
		}
		case ModeExports:
		{
			std::cout << "Exports:\n";
			for (const MachO::Export* e : macho->exports())
			{
				std::cout << '\t' << e->name << " at " << "[0x" << std::hex << std::setfill('0');
				
				if (macho->is64())
					std::cout << std::setw(16);
				else
					std::cout << std::setw(8);
				
				std::cout << e->addr << std::setw(0) << "] (flag: " << e->flag << ")" << std::dec << std::endl;
			}
			break;
		}
		case ModeBinds:
		{
			std::cout << "Binds:\n";
			for (const MachO::Bind* b : macho->binds())
			{
				std::cout << '\t' << b->name << " at " << "[0x" << std::hex << std::setfill('0');
				
				if (macho->is64())
					std::cout << std::setw(16);
				else
					std::cout << std::setw(8);
				
				std::cout << b->vmaddr << std::setw(0) << "] (addend: " << b->addend << ", type: " << int(b->type) << ", ordinal: " << int(b->ordinal) << ")";
				
				if (b->is_weak)
					std::cout << 'W';
				if (b->is_classic)
					std::cout << 'C';
				
				std::cout << std::dec << std::endl;
			}
			break;
		}
		case ModeSegments:
		{
			std::cout << "Segments:\n" << std::hex;
			printSegments(macho->segments64());
			printSegments(macho->segments());

			break;
		}
	}
	
	delete macho;
}

OpMode getOpMode(const char* opt)
{
	if (!opt)
		return ModeDylibs;
	if (!strcmp(opt, "-d") || !strcmp(opt, "--dylibs"))
		return ModeDylibs;
	else if (!strcmp(opt, "-s") || !strcmp(opt, "--symbols"))
		return ModeSymbols;
	else if (!strcmp(opt, "-e") || !strcmp(opt, "--exports"))
		return ModeExports;
	else if (!strcmp(opt, "-b") || !strcmp(opt, "--binds"))
		return ModeBinds;
	else if (!strcmp(opt, "-g") || !strcmp(opt, "--segments"))
		return ModeSegments;
	else
	{
		std::stringstream ss;
		ss << "Unknown option: " << opt;
		throw std::runtime_error(ss.str());
	}
}

std::string protString(int prot)
{
	char rv[4] = "---";
	if (prot & VM_PROT_READ)
		rv[0] = 'r';
	if (prot & VM_PROT_WRITE)
		rv[1] = 'w';
	if (prot & VM_PROT_EXECUTE)
		rv[2] = 'x';
	return rv;
}

