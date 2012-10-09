#include "./category.h"
#include "../common/property.h"
#include "../common/method.h"
#include "./protocol.h"
#include "../../util/log.h"
#include <map>

extern std::map<const void*,Class> g_classPointers;

void ProcessCategoriesNew(const struct mach_header* mh, intptr_t slide)
{
	category_t** cats;
	unsigned long size;

	cats = reinterpret_cast<category_t**>(
		getsectdata(mh, SEG_OBJC_CATLIST_NEW, SECT_OBJC_CATLIST_NEW, &size)
	);
	if (!cats)
	{
		LOG << "No categories found\n";
		return;
	}

	LOG << "Processing categories, " << (size / sizeof(category_t*)) << " found\n";

	for (size_t i = 0; i < size / sizeof(category_t*); i++)
	{
		// g_classPointers
		category_t* cat = cats[i];
		auto itClass = g_classPointers.find(cat->cls);
		Class c;

		if (itClass != g_classPointers.end())
			c = itClass->second;
		else
			c = reinterpret_cast<Class>(cat->cls);
			
		LOG << "Processing category " << cat->name << " on top of class @" << c << std::endl;

		if (cat->methods)
			ConvertMethodListGen(c, cat->methods);
		if (cat->staticMethods)
			ConvertMethodListGen(object_getClass(id(c)), cat->staticMethods);
		if (cat->protocols)
			AddClassProtocols(c, cat->protocols, slide);
    	if (cat->properties)
	    {
    	    ConvertProperties(cat->properties, [c](const char* name, const objc_property_attribute_t* attr, unsigned int count) { class_addProperty(c, name, attr, count); bug_gnustepFixPropertyCount(c); });
	    }
	}
}
