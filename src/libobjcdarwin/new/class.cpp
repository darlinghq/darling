#include "./class.h"
#include "../../util/trace.h"
#include "../../util/log.h"
#include <map>
#include <algorithm>
#include "../common/method.h"
#include "../common/property.h"
#include "../common/ref.h"
#include "./ivar.h"
#include "./protocol.h"
#include "../TopologySort.h"

extern std::map<const void*,Class> g_classPointers;

Class RegisterClass(const class_t* cls, intptr_t slide)
{
	LOG << "Processing ObjC class " << cls->data()->className << std::endl;
	
	const class_t* meta = cls->isa;
	Class conv, super;
	auto itSuper = g_classPointers.find(cls->superclass);

	if (itSuper != g_classPointers.end())
		super = itSuper->second;
	else
		super = reinterpret_cast<Class>(cls->superclass);
	
	LOG << "...superclass is @" << super << std::endl;
	assert(objc_getClass(cls->data()->className) == nullptr);
	conv = objc_allocateClassPair(super, cls->data()->className, 0);
	
	const class_ro_t* ro = cls->data();
	const class_ro_t* roMeta = meta->data();
	
	if (ro->baseMethods)
		ConvertMethodListGen(conv, ro->baseMethods);
	if (roMeta->baseMethods)
		ConvertMethodListGen(object_getClass(id(conv)), roMeta->baseMethods);
	if (ro->ivars)
		ConvertIvarList(conv, ro->ivars);
	if (ro->baseProtocols)
		AddClassProtocols(conv, ro->baseProtocols, slide);
	if (ro->baseProperties)
	{
		ConvertProperties(ro->baseProperties, [conv](const char* name, const objc_property_attribute_t* attr, unsigned int count) { class_addProperty(conv, name, attr, count); bug_gnustepFixPropertyCount(conv); });
	}
	
        #ifdef __i386__
        if (ro->instSize != conv->instance_size) {
            throw std::logic_error("Instance size not copied");
        }
        if (roMeta->instSize != conv->isa->instance_size) {
            throw std::logic_error("Instance size not copied (meta)");
        }
        #endif
	// conv->instance_size = ro->instSize;
	// conv->isa->instance_size = roMeta->instSize;
	
	objc_registerClassPair(conv);
	
	LOG << "ObjC class " << cls->data()->className << " now @" << conv << std::endl;
	g_classPointers[cls] = conv;
	g_classPointers[cls->isa] = object_getClass(id(conv));
	
	return conv;
}

std::vector<const char*> ProcessClassesNew(const struct mach_header* mh, intptr_t slide, const class_t** classes, unsigned long size)
{
	std::vector<const class_t*> vecClasses;
	std::set<const class_t*> setClasses;
	std::vector<const char*> classNames;
	
	std::copy(classes, classes+size/sizeof(class_t*), std::inserter(setClasses, setClasses.begin()));

	topology_sort<const class_t>(setClasses, vecClasses,
		[&setClasses](const class_t* t) { return setClasses.count(t->superclass) ? std::set<const class_t*>{t->superclass} : std::set<const class_t*>();  }
	);

	for (const class_t* cls : vecClasses)
	{
		RegisterClass(cls, slide);
		classNames.push_back(cls->data()->className);
	}

	return classNames;
}

void UpdateClassRefs(const struct mach_header* mh)
{
	unsigned long refsize, refsize_s;
	class_t **class_refs, **class_refs_end, **super_refs, **super_refs_end;
	
	class_refs = reinterpret_cast<class_t**>(
		getsectdata(mh, SEG_OBJC_CLASSREFS_NEW, SECT_OBJC_CLASSREFS_NEW, &refsize)
	);
	super_refs = reinterpret_cast<class_t**>(
		getsectdata(mh, SEG_OBJC_SUPERREFS_NEW, SECT_OBJC_SUPERREFS_NEW, &refsize_s)
	);
	if (class_refs)
		class_refs_end = class_refs + refsize / sizeof(class_t*);
	
	if (super_refs)
		super_refs_end = super_refs + refsize_s / sizeof(class_t*);
	
	if (class_refs)
		find_and_fix((void**) class_refs, (void**) class_refs_end, g_classPointers);
	
	if (super_refs)
		find_and_fix((void**) super_refs, (void**) super_refs_end, g_classPointers);
}

