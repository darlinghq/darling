#include "NativeObject.h"
#include <dlfcn.h>
#include <sstream>
#include <stdexcept>
#include "MachOMgr.h"
#include <iostream>

NativeObject::NativeObject(const std::string& path)
: m_path(path)
{
}

void NativeObject::load()
{
	m_nativeRef = ::dlopen(m_path.c_str(), translateFlags(m_attribs));
	if (!m_nativeRef)
	{
		const char* err = ::dlerror();
		std::stringstream ss;
		
		ss << "Failed to load native library: " << err;
		throw std::runtime_error(ss.str());
	}
	else
	{
		if (MachOMgr::instance()->printLibraries())
			std::cerr << "dyld: Loaded " << m_path << std::endl;
	}
}

void NativeObject::unload()
{
	::dlclose(m_nativeRef);
}

int NativeObject::translateFlags(int flag)
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
	//if (flag & __DARLING_RTLD_NOBIND)
	//	native_flags |= __DARLING_RTLD_NOBIND;
	return native_flags;
}

void* NativeObject::getExportedSymbol(const std::string& symbolName, bool nonWeakOnly) const
{
	return ::dlsym(m_nativeRef, symbolName.c_str());
}

bool NativeObject::findSymbolInfo(const void* addr, Dl_info* p) const
{
	return ::dladdr(addr, p) != 0;
}
