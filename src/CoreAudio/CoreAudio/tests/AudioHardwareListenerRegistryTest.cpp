// src/CoreAudio/CoreAudio/tests/AudioHardwareListenerRegistryTest.cpp
#include "../AudioHardwareListenerRegistry.h"
#include <cassert>
#include <cstdio>
#include <vector>

int main()
{
	AudioHardwareListenerRegistry registry;

	std::vector<std::pair<uint32_t, void*>> fired;
	void (*proc1)(uint32_t, void*) = [](uint32_t selector, void* ctx) {
		static_cast<std::vector<std::pair<uint32_t, void*>>*>(ctx)->push_back({selector, ctx});
	};

	int ctxA = 1, ctxB = 2;
	registry.add(/*selector=*/'vol ', (void*)proc1, &ctxA);
	registry.add(/*selector=*/'vol ', (void*)proc1, &ctxB);
	registry.add(/*selector=*/'dev#', (void*)proc1, &ctxA);

	int firedCountVol = 0, firedCountDev = 0;
	registry.fire('vol ', [&](void* /*proc*/, void* /*ctx*/) { firedCountVol++; });
	registry.fire('dev#', [&](void* /*proc*/, void* /*ctx*/) { firedCountDev++; });
	registry.fire('xxxx', [&](void* /*proc*/, void* /*ctx*/) { firedCountVol++; firedCountDev++; });

	assert(firedCountVol == 2); // two listeners on 'vol '
	assert(firedCountDev == 1); // one listener on 'dev#'

	registry.remove('vol ', (void*)proc1, &ctxA);
	firedCountVol = 0;
	registry.fire('vol ', [&](void*, void*) { firedCountVol++; });
	assert(firedCountVol == 1); // only ctxB's listener remains

	printf("AudioHardwareListenerRegistryTest: OK\n");
	return 0;
}
