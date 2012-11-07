#include "boostxml.h"

boostxml::boostxml()
{
	m_xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	m_xml << "<TestLog>\n";
	m_xml << "\t<TestSuite name=\"Darling\">\n";
}

void boostxml::addOK(const std::string& path, int time)
{
	m_xml << "\t\t<TestCase name=\"" << extractName(path) << "\">\n";
	m_xml << "\t\t\t<TestingTime>" << time << "</TestingTime>\n";
	m_xml << "\t\t</TestCase>\n";
}

void boostxml::addFailure(const std::string& path, int time, const std::string& error)
{
	m_xml << "\t\t<TestCase name=\"" << extractName(path) << "\">\n";
	m_xml << "\t\t\t<TestingTime>" << time << "</TestingTime>\n";
	m_xml << "\t\t\t<Error file=\"" << path << "\" line=\"0\">\n";
	m_xml << "<![CDATA[" << error << "]]>\n";
	m_xml << "\t\t\t</Error>\n";
	m_xml << "\t\t</TestCase>\n";
}

std::string boostxml::str()
{
	m_xml << "\t</TestSuite>\n</TestLog>\n";
	return m_xml.str();
}

std::string boostxml::extractName(const std::string& path)
{
	size_t pos = path.rfind('/');
	if (pos == std::string::npos)
		return path;
	else
		return path.substr(pos+1);
}

