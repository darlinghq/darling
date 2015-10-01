#include "../mach_includes.h"
#include "../mig_includes_pre.h"
//#include "../mig/clockServer.h"
#include "../ipc_port.h"
#include <mach/mach_types.h>
#include "clock.h"

kern_return_t clock_get_time
(
	clock_serv_t clock_serv,
	mach_timespec_t *cur_time
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t clock_get_attributes
(
	clock_serv_t clock_serv,
	clock_flavor_t flavor,
	clock_attr_t clock_attr,
	mach_msg_type_number_t *clock_attrCnt
)
{
	return KERN_NOT_SUPPORTED;
}

kern_return_t clock_alarm
(
	clock_serv_t clock_serv,
	alarm_type_t alarm_type,
	mach_timespec_t alarm_time,
	clock_reply_t alarm_port,
	mach_msg_type_name_t alarm_portPoly
)
{
	return KERN_NOT_SUPPORTED;
}
