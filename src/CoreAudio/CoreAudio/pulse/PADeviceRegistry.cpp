// src/CoreAudio/CoreAudio/pulse/PADeviceRegistry.cpp
#include "PADeviceRegistry.h"

PADeviceRegistry::PADeviceRegistry(uint32_t firstId)
: m_nextId(firstId)
{
}

uint32_t PADeviceRegistry::idForName(const std::string& name)
{
	if (m_inRefresh)
		m_touchedThisRefresh.insert(name);

	auto it = m_nameToId.find(name);
	if (it != m_nameToId.end())
		return it->second;

	uint32_t id = m_nextId++;
	m_nameToId.emplace(name, id);
	m_idToName.emplace(id, name);
	return id;
}

std::string PADeviceRegistry::nameForId(uint32_t id) const
{
	auto it = m_idToName.find(id);
	if (it == m_idToName.end())
		return std::string();
	return it->second;
}

void PADeviceRegistry::beginRefresh()
{
	m_inRefresh = true;
	m_touchedThisRefresh.clear();
}

void PADeviceRegistry::endRefresh()
{
	m_inRefresh = false;

	for (auto it = m_nameToId.begin(); it != m_nameToId.end(); )
	{
		if (m_touchedThisRefresh.count(it->first) == 0)
		{
			m_idToName.erase(it->second);
			it = m_nameToId.erase(it);
		}
		else
		{
			++it;
		}
	}
	m_touchedThisRefresh.clear();
}

std::vector<uint32_t> PADeviceRegistry::allIds() const
{
	std::vector<uint32_t> ids;
	ids.reserve(m_idToName.size());
	for (const auto& kv : m_idToName)
		ids.push_back(kv.first);
	return ids;
}
