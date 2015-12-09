#include "LoadableObject.h"
#include <libmach-o/MachO.h>
#include "MachOObject.h"
#include "NativeObject.h"
#include "dl_public.h"
#include "arch.h"

namespace Darling {
	
LoadableObject::LoadableObject()
{
	m_refs = 1;
}

int LoadableObject::addRef()
{
	return ++m_refs;
}

int LoadableObject::delRef()
{
	int refs = --m_refs;
	if (!refs && !m_noDelete)
	{
		if (isLoaded())
			unload();
		delete this;
	}
	
	return refs;
}

LoadableObject* LoadableObject::instantiateForPath(const std::string& path, MachOObject* requester, int flag)
{
	if (MachO::isMachO(path.c_str()))
	{
		MachOObject* obj = new MachOObject(path);
		obj->setRequesterRunpaths(requester);
		obj->setNoRecursion(flag & DARWIN_RTLD_FIRST);
		return obj;
	}
	else
	{
		return new NativeObject(path);
	}
}

void LoadableObject::transitionState(dyld_image_states newState)
{
	m_state = newState;
}

} // namespace Darling
