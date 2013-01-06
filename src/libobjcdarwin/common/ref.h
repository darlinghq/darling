#ifndef COMMON_REF_H
#define COMMON_REF_H
#include <algorithm>
#include <map>

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

template<typename OrigType, typename NewType>
static void find_and_fix(OrigType** start, OrigType** end, const std::map<const OrigType*,NewType*>& map)
{
	LOG << "ObjC fixup in range " << start << " -> " << end << std::endl;
	std::for_each(start, end, [&map](OrigType*& origPtr) {
		auto it = map.find(origPtr);
		if (it != map.end())
		{
			LOG << "ObjC fixup @" << &origPtr << ": " << origPtr << " -> " << it->second << std::endl;
			origPtr = reinterpret_cast<OrigType*>(it->second);
		}
		else
			LOG << "ObjC fixup FAILED @" << &origPtr << std::endl;
	});
}

#endif
