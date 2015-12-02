#include <cstring>
#include <string>
#include <map>

namespace Darling
{
	typedef bool (*DlsymHookFunc)(char* symName);
	void registerDlsymHook(DlsymHookFunc func);
	void deregisterDlsymHook(DlsymHookFunc func);
};

const char* const g_mapping[] = { "OBJC_CLASS_$_NSBundle", "OBJC_METACLASS_$_NSBundle", "_OBJC_CLASS_NSDarwinBundle", "_OBJC_METACLASS_NSDarwinBundle" };


static bool NameTranslator(char* symName);

__attribute__((constructor))
	static void initTranslation()
{
	Darling::registerDlsymHook(NameTranslator);
}

__attribute__((destructor))
	static void exitTranslation()
{
	Darling::deregisterDlsymHook(NameTranslator);
}

static bool NameTranslator(char* name)
{
	for(size_t i = 0; i < (sizeof(g_mapping)/sizeof(const char* const)) >> 1; i++)
	{
		if(strcmp(name, g_mapping[i]) == 0)
		{
			strcpy(name, g_mapping[i + ((sizeof(g_mapping)/sizeof(const char* const)) >> 1)]);
			return true;
		}
	}

	return false;
}
