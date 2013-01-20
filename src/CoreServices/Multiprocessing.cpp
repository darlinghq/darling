#include "Multiprocessing.h"
#include <unistd.h>
#include <ctime>
#include "MacErrors.h"

Boolean _MPIsFullyInitialized()
{
	return true;
}

OSStatus MPDelayUntil(AbsoluteTime* time)
{
	struct timespec ts = { time_t(*time / 1000000000ll), long(*time % 1000000000ll) };
	nanosleep(&ts, nullptr);
	return noErr;
}

unsigned long MPProcessors()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

