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

enum OpMode { ModeDylibs, ModeSymbols, ModeExports, ModeBinds };

void printBinInfo(const char* path, const char* arch, const char* opt);
OpMode getOpMode(const char* opt);

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " <file> [arch] [option]\n";
		std::cerr << "\nOptions:\n"
			"\t-d --dylibs\tList dylibs (default)\n"
			"\t-s --symbols\tList symbols\n"
			"\t-e --exports\tList exports\n"
			"\t-b --binds\tList binds\n"
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
			for (size_t i = 0; i < macho->dylibs().size(); i++)
			{
				std::cout << "\t" << macho->dylibs()[i] << std::endl;
			}
			break;
		}
		case ModeSymbols:
		{
			std::cout << "Symbols:\n";
			for (size_t i = 0; i < macho->symbols().size(); i++)
			{
				const MachO::Symbol& s = macho->symbols()[i];
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
			for (size_t i = 0; i < macho->exports().size(); i++)
			{
				const MachO::Export* e = macho->exports()[i];
				
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
			for (size_t i = 0; i < macho->binds().size(); i++)
			{
				const MachO::Bind* b = macho->binds()[i];
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
	else
	{
		std::stringstream ss;
		ss << "Unknown option: " << opt;
		throw std::runtime_error(ss.str());
	}
}

