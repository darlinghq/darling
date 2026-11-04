// src/CoreAudio/CoreAudio/pulse/PADeviceRegistry.h
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
	explicit PADeviceRegistry(uint32_t firstId);

	// Returns the existing ID for `name`, or allocates a new one.
	uint32_t idForName(const std::string& name);

	// Empty string if `id` is not currently assigned to any name.
	std::string nameForId(uint32_t id) const;

	// Marks the start of a discovery pass: any ID not touched by
	// idForName() before the matching endRefresh() is released.
	void beginRefresh();
	void endRefresh();

	std::vector<uint32_t> allIds() const;

private:
	uint32_t m_nextId;
	std::unordered_map<std::string, uint32_t> m_nameToId;
	std::unordered_map<uint32_t, std::string> m_idToName;
	std::unordered_set<std::string> m_touchedThisRefresh;
	bool m_inRefresh = false;
};

#endif
