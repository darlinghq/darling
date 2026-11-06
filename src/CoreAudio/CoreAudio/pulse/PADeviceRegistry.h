// src/CoreAudio/CoreAudio/pulse/PADeviceRegistry.h
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

#ifndef PADEVICEREGISTRY_H
#define PADEVICEREGISTRY_H
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Pure bookkeeping: maps a PulseAudio sink/source *name* to a stable
// AudioObjectID (plain uint32_t here to stay independent of CoreAudio
// headers; AudioObjectID is itself just a typedef for UInt32/uint32_t).
//
// Usage per discovery pass:
//   registry.beginRefresh();
//   for each currently-reported PulseAudio name: registry.idForName(name);
//   registry.endRefresh(); // IDs for names not touched since beginRefresh() are released
class PADeviceRegistry
{
public:
	// `firstId` should be greater than 0 to avoid colliding with the
	// AudioObjectID sentinel values (kAudioObjectUnknown / kAudioObjectSystemObject)
	// once this registry is wired into the real HAL layer.
	explicit PADeviceRegistry(uint32_t firstId);

	// Returns the existing ID for `name`, or allocates a new one.
	uint32_t idForName(const std::string& name);

	// Empty string if `id` is not currently assigned to any name.
	std::string nameForId(uint32_t id) const;

	// Marks the start of a discovery pass: any ID not touched by
	// idForName() before the matching endRefresh() is released.
	void beginRefresh();
	void endRefresh();

	// Order of the returned IDs is unspecified; it reflects unordered_map
	// iteration order, not allocation or discovery order.
	std::vector<uint32_t> allIds() const;

private:
	// Monotonically increasing, never reused. Unbounded: after
	// UINT32_MAX - firstId allocations this would overflow, but that is
	// not reachable in practice.
	uint32_t m_nextId;
	std::unordered_map<std::string, uint32_t> m_nameToId;
	std::unordered_map<uint32_t, std::string> m_idToName;
	std::unordered_set<std::string> m_touchedThisRefresh;
	bool m_inRefresh = false;
};

#endif /* PADEVICEREGISTRY_H */
