#ifndef CLASSREGISTER_H
#define CLASSREGISTER_H
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <cstddef>
#include <algorithm>
#include <objc/runtime.h>
#include "AppleLayout.h"
#include "../util/log.h"

/* TODO?
 * __DATA,__objc_classlist  was __OBJC2,__class_list
 * __DATA,__objc_catlist  was __OBJC2,__category_list
 * __DATA,__objc_protolist  was __OBJC2,__protocol_list
 * __DATA,__objc_msgrefs  was __OBJC2,__message_refs
 * __DATA,__objc_classrefs  was __OBJC2,__class_refs
 * __DATA,__objc_superrefs  was __OBJC2,__super_refs
 * __DATA,__objc_imageinfo  was __OBJC,__image_info
 * */

static const char* SEG_DATA = "__DATA";
static const char* SEG_OBJC_CLASSLIST_NEW = SEG_DATA;
static const char* SECT_OBJC_CLASSLIST_NEW = "__objc_classlist";
static const char* SEG_OBJC_CLASSREFS_NEW = SEG_DATA;
static const char* SECT_OBJC_CLASSREFS_NEW = "__objc_classrefs";
static const char* SEG_OBJC_SUPERREFS_NEW = SEG_DATA;
static const char* SECT_OBJC_SUPERREFS_NEW = "__objc_superrefs";
static const char* SEG_OBJC_SELREFS_NEW = SEG_DATA;
static const char* SECT_OBJC_SELREFS_NEW = "__objc_selrefs";
static const char* SEG_OBJC_MSGREFS_NEW = SEG_DATA; // used with objc_msgSend_fixup
static const char* SECT_OBJC_MSGREFS_NEW = "__objc_msgrefs";
static const char* SEG_OBJC_PROTOREFS_NEW = SEG_DATA;
static const char* SECT_OBJC_PROTOREFS_NEW = "__objc_protorefs";
static const char* SEG_OBJC_PROTOLIST_NEW = SEG_DATA;
static const char* SECT_OBJC_PROTOLIST_NEW = "__objc_protolist";

static const char* SEG_OBJC = "__OBJC";
static const char* SEG_OBJC_CLASSLIST_OLD = SEG_OBJC;
static const char* SECT_OBJC_CLASSLIST_OLD = "__class";
static const char* SEG_OBJC_METALIST_OLD = SEG_OBJC;
static const char* SECT_OBJC_METALIST_OLD = "__meta_class";
static const char* SEG_OBJC_CLASSREFS_OLD = SEG_OBJC;
static const char* SECT_OBJC_CLASSREFS_OLD = "__cls_refs";
static const char* SEG_OBJC_SELREFS_OLD = SEG_OBJC;
static const char* SECT_OBJC_SELREFS_OLD = "__message_refs";
static const char* SEG_OBJC_PROTOCOLS_OLD = SEG_OBJC;
static const char* SECT_OBJC_PROTOCOLS_OLD = "__protocol";
static const char* SEG_OBJC_PROTOEXT_OLD = SEG_OBJC;
static const char* SECT_OBJC_PROTOEXT_OLD = "__protocol_ext";

static void ProcessImageLoad(const struct mach_header* mh, intptr_t slide);
static void ProcessImageUnload(const struct mach_header* mh, intptr_t slide);
static Class RegisterClass(const class_t* cls, intptr_t slide);
static Class RegisterClass(old_class* cls);
static void ProcessProtocolsNew(const struct mach_header* mh, intptr_t slide);
static void ProcessProtocolsOld(const struct mach_header* mh, intptr_t slide);
static Protocol* RegisterProtocol(const protocol_t* prot, intptr_t slide);
static Protocol* RegisterProtocol(old_protocol* prot, uintptr_t extStart, unsigned long extLen);
static void UpdateSelectors(const struct mach_header* mh, intptr_t slide);
static void ProcessRuntimeOld(const struct mach_header* mh, intptr_t slide, old_class* classes, unsigned long size);
static void ProcessRuntimeNew(const struct mach_header* mh, intptr_t slide, const class_t** classes, unsigned long size);
static void ConvertProperties(Class c, const property_list_t* props);
static void RegisterProtocolMethods(Protocol* p, const method_list_t* list, const char** extTypes, size_t& extIndex, bool required, bool instance);
static void RegisterProtocolMethods(Protocol* p, const old_method_decl_list* list, bool required, bool instance);

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

// mprotect() requires that mem be on a page boundary.
// This function satisfies this requirement.
static int mprotect_pagemult(void* mem, size_t len, int prot);

template<typename T>
int mprotect_pagemult(T** mem, size_t len, int prot)
{
	return mprotect_pagemult(reinterpret_cast<void*>(mem), len, prot);
}

/* UNUSED
template<typename ForwardIterator, typename T, typename Processor> process_split_sections(ForwardIterator start, ForwardIterator end, const T& sep, Processor proc)
{
	ForwardIterator beg = start;
	while (start != end)
	{
		if (*start == sep)
		{
			proc(beg, start);
			start++;
			beg = start;
		}
		else
			start++;
	}

	if (beg != end)
		proc(beg, end);
}
*/

#endif

