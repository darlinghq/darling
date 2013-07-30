#ifndef LOADABLEOBJECT_H
#define LOADABLEOBJECT_H
#include <dlfcn.h>
#include <string>

class MachOObject;

class LoadableObject
{
protected:
	virtual ~LoadableObject() {}
public:
	inline void setLoadAttributes(int attribs) { m_attribs = attribs; }
	
	virtual void load() = 0;
	virtual void unload() = 0;
	
	virtual void* getExportedSymbol(const std::string& symbolName, bool nonWeakOnly = false) const = 0;
	virtual bool findSymbolInfo(const void* addr, Dl_info* p) const = 0;
	
	int addRef();
	int delRef();
protected:
	static LoadableObject* instantiateForPath(const std::string& path, MachOObject* requester);
protected:
	int m_refs = 1;
	int m_attribs = 0;
};

#define DARWIN_RTLD_LAZY		0x1
#define DARWIN_RTLD_NOW			0x2
#define DARWIN_RTLD_LOCAL		0x4
#define DARWIN_RTLD_GLOBAL		0x8
#define DARWIN_RTLD_NOLOAD		0x10
#define DARWIN_RTLD_NODELETE	0x80
#define DARWIN_RTLD_FIRST		0x100

#endif
