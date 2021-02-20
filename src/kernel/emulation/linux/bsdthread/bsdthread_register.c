#include "bsdthread_register.h"
#include "../base.h"
#include "../errno.h"
#include <sys/errno.h>
#include "../signal/sigexc.h"
#include "../mach/lkm.h"
#include "../../../../external/lkm/api.h"
#include <stdint.h>
#include <pthread/tsd_private.h>
#include <linux-syscalls/linux.h>

int pthread_obj_size;
bsdthread_entry_t pthread_entry_point;
bsdwqthread_entry_t wqueue_entry_point;

#define PTHREAD_FEATURE_DISPATCHFUNC 1
#define PTHREAD_FEATURE_QOS_MAINTENANCE 0x10
#define PTHREAD_FEATURE_QOS_DEFAULT 0x40000000
#define WORKQ_FEATURE_FINEPRIO 0x02
#define WORKQ_FEATURE_KEVENT 0x40

struct _pthread_registration_data {
	uint64_t version;

	uint64_t dispatch_queue_offset;
	uint64_t main_qos;
	uint32_t tsd_offset;
	uint32_t return_to_kernel_offset;
	uint32_t mach_thread_self_offset;
} __attribute__ ((packed));

static struct _pthread_registration_data g_pth_regdata;

long sys_bsdthread_register(void* thread_start, void* wqthread, int pthsize,
		const struct _pthread_registration_data* pth_regdata,
		unsigned long pth_regdata_len, unsigned long long dpq_offset)
{
	pthread_obj_size = pthsize;
	pthread_entry_point = (bsdthread_entry_t) thread_start;
	wqueue_entry_point = (bsdwqthread_entry_t) wqthread;
	g_pth_regdata = *pth_regdata;

	struct set_thread_handles_args args;
	args.pthread_handle = (unsigned long) _pthread_getspecific_direct(_PTHREAD_TSD_SLOT_PTHREAD_SELF);
	args.dispatch_qaddr = args.pthread_handle + g_pth_regdata.tsd_offset + g_pth_regdata.dispatch_queue_offset;

	lkm_call(NR_set_thread_handles, &args);

	return /* WORKQ_FEATURE_KEVENT | WORKQ_FEATURE_FINEPRIO | PTHREAD_FEATURE_QOS_MAINTENANCE 
		| PTHREAD_FEATURE_DISPATCHFUNC | PTHREAD_FEATURE_QOS_DEFAULT */ 0;
}

void pthread_entry_point_wrapper(void* self, int thread_port, void* funptr,
		void* funarg, unsigned long stack_addr, unsigned int flags)
{
	sigexc_thread_setup();

	struct set_thread_handles_args args;
	args.pthread_handle = (unsigned long) self;
	args.dispatch_qaddr = args.pthread_handle + g_pth_regdata.tsd_offset + g_pth_regdata.dispatch_queue_offset;

	lkm_call(NR_set_thread_handles, &args);
	pthread_entry_point(self, thread_port, funptr, funarg, stack_addr, flags);
}

void wqueue_entry_point_wrapper(void* self, int thread_port, void* stackaddr,
		void* item, int reuse, int nevents)
{
	sigexc_thread_setup();
	wqueue_entry_point(self, thread_port, stackaddr, item, reuse, nevents);
}
