#ifndef APPLELAYOUT_H
#define APPLELAYOUT_H
#include "../common/AppleLayout.h"

struct old_method_decl
{
	const char *name, *types;
};
struct old_method_decl_list
{
	int count;
	old_method_decl list[];
};
struct old_property
{
	const char *name, *attributes;
};

struct old_property_list
{
	uint32_t entsize, count;
	old_property list[];
};

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
	union
	{
		old_property_list **propertyLists;
		old_property_list *propertyList;
	};
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

union clsref
{
	const char* clsName;
	Class cls;
};

struct symtab
{
	unsigned long selCount;
	SEL* selectors;
	uint16_t countClasses, countCategories;
	void* classesAndCategories[];
};

struct module_info
{
	unsigned long version, size;
	const char* name;
	struct symtab* symtab;
};

struct old_category
{
	const char* name;
	const char* clsName;
	old_method_list *methods, *staticMethods;
	old_protocol_list* protocols; // since version 5
	uint32_t size;
	old_property_list* properties; // since version 7
};


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
static const char* SEG_OBJC_MODINFO_OLD = SEG_OBJC;
static const char* SECT_OBJC_MODINFO_OLD = "__module_info";

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

#endif

