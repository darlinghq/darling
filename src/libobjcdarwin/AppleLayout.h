#ifndef APPLELAYOUT_H
#define APPLELAYOUT_H
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

struct old_method_decl
{
	const char *name, *types;
};
struct old_method_decl_list
{
	int count;
	old_method_decl list[];
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
typedef property_t old_property;
typedef property_list_t old_property_list;

struct old_protocol;
struct old_protocol_list
{
	old_protocol_list* linked;
	long count;
	old_protocol* list[];
};

struct old_protocol_ext
{
	uint32_t size;
	old_method_decl_list *optMethods, *optStaticMethods;
	old_property_list* properties;
	const char** extMethTypes;
};

struct old_protocol
{
	union
	{
		id isa;
		old_protocol_ext* ext; // __protocol_ext section
		uintptr_t ext_ptrValue;
	};
	const char* name;
	old_protocol_list* protocols;
	old_method_decl_list *methods, *staticMethods;
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

#if 0 // UNUSED
struct class_rw_t
{
	uint32_t flags, version;
	class_ro_t* ro;

	union
	{
		method_list_t* method_list;
		method_list_t** method_lists;
	};

	void* todo[4]; // TODO: four more pointers
};
#endif

struct class_t
{
	class_t* isa; // instance of
	class_t* superclass;
	void* cache; // empty cache imported here
	void* vtable;
	uintptr_t data_and_flags;

	// TODO: WTF? Should be rw data
	class_ro_t* data() const
	{
		uintptr_t p = data_and_flags & ~uintptr_t(3);
		return reinterpret_cast<class_ro_t*>(p);
	}
};

union old_class_ptr
{
	struct old_class* cls;
	const char* name;
	uintptr_t ptrValue;
	Class clsNew;
};

struct old_class
{
	old_class_ptr isa;
	old_class_ptr super_class;
	const char *name;
	long version;
	long info;
	long instance_size;
	struct old_ivar_list *ivars;
	struct old_method_list *methodList;
	void* cache;
	struct old_protocol_list *protocols;
	// CLS_EXT only
	const uint8_t *ivar_layout;
	struct old_class_ext *ext;
};

struct old_class_ext
{
	uint32_t size;
	const uint8_t *weak_ivar_layout;
	old_property_list **propertyLists;
};

struct old_category
{
	char *category_name;
	char *class_name;
	struct old_method_list *instance_methods;
	struct old_method_list *class_methods;
	struct old_protocol_list *protocols;
	uint32_t size;
	old_property_list *instance_properties;
};

struct old_ivar
{
	char *name;
	char *type;
	int offset;
#ifdef __x86_64__
	int space;
#endif
};

struct old_ivar_list
{
	int count;
#ifdef __x86_64__
	int space;
#endif
	/* variable length structure */
	struct old_ivar ivar_list[1];
};


struct old_method
{
	const char* selName;
	const char* types;
    void* impl;
};

struct old_method_list
{
	struct old_method_list *obsolete;

	int count;
#ifdef __x86_64__
	int space;
#endif
	/* variable length structure */
	struct old_method method_list[1];
};

union selref
{
	const char* selName;
	SEL sel;
};
struct msgref
{
	void* objc_msgSend_fixup; // function pointer
	selref sel;
};
union clsref
{
	const char* clsName;
	Class cls;
};

#endif

