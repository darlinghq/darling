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
}

void NativeObject::unload()
{
	MachOMgr::instance()->remove(this);

	if (!MachOMgr::instance()->isDestroying())
	{
		if (m_path.find('/') != std::string::npos)
			::dlclose(m_nativeRef);
	}
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
	
	if (symbolName == "main")
		return nullptr; // Don't return main() from Darling itself

	name = symbolName;

	if (std::regex_match(name, match, g_reObjcSymbol))
		name = match.format("_$1$2");
	else if (std::regex_match(name, match, g_reObjcEhSymbol))
	{
		static long dummy = 0;
		return &dummy;
	}

	return ::dlvsym(m_nativeRef, name.c_str(), "DARWIN");
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

} // namespace Darling

