#ifndef IOKIT_IOOBJECT_H
#define IOKIT_IOOBJECT_H
#include <stdint.h>

class io_object
{
public:
	io_object();
	virtual ~io_object();
	virtual bool operator==(const io_object& that) = 0;
	
	void retain();
	void release();
	int refs() const { return m_refs; }
private:
	int m_refs;
};

typedef io_object* io_object_t;

extern "C" int IOObjectRetain(io_object_t obj);
extern "C" int IOObjectRelease(io_object_t obj);
extern "C" uint32_t IOObjectGetRetainCount(io_object_t obj);
extern "C" uint32_t IOObjectGetUserRetainCount(io_object_t obj);
extern "C" bool IOObjectIsEqualTo(io_object_t o1, io_object_t o2);

#endif

