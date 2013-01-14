#include "ASLResponse.h"

ASLResponse::ASLResponse(const std::vector<ASLMsg*>& data)
	: m_pos(0)
{
	m_data = data;
}

ASLResponse::~ASLResponse()
{
	for (ASLMsg* msg : m_data)
		delete msg;
}

ASLMsg* ASLResponse::next()
{
	if (m_pos >= m_data.size())
		return nullptr;
	return m_data[m_pos++];
}

