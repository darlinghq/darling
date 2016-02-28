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

#ifndef NATIVEOBJECT_H
#define NATIVEOBJECT_H
#include <string>
#include "LoadableObject.h"

namespace Darling {

class NativeObject : public LoadableObject
{
public:
	NativeObject(const std::string& path);
	NativeObject(void* nativeRef, const std::string& name);
	
	virtual void load() override;
	virtual void unload() override;
	virtual void* getExportedSymbol(const std::string& symbolName, bool nonWeakOnly) const override;
	virtual bool findSymbolInfo(const void* addr, Dl_info* p) const override;
	virtual const char* name() const override;
	virtual bool isLoaded() const override { return m_nativeRef != nullptr; }
	virtual const std::string& path() const override;

	inline void* nativeRef() { return m_nativeRef; }
	virtual void* baseAddress() const override;
	void* getSection(const std::string& segmentName, const std::string& sectionName, uintptr_t* sectionSize = nullptr);
private:
	void updateName();
private:
	mutable void* m_nativeRef = nullptr;
	std::string m_name, m_path;
	void* m_baseAddress = nullptr;
};

} // namespace Darling

#endif
