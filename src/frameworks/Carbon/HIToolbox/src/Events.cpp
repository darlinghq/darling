#include <HIToolbox/Events.h>

UInt8 LMGetKbdType(void)
{
	return 0;
}

UInt8 LMGetKbdLast(void)
{
	// There is only one logical keyboard, so the last keyboard used is
	// always of the current keyboard type.
	return LMGetKbdType();
}
