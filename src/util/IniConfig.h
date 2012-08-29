#ifndef UTIL_INICONFIG_H
#define UTIL_INICONFIG_H
#include <map>
#include <string>
#include <vector>
#include <fstream>

class IniConfig
{
public:
	IniConfig(const char* path);
	~IniConfig();
	
	typedef std::map<std::string, std::string> ValueMap;
	typedef std::map<std::string, ValueMap*> SectionMap;

	std::vector<std::string> getSections() const;
	const ValueMap* getSection(std::string section) const;

	inline bool hasSection(const char* name) const { return getSection(name) != 0; }
protected:
	void loadConfig(std::ifstream& f);
	static std::string& trim(std::string& str);
	static std::string& removeComment(std::string& str);
private:
	SectionMap m_sections;
};

#endif

