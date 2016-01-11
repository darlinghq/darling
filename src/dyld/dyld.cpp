/*
This file is part of Darling.

Copyright (C) 2015 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <libdyld/MachOMgr.h>
#include <libdyld/MachOObject.h>
#include <libdyld/NativeObject.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>
#include <util/stlutils.h>
#include <libdyld/arch.h>
#include <regex>
#include <fstream>
#include "dirstructure.h"
#include <libdyld/VirtualPrefix.h>

static void printHelp(const char* argv0);
static bool isELF(const char* path);
static std::string locateBundleExecutable(std::string bundlePath);

using namespace Darling;

int main(int argc, char** argv, char** envp)
{
	if (argc < 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
	{
		printHelp(argv[0]);
		return 1;
	}
	
	if (!HasUserDirectoryStructure())
		SetupUserDirectoryStructure();

	try
	{
		MachOMgr* mgr = MachOMgr::instance();
		std::string executable;

		executable = locateBundleExecutable(argv[1]);
		argv[1] = const_cast<char*>(executable.c_str());

		mgr->detectSysRootFromPath(argv[1]);

		mgr->setPrintInitializers(getenv("DYLD_PRINT_INITIALIZERS") != nullptr);
		mgr->setPrintLibraries(getenv("DYLD_PRINT_LIBRARIES") != nullptr);
#ifdef __arm__ // dyld_stub_binder hasn't been ported to ARM yet
		mgr->setBindAtLaunch(true);
#else
		mgr->setBindAtLaunch(getenv("DYLD_BIND_AT_LAUNCH") != nullptr);
#endif
		mgr->setIgnoreMissingSymbols(getenv("DYLD_IGN_MISSING_SYMS") != nullptr);
		mgr->setPrintSegments(getenv("DYLD_PRINT_SEGMENTS") != nullptr);
		mgr->setPrintBindings(getenv("DYLD_PRINT_BINDINGS") != nullptr);
		mgr->setPrintRpathExpansion(getenv("DYLD_PRINT_RPATHS") != nullptr);
		mgr->setForceFlatNamespace(getenv("DYLD_FORCE_FLAT_NAMESPACE") != nullptr);

		if (const char* path = getenv("DYLD_LIBRARY_PATH"))
			mgr->setLibraryPath(path);
		if (const char* path = getenv("DYLD_ROOT_PATH"))
			mgr->setSysRoot(path);
		if (const char* path = getenv("DYLD_TRAMPOLINE"))
			mgr->setUseTrampolines(true, path);
		if (const char* path = getenv("DPREFIX"))
			__prefix_set(path);

		if (isELF(argv[1]))
		{
			NativeObject* obj;
			typedef int (mainPtr)(int argc, char** argv, char** envp);
			mainPtr* main;

			obj = new NativeObject(__prefix_translate_path(argv[1]));
			obj->load();

			main = (mainPtr*) obj->getExportedSymbol("main", false);

			if (!main)
				throw std::runtime_error("No entry point found");

			exit(main(argc-1, &argv[1], envp));
		}
		else
		{
			MachOObject* obj;

			obj = new MachOObject(__prefix_translate_path(argv[1]));
			if (!obj->isMainModule())
			{
				throw std::runtime_error("This is not a Mach-O executable; dynamic libraries, "
				"kernel extensions and other Mach-O files cannot be executed with dyld");
			}
		
			obj->setCommandLine(argc-1, &argv[1], envp);

			obj->load();
			obj->run();
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "dyld: Cannot execute binary file: " << e.what() << std::endl;
		return ENOSYS;
	}
}

static void printHelp(const char* argv0)
{
	std::cerr << "This is Darling dyld for " ARCH_NAME ", a dynamic loader for Mach-O executables.\n\n";
	std::cerr << "Copyright (C) 2012-2015 Lubos Dolezel\n\n";

	std::cerr << "Usage: " << argv0 << " program-path [arguments...]\n\n";

	std::cerr << "Environment variables:\n"
		"\tDYLD_DEBUG=expr - enable(+) or disable(-) debugging channels (debug, trace, error), e.g. +debug,-error\n"
#ifdef HAS_DEBUG_HELPERS
		"\tDYLD_TRAMPOLINE=file - enable debugging trampolines, with argument info in file\n"
		"\tDYLD_IGN_MISSING_SYMS - replace missing symbols with a stub function\n\n"
#endif
		"\tDYLD_ROOT_PATH=<path> - set the base for library path resolution (overrides autodetection)\n"
		"\tDYLD_BIND_AT_LAUNCH - force dyld to bind all lazy references on startup\n"
		"\tDYLD_PRINT_INITIALIZERS - print initializers when they are invoked\n"
		"\tDYLD_PRINT_LIBRARIES - print libraries when they are loaded\n"
		"\tDYLD_PRINT_SEGMENTS - print segments when they are mapped into memory\n"
		"\tDYLD_PRINT_BINDINGS - print out when binds/ext. relocs are being resolved\n"
		"\tDYLD_PRINT_RPATHS - print @rpath resolution attempts\n\n";
}

static std::string locateBundleExecutable(std::string bundlePath)
{
	std::regex re(".*/([^\\.]+)\\.app/?$", std::regex::icase);
	std::smatch match;
	
	std::string myBundlePath = "./" + bundlePath; // TODO: fix the regexp to work without this
	
	if (std::regex_match(myBundlePath, match, re))
	{
		std::stringstream ss;
		ss << bundlePath;

		if (bundlePath.back() != '/')
			ss << '/';

		ss << "Contents/MacOS/";
		ss << match[1];

		return ss.str();
	}
	else
		return bundlePath;
}

static bool isELF(const char* path)
{
	std::ifstream file(path, std::ios_base::in | std::ios_base::binary);
	uint32_t signature;

	if (!file.is_open())
		return false;

	file.read((char*) &signature, sizeof(signature));
	file.close();

	return signature == *((const uint32_t*) "\177ELF");
}

