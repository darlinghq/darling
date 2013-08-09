#include <libdyld/DylibSearch.h>
#include <libdyld/MachOMgr.h>
#include <libdyld/MachOObject.h>
#include <libdyld/NativeObject.h>
#include <iostream>
#include <stdexcept>
#include <set>
#include <string>
#include <unistd.h>

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

int main(int argc, char** argv, char** envp)
{
	try
	{
		MachOObject* obj;
		std::set<std::string> deps;
	
		obj = new MachOObject(argv[1]);
		
		MachOMgr::instance()->add(obj, true);
		
		std::cout << c(ANSI_COLOR_GRAY) << "Resolving dependencies of " << obj->path() << "\n\n" << c(ANSI_COLOR_RESET);
		
		resolve(obj, deps);
		
		MachOMgr::instance()->remove(obj);
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
