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
private:
	void updateName();
private:
	mutable void* m_nativeRef;
	std::string m_name, m_path;
};

} // namespace Darling

#endif
