#include "ControlFile.h"
#include <fstream>
#include <stdexcept>
#include <util/Regexp.h>

ControlFile::ControlFile(const std::string& path)
{
	std::ifstream file(path);
	if (!file.is_open())
		throw std::runtime_error("Cannot open .la file");
	
	parse(file);
}

std::string ControlFile::get(const std::string& key) const
{
	auto it = m_values.find(key);
	if (it == m_values.end())
		throw std::runtime_error("Given key not found in .la file");
	else
		return it->second;
}

void ControlFile::parse(std::istream& stream)
{
	static Darling::Regexp reUnquoted("([^=]+)=(.*)");
	static Darling::Regexp reQuoted("([^=]+)='([^']+)'");
	
	while (!stream.eof())
	{
		std::string line;
		
		std::getline(stream, line);
		
		if (line.empty() || line[0] == '#')
			continue;
		
		if (reQuoted.matches(line))
			m_values[reQuoted.group(1)] = reQuoted.group(2);
		else if (reUnquoted.matches(line))
			m_values[reUnquoted.group(1)] = reUnquoted.group(2);
	}
}
