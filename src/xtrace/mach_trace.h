#ifndef XTRACE_MACH_TRACE
#define XTRACE_MACH_TRACE

#include "base.h"

#ifdef XTRACE_CPP
void xtrace_print_kern_return(xtrace::String* log, kern_return_t kr);
#endif

XTRACE_DECLARATIONS_C_BEGIN
const char* xtrace_msg_type_to_str(mach_msg_type_name_t type_name, int full);
XTRACE_DECLARATIONS_C_END

#endif // XTRACE_MACH_TRACE
