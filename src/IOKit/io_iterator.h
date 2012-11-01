#ifndef IOKIT_IOITERATOR_H
#define IOKIT_IOITERATOR_H
#include "io_object.h"

class io_iterator : public io_object
{
public:
	virtual io_object_t next() = 0;
};

typedef io_iterator* io_iterator_t;

#endif
