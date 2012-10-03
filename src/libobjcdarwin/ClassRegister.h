#ifndef CLASSREGISTER_H
#define CLASSREGISTER_H
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <cstddef>
#include <objc/runtime.h>

#ifndef INTPTR_MAX
#error
#endif

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
	method_t method_list[0];

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

struct class_ro_t
{
	uint32_t flags, instStart, instSize;
#ifdef __x86_64__
	uint32_t nothing;
#endif
	void* ivarLayout;
	const char* className;
	const method_list_t* baseMethods; // instance methods for classes, static methods for metaclasses
	const void* baseProtocols;
	const ivar_list_t* ivars;

	void* todo[2]; // TODO: two more pointers
};

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
	struct old_property_list **propertyLists;
};

struct old_category
{
	char *category_name;
	char *class_name;
	struct old_method_list *instance_methods;
	struct old_method_list *class_methods;
	struct old_protocol_list *protocols;
	uint32_t size;
	struct old_property_list *instance_properties;
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


extern "C" Class objcdarwin_class_lookup(const class_t* cls);

#endif

