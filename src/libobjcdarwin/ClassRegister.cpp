#include "ClassRegister.h"
#include "../dyld/public.h"
#include "../util/trace.h"
#include "../util/log.h"

#ifndef OBJC_ABI_2
#	include "old/protocol.h"
#	include "old/class.h"
#	include "old/category.h"
#else
#	include "new/protocol.h"
#	include "new/class.h"
#	include "new/category.h"
#endif
#include "common/selector.h"
#include <map>
#include <queue>
#include "NSDarwinFramework.h"

// Superclass references in Mach-O don't use classref
// Neither do category class references
std::map<const void*,Class> g_classPointers;
std::queue<std::pair<Class,IMP>> g_pendingInitClasses;

static void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace)
{
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos)
	{
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}

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


void ProcessImageLoad(const struct mach_header* mh, intptr_t slide)
{
	unsigned long size;
	std::vector<const char*> classNames;
	void* dataPtr = nullptr;

#ifdef OBJC_ABI_2
	const class_t** classes;
	ProcessProtocolsNew(mh, slide);

	classes = reinterpret_cast<const class_t**>(
		getsectdata(mh, SEG_OBJC_CLASSLIST_NEW, SECT_OBJC_CLASSLIST_NEW, &size)
	);
	if (classes)
	{
		classNames = ProcessClassesNew(mh, slide, classes, size);
		dataPtr = (void*)*classes;
	}

	ProcessCategoriesNew(mh, slide);
#else
	module_info* modinfo;
	ProcessProtocolsOld(mh, slide);

	modinfo = reinterpret_cast<module_info*>(
		getsectdata(mh, SEG_OBJC_MODINFO_OLD, SECT_OBJC_MODINFO_OLD, &size)
	);

	if (modinfo && modinfo->symtab)
	{
		classNames = ProcessClassesOld(mh, slide, modinfo);
		dataPtr = modinfo;
		ProcessCategoriesOld(mh, slide, modinfo);
	}
#endif

	UpdateClassRefs(mh);
	UpdateSelectors(mh, slide);

	// Do this only for dynamic libs (slide > 0)
	if (!classNames.empty() && slide > 0)
	{
		// Generate a NSFramework_XXXX class for GNUstep's NSBundle
		const char* path = dyld_image_path_containing_address(dataPtr);
		RegisterFramework(&classNames[0], classNames.size(), path);
	}

	static SEL selInit = sel_getUid("load");
	while (!g_pendingInitClasses.empty())
	{
		auto pair = g_pendingInitClasses.front();
		g_pendingInitClasses.pop();
		pair.second(reinterpret_cast<objc_object*>(pair.first), selInit);
	}
}

void ProcessImageUnload(const struct mach_header* mh, intptr_t)
{
	// TODO
}

