#include <cstring>
#include <string>
#include <map>

namespace Darling
{
	typedef bool (*DlsymHookFunc)(char* symName);
	void registerDlsymHook(DlsymHookFunc func);
	void deregisterDlsymHook(DlsymHookFunc func);
};

static const std::map<std::string, std::string> g_mapping {
	std::make_pair<std::string,std::string>("OBJC_CLASS_$_NSBundle", "_OBJC_CLASS_NSDarwinBundle"),
	std::make_pair<std::string,std::string>("OBJC_METACLASS_$_NSBundle", "_OBJC_METACLASS_NSDarwinBundle"),
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
	auto it = g_mapping.find(name);
	if (it != g_mapping.end())
	{
		strcpy(name, it->second.c_str());
		return true;
	}

	return false;
}

