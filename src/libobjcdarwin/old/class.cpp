#include "./class.h"
#include "../../util/log.h"
#include "../TopologySort.h"
#include "../common/method.h"
#include "ivar.h"
#include "../common/property.h"
#include <map>

extern std::map<const void*,Class> g_classPointers;

Class RegisterClass(old_class* cls, bool hasExt)
{
	LOG << "Processing old ObjC class " << cls->name << std::endl;
	
	const old_class* meta = cls->isa.cls;
	Class conv, super;
	/*
	old_class* psuper = cls->super_class.cls;
	auto itSuper = g_classPointers.find(psuper); // TODO: may not be needed, should always be a string

	if (itSuper != g_classPointers.end())
		super = itSuper->second;
	else
		super = reinterpret_cast<Class>(psuper);
	*/

	super = (Class) objc_getClass(cls->super_class.name);
	LOG << "...with superclass @" << super << std::endl;
	conv = objc_allocateClassPair(super, cls->name, 0);
	
	if (cls->methodList)
		ConvertMethodListGen(conv, cls->methodList);
	if (meta->methodList)
		ConvertMethodListGen(object_getClass(id(conv)), meta->methodList);
	if (cls->ivars)
		ConvertIvarList(conv, cls->ivars);
	if (cls->protocols)
		AddClassProtocols(conv, cls->protocols);
	
	if (hasExt && cls->ext && cls->ext->propertyLists)
	{
		LOG << "Class has EXT and a property list/lists\n";
		//if (cls->info & CLS_NO_PROPERTY_ARRAY)
		if (true)
		{
			ConvertProperties(cls->ext->propertyList, [conv](const char* name, const objc_property_attribute_t* attr, unsigned int count) { class_addProperty(conv, name, attr, count); bug_gnustepFixPropertyCount(conv); });
		}
		else
		{
			for (size_t i = 0; cls->ext->propertyLists[i] != nullptr; i++)
			{
				const old_property_list* l = cls->ext->propertyLists[i];
				ConvertProperties(l, [conv](const char* name, const objc_property_attribute_t* attr, unsigned int count) { class_addProperty(conv, name, attr, count); bug_gnustepFixPropertyCount(conv); });
			}
		}
	}
	
	objc_registerClassPair(conv);
	g_classPointers[cls] = conv;
	g_classPointers[cls->name] = conv;

	LOG << "ObjC class " << cls->name << " @" << conv << std::endl;
	
	return conv;
}

void AddClassProtocols(Class conv, old_protocol_list* list)
{
	for (long i = 0; i < list->count; i++)
	{
		Protocol* p = objc_getProtocol(list->list[i]->name);
		assert(p != nullptr);
		class_addProtocol(conv, p);
	}
}

void ProcessClassesOld(const struct mach_header* mh, intptr_t slide, module_info* info)
{
	unsigned long cstrLen;
	void* ptr;
	std::vector<old_class*> vecClasses;
	std::set<old_class*> setClasses;
	std::map<const char*,old_class*> mapClassNames;

	// ptr = getsectdata(mh, "__TEXT", "__cstring", &cstrLen);
	// if (ptr)
	// 	g_cstringSection = std::pair<uintptr_t,uintptr_t>(uintptr_t(ptr), cstrLen);
	// else
	// 	g_cstringSection = std::pair<uintptr_t,uintptr_t>(0, 0);
	
	for (uint16_t i = 0; i < info->symtab->countClasses; i++)
	{
		old_class* cls = static_cast<old_class*>(info->symtab->classesAndCategories[i]);
		mapClassNames[cls->name] = cls;
		setClasses.insert(cls);
	}

	topology_sort(setClasses, vecClasses,
		[&mapClassNames](old_class* t) -> std::set<old_class*> { auto it = mapClassNames.find(t->super_class.name); return (it != mapClassNames.end()) ? std::set<old_class*>{it->second} : std::set<old_class*>(); }
	);
			
	for (old_class* c : vecClasses)
		RegisterClass(c, info->version >= 6);

	clsref* class_refs;
	unsigned long refsize;

	class_refs = reinterpret_cast<clsref*>(
		getsectdata(mh, SEG_OBJC_CLASSREFS_OLD, SECT_OBJC_CLASSREFS_OLD, &refsize)
	);

	if (class_refs)
	{
		for (size_t i = 0; i < refsize / sizeof(clsref); i++)
		{
			Class c = (Class) objc_getClass(class_refs[i].clsName);
			LOG << "ObjC fixup classref @" << &class_refs[i].cls << ": " << class_refs[i].cls << " -> " << c << std::endl;
			class_refs[i].cls = c;
		}
	}

	// Change class names in 'super_class' from strings to pointers to Class
	// for (size_t i = 0; i < size / sizeof(old_class); i++)
	for (uint16_t i = 0; i < info->symtab->countClasses; i++)
	{
		old_class* cls = static_cast<old_class*>(info->symtab->classesAndCategories[i]);
		Class c = (Class) objc_getClass(cls->super_class.name);
		LOG << "ObjC fixup super_class @" << &cls->super_class << ": " << cls->super_class.name << " -> " << c << std::endl;
		cls->super_class.clsNew = c;
	}

	// Fix the same in metaclasses
	// for (size_t i = 0; i < size / sizeof(old_class); i++)
	for (uint16_t i = 0; i < info->symtab->countClasses; i++)
	{
		old_class* cls = static_cast<old_class*>(info->symtab->classesAndCategories[i])->isa.cls;
		Class c = (Class) objc_getMetaClass(cls->name);
		LOG << "ObjC fixup super_class @" << cls << ": " << cls->name << " -> " << c << std::endl;
		cls->super_class.clsNew = c;
	}
}
