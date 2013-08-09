#include "DylibSearch.h"
#include "config.h"
#include <stdexcept>
#include <iostream>
#include "MachOObject.h"
#include "MachOMgr.h"
#include <regex.h>
#include <unistd.h>
#include <util/stlutils.h>
#include <cassert>

namespace Darling {

DylibSearch::DylibSearch()
	: m_reFrameworkPath("/System/Library/Frameworks/([a-zA-Z0-9\\.]+)/Versions/([a-zA-Z0-9\\.]+)/.*")
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
	if (dylib.empty())
		return std::string();

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
	if (const char* aliasTarget = resolveAlias(dylib))
		return aliasTarget;
	
	// Search in DYLD_LIBRARY_PATH
	std::string ldpath = resolveInLdPath(dylib);
	if (!ldpath.empty())
		return ldpath;

	// Try the path as is
	if (::access(dylib.c_str(), F_OK) == 0)
		return dylib;
	
	// If absolute, search in sysroot
	if (dylib[0] == '/' && !MachOMgr::instance()->sysRoot().empty())
	{
		std::string path = MachOMgr::instance()->sysRoot();
		path += '/';
		path += dylib;

		if (::access(path.c_str(), F_OK) == 0)
			return path;
	}

	return std::string();
}

std::string DylibSearch::resolveInLdPath(std::string name)
{
	std::string ldpath = MachOMgr::instance()->libraryPath();
	std::vector<std::string> elems;

	if (ldpath.empty())
		return std::string();

	elems = string_explode(name, ':');

	for (const std::string& e : elems)
	{
		std::string path = e + "/" + name;

		if (::access(path.c_str(), F_OK) == 0)
			return path;
	}

	return std::string();
}

const char* DylibSearch::resolveAlias(const std::string& library)
{
	if (!m_config)
		return nullptr;

	if (m_config->hasSection("dylibs"))
	{
		const IniConfig::ValueMap* m = m_config->getSection("dylibs");
		auto it = m->find(library);
		if (it != m->end())
			return it->second.c_str();
	}

	if (m_reFrameworkPath.matches(library))
	{
		std::string name, version;
		
		name = m_reFrameworkPath.group(1);
		version = m_reFrameworkPath.group(2);
			
		if (m_config->hasSection(name))
		{
			const IniConfig::ValueMap* m = m_config->getSection(name);
			auto it = m->find(version);
				
			if (it != m->end())
				return it->second.c_str();
		}
	}
	return nullptr;
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
	bool print = MachOMgr::instance()->printRpathExpansion();

	for (const char* rpath : requester->runPaths())
	{
		std::string expanded = name;
		std::string result;
		
		expanded.replace(0, 6, rpath);

		if (print)
			std::cerr << "dyld: Expanding \"" << name << "\" -> \"" << expanded << "\"\n";

		result = resolve(expanded, requester);
		
		if (!result.empty())
		{
			if (print)
				std::cerr << "dyld: @rpath expansion successful\n";
			return result;
		}
		else
		{
			if (print)
				std::cerr << "dyld: @rpath expansion not successful\n";
		}
	}
	
	return std::string();
}

} // namespace Darling
