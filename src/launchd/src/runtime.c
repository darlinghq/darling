/*
 * Copyright (c) 1999-2008 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#include "config.h"
#include "runtime.h"

#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/boolean.h>
#include <mach/message.h>
#include <mach/notify.h>
#include <mach/mig_errors.h>
#include <mach/mach_traps.h>
#include <mach/mach_interface.h>
#include <mach/host_info.h>
#include <mach/mach_host.h>
#include <mach/mach_time.h>
#include <mach/exception.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/proc_info.h>
#include <libproc.h>
#include <sys/event.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/fcntl.h>
#include <sys/kdebug.h>
#include <bsm/libbsm.h>
#include <malloc/malloc.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <syslog.h>
#include <signal.h>
#include <dlfcn.h>
#include <os/assumes.h>

#include "internalServer.h"
#include "internal.h"
#include "notifyServer.h"
#include "mach_excServer.h"

/* We shouldn't be including these */
#include "launch.h"
#include "launchd.h"
#include "core.h"
#include "vproc.h"
#include "vproc_priv.h"
#include "vproc_internal.h"
#include "jobServer.h"
#include "job_reply.h"

#include <xpc/launchd.h>

static mach_port_t ipc_port_set;
static mach_port_t demand_port_set;
static mach_port_t launchd_internal_port;
static int mainkq;

#define BULK_KEV_MAX 100
static struct kevent *bulk_kev;
static int bulk_kev_i;
static int bulk_kev_cnt;

static pthread_t kqueue_demand_thread;

static void mportset_callback(void);
static kq_callback kqmportset_callback = (kq_callback)mportset_callback;
static void *kqueue_demand_loop(void *arg);

boolean_t launchd_internal_demux(mach_msg_header_t *Request, mach_msg_header_t *Reply);
static void launchd_runtime2(mach_msg_size_t msg_size);
static mach_msg_size_t max_msg_size;
static mig_callback *mig_cb_table;
static size_t mig_cb_table_sz;
static timeout_callback runtime_idle_callback;
static mach_msg_timeout_t runtime_idle_timeout;
static struct ldcred ldc;
static audit_token_t ldc_token;
static size_t runtime_standby_cnt;

static void do_file_init(void) __attribute__((constructor));
static mach_timebase_info_data_t tbi;
static uint64_t tbi_safe_math_max;
static uint64_t time_of_mach_msg_return;
static double tbi_float_val;

static const int sigigns[] = { SIGHUP, SIGINT, SIGPIPE, SIGALRM, SIGTERM,
	SIGURG, SIGTSTP, SIGTSTP, SIGCONT, SIGTTIN, SIGTTOU, SIGIO, SIGXCPU,
	SIGXFSZ, SIGVTALRM, SIGPROF, SIGWINCH, SIGINFO, SIGUSR1, SIGUSR2
};
static sigset_t sigign_set;
bool pid1_magic;
bool launchd_apple_internal;
bool launchd_flat_mach_namespace = true;
bool launchd_malloc_log_stacks = false;
bool launchd_use_gmalloc = false;
bool launchd_log_per_user_shutdown = false;
#if !TARGET_OS_EMBEDDED
bool launchd_log_shutdown = true;
#else
bool launchd_log_shutdown = false;
#endif
bool launchd_log_perf = false;
bool launchd_log_debug = false;
bool launchd_trap_sigkill_bugs = false;
bool launchd_no_jetsam_perm_check = false;
bool launchd_osinstaller = false;
bool launchd_allow_global_dyld_envvars = false;
#if TARGET_OS_EMBEDDED
bool launchd_appletv = false;
#endif
pid_t launchd_wsp = 0;
size_t runtime_busy_cnt;

#if TARGET_OS_EMBEDDED
#define LAUNCHD_CONFIG_PREFIX "/"
#else
#define LAUNCHD_CONFIG_PREFIX "/private/var/db/"
#endif

#define config_check(s, sb) (stat(LAUNCHD_CONFIG_PREFIX s, &sb) == 0)

mach_port_t
runtime_get_kernel_port(void)
{
	return launchd_internal_port;
}

union vproc_mig_max_sz {
	union __RequestUnion__job_mig_job_subsystem req;
	union __ReplyUnion__job_mig_job_subsystem rep;
};

union internal_max_sz {
	union __RequestUnion__x_internal_subsystem req;
	union __ReplyUnion__internal_subsystem rep;
};

union xpc_domain_max_sz {
	union __RequestUnion__xpc_domain_xpc_domain_subsystem req;
	union __ReplyUnion__xpc_domain_xpc_domain_subsystem rep;
};

union mach_exc_max_sz {
	union __RequestUnion__catch_mach_exc_subsystem req;
	union __ReplyUnion__catch_mach_exc_subsystem rep;
};

union do_notify_max_sz {
	union __RequestUnion__do_notify_subsystem req;
	union __ReplyUnion__do_notify_subsystem rep;
};

void
launchd_runtime_init(void)
{
	pid_t p = getpid();

	(void)posix_assert_zero((mainkq = kqueue()));

	os_assert_zero(mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET, &demand_port_set));
	os_assert_zero(mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET, &ipc_port_set));
	posix_assert_zero(kevent_mod(demand_port_set, EVFILT_MACHPORT, EV_ADD, 0, 0, &kqmportset_callback));

	os_assert_zero(launchd_mport_create_recv(&launchd_internal_port));
	os_assert_zero(launchd_mport_make_send(launchd_internal_port));

	max_msg_size = sizeof(union vproc_mig_max_sz);
	if (sizeof(union xpc_domain_max_sz) > max_msg_size) {
		max_msg_size = sizeof(union xpc_domain_max_sz);
	}

	os_assert_zero(runtime_add_mport(launchd_internal_port, launchd_internal_demux));
	os_assert_zero(pthread_create(&kqueue_demand_thread, NULL, kqueue_demand_loop, NULL));
	os_assert_zero(pthread_detach(kqueue_demand_thread));

	(void)posix_assumes_zero(sysctlbyname("vfs.generic.noremotehang", NULL, NULL, &p, sizeof(p)));
}

void
launchd_runtime_init2(void)
{
	size_t i;

	__OS_COMPILETIME_ASSERT__(SIG_ERR == (typeof(SIG_ERR))-1);
	for (i = 0; i < (sizeof(sigigns) / sizeof(int)); i++) {
		sigaddset(&sigign_set, sigigns[i]);
		(void)posix_assumes_zero(signal(sigigns[i], SIG_IGN));
	}
}

#define FLAGIF(f) if (flags & f) { flags_off += sprintf(flags_off, #f); flags &= ~f; }
const char *
reboot_flags_to_C_names(unsigned int flags)
{
#define MAX_RB_STR "RB_ASKNAME|RB_SINGLE|RB_NOSYNC|RB_HALT|RB_INITNAME|RB_DFLTROOT|RB_ALTBOOT|RB_UNIPROC|RB_SAFEBOOT|RB_UPSDELAY|0xdeadbeeffeedface"
	static char flags_buf[sizeof(MAX_RB_STR)];
	char *flags_off = NULL;

	if (flags == 0) {
		return "RB_AUTOBOOT";
	}

	while (flags) {
		if (flags_off) {
			*flags_off = '|';
			flags_off++;
			*flags_off = '\0';
		} else {
			flags_off = flags_buf;
		}

		FLAGIF(RB_ASKNAME)
		else FLAGIF(RB_SINGLE)
		else FLAGIF(RB_NOSYNC)
		else FLAGIF(RB_HALT)
		else FLAGIF(RB_INITNAME)
		else FLAGIF(RB_DFLTROOT)
		else FLAGIF(RB_ALTBOOT)
		else FLAGIF(RB_UNIPROC)
		else FLAGIF(RB_SAFEBOOT)
		else FLAGIF(RB_UPSDELAY)
		else {
			flags_off += sprintf(flags_off, "0x%x", flags);
			flags = 0;
		}
	}

	return flags_buf;
}

const char *
signal_to_C_name(unsigned int sig)
{
	static char unknown[25];

#define SIG2CASE(sg)	case sg: return #sg

	switch (sig) {
	SIG2CASE(SIGHUP);
	SIG2CASE(SIGINT);
	SIG2CASE(SIGQUIT);
	SIG2CASE(SIGILL);
	SIG2CASE(SIGTRAP);
	SIG2CASE(SIGABRT);
	SIG2CASE(SIGFPE);
	SIG2CASE(SIGKILL);
	SIG2CASE(SIGBUS);
	SIG2CASE(SIGSEGV);
	SIG2CASE(SIGSYS);
	SIG2CASE(SIGPIPE);
	SIG2CASE(SIGALRM);
	SIG2CASE(SIGTERM);
	SIG2CASE(SIGURG);
	SIG2CASE(SIGSTOP);
	SIG2CASE(SIGTSTP);
	SIG2CASE(SIGCONT);
	SIG2CASE(SIGCHLD);
	SIG2CASE(SIGTTIN);
	SIG2CASE(SIGTTOU);
	SIG2CASE(SIGIO);
	SIG2CASE(SIGXCPU);
	SIG2CASE(SIGXFSZ);
	SIG2CASE(SIGVTALRM);
	SIG2CASE(SIGPROF);
	SIG2CASE(SIGWINCH);
	SIG2CASE(SIGINFO);
	SIG2CASE(SIGUSR1);
	SIG2CASE(SIGUSR2);
	default:
		snprintf(unknown, sizeof(unknown), "%u", sig);
		return unknown;
	}
}

void
log_kevent_struct(int level, struct kevent *kev_base, int indx)
{
	struct kevent *kev = &kev_base[indx];
	const char *filter_str;
	char ident_buf[100];
	char filter_buf[100];
	char fflags_buf[1000];
	char flags_buf[1000] = "0x0";
	char *flags_off = NULL;
	char *fflags_off = NULL;
	unsigned short flags = kev->flags;
	unsigned int fflags = kev->fflags;

	if (likely(!(LOG_MASK(level & ~LOG_CONSOLE) & LOG_DEBUG))) {
		return;
	}

	if (flags) while (flags) {
		if (flags_off) {
			*flags_off = '|';
			flags_off++;
			*flags_off = '\0';
		} else {
			flags_off = flags_buf;
		}

		FLAGIF(EV_ADD)
		else FLAGIF(EV_RECEIPT)
		else FLAGIF(EV_DELETE)
		else FLAGIF(EV_ENABLE)
		else FLAGIF(EV_DISABLE)
		else FLAGIF(EV_CLEAR)
		else FLAGIF(EV_EOF)
		else FLAGIF(EV_ONESHOT)
		else FLAGIF(EV_ERROR)
		else {
			flags_off += sprintf(flags_off, "0x%hx", flags);
			flags = 0;
		}
	}

	snprintf(ident_buf, sizeof(ident_buf), "%ld", kev->ident);
	snprintf(fflags_buf, sizeof(fflags_buf), "0x%x", fflags);

	switch (kev->filter) {
	case EVFILT_READ:
		filter_str = "EVFILT_READ";
		break;
	case EVFILT_WRITE:
		filter_str = "EVFILT_WRITE";
		break;
	case EVFILT_AIO:
		filter_str = "EVFILT_AIO";
		break;
	case EVFILT_VNODE:
		filter_str = "EVFILT_VNODE";
		if (fflags) while (fflags) {
			if (fflags_off) {
				*fflags_off = '|';
				fflags_off++;
				*fflags_off = '\0';
			} else {
				fflags_off = fflags_buf;
			}

#define FFLAGIF(ff) if (fflags & ff) { fflags_off += sprintf(fflags_off, #ff); fflags &= ~ff; }

			FFLAGIF(NOTE_DELETE)
			else FFLAGIF(NOTE_WRITE)
			else FFLAGIF(NOTE_EXTEND)
			else FFLAGIF(NOTE_ATTRIB)
			else FFLAGIF(NOTE_LINK)
			else FFLAGIF(NOTE_RENAME)
			else FFLAGIF(NOTE_REVOKE)
			else {
				fflags_off += sprintf(fflags_off, "0x%x", fflags);
				fflags = 0;
			}
		}
		break;
	case EVFILT_PROC:
		filter_str = "EVFILT_PROC";
		if (fflags) while (fflags) {
			if (fflags_off) {
				*fflags_off = '|';
				fflags_off++;
				*fflags_off = '\0';
			} else {
				fflags_off = fflags_buf;
			}

			FFLAGIF(NOTE_EXIT)
			else FFLAGIF(NOTE_REAP)
			else FFLAGIF(NOTE_FORK)
			else FFLAGIF(NOTE_EXEC)
			else FFLAGIF(NOTE_SIGNAL)
			else FFLAGIF(NOTE_TRACK)
			else FFLAGIF(NOTE_TRACKERR)
			else FFLAGIF(NOTE_CHILD)
			else {
				fflags_off += sprintf(fflags_off, "0x%x", fflags);
				fflags = 0;
			}
		}
		break;
	case EVFILT_SIGNAL:
		filter_str = "EVFILT_SIGNAL";
		strcpy(ident_buf, signal_to_C_name(kev->ident));
		break;
	case EVFILT_TIMER:
		filter_str = "EVFILT_TIMER";
		snprintf(ident_buf, sizeof(ident_buf), "0x%lx", kev->ident);
		if (fflags) while (fflags) {
			if (fflags_off) {
				*fflags_off = '|';
				fflags_off++;
				*fflags_off = '\0';
			} else {
				fflags_off = fflags_buf;
			}

			FFLAGIF(NOTE_SECONDS)
			else FFLAGIF(NOTE_USECONDS)
			else FFLAGIF(NOTE_NSECONDS)
			else FFLAGIF(NOTE_ABSOLUTE)
			else {
				fflags_off += sprintf(fflags_off, "0x%x", fflags);
				fflags = 0;
			}
		}
		break;
	case EVFILT_MACHPORT:
		filter_str = "EVFILT_MACHPORT";
		snprintf(ident_buf, sizeof(ident_buf), "0x%lx", kev->ident);
		break;
	case EVFILT_FS:
		filter_str = "EVFILT_FS";
		snprintf(ident_buf, sizeof(ident_buf), "0x%lx", kev->ident);
		if (fflags) while (fflags) {
			if (fflags_off) {
				*fflags_off = '|';
				fflags_off++;
				*fflags_off = '\0';
			} else {
				fflags_off = fflags_buf;
			}

			FFLAGIF(VQ_NOTRESP)
			else FFLAGIF(VQ_NEEDAUTH)
			else FFLAGIF(VQ_LOWDISK)
			else FFLAGIF(VQ_MOUNT)
			else FFLAGIF(VQ_UNMOUNT)
			else FFLAGIF(VQ_DEAD)
			else FFLAGIF(VQ_ASSIST)
			else FFLAGIF(VQ_NOTRESPLOCK)
			else FFLAGIF(VQ_UPDATE)
			else {
				fflags_off += sprintf(fflags_off, "0x%x", fflags);
				fflags = 0;
			}
		}
		break;
	default:
		snprintf(filter_buf, sizeof(filter_buf), "%hd", kev->filter);
		filter_str = filter_buf;
		break;
	}

	launchd_syslog(level, "KEVENT[%d]: udata = %p data = 0x%lx ident = %s filter = %s flags = %s fflags = %s",
			indx, kev->udata, kev->data, ident_buf, filter_str, flags_buf, fflags_buf);
}

void
mportset_callback(void)
{
	mach_port_name_array_t members;
	mach_msg_type_number_t membersCnt;
	mach_port_status_t status;
	mach_msg_type_number_t statusCnt;
	struct kevent kev;
	unsigned int i;

	if (os_assumes_zero(mach_port_get_set_status(mach_task_self(), demand_port_set, &members, &membersCnt)) != 0) {
		return;
	}

	for (i = 0; i < membersCnt; i++) {
		statusCnt = MACH_PORT_RECEIVE_STATUS_COUNT;
		if (mach_port_get_attributes(mach_task_self(), members[i], MACH_PORT_RECEIVE_STATUS, (mach_port_info_t)&status,
					&statusCnt) != KERN_SUCCESS) {
			continue;
		}
		if (status.mps_msgcount) {
			EV_SET(&kev, members[i], EVFILT_MACHPORT, 0, 0, 0, job_find_by_service_port(members[i]));
#if 0
			if (kev.udata != NULL) {
#endif
				log_kevent_struct(LOG_DEBUG, &kev, 0);
				(*((kq_callback *)kev.udata))(kev.udata, &kev);
#if 0
			} else {
				log_kevent_struct(LOG_ERR, &kev, 0);
			}
#endif
			/* the callback may have tainted our ability to continue this for loop */
			break;
		}
	}

	(void)os_assumes_zero(vm_deallocate(mach_task_self(), (vm_address_t)members, (vm_size_t) membersCnt * sizeof(mach_port_name_t)));
}

void *
kqueue_demand_loop(void *arg __attribute__((unused)))
{
	fd_set rfds;

	/*
	 * Yes, at first glance, calling select() on a kqueue seems silly.
	 *
	 * This avoids a race condition between the main thread and this helper
	 * thread by ensuring that we drain kqueue events on the same thread
	 * that manipulates the kqueue.
	 */

	for (;;) {
		FD_ZERO(&rfds);
		FD_SET(mainkq, &rfds);
		int r = select(mainkq + 1, &rfds, NULL, NULL, NULL);
		if (r == 1) {
			(void)os_assumes_zero(handle_kqueue(launchd_internal_port, mainkq));
		} else if (posix_assumes_zero(r) != -1) {
			(void)os_assumes_zero(r);
		}
	}

	return NULL;
}

kern_return_t
x_handle_kqueue(mach_port_t junk __attribute__((unused)), integer_t fd)
{
	struct timespec ts = { 0, 0 };
	struct kevent *kevi, kev[BULK_KEV_MAX];
	int i;

	bulk_kev = kev;

	if ((bulk_kev_cnt = kevent(fd, NULL, 0, kev, BULK_KEV_MAX, &ts)) != -1) {
#if 0	
		for (i = 0; i < bulk_kev_cnt; i++) {
			log_kevent_struct(LOG_DEBUG, &kev[0], i);
		}
#endif
		for (i = 0; i < bulk_kev_cnt; i++) {
			bulk_kev_i = i;
			kevi = &kev[i];

			if (kevi->filter) {
				launchd_syslog(LOG_DEBUG, "Dispatching kevent (ident/filter): %lu/%hd", kevi->ident, kevi->filter);
				log_kevent_struct(LOG_DEBUG, kev, i);

				struct job_check_s {
					kq_callback kqc;
				};

				struct job_check_s *check = kevi->udata;
				if (check && check->kqc) {
					runtime_ktrace(RTKT_LAUNCHD_BSD_KEVENT|DBG_FUNC_START, kevi->ident, kevi->filter, kevi->fflags);
					(*((kq_callback *)kevi->udata))(kevi->udata, kevi);
					runtime_ktrace0(RTKT_LAUNCHD_BSD_KEVENT|DBG_FUNC_END);
				} else {
					launchd_syslog(LOG_ERR, "The following kevent had invalid context data. Please file a bug with the following information:");
					log_kevent_struct(LOG_EMERG, &kev[0], i);
				}
				launchd_syslog(LOG_DEBUG, "Handled kevent.");
			}
		}
	} else {
		(void)os_assumes_zero(errno);
	}

	bulk_kev = NULL;

	return 0;
}

void
launchd_runtime(void)
{
	launchd_runtime2(max_msg_size);
	dispatch_main();
}

kern_return_t
launchd_set_bport(mach_port_t name)
{
	return errno = task_set_bootstrap_port(mach_task_self(), name);
}

kern_return_t
launchd_get_bport(mach_port_t *name)
{
	return errno = task_get_bootstrap_port(mach_task_self(), name);
}

kern_return_t
launchd_mport_notify_req(mach_port_t name, mach_msg_id_t which)
{
	mach_port_mscount_t msgc = (which == MACH_NOTIFY_PORT_DESTROYED) ? 0 : 1;
	mach_port_t previous, where = (which == MACH_NOTIFY_NO_SENDERS) ? name : launchd_internal_port;

	if (which == MACH_NOTIFY_NO_SENDERS) {
		/* Always make sure the send count is zero, in case a receive right is
		 * reused
		 */
		errno = mach_port_set_mscount(mach_task_self(), name, 0);
		if (unlikely(errno != KERN_SUCCESS)) {
			return errno;
		}
	}

	errno = mach_port_request_notification(mach_task_self(), name, which, msgc, where, MACH_MSG_TYPE_MAKE_SEND_ONCE, &previous);

	if (likely(errno == 0) && previous != MACH_PORT_NULL) {
		(void)os_assumes_zero(launchd_mport_deallocate(previous));
	}

	return errno;
}

pid_t
runtime_fork(mach_port_t bsport)
{
	sigset_t emptyset, oset;
	pid_t r = -1;
	int saved_errno;
	size_t i;

	sigemptyset(&emptyset);

	(void)os_assumes_zero(launchd_mport_make_send(bsport));
	(void)os_assumes_zero(launchd_set_bport(bsport));
	(void)os_assumes_zero(launchd_mport_deallocate(bsport));

	__OS_COMPILETIME_ASSERT__(SIG_ERR == (typeof(SIG_ERR))-1);
	(void)posix_assumes_zero(sigprocmask(SIG_BLOCK, &sigign_set, &oset));
	for (i = 0; i < (sizeof(sigigns) / sizeof(int)); i++) {
		(void)posix_assumes_zero(signal(sigigns[i], SIG_DFL));
	}

	r = fork();
	saved_errno = errno;

	if (r != 0) {
		for (i = 0; i < (sizeof(sigigns) / sizeof(int)); i++) {
			(void)posix_assumes_zero(signal(sigigns[i], SIG_IGN));
		}
		(void)posix_assumes_zero(sigprocmask(SIG_SETMASK, &oset, NULL));
		(void)os_assumes_zero(launchd_set_bport(MACH_PORT_NULL));
	} else {
		pid_t p = -getpid();
		(void)posix_assumes_zero(sysctlbyname("vfs.generic.noremotehang", NULL, NULL, &p, sizeof(p)));
		(void)posix_assumes_zero(sigprocmask(SIG_SETMASK, &emptyset, NULL));
	}

	errno = saved_errno;

	return r;
}


void
runtime_set_timeout(timeout_callback to_cb, unsigned int sec)
{
	if (sec == 0 || to_cb == NULL) {
		runtime_idle_callback = NULL;
		runtime_idle_timeout = 0;
	}

	runtime_idle_callback = to_cb;
	runtime_idle_timeout = sec * 1000;
}

kern_return_t
runtime_add_mport(mach_port_t name, mig_callback demux)
{
	size_t needed_table_sz = (MACH_PORT_INDEX(name) + 1) * sizeof(mig_callback);
	mach_port_t target_set = demux ? ipc_port_set : demand_port_set;

	if (unlikely(needed_table_sz > mig_cb_table_sz)) {
		needed_table_sz *= 2; /* Let's try and avoid realloc'ing for a while */
		mig_callback *new_table = malloc(needed_table_sz);

		if (!new_table) {
			return KERN_RESOURCE_SHORTAGE;
		}

		if (likely(mig_cb_table)) {
			memcpy(new_table, mig_cb_table, mig_cb_table_sz);
			free(mig_cb_table);
		}

		mig_cb_table_sz = needed_table_sz;
		mig_cb_table = new_table;
	}

	mig_cb_table[MACH_PORT_INDEX(name)] = demux;

	return errno = mach_port_move_member(mach_task_self(), name, target_set);
}

kern_return_t
runtime_remove_mport(mach_port_t name)
{
	mig_cb_table[MACH_PORT_INDEX(name)] = NULL;

	return errno = mach_port_move_member(mach_task_self(), name, MACH_PORT_NULL);
}

kern_return_t
launchd_mport_make_send(mach_port_t name)
{
	return errno = mach_port_insert_right(mach_task_self(), name, name, MACH_MSG_TYPE_MAKE_SEND);
}

kern_return_t
launchd_mport_copy_send(mach_port_t name)
{
	return errno = mach_port_insert_right(mach_task_self(), name, name, MACH_MSG_TYPE_COPY_SEND);
}

kern_return_t
launchd_mport_make_send_once(mach_port_t name, mach_port_t *so)
{
	mach_msg_type_name_t right = 0;
	return errno = mach_port_extract_right(mach_task_self(), name, MACH_MSG_TYPE_MAKE_SEND_ONCE, so, &right);
}

kern_return_t
launchd_mport_close_recv(mach_port_t name)
{
	return errno = mach_port_mod_refs(mach_task_self(), name, MACH_PORT_RIGHT_RECEIVE, -1);
}

kern_return_t
launchd_mport_create_recv(mach_port_t *name)
{
	return errno = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, name);
}

kern_return_t
launchd_mport_deallocate(mach_port_t name)
{
	return errno = mach_port_deallocate(mach_task_self(), name);
}

int
kevent_bulk_mod(struct kevent *kev, size_t kev_cnt)
{
	size_t i;

	for (i = 0; i < kev_cnt; i++) {
		kev[i].flags |= EV_CLEAR|EV_RECEIPT;
	}

	return kevent(mainkq, kev, kev_cnt, kev, kev_cnt, NULL);
}

int
kevent_mod(uintptr_t ident, short filter, u_short flags, u_int fflags, intptr_t data, void *udata)
{
	struct kevent kev;
	int r;

	switch (filter) {
	case EVFILT_READ:
	case EVFILT_WRITE:
		break;
	case EVFILT_TIMER:
		/* Workaround 5225889 */
		if (flags & EV_ADD) {
			(void)kevent_mod(ident, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
		}
		/* fall through */
	default:
		flags |= EV_CLEAR;
		break;
	}

	flags |= EV_RECEIPT;

	if (flags & EV_ADD && !udata) {
		errno = EINVAL;
		return -1;
	} else if ((flags & EV_DELETE) && bulk_kev) {
		int i = 0;
		for (i = bulk_kev_i + 1; i < bulk_kev_cnt; i++) {
			if (bulk_kev[i].filter == filter && bulk_kev[i].ident == ident) {
				launchd_syslog(LOG_DEBUG, "Pruning the following kevent:");
				log_kevent_struct(LOG_DEBUG, &bulk_kev[0], i);
				bulk_kev[i].filter = (short)0;
			}
		}
	}

	EV_SET(&kev, ident, filter, flags, fflags, data, udata);

	r = kevent(mainkq, &kev, 1, &kev, 1, NULL);

	if (r != 1) {
		return -1;
	}

	if (kev.flags & EV_ERROR) {
		if ((flags & EV_ADD) && kev.data) {
			launchd_syslog(LOG_DEBUG, "%s(): See the next line...", __func__);
			log_kevent_struct(LOG_DEBUG, &kev, 0);
			errno = kev.data;
			return -1;
		}
	} else {
		(void)os_assert_zero(kev.flags);
	}

	return r;
}

boolean_t
launchd_internal_demux(mach_msg_header_t *Request, mach_msg_header_t *Reply)
{
	if (internal_server_routine(Request)) {
		return internal_server(Request, Reply);
	} else if (notify_server_routine(Request)) {
		return notify_server(Request, Reply);
	} else {
		return mach_exc_server(Request, Reply);
	}
}

kern_return_t
do_mach_notify_port_destroyed(mach_port_t notify __attribute__((unused)), mach_port_t rights)
{
	/* This message is sent to us when a receive right is returned to us. */
	if (!job_ack_port_destruction(rights)) {
		(void)os_assumes_zero(launchd_mport_close_recv(rights));
	}

	return KERN_SUCCESS;
}

kern_return_t
do_mach_notify_port_deleted(mach_port_t notify __attribute__((unused)), mach_port_name_t name __attribute__((unused)))
{
	/* If we deallocate/destroy/mod_ref away a port with a pending
	 * notification, the original notification message is replaced with
	 * this message. To quote a Mach kernel expert, "the kernel has a
	 * send-once right that has to be used somehow."
	 */
	return KERN_SUCCESS;
}

kern_return_t
do_mach_notify_no_senders(mach_port_t notify, mach_port_mscount_t mscount __attribute__((unused)))
{
	job_t j = job_mig_intran(notify);

	/* This message is sent to us when the last customer of one of our objects
	 * goes away.
	 */

	if (!j) {
		return KERN_FAILURE;
	}

	job_ack_no_senders(j);

	return KERN_SUCCESS;
}

kern_return_t
do_mach_notify_send_once(mach_port_t notify __attribute__((unused)))
{
	/*
	 * This message is sent for each send-once right that is deallocated without
	 * being used.
	 */

	return KERN_SUCCESS;
}

kern_return_t
do_mach_notify_dead_name(mach_port_t notify __attribute__((unused)), mach_port_name_t name)
{
	/* This message is sent to us when one of our send rights no longer has a
	 * receiver somewhere else on the system.
	 */
	if (name == launchd_drain_reply_port) {
		(void)os_assumes_zero(launchd_mport_deallocate(name));
		launchd_drain_reply_port = MACH_PORT_NULL;
	}

	if (root_jobmgr) {
		root_jobmgr = jobmgr_delete_anything_with_port(root_jobmgr, name);
	}

	/* A dead-name notification about a port appears to increment the rights on
	 * said port. Let's deallocate it so that we don't leak dead-name ports.
	 */
	(void)os_assumes_zero(launchd_mport_deallocate(name));

	return KERN_SUCCESS;
}

mach_msg_return_t
launchd_exc_runtime_once(mach_port_t port, mach_msg_size_t rcv_msg_size, mach_msg_size_t send_msg_size, mig_reply_error_t *bufRequest, mig_reply_error_t *bufReply, mach_msg_timeout_t to)
{
	mach_msg_return_t mr = ~MACH_MSG_SUCCESS;
	mach_msg_option_t rcv_options =	MACH_RCV_MSG
		| MACH_RCV_TIMEOUT
		| MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_AUDIT)
		| MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0);

	do {
		mr = mach_msg(&bufRequest->Head, rcv_options, 0, rcv_msg_size, port, to, MACH_PORT_NULL);
		switch (mr) {
		case MACH_RCV_TIMED_OUT:
			launchd_syslog(LOG_DEBUG, "Message queue is empty.");
			break;
		case MACH_RCV_TOO_LARGE:
			launchd_syslog(LOG_INFO, "Message is larger than %u bytes.", rcv_msg_size);
			break;
		default:
			(void)os_assumes_zero(mr);
		}

		if (mr == MACH_MSG_SUCCESS) {
			if (!mach_exc_server(&bufRequest->Head, &bufReply->Head)) {
				launchd_syslog(LOG_WARNING, "Exception server routine failed.");
				break;
			}

			mach_msg_return_t smr = ~MACH_MSG_SUCCESS;
			mach_msg_option_t send_options = MACH_SEND_MSG | MACH_SEND_TIMEOUT;

			(void)os_assumes(bufReply->Head.msgh_size <= send_msg_size);
			smr = mach_msg(&bufReply->Head, send_options, bufReply->Head.msgh_size, 0, MACH_PORT_NULL, to + 100, MACH_PORT_NULL);
			switch (smr) {
			case MACH_SEND_TIMED_OUT:
				launchd_syslog(LOG_WARNING, "Timed out while trying to send reply to exception message.");
				break;
			case MACH_SEND_INVALID_DEST:
				launchd_syslog(LOG_WARNING, "Tried sending a message to a port that we don't possess a send right to.");
				break;
			default:
				if (smr) {
					launchd_syslog(LOG_WARNING, "Couldn't deliver exception reply: 0x%x", smr);
				}
				break;
			}
		}
	} while (0);

	return mr;
}

void
runtime_record_caller_creds(audit_token_t *token)
{
	(void)memcpy(&ldc_token, token, sizeof(*token));
	audit_token_to_au32(*token, NULL, &ldc.euid,&ldc.egid, &ldc.uid, &ldc.gid,
		&ldc.pid, &ldc.asid, NULL);
}

struct ldcred *
runtime_get_caller_creds(void)
{
	return &ldc;
}

audit_token_t *
runtime_get_caller_token(void)
{
	return &ldc_token;
}

static boolean_t
launchd_mig_demux(mach_msg_header_t *request, mach_msg_header_t *reply)
{
	boolean_t result = false;

	time_of_mach_msg_return = runtime_get_opaque_time();
	launchd_syslog(LOG_DEBUG, "MIG callout: %u", request->msgh_id);
	mig_callback the_demux = mig_cb_table[MACH_PORT_INDEX(request->msgh_local_port)];
	mach_msg_audit_trailer_t *tp = (mach_msg_audit_trailer_t *)((vm_offset_t)request + round_msg(request->msgh_size));
	runtime_record_caller_creds(&tp->msgh_audit);

	result = the_demux(request, reply);
	if (!result) {
		launchd_syslog(LOG_DEBUG, "Demux failed. Trying other subsystems...");
		if (request->msgh_id == MACH_NOTIFY_NO_SENDERS) {
			launchd_syslog(LOG_DEBUG, "MACH_NOTIFY_NO_SENDERS");
			result = notify_server(request, reply);
		} else if (the_demux == job_server) {
			launchd_syslog(LOG_DEBUG, "Trying domain subsystem...");
			result = xpc_domain_server(request, reply);
		} else {
			launchd_syslog(LOG_ERR, "Cannot handle MIG request with ID: 0x%x", request->msgh_id);
		}
	} else {
		launchd_syslog(LOG_DEBUG, "MIG demux succeeded.");
	}

	return result;
}

void
launchd_runtime2(mach_msg_size_t msg_size)
{
	for (;;) {
		launchd_log_push();

		mach_port_t recvp = MACH_PORT_NULL;
		xpc_object_t request = NULL;
		int result = xpc_pipe_try_receive(ipc_port_set, &request, &recvp, launchd_mig_demux, msg_size, 0);
		if (result == 0 && request) {
			boolean_t handled = false;
			time_of_mach_msg_return = runtime_get_opaque_time();
			launchd_syslog(LOG_DEBUG, "XPC request.");

			xpc_object_t reply = NULL;
			if (xpc_event_demux(recvp, request, &reply)) {
				handled = true;
			} else if (xpc_process_demux(recvp, request, &reply)) {
				handled = true;
			}

			if (!handled) {
				launchd_syslog(LOG_DEBUG, "XPC routine could not be handled.");
				xpc_release(request);
				continue;
			}

			launchd_syslog(LOG_DEBUG, "XPC routine was handled.");
			if (reply) {
				launchd_syslog(LOG_DEBUG, "Sending reply.");
				result = xpc_pipe_routine_reply(reply);
				if (result == 0) {
					launchd_syslog(LOG_DEBUG, "Reply sent successfully.");
				} else if (result != EPIPE) {
					launchd_syslog(LOG_ERR, "Failed to send reply message: 0x%x", result);
				}

				xpc_release(reply);
			}

			xpc_release(request);
		} else if (result == 0) {
			launchd_syslog(LOG_DEBUG, "MIG request.");
		} else if (result == EINVAL) {
			launchd_syslog(LOG_ERR, "Rejected invalid request message.");
		}
	}
}

int
runtime_close(int fd)
{
	int i;

	if (bulk_kev) for (i = bulk_kev_i + 1; i < bulk_kev_cnt; i++) {
		switch (bulk_kev[i].filter) {
		case EVFILT_VNODE:
		case EVFILT_WRITE:
		case EVFILT_READ:
			if (unlikely((int)bulk_kev[i].ident == fd)) {
				launchd_syslog(LOG_DEBUG, "Skipping kevent index: %d", i);
				bulk_kev[i].filter = 0;
			}
		default:
			break;
		}
	}

	return close(fd);
}

int
runtime_fsync(int fd)
{
#if 0
	if (launchd_apple_internal) {
		return fcntl(fd, F_FULLFSYNC, NULL);
	} else {
		return fsync(fd);
	}
#else
	return fsync(fd);
#endif
}

/*
 * We should break this into two reference counts.
 *
 * One for hard references that would prevent exiting.
 * One for soft references that would only prevent idle exiting.
 *
 * In the long run, reference counting should completely automate when a
 * process can and should exit.
 */
void
runtime_add_ref(void)
{
	if (!pid1_magic) {
#if !TARGET_OS_EMBEDDED
		vproc_transaction_begin(NULL);
#endif
	}

	runtime_busy_cnt++;
	launchd_syslog(LOG_PERF, "Incremented busy count. Now: %lu", runtime_busy_cnt);
	runtime_remove_timer();
}

void
runtime_del_ref(void)
{
	if (!pid1_magic) {
#if !TARGET_OS_EMBEDDED
		if (_vproc_transaction_count() == 0) {
			launchd_syslog(LOG_PERF, "Exiting cleanly.");
		}

		vproc_transaction_end(NULL, NULL);
#endif
	}

	runtime_busy_cnt--;
	launchd_syslog(LOG_PERF, "Decremented busy count. Now: %lu", runtime_busy_cnt);
	runtime_install_timer();
}

void
runtime_add_weak_ref(void)
{
	if (!pid1_magic) {
#if !TARGET_OS_EMBEDDED
		_vproc_standby_begin();
#endif
	}
	runtime_standby_cnt++;
}

void
runtime_del_weak_ref(void)
{
	if (!pid1_magic) {
#if !TARGET_OS_EMBEDDED
		_vproc_standby_end();
#endif
	}
	runtime_standby_cnt--;
}

void
runtime_install_timer(void)
{
	if (!pid1_magic && runtime_busy_cnt == 0) {
		launchd_syslog(LOG_PERF, "Gone idle. Installing idle-exit timer.");
		(void)posix_assumes_zero(kevent_mod((uintptr_t)&launchd_runtime_busy_time, EVFILT_TIMER, EV_ADD, NOTE_SECONDS, 10, root_jobmgr));
	}
}

void
runtime_remove_timer(void)
{
	if (!pid1_magic && runtime_busy_cnt > 0) {
		if (runtime_busy_cnt == 1) {
			launchd_syslog(LOG_PERF, "No longer idle. Removing idle-exit timer.");	
		}
		(void)posix_assumes_zero(kevent_mod((uintptr_t)&launchd_runtime_busy_time, EVFILT_TIMER, EV_DELETE, 0, 0, NULL));
	}
}

kern_return_t
catch_mach_exception_raise(mach_port_t exception_port __attribute__((unused)), mach_port_t thread, mach_port_t task,
		exception_type_t exception, mach_exception_data_t code, mach_msg_type_number_t codeCnt)
{
	pid_t p4t = -1;

	(void)os_assumes_zero(pid_for_task(task, &p4t));

	launchd_syslog(LOG_NOTICE, "%s(): PID: %u thread: 0x%x type: 0x%x code: %p codeCnt: 0x%x",
			__func__, p4t, thread, exception, code, codeCnt);

	(void)os_assumes_zero(launchd_mport_deallocate(thread));
	(void)os_assumes_zero(launchd_mport_deallocate(task));

	return KERN_SUCCESS;
}

kern_return_t
catch_mach_exception_raise_state(mach_port_t exception_port __attribute__((unused)),
		exception_type_t exception, const mach_exception_data_t code, mach_msg_type_number_t codeCnt,
		int *flavor, const thread_state_t old_state, mach_msg_type_number_t old_stateCnt,
		thread_state_t new_state, mach_msg_type_number_t *new_stateCnt)
{
	launchd_syslog(LOG_NOTICE, "%s(): type: 0x%x code: %p codeCnt: 0x%x flavor: %p old_state: %p old_stateCnt: 0x%x new_state: %p new_stateCnt: %p",
			__func__, exception, code, codeCnt, flavor, old_state, old_stateCnt, new_state, new_stateCnt);

	memcpy(new_state, old_state, old_stateCnt * sizeof(old_state[0]));
	*new_stateCnt = old_stateCnt;

	return KERN_SUCCESS;
}

kern_return_t
catch_mach_exception_raise_state_identity(mach_port_t exception_port __attribute__((unused)), mach_port_t thread, mach_port_t task,
		exception_type_t exception, mach_exception_data_t code, mach_msg_type_number_t codeCnt,
		int *flavor, thread_state_t old_state, mach_msg_type_number_t old_stateCnt,
		thread_state_t new_state, mach_msg_type_number_t *new_stateCnt)
{
	pid_t p4t = -1;

	(void)os_assumes_zero(pid_for_task(task, &p4t));

	launchd_syslog(LOG_NOTICE, "%s(): PID: %u thread: 0x%x type: 0x%x code: %p codeCnt: 0x%x flavor: %p old_state: %p old_stateCnt: 0x%x new_state: %p new_stateCnt: %p",
			__func__, p4t, thread, exception, code, codeCnt, flavor, old_state, old_stateCnt, new_state, new_stateCnt);

	memcpy(new_state, old_state, old_stateCnt * sizeof(old_state[0]));
	*new_stateCnt = old_stateCnt;

	(void)os_assumes_zero(launchd_mport_deallocate(thread));
	(void)os_assumes_zero(launchd_mport_deallocate(task));

	return KERN_SUCCESS;
}

// FIXME: should this be thread safe? With dispatch_once?
uint64_t
runtime_get_uniqueid(void)
{
	static bool once;
	static uint64_t uniqueid;
	if (unlikely(!once)) {
		once = true;

		struct proc_uniqidentifierinfo info;
		int size;
		size = proc_pidinfo(getpid(), PROC_PIDUNIQIDENTIFIERINFO, 0, &info, sizeof(info));
		if (size == PROC_PIDUNIQIDENTIFIERINFO_SIZE) {
			uniqueid = info.p_uniqueid;
		}
	}
	return uniqueid;
}

void
launchd_log_vm_stats(void)
{
	static struct vm_statistics orig_stats;
	static bool did_first_pass;
	unsigned int count = HOST_VM_INFO_COUNT;
	struct vm_statistics stats, *statsp;
	mach_port_t mhs = mach_host_self();

	statsp = did_first_pass ? &stats : &orig_stats;

	if (os_assumes_zero(host_statistics(mhs, HOST_VM_INFO, (host_info_t)statsp, &count)) != KERN_SUCCESS) {
		return;
	}

	if (count != HOST_VM_INFO_COUNT) {
		(void)os_assumes_zero(count);
	}

	if (did_first_pass) {
		launchd_syslog(LOG_DEBUG, "VM statistics (now - orig): Free: %d Active: %d Inactive: %d Reactivations: %d PageIns: %d PageOuts: %d Faults: %d COW-Faults: %d Purgeable: %d Purges: %d",
				stats.free_count - orig_stats.free_count,
				stats.active_count - orig_stats.active_count,
				stats.inactive_count - orig_stats.inactive_count,
				stats.reactivations - orig_stats.reactivations,
				stats.pageins - orig_stats.pageins,
				stats.pageouts - orig_stats.pageouts,
				stats.faults - orig_stats.faults,
				stats.cow_faults - orig_stats.cow_faults,
				stats.purgeable_count - orig_stats.purgeable_count,
				stats.purges - orig_stats.purges);
	} else {
		launchd_syslog(LOG_DEBUG, "VM statistics (now): Free: %d Active: %d Inactive: %d Reactivations: %d PageIns: %d PageOuts: %d Faults: %d COW-Faults: %d Purgeable: %d Purges: %d",
				orig_stats.free_count,
				orig_stats.active_count,
				orig_stats.inactive_count,
				orig_stats.reactivations,
				orig_stats.pageins,
				orig_stats.pageouts,
				orig_stats.faults,
				orig_stats.cow_faults,
				orig_stats.purgeable_count,
				orig_stats.purges);

		did_first_pass = true;
	}

	launchd_mport_deallocate(mhs);
}

int64_t
runtime_get_wall_time(void)
{
	struct timeval tv;
	int64_t r;

	(void)posix_assumes_zero(gettimeofday(&tv, NULL));

	r = tv.tv_sec;
	r *= USEC_PER_SEC;
	r += tv.tv_usec;

	return r;
}

uint64_t
runtime_get_opaque_time(void)
{
	return mach_absolute_time();
}

uint64_t
runtime_get_opaque_time_of_event(void)
{
	return time_of_mach_msg_return;
}

uint64_t
runtime_get_nanoseconds_since(uint64_t o)
{
	return runtime_opaque_time_to_nano(runtime_get_opaque_time_of_event() - o);
}

uint64_t
runtime_opaque_time_to_nano(uint64_t o)
{
#if defined(__i386__) || defined(__x86_64__)
	if (unlikely(tbi.numer != tbi.denom)) {
#elif defined(__ppc__) || defined(__ppc64__)
	if (likely(tbi.numer != tbi.denom)) {
#else
	if (tbi.numer != tbi.denom) {
#endif
#ifdef __LP64__
		__uint128_t tmp = o;
		tmp *= tbi.numer;
		tmp /= tbi.denom;
		o = tmp;
#else
		if (o <= tbi_safe_math_max) {
			o *= tbi.numer;
			o /= tbi.denom;
		} else {
			double d = o;
			d *= tbi_float_val;
			o = d;
		}
#endif
	}

	return o;
}

void
do_file_init(void)
{
	struct stat sb;

	os_assert_zero(mach_timebase_info(&tbi));
	tbi_float_val = tbi.numer;
	tbi_float_val /= tbi.denom;
	tbi_safe_math_max = UINT64_MAX / tbi.numer;

	launchd_system_start = runtime_get_wall_time();

	if (getpid() == 1) {
		pid1_magic = true;
	}

	if (stat("/AppleInternal", &sb) == 0 && stat("/var/db/disableAppleInternal", &sb) == -1) {
		launchd_apple_internal = true;
	}

	if (config_check(".launchd_use_gmalloc", sb)) {
		launchd_use_gmalloc = true;
	}

	if (config_check(".launchd_log_shutdown", sb)) {
		launchd_log_shutdown = true;
	}

	if (config_check(".launchd_log_debug", sb)) {
		launchd_log_debug = true;
	}

	if (config_check(".launchd_log_perf", sb)) {
		launchd_log_perf = true;
	}

	if (config_check("/etc/rc.cdrom", sb)) {
		launchd_osinstaller = true;
	}

	if (!pid1_magic && config_check(".launchd_allow_global_dyld_envvars", sb)) {
		launchd_allow_global_dyld_envvars = true;
	}

	char buff[1024];
	size_t len = sizeof(buff) - 1;
	int r = pid1_magic ? sysctlbyname("kern.bootargs", buff, &len, NULL, 0) : -1;
	if (r == 0) {
		if (strnstr(buff, "-v", len)) {
			launchd_verbose_boot = true;
		}
		if (strnstr(buff, "launchd_trap_sigkill_bugs", len)) {
			launchd_trap_sigkill_bugs = true;
		}
		if (strnstr(buff, "launchd_no_jetsam_perm_check", len)) {
			launchd_no_jetsam_perm_check = true;
		}
	}

	len = sizeof(buff) - 1;
#if TARGET_OS_EMBEDDED
	r = sysctlbyname("hw.machine", buff, &len, NULL, 0);
	if (r == 0) {
		if (strnstr(buff, "AppleTV", len)) {
			launchd_appletv = true;
		}
	}
#endif

#if !TARGET_OS_EMBEDDED
	if (pid1_magic && launchd_verbose_boot && config_check(".launchd_shutdown_debugging", sb)) {
		launchd_shutdown_debugging = true;
	}
#else
	if (pid1_magic && config_check(".launchd_shutdown_debugging", sb)) {
		launchd_shutdown_debugging = true;
	}
#endif
}
