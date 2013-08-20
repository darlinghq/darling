#include "libltdl.h"
#include <libdyld/dl_public.h>
#include <libdyld/DylibSearch.h>
#include <util/debug.h>
#include <util/stlutils.h>
#include "ControlFile.h"

static std::string g_extraPaths;

static void replacePathLeaf(std::string& path, const std::string& leaf);

int __darwin_lt_dlinit()
{
	return 0;
}

int __darwin_lt_dlexit()
{
	return 0;
}

lt_dlhandle __darwin_lt_dlopen(const char *filename)
{
	std::string path;
	Darling::DylibSearch search;
	static std::vector<std::string> suffixes = { ".la" };
	
	search.setAdditionalSuffixes(suffixes);
	search.setAdditionalPaths(g_extraPaths);
	
	path = search.resolve(filename, nullptr);
	
	if (string_endsWith(path, ".la"))
	{
		try
		{
			ControlFile la(path);
			std::string soname = la.get("dlname");
			replacePathLeaf(path, soname);
		}
		catch (const std::exception& e)
		{
			Darling::dl_setLastError(e.what());
		}
	}
	
	return (lt_dlhandle) __darwin_dlopen(path.c_str(), RTLD_GLOBAL);
}

lt_dlhandle __darwin_lt_dlopenext(const char *filename)
{
	return __darwin_lt_dlopen(filename);
}

int __darwin_lt_dlclose (lt_dlhandle handle)
{
	return __darwin_dlclose(handle);
}

lt_ptr __darwin_lt_dlsym (lt_dlhandle handle, const char *name)
{
	return __darwin_dlsym(handle, name);
}

const char* __darwin_lt_dlerror()
{
	return __darwin_dlerror();
}

int __darwin_lt_dlpreload (const lt_dlsymlist *preloaded)
{
	STUB();
	return 0;
}

int __darwin_lt_dlpreload_default (const lt_dlsymlist *preloaded)
{
	STUB();
	return 0;
}

int __darwin_lt_dladdsearchdir (const char *search_dir)
{
	if (!g_extraPaths.empty())
		g_extraPaths += ':';
	g_extraPaths += search_dir;
	
	return 0;
}

int __darwin_lt_dlsetsearchpath (const char *search_path)
{
	g_extraPaths = search_path;
	return 0;
}

const char* __darwin_lt_dlgetsearchpath()
{
	return g_extraPaths.c_str();
}

int __darwin_lt_dlmakeresident (lt_dlhandle handle)
{
	STUB();
	return 0;
}

int __darwin_lt_dlisresident (lt_dlhandle handle)
{
	STUB();
	return 0;
}

void replacePathLeaf(std::string& path, const std::string& leaf)
{
	size_t pos = path.rfind('/');
	
	if (pos == std::string::npos)
		path = leaf;
	else
		path.replace(pos+1, path.length() - pos - 1, leaf);
}

