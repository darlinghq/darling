/*
 * Darling Mach Linux Kernel Module
 * Copyright (C) 2017 Lubos Dolezel
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

#include <mach/mk_timer.h>
#include "../mach_includes.h"
#include "timer.h"
#include "../ipc_server.h"
#include "../ipc_port.h"
#include "../ipc_right.h"
#include "../debug.h"
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

struct mach_timer
{
	darling_mach_port_t* port;

	bool armed;
	uint64_t expire_time, arm_time;

	struct timer_list timer;

	struct work_struct work;
};

static struct workqueue_struct* _mach_timer_wq;

static void
mach_timer_destroy(server_port_t* port);

static void
mach_timer_expired(unsigned long data);

static void
mach_timer_expired_worker(struct work_struct* work);

void
mach_timer_setup(darling_mach_port_t* port)
{
	struct mach_timer* mt;
	
	port->is_server_port = true;
	port->server_port.port_type = PORT_TYPE_TIMER;
	port->server_port.subsystem = NULL;
	port->server_port.cb_free = mach_timer_destroy;
	
	mt = (struct mach_timer*) kmalloc(sizeof(struct mach_timer),
			GFP_KERNEL);
	port->server_port.private_data = mt;
	
	mt->port = port;
	mt->armed = false;

	setup_timer(&mt->timer, mach_timer_expired, (unsigned long) mt);
	INIT_WORK(&mt->work, mach_timer_expired_worker);
}

// Arm the timer. Change the expire time if it is already armed.
kern_return_t
mach_timer_arm(darling_mach_port_t* port, uint64_t expire_time)
{
	struct mach_timer* mt = (struct mach_timer*) port->server_port.private_data;
	struct timespec now;
	int64_t diffns;

	ktime_get_ts(&now);

	mt->arm_time = now.tv_nsec + now.tv_sec * NSEC_PER_SEC;
	mt->expire_time = expire_time;
	mt->armed = true;

	diffns = expire_time - mt->arm_time;

	mod_timer(&mt->timer, jiffies + nsecs_to_jiffies(diffns));

	return KERN_SUCCESS;
}

// Disarm the timer. Do nothing if it isn't armed.
kern_return_t
mach_timer_cancel(darling_mach_port_t* port, uint64_t* expire_time)
{
	struct mach_timer* mt = (struct mach_timer*) port->server_port.private_data;

	if (mt->armed)
	{
		*expire_time = mt->expire_time;

		mt->armed = false;
		while (!del_timer(&mt->timer))
			usleep_range(1000, 20000);
	}
	else
		*expire_time = 0;

	return KERN_SUCCESS;
}

void
mach_timer_destroy(server_port_t* port)
{
	struct mach_timer* mt;
	
	mt = (struct mach_timer*) port->private_data;

	if (mt->armed)
	{
		mt->armed = false;
		while (!del_timer(&mt->timer))
			usleep_range(1000, 20000);

		// Ensure this timer hasn't just recently fired,
		// because we are about to be destroyed.
		flush_workqueue(_mach_timer_wq);
	}
}

// Executed in a workqueue
static void
mach_timer_expired_worker(struct work_struct* work)
{
	struct mach_timer* mt;
	mk_timer_expire_msg_t* msg;
	struct ipc_kmsg* kmsg;
	darling_mach_port_right_t* send_once;
	kern_return_t rv;

	msg = (mk_timer_expire_msg_t*) kzalloc(sizeof(mk_timer_expire_msg_t),
		GFP_KERNEL);

	mt = container_of(work, struct mach_timer, work);

#if 0 // It seems XNU no longer fills these out
	struct timespec now;

	msg->unused[0] = mt->arm_time;
	msg->unused[1] = mt->expire_time;

	ktime_get_ts(&now);

	msg->unused[2] = now.tv_nsec + now.tv_sec * NSEC_PER_SEC;
#endif

	msg->header.msgh_size = sizeof(*msg);
	msg->header.msgh_bits = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_MOVE_SEND_ONCE, 0, 0, 0);

	// Create a temporary send once right and call ipc_msg_deliver().
	ipc_port_lock(mt->port);
	send_once = ipc_right_new(mt->port, MACH_PORT_RIGHT_SEND_ONCE);

	// Allocate a kernel message wrapper
	kmsg = (struct ipc_kmsg*) kmalloc(sizeof(struct ipc_kmsg), GFP_KERNEL);

	kmsg->msg = (mach_msg_header_t*) msg;
	kmsg->target = send_once;
	kmsg->reply = NULL;
	kmsg->complex_items = NULL;

	// We specify no timeout, but messages sent to send once rights
	// are delivered asynchronously.
	rv = ipc_msg_deliver(kmsg, MACH_MSG_TIMEOUT_NONE, 0);

	kfree(kmsg);

	if (rv != KERN_SUCCESS)
	{
		debug_msg("Timer notification delivery failed: %d\n", rv);

		kfree(msg);
		// ipc_right_put(send_once);
		ipc_port_unlock(mt->port);
	}
	else
		debug_msg("Timer notification posted\n");
}

// Executed in atomic context
static void
mach_timer_expired(unsigned long data)
{
	struct mach_timer* mt = (struct mach_timer*) data;

	if (mt->armed)
	{
		mt->armed = false;
		queue_work(_mach_timer_wq, &mt->work);
	}
}

void
mach_timer_init(void)
{
	_mach_timer_wq = create_singlethread_workqueue("mach_timer");
}

void
mach_timer_exit(void)
{
	destroy_workqueue(_mach_timer_wq);
}

