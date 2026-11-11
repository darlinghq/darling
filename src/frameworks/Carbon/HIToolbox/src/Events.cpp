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

Boolean IsCmdChar(const EventRecord* event, short test)
{
	// QUIRK: reports false for every event, including a key-down with the Command key held and
	// a matching character. The function is deprecated and not documented to behave this way.
	// Evidence: black-box observation on macOS 27 (tier 2).
	// Test: tests/src/file_manager_create_res_file.c (IsCmdChar check)
	(void) event;
	(void) test;
	return false;
}

Boolean CheckEventQueueForUserCancel(void)
{
	// There is no Carbon event queue to inspect.
	return false;
}
