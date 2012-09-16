#include "auto.h"

int Darling::flagsDarwinToNative(const MappedFlag* flags, size_t count, int darwin)
{
	int out = 0;
	for (size_t i = 0; i < count; i++)
	{
		if (darwin & flags[i].darwin)
			out |= flags[i].native;
	}
	return out;
}

int Darling::flagsNativeToDarwin(const MappedFlag* flags, size_t count, int native)
{
	int out = 0;
	for (size_t i = 0; i < count; i++)
	{
		if (native & flags[i].native)
			out |= flags[i].darwin;
	}
	return out;
}

