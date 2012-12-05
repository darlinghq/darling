#include <cstring>

namespace Darling
{
	typedef bool (*DlsymHookFunc)(char* symName);
	void registerDlsymHook(DlsymHookFunc func);
	void deregisterDlsymHook(DlsymHookFunc func);
};

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
	if (strcmp(name, "__CFConstantStringClassReference") == 0)
	{
		strcpy(name, "_OBJC_CLASS_NSCFString");
		return true;
	}

	return false;
}

