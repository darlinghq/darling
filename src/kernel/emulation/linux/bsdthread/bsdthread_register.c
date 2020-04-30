#include "bsdthread_register.h"
#include "../base.h"
#include "../errno.h"
#include "../../../../../platform-include/sys/errno.h"
#include "../signal/sigexc.h"
#include <linux-syscalls/linux.h>

int pthread_obj_size;
bsdthread_entry_t pthread_entry_point;
bsdwqthread_entry_t wqueue_entry_point;

#define PTHREAD_FEATURE_DISPATCHFUNC 1
#define PTHREAD_FEATURE_QOS_MAINTENANCE 0x10
#define PTHREAD_FEATURE_QOS_DEFAULT 0x40000000
#define WORKQ_FEATURE_FINEPRIO 0x02
#define WORKQ_FEATURE_KEVENT 0x40

long sys_bsdthread_register(void* thread_start, void* wqthread, int pthsize,
		void* dummy, void* targetconc, unsigned long long dpq_offset)
{
	pthread_obj_size = pthsize;
	pthread_entry_point = (bsdthread_entry_t) thread_start;
	wqueue_entry_point = (bsdwqthread_entry_t) wqthread;
	return /* WORKQ_FEATURE_KEVENT | WORKQ_FEATURE_FINEPRIO | PTHREAD_FEATURE_QOS_MAINTENANCE 
		| PTHREAD_FEATURE_DISPATCHFUNC | PTHREAD_FEATURE_QOS_DEFAULT */ 0;
}

void pthread_entry_point_wrapper(void* self, int thread_port, void* funptr,
		void* funarg, unsigned long stacksize, unsigned int flags)
{
	sigexc_thread_setup();
	pthread_entry_point(self, thread_port, funptr, funarg, stacksize, flags);
}

void wqueue_entry_point_wrapper(void* self, int thread_port, void* stackaddr,
		void* item, int reuse, int nevents)
{
	sigexc_thread_setup();
	wqueue_entry_point(self, thread_port, stackaddr, item, reuse, nevents);
}
