#include "../PADeviceRegistry.h"
#include <cassert>
#include <cstdio>
#include <vector>
#include <string>

int main()
{
	PADeviceRegistry registry(1000); // first ID handed out is 1000

	// Fresh names get fresh, increasing IDs.
	uint32_t idA = registry.idForName("alsa_output.pci-0000_00_1f.3.analog-stereo");
	uint32_t idB = registry.idForName("alsa_output.usb-Blue_Microphones.analog-stereo");
	assert(idA == 1000);
	assert(idB == 1001);

	// Re-querying the same name returns the same ID.
	assert(registry.idForName("alsa_output.pci-0000_00_1f.3.analog-stereo") == idA);

	// nameForId is the inverse.
	assert(registry.nameForId(idA) == "alsa_output.pci-0000_00_1f.3.analog-stereo");
	assert(registry.nameForId(999) == std::string());

	// After a refresh where a device disappears, its ID is released...
	registry.beginRefresh();
	registry.idForName("alsa_output.usb-Blue_Microphones.analog-stereo"); // idB still present
	registry.endRefresh();
	assert(registry.nameForId(idA) == std::string()); // idA's device vanished

	// ...and a later re-appearance of the same name gets a *new* ID, never idA again
	// (matches real AudioObjectID semantics: IDs are not guaranteed stable forever).
	uint32_t idA2 = registry.idForName("alsa_output.pci-0000_00_1f.3.analog-stereo");
	assert(idA2 != idA);
	assert(idA2 == 1002);

	std::vector<uint32_t> all = registry.allIds();
	assert(all.size() == 2); // idB and idA2

	printf("PADeviceRegistryTest: OK\n");
	return 0;
}
