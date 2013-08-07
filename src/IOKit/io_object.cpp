#include "io_object.h"

io_object::io_object()
: m_refs(1)
{
}

io_object::~io_object()
{
}

void io_object::retain()
{
	m_refs++;
}

void io_object::release()
{
	if (!(--m_refs))
		delete this;
}

int IOObjectRetain(io_object_t obj)
{
	obj->retain();
	return 0;
}

int IOObjectRelease(io_object_t obj)
{
	if (obj)
		obj->release();
	return 0;
}

uint32_t IOObjectGetRetainCount(io_object_t obj)
{
	if (!obj)
		return 0;
	return obj->refs();
}

uint32_t IOObjectGetUserRetainCount(io_object_t obj)
{
	if (!obj)
		return 0;
	return obj->refs();
}

bool IOObjectIsEqualTo(io_object_t o1, io_object_t o2)
{
	if (o1 == o2)
		return true;
	if (!o1 || !o2)
		return false;
	else
		return *o1 == *o2;
}
