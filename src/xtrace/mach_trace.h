#ifndef XTRACE_MACH_TRACE
#define XTRACE_MACH_TRACE

#include "base.h"

XTRACE_DECLARATIONS_C_BEGIN

const char* xtrace_msg_type_to_str(mach_msg_type_name_t type_name, int full);
void xtrace_print_kern_return(kern_return_t kr);

XTRACE_DECLARATIONS_C_END

#endif // XTRACE_MACH_TRACE
