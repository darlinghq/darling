/*
 * Darling Mach Linux Kernel Module
 * Copyright (C) 2015 Lubos Dolezel
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "../mig_includes_pre.h"
#include <mach/mach_types.h>
#include "../mach_includes.h"
#include <linux/sched.h>
//#include "../mig/clockServer.h"
#include "../ipc_port.h"
#include "clock.h"

kern_return_t clock_get_time
(
	clock_serv_t clock_serv,
	mach_timespec_t *cur_time
)
{
	u64 tm;
	
	tm = get_jiffies_64();
	
	cur_time->tv_sec = tm / HZ;
	cur_time->tv_nsec = (tm % HZ) * (NSEC_PER_SEC / HZ);
	
	return KERN_SUCCESS;
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
