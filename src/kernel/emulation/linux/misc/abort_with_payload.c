#include "abort_with_payload.h"
#include "../signal/kill.h"
#include "../simple.h"
#include <sys/signal.h>

long sys_abort_with_payload(unsigned int reason_namespace, unsigned long long reason_code, void *payload, unsigned int payload_size, const char *reason_string, unsigned long long reason_flags)
{
	__simple_printf("abort_with_payload: reason: %s; code: %lu\n", reason_string, reason_code);
	sys_kill(0, SIGABRT, 1);
	return 0;
}
