#ifndef IOKIT_IOOBJECT_H
#define IOKIT_IOOBJECT_H

class io_object
{
public:
	virtual ~io_object();
};

typedef io_object* io_object_t;

extern "C" int IOObjectRelease(io_object_t obj);

#endif

