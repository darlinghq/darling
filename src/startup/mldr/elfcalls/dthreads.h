#ifndef _ELFLOADER_DTHREADS_H_
#define _ELFLOADER_DTHREADS_H_

// Darwin + pthreads = dthreads ;)
// also: "dwq" stands for "Darwin workqueue"

#include <stdint.h>

#define DARWIN_TAILQ_ENTRY(type)      \
	struct {                            \
		struct type* tqe_next;            \
		struct type** tqe_prev;           \
	}

typedef struct darwin_os_unfair_lock_s {
	uint32_t _os_unfair_lock_opaque;
} darwin_os_unfair_lock;

typedef darwin_os_unfair_lock dthread_lock;

typedef int darwin_errno_t;

struct __darwin_pthread_handler_rec;
struct dthread_join_context_s;

struct darwin_sched_param {
	int sched_priority;
	int quantum;
};

#define DARWIN_MAXTHREADNAMESIZE 64

// remember to change this as necessary depending on our target Darwin-based OS (e.g. if we ever add support for iOS)
#if 0 /* TARGET_OS_EMBEDDED */
#define DARWIN_EXTERNAL_POSIX_THREAD_KEYS_MAX 256
#define DARWIN_INTERNAL_POSIX_THREAD_KEYS_MAX 256
#define DARWIN_INTERNAL_POSIX_THREAD_KEYS_END 512
#else
#define DARWIN_EXTERNAL_POSIX_THREAD_KEYS_MAX 512
#define DARWIN_INTERNAL_POSIX_THREAD_KEYS_MAX 256
#define DARWIN_INTERNAL_POSIX_THREAD_KEYS_END 768
#endif

#define DTHREAD_TSD_SLOT_PTHREAD_SELF       0
#define DTHREAD_TSD_SLOT_ERRNO              1
#define DTHREAD_TSD_SLOT_MIG_REPLY          2
#define DTHREAD_TSD_SLOT_MACH_THREAD_SELF   3
#define DTHREAD_TSD_SLOT_PTHREAD_QOS_CLASS  4
#define DTHREAD_TSD_SLOT_RETURN_TO_KERNEL   5
#define DTHREAD_TSD_SLOT_PTR_MUNGE          7
#define DTHREAD_TSD_SLOT_MACH_SPECIAL_REPLY 8

#define DTHREAD_START_TSD_BASE_SET 0x10000000

#define DTHREAD_CANCEL_ENABLE   0x01
#define DTHREAD_CANCEL_DEFERRED 0x02

#define DTHREAD_INHERIT_SCHED 1

#define DARWIN_POLICY_TIMESHARE 1

#define DARWIN_THREAD_QOS_LEGACY 4

#define DTHREAD_PRIORITY_QOS_CLASS_SHIFT 8ULL

#define DTHREAD_PRIORITY_PRIORITY_MASK 0x000000ff

#define DTHREAD_DEFAULT_PRIORITY (1U << (DARWIN_THREAD_QOS_LEGACY - 1 + DTHREAD_PRIORITY_QOS_CLASS_SHIFT)) | ((uint8_t)-1 & DTHREAD_PRIORITY_PRIORITY_MASK)

#define DWQ_FLAG_THREAD_TSD_BASE_SET  0x00200000

// ***
// KEEP IN SYNC WITH THE DEFINITION OF `struct _pthread` IN libpthread's `internal.h`
// ***
//
// that also means that we have to remember to account for changes in some of the things it depends on,
// like `os_unfair_lock`, `TAILQ_ENTRY`, `sched_param`, and more
//
// (although, i doubt that the total size of the structure will ever change; there's a comment in the real
// structure definition that says that they have to keep the TSD offset stable to not break older software)
typedef struct _dthread {
	long sig;
	struct __darwin_pthread_handler_rec* __cleanup_stack;

	DARWIN_TAILQ_ENTRY(_pthread) tl_plist;
	struct dthread_join_context_s* tl_join_ctx;
	void* tl_exit_value;
	uint32_t tl_policy:8,
			tl_joinable:1,
			tl_joiner_cleans_up:1,
			tl_has_custom_stack:1,
			__tl_pad:21;
	uint32_t tl_exit_gate;
	struct darwin_sched_param tl_param;
	void* __unused_padding;

	dthread_lock lock;
	uint16_t max_tsd_key;
	uint16_t inherit:8,
			kernalloc:1,
			schedset:1,
			wqthread:1,
			wqkillset:1,
			__flags_pad:4;

	char pthread_name[DARWIN_MAXTHREADNAMESIZE];

	void* (*fun)(void*);
	void* arg;
	int wq_nevents;
	bool wq_outsideqos;
	uint8_t canceled;
	uint16_t cancel_state;
	darwin_errno_t cancel_error;
	darwin_errno_t err_no;

	void* stackaddr;
	void* stackbottom;
	void* freeaddr;
	size_t freesize;
	size_t guardsize;

	__attribute__((aligned(8)))
	uint64_t thread_id;

	__attribute__((aligned(16)))
	void* tsd[DARWIN_EXTERNAL_POSIX_THREAD_KEYS_MAX + DARWIN_INTERNAL_POSIX_THREAD_KEYS_MAX];
}* dthread_t;

#endif // _ELFLOADER_DTHREADS_H_
