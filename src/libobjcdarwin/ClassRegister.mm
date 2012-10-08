#include "ClassRegister.h"
#include "../dyld/public.h"
#include "../util/trace.h"
#include "../util/log.h"
#include <climits>
#include <stddef.h>
#include <Foundation/NSObjCRuntime.h>
#include <iostream>
#include <map>
#include <algorithm>
#include <cstring>
#include <cassert>
#include <sys/mman.h>
#include "TopologySort.h"

static std::pair<uintptr_t,uintptr_t> g_cstringSection;
// Superclass references in Mach-O don't use classref
static std::map<const void*,Class> g_superClasses;

// Here we process Mach-O files that have been loaded before this native library
// Then we register a handler to process all images loaded in the future
__attribute__((constructor))
	void RegisterAlreadyLoadedClasses()
{
	for (uint32_t i = 0; i < _dyld_image_count(); i++)
	{
		const struct mach_header* hdr = _dyld_get_image_header(i);
		ProcessImageLoad(hdr, 0);
	}

	_dyld_register_func_for_add_image(ProcessImageLoad);
	_dyld_register_func_for_remove_image(ProcessImageUnload);
	
	//std::cout << "Done registering\n";
}

template<typename ListType> void ConvertMethodListGen(Class c, const ListType* list)
{
	LOG << list->count << " methods within\n";
	
	for (size_t i = 0; i < list->count; i++)
	{
		auto* m = &list->method_list[i];
		
		LOG << "Method: selName: " << m->selName << "; types: " << m->types << "; impl: " << m->impl << std::endl;

		SEL sel = sel_registerName(m->selName);
		class_addMethod(c, sel, reinterpret_cast<IMP>(m->impl), m->types);
	}
}

static void ConvertIvarList(Class c, const ivar_list_t* list)
{
	LOG << list->count << " ivars within\n";

	for (size_t i = 0; i < list->count; i++)
	{
		auto* v = &list->ivar_list[i];

		LOG << "Ivar: name: " << v->name << "; type: " << v->type << "; offset: " << *v->offset << "; size: " << v->size << "; alignment: " << v->alignment << std::endl;
		class_addIvar(c, v->name, v->size, v->alignment, v->type);
	}
}

static void ConvertIvarList(Class c, const old_ivar_list* list)
{
	LOG << list->count << " ivars within\n";

	for (size_t i = 0; i < list->count; i++)
	{
		auto* v = &list->ivar_list[i];
		NSUInteger size, alignment;

		NSGetSizeAndAlignment(v->type, &size, &alignment);
		alignment = 1; // TODO: why do we need alignment when we have an offset?
		
		LOG << "Ivar: name: " << v->name << "; type: " << v->type << "; offset: " << v->offset << "; size: " << size << "; alignment: " << alignment << std::endl;
		class_addIvar(c, v->name, size, alignment, v->type);
	}
}

static bool nextAttribute(objc_property_attribute_t& next, const char*& pos, std::vector<std::string>& strings)
{
	if (!*pos)
		return false;
	else if (*pos == ',')
		pos++;

	if (*pos == '"')
	{
		const char* start = ++pos;
		while (*pos != '"' && *pos)
			pos++;

		assert(*pos != '\0');

		strings.push_back(std::string(start, pos-start));
		next.name = strings.back().c_str();

		start = ++pos;
		while (*pos != ',' && *pos)
			pos++;

		strings.push_back(std::string(start, pos-start));
		next.value = strings.back().c_str();
	}
	else
	{
		strings.push_back(std::string(1, *pos));
		next.name = strings.back().c_str();
		const char* start = ++pos;

		while (*pos != ',' && *pos)
			pos++;

		strings.push_back(std::string(start, pos-start));
		next.value = strings.back().c_str();
	}

	return true;
}

template<typename Func>
void ConvertProperties(const property_list_t* props, Func f)
{
	for (uint32_t i = 0; i < props->count; i++)
	{
		const property_t* prop = &props->list[i];
		std::vector<objc_property_attribute_t> attribs;
		std::vector<std::string> strings;
		objc_property_attribute_t next;
		const char* pos = prop->attributes;

		while (nextAttribute(next, pos, strings))
			attribs.push_back(next);

		f(prop->name, &attribs[0], attribs.size());
	}
}

Class RegisterClass(const class_t* cls, intptr_t slide)
{
	LOG << "Processing ObjC class " << cls->data()->className << std::endl;
	
	const class_t* meta = cls->isa;
	Class conv, super;
	auto itSuper = g_superClasses.find(cls->superclass);

	if (itSuper != g_superClasses.end())
		super = itSuper->second;
	else
		super = reinterpret_cast<Class>(cls->superclass);
	
	LOG << "...superclass is @" << super << std::endl;
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
	{
		for (size_t i = 0; i < ro->baseProtocols->count; i++)
		{
			const char* name = ro->baseProtocols->elem(i, slide)->name;
			Protocol* p = objc_getProtocol(name);
			assert(p != nullptr);
			class_addProtocol(conv, p);
		}
	}
	if (ro->baseProperties)
		ConvertProperties(ro->baseProperties, [conv](const char* name, const objc_property_attribute_t* attr, unsigned int count) { class_addProperty(conv, name, attr, count);  });
	
	// conv->instance_size = ro->instSize;
	// conv->isa->instance_size = roMeta->instSize;
	
	objc_registerClassPair(conv);
	LOG << "ObjC class " << cls->data()->className << " now @" << conv << std::endl;
	g_superClasses[cls] = conv;

	return conv;
}


// This is very wrong. Does it have to be this way?
static old_class* PtrToClass(old_class_ptr ptr)
{
	if (ptr.ptrValue >= g_cstringSection.first && ptr.ptrValue < g_cstringSection.first+g_cstringSection.second)
	{
		//std::cout << ptr.cls << " found to be a string: " << ptr.name << std::endl;
		id cls = objc_getClass(ptr.name); // it is a string
		
		assert(cls != nullptr);
		return (old_class*) cls;
	}
	else
	{
		//std::cout << ptr.cls << " found to be ptr to class.\n";
		return ptr.cls;
	}
}

Class RegisterClass(old_class* cls)
{
	LOG << "Processing old ObjC class " << cls->name << std::endl;
	
	const old_class* meta = PtrToClass(cls->isa);
	Class conv, super;
	old_class* psuper = PtrToClass(cls->super_class);
	auto itSuper = g_superClasses.find(psuper); // TODO: may not be needed, should always be a string

	if (itSuper != g_superClasses.end())
		super = itSuper->second;
	else
		super = reinterpret_cast<Class>(psuper);
	LOG << "...with superclass @" << super << std::endl;
	conv = objc_allocateClassPair(super, cls->name, 0);
	
	if (cls->methodList)
		ConvertMethodListGen(conv, cls->methodList);
	if (meta->methodList)
		ConvertMethodListGen(object_getClass(id(conv)), meta->methodList);
	if (cls->ivars)
		ConvertIvarList(conv, cls->ivars);
	if (cls->protocols)
	{
		for (long i = 0; i < cls->protocols->count; i++)
		{
			Protocol* p = objc_getProtocol(cls->protocols->list[i]->name);
			assert(p != nullptr);
			class_addProtocol(conv, p);
		}
	}
	// TODO: properties in EXT
	
	objc_registerClassPair(conv);
	g_superClasses[cls] = conv;
	g_superClasses[cls->name] = conv;

	LOG << "ObjC class " << cls->name << " @" << conv << std::endl;
	
	return conv;
}

void RegisterProtocolMethods(Protocol* p, const method_list_t* list, const char** extTypes, size_t& extIndex, bool required, bool instance)
{
	LOG << "Registering Protocol methods (" << required << ", " << instance << "): " << list->count << " methods within\n";
	for (size_t i = 0; i < list->count; i++, extIndex++)
	{
		SEL sel = sel_registerName(list->method_list[i].selName);
		protocol_addMethodDescription(p, sel, list->method_list[i].types, required, instance);
		// TODO: what do we do with extTypes?
	}
}

Protocol* RegisterProtocol(const protocol_t* prot, intptr_t slide)
{
	// For reasons unknown, the NSObject protocol is duplicated into the binaries
	if (strcmp(prot->name, "NSObject") == 0)
		return objc_getProtocol(prot->name);

	LOG << "Processing ObjC Protocol: " << prot->name << std::endl;
	Protocol* conv = objc_allocateProtocol(prot->name);
	size_t methodIndex = 0;

	if (prot->protocols)
	{
		for (size_t i = 0; i < prot->protocols->count; i++)
		{
			const char* name = prot->protocols->elem(i, slide)->name;
			protocol_addProtocol(conv, objc_getProtocol(name));
		}
	}

	if (prot->methods)
		RegisterProtocolMethods(conv, prot->methods, prot->extMethTypes, methodIndex, true, true);
	if (prot->staticMethods)
		RegisterProtocolMethods(conv, prot->staticMethods, prot->extMethTypes, methodIndex, true, false);
	if (prot->optMethods)
		RegisterProtocolMethods(conv, prot->optMethods, prot->extMethTypes, methodIndex, false, true);
	if (prot->optStaticMethods)
		RegisterProtocolMethods(conv, prot->optStaticMethods, prot->extMethTypes, methodIndex, false, false);

	if (prot->properties)
		ConvertProperties(prot->properties, [conv](const char* name, const objc_property_attribute_t* attr, unsigned int count) { protocol_addProperty(conv, name, attr, count, true, true);  });
	
	objc_registerProtocol(conv);
	return conv;
}

void RegisterProtocolMethods(Protocol* p, const old_method_decl_list* list, bool required, bool instance)
{
	LOG << "Registering Protocol methods (" << required << ", " << instance << "): " << list->count << " methods within\n";
	for (size_t i = 0; i < list->count; i++)
	{
		SEL sel = sel_registerName(list->list[i].name);
		protocol_addMethodDescription(p, sel, list->list[i].types, required, instance);
		// TODO: what do we do with extTypes?
	}
}

Protocol* RegisterProtocol(old_protocol* prot, uintptr_t extStart, unsigned long extLen)
{
	// For reasons unknown, the NSObject protocol is duplicated into the binaries
	if (strcmp(prot->name, "NSObject") == 0)
		return objc_getProtocol(prot->name);

	LOG << "Processing old ObjC Protocol: " << prot->name << std::endl;
	Protocol* conv = objc_allocateProtocol(prot->name);

	if (prot->protocols)
	{
		for (size_t i = 0; i < prot->protocols->count; i++)
		{
			const char* name = prot->protocols->list[i]->name;
			protocol_addProtocol(conv, objc_getProtocol(name));
		}
	}

	if (prot->methods)
		RegisterProtocolMethods(conv, prot->methods, true, true);
	if (prot->staticMethods)
		RegisterProtocolMethods(conv, prot->staticMethods, true, false);

	// Protocol EXT - weird stuff
	if (prot->ext_ptrValue >= extStart && prot->ext_ptrValue < extStart + extLen)
	{
		// TODO: check if rebase is present
		if (prot->ext->optMethods)
			RegisterProtocolMethods(conv, prot->ext->optMethods, false, true);
		if (prot->ext->optStaticMethods)
			RegisterProtocolMethods(conv, prot->ext->optStaticMethods, false, false);
		if (prot->ext->properties)
			ConvertProperties(prot->ext->properties, [conv](const char* name, const objc_property_attribute_t* attr, unsigned int count) { protocol_addProperty(conv, name, attr, count, true, true);  });
	}

	objc_registerProtocol(conv);
	return conv;
}

int mprotect_pagemult(void* mem, size_t len, int prot)
{
	/*
	static int psize = ::getpagesize();
	uintptr_t ptr = reinterpret_cast<uintptr_t>(mem);
	if (uintptr_t diff = ptr % psize)
	{
		ptr -= diff;
		len += diff;
		mem = reinterpret_cast<void*>(ptr);
	}
	return ::mprotect(mem, len, prot);
	*/
	// Short circuit: the segments that are now in use are always R/W
	return 0;
}

void ProcessRuntimeOld(const struct mach_header* mh, intptr_t slide, old_class* classes, unsigned long size)
{
	unsigned long cstrLen;
	void* ptr;
	std::vector<old_class*> vecClasses;
	std::set<old_class*> setClasses;
	std::map<const char*,old_class*> mapClassNames;

	ProcessProtocolsOld(mh, slide);
			
	ptr = getsectdata(mh, "__TEXT", "__cstring", &cstrLen);
	if (ptr)
		g_cstringSection = std::pair<uintptr_t,uintptr_t>(uintptr_t(ptr), cstrLen);
	else
		g_cstringSection = std::pair<uintptr_t,uintptr_t>(0, 0);
	
	for (size_t i = 0; i < size / sizeof(old_class); i++)
	{
		mapClassNames[classes[i].name] = classes+i;
		setClasses.insert(classes+i);
	}

	topology_sort(setClasses, vecClasses,
		[&mapClassNames](old_class* t) -> std::set<old_class*> { auto it = mapClassNames.find(t->super_class.name); return (it != mapClassNames.end()) ? std::set<old_class*>{it->second} : std::set<old_class*>(); }
	);
			
	for (old_class* c : vecClasses)
		RegisterClass(c);

	clsref* class_refs;
	unsigned long refsize;

	class_refs = reinterpret_cast<clsref*>(
		getsectdata(mh, SEG_OBJC_CLASSREFS_OLD, SECT_OBJC_CLASSREFS_OLD, &refsize)
	);

	if (class_refs)
	{
		for (size_t i = 0; i < refsize / sizeof(clsref); i++)
		{
			Class c = objc_getClass(class_refs[i].clsName);
			LOG << "ObjC fixup classref @" << &class_refs[i].cls << ": " << class_refs[i].cls << " -> " << c << std::endl;
			class_refs[i].cls = c;
		}
	}

	// Change class names in 'super_class' from strings to pointers to Class
	for (size_t i = 0; i < size / sizeof(old_class); i++)
	{
		Class c = objc_getClass(classes[i].super_class.name);
		LOG << "ObjC fixup super_class @" << &classes[i].super_class << ": " << classes[i].super_class.name << " -> " << c << std::endl;
		classes[i].super_class.clsNew = c;
	}

	// Fix the same in metaclasses
	classes = reinterpret_cast<old_class*>(
		getsectdata(mh, SEG_OBJC_METALIST_OLD, SECT_OBJC_METALIST_OLD, &size)
	);

	if (classes)
	{
		for (size_t i = 0; i < size / sizeof(old_class); i++)
		{
			Class c = objc_getClass(classes[i].super_class.name);
			LOG << "ObjC fixup super_class @" << &classes[i].super_class << ": " << classes[i].super_class.name << " -> " << c << std::endl;
			classes[i].super_class.clsNew = c;
		}
	}
}

void ProcessProtocolsNew(const struct mach_header* mh, intptr_t slide)
{
	const protocol_t** protocol_list;
	unsigned long protosize;

	protocol_list = reinterpret_cast<const protocol_t**>(
		getsectdata(mh, SEG_OBJC_PROTOLIST_NEW, SECT_OBJC_PROTOLIST_NEW, &protosize)
	);

	if (protocol_list)
	{
		unsigned long refsize;
		protocol_t** protocol_refs;
		protocol_t** protocol_refs_end;
		std::set<const protocol_t*> setProtocols;
		std::vector<const protocol_t*> vecProtocols;

		protocol_refs = reinterpret_cast<protocol_t**>(
			getsectdata(mh, SEG_OBJC_PROTOREFS_NEW, SECT_OBJC_PROTOREFS_NEW, &refsize)
		);

		if (protocol_refs)
			protocol_refs_end = protocol_refs + refsize / sizeof(protocol_t*);

		std::copy(protocol_list, protocol_list+protosize/sizeof(protocol_t*), std::inserter(setProtocols, setProtocols.begin()));
		topology_sort<const protocol_t>(setProtocols, vecProtocols,
			[&setProtocols,slide](const protocol_t* p) { return p->protocols ?  std::set<const protocol_t*>(p->protocols->begin(slide), p->protocols->end(slide)) : std::set<const protocol_t*>();  });

		for (const protocol_t* proto : vecProtocols)
		{
			Protocol* p = RegisterProtocol(proto, slide);
			
			if (protocol_refs)
				find_and_fix(protocol_refs, protocol_refs_end, proto, p);
		}
	}
}

void ProcessProtocolsOld(const struct mach_header* mh, intptr_t slide)
{
	old_protocol* protocols;
	unsigned long protosize;

	protocols = reinterpret_cast<old_protocol*>(
		getsectdata(mh, SEG_OBJC_PROTOCOLS_OLD, SECT_OBJC_PROTOCOLS_OLD, &protosize)
	);

	if (protocols)
	{
		std::vector<old_protocol*> vecProtocols;
		std::set<old_protocol*> setProtocols;
		// std::map<const char*,old_protocol*> mapProtocols;

		for (size_t i = 0; i < protosize / sizeof(old_protocol); i++)
		{
			if (strcmp(protocols[i].name, "NSObject") != 0) // No need to re-register protocol NSObject
				setProtocols.insert(protocols + i);
			// mapProtocols[protocols[i].name] = protocols + i;
		}

		topology_sort<old_protocol>(setProtocols, vecProtocols,
			[&setProtocols](old_protocol* p) -> std::set<old_protocol*>
			{
				std::set<old_protocol*> set;
				if (p->protocols)
				{
					for (size_t i = 0; i < p->protocols->count; i++)
					{
						if (setProtocols.count(p->protocols->list[i]))
							set.insert(p->protocols->list[i]);
					}
				}
				return set;
			}
		);

		// Find the section that holds protocol_ext structs
		uintptr_t extStart;
		unsigned long extLen = 0;

		extStart = reinterpret_cast<uintptr_t>(
			getsectdata(mh, SEG_OBJC_PROTOEXT_OLD, SECT_OBJC_PROTOEXT_OLD, &extLen)
		);

		for (old_protocol* old : vecProtocols)
		{
			Protocol* p = RegisterProtocol(old, extStart, extLen);
			old->isa = p; // This is what is referenced in the code
		}
	}
}

void ProcessRuntimeNew(const struct mach_header* mh, intptr_t slide, const class_t** classes, unsigned long size)
{
	class_t **class_refs, **class_refs_end, **super_refs, **super_refs_end;
	unsigned long refsize, refsize_s;
	std::vector<const class_t*> vecClasses;
	std::set<const class_t*> setClasses;

	ProcessProtocolsNew(mh, slide);

	class_refs = reinterpret_cast<class_t**>(
		getsectdata(mh, SEG_OBJC_CLASSREFS_NEW, SECT_OBJC_CLASSREFS_NEW, &refsize)
	);
	super_refs = reinterpret_cast<class_t**>(
		getsectdata(mh, SEG_OBJC_SUPERREFS_NEW, SECT_OBJC_SUPERREFS_NEW, &refsize_s)
	);
	if (class_refs)
	{
		class_refs_end = class_refs + refsize / sizeof(class_t*);
		mprotect_pagemult(class_refs, refsize, PROT_READ | PROT_WRITE);
	}
	
	if (super_refs)
	{
		super_refs_end = super_refs + refsize_s / sizeof(class_t*);
		mprotect_pagemult(super_refs, refsize, PROT_READ | PROT_WRITE);
	}

	std::copy(classes, classes+size/sizeof(class_t*), std::inserter(setClasses, setClasses.begin()));

	topology_sort<const class_t>(setClasses, vecClasses,
		[&setClasses](const class_t* t) { return setClasses.count(t->superclass) ? std::set<const class_t*>{t->superclass} : std::set<const class_t*>();  }
	);

	for (const class_t* cls : vecClasses)
	{
		Class c = RegisterClass(cls, slide);

		if (class_refs)
			find_and_fix(class_refs, class_refs_end, cls, c);
		if (super_refs)
			find_and_fix(super_refs, super_refs_end, cls, c);
	}

	if (class_refs)
		mprotect_pagemult(class_refs, refsize, PROT_READ);
	if (super_refs)
		mprotect_pagemult(super_refs, refsize_s, PROT_READ);
}

void UpdateSelectors(const struct mach_header* mh, intptr_t slide)
{
	selref* sel_refs;
	msgref* msg_refs;
	unsigned long selsize, msgsize;

	sel_refs = reinterpret_cast<selref*>(
		getsectdata(mh, SEG_OBJC_SELREFS_NEW, SECT_OBJC_SELREFS_NEW, &selsize)
	);
	
	msg_refs = reinterpret_cast<msgref*>(
		getsectdata(mh, SEG_OBJC_MSGREFS_NEW, SECT_OBJC_MSGREFS_NEW, &msgsize)
	);
	
	if (!sel_refs)
	{
		sel_refs = reinterpret_cast<selref*>(
			getsectdata(mh, SEG_OBJC_SELREFS_OLD, SECT_OBJC_SELREFS_OLD, &selsize)
		);
	}

	if (sel_refs)
	{
		mprotect_pagemult(sel_refs, selsize, PROT_READ | PROT_WRITE);
		
		for (size_t i = 0; i < selsize / sizeof(selref); i++)
		{
			SEL native = sel_getUid(sel_refs[i].selName);
			LOG << "ObjC SEL fixup @" << (sel_refs+i) << ": " << sel_refs[i].sel << " -> " << native << std::endl;
			sel_refs[i].sel = native;
		}

		mprotect_pagemult(sel_refs, selsize, PROT_READ);
	}
	if (msg_refs)
	{
		mprotect_pagemult(msg_refs, msgsize, PROT_READ | PROT_WRITE);
		
		for (size_t i = 0; i < msgsize / sizeof(msgref); i++)
		{
			SEL native = sel_getUid(msg_refs[i].sel.selName);
			LOG << "ObjC msgref fixup @" << &msg_refs[i].sel.sel << ": " << msg_refs[i].sel.sel << " -> " << native << std::endl;
			msg_refs[i].sel.sel = native;
		}

		mprotect_pagemult(msg_refs, msgsize, PROT_READ);
	}
}

void ProcessImageLoad(const struct mach_header* mh, intptr_t slide)
{
	unsigned long size;
	const class_t** classes;

	classes = reinterpret_cast<const class_t**>(
		getsectdata(mh, SEG_OBJC_CLASSLIST_NEW, SECT_OBJC_CLASSLIST_NEW, &size)
	);

	if (!classes)
	{
		// Try the old runtime
		old_class* classes;
		
		classes = reinterpret_cast<old_class*>(
			getsectdata(mh, SEG_OBJC_CLASSLIST_OLD, SECT_OBJC_CLASSLIST_OLD, &size)
		);
		
		if (classes)
		{
			ProcessRuntimeOld(mh, slide, classes, size);
		}
	}
	else
	{
		ProcessRuntimeNew(mh, slide, classes, size);
	}

	UpdateSelectors(mh, slide);
}

void ProcessImageUnload(const struct mach_header* mh, intptr_t)
{
	// TODO
}

