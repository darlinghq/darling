#include "./category.h"
#include "../../util/log.h"
#include "./class.h"
#include "../common/property.h"
#include "../common/method.h"

void ProcessCategoriesOld(const struct mach_header* mh, intptr_t slide, module_info* modinfo)
{
	struct symtab* symtab = modinfo->symtab;
	const size_t firstCat = symtab->countClasses;

	if (!symtab->countCategories)
	{
		LOG << "No categories found\n";
		return;
	}

	for (size_t i = 0; i < symtab->countCategories; i++)
	{
		old_category* cat = reinterpret_cast<old_category*>(symtab->classesAndCategories[i+firstCat]);
		Class cls = (Class) objc_getClass(cat->clsName);

		LOG << "Processing category " << cat->name << " @" << cat << " on top of " << cat->clsName << " @" << cls << std::endl;

		assert(cls != nullptr);

		if (cat->methods)
			ConvertMethodListGen(cls, cat->methods);
		if (cat->staticMethods)
			ConvertMethodListGen(object_getClass(id(cls)), cat->staticMethods);

		if (modinfo->version >= 5 && cat->protocols)
			AddClassProtocols(cls, cat->protocols);

		if (modinfo->version >= 7 && cat->properties)
		{
			ConvertProperties(cat->properties, [cls](const char* name, const objc_property_attribute_t* attr, unsigned int count) { class_addProperty(cls, name, attr, count); bug_gnustepFixPropertyCount(cls); });
		}
	}
}

