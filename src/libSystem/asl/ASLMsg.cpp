#include "ASLMsg.h"
#include "asl.h"

ASLMsg::ASLMsg(int type)
	: m_type(type)
{
}

const char* ASLMsg::get(const char* key) const
{
	auto it = m_pairs.find(key);
	if (it == m_pairs.end())
		return nullptr;
	else
		return it->second.value.c_str();
}

bool ASLMsg::set(const char* key, const char* value)
{
	return set(key, 0, value);
}

bool ASLMsg::set(const char* key, int op, const char* value)
{
	if (op && m_type != ASL_TYPE_QUERY)
		return false;

	MapValue v = { value, op };
	m_pairs[key] = v;
	return true;
}

const char* ASLMsg::get(int index) const
{
	auto it = m_pairs.begin();
	const char* retval = nullptr;

	while (index > 0 && it != m_pairs.end())
	{
		it++;
		index--;
	}

	if (it != m_pairs.end())
		return it->second.value.c_str();
	else
		return nullptr;
}

bool ASLMsg::unset(const char* key)
{
	m_pairs.erase(key);
	return true;
}

