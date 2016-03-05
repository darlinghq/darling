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

#include "DylibSearch.h"
#include "darling-config.h"
#include <stdexcept>
#include <iostream>
#include "MachOObject.h"
#include "MachOMgr.h"
#include "VirtualPrefix.h"
//#include "DummyObject.h"
#include <regex.h>
#include <unistd.h>
#include <util/stlutils.h>
#include <cassert>
#include <cstring>

namespace Darling {

DylibSearch::DylibSearch()
	: m_reFrameworkPath("/System/Library/Frameworks/([a-zA-Z0-9\\.]+)/Versions/([a-zA-Z0-9\\.]+)/.*"),
	m_reDefaultFrameworkPath("/System/Library/Frameworks/([a-zA-Z0-9\\.]+)/([a-zA-Z0-9\\.]+)")
{
	try
	{
		if (__prefix_get() != nullptr)
			m_config = new IniConfig(__prefix_translate_path(ETC_DARLING_PATH "/dylib.conf"));
		else
			m_config = new IniConfig(LIBEXEC_PATH ETC_DARLING_PATH "/dylib.conf");
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}

DylibSearch* DylibSearch::instance()
{
	try
	{
		static DylibSearch singleton;
		return &singleton;
	}
	catch (const std::regex_error& e)
	{
		std::cerr << "This version of Darling has been compiled against broken version of libstdc++.\n";
		abort();
	}
}

void DylibSearch::setAdditionalPaths(const std::string& paths)
{
	m_extraPaths = string_explode(paths, ':', false);
}

void DylibSearch::setAdditionalSuffixes(const std::vector<std::string>& suffixes)
{
	m_suffixes = suffixes;
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
	{
		std::string p;
		
		if (!strchr(aliasTarget, '/'))
		{
			p = LIB_PATH;
			p += '/';
			p += aliasTarget;
			// std::cout << p << std::endl;
		}
		
		return p;
	}
	
	// Search in extra paths
	std::string epath;
	epath = resolveInPathList(dylib, m_extraPaths);
	if (!epath.empty())
		return epath;
	
	// Search in DYLD_LIBRARY_PATH
	epath = resolveInLdPath(dylib);
	if (!epath.empty())
		return epath;
	
	// Try the path as is
	epath = checkPresence(dylib);
	if (!epath.empty())
		return epath;

	// If absolute, search in sysroot
	if (dylib[0] == '/')
	{
		const char* prefix = __prefix_get();
		
		if (!MachOMgr::instance()->sysRoot().empty())
		{
			std::vector<std::string> roots = string_explode(MachOMgr::instance()->sysRoot(), ':');

			for (const std::string& in_path : roots)
			{
				std::string path;
				
				if (prefix != nullptr)
					path = prefix;
				
				path += in_path;
				path += '/';
				path += dylib;

				epath = checkPresence(path);
				if (!epath.empty())
					return epath;
			}
		}
		if (prefix != nullptr)
		{
			std::string path = prefix;
			path += dylib;
			
			epath = checkPresence(path);
			if (!epath.empty())
				return epath;
		}
	}
	
	/*if (MachOMgr::instance()->ignoreMissingDependencies())
	{
		
	}*/

	return std::string();
}

std::string DylibSearch::checkPresence(const std::string& path)
{
	std::string str = path;
	
	if (::access(str.c_str(), F_OK) == 0)
		return str;
	
	str += ".dylib";
	
	if (::access(str.c_str(), F_OK) == 0)
		return str;
	
	for (const std::string& suffix : m_suffixes)
	{
		str = path + suffix;
		
		if (::access(str.c_str(), F_OK) == 0)
			return str;
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
	
	return resolveInPathList(name, elems);
}

std::string DylibSearch::resolveInPathList(std::string name, const std::vector<std::string>& paths)
{
	const char* prefix = __prefix_get();
	for (const std::string& e : paths)
	{
		std::string path;
		
		if (prefix)
			path = prefix;
		
		path += e + "/" + name;

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

	std::smatch match;
	if (std::regex_match(library, match, m_reFrameworkPath))
	{
		std::string name, version;
		
		name = match[1];
		version = match[2];
			
		if (m_config->hasSection(name))
		{
			const IniConfig::ValueMap* m = m_config->getSection(name);
			auto it = m->find(version);
				
			if (it != m->end())
				return it->second.c_str();
		}
	}
	
	if (std::regex_match(library, match, m_reDefaultFrameworkPath))
	{
		std::string name;
		
		name = match[1];
		
		if (m_config->hasSection(name))
		{
			const IniConfig::ValueMap* m = m_config->getSection(name);
			auto it = m->find("default");
			
			if (it != m->end())
				return it->second.c_str();
			else
				return m->begin()->second.c_str();
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

	for (const auto& rpath : requester->runPaths())
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

std::string DylibSearch::realpath(const std::string& path)
{
	char* real;
	std::string rv;
	
	real = ::realpath(path.c_str(), nullptr);
	
	if (real)
	{
		rv = real;
	
		free(real);
		return rv;
	}
	else
		return path;
}

} // namespace Darling
