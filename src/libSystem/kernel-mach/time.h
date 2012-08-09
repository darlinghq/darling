#ifndef MACH_TIME_H
#define MACH_TIME_H
#include <stdint.h>
#include <mach/mach_time.h>

extern "C"
{

uint64_t mach_absolute_time();
int mach_timebase_info(struct mach_timebase_info* info);

}

#endif
