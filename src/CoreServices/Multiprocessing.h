#ifndef MULTIPROCESSING_H
#define MULTIPROCESSING_H
#include "MacTypes.h"

extern "C"
{

Boolean _MPIsFullyInitialized();
OSStatus MPDelayUntil(AbsoluteTime* time);
unsigned long MPProcessors();

// other functions are missing...

}

#endif
