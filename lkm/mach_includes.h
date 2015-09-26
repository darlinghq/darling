#ifndef MACH_INCLUDES_H
#define MACH_INCLUDES_H

#include <mach/port.h>
#include <mach/message.h>
#include <mach/mig.h>
#include <mach/mach_types.h>

/* Undef some names that conflict with Linux defines */
#undef __used
#undef __deprecated
#undef __pure
#undef __unused
#undef ntohs
#undef SIZE_MAX
#undef PAGE_SIZE
#undef PAGE_SHIFT
#undef PAGE_MASK
#undef NSEC_PER_USEC
#undef NSEC_PER_MSEC
#undef NSEC_PER_SEC
#undef USEC_PER_SEC

#endif
