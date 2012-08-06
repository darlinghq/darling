#include "MachO.h"
#include "MachOLoader.h"
#include "arch.h"
#include <iostream>
#include <limits.h>
#include <cstdlib>
#include <stdexcept>

char g_darwin_executable_path[PATH_MAX];
char g_loader_path[PATH_MAX];

int main(int argc, char** argv, char** envp)
{
	if (argc == 1)
	{
		std::cerr << "This is Darwin dyld for " ARCH_NAME ". ";
		std::cerr << "Usage: " << argv[0] << " program-path [arguments...]\n";
		return 1;
	}
	
	if (!::realpath(argv[0], g_loader_path))
		::strcpy(g_loader_path, argv[0]);
	if (!::realpath(argv[1], g_darwin_executable_path))
		::strcpy(g_darwin_executable_path, argv[1]);
	
	initSignalHandler();
	//initRename();
	initNoTrampoline();
	//initLibMac();
	//initDlfcn();

	MachO* mainBinary = MachO::readFile(argv[1], ARCH_NAME);
#ifdef __x86_64__
	if (!mainBinary->is64())
	{
		std::cerr << argv[1] << ": Cannot execute binary file.\nThis version of Darwin dyld cannot run binaries for x86.\n";
		return -ENOEXEC;
	}
#else
	if (mainBinary->is64())
	{
		td::cerr << argv[1] << ": Cannot execute binary file.\nThis version of Darwin dyld cannot run binaries for x86-64.\n";
		return -ENOEXEC;
	}
#endif

	try
	{
		MachOLoader loader;
		g_loader = &loader;
		loader.run(*mainBinary, argc-1, argv+1, envp);
		g_loader = 0;
		return 2;
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
}
