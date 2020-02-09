#include <mach/message.h>

#ifdef __cplusplus
extern "C" {
#endif

void xtrace_setup_mig_tracing(void);
void xtrace_print_mig_message(const mach_msg_header_t* message, mach_port_name_t request_port);

#ifdef __cplusplus
}
#endif
