#include <cstring>
#include <memory>
#include <dlfcn.h>

__attribute__((visibility ("default"))) extern "C" void* _objc_empty_cache = 0;
__attribute__((visibility ("default"))) extern "C" void* _objc_empty_vtable = 0;

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

	return false;
}

