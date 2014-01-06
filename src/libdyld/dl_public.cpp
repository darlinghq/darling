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

static std::set<Darling::DlsymHookFunc> g_dlsymHooks;

using namespace Darling;

void Darling::dl_setLastError(const std::string& str)
{
	strncpy(g_lastError, str.c_str(), sizeof(g_lastError));
	g_lastError[sizeof(g_lastError)-1] = 0;
	g_lastErrorRead = false;
}

void* __darwin_dlopen(const char* filename, int flag)
{
	std::string resolved;

	if (!filename)
		return MachOMgr::instance()->mainModule();
	
	resolved = DylibSearch::instance()->resolve(filename, nullptr);

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
				if (MachO::isMachO(resolved.c_str()))
				{
					MachOObject* mach;

					obj = mach = new MachOObject(resolved);
					mach->setNoRecursion(flag & DARWIN_RTLD_FIRST);
				}
				else
				{
					obj = new NativeObject(resolved);
				}

				if (flag & DARWIN_RTLD_NOW)
					obj->setBindAllAtLoad(true);

				obj->load();
			}
			catch (const std::exception& e)
			{
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

int __darwin_dlclose(void* handle)
{
	if (!handle)
		return 1;

	LoadableObject* obj = static_cast<LoadableObject*>(handle);
	obj->delRef();

	return 0;
}

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

	for (Darling::DlsymHookFunc hook : g_dlsymHooks)
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

void* __darwin_dlsym(void* handle, const char* symbol)
{
	std::string name = symbol;
	std::string translated;
	void* addr = nullptr;
	
	LOG << "__darwin_dlsym(): " << symbol << std::endl;

	removeSuffix(name, "$UNIX2003");
	removeSuffix(name, "$DARWIN_EXTSN");
	removeSuffix(name, "$NOCANCEL");
	
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

		if (!translated.empty())
			addr = module->getExportedSymbol(translated.c_str(), false);

		if (!addr)
			addr = module->getExportedSymbol(name.c_str(), false);
	}

	if (!addr)
		dl_setLastError("Symbol not found");

	return addr;
}

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
	STUB();
	return nullptr;
}

const char* NSLibraryNameForModule(NSModule m)
{
	STUB();
	return nullptr;
}

void Darling::registerDlsymHook(DlsymHookFunc func)
{
	g_dlsymHooks.insert(func);
}

void Darling::deregisterDlsymHook(DlsymHookFunc func)
{
	//g_dlsymHooks.erase(func);
}

