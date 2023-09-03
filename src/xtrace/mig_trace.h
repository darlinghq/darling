#ifndef XTRACE_MIG_TRACE
#define XTRACE_MIG_TRACE

#include <mach/message.h>
#include "base.h"

#ifdef XTRACE_CPP
void xtrace_print_mig_message(xtrace::String* log, const mach_msg_header_t* message, mach_port_name_t request_port);
#endif

XTRACE_DECLARATIONS_C_BEGIN
void xtrace_setup_mig_tracing(void);
XTRACE_DECLARATIONS_C_END

#endif // XTRACE_MIG_TRACE
