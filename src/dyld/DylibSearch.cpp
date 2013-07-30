#include "DylibSearch.h"
#include "config.h"
#include <stdexcept>
#include <iostream>
#include "MachOObject.h"
#include "MachOMgr.h"

DylibSearch::DylibSearch()
{
	try
	{
		m_config = new IniConfig(ETC_DARLING_PATH "/dylib.conf");
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}

DylibSearch* DylibSearch::instance()
{
	static DylibSearch singleton;
	return &singleton;
}

std::string DylibSearch::resolve(std::string dylib, MachOObject* requester)
{
	// expand @rpath, @executable_path and @loader_path
	if (requester != nullptr && dylib[0] == '@')
	{
		if (dylib.compare(0, 16, "@executable_path") == 0)
		{
			MachOObject* mainModule = MachOMgr::instance()->mainModule();
			
			if (!mainModule)
				throw std::runtime_error("Cannot resolve @executable_path without a main module");
			
			dylib.replace(0, 16, mainModule->directory());
		}
		else if (dylib.compare(0, 12, "@loader_path") == 0)
		{
			dylib.replace(0, 12, requester->directory());
		}
		else if (dylib.compare(0, 6, "@rpath") == 0)
		{
			return resolveViaRpath(dylib, requester);
		}
	}
	
	// Search in configuration
	
	// Search in DYLD_LIBRARY_PATH
	
	// Search in sysroot
	
	
}

std::string DylibSearch::filenameFromPath(const std::string& path)
{
	size_t pos = path.rfind('/');
	if (pos == std::string::npos)
		return path;
	else
		return path.substr(pos+1);
}

std::string DylibSearch::resolveViaRpath(std::string name, MachOObject* requester)
{
	for (const char* rpath : requester->runPaths())
	{
		std::string expanded = name;
		std::string result;
		
		expanded.replace(0, 6, rpath);
		result = resolve(expanded, requester);
		
		if (!result.empty())
			return result;
	}
	
	return std::string();
}
