#ifndef XTRACE_MIG_TRACE
#define XTRACE_MIG_TRACE

#include <mach/message.h>
#include "base.h"

XTRACE_DECLARATIONS_C_BEGIN

void xtrace_setup_mig_tracing(void);
void xtrace_print_mig_message(const mach_msg_header_t* message, mach_port_name_t request_port);

XTRACE_DECLARATIONS_C_END

#endif // XTRACE_MIG_TRACE
