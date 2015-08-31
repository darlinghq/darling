#ifndef MACH_INCLUDES_H
#define MACH_INCLUDES_H

#include <mach/port.h>
#include <mach/message.h>

/* Undef some names that conflict with Linux defines */
#undef __used
#undef __deprecated
#undef __pure
#undef __unused
#undef SIZE_MAX
#undef PAGE_SIZE
#undef PAGE_SHIFT
#undef PAGE_MASK

#endif
