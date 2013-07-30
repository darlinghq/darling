#ifndef DYLIBSEARCH_H
#define DYLIBSEARCH_H
#include <util/IniConfig.h>
#include <string>

class MachOObject;

class DylibSearch
{
private:
	DylibSearch();
public:
	static DylibSearch* instance();
	
	std::string resolve(std::string name, MachOObject* requester);
private:
	std::string resolveViaRpath(std::string name, MachOObject* requester);
	static std::string filenameFromPath(const std::string& path);
private:
	IniConfig* m_config;
};

#endif
