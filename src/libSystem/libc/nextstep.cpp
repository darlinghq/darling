#include "config.h"
#include "nextstep.h"
#include <cstring>
#include <unistd.h>
#include <dyld/MachOMgr.h>
#include <dyld/MachOObject.h>

using namespace Darling;

char*** _NSGetEnviron()
{
	MachOObject* mainModule = MachOMgr::instance()->mainModule();

	// When dyld is used as a simple loader, we fall back to direct access to global environ
	if (!mainModule)
		return &environ;
	else
	{
		char*** env;
		mainModule->commandLine(nullptr, nullptr, &env);
		return env;
	}
}

int* _NSGetArgc()
{
	MachOObject* mainModule = MachOMgr::instance()->mainModule();

	if (!mainModule)
		return nullptr;
	else
	{
		int* argc;
		mainModule->commandLine(&argc, nullptr, nullptr);
		return argc;
	}
}

char*** _NSGetArgv()
{
	MachOObject* mainModule = MachOMgr::instance()->mainModule();

	if (!mainModule)
		return nullptr;
	else
	{
		char*** argv;
		mainModule->commandLine(nullptr, &argv, nullptr);
		return argv;
	}
}

