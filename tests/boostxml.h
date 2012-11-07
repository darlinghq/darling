#ifndef BOOSTXML_H
#define BOOSTXML_H
#include <sstream>
#include <string>

class boostxml
{
public:
	boostxml();
	std::string str();

	void addOK(const std::string& path, int time);
	void addFailure(const std::string& path, int time, const std::string& error);
private:
	static std::string extractName(const std::string& path);
private:
	std::stringstream m_xml;
};

#endif

