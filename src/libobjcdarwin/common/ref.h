#ifndef COMMON_REF_H
#define COMMON_REF_H
#include <algorithm>

template<typename OrigType, typename NewType>
static void find_and_fix(OrigType** start, OrigType** end, const OrigType* what, const NewType* ptr)
{
	OrigType** pos = std::find(start, end, const_cast<OrigType*>(what));
	if (pos != end)
	{
		LOG << "ObjC fixup @" << pos << ": " << *pos << " -> " << ptr << std::endl;
		*reinterpret_cast<uintptr_t*>(pos) = reinterpret_cast<uintptr_t>(ptr);
	}
}

#endif
