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

#include <libdyld/DylibSearch.h>
#include <libdyld/MachOMgr.h>
#include <libdyld/MachOObject.h>
#include <libdyld/NativeObject.h>
#include <iostream>
#include <stdexcept>
#include <set>
#include <string>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <libdyld/VirtualPrefix.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_GRAY    "\x1b[1m\x1b[30m"
#define ANSI_COLOR_RESET   "\x1b[0m"

using namespace Darling;

static void resolve(MachOObject* obj, std::set<std::string>& deps);
static const char* c(const char* color);
static void showHelp(const char* argv0);

int main(int argc, char** argv, char** envp)
{
	if (argc != 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
		showHelp(argv[0]);
	
	try
	{
		MachOObject* obj;
		std::set<std::string> deps;
		MachOMgr* mgr = MachOMgr::instance();
		
		mgr->detectSysRootFromPath(argv[1]);
		if (const char* path = getenv("DPREFIX"))
			__prefix_set(path);
		
		mgr->setLoadAnyArchitecture(true);
		
		obj = new MachOObject(__prefix_translate_path(argv[1]));
		
		mgr->add(obj, true);
		
		std::cout << c(ANSI_COLOR_GRAY) << "Resolving dependencies of " << obj->path() << "\n\n" << c(ANSI_COLOR_RESET);
		
		resolve(obj, deps);
		
		mgr->remove(obj);
	}
	catch (const std::exception& e)
	{
		std::cerr << c(ANSI_COLOR_RED) << e.what() << c(ANSI_COLOR_RESET) << std::endl;
		return 1;
	}
}

void resolve(MachOObject* obj, std::set<std::string>& deps)
{
	for (std::string dylib : obj->declaredDependencies())
	{
		LoadableObject* dep;
		
		if (dylib.empty())
			continue;
		
		std::string path = DylibSearch::instance()->resolve(dylib, obj);
		if (path.empty())
		{
			if (deps.find(dylib) == deps.end())
			{
				deps.insert(dylib);
				
				std::cout << dylib << " => " << c(ANSI_COLOR_RED) << "not found" << c(ANSI_COLOR_RESET) << std::endl;
			}
			
			continue;
		}

		dep = LoadableObject::instantiateForPath(path, obj);
		
		if (deps.find(dep->path()) != deps.end())
		{
			dep->delRef();
			continue;
		}
		
		deps.insert(dep->path());
		
		if (MachOObject* mach = dynamic_cast<MachOObject*>(dep))
		{
			std::cout << dylib << " => " << c(ANSI_COLOR_BLUE) << dep->path() << c(ANSI_COLOR_RESET) << std::endl;
			
			resolve(mach, deps);
		}
		else
			std::cout << dylib << " => " << c(ANSI_COLOR_GREEN) << dep->path() << c(ANSI_COLOR_RESET) << std::endl;
	}
}

const char* c(const char* color)
{
	static bool tty = isatty(1);
	return tty ? color : "";
}

void showHelp(const char* argv0)
{
	std::cerr << "Darling dyldd is a tool that recursively displays dependencies of a given Mach-O executable.\n";
	std::cerr << "Copyright (C) 2012-2014 Lubos Dolezel\n\n";
	std::cerr << "Usage: " << argv0 << " executable\n\n";
	std::cerr << "If your terminal supports colors, then native dependencies are " << c(ANSI_COLOR_GREEN) << "green" << c(ANSI_COLOR_RESET)
		<< " and Mach-O dependencies are " << c(ANSI_COLOR_BLUE) << "blue" << c(ANSI_COLOR_RESET) << ".\n";	
	
	exit(1);
}
