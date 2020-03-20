#include "audit_session_self.h"
#include <errno.h>
#include <stdio.h>

mach_port_name_t sys_audit_session_self(void)
{
	printf("audit_session_self STUB\n");
	return ENOTSUP;
}
