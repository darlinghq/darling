// This function is called from _gai_addr_sort()
// On macOS, it resides in libsystem_network.dylib, which is not open source.

int sa_dst_compare()
{
	return 0;
}
