#ifndef CONTROLFILE_H
#define CONTROLFILE_H
#include <string>
#include <map>
#include <istream>

// Parses .la files
class ControlFile
{
public:
	ControlFile(const std::string& path);
	std::string get(const std::string& key) const;
private:
	void parse(std::istream& stream);
private:
	std::map<std::string,std::string> m_values;
};

#endif
