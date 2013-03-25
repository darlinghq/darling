#include <cstring>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include "../util/stlutils.h"

namespace Darling
{
    typedef bool (*DlsymHookFunc)(char* symName);
    void registerDlsymHook(DlsymHookFunc func);
    void deregisterDlsymHook(DlsymHookFunc func);
};

static const char* g_directmap =
#include "namemap.lst"
;

static std::map<std::string,std::string>* g_nameMap;

static bool NameTranslator(char* symName);

__attribute__((constructor))
	static void initTranslation()
{
	std::istringstream istr(g_directmap);
	std::string line;

	g_nameMap = new std::map<std::string,std::string>;

	while (std::getline(istr, line))
	{
		if (line.empty() || line[0] == '#')
			continue;
		
		std::vector<std::string> segs = string_explode(line, ';');
		if (segs.size() < 2)
			continue;

		if (segs[0].compare(0, 3, "32!") == 0)
		{
#ifdef __x86_64__
			continue;
#else
			segs[0] = segs[0].substr(3);
#endif
		}

		if (segs[0].compare(0, 3, "64!") == 0)
		{
#ifdef __i386__
			continue;
#else
			segs[0] = segs[0].substr(3);
#endif
		}

		// std::cout << segs[0] << " -> " << segs[1] << std::endl;
		(*g_nameMap)[segs[0]] = segs[1];
	}

	Darling::registerDlsymHook(NameTranslator);
}

__attribute__((destructor))
	static void exitTranslation()
{
	Darling::deregisterDlsymHook(NameTranslator);
	delete g_nameMap;
}

bool NameTranslator(char* symName)
{
	auto it = g_nameMap->find(symName);
	if (it != g_nameMap->end())
	{
		strcpy(symName, it->second.c_str());
		return true;
	}
	
	it = g_nameMap->find(std::string("__darwin_")+symName);
	if (it != g_nameMap->end())
	{
		strcpy(symName, it->second.c_str());
		return true;
	}
	
	return false;
}

