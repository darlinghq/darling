#include "ASLClient.h"

ASLClient::ASLClient()
	: m_filter(0xffffffff)
{
}

int ASLClient::setFilter(int filter)
{
	m_filter = filter;
}

bool ASLClient::willBeFilteredOut(int level)
{
	return (m_filter & (1 << level)) == 0;
}

