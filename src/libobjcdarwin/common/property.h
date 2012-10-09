#ifndef COMMON_PROPERTY_H
#define COMMON_PROPERTY_H
#include <objc/runtime.h>
#include <stdint.h>
#include "./AppleLayout.h"
#include "../../util/log.h"
#include "attribute.h"
#include <vector>

template<typename PropList, typename Func>
void ConvertProperties(const PropList* props, Func f)
{
	LOG << "Registering " << props->count << " properties\n";
	for (uint32_t i = 0; i < props->count; i++)
	{
		auto* prop = &props->list[i];
		std::vector<objc_property_attribute_t> attribs;
		std::vector<std::string> strings;
		objc_property_attribute_t next;
		const char* pos = prop->attributes;

		while (nextAttribute(next, pos, strings))
			attribs.push_back(next);

		//attribs.clear();
		LOG << "\tProperty " << prop->name << " with " << attribs.size() << " attributes\n";
		f(prop->name, &attribs[0], attribs.size());
	}
}

void bug_gnustepFixPropertyCount(Class c);


#endif
