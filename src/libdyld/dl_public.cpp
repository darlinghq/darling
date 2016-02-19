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

#include "dl_public.h"
#include "MachOMgr.h"
#include "MachOObject.h"
#include "NativeObject.h"
#include "DylibSearch.h"
#include <string>
#include <cstring>
#include <sstream>
#include <set>
#include <libmach-o/MachO.h>
#include <util/debug.h>

// This would require g++ 4.8 runtime
//static thread_local std::string g_lastError;

#define thread_local __thread

static thread_local char g_lastError[1024];
static thread_local bool g_lastErrorRead = false;

extern "C" int g_argc asm("NXArgc");
extern "C" char** g_argv asm("NXArgv");

static std::set<Darling::DlsymHookFunc>* g_dlsymHooks = new std::set<Darling::DlsymHookFunc>;

using namespace Darling;

void Darling::dl_setLastError(const std::string& str)
{
	strncpy(g_lastError, str.c_str(), sizeof(g_lastError));
	g_lastError[sizeof(g_lastError)-1] = 0;
	g_lastErrorRead = false;
}

__asm__(".symver __darwin_dlopen, dlopen@DARWIN");
void* __darwin_dlopen(const char* filename, int flag)
{
	std::string resolved;
	MachOObject* callerModule;
	void* callerLocation = __builtin_return_address(0);

	if (!filename)
		return MachOMgr::instance()->mainModule();

	callerModule = MachOMgr::instance()->objectForAddress(callerLocation);
	if (!callerModule)
		callerModule = MachOMgr::instance()->mainModule();
	
	resolved = DylibSearch::instance()->resolve(filename, callerModule);

	LOG << "dlopen(): " << filename << " resolved to " << resolved << std::endl;
	if (resolved.empty())
		resolved = filename;

	LoadableObject* obj;

	obj = MachOMgr::instance()->lookup(resolved);

	// Alternative search for native objects
	if (!obj && !MachO::isMachO(resolved.c_str()))
	{
		void* native = ::dlopen(resolved.c_str(), RTLD_NOLOAD);
		if (native)
		{
			obj = MachOMgr::instance()->objectByNativeRef(native);
			::dlclose(native);
		}
	}

	if (!obj)
	{
		if (!(flag & DARWIN_RTLD_NOLOAD))
		{
			try
			{
				obj = LoadableObject::instantiateForPath(resolved, callerModule, flag);

				if (flag & DARWIN_RTLD_NOW)
					obj->setBindAllAtLoad(true);

				obj->load();
			}
			catch (const std::exception& e)
			{
				LOG << "dlopen() failed with an error: " << e.what() << std::endl;
				dl_setLastError(e.what());
				return nullptr;
			}
		}
	}
	else
		obj->addRef();

	if (obj)
	{
		obj->setGlobalExports(flag & DARWIN_RTLD_GLOBAL);
		obj->setNoDelete(flag & DARWIN_RTLD_NODELETE);
	}

	return obj;
}

__asm__(".symver __darwin_dlclose, dlclose@DARWIN");
int __darwin_dlclose(void* handle)
{
	if (!handle)
		return 1;

	LoadableObject* obj = static_cast<LoadableObject*>(handle);
	obj->delRef();

	return 0;
}

__asm__(".symver __darwin_dlerror, dlerror@DARWIN");
const char* __darwin_dlerror(void)
{
	if (g_lastErrorRead || !g_lastError[0])
		return nullptr;

	g_lastErrorRead = true;
	return g_lastError;
}

static std::string processSymbolViaHooks(const std::string& symbol)
{
	char translated[1024];

	strcpy(translated, symbol.c_str());

	for (Darling::DlsymHookFunc hook : *g_dlsymHooks)
	{
		if (hook(translated))
			return translated;
	}

	return std::string();
}

static void removeSuffix(std::string& str, const char* suf)
{
	size_t pos = str.rfind(suf);
	if (pos != std::string::npos)
		str.resize(pos);
}

__asm__(".symver __darwin_dlsym, dlsym@DARWIN");
void* __darwin_dlsym(void* handle, const char* symbol)
{
	std::string name = symbol;
	std::string translated;
	void* addr = nullptr;
	
	LOG << "__darwin_dlsym(): " << symbol << std::endl;

	translated = processSymbolViaHooks(name);

	if (handle == DARWIN_RTLD_DEFAULT)
	{
		if (!translated.empty())
			addr = MachOMgr::instance()->getExportedSymbol(translated.c_str());

		if (!addr)
			addr = MachOMgr::instance()->getExportedSymbol(name.c_str());
	}
	else if (handle == DARWIN_RTLD_NEXT)
	{
		MachOObject* moduleAfter;
		void* callerLocation = __builtin_return_address(0);

		moduleAfter = MachOMgr::instance()->objectForAddress(callerLocation);

		if (!translated.empty())
			addr = MachOMgr::instance()->getExportedSymbol(translated.c_str(), moduleAfter);

		if (!addr)
			addr = MachOMgr::instance()->getExportedSymbol(name.c_str(), moduleAfter);
	}
	else
	{
		LoadableObject* module;

		if (handle == DARWIN_RTLD_MAIN_ONLY)
			module = MachOMgr::instance()->mainModule();
		else if (handle == DARWIN_RTLD_SELF)
		{
			void* callerLocation = __builtin_return_address(0);
			module = MachOMgr::instance()->objectForAddress(callerLocation);
		}
		else
			module = static_cast<LoadableObject*>(handle);

		LOG << "Looking in " << module->name() << std::endl;
		if (!translated.empty())
			addr = module->getExportedSymbol(translated.c_str(), false);

		if (!addr)
			addr = module->getExportedSymbol(name.c_str(), false);
	}

	if (!addr)
		dl_setLastError("Symbol not found");

	return addr;
}

__asm__(".symver __darwin_dladdr, dladdr@DARWIN");
int __darwin_dladdr(void *addr, Dl_info *info)
{
	MachOObject* obj;

	obj = MachOMgr::instance()->objectForAddress(addr);

	if (obj)
		return obj->findSymbolInfo(addr, info);
	else
		return dladdr(addr, info);
}

NSSymbol NSLookupAndBindSymbol(const char* symbolName)
{
	return __darwin_dlsym(DARWIN_RTLD_DEFAULT, symbolName);
}

NSSymbol NSLookupSymbolInModule(NSModule module, const char* symbolName)
{
	return __darwin_dlsym(module, symbolName);
}

const char* NSNameOfSymbol(NSSymbol symbolAddr)
{
	Dl_info info;
	
	if (!__darwin_dladdr(symbolAddr, &info))
		return nullptr;
	
	return info.dli_sname;
}

void* NSAddressOfSymbol(NSSymbol nssymbol)
{
	return nssymbol;
}

int NSIsSymbolNameDefined(const char* name)
{
	return NSLookupAndBindSymbol(name) != nullptr;
}

NSModule NSModuleForSymbol(NSSymbol symbol)
{
	return MachOMgr::instance()->objectForAddress(symbol);
}

int NSIsSymbolNameDefinedInImage(const struct mach_header *image, const char *symbolName)
{
	MachOObject* obj;
	
	obj = MachOMgr::instance()->objectByHeader((struct mach_header*) image);

	if (!obj)
		return false;

	return obj->getExportedSymbol(symbolName, false) != nullptr;
}

const char* NSNameOfModule(NSModule m)
{
	return NSLibraryNameForModule(m);
}

const char* NSLibraryNameForModule(NSModule m)
{
	if (!m)
		return nullptr;
	
	return m->path().c_str();
}

void Darling::registerDlsymHook(DlsymHookFunc func)
{
	g_dlsymHooks->insert(func);
}

void Darling::deregisterDlsymHook(DlsymHookFunc func)
{
	g_dlsymHooks->erase(func);
}

struct ProgramVars
{
	const void* mh;
	int* NXArgcPtr;
	const char*** NXArgvPtr;
	const char*** environPtr;
	const char** __prognamePtr;
};
extern "C" char** __darwin_environ;
void __darling_get_args(int** argc, char**** argv, char**** env, struct ::ProgramVars* vars)
{
	*argc = &g_argc;
	*argv = &g_argv;
	*env = &__darwin_environ;
	vars->NXArgcPtr = &g_argc;
	vars->NXArgvPtr = (const char***) &g_argv;
	vars->environPtr = (const char***) &__darwin_environ;
	vars->__prognamePtr = (const char**) &g_argv[0];

}
extern "C"
void __darling_set_environ(char** _environ)
{
	environ = _environ;
}

static NSLinkEditErrorHandlers g_linkEditErrorHandlers;

NSObjectFileImageReturnCode NSCreateObjectFileImageFromFile(const char* pathName, NSObjectFileImage * objectFileImage)
{
	// TODO: this works here, but would not work for loading images from memory
	*objectFileImage = strdup(pathName);
	return NSObjectFileImageSuccess;
}

NSModule NSLinkModule( NSObjectFileImage objectFileImage, const char* moduleName, uint32_t options)
{
	int opts = 0;
	void* module;
	
	if (options & NSLINKMODULE_OPTION_BINDNOW)
		opts |= DARWIN_RTLD_NOW;
	if (options & NSLINKMODULE_OPTION_PRIVATE)
		opts |= DARWIN_RTLD_LOCAL;
	else
		opts |= DARWIN_RTLD_GLOBAL;
	
	module = __darwin_dlopen((char*) objectFileImage, opts);
	if (!module && !(options & NSLINKMODULE_OPTION_RETURN_ON_ERROR))
	{
		// invoke callback
		if (g_linkEditErrorHandlers.linkEdit)
		{
			g_linkEditErrorHandlers.linkEdit(NSLinkEditOtherError, -1,
					(char*) objectFileImage, __darwin_dlerror());
		}
	}
	
	return NSModule(module);
}

bool NSDestroyObjectFileImage( NSObjectFileImage objectFileImage)
{
	free(objectFileImage);
	return true;
}

bool NSUnLinkModule( NSModule module, uint32_t options)
{
	return __darwin_dlclose(module) == 0;
}

void NSLinkEditError(NSLinkEditErrors *c, int *errorNumber, const char** fileName, const char** errorString)
{
	const char* error;
	
	error = __darwin_dlerror();
	if (error && *error)
	{
		*c = NSLinkEditOtherError;
		*errorNumber = -1;
		*fileName = "<TODO>";
		*errorString = error;
	}
	else
	{
		*c = NSLinkEditUndefinedError;
		*errorNumber = 0;
		*fileName = NULL;
		*errorString = NULL;
	}
}

void NSInstallLinkEditErrorHandlers(const NSLinkEditErrorHandlers *handlers)
{
	g_linkEditErrorHandlers = *handlers;
}
