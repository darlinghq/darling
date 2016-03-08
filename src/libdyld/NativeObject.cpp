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

#include "NativeObject.h"
#include <dlfcn.h>
#include <sstream>
#include <stdexcept>
#include "MachOMgr.h"
#include <iostream>
#include <cstdlib>
#include <link.h>
#include <regex>

namespace Darling {

static std::regex g_reObjcSymbol("(OBJC_[^\\$]+)\\$_(.+)");
static std::regex g_reObjcEhSymbol("OBJC_EHTYPE_.+");

static NativeObject* g_mainObject = nullptr;
void NativeObject::setMainObject(NativeObject* obj)
{
	g_mainObject = obj;
}

NativeObject* NativeObject::mainObject()
{
	return g_mainObject;
}

NativeObject::NativeObject(const std::string& path)
: m_path(path), m_name(path)
{
}

NativeObject::NativeObject(void* nativeRef, const std::string& name)
	: m_nativeRef(nativeRef), m_path(name), m_name(name)
{
}

void NativeObject::load()
{
	int flags = 0;
	struct link_map* lm;

	flags |= globalExports() ? RTLD_GLOBAL : RTLD_LOCAL;
	//flags |= bindAllAtLoad() ? RTLD_NOW : RTLD_LAZY;
	flags |= RTLD_LAZY;
	
	if (MachOMgr::instance()->printLibraries())
		std::cerr << "dyld: Loading " << m_path << std::endl;

	m_nativeRef = ::dlopen(m_path.c_str(), flags);
	if (!m_nativeRef)
	{
		const char* err = ::dlerror();
		std::stringstream ss;
		
		ss << "Failed to load native library: " << err;
		throw std::runtime_error(ss.str());
	}
	else
	{

		updateName();

		if (MachOMgr::instance()->printLibraries())
			std::cerr << "dyld: Loaded " << m_path << std::endl;
		
		MachOMgr::instance()->add(this);
	}

	if (::dlinfo(m_nativeRef, RTLD_DI_LINKMAP, &lm) == 0)
		m_baseAddress = (void*) lm->l_addr;

	transitionState(dyld_image_state_mapped);
	transitionState(dyld_image_state_dependents_mapped);
	transitionState(dyld_image_state_rebased);
	transitionState(dyld_image_state_bound);
	transitionState(dyld_image_state_dependents_initialized);
	transitionState(dyld_image_state_initialized);
}

void NativeObject::unload()
{
	MachOMgr::instance()->remove(this);

	if (!MachOMgr::instance()->isDestroying())
	{
		if (m_path.find('/') != std::string::npos)
			::dlclose(m_nativeRef);
	}

	transitionState(dyld_image_state_terminated);
}

void NativeObject::updateName()
{
	struct link_map* map = nullptr;
	size_t pos;

	dlinfo(m_nativeRef, RTLD_DI_LINKMAP, &map);

	if (map)
	{
		char* real = realpath(map->l_name, nullptr);
		if (real)
		{
			m_path = real;
			free(real);
		}
	}

	pos = m_path.rfind('/');
	if (pos == std::string::npos)
		m_name = m_path;
	else
		m_name = m_path.substr(pos+1);
}

void* NativeObject::getExportedSymbol(const std::string& symbolName, bool nonWeakOnly) const
{
	void* addr;
	std::string name;
	std::smatch match;
	
	//if (symbolName == "main")
	//	return nullptr; // Don't return main() from Darling itself

	name = symbolName;

	if (std::regex_match(name, match, g_reObjcSymbol))
	{
		name = match.format("_$1$2");
	}
	else if (std::regex_match(name, match, g_reObjcEhSymbol))
	{
		static long dummy = 0;
		return &dummy;
	}

	addr = ::dlvsym(m_nativeRef, name.c_str(), "DARWIN");
	
	// if (!addr)
	// 	std::cerr << "Cannot find " << name << " in " << path() << std::endl;
	
	return addr;
}

bool NativeObject::findSymbolInfo(const void* addr, Dl_info* p) const
{
	return ::dladdr(addr, p) != 0;
}

const char* NativeObject::name() const
{
	return m_name.c_str();
}

const std::string& NativeObject::path() const
{
	return m_path;
}

void* NativeObject::baseAddress() const
{
	return m_baseAddress;
}

// static int dl_cb(struct dl_phdr_info *info, void* data)
// {
// 	return 0;
// }

void* NativeObject::getSection(const std::string& segmentName, const std::string& sectionName, uintptr_t* sectionSize)
{
	// dl_iterate_phdr(dl_cb, nullptr);
	return nullptr;
}

} // namespace Darling

