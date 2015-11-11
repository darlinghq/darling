#ifndef DYLIBSEARCH_H
#define DYLIBSEARCH_H
#include <util/IniConfig.h>
#include <regex>
#include <string>
#include <vector>

namespace Darling {

class MachOObject;

class DylibSearch
{
private:
public:
	DylibSearch();
	
	// Returns the default instance
	static DylibSearch* instance();
	
	// Colon-separated list of files
	void setAdditionalPaths(const std::string& paths);
	void setAdditionalSuffixes(const std::vector<std::string>& suffixes);
	
	std::string resolve(std::string name, MachOObject* requester);
private:
	std::string resolveViaRpath(std::string name, MachOObject* requester);
	std::string resolveInLdPath(std::string name);
	std::string resolveInPathList(std::string name, const std::vector<std::string>& paths);
	const char* resolveAlias(const std::string& library);
	static std::string filenameFromPath(const std::string& path);
	static std::string realpath(const std::string& path);
	// Uses access() to check for existence + tries extra suffixes
	std::string checkPresence(const std::string& path);
private:
	IniConfig* m_config;
	std::regex m_reFrameworkPath, m_reDefaultFrameworkPath;
	std::vector<std::string> m_extraPaths, m_suffixes;
};

}

#endif
