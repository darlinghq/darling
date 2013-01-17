#include "ASLClient.h"

ASLClient::ASLClient()
	: m_filter(0xffffffff)
{
}

int ASLClient::setFilter(int filter)
{
	m_filter = filter;
	return true;
}

bool ASLClient::willBeFilteredOut(int level)
{
	return (m_filter & (1 << level)) == 0;
}

bool ASLClient::addLogFile(int fd)
{
	m_extraFiles.insert(fd);
	return true;
}

bool ASLClient::removeLogFile(int fd)
{
	m_extraFiles.erase(fd);
	return true;
}

