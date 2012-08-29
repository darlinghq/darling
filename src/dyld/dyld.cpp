#include "MachO.h"
#include "MachOLoader.h"
#include "arch.h"
#include "log.h"
#include <iostream>
#include <limits.h>
#include <cstdlib>
#include <stdexcept>
#include <cstring>
#include <cassert>
#include <locale.h>
#include <mcheck.h>

char g_darwin_executable_path[PATH_MAX];
char g_loader_path[PATH_MAX];
char g_sysroot[PATH_MAX] = "";
bool g_trampoline = false;

MachO* g_mainBinary = 0;
MachOLoader* g_loader = 0;
int g_argc;
char** g_argv;

static void autoSysrootSearch();

int main(int argc, char** argv, char** envp)
{
	if (argc == 1)
	{
		std::cerr << "This is Darwin dyld for " ARCH_NAME ". ";
		std::cerr << "Usage: " << argv[0] << " program-path [arguments...]\n\n";
		std::cerr << "Environment variables:\n"
			"\tDYLD_DEBUG=1 - enable debug info (lots of output)\n"
			"\tDYLD_IGN_MISSING_SYMS=1 - replace missing symbol references with a stub function\n"
			"\tDYLD_SYSROOT=<path> - set the base for library path resolution (overrides autodetection)\n"
			"\tDYLD_MTRACE=1 - enable mtrace\n"
			"\tDYLD_TRAMPOLINE=1 - access all bound functions via a debug trampoline\n";
		return 1;
	}
	
	if (!::realpath(argv[0], g_loader_path))
		::strcpy(g_loader_path, argv[0]);
	if (!::realpath(argv[1], g_darwin_executable_path))
		::strcpy(g_darwin_executable_path, argv[1]);
	
	setlocale(LC_CTYPE, "");
	if (getenv("DYLD_MTRACE") && atoi(getenv("DYLD_MTRACE")))
		mtrace();
	if (getenv("DYLD_TRAMPOLINE") && atoi(getenv("DYLD_TRAMPOLINE")))
		g_trampoline = true;

	g_mainBinary = MachO::readFile(argv[1], ARCH_NAME);
	try
	{
		if (!g_mainBinary)
			throw std::runtime_error("Cannot open binary file");
#ifdef __x86_64__
		if (!g_mainBinary->is64())
		{
			std::cerr << argv[1] << ": Cannot execute binary file.\nThis version of Darwin dyld cannot run binaries for x86.\n";
			return -ENOEXEC;
		}
#else
		if (g_mainBinary->is64())
		{
			std::cerr << argv[1] << ": Cannot execute binary file.\nThis version of Darwin dyld cannot run binaries for x86-64.\n";
			return -ENOEXEC;
		}
#endif

		g_loader = new MachOLoader;
		g_argv = argv+1;
		g_argc = argc-1;
		
		autoSysrootSearch();
		
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

extern "C" const char* dyld_getDarwinExecutablePath()
{
	return g_darwin_executable_path;
}

extern "C" const char* dyld_getLoaderPath()
{
	return g_loader_path;
}

void autoSysrootSearch()
{
	if (const char* s = getenv("DYLD_SYSROOT"))
	{
		strncpy(g_sysroot, s, PATH_MAX-1);
		g_sysroot[PATH_MAX-1] = 0;
	}
	else
	{
		std::string path = g_darwin_executable_path;
		size_t pos = path.rfind("/usr/");
	
		LOG << "Looking for SYSROOT signs in " << g_darwin_executable_path << std::endl;
	
		if (pos != std::string::npos && pos > 0)
		{
			std::string sysroot = path.substr(0, pos);
			LOG << "SYSROOT detected to be " << sysroot << std::endl;
			strncpy(g_sysroot, sysroot.c_str(), PATH_MAX-1);
			g_sysroot[PATH_MAX-1] = 0;
		}
	}
}

