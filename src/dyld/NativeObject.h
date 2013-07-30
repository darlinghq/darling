#ifndef NATIVEOBJECT_H
#define NATIVEOBJECT_H
#include <string>
#include "LoadableObject.h"

class NativeObject : public LoadableObject
{
public:
	NativeObject(const std::string& path);
	
	virtual void load() override;
	virtual void unload() override;
	virtual void* getExportedSymbol(const std::string& symbolName, bool nonWeakOnly) const override;
	virtual bool findSymbolInfo(const void* addr, Dl_info* p) const override;
private:
	int translateFlags(int flag);
private:
	mutable void* m_nativeRef;
	std::string m_path;
};

#endif
