#include "MachO.h"
#include "FatMachO.h"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <map>
#include <vector>
#include <stdexcept>

void printBinInfo(const char* path, const char* arch);

int main(int argc, char** argv)
{
	if (argc != 2 && argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <file> [arch]\n";
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
			if (argc == 3)
			{
				printBinInfo(argv[1], argv[2]);
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
			printBinInfo(argv[1], 0);
		::close(fd);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 2;
	}
}

void printBinInfo(const char* path, const char* arch)
{
	MachO* macho = MachO::readFile(path, arch, true);
	if (!macho)
		throw std::runtime_error("Load failed");
	
	std::cout << "Dylibs:\n";
	for (size_t i = 0; i < macho->dylibs().size(); i++)
	{
		std::cout << "\t" << macho->dylibs()[i] << std::endl;
	}
	
	delete macho;
}
