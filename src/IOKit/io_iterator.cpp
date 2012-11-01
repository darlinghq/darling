#include "io_iterator.h"

int IOIteratorIsValid(io_iterator_t iter)
{
	return iter != nullptr;
}

io_object_t IOIteratorNext(io_iterator_t iter)
{
	return iter->next();
}

void IOIteratorReset(io_iterator_t iter)
{
	iter->reset();
}
