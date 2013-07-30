#include "LoadableObject.h"
#include <libmach-o/MachO.h>
#include "MachOObject.h"
#include "NativeObject.h"
#include "arch.h"

int LoadableObject::addRef()
{
	return __sync_add_and_fetch(&m_refs, 1);
}

int LoadableObject::delRef()
{
	int refs = __sync_add_and_fetch(&m_refs, -1);
	if (!refs)
	{
		unload();
		delete this;
	}
	
	return refs;
}

LoadableObject* LoadableObject::instantiateForPath(const std::string& path, MachOObject* requester)
{
	if (MachO::isMachO(path.c_str()))
	{
		MachOObject* obj = new MachOObject(path);
		obj->setRequesterRunpaths(requester->runPaths());
		return obj;
	}
	else
	{
		return new NativeObject(path);
	}
}
