/*
 * This file is part of Darling.
 *
 * Copyright (C) 2012-2013 Lubos Dolezel
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
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 * */

#include <libmach-o/MachO.h>
#include <libmach-o/FatMachO.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <map>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <iomanip>

enum OpMode { ModeDylibs, ModeSymbols, ModeExports, ModeBinds, ModeSegments, ModeRelocations, ModeRebases };

void printBinInfo(const char* path, const char* arch, const char* opt);
OpMode getOpMode(const char* opt);
std::string protString(int prot);
void processArchArgument(std::string& fileName, std::string& arch);
std::string autoSelectArchitecture(const std::map<std::string, fat_arch>& archs);

const char* g_archPrio[] = 
#ifdef __x86_64__
	{ "x86-64", "x86", nullptr }
#elif defined(__i386__)
	{ "x86", "x86-64", nullptr }
#elif defined(__ppc__)
	{ "ppc", nullptr }
#elif defined(__ppc64__)
	{ "ppc64", nullptr }
#elif defined (__arm__)
	{ "arm", nullptr }
#endif
;

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "Mach Object File Format (Mach-O) Tool\n";
		std::cerr << "Usage: " << argv[0] << " <file>[:arch] [option]\n";
		std::cerr << "\nOptions:\n"
			"\t-d --dylibs\tList dylibs (default)\n"
			"\t-s --symbols\tList symbols\n"
			"\t-e --exports\tList exports\n"
			"\t-b --binds\tList binds\n"
			"\t-r --reloc\tList external relocations\n"
			"\t-a --rebases\tList rebases\n"
			"\t-g --segments\tList segments and sections\n"
			"\t-f --fat\tList FAT Mach-O architectures\n"
			"\n";
		return 1;
	}
	
	try
	{
		std::string fileName = argv[1];
		std::string arch;
		
		processArchArgument(fileName, arch);
		
		int fd = ::open(fileName.c_str(), O_RDONLY);
		if (fd == -1)
			throw std::runtime_error("Cannot open file");
		
		std::cout << "Processing file: " << fileName << std::endl;
		
		std::map<std::string, fat_arch> archs;
		if (FatMachO::readFatInfo(fd, &archs))
		{
			if (archs.empty())
				throw std::runtime_error("FAT Mach-O file contains no subfiles!");
			if (arch.empty())
				arch = autoSelectArchitecture(archs);
			
			if (argc >= 3 && ( !strcmp(argv[2], "-f") || !strcmp(argv[2], "--fat")))
			{
				// print fat info
				std::cout << "Fat file architectures:\n";
				
				for (std::map<std::string, fat_arch>::iterator it = archs.begin(); it != archs.end(); it++)
				{
					std::cout << "\t" << it->first << std::endl;
				}
			}
			else
			{
				const char* opt = 0;
				if (argc >= 3)
					opt = argv[2];
				printBinInfo(fileName.c_str(), arch.c_str(), opt);
			}
		}
		else
		{
			const char* opt = 0;
			if (argc >= 3)
				opt = argv[2];
			printBinInfo(fileName.c_str(), 0, opt);
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

	std::cout << "Platform: " << macho->platform() << std::endl;
	
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
				
				std::cout << b->vmaddr << std::setw(0) << "] ";

				if (!b->is_classic)
					std::cout << "(addend: " << b->addend;
				else
					std::cout << "(value: " << b->value;

				std::cout << ", type: " << int(b->type) << ", ordinal: " << int(b->ordinal) << ")";
				
				if (b->is_weak)
					std::cout << 'W';
				if (b->is_classic)
					std::cout << 'C';
				if (b->is_lazy)
					std::cout << 'L';
				
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
		case ModeRebases:
		{
			std::cout << "Rebases:\n";

			for (MachO::Rebase* r : macho->rebases())
			{
				std::cout << "\t at [0x" << std::hex << std::setfill('0');

				if (macho->is64())
					std::cout << std::setw(16);
				else
					std::cout << std::setw(8);

				std::cout << r->vmaddr << std::dec << std::setw(0) << "]\n";
			}
			break;
		}
		case ModeRelocations:
		{
			std::cout << "External relocations:\n";

			for (MachO::Relocation* r : macho->relocations())
			{
				std::cout << '\t' << r->name << " at " << "[0x" << std::hex << std::setfill('0');
				
				if (macho->is64())
					std::cout << std::setw(16);
				else
					std::cout << std::setw(8);
				
				std::cout << r->addr << std::setw(0) << "] ";

				if (r->pcrel)
					std::cout << "PC-REL";
				std::cout << std::endl;
			}
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
	else if (!strcmp(opt, "-r") || !strcmp(opt, "--reloc"))
		return ModeRelocations;
	else if (!strcmp(opt, "-a") || !strcmp(opt, "--rebases"))
		return ModeRebases;
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

void processArchArgument(std::string& fileName, std::string& arch)
{
	size_t p = fileName.find(':');
	if (p != std::string::npos)
	{
		arch = fileName.substr(p+1);
		fileName.resize(p);
	}
}

std::string autoSelectArchitecture(const std::map<std::string, fat_arch>& archs)
{
	std::string choice;
	for (int i = 0; g_archPrio[i] != nullptr; i++)
	{
		if (archs.find(g_archPrio[i]) != archs.end())
		{
			choice = g_archPrio[i];
			break;
		}
	}
	
	if (choice.empty())
		choice = archs.begin()->first;
	
	return choice;
}

