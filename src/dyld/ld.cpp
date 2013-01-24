/*
This file is part of Darling.

Copyright (C) 2012 Lubos Dolezel

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

#include "config.h"
#define DARWIN_LD_INTERNAL
#include "MachOLoader.h"
#include "ld.h"
#include "arch.h"
#include "MachO.h"
#include "mutex.h"
#include "trace.h"
#include "FileMap.h"
#include "log.h"
#include "IniConfig.h"
#include "stlutils.h"
#include <unistd.h>
#include <map>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <regex.h>
#include <libgen.h>
#include <cassert>
#include <list>
#include <algorithm>
#include <execinfo.h>
#include "dyld.h"
#include <glob.h>

static Darling::Mutex g_ldMutex;
static std::map<std::string, LoadedLibrary*> g_ldLibraries;
static __thread char g_ldError[256] = "";
static regex_t g_reFrameworkPath;
static LoadedLibrary g_dummyLibrary;

static std::list<std::string> g_searchPath;

static const char* g_suffixes[] = { "$DARWIN_EXTSN", "$UNIX2003", "$NOCANCEL" };
static IniConfig* g_iniConfig = 0;

static void* attemptDlopen(const char* filename, int flag);
static int translateFlags(int flags);
__attribute__((constructor)) static void initLD();
static std::list<Darling::DlsymHookFunc> g_dlsymHooks;

static void* g_libStdCxxDarwin = nullptr;

extern MachOLoader* g_loader;
extern char g_darwin_executable_path[PATH_MAX];
extern char g_sysroot[PATH_MAX];
extern FileMap g_file_map;

#define RET_IF(x) { if (void* p = x) return p; }

static void findSearchpathsWildcard(std::string ldconfig_file_pattern);

static void findSearchpaths(std::string ldconfig_file)
{
	std::ifstream read(ldconfig_file);

	if(!read.is_open())
		LOG << "can't read ldconfig config file - " << ldconfig_file  << std::endl;

	std::string line;
	while(std::getline(read,line))
	{
		line.erase(line.begin(), std::find_if(line.begin(), line.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));	

		if(line.find("include") == 0)
		{
			line = line.substr(7);
			line.erase(line.begin(), std::find_if(line.begin(), line.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));

			if (line.find('*') == std::string::npos)
				findSearchpaths(line);
			else
				findSearchpathsWildcard(line);
		}
		else
		{
			size_t pos = line.find('#');
			if (pos != std::string::npos)
				line.resize(pos);
			// TODO: trim
			g_searchPath.push_back(line);	
		}
	}
}

static void findSearchpathsWildcard(std::string ldconfig_file_pattern)
{
	// Use glob to break down wildcards (ex. "/etc/ld.so.conf.d/*.conf")
	glob_t globbuf;
	
	glob(ldconfig_file_pattern.c_str(), GLOB_NOSORT, nullptr, &globbuf);

	for (size_t i = 0; i < globbuf.gl_pathc; i++)
		findSearchpaths(globbuf.gl_pathv[i]);

	globfree(&globbuf);
}

static void initLD()
{
	int rv = regcomp(&g_reFrameworkPath, "/System/Library/Frameworks/([a-zA-Z0-9\\.]+)/Versions/([a-zA-Z0-9\\.]+)/.*", REG_EXTENDED);
	assert(rv == 0);
	
	g_dummyLibrary.name = "/dev/null";
	g_dummyLibrary.refCount = 1;
	g_dummyLibrary.type = LoadedLibraryDummy;
	g_dummyLibrary.nativeRef = 0;
	g_dummyLibrary.exports = 0;
	
	try
	{
		g_iniConfig = new IniConfig(ETC_DARLING_PATH "/dylib.conf");
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	//add hardcoded library paths
	g_searchPath.push_back(LIB_PATH);
	g_searchPath.push_back("/lib");
	g_searchPath.push_back("/usr/lib");
	//find paths from ldconfig
	findSearchpathsWildcard(LD_SO_CONFIG);
}

static std::string replacePathPrefix(const char* prefix, const char* prefixed, const char* replacement)
{
	std::string path = replacement;
	path += (prefixed + strlen(prefix));
	return path;
}

void* __darwin_dlopen(const char* filename, int flag)
{
	void* callerLocation;
	std::vector<std::string> rpathList;
	const FileMap::ImageMap *mainExecutable, *callerExecutable;
	
	callerLocation = __builtin_return_address(0);
	mainExecutable = g_file_map.mainExecutable();
	callerExecutable = g_file_map.imageMapForAddr(callerLocation);
	
	rpathList.insert(rpathList.end(), mainExecutable->rpaths.begin(), mainExecutable->rpaths.end());
	if (callerExecutable != nullptr && callerExecutable != mainExecutable)
		rpathList.insert(rpathList.end(), callerExecutable->rpaths.begin(), callerExecutable->rpaths.end());
	
	return Darling::DlopenWithContext(filename, flag, rpathList);
}

static const char* resolveAlias(const char* library)
{
	if (!g_iniConfig)
		return nullptr;
	if (g_iniConfig->hasSection("dylibs"))
	{
		const IniConfig::ValueMap* m = g_iniConfig->getSection("dylibs");
		auto it = m->find(library);
		if (it != m->end())
			return it->second.c_str();
	}
	if (strncmp(library, "/System/Library/Frameworks/", 27) == 0)
	{
		regmatch_t match[3];
		if (regexec(&g_reFrameworkPath, library, sizeof(match)/sizeof(match[0]), match, 0) != REG_NOMATCH)
		{
			std::string name, version;
			
			name = std::string(library+match[1].rm_so, match[1].rm_eo - match[1].rm_so);
			version = std::string(library+match[2].rm_so, match[2].rm_eo - match[2].rm_so);
			
			if (g_iniConfig->hasSection(name.c_str()))
			{
				const IniConfig::ValueMap* m = g_iniConfig->getSection(name.c_str());
				auto it = m->find(version);
				
				if (it != m->end())
					return it->second.c_str();
			}
		}
	}
	return nullptr;
}

void* Darling::DlopenWithContext(const char* filename, int flag, const std::vector<std::string>& rpaths, bool* notFoundError)
{
	TRACE2(filename, flag);
	
	Darling::MutexLock l(g_ldMutex);
	std::string path;
	
	g_ldError[0] = 0;
	
	if (notFoundError != nullptr)
		*notFoundError = false;
	
	if (!filename)
	{
		strcpy(g_ldError, "Invalid argument");
		return 0;
	}
	
	flag = translateFlags(flag);

	if (strncmp(filename, "@executable_path", 16) == 0)
	{
		path = replacePathPrefix("@executable_path", filename, g_darwin_executable_path);
		LOG << "Full path after replacing @executable_path: " << path << std::endl;
		if (::access(path.c_str(), R_OK) == 0)
			RET_IF( attemptDlopen(path.c_str(), flag) );
	}
	else if (strncmp(filename, "@loader_path", 12) == 0)
	{
		path = replacePathPrefix("@loader_path", filename, g_loader->getCurrentLoader().c_str());
		LOG << "Current ldr: " << g_loader->getCurrentLoader() << std::endl;
		LOG << "Full path after replacing @loader_path: " << path << std::endl;
		if (::access(path.c_str(), R_OK) == 0)
			RET_IF( attemptDlopen(path.c_str(), flag) );
	}
	else if (strncmp(filename, "@rpath", 6) == 0)
	{
		// @rpath - https://wincent.com/wiki/@executable_path,_@load_path_and_@rpath
		for (std::string rpathSearch : rpaths)
		{
			bool recNotFoundError;
			path = replacePathPrefix("@rpath", filename, rpathSearch.c_str());
			
			RET_IF( DlopenWithContext(path.c_str(), flag, rpaths, &recNotFoundError) );

			// Stop if there was an error, vs just not found
			if (!recNotFoundError)
				return nullptr;
		}
	}
	else
	{
		if (const char* ldp = getenv("DYLD_LIBRARY_PATH"))
		{
			path = std::string(ldp) + "/" + filename;
			if (::access(path.c_str(), R_OK) == 0)
				RET_IF( attemptDlopen(path.c_str(), flag) );
		}

		if (const char* realPath = resolveAlias(filename))
			RET_IF( attemptDlopen(realPath, flag) );

		if (strncmp(filename, "/usr/lib/", 9) == 0)
			filename = filename + 9;
		
		std::list<std::string>::iterator it;
		for (it=g_searchPath.begin(); it!=g_searchPath.end(); ++it)
		{
			path = *it + "/" + filename;
			LOG << "Trying " << path << std::endl;
			if (::access(path.c_str(), R_OK) == 0)
			{
				RET_IF( attemptDlopen(path.c_str(), flag) );
			}
			else
			{
				if (::access((path+".so").c_str(), R_OK) == 0)
					RET_IF( attemptDlopen(path.c_str(), flag) );
			}
		}
		
		// Unlike ld-linux, dyld seems to search in . too
		if (!strchr(filename, '/'))
		{
			if (::access(filename, R_OK) == 0)
				RET_IF( attemptDlopen(filename, flag) );
		}
	}
	
	if (!g_ldError[0])
	{
		snprintf(g_ldError, sizeof(g_ldError)-1, "File not found: %s", filename);
		
		if (notFoundError != nullptr)
			*notFoundError = true;
	}
	
	return nullptr;
}

static int translateFlags(int flag)
{
	int native_flags = 0;
	if (flag & DARWIN_RTLD_LAZY)
		native_flags |= RTLD_LAZY;
	if (flag & DARWIN_RTLD_NOW)
		native_flags |= RTLD_NOW;
	if (flag & DARWIN_RTLD_LOCAL)
		native_flags |= RTLD_LOCAL;
	if (flag & DARWIN_RTLD_GLOBAL)
		native_flags |= RTLD_GLOBAL;
	if (flag & DARWIN_RTLD_NOLOAD)
		native_flags |= RTLD_NOLOAD;
	if (flag & DARWIN_RTLD_NODELETE)
		native_flags |= RTLD_NODELETE;
	if (flag & __DARLING_RTLD_NOBIND)
		native_flags |= __DARLING_RTLD_NOBIND;
	return native_flags;
}

static bool isSymlink(const char* path)
{
	struct stat st;
	if (::lstat(path, &st) == -1)
		return false;
	return S_ISLNK(st.st_mode);
}

void* attemptDlopen(const char* filename, int flag)
{
	char name[2048];
	
	TRACE2(filename,flag);
	
	// We need to run access() here not to use realpath for "libncurses.so.5" for example
	if (::access(filename, R_OK) == 0)
	{
		if (!realpath(filename, name))
		{
			strcpy(g_ldError, strerror(errno));
			return 0;
		}
	}
	else
		strcpy(name, filename);
	
	if (strcmp(name, "/dev/null") == 0)
	{
		// We return a dummy
		return &g_dummyLibrary;
	}
	
	std::map<std::string,LoadedLibrary*>::iterator it = g_ldLibraries.find(name);
	if (it != g_ldLibraries.end())
	{
		// TODO: flags
		it->second->refCount++;
		if (it->second->type == LoadedLibraryNative)
		{
			// add a reference in native ld
			::dlopen(name, RTLD_NOW);
		}
		
		return it->second;
	}
	else
	{
		// actually load the library
		char* p = strstr(name, ".so");
		// we followed a link, so we need to check for .so., too
		if ((p && name+strlen(name)-p == 3) || strstr(name, ".so.")) // endsWith()
		{
			LOG << "Loading a native library " << name << std::endl;
			// We're loading a native library
			// TODO: flags
			int flags = RTLD_NOW | RTLD_GLOBAL;
			void* d;
			
			// TODO: another hack location for libstdc++darwin.so
			if (strcmp(name, "libstdc++darwin.so") == 0)
				flags |= RTLD_DEEPBIND;
			
			d = ::dlopen(name, flags);
			
			if (d != 0)
			{
				LOG << "Native library loaded\n";
				
				LoadedLibrary* lib = new LoadedLibrary;
				lib->name = name;
				lib->refCount = 1;
				lib->type = LoadedLibraryNative;
				lib->nativeRef = d;
				//lib->slide = lib->base = 0;
				
				if (strstr(name, "libstdc++darwin") != nullptr)
					g_libStdCxxDarwin = d;
				
				g_ldLibraries[name] = lib;
				return lib;
			}
			else
			{
				const char* err = ::dlerror();
				LOG << "Native library failed to load: " << err << std::endl;

				if (err && !g_ldError[0]) // we don't overwrite previous errors
				{
					LOG << "Library failed to load: " << err << std::endl;
					strcpy(g_ldError, err);
				}
				return 0;
			}
		}
		else
		{
			LOG << "Loading a Mach-O library\n";
			// We're loading a Mach-O library
			try
			{
				MachO* machO = MachO::readFile(name, ARCH_NAME);
				if (!machO)
				{
					snprintf(g_ldError, sizeof(g_ldError)-1, "Cannot parse Mach-O library: %s", name);
					return 0;
				}
				
				LoadedLibrary* lib = new LoadedLibrary;
				bool nobind = (flag & __DARLING_RTLD_NOBIND) != 0;
				
				lib->name = name;
				lib->refCount = 1;
				lib->type = LoadedLibraryDylib;
				lib->machoRef = machO;
				
				bool global = flag & RTLD_GLOBAL && !(flag & RTLD_LOCAL);
				bool lazy = flag & RTLD_LAZY && !(flag & RTLD_NOW);

				// Insert an entry before doing the full load to prevent recursive loading
				g_ldLibraries[name] = lib;

				//if (!global)
				//{
					lib->exports = new Exports;
					g_loader->load(*machO, name, lib->exports, nobind, lazy);
				//}
				//else
				//	g_loader->load(*machO, name, 0, nobind, lazy);
				
				if (!nobind)
				{
					char* apple[2] = { g_darwin_executable_path, 0 };
					g_loader->runPendingInitFuncs(g_argc, g_argv, environ, apple);
				}
				
				return lib;
			}
			catch (const std::exception& e)
			{
				strcpy(g_ldError, e.what());
				return 0;
			}
		}
	}
}

int __darwin_dlclose(void* handle)
{
	TRACE1(handle);
	
	Darling::MutexLock l(g_ldMutex);
	g_ldError[0] = 0;
	
	if (!handle)
		return 0;
	
	LoadedLibrary* lib = reinterpret_cast<LoadedLibrary*>(handle);
	/*if (!lib)
	{
		strcpy(g_ldError, "Invalid handle passed to __darwin_dlclose()");
		return -1;
	}*/
	
	lib->refCount--;
	
	if (lib->type == LoadedLibraryNative)
		::dlclose(lib->nativeRef);
	if (!lib->refCount)
	{
		if (lib->type == LoadedLibraryDylib)
		{
			// TODO: unmap in g_loader!
			delete lib->exports;
			
			for (std::map<std::string,LoadedLibrary*>::iterator it = g_ldLibraries.begin(); it != g_ldLibraries.end(); it++)
			{
				if (it->second == lib)
				{
					g_ldLibraries.erase(it);
					delete lib;
					break;
				}
			}
		}
	}
	
	return 0;
}

const char* __darwin_dlerror(void)
{
	//TRACE();
	
	return g_ldError[0] ? g_ldError : 0;
}

static const char* translateSymbol(const char* symbol)
{
	bool translated = false;
	static char symbuffer[255];

	strcpy(symbuffer, symbol);

	for (auto f : g_dlsymHooks)
	{
		if (f(symbuffer))
		{
			translated = true;
			break;
		}
	}
	
	if (!translated)
	{
		std::string s = symbol;
		for (int i = 0; i < sizeof(g_suffixes) / sizeof(g_suffixes[0]); i++)
		{
			size_t pos = s.find(g_suffixes[i]);
			if (pos != std::string::npos)
				s.erase(pos, strlen(g_suffixes[i]));
		}
	
		strcpy(symbuffer, s.c_str());
	}

	return symbuffer;
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
	const FileMap::ImageMap* map = g_file_map.imageMapForAddr(symbol);
	
	if (!map)
		return nullptr;
	
	auto it = g_ldLibraries.find(map->filename);
	if (it == g_ldLibraries.end())
		return nullptr;
	
	return it->second;
}

int NSIsSymbolNameDefinedInImage(const struct mach_header *image, const char *symbolName)
{
	// TODO: this inefficient double search will be avoided when we unite LoadedLibrary and FileMap::ImageMap
	const FileMap::ImageMap* map = g_file_map.imageMapForHeader(image);
	LoadedLibrary* ldlib = g_ldLibraries[map->filename];
	
	return __darwin_dlsym(ldlib, symbolName) != nullptr;
}

const char* NSNameOfModule(NSModule m)
{
	if (!m)
		return nullptr;
	
	LoadedLibrary* ldlib = static_cast<LoadedLibrary*>(m);
	return ldlib->name.c_str();
}

const char* NSLibraryNameForModule(NSModule m)
{
	// TODO: we need to check what these two functions return on a real system
	return NSNameOfModule(m);
}

void* __darwin_dlsym(void* handle, const char* symbol, void* extra)
{
	TRACE2(handle, symbol);
	
	Darling::MutexLock l(g_ldMutex);
	g_ldError[0] = 0;
	
	if (!handle)
		handle = DARWIN_RTLD_DEFAULT;

	//if (handle == DARWIN_RTLD_NEXT || handle == DARWIN_RTLD_SELF || handle == DARWIN_RTLD_MAIN_ONLY || !handle)
	//{
	//	LOG << "Cannot yet handle certain DARWIN_RTLD_* search strategies, falling back to RTLD_DEFAULT\n";
	//	handle = DARWIN_RTLD_DEFAULT;
	//}

//handling:
	if (handle == DARWIN_RTLD_DEFAULT || handle == __DARLING_RTLD_STRONG)
	{
		// First try native with the __darwin prefix
		void* sym;
		char* buf = reinterpret_cast<char*>(malloc(strlen(symbol)+20));
		strcpy(buf, "__darwin_");
		strcat(buf, symbol);
		
		sym = ::dlsym(RTLD_DEFAULT, buf);
		if (sym)
			return sym;
		
		// Now try Darwin libraries
		const std::list<Exports*>& le = g_loader->getExports();
		std::list<Exports*>::const_iterator it = le.begin();

		while (it != le.end())
		{
			Exports* e = *it;
			Exports::iterator itSym = e->find(symbol);
		
			if (itSym == e->end())
				itSym = e->find(std::string("_") + symbol); // TODO: WTF?
		
			if (itSym != e->end())
			{
				if (handle != __DARLING_RTLD_STRONG || !(itSym->second.flag & 4))
				{
					MachO::Export& exp = itSym->second;

					// If there is a resolver function, we lazily call it now
					// and update the original stub address with the real one returned.

					if (exp.resolver)
					{
						typedef uintptr_t (ResolverFunc)();
						ResolverFunc* func = (ResolverFunc*) exp.resolver;
						exp.addr = func();
						exp.resolver = 0;
					}
					return reinterpret_cast<void*>(itSym->second.addr);
				}
			}
			it++;
		}
		
		// Now try without a prefix
		const char* translated = translateSymbol(symbol);
		LOG << "Trying " << translated << std::endl;
#if 0 // FIXME: This causes serious regressions!
		for (auto& pair : g_ldLibraries)
		{
			if (pair.second->type == LoadedLibraryNative)
			{
				LOG << "Trying in " << pair.first << std::endl;
				RET_IF(::dlsym(pair.second->nativeRef, translated));
				if (strcmp(translated, symbol) != 0)
					RET_IF(::dlsym(pair.second->nativeRef, symbol));
			}
		}
#endif

		// FIXME: this is very crude, but simple and effective...
		// "St" == namespace std
		if (g_libStdCxxDarwin && strstr(translated, "St") != nullptr)
		{
			RET_IF(::dlsym(g_libStdCxxDarwin, translated));
		}
		
		RET_IF(::dlsym(RTLD_DEFAULT, translated));
		
		if (strcmp(translated, symbol) != 0)
			RET_IF(::dlsym(RTLD_DEFAULT, symbol));

		// Now we fail
		snprintf(g_ldError, sizeof(g_ldError)-1, "Cannot find symbol '%s'", symbol);
		return nullptr;
	}
	else if (handle == DARWIN_RTLD_NEXT)
	{
		// For this, we'll have to rethink and integrate Exports in MachOLoader and LoadedLibrary in here
		abort();
	}
	else if (handle == DARWIN_RTLD_MAIN_ONLY || handle == DARWIN_RTLD_SELF)
	{
		Exports* exports = nullptr;
		if (handle == DARWIN_RTLD_MAIN_ONLY)
			exports = g_loader->getMainExecutableExports();
		else
		{
			void* retaddr;
			const char* name;

			backtrace(&retaddr, 1);

			name = g_file_map.fileNameForAddr(retaddr);
			if (!name)
			{
				strcpy(g_ldError, "Couldn't determine the current module");
				return nullptr;
			}

			auto it = g_ldLibraries.find(name);
			if (it == g_ldLibraries.end())
			{
				strcpy(g_ldError, "Couldn't determine the current module by path");
				return nullptr;
			}

			exports = it->second->exports;
		}

		Exports::iterator itSym = exports->find(symbol);
		if (itSym == exports->end())
		{
			snprintf(g_ldError, sizeof(g_ldError)-1, "Cannot find symbol '%s'", symbol);
			return 0;
		}
		else
			return reinterpret_cast<void*>(itSym->second.addr);
	}
	else
	{
		LoadedLibrary* lib = nullptr;
		
		lib = reinterpret_cast<LoadedLibrary*>(handle);
		
		if (lib->type == LoadedLibraryNative)
		{
			void* rv = ::dlsym(lib->nativeRef, symbol);
			if (!rv)
				strcpy(g_ldError, ::dlerror());
			return rv;
		}
		else if (lib->type == LoadedLibraryDummy)
		{
			snprintf(g_ldError, sizeof(g_ldError)-1, "Cannot find symbol '%s'", symbol);
			return 0;
		}
		//else if (!lib->exports)
		//{
		//	// TODO: this isn't 100% correct
		//	handle = DARWIN_RTLD_DEFAULT;
		//	goto handling;
		//}
		else
		{
			Exports::iterator itSym = lib->exports->find(symbol);
			if (itSym == lib->exports->end())
			{
				snprintf(g_ldError, sizeof(g_ldError)-1, "Cannot find symbol '%s'", symbol);
				return 0;
			}
			else
				return reinterpret_cast<void*>(itSym->second.addr);
		}
	}
}

int __darwin_dladdr(void *addr, Dl_info *info)
{
	TRACE2(addr, info);
	
	Darling::MutexLock l(g_ldMutex);
	g_ldError[0] = 0;
	
	if (!g_file_map.findSymbolInfo(addr, info))
	{
		strcpy(g_ldError, "Specified address not mapped to a Mach-O file");
		return -1;
	}
	if (!info->dli_fbase)
	{
		strcpy(g_ldError, "Specified address not resolvable to a symbol");
		return -1;
	}
	
	return 0;
}

void Darling::registerDlsymHook(Darling::DlsymHookFunc func)
{
	g_dlsymHooks.push_front(func);
}

void Darling::deregisterDlsymHook(Darling::DlsymHookFunc func)
{
	//auto it = std::find(g_dlsymHooks.begin(), g_dlsymHooks.end(), func);
	//if (it != g_dlsymHooks.end())
	//	g_dlsymHooks.erase(it);
}

