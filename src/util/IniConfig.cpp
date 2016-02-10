#include "IniConfig.h"
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <ctype.h>
#include <iostream>

IniConfig::IniConfig(const char* path, bool silentFail)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		if (!silentFail)
			throw std::runtime_error("Specified config file not found!");
	}
	else
		loadConfig(file);
}

IniConfig::~IniConfig()
{
	for (auto mv : m_sections)
		delete mv.second;
}

std::vector<std::string> IniConfig::getSections() const
{
	std::vector<std::string> names;

	std::transform(m_sections.begin(), m_sections.end(), std::back_inserter(names),
			[](const SectionMap::value_type& v) { return v.first; });

	return names;
}

void IniConfig::loadConfig(std::ifstream& f)
{
	std::string line, section;
	ValueMap* valMap = 0;

	while (std::getline(f, line))
	{
		trim(removeComment(line));

		if (line.empty())
			continue;

		if (line[0] != '[' || *line.rbegin() != ']')
		{
			if (!valMap)
			{
				std::cerr << "Skipping invalid config file line: " << line << std::endl;
				continue;
			}
			else
			{
				size_t pos = line.find('=');
				if (pos == std::string::npos)
				{
					std::cerr << "Skipping invalid config file line: " << line << std::endl;
					continue;
				}
				else
				{
					std::string key, value;
					key = line.substr(0, pos);
					value = line.substr(pos+1);
					trim(key);
					trim(value);

					(*valMap)[key] = value;
				}
			}
		}
		else
		{
			if (valMap)
				m_sections[section] = valMap;

			valMap = new ValueMap;
			section = line.substr(1, line.size()-2);
		}
	}

	if (valMap)
		m_sections[section] = valMap;
}

std::string& IniConfig::removeComment(std::string& str)
{
	size_t pos = str.find('#');
	if (pos != std::string::npos)
		str.resize(pos);
	return str;
}

std::string& IniConfig::trim(std::string& str)
{
	int s, e;

	for (s = 0; s < str.size() && isspace(str[s]); s++);

	for (e = str.size(); e > 0 && isspace(str[e-1]); e--);

	str = str.substr(s, e-s);
	return str;
}

const IniConfig::ValueMap* IniConfig::getSection(const std::string& section) const
{
	SectionMap::const_iterator it = m_sections.find(section);
	if (it == m_sections.end())
		return 0;
	else
		return it->second;
}

#ifdef TEST

int main(int argc, char** argv)
{
	IniConfig config(argv[1]);

	std::cout << "Sections: ";
	for (std::string s : config.getSections())
		std::cout << s << ' ';
	std::cout << std::endl;

	for (std::string s : config.getSections())
	{
		const IniConfig::ValueMap* vals = config.getSection(s);
		std::cout << "Section " << s << std::endl;

		for (auto it = vals->begin(); it != vals->end(); it++)
		{
			std:: cout << it->first << " -> " << it->second << std::endl;
		}
	}

	return 0;
}

#endif

// C API

iniconfig_t iniconfig_load(const char* path)
{
	try
	{
		return new IniConfig(path);
	}
	catch (...)
	{
		return nullptr;
	}
}

void iniconfig_free(iniconfig_t config)
{
	delete config;
}

inivalmap_t iniconfig_getsection(iniconfig_t config, const char* section)
{
	return config->getSection(section);
}

const char* iniconfig_valmap_get(inivalmap_t map, const char* key)
{
	auto it = map->find(key);
	if (it == map->end())
		return nullptr;
	else
		return it->second.c_str();
}

