#ifndef NEW_APPLELAYOUT_H
#define NEW_APPLELAYOUT_H
#include "../common/AppleLayout.h"
#include <iterator>

struct method_t
{
	const char* selName;
	const char* types;
	void* impl;
};

struct method_list_t
{
	uint32_t entsize_and_flags;
	uint32_t count;
	method_t method_list[];

	uint32_t entsize() const { return entsize_and_flags & ~uint32_t(3); }
};

struct ivar_t
{
	uintptr_t* offset;
	const char* name;
	const char* type;
	uint32_t alignment, size;
};

struct ivar_list_t
{
	uint32_t entsize, count;
	ivar_t ivar_list[];
};

struct protocol_list_t;
struct property_list_t;
struct protocol_t
{
	id isa;
	const char* name;
	protocol_list_t* protocols; // inherited protocols
	method_list_t *methods, *staticMethods, *optMethods, *optStaticMethods;
	property_list_t* properties;
	uint32_t size, flags;
	const char** extMethTypes; // reflects the order and count of methods above as listed in this struct

	inline const protocol_t* next() const
	{
		uintptr_t ptr = reinterpret_cast<uintptr_t>(this);
		ptr += size;
		return reinterpret_cast<const protocol_t*>(ptr);
	}
};

template <typename ProtoList>
class protocol_iterator : public std::iterator<std::input_iterator_tag, const protocol_t*>
{
	const ProtoList* m_list;
	size_t m_index;
	intptr_t m_slide;
public:
	protocol_iterator(const ProtoList* list, size_t index, intptr_t slide) : m_list(list), m_index(index), m_slide(slide) {}
	protocol_iterator(const protocol_iterator& that) : m_list(that.m_list), m_index(that.m_index), m_slide(that.m_slide) {}
	protocol_iterator& operator++() { m_index++; return *this; }
	bool operator==(const protocol_iterator& that) { return that.m_index == m_index; }
	bool operator!=(const protocol_iterator& that) { return that.m_index != m_index; }
	const protocol_t* operator*() { return m_list->elem(m_index, m_slide); }
};

struct protocol_list_t
{
	uintptr_t count;
	uintptr_t list[]; // the compiler doesn't add a rebase here
	inline const protocol_t* elem(size_t i, uintptr_t slide) const
	{
		return reinterpret_cast<const protocol_t*>(list[i] + slide);
	}
	protocol_iterator<protocol_list_t> begin(intptr_t slide) const { return protocol_iterator<protocol_list_t>(this, 0, slide); }
	protocol_iterator<protocol_list_t> end(intptr_t slide) const { return protocol_iterator<protocol_list_t>(this, count, slide); }
};

struct property_t
{
	const char *name, *attributes;
};

struct property_list_t
{
	uint32_t entsize, count;
	property_t list[];
};

struct class_ro_t
{
	uint32_t flags, instStart, instSize;
#ifdef __x86_64__
	uint32_t nothing;
#endif
	void* ivarLayout;
	const char* className;
	const method_list_t* baseMethods; // instance methods for classes, static methods for metaclasses
	const protocol_list_t* baseProtocols;
	const ivar_list_t* ivars;

	void* todo[1]; // TODO: more pointers
	const property_list_t* baseProperties;
};

struct class_t
{
	class_t* isa; // instance of
	class_t* superclass;
	void* cache; // empty cache imported here
	void* vtable;
	uintptr_t data_and_flags;

	class_ro_t* data() const
	{
		uintptr_t p = data_and_flags & ~uintptr_t(3);
		return reinterpret_cast<class_ro_t*>(p);
	}
};

struct category_t
{
	const char* name;
	id cls;
	method_list_t *methods, *staticMethods;
	protocol_list_t *protocols;
	property_list_t *properties;
};

#ifndef CLS_CLASS
#define CLS_CLASS               0x1
#define CLS_META                0x2
#define CLS_POSING              0x8
#define CLS_METHOD_ARRAY        0x100
#define CLS_HAS_CXX_STRUCTORS   0x2000
#define CLS_NO_METHOD_ARRAY     0x4000
#define CLS_HAS_LOAD_METHOD     0x8000
#define CLS_NO_PROPERTY_ARRAY	0x80000
#endif

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
static const char* SEG_OBJC_CATLIST_NEW = SEG_DATA;
static const char* SECT_OBJC_CATLIST_NEW = "__objc_catlist";

/* TODO?
 * __DATA,__objc_classlist  was __OBJC2,__class_list
 * __DATA,__objc_catlist  was __OBJC2,__category_list
 * __DATA,__objc_protolist  was __OBJC2,__protocol_list
 * __DATA,__objc_msgrefs  was __OBJC2,__message_refs
 * __DATA,__objc_classrefs  was __OBJC2,__class_refs
 * __DATA,__objc_superrefs  was __OBJC2,__super_refs
 * __DATA,__objc_imageinfo  was __OBJC,__image_info
 * */

#endif

