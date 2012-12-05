/*
This file is part of Darling.

Copyright (C) 2012 Lubos Dolezel
Copyright (C) 2011 Shinichiro Hamaji

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "MachO.h"
#include "MachOLoader.h"
#include "arch.h"
#include "log.h"
#include "binfmt_misc.h"
#include <iostream>
#include <limits.h>
#include <cstdlib>
#include <stdexcept>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <locale.h>
#include <mcheck.h>

char g_darwin_executable_path[PATH_MAX];
char g_loader_path[PATH_MAX];
char g_sysroot[PATH_MAX] = "";
bool g_trampoline = false;

extern "C" char* __loader_path;
char* __loader_path = g_loader_path;

MachO* g_mainBinary = 0;
MachOLoader* g_loader = 0;
int g_argc;
char** g_argv;

static void autoSysrootSearch();

int main(int argc, char** argv, char** envp)
{
	if (argc == 1)
	{
		std::cerr << "This is Darling dyld for " ARCH_NAME ".\n";
		std::cerr << "Copyright (C) 2012 Lubos Dolezel\n"
			"Copyright (C) 2011 Shinichiro Hamaji\n\n";
		
		std::cerr << "Usage: " << argv[0] << " program-path [arguments...]\n\n";
		std::cerr << "Environment variables:\n"
			"\tDYLD_DEBUG=1 - enable debug info (lots of output)\n"
			"\tDYLD_IGN_MISSING_SYMS=1 - replace missing symbol references with a stub function\n"
			"\tDYLD_MTRACE=1 - enable mtrace\n"
			"\tDYLD_TRAMPOLINE=1 - access all bound functions via a debug trampoline\n"
			"\tDYLD_ROOT_PATH=<path> - set the base for library path resolution (overrides autodetection)\n"
			"\tDYLD_BIND_AT_LAUNCH=1 - force dyld to bind all lazy references on startup\n";
		return 1;
	}
	
	try
	{
		if (!::realpath(argv[0], g_loader_path))
			::strcpy(g_loader_path, argv[0]);

		if (argc == 2)
		{
			if (!strcmp(argv[1], "--register"))
			{
				Darling::binfmtRegister(g_loader_path);
				return 0;
			}
			else if (!strcmp(argv[1], "--deregister"))
			{
				Darling::binfmtDeregister();
				return 0;
			}
		}

		if (!::realpath(argv[1], g_darwin_executable_path))
			::strcpy(g_darwin_executable_path, argv[1]);
	
		// setlocale(LC_CTYPE, "");
		if (getenv("DYLD_MTRACE") && atoi(getenv("DYLD_MTRACE")))
			mtrace();
		if (getenv("DYLD_TRAMPOLINE") && atoi(getenv("DYLD_TRAMPOLINE")))
			g_trampoline = true;

		g_mainBinary = MachO::readFile(argv[1], ARCH_NAME);
		
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
		// Modify the argument list so that the dyld name disappears from the process list.
		// The Linux kernel doesn't really support this - it remembers the byte length of the cmdline, which will now decrease.
		// Any app that examines this process' /proc/.../cmdline will from now on see a group of empty arguments after the real arguments.
		// We fix this for NSProcessInfo in libobjcdarwin.
		/*	
		uintptr_t totalLen = argv[argc-1] + strlen(argv[argc-1]) + 1 - argv[0];
		uintptr_t shortenedLen = totalLen - (strlen(argv[0]) + 1);

		memmove(argv[0], argv[1], shortenedLen);
		memset(argv[0]+shortenedLen, 0, totalLen - shortenedLen);

		// Reconstruct the argv array
		for (int pos = 0, index = 1; index < argc-1; pos++)
		{
			if (!argv[0][pos])
				argv[index++] = &argv[0][pos+1];
		}
		*/

		g_argv = argv+1;
		g_argc = argc-1;
		g_loader = new MachOLoader;
		
		autoSysrootSearch();
		bool forceBind = false;
		
		if (g_trampoline || getenv("DYLD_BIND_AT_LAUNCH") != nullptr)
			forceBind = true;
		
		g_loader->run(*g_mainBinary, g_argc, g_argv, envp, forceBind);
		
		delete g_loader;
		g_loader = 0;
		return 2;
	}
	catch (fat_architecture_not_supported& e)
	{
		if (char* p = strstr(argv[0], "/" DYLD_FULL_NAME)) // multilib
		{
			// Try to automatically execute "the other" dyld on multilib systems
			// if that other dyld's platform is supported in the fat binary
			if (std::find(e.archs().begin(), e.archs().end(), ARCH_CROSS_NAME) != e.archs().end())
			{
				strcpy(p+1, DYLD_CROSS_NAME);
				execvp(argv[0], argv);
			}
		}
		
		std::cerr << argv[1] << " is a fat binary, but doesn't support the following architecture: " << ARCH_NAME << std::endl;
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
	if (const char* s = getenv("DYLD_ROOT_PATH"))
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

