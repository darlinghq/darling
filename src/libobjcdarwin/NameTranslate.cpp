#include <cstring>
#include <memory>
#include <dlfcn.h>
#include <cstdio>
#include "visibility.h"

DARLING_VISIBLE extern "C" void* _objc_empty_cache = 0;
DARLING_VISIBLE extern "C" void* _objc_empty_vtable = 0;

#ifdef __x86_64__
DARLING_VISIBLE extern "C" void* _objc_dummy_ehtype = 0;
#endif

namespace Darling
{
	typedef bool (*DlsymHookFunc)(char* symName);
	void registerDlsymHook(DlsymHookFunc func);
	void deregisterDlsymHook(DlsymHookFunc func);
};

static bool ClassTranslator(char* symName);

__attribute__((constructor))
	static void initTranslation()
{
	Darling::registerDlsymHook(ClassTranslator);
}

__attribute__((destructor))
	static void exitTranslation()
{
	Darling::deregisterDlsymHook(ClassTranslator);
}

static bool ClassTranslator(char* name)
{
#ifdef __x86_64__
	if (strncmp(name, "OBJC_EHTYPE_$_", 14) == 0)
	{
		strcpy(name, "_objc_dummy_ehtype");
		return true;
	}
	else
#endif
	
	if (char* off = strstr(name, "_$_"))
	{
		size_t offset = off - name;
		
		memmove(name+1, name, strlen(name)+1);
		*name = '_'; // prefix with an underscore
		offset++;
		// remove $_
		memmove(name + offset, name + offset + 2, strlen(name + offset + 2)+1);

		return true;
	}
	// Or we could create a variable like this and memcpy the objc_class
	else if (strcmp(name, "__CFConstantStringClassReference") == 0)
	{
		strcpy(name, "_OBJC_CLASS_NSCFString");
		return true;
	}
#ifdef __x86_64__
	else if (strcmp(name, "__objc_personality_v0") == 0)
	{
		strcpy(name, "__gnu_objc_personality_v0");
		return true;
	}
#endif

	return false;
}

