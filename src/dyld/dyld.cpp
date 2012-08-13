#include "MachO.h"
#include "MachOLoader.h"
#include "arch.h"
#include <iostream>
#include <limits.h>
#include <cstdlib>
#include <stdexcept>
#include <cstring>

char g_darwin_executable_path[PATH_MAX];
char g_loader_path[PATH_MAX];
MachO* g_mainBinary = 0;
MachOLoader* g_loader = 0;
int g_argc;
char** g_argv;

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
	
	//initSignalHandler();
	//initRename();
	//initNoTrampoline();
	//initLibMac();
	//initDlfcn();

	g_mainBinary = MachO::readFile(argv[1], ARCH_NAME);
#ifdef __x86_64__
	if (!g_mainBinary->is64())
	{
		std::cerr << argv[1] << ": Cannot execute binary file.\nThis version of Darwin dyld cannot run binaries for x86.\n";
		return -ENOEXEC;
	}
#else
	if (g_mainBinary->is64())
	{
		td::cerr << argv[1] << ": Cannot execute binary file.\nThis version of Darwin dyld cannot run binaries for x86-64.\n";
		return -ENOEXEC;
	}
#endif

	try
	{
		g_loader = new MachOLoader;
		g_argv = argv+1;
		g_argc = argc-1;
		g_loader->run(*g_mainBinary, argc-1, argv+1, envp);
		
		delete g_loader;
		g_loader = 0;
		return 2;
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
}
