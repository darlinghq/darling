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
	if (!event || (event->what != keyDown && event->what != autoKey))
		return false;
	if (!(event->modifiers & cmdKey))
		return false;
	return (event->message & charCodeMask) == (unsigned long) (test & 0xFF);
}

Boolean CheckEventQueueForUserCancel(void)
{
	// There is no Carbon event queue to inspect.
	return false;
}
