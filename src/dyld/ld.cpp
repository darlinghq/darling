#define DARWIN_LD_INTERNAL
#include "MachOLoader.h"
#include "ld.h"
#include "arch.h"
#include "MachO.h"
#include "mutex.h"
#include "trace.h"
#include "FileMap.h"
#include "config.h"
#include "log.h"
#include <unistd.h>
#include <map>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

static Darling::Mutex g_ldMutex;
static std::map<std::string, LoadedLibrary*> g_ldLibraries;
static __thread char g_ldError[256] = "";

extern char g_darwin_executable_path[PATH_MAX];

// TODO: this may need a rework for a 32bit loader on a 64bit system
static const char* g_searchPath[] = {
	LIB_PATH, "/usr/lib", "/usr/local/lib"
};

static void* attemptDlopen(const char* filename, int flag);
static int translateFlags(int flags);

extern MachOLoader* g_loader;
extern char g_darwin_executable_path[PATH_MAX];
extern int g_argc;
extern char** g_argv;
extern FileMap g_file_map;

#define RET_IF(x) { if (void* p = x) return p; }

void* __darwin_dlopen(const char* filename, int flag)
{
	TRACE2(filename, flag);
	
	Darling::MutexLock l(g_ldMutex);
	
	g_ldError[0] = 0;
	
	if (!filename)
	{
		strcpy(g_ldError, "Invalid argument");
		return 0;
	}
	
	flag = translateFlags(flag);
	
	std::string path;
start_search:
	if (*filename == '/')
	{
		path = std::string(filename) + ".so";
		LOG << "Trying " << path << std::endl;
		if (::access(path.c_str(), R_OK) == 0)
			RET_IF( attemptDlopen(path.c_str(), flag) );
		
		path = std::string(LIB_PATH) + filename + ".so";
		if (::access(path.c_str(), R_OK) == 0)
			RET_IF( attemptDlopen(path.c_str(), flag) );
		
		if (::access(filename, R_OK) == 0)
			RET_IF( attemptDlopen(filename, flag) );
		
		path = std::string(LIB_PATH) + filename;
		if (::access(path.c_str(), R_OK) == 0)
			RET_IF( attemptDlopen(path.c_str(), flag) );
		
		if (strcmp(INSTALL_PREFIX, "/usr") != 0)
		{
			// We need to change the prefix in filename if present
			if (strncmp(filename, "/usr", 4) == 0 && strncmp(filename, INSTALL_PREFIX, strlen(INSTALL_PREFIX)) != 0)
			{
				char* name = reinterpret_cast<char*>(alloca( strlen(INSTALL_PREFIX) + strlen(filename) + 1 ));
				strcpy(name, INSTALL_PREFIX);
				strcat(name, filename+4);
				filename = name;
				LOG << "Remapping prefix, loading " << name << " instead\n";
				goto start_search;
			}
		}
	}
	else if (strncmp(filename, "@executable_path", 16) == 0)
	{
		size_t lastSlash;
		path = g_darwin_executable_path;
		
		lastSlash = path.rfind('/');
		if (lastSlash != std::string::npos)
		{
			path.resize(lastSlash);
			path += filename + 16;
		}
		else
		{
			path = filename + 16;
		}
		
		if (::access(path.c_str(), R_OK) == 0)
			RET_IF( attemptDlopen(path.c_str(), flag) );
	}
	else
	{
		for (int i = 0; i < sizeof(g_searchPath) / sizeof(g_searchPath[0]); i++)
		{
			path = std::string(g_searchPath[i]) + "/" + filename + ".so";
			if (::access(path.c_str(), R_OK) == 0)
				RET_IF( attemptDlopen(path.c_str(), flag) );
		}
		for (int i = 0; i < sizeof(g_searchPath) / sizeof(g_searchPath[0]); i++)
		{
			path = std::string(g_searchPath[i]) + "/" + filename;
			if (::access(path.c_str(), R_OK) == 0)
				RET_IF( attemptDlopen(path.c_str(), flag) );
		}
	}
	
	strcpy(g_ldError, "File not found");
	return 0;
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
	return native_flags;
}

static bool isSymlink(const char* path)
{
	struct stat st;
	if (::stat(path, &st) == -1)
		return false;
	return S_ISLNK(st.st_mode);
}

void* attemptDlopen(const char* filename, int flag)
{
	char name[2048];
	
	TRACE2(filename,flag);
	
	// Resolve symlinks so that we don't load the same library multiple times
	if (isSymlink(filename) && ::readlink(filename, name, sizeof name) == -1)
	{
		LOG << "Invalid symlink found: " << filename << std::endl;
		return 0;
	}
	else
		strcpy(name, filename);
	
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
			void* d = ::dlopen(name, RTLD_NOW);
			if (d != 0)
			{
				LoadedLibrary* lib = new LoadedLibrary;
				lib->name = name;
				lib->refCount = 1;
				lib->type = LoadedLibraryNative;
				lib->nativeRef = d;
				//lib->slide = lib->base = 0;
				
				g_ldLibraries[name] = lib;
				return lib;
			}
			else
			{
				LOG << "Library failed to load: " << ::dlerror() << std::endl;
				strcpy(g_ldError, ::dlerror());
				return 0;
			}
		}
		else
		{
			// We're loading a Mach-O library
			try
			{
				MachO* machO = MachO::readFile(name, ARCH_NAME);
				LoadedLibrary* lib = new LoadedLibrary;
				
				lib->name = name;
				lib->refCount = 1;
				lib->type = LoadedLibraryDylib;
				lib->machoRef = machO;
				
				bool global = flag & RTLD_GLOBAL && !(flag & RTLD_LOCAL);
				
				if (!global)
				{
					lib->exports = new Exports;
					g_loader->load(*machO, lib->exports);
				}
				else
					g_loader->load(*machO, 0);
				
				char* apple[2] = { g_darwin_executable_path, 0 };
				g_loader->runPendingInitFuncs(g_argc, g_argv, environ, apple);
				
				g_ldLibraries[name] = lib;
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

void* __darwin_dlsym(void* handle, const char* symbol)
{
	TRACE2(handle, symbol);
	
	Darling::MutexLock l(g_ldMutex);
	g_ldError[0] = 0;
	
	if (handle == DARWIN_RTLD_NEXT || handle == DARWIN_RTLD_SELF || handle == DARWIN_RTLD_MAIN_ONLY || !handle)
	{
		LOG << "Cannot yet handle certain DARWIN_RTLD_* search strategies, falling back to RTLD_DEFAULT\n";
		handle = DARWIN_RTLD_DEFAULT;
	}

handling:
	if (handle == DARWIN_RTLD_DEFAULT)
	{
		// First try native with the __darwin prefix
		void* sym;
		char* buf = reinterpret_cast<char*>(malloc(strlen(symbol+20)));
		strcpy(buf, "__darwin_");
		strcat(buf, symbol);
		
		sym = ::dlsym(RTLD_DEFAULT, buf);
		if (sym)
			return sym;
		
		// Now try Darwin libraries
		const Exports& e = g_loader->getExports();
		Exports::const_iterator itSym = e.find(symbol);
		if (itSym == e.end())
		{
			// Now try without a prefix
			sym = ::dlsym(RTLD_DEFAULT, symbol);
			if (sym)
				return sym;
			
			// Now we fail
			snprintf(g_ldError, sizeof(g_ldError)-1, "Cannot find symbol '%s'", symbol);
			return 0;
		}
		else
			return reinterpret_cast<void*>(itSym->second.addr);
	}
	else
	{
		LoadedLibrary* lib = reinterpret_cast<LoadedLibrary*>(handle);
		/*if (!lib)
		{
			strcpy(g_ldError, "Invalid handle passed to __darwin_dlsym()");
			return 0;
		}*/
		
		if (lib->type == LoadedLibraryNative)
		{
			void* rv = ::dlsym(lib->nativeRef, symbol);
			if (!rv)
				strcpy(g_ldError, ::dlerror());
			return rv;
		}
		else if (!lib->exports)
		{
			// TODO: this isn't 100% correct
			handle = DARWIN_RTLD_DEFAULT;
			goto handling;
		}
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
	
	
	// TODO: implement - examine g_file_map
	strcpy(g_ldError, "Not implemented yet");
	return -1;
}
