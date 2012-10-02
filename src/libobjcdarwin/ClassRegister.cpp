#include "ClassRegister.h"
#include "../dyld/public.h"
#include "class.h"
#include "method_list.h"
#include <iostream>
#include <map>
#include <cstring>
#include <cassert>

/*
__DATA,__objc_classlist  was __OBJC2,__class_list
__DATA,__objc_catlist  was __OBJC2,__category_list
__DATA,__objc_protolist  was __OBJC2,__protocol_list
__DATA,__objc_msgrefs  was __OBJC2,__message_refs
__DATA,__objc_classrefs  was __OBJC2,__class_refs
__DATA,__objc_superrefs  was __OBJC2,__super_refs
__DATA,__objc_imageinfo  was __OBJC,__image_info
*/

#define SEG_OBJC_CLASSLIST_NEW "__DATA"
#define SECT_OBJC_CLASSLIST_NEW "__objc_classlist"
#define SEG_OBJC_CLASSLIST_OLD "__OBJC"
#define SECT_OBJC_CLASSLIST_OLD "__class" // TODO: check if correct

static std::map<const void*, objc_class*> g_convertedClasses;
static std::pair<uintptr_t, uintptr_t> g_cstringSection;

static void ProcessImageLoad(const struct mach_header* mh, intptr_t slide);
static void ProcessImageUnload(const struct mach_header* mh, intptr_t slide);
static void RegisterClass(const class_t* cls);
static void RegisterClass(const old_class* cls);
static old_class* PtrToClass(old_class_ptr ptr);

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

Class objcdarwin_class_lookup(const class_t* cls)
{
	//std::cout << "Looking up " << cls << std::endl << std::flush;
	auto it = g_convertedClasses.find(cls);
	if (it != g_convertedClasses.end())
		return it->second;
	else
		return Class(cls); // assume native or instance ptr
}

// This is very wrong. Does it have to be this way?s
old_class* PtrToClass(old_class_ptr ptr)
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

template<typename ListType> void ConvertMethodListGen(Class c, const ListType* list)
{
	std::cout << list->count << " methods within\n";
	
	for (size_t i = 0; i < list->count; i++)
	{
		auto* m = &list->method_list[i];
		
		std::cout << "Method: selName: " << m->selName << "; types: " << m->types << "; impl: " << m->impl << std::endl;

		SEL sel = sel_registerName(m->selName);
		class_addMethod(c, sel, reinterpret_cast<IMP>(m->impl), m->types);
	}
}

void RegisterClass(const class_t* cls)
{
	std::cout << "Processing ObjC class " << cls->data()->className;
	std::cout << std::endl;
	
	// objc_allocateClassPair
	// objc_registerClassPair
	const class_t* meta = cls->isa;
	Class conv, super;
	if (g_convertedClasses.find(cls->superclass) != g_convertedClasses.end())
		super = g_convertedClasses[cls->superclass];
	else
		super = reinterpret_cast<objc_class*>(cls->superclass);
	
	conv = objc_allocateClassPair(super, cls->data()->className, 0);
	
	const class_ro_t* ro = cls->data();
	const class_ro_t* roMeta = meta->data();
	
	if (ro->baseMethods)
		ConvertMethodListGen(conv, ro->baseMethods);
	if (roMeta->baseMethods)
		ConvertMethodListGen(conv->isa, roMeta->baseMethods);
	
	conv->instance_size = ro->instSize;
	conv->isa->instance_size = roMeta->instSize;
	
	objc_registerClassPair(conv);
	g_convertedClasses[cls] = conv;
	g_convertedClasses[meta] = conv->isa;
}

void RegisterClass(const old_class* cls)
{
	std::cout << "Processing old ObjC class " << cls->name;
	std::cout << std::endl;
	
	const old_class* meta = PtrToClass(cls->isa);
	Class conv, super;
	old_class* psuper = PtrToClass(cls->super_class);
	
	if (g_convertedClasses.find(psuper) != g_convertedClasses.end())
		super = g_convertedClasses[psuper];
	else
		super = reinterpret_cast<objc_class*>(psuper);
	
	conv = objc_allocateClassPair(super, cls->name, 0);
	
	std::cout << "Instance methods...\n";
	if (cls->methodList)
		ConvertMethodListGen(conv, cls->methodList);
	std::cout << "Class methods...\n";
	if (meta->methodList)
		ConvertMethodListGen(conv->isa, meta->methodList);
	
	objc_registerClassPair(conv);
	
	assert(id(conv) == objc_getClass(cls->name));
	assert(id(conv->isa) == objc_getMetaClass(cls->name));
	assert(class_isMetaClass(conv->isa));
	assert(!class_isMetaClass(conv));
	
	g_convertedClasses[cls] = conv;
	g_convertedClasses[cls->name] = conv;
	g_convertedClasses[meta] = conv->isa;
	
	/*
	SEL ss = sel_get_any_uid("doHello:::::::");
	std::cout << "objc_msg_lookup: " << conv << ' ' << ss << std::endl;
	void* p = (void*) objc_msg_lookup(id(conv), ss);
	
	std::cout << "Impl: " << p << "; Sel: " << ss << std::endl;
	*/
}

void ProcessImageLoad(const struct mach_header* mh, intptr_t)
{
	unsigned long size;
	const class_t** classes;

	classes = reinterpret_cast<const class_t**>(
		getsectdata(mh, SEG_OBJC_CLASSLIST_NEW, SECT_OBJC_CLASSLIST_NEW, &size)
	);

	if (!classes)
	{
		// Try the old runtime
		const old_class* classes;
		
		classes = reinterpret_cast<const old_class*>(
			getsectdata(mh, SEG_OBJC_CLASSLIST_OLD, SECT_OBJC_CLASSLIST_OLD, &size)
		);
		
		if (classes)
		{
			// g_cstringSection
			unsigned long cstrLen;
			void* ptr;
			
			ptr = getsectdata(mh, "__TEXT", "__cstring", &cstrLen);
			if (ptr)
				g_cstringSection = std::pair<uintptr_t,uintptr_t>(uintptr_t(ptr), cstrLen);
			else
				g_cstringSection = std::pair<uintptr_t,uintptr_t>(0, 0);
			
			for (size_t i = 0; i < size / sizeof(old_class); i++)
				RegisterClass(classes+i);
		}
	}
	else
	{
		for (size_t i = 0; i < size / sizeof(class_t*); i++)
			RegisterClass(classes[i]);
	}
}

void ProcessImageUnload(const struct mach_header* mh, intptr_t)
{
	// TODO
}

