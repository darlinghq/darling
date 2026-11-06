// src/CoreAudio/CoreAudio/pulse/PADeviceRegistry.cpp
/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

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
	// Guard against misuse: calling endRefresh() without a preceding
	// beginRefresh() (or calling it twice in a row) would otherwise wipe
	// the entire registry, since m_touchedThisRefresh would be empty and
	// every entry would look untouched. Treat it as a no-op instead.
	if (!m_inRefresh)
		return;

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
