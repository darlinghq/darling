#include "LoadableObject.h"

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
