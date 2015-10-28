/*
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
#include "core.h"
#include "internal.h"
#include "helper.h"

#include <TargetConditionals.h>
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
#include <mach/exception.h>
#include <mach/host_reboot.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/event.h>
#include <sys/stat.h>
#include <sys/ucred.h>
#include <sys/fcntl.h>
#include <sys/un.h>
#include <sys/reboot.h>
#include <sys/wait.h>
#include <sys/sysctl.h>
#include <sys/sockio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/pipe.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/kern_memorystatus.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet6/nd6.h>
#include <bsm/libbsm.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <paths.h>
#include <pwd.h>
#include <grp.h>
#include <ttyent.h>
#include <dlfcn.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <glob.h>
#include <System/sys/spawn.h>
#include <System/sys/spawn_internal.h>
#include <spawn.h>
#include <spawn_private.h>
#include <time.h>
#include <libinfo.h>
#include <os/assumes.h>
#include <xpc/launchd.h>
#include <asl.h>
#include <_simple.h>

#include <libproc.h>
#include <libproc_internal.h>
#include <System/sys/proc_info.h>
#include <malloc/malloc.h>
#include <pthread.h>
#if HAVE_SANDBOX
#define __APPLE_API_PRIVATE
#include <sandbox.h>
#endif
#if HAVE_QUARANTINE
#include <quarantine.h>
#endif
#if HAVE_RESPONSIBILITY
#include <responsibility.h>
#endif
#if !TARGET_OS_EMBEDDED
extern int gL1CacheEnabled;
#endif
#if HAVE_SYSTEMSTATS
#include <systemstats/systemstats.h>
#endif

#include "launch.h"
#include "launch_priv.h"
#include "launch_internal.h"
#include "bootstrap.h"
#include "bootstrap_priv.h"
#include "vproc.h"
#include "vproc_internal.h"

#include "reboot2.h"

#include "launchd.h"
#include "runtime.h"
#include "ipc.h"
#include "job.h"
#include "jobServer.h"
#include "job_reply.h"
#include "job_forward.h"
#include "mach_excServer.h"

#define POSIX_SPAWN_IOS_INTERACTIVE 0

#if TARGET_OS_EMBEDDED
/* Default memory highwatermark for daemons as set out in <rdar://problem/10307788>. */
#define DEFAULT_JETSAM_DAEMON_HIGHWATERMARK 5
#endif

/* LAUNCHD_DEFAULT_EXIT_TIMEOUT
 *   If the job hasn't exited in the given number of seconds after sending
 *   it a SIGTERM, SIGKILL it. Can be overriden in the job plist.
 */
#define LAUNCHD_MIN_JOB_RUN_TIME 10
#define LAUNCHD_DEFAULT_EXIT_TIMEOUT 20
#define LAUNCHD_SIGKILL_TIMER 4
#define LAUNCHD_LOG_FAILED_EXEC_FREQ 10

#define SHUTDOWN_LOG_DIR "/var/log/shutdown"

#define TAKE_SUBSET_NAME "TakeSubsetName"
#define TAKE_SUBSET_PID "TakeSubsetPID"
#define TAKE_SUBSET_PERPID "TakeSubsetPerPID"

#define IS_POWER_OF_TWO(v) (!(v & (v - 1)) && v)

extern char **environ;

struct waiting_for_removal {
	SLIST_ENTRY(waiting_for_removal) sle;
	mach_port_t reply_port;
};

static bool waiting4removal_new(job_t j, mach_port_t rp);
static void waiting4removal_delete(job_t j, struct waiting_for_removal *w4r);

struct machservice {
	SLIST_ENTRY(machservice) sle;
	SLIST_ENTRY(machservice) special_port_sle;
	LIST_ENTRY(machservice) name_hash_sle;
	LIST_ENTRY(machservice) port_hash_sle;
	struct machservice *alias;
	job_t job;
	unsigned int gen_num;
	mach_port_name_t port;
	unsigned int
		isActive:1,
		reset:1,
		recv:1,
		hide:1,
		kUNCServer:1,
		per_user_hack:1,
		debug_on_close:1,
		per_pid:1,
		delete_on_destruction:1,
		drain_one_on_crash:1,
		drain_all_on_crash:1,
		upfront:1,
		event_channel:1,
		recv_race_hack :1,
		/* Don't let the size of this field to get too small. It has to be large
		 * enough to represent the reasonable range of special port numbers.
		 */
		special_port_num:17;
	const char name[0];
};

// HACK: This should be per jobmgr_t
static SLIST_HEAD(, machservice) special_ports;

#define PORT_HASH_SIZE 32
#define HASH_PORT(x) (IS_POWER_OF_TWO(PORT_HASH_SIZE) ? (MACH_PORT_INDEX(x) & (PORT_HASH_SIZE - 1)) : (MACH_PORT_INDEX(x) % PORT_HASH_SIZE))

static LIST_HEAD(, machservice) port_hash[PORT_HASH_SIZE];

static void machservice_setup(launch_data_t obj, const char *key, void *context);
static void machservice_setup_options(launch_data_t obj, const char *key, void *context);
static void machservice_resetport(job_t j, struct machservice *ms);
static void machservice_stamp_port(job_t j, struct machservice *ms);
static struct machservice *machservice_new(job_t j, const char *name, mach_port_t *serviceport, bool pid_local);
static struct machservice *machservice_new_alias(job_t aj, struct machservice *orig);
static void machservice_ignore(job_t j, struct machservice *ms);
static void machservice_watch(job_t j, struct machservice *ms);
static void machservice_delete(job_t j, struct machservice *, bool port_died);
static void machservice_request_notifications(struct machservice *);
static mach_port_t machservice_port(struct machservice *);
static job_t machservice_job(struct machservice *);
static bool machservice_hidden(struct machservice *);
static bool machservice_active(struct machservice *);
static const char *machservice_name(struct machservice *);
static bootstrap_status_t machservice_status(struct machservice *);
void machservice_drain_port(struct machservice *);

struct socketgroup {
	SLIST_ENTRY(socketgroup) sle;
	int *fds;
	unsigned int fd_cnt;
	union {
		const char name[0];
		char name_init[0];
	};
};

static bool socketgroup_new(job_t j, const char *name, int *fds, size_t fd_cnt);
static void socketgroup_delete(job_t j, struct socketgroup *sg);
static void socketgroup_watch(job_t j, struct socketgroup *sg);
static void socketgroup_ignore(job_t j, struct socketgroup *sg);
static void socketgroup_callback(job_t j);
static void socketgroup_setup(launch_data_t obj, const char *key, void *context);
static void socketgroup_kevent_mod(job_t j, struct socketgroup *sg, bool do_add);

struct calendarinterval {
	LIST_ENTRY(calendarinterval) global_sle;
	SLIST_ENTRY(calendarinterval) sle;
	job_t job;
	struct tm when;
	time_t when_next;
};

static LIST_HEAD(, calendarinterval) sorted_calendar_events;

static bool calendarinterval_new(job_t j, struct tm *w);
static bool calendarinterval_new_from_obj(job_t j, launch_data_t obj);
static void calendarinterval_new_from_obj_dict_walk(launch_data_t obj, const char *key, void *context);
static void calendarinterval_delete(job_t j, struct calendarinterval *ci);
static void calendarinterval_setalarm(job_t j, struct calendarinterval *ci);
static void calendarinterval_callback(void);
static void calendarinterval_sanity_check(void);

struct envitem {
	SLIST_ENTRY(envitem) sle;
	char *value;
	union {
		const char key[0];
		char key_init[0];
	};
};

static bool envitem_new(job_t j, const char *k, const char *v, bool global);
static void envitem_delete(job_t j, struct envitem *ei, bool global);
static void envitem_setup(launch_data_t obj, const char *key, void *context);

struct limititem {
	SLIST_ENTRY(limititem) sle;
	struct rlimit lim;
	unsigned int setsoft:1, sethard:1, which:30;
};

static bool limititem_update(job_t j, int w, rlim_t r);
static void limititem_delete(job_t j, struct limititem *li);
static void limititem_setup(launch_data_t obj, const char *key, void *context);
#if HAVE_SANDBOX
static void seatbelt_setup_flags(launch_data_t obj, const char *key, void *context);
#endif

static void jetsam_property_setup(launch_data_t obj, const char *key, job_t j);

typedef enum {
	NETWORK_UP = 1,
	NETWORK_DOWN,
	SUCCESSFUL_EXIT,
	FAILED_EXIT,
	CRASHED,
	DID_NOT_CRASH,
	OTHER_JOB_ENABLED,
	OTHER_JOB_DISABLED,
	OTHER_JOB_ACTIVE,
	OTHER_JOB_INACTIVE,
} semaphore_reason_t;

struct semaphoreitem {
	SLIST_ENTRY(semaphoreitem) sle;
	semaphore_reason_t why;

	union {
		const char what[0];
		char what_init[0];
	};
};

struct semaphoreitem_dict_iter_context {
	job_t j;
	semaphore_reason_t why_true;
	semaphore_reason_t why_false;
};

static bool semaphoreitem_new(job_t j, semaphore_reason_t why, const char *what);
static void semaphoreitem_delete(job_t j, struct semaphoreitem *si);
static void semaphoreitem_setup(launch_data_t obj, const char *key, void *context);
static void semaphoreitem_setup_dict_iter(launch_data_t obj, const char *key, void *context);
static void semaphoreitem_runtime_mod_ref(struct semaphoreitem *si, bool add);

struct externalevent {
	LIST_ENTRY(externalevent) sys_le;
	LIST_ENTRY(externalevent) job_le;
	struct eventsystem *sys;

	uint64_t id;
	job_t job;
	bool state;
	bool wanted_state;
	bool internal;
	xpc_object_t event;
	xpc_object_t entitlements;

	char name[0];
};

struct externalevent_iter_ctx {
	job_t j;
	struct eventsystem *sys;
};

static bool externalevent_new(job_t j, struct eventsystem *sys, const char *evname, xpc_object_t event, uint64_t flags);
static void externalevent_delete(struct externalevent *ee);
static void externalevent_setup(launch_data_t obj, const char *key, void *context);
static struct externalevent *externalevent_find(const char *sysname, uint64_t id);

struct eventsystem {
	LIST_ENTRY(eventsystem) global_le;
	LIST_HEAD(, externalevent) events;
	uint64_t curid;
	char name[0];
};

static struct eventsystem *eventsystem_new(const char *name);
static void eventsystem_delete(struct eventsystem *sys) __attribute__((unused));
static void eventsystem_setup(launch_data_t obj, const char *key, void *context);
static struct eventsystem *eventsystem_find(const char *name);
static void eventsystem_ping(void);

struct waiting4attach {
	LIST_ENTRY(waiting4attach) le;
	mach_port_t port;
	pid_t dest;
	xpc_service_type_t type;
	char name[0];
};

static LIST_HEAD(, waiting4attach) _launchd_domain_waiters;

static struct waiting4attach *waiting4attach_new(jobmgr_t jm, const char *name, mach_port_t port, pid_t dest, xpc_service_type_t type);
static void waiting4attach_delete(jobmgr_t jm, struct waiting4attach *w4a);
static struct waiting4attach *waiting4attach_find(jobmgr_t jm, job_t j);

#define ACTIVE_JOB_HASH_SIZE 32
#define ACTIVE_JOB_HASH(x) (IS_POWER_OF_TWO(ACTIVE_JOB_HASH_SIZE) ? (x & (ACTIVE_JOB_HASH_SIZE - 1)) : (x % ACTIVE_JOB_HASH_SIZE))

#define MACHSERVICE_HASH_SIZE	37

#define LABEL_HASH_SIZE 53
struct jobmgr_s {
	kq_callback kqjobmgr_callback;
	LIST_ENTRY(jobmgr_s) xpc_le;
	SLIST_ENTRY(jobmgr_s) sle;
	SLIST_HEAD(, jobmgr_s) submgrs;
	LIST_HEAD(, job_s) jobs;
	LIST_HEAD(, waiting4attach) attaches;

	/* For legacy reasons, we keep all job labels that are imported in the root
	 * job manager's label hash. If a job manager is an XPC domain, then it gets
	 * its own label hash that is separate from the "global" one stored in the
	 * root job manager.
	 */
	LIST_HEAD(, job_s) label_hash[LABEL_HASH_SIZE];
	LIST_HEAD(, job_s) active_jobs[ACTIVE_JOB_HASH_SIZE];
	LIST_HEAD(, machservice) ms_hash[MACHSERVICE_HASH_SIZE];
	LIST_HEAD(, job_s) global_env_jobs;
	mach_port_t jm_port;
	mach_port_t req_port;
	jobmgr_t parentmgr;
	int reboot_flags;
	time_t shutdown_time;
	unsigned int global_on_demand_cnt;
	unsigned int normal_active_cnt;
	unsigned int 
		shutting_down:1,
		session_initialized:1, 
		killed_stray_jobs:1,
		monitor_shutdown:1,
		shutdown_jobs_dirtied:1,
		shutdown_jobs_cleaned:1,
		xpc_singleton:1;
	uint32_t properties;
	// XPC-specific properties.
	char owner[MAXCOMLEN];
	char *shortdesc;
	mach_port_t req_bsport;
	mach_port_t req_excport;
	mach_port_t req_asport;
	mach_port_t req_gui_asport;
	pid_t req_pid;
	uid_t req_euid;
	gid_t req_egid;
	au_asid_t req_asid;
	vm_offset_t req_ctx;
	mach_msg_type_number_t req_ctx_sz;
	mach_port_t req_rport;
	uint64_t req_uniqueid;
	kern_return_t error;
	union {
		const char name[0];
		char name_init[0];
	};
};

// Global XPC domains.
static jobmgr_t _s_xpc_system_domain;
static LIST_HEAD(, jobmgr_s) _s_xpc_user_domains;
static LIST_HEAD(, jobmgr_s) _s_xpc_session_domains;

#define jobmgr_assumes(jm, e) os_assumes_ctx(jobmgr_log_bug, jm, (e))
#define jobmgr_assumes_zero(jm, e) os_assumes_zero_ctx(jobmgr_log_bug, jm, (e))
#define jobmgr_assumes_zero_p(jm, e) posix_assumes_zero_ctx(jobmgr_log_bug, jm, (e))

static jobmgr_t jobmgr_new(jobmgr_t jm, mach_port_t requestorport, mach_port_t transfer_port, bool sflag, const char *name, bool no_init, mach_port_t asport);
static jobmgr_t jobmgr_new_xpc_singleton_domain(jobmgr_t jm, name_t name);
static jobmgr_t jobmgr_find_xpc_per_user_domain(jobmgr_t jm, uid_t uid);
static jobmgr_t jobmgr_find_xpc_per_session_domain(jobmgr_t jm, au_asid_t asid);
static job_t jobmgr_import2(jobmgr_t jm, launch_data_t pload);
static jobmgr_t jobmgr_parent(jobmgr_t jm);
static jobmgr_t jobmgr_do_garbage_collection(jobmgr_t jm);
static bool jobmgr_label_test(jobmgr_t jm, const char *str);
static void jobmgr_reap_bulk(jobmgr_t jm, struct kevent *kev);
static void jobmgr_log_stray_children(jobmgr_t jm, bool kill_strays);
static void jobmgr_kill_stray_children(jobmgr_t jm, pid_t *p, size_t np);
static void jobmgr_remove(jobmgr_t jm);
static void jobmgr_dispatch_all(jobmgr_t jm, bool newmounthack);
static job_t jobmgr_init_session(jobmgr_t jm, const char *session_type, bool sflag);
static job_t jobmgr_find_by_pid_deep(jobmgr_t jm, pid_t p, bool anon_okay);
static job_t jobmgr_find_by_pid(jobmgr_t jm, pid_t p, bool create_anon);
static job_t managed_job(pid_t p);
static jobmgr_t jobmgr_find_by_name(jobmgr_t jm, const char *where);
static job_t job_mig_intran2(jobmgr_t jm, mach_port_t mport, pid_t upid);
static job_t jobmgr_lookup_per_user_context_internal(job_t j, uid_t which_user, mach_port_t *mp);
static void job_export_all2(jobmgr_t jm, launch_data_t where);
static void jobmgr_callback(void *obj, struct kevent *kev);
static void jobmgr_setup_env_from_other_jobs(jobmgr_t jm);
static void jobmgr_export_env_from_other_jobs(jobmgr_t jm, launch_data_t dict);
static struct machservice *jobmgr_lookup_service(jobmgr_t jm, const char *name, bool check_parent, pid_t target_pid);
static void jobmgr_logv(jobmgr_t jm, int pri, int err, const char *msg, va_list ap) __attribute__((format(printf, 4, 0)));
static void jobmgr_log(jobmgr_t jm, int pri, const char *msg, ...) __attribute__((format(printf, 3, 4)));
static void jobmgr_log_perf_statistics(jobmgr_t jm, bool signal_children);
// static void jobmgr_log_error(jobmgr_t jm, int pri, const char *msg, ...) __attribute__((format(printf, 3, 4)));
static bool jobmgr_log_bug(_SIMPLE_STRING asl_message, void *ctx, const char *message);

#define AUTO_PICK_LEGACY_LABEL (const char *)(~0)
#define AUTO_PICK_ANONYMOUS_LABEL (const char *)(~1)
#define AUTO_PICK_XPC_LABEL (const char *)(~2)

struct suspended_peruser {
	LIST_ENTRY(suspended_peruser) sle;
	job_t j;
};

struct job_s {
	// MUST be first element of this structure.
	kq_callback kqjob_callback;
	LIST_ENTRY(job_s) sle;
	LIST_ENTRY(job_s) subjob_sle;
	LIST_ENTRY(job_s) needing_session_sle;
	LIST_ENTRY(job_s) jetsam_sle;
	LIST_ENTRY(job_s) pid_hash_sle;
	LIST_ENTRY(job_s) global_pid_hash_sle;
	LIST_ENTRY(job_s) label_hash_sle;
	LIST_ENTRY(job_s) global_env_sle;
	SLIST_ENTRY(job_s) curious_jobs_sle;
	LIST_HEAD(, suspended_peruser) suspended_perusers;
	LIST_HEAD(, waiting_for_exit) exit_watchers;
	LIST_HEAD(, job_s) subjobs;
	LIST_HEAD(, externalevent) events;
	SLIST_HEAD(, socketgroup) sockets;
	SLIST_HEAD(, calendarinterval) cal_intervals;
	SLIST_HEAD(, envitem) global_env;
	SLIST_HEAD(, envitem) env;
	SLIST_HEAD(, limititem) limits;
	SLIST_HEAD(, machservice) machservices;
	SLIST_HEAD(, semaphoreitem) semaphores;
	SLIST_HEAD(, waiting_for_removal) removal_watchers;
	struct waiting4attach *w4a;
	job_t original;
	job_t alias;
	cpu_type_t *j_binpref;
	size_t j_binpref_cnt;
	mach_port_t j_port;
	mach_port_t exit_status_dest;
	mach_port_t exit_status_port;
	mach_port_t spawn_reply_port;
	uid_t mach_uid;
	jobmgr_t mgr;
	size_t argc;
	char **argv;
	char *prog;
	char *rootdir;
	char *workingdir;
	char *username;
	char *groupname;
	char *stdinpath;
	char *stdoutpath;
	char *stderrpath;
	char *alt_exc_handler;
	char *cfbundleidentifier;
	unsigned int nruns;
	uint64_t trt;
#if HAVE_SANDBOX
	char *seatbelt_profile;
	uint64_t seatbelt_flags;
	char *container_identifier;
#endif
#if HAVE_QUARANTINE
	void *quarantine_data;
	size_t quarantine_data_sz;
#endif
	pid_t p;
	uint64_t uniqueid;
	int last_exit_status;
	int stdin_fd;
	int fork_fd;
	int nice;
	uint32_t pstype;
	uint32_t psproctype;
	int32_t jetsam_priority;
	int32_t jetsam_memlimit;
	int32_t main_thread_priority;
	uint32_t timeout;
	uint32_t exit_timeout;
	uint64_t sent_signal_time;
	uint64_t start_time;
	uint32_t min_run_time;
	bool unthrottle;
	uint32_t start_interval;
	uint32_t peruser_suspend_count;
	uuid_t instance_id;
	mode_t mask;
	mach_port_t asport;
	au_asid_t asid;
	uuid_t expected_audit_uuid;
	bool 	
		// man launchd.plist --> Debug
		debug:1,
		// man launchd.plist --> KeepAlive == false
		ondemand:1,
		// man launchd.plist --> SessionCreate
		session_create:1,
		// man launchd.plist --> LowPriorityIO
		low_pri_io:1,
		// man launchd.plist --> InitGroups
		no_init_groups:1,
		/* A legacy mach_init concept to make bootstrap_create_server/service()
		 * work
		 */
		priv_port_has_senders:1,
		// A hack during job importing
		importing_global_env:1,
		// A hack during job importing
		importing_hard_limits:1,
		// man launchd.plist --> Umask
		setmask:1,
		// A process that launchd knows about but doesn't manage.
		anonymous:1,
		// A legacy mach_init concept to detect sick jobs
		checkedin:1,
		// A job created via bootstrap_create_server()
		legacy_mach_job:1,
		// A job created via spawn_via_launchd()
		legacy_LS_job:1,
		// A legacy job that wants inetd compatible semantics
		inetcompat:1,
		// A twist on inetd compatibility
		inetcompat_wait:1,
		/* An event fired and the job should start, but not necessarily right
		 * away.
		 */	
		start_pending:1,
		// man launchd.plist --> EnableGlobbing
		globargv:1,
		// man launchd.plist --> WaitForDebugger
		wait4debugger:1,
		// One-shot WaitForDebugger.
		wait4debugger_oneshot:1,
		// MachExceptionHandler == true
		internal_exc_handler:1,
		// A hack to support an option of spawn_via_launchd()
		stall_before_exec:1,
		/* man launchd.plist --> LaunchOnlyOnce.
		 *
		 * Note: <rdar://problem/5465184> Rename this to "HopefullyNeverExits".
		 */
		only_once:1,
		/* Make job_ignore() / job_watch() work. If these calls were balanced,
		 * then this wouldn't be necessarily.
		 */
		currently_ignored:1,
		/* A job that forced all other jobs to be temporarily launch-on-
		 * demand
		 */
		forced_peers_to_demand_mode:1,
		// man launchd.plist --> Nice
		setnice:1,
		/* A job was asked to be unloaded/removed while running, we'll remove it
		 * after it exits.
		 */
		removal_pending:1,
		// job_kill() was called.
		sent_sigkill:1,
		// Enter the kernel debugger before killing a job.
		debug_before_kill:1,
		// A hack that launchd+launchctl use during jobmgr_t creation.
		weird_bootstrap:1,
		// man launchd.plist --> StartOnMount
		start_on_mount:1,
		// This job is a per-user launchd managed by the PID 1 launchd.
		per_user:1,
		// A job thoroughly confused launchd. We need to unload it ASAP.
		unload_at_mig_return:1,
		// man launchd.plist --> AbandonProcessGroup
		abandon_pg:1,
		/* During shutdown, do not send SIGTERM to stray processes in the
		 * process group of this job.
		 */
		ignore_pg_at_shutdown:1,
		/* Don't let this job create new 'job_t' objects in launchd. Has been
		 * seriously overloaded for the purposes of sandboxing.
		 */
		deny_job_creation:1,
		// man launchd.plist --> EnableTransactions
		enable_transactions:1,
		// The job was sent SIGKILL because it was clean.
		clean_kill:1,
		// The job has an OtherJobEnabled KeepAlive criterion.
		nosy:1,
		// The job exited due to a crash.
		crashed:1,
		// We've received NOTE_EXIT for the job and reaped it.
		reaped:1,
		// job_stop() was called.
		stopped:1,
		/* The job is to be kept alive continuously, but it must first get an
		 * initial kick off.
		 */
		needs_kickoff:1,
		// The job is a bootstrapper.
		is_bootstrapper:1,
		// The job owns the console.
		has_console:1,
		/* The job runs as a non-root user on embedded but has select privileges
		 * of the root user. This is SpringBoard.
		 */
		embedded_god:1,
		// The job is responsible for drawing the home screen on embedded.
		embedded_home:1,
		// We got NOTE_EXEC for the job.
		did_exec:1,
		// The job is an XPC service, and XPC proxy successfully exec(3)ed.
		xpcproxy_did_exec:1,
		// The (anonymous) job called vprocmgr_switch_to_session().
		holds_ref:1,
		// The job has Jetsam limits in place.
		jetsam_properties:1,
		// The job's Jetsam memory limits should only be applied in the background
 		jetsam_memory_limit_background:1,
		/* This job was created as the result of a look up of a service provided
		 * by a MultipleInstance job.
		 */
		dedicated_instance:1,
		// The job supports creating additional instances of itself.
		multiple_instances:1,
		/* The sub-job was already removed from the parent's list of
		 * sub-jobs.
		 */
		former_subjob:1,
		/* The job is responsible for monitoring external events for this
		 * launchd.
		 */
		event_monitor:1,
		// The event monitor job has retrieved the initial list of events.
		event_monitor_ready2signal:1,
		// A lame hack.
		removing:1,
		// Disable ASLR when launching this job.
		disable_aslr:1,
		// The job is an XPC Service.
		xpc_service:1,
		// The job is the Performance team's shutdown monitor.
		shutdown_monitor:1,
		// We should open a transaction for the job when shutdown begins.
		dirty_at_shutdown:1,
		/* The job was sent SIGKILL but did not exit in a timely fashion,
		 * indicating a kernel bug.
		 */
		workaround9359725:1,
		// The job is the XPC domain bootstrapper.
		xpc_bootstrapper:1,
		// The job is an app (on either iOS or OS X) and has different resource
		// limitations.
		app:1,
		// FairPlay decryption failed on the job. This should only ever happen
		// to apps.
		fpfail:1,
		// The job failed to exec(3) for reasons that may be transient, so we're
		// waiting for UserEventAgent to tell us when it's okay to try spawning
		// again (i.e. when the executable path appears, when the UID appears,
		// etc.).
		waiting4ok:1,
		// The job exited due to memory pressure.
		jettisoned:1,
		// The job supports idle-exit.
		idle_exit:1,
		// The job was implicitly reaped by the kernel.
		implicit_reap:1,
		system_app :1,
		joins_gui_session :1,
		low_priority_background_io :1,
		legacy_timers :1;

	const char label[0];
};

static size_t hash_label(const char *label) __attribute__((pure));
static size_t hash_ms(const char *msstr) __attribute__((pure));
static SLIST_HEAD(, job_s) s_curious_jobs;
static LIST_HEAD(, job_s) managed_actives[ACTIVE_JOB_HASH_SIZE];

#define job_assumes(j, e) os_assumes_ctx(job_log_bug, j, (e))
#define job_assumes_zero(j, e) os_assumes_zero_ctx(job_log_bug, j, (e))
#define job_assumes_zero_p(j, e) posix_assumes_zero_ctx(job_log_bug, j, (e))

static void job_import_keys(launch_data_t obj, const char *key, void *context);
static void job_import_bool(job_t j, const char *key, bool value);
static void job_import_string(job_t j, const char *key, const char *value);
static void job_import_integer(job_t j, const char *key, long long value);
static void job_import_dictionary(job_t j, const char *key, launch_data_t value);
static void job_import_array(job_t j, const char *key, launch_data_t value);
static void job_import_opaque(job_t j, const char *key, launch_data_t value);
static bool job_set_global_on_demand(job_t j, bool val);
static const char *job_active(job_t j);
static void job_watch(job_t j);
static void job_ignore(job_t j);
static void job_reap(job_t j);
static bool job_useless(job_t j);
static bool job_keepalive(job_t j);
static void job_dispatch_curious_jobs(job_t j);
static void job_start(job_t j);
static void job_start_child(job_t j) __attribute__((noreturn));
static void job_setup_attributes(job_t j);
static bool job_setup_machport(job_t j);
static kern_return_t job_setup_exit_port(job_t j);
static void job_setup_fd(job_t j, int target_fd, const char *path, int flags);
static void job_postfork_become_user(job_t j);
static void job_postfork_test_user(job_t j);
static void job_log_pids_with_weird_uids(job_t j);
static void job_setup_exception_port(job_t j, task_t target_task);
static void job_callback(void *obj, struct kevent *kev);
static void job_callback_proc(job_t j, struct kevent *kev);
static void job_callback_timer(job_t j, void *ident);
static void job_callback_read(job_t j, int ident);
static void job_log_stray_pg(job_t j);
static void job_log_children_without_exec(job_t j);
static job_t job_new_anonymous(jobmgr_t jm, pid_t anonpid) __attribute__((malloc, nonnull, warn_unused_result));
static job_t job_new(jobmgr_t jm, const char *label, const char *prog, const char *const *argv) __attribute__((malloc, nonnull(1,2), warn_unused_result));
static job_t job_new_alias(jobmgr_t jm, job_t src);
static job_t job_new_via_mach_init(job_t j, const char *cmd, uid_t uid, bool ond) __attribute__((malloc, nonnull, warn_unused_result));
static job_t job_new_subjob(job_t j, uuid_t identifier);
static void job_kill(job_t j);
static void job_uncork_fork(job_t j);
static void job_logv(job_t j, int pri, int err, const char *msg, va_list ap) __attribute__((format(printf, 4, 0)));
static void job_log_error(job_t j, int pri, const char *msg, ...) __attribute__((format(printf, 3, 4)));
static bool job_log_bug(_SIMPLE_STRING asl_message, void *ctx, const char *message);
static void job_log_perf_statistics(job_t j, struct rusage_info_v1 *ri, int64_t exit_status);
#if HAVE_SYSTEMSTATS
static void job_log_systemstats(pid_t pid, uint64_t uniqueid, uint64_t parent_uniqueid, pid_t req_pid, uint64_t req_uniqueid, const char *name, struct rusage_info_v1 *ri, int64_t exit_status);
#endif
static void job_set_exception_port(job_t j, mach_port_t port);
static kern_return_t job_mig_spawn_internal(job_t j, vm_offset_t indata, mach_msg_type_number_t indataCnt, mach_port_t asport, job_t *outj);
static void job_open_shutdown_transaction(job_t ji);
static void job_close_shutdown_transaction(job_t ji);
static launch_data_t job_do_legacy_ipc_request(job_t j, launch_data_t request, mach_port_t asport);
static void job_setup_per_user_directory(job_t j, uid_t uid, const char *path);
static void job_setup_per_user_directories(job_t j, uid_t uid, const char *label);
static void job_update_jetsam_properties(job_t j, xpc_jetsam_band_t band, uint64_t user_data);
static void job_update_jetsam_memory_limit(job_t j, int32_t limit);

#if TARGET_OS_EMBEDDED
static bool job_import_defaults(launch_data_t pload);
#endif

static struct priority_properties_t {
	long long band;
	int priority;
} _launchd_priority_map[] = {
	{ XPC_JETSAM_BAND_SUSPENDED, JETSAM_PRIORITY_IDLE },
	{ XPC_JETSAM_BAND_BACKGROUND_OPPORTUNISTIC, JETSAM_PRIORITY_BACKGROUND_OPPORTUNISTIC },
	{ XPC_JETSAM_BAND_BACKGROUND, JETSAM_PRIORITY_BACKGROUND },
	{ XPC_JETSAM_BAND_MAIL, JETSAM_PRIORITY_MAIL },
	{ XPC_JETSAM_BAND_PHONE, JETSAM_PRIORITY_PHONE },
	{ XPC_JETSAM_BAND_UI_SUPPORT, JETSAM_PRIORITY_UI_SUPPORT },
	{ XPC_JETSAM_BAND_FOREGROUND_SUPPORT, JETSAM_PRIORITY_FOREGROUND_SUPPORT },
	{ XPC_JETSAM_BAND_FOREGROUND, JETSAM_PRIORITY_FOREGROUND },
	{ XPC_JETSAM_BAND_AUDIO, JETSAM_PRIORITY_AUDIO_AND_ACCESSORY },
	{ XPC_JETSAM_BAND_ACCESSORY, JETSAM_PRIORITY_AUDIO_AND_ACCESSORY },
	{ XPC_JETSAM_BAND_CRITICAL, JETSAM_PRIORITY_CRITICAL },
	{ XPC_JETSAM_BAND_TELEPHONY, JETSAM_PRIORITY_TELEPHONY },
};

static const struct {
	const char *key;
	int val;
} launchd_keys2limits[] = {
	{ LAUNCH_JOBKEY_RESOURCELIMIT_CORE, RLIMIT_CORE },
	{ LAUNCH_JOBKEY_RESOURCELIMIT_CPU, RLIMIT_CPU },
	{ LAUNCH_JOBKEY_RESOURCELIMIT_DATA, RLIMIT_DATA },
	{ LAUNCH_JOBKEY_RESOURCELIMIT_FSIZE, RLIMIT_FSIZE },
	{ LAUNCH_JOBKEY_RESOURCELIMIT_MEMLOCK, RLIMIT_MEMLOCK },
	{ LAUNCH_JOBKEY_RESOURCELIMIT_NOFILE, RLIMIT_NOFILE },
	{ LAUNCH_JOBKEY_RESOURCELIMIT_NPROC, RLIMIT_NPROC },
	{ LAUNCH_JOBKEY_RESOURCELIMIT_RSS, RLIMIT_RSS },
	{ LAUNCH_JOBKEY_RESOURCELIMIT_STACK, RLIMIT_STACK },
};

static time_t cronemu(int mon, int mday, int hour, int min);
static time_t cronemu_wday(int wday, int hour, int min);
static bool cronemu_mon(struct tm *wtm, int mon, int mday, int hour, int min);
static bool cronemu_mday(struct tm *wtm, int mday, int hour, int min);
static bool cronemu_hour(struct tm *wtm, int hour, int min);
static bool cronemu_min(struct tm *wtm, int min);

// miscellaneous file local functions
static size_t get_kern_max_proc(void);
static char **mach_cmd2argv(const char *string);
static size_t our_strhash(const char *s) __attribute__((pure));

void eliminate_double_reboot(void);

#pragma mark XPC Domain Forward Declarations
static job_t _xpc_domain_import_service(jobmgr_t jm, launch_data_t pload);
static int _xpc_domain_import_services(job_t j, launch_data_t services);

#pragma mark XPC Event Forward Declarations
static int xpc_event_find_channel(job_t j, const char *stream, struct machservice **ms);
static int xpc_event_get_event_name(job_t j, xpc_object_t request, xpc_object_t *reply);
static int xpc_event_set_event(job_t j, xpc_object_t request, xpc_object_t *reply);
static int xpc_event_copy_event(job_t j, xpc_object_t request, xpc_object_t *reply);
static int xpc_event_channel_check_in(job_t j, xpc_object_t request, xpc_object_t *reply);
static int xpc_event_channel_look_up(job_t j, xpc_object_t request, xpc_object_t *reply);
static int xpc_event_provider_check_in(job_t j, xpc_object_t request, xpc_object_t *reply);
static int xpc_event_provider_set_state(job_t j, xpc_object_t request, xpc_object_t *reply);

#pragma mark XPC Process Forward Declarations
static int xpc_process_set_jetsam_band(job_t j, xpc_object_t request, xpc_object_t *reply);
static int xpc_process_set_jetsam_memory_limit(job_t j, xpc_object_t request, xpc_object_t *reply);

// file local globals
static job_t _launchd_embedded_god = NULL;
static job_t _launchd_embedded_home = NULL;
static size_t total_children;
static size_t total_anon_children;
static mach_port_t the_exception_server;
static job_t workaround_5477111;
static LIST_HEAD(, job_s) s_needing_sessions;
static LIST_HEAD(, eventsystem) _s_event_systems;
static struct eventsystem *_launchd_support_system;
static job_t _launchd_event_monitor;
static job_t _launchd_xpc_bootstrapper;
static job_t _launchd_shutdown_monitor;

#if TARGET_OS_EMBEDDED
static xpc_object_t _launchd_defaults_cache;

mach_port_t launchd_audit_port = MACH_PORT_DEAD;
pid_t launchd_audit_session = 0;
#else
mach_port_t launchd_audit_port = MACH_PORT_NULL;
au_asid_t launchd_audit_session = AU_DEFAUDITSID;
#endif

static int s_no_hang_fd = -1;

// process wide globals
mach_port_t inherited_bootstrap_port;
jobmgr_t root_jobmgr;
bool launchd_shutdown_debugging = false;
bool launchd_verbose_boot = false;
bool launchd_embedded_handofgod = false;
bool launchd_runtime_busy_time = false;

void
job_ignore(job_t j)
{
	struct socketgroup *sg;
	struct machservice *ms;

	if (j->currently_ignored) {
		return;
	}

	job_log(j, LOG_DEBUG, "Ignoring...");

	j->currently_ignored = true;

	SLIST_FOREACH(sg, &j->sockets, sle) {
		socketgroup_ignore(j, sg);
	}

	SLIST_FOREACH(ms, &j->machservices, sle) {
		machservice_ignore(j, ms);
	}
}

void
job_watch(job_t j)
{
	struct socketgroup *sg;
	struct machservice *ms;

	if (!j->currently_ignored) {
		return;
	}

	job_log(j, LOG_DEBUG, "Watching...");

	j->currently_ignored = false;

	SLIST_FOREACH(sg, &j->sockets, sle) {
		socketgroup_watch(j, sg);
	}

	SLIST_FOREACH(ms, &j->machservices, sle) {
		machservice_watch(j, ms);
	}
}

void
job_stop(job_t j)
{
	int sig;

	if (unlikely(!j->p || j->stopped || j->anonymous)) {
		return;
	}

#if TARGET_OS_EMBEDDED
	if (launchd_embedded_handofgod && _launchd_embedded_god) {
		if (!_launchd_embedded_god->username || !j->username) {
			errno = EPERM;
			return;
		}

		if (strcmp(j->username, _launchd_embedded_god->username) != 0) {
			errno = EPERM;
			return;
		}
	} else if (launchd_embedded_handofgod) {
		errno = EINVAL;
		return;
	}
#endif

	j->sent_signal_time = runtime_get_opaque_time();

	job_log(j, LOG_DEBUG | LOG_CONSOLE, "Stopping job...");

	int error = -1;
	error = proc_terminate(j->p, &sig);
	if (error) {
		job_log(j, LOG_ERR | LOG_CONSOLE, "Could not terminate job: %d: %s", error, strerror(error));
		job_log(j, LOG_NOTICE | LOG_CONSOLE, "Using fallback option to terminate job...");
		error = kill2(j->p, SIGTERM);
		if (error) {
			job_log(j, LOG_ERR, "Could not signal job: %d: %s", error, strerror(error));
		} else {
			sig = SIGTERM;
		}
	}

	if (!error) {
		switch (sig) {
		case SIGKILL:
			j->sent_sigkill = true;
			j->clean_kill = true;

			/* We cannot effectively simulate an exit for jobs during the course
			 * of a normal run. Even if we pretend that the job exited, we will
			 * still not have gotten the receive rights associated with the
			 * job's MachServices back, so we cannot safely respawn it.
			 */
			if (j->mgr->shutting_down) {
				error = kevent_mod((uintptr_t)&j->exit_timeout, EVFILT_TIMER, EV_ADD|EV_ONESHOT, NOTE_SECONDS, LAUNCHD_SIGKILL_TIMER, j);
				(void)job_assumes_zero_p(j, error);
			}

			job_log(j, LOG_DEBUG | LOG_CONSOLE, "Sent job SIGKILL.");
			break;
		case SIGTERM:
			if (j->exit_timeout) {
				error = kevent_mod((uintptr_t)&j->exit_timeout, EVFILT_TIMER, EV_ADD|EV_ONESHOT, NOTE_SECONDS, j->exit_timeout, j);
				(void)job_assumes_zero_p(j, error);
			} else {
				job_log(j, LOG_NOTICE, "This job has an infinite exit timeout");
			}
			job_log(j, LOG_DEBUG, "Sent job SIGTERM.");
			break;
		default:
			job_log(j, LOG_ERR | LOG_CONSOLE, "Job was sent unexpected signal: %d: %s", sig, strsignal(sig));
			break;
		}
	}

	j->stopped = true;
}

launch_data_t
job_export(job_t j)
{
	launch_data_t tmp, tmp2, tmp3, r = launch_data_alloc(LAUNCH_DATA_DICTIONARY);

	if (r == NULL) {
		return NULL;
	}

	if ((tmp = launch_data_new_string(j->label))) {
		launch_data_dict_insert(r, tmp, LAUNCH_JOBKEY_LABEL);
	}
	if ((tmp = launch_data_new_string(j->mgr->name))) {
		launch_data_dict_insert(r, tmp, LAUNCH_JOBKEY_LIMITLOADTOSESSIONTYPE);
	}
	if ((tmp = launch_data_new_bool(j->ondemand))) {
		launch_data_dict_insert(r, tmp, LAUNCH_JOBKEY_ONDEMAND);
	}

	long long status = j->last_exit_status;
	if (j->fpfail) {
		status = LAUNCH_EXITSTATUS_FAIRPLAY_FAIL;
	}
	if ((tmp = launch_data_new_integer(status))) {
		launch_data_dict_insert(r, tmp, LAUNCH_JOBKEY_LASTEXITSTATUS);
	}

	if (j->p && (tmp = launch_data_new_integer(j->p))) {
		launch_data_dict_insert(r, tmp, LAUNCH_JOBKEY_PID);
	}
	if ((tmp = launch_data_new_integer(j->timeout))) {
		launch_data_dict_insert(r, tmp, LAUNCH_JOBKEY_TIMEOUT);
	}
	if (j->prog && (tmp = launch_data_new_string(j->prog))) {
		launch_data_dict_insert(r, tmp, LAUNCH_JOBKEY_PROGRAM);
	}
	if (j->stdinpath && (tmp = launch_data_new_string(j->stdinpath))) {
		launch_data_dict_insert(r, tmp, LAUNCH_JOBKEY_STANDARDINPATH);
	}
	if (j->stdoutpath && (tmp = launch_data_new_string(j->stdoutpath))) {
		launch_data_dict_insert(r, tmp, LAUNCH_JOBKEY_STANDARDOUTPATH);
	}
	if (j->stderrpath && (tmp = launch_data_new_string(j->stderrpath))) {
		launch_data_dict_insert(r, tmp, LAUNCH_JOBKEY_STANDARDERRORPATH);
	}
	if (likely(j->argv) && (tmp = launch_data_alloc(LAUNCH_DATA_ARRAY))) {
		size_t i;

		for (i = 0; i < j->argc; i++) {
			if ((tmp2 = launch_data_new_string(j->argv[i]))) {
				launch_data_array_set_index(tmp, tmp2, i);
			}
		}

		launch_data_dict_insert(r, tmp, LAUNCH_JOBKEY_PROGRAMARGUMENTS);
	}

	if (j->enable_transactions && (tmp = launch_data_new_bool(true))) {
		launch_data_dict_insert(r, tmp, LAUNCH_JOBKEY_ENABLETRANSACTIONS);
	}

	if (j->session_create && (tmp = launch_data_new_bool(true))) {
		launch_data_dict_insert(r, tmp, LAUNCH_JOBKEY_SESSIONCREATE);
	}

	if (j->inetcompat && (tmp = launch_data_alloc(LAUNCH_DATA_DICTIONARY))) {
		if ((tmp2 = launch_data_new_bool(j->inetcompat_wait))) {
			launch_data_dict_insert(tmp, tmp2, LAUNCH_JOBINETDCOMPATIBILITY_WAIT);
		}
		launch_data_dict_insert(r, tmp, LAUNCH_JOBKEY_INETDCOMPATIBILITY);
	}

	if (!SLIST_EMPTY(&j->sockets) && (tmp = launch_data_alloc(LAUNCH_DATA_DICTIONARY))) {
		struct socketgroup *sg;
		unsigned int i;

		SLIST_FOREACH(sg, &j->sockets, sle) {
			if ((tmp2 = launch_data_alloc(LAUNCH_DATA_ARRAY))) {
				for (i = 0; i < sg->fd_cnt; i++) {
					if ((tmp3 = launch_data_new_fd(sg->fds[i]))) {
						launch_data_array_set_index(tmp2, tmp3, i);
					}
				}
				launch_data_dict_insert(tmp, tmp2, sg->name);
			}
		}

		launch_data_dict_insert(r, tmp, LAUNCH_JOBKEY_SOCKETS);
	}

	if (!SLIST_EMPTY(&j->machservices) && (tmp = launch_data_alloc(LAUNCH_DATA_DICTIONARY))) {
		struct machservice *ms;

		tmp3 = NULL;

		SLIST_FOREACH(ms, &j->machservices, sle) {
			if (ms->per_pid) {
				if (tmp3 == NULL) {
					tmp3 = launch_data_alloc(LAUNCH_DATA_DICTIONARY);
				}
				if (tmp3) {
					tmp2 = launch_data_new_machport(MACH_PORT_NULL);
					launch_data_dict_insert(tmp3, tmp2, ms->name);
				}
			} else {
				tmp2 = launch_data_new_machport(MACH_PORT_NULL);
				launch_data_dict_insert(tmp, tmp2, ms->name);
			}
		}

		launch_data_dict_insert(r, tmp, LAUNCH_JOBKEY_MACHSERVICES);

		if (tmp3) {
			launch_data_dict_insert(r, tmp3, LAUNCH_JOBKEY_PERJOBMACHSERVICES);
		}
	}

	return r;
}

static void
jobmgr_log_active_jobs(jobmgr_t jm)
{
	const char *why_active;
	jobmgr_t jmi;
	job_t ji;

	SLIST_FOREACH(jmi, &jm->submgrs, sle) {
		jobmgr_log_active_jobs(jmi);
	}

	int level = LOG_DEBUG;
	if (pid1_magic) {
		level |= LOG_CONSOLE;
	}

	LIST_FOREACH(ji, &jm->jobs, sle) {
		if ((why_active = job_active(ji))) {
			if (ji->p != 1) {
				job_log(ji, level, "%s", why_active);

				uint32_t flags = 0;
				(void)proc_get_dirty(ji->p, &flags);
				if (!(flags & PROC_DIRTY_TRACKED)) {
					continue;
				}

				char *dirty = "clean";
				if (flags & PROC_DIRTY_IS_DIRTY) {
					dirty = "dirty";
				}

				char *idle_exit = "idle-exit unsupported";
				if (flags & PROC_DIRTY_ALLOWS_IDLE_EXIT) {
					idle_exit = "idle-exit supported";
				}

				job_log(ji, level, "Killability: %s/%s", dirty, idle_exit);
			}
		}
	}
}

static void
jobmgr_still_alive_with_check(jobmgr_t jm)
{
	int level = LOG_DEBUG;
	if (pid1_magic) {
		level |= LOG_CONSOLE;
	}

	jobmgr_log(jm, level, "Still alive with %lu/%lu (normal/anonymous) children.", total_children, total_anon_children);
	jobmgr_log_active_jobs(jm);
	launchd_log_push();
}

jobmgr_t
jobmgr_shutdown(jobmgr_t jm)
{
	jobmgr_t jmi, jmn;
	jobmgr_log(jm, LOG_DEBUG, "Beginning job manager shutdown with flags: %s", reboot_flags_to_C_names(jm->reboot_flags));

	jm->shutdown_time = runtime_get_wall_time() / USEC_PER_SEC;

	struct tm curtime;
	(void)localtime_r(&jm->shutdown_time, &curtime);

	char date[26];
	(void)asctime_r(&curtime, date);
	// Trim the new line that asctime_r(3) puts there for some reason.
	date[24] = 0;

	if (jm == root_jobmgr && pid1_magic) {
		jobmgr_log(jm, LOG_DEBUG | LOG_CONSOLE, "Userspace shutdown begun at: %s", date);
	} else {
		jobmgr_log(jm, LOG_DEBUG, "Job manager shutdown begun at: %s", date);
	}

	jm->shutting_down = true;

	SLIST_FOREACH_SAFE(jmi, &jm->submgrs, sle, jmn) {
		jobmgr_shutdown(jmi);
	}

	if (!jm->parentmgr) {
		if (pid1_magic) {
			// Spawn the shutdown monitor.
			if (_launchd_shutdown_monitor && !_launchd_shutdown_monitor->p) {
				job_log(_launchd_shutdown_monitor, LOG_NOTICE | LOG_CONSOLE, "Starting shutdown monitor.");
				job_dispatch(_launchd_shutdown_monitor, true);
			}
		}

		(void)jobmgr_assumes_zero_p(jm, kevent_mod((uintptr_t)jm, EVFILT_TIMER, EV_ADD, NOTE_SECONDS, 5, jm));
	}

	return jobmgr_do_garbage_collection(jm);
}

void
jobmgr_remove(jobmgr_t jm)
{
	jobmgr_t jmi;
	job_t ji;

	jobmgr_log(jm, LOG_DEBUG, "Removing job manager.");
	if (!SLIST_EMPTY(&jm->submgrs)) {
		size_t cnt = 0;
		while ((jmi = SLIST_FIRST(&jm->submgrs))) {
			jobmgr_remove(jmi);
			cnt++;
		}

		(void)jobmgr_assumes_zero(jm, cnt);
	}

	while ((ji = LIST_FIRST(&jm->jobs))) {
		if (!ji->anonymous && ji->p != 0) {
			job_log(ji, LOG_ERR, "Job is still active at job manager teardown.");
			ji->p = 0;
		}

		job_remove(ji);
	}

	struct waiting4attach *w4ai = NULL;
	while ((w4ai = LIST_FIRST(&jm->attaches))) {
		waiting4attach_delete(jm, w4ai);
	}

	if (jm->req_port) {
		(void)jobmgr_assumes_zero(jm, launchd_mport_deallocate(jm->req_port));
	}
	if (jm->jm_port) {
		(void)jobmgr_assumes_zero(jm, launchd_mport_close_recv(jm->jm_port));
	}

	if (jm->req_bsport) {
		(void)jobmgr_assumes_zero(jm, launchd_mport_deallocate(jm->req_bsport));
	}
	if (jm->req_excport) {
		(void)jobmgr_assumes_zero(jm, launchd_mport_deallocate(jm->req_excport));
	}
	if (MACH_PORT_VALID(jm->req_asport)) {
		(void)jobmgr_assumes_zero(jm, launchd_mport_deallocate(jm->req_asport));
	}
	if (jm->req_rport) {
		kern_return_t kr = xpc_call_wakeup(jm->req_rport, jm->error);
		if (!(kr == KERN_SUCCESS || kr == MACH_SEND_INVALID_DEST)) {
			/* If the originator went away, the reply port will be a dead name,
			 * and we expect this to fail.
			 */
			(void)jobmgr_assumes_zero(jm, kr);
		}
	}
	if (jm->req_ctx) {
		(void)jobmgr_assumes_zero(jm, vm_deallocate(mach_task_self(), jm->req_ctx, jm->req_ctx_sz));
	}

	time_t ts = runtime_get_wall_time() / USEC_PER_SEC;
	struct tm curtime;
	(void)localtime_r(&ts, &curtime);

	char date[26];
	(void)asctime_r(&curtime, date);
	date[24] = 0;

	time_t delta = ts - jm->shutdown_time;
	if (jm == root_jobmgr && pid1_magic) {
		jobmgr_log(jm, LOG_DEBUG | LOG_CONSOLE, "Userspace shutdown finished at: %s", date);
		jobmgr_log(jm, LOG_DEBUG | LOG_CONSOLE, "Userspace shutdown took approximately %ld second%s.", delta, (delta != 1) ? "s" : "");
	} else {
		jobmgr_log(jm, LOG_DEBUG, "Job manager shutdown finished at: %s", date);
		jobmgr_log(jm, LOG_DEBUG, "Job manager shutdown took approximately %ld second%s.", delta, (delta != 1) ? "s" : "");
	}

	if (jm->parentmgr) {
		runtime_del_weak_ref();
		SLIST_REMOVE(&jm->parentmgr->submgrs, jm, jobmgr_s, sle);

		// Hack for the guest user so that its stuff doesn't persist.
		//
		// <rdar://problem/14527875>
		if (strcmp(jm->name, VPROCMGR_SESSION_AQUA) == 0 && getuid() == 201) {
			raise(SIGTERM);
		}
	} else if (pid1_magic) {
		eliminate_double_reboot();
		launchd_log_vm_stats();
		jobmgr_log_stray_children(jm, true);
		jobmgr_log(root_jobmgr, LOG_NOTICE | LOG_CONSOLE, "About to call: reboot(%s).", reboot_flags_to_C_names(jm->reboot_flags));
		launchd_closelog();
		(void)jobmgr_assumes_zero_p(jm, reboot(jm->reboot_flags));
	} else {
		jobmgr_log(jm, LOG_DEBUG, "About to exit");
		launchd_closelog();
		exit(EXIT_SUCCESS);
	}

	free(jm);
}

void
job_remove(job_t j)
{
	struct waiting_for_removal *w4r;
	struct calendarinterval *ci;
	struct semaphoreitem *si;
	struct socketgroup *sg;
	struct machservice *ms;
	struct limititem *li;
	struct envitem *ei;

	if (j->alias) {
		/* HACK: Egregious code duplication. But as with machservice_delete(),
		 * job aliases can't (and shouldn't) have any complex behaviors 
		 * associated with them.
		 */
		while ((ms = SLIST_FIRST(&j->machservices))) {
			machservice_delete(j, ms, false);
		}

		LIST_REMOVE(j, sle);
		LIST_REMOVE(j, label_hash_sle);
		free(j);
		return;
	}

#if TARGET_OS_EMBEDDED
	if (launchd_embedded_handofgod && _launchd_embedded_god) {
		if (!(_launchd_embedded_god->username && j->username)) {
			errno = EPERM;
			return;
		}

		if (strcmp(j->username, _launchd_embedded_god->username) != 0) {
			errno = EPERM;
			return;
		}
	} else if (launchd_embedded_handofgod) {
		errno = EINVAL;
		return;
	}
#endif

	/* Do this BEFORE we check and see whether the job is still active. If we're
	 * a sub-job, we're being removed due to the parent job removing us.
	 * Therefore, the parent job will free itself after this call completes. So
	 * if we defer removing ourselves from the parent's list, we'll crash when
	 * we finally get around to it.
	 */
	if (j->dedicated_instance && !j->former_subjob) {
		LIST_REMOVE(j, subjob_sle);
		j->former_subjob = true;
	}

	if (unlikely(j->p)) {
		if (j->anonymous) {
			job_reap(j);
		} else {
			job_log(j, LOG_DEBUG, "Removal pended until the job exits");

			if (!j->removal_pending) {
				j->removal_pending = true;
				job_stop(j);
			}

			return;
		}
	}

	if (!j->removing) {
		j->removing = true;
		job_dispatch_curious_jobs(j);
	}

	ipc_close_all_with_job(j);

	if (j->forced_peers_to_demand_mode) {
		job_set_global_on_demand(j, false);
	}

	if (job_assumes_zero(j, j->fork_fd)) {
		(void)posix_assumes_zero(runtime_close(j->fork_fd));
	}

	if (j->stdin_fd) {
		(void)posix_assumes_zero(runtime_close(j->stdin_fd));
	}

	if (j->j_port) {
		(void)job_assumes_zero(j, launchd_mport_close_recv(j->j_port));
	}

	while ((sg = SLIST_FIRST(&j->sockets))) {
		socketgroup_delete(j, sg);
	}
	while ((ci = SLIST_FIRST(&j->cal_intervals))) {
		calendarinterval_delete(j, ci);
	}
	while ((ei = SLIST_FIRST(&j->env))) {
		envitem_delete(j, ei, false);
	}
	while ((ei = SLIST_FIRST(&j->global_env))) {
		envitem_delete(j, ei, true);
	}
	while ((li = SLIST_FIRST(&j->limits))) {
		limititem_delete(j, li);
	}
	while ((ms = SLIST_FIRST(&j->machservices))) {
		machservice_delete(j, ms, false);
	}
	while ((si = SLIST_FIRST(&j->semaphores))) {
		semaphoreitem_delete(j, si);
	}
	while ((w4r = SLIST_FIRST(&j->removal_watchers))) {
		waiting4removal_delete(j, w4r);
	}

	struct externalevent *eei = NULL;
	while ((eei = LIST_FIRST(&j->events))) {
		externalevent_delete(eei);
	}

	if (j->event_monitor) {
		_launchd_event_monitor = NULL;
	}
	if (j->xpc_bootstrapper) {
		_launchd_xpc_bootstrapper = NULL;
	}

	if (j->prog) {
		free(j->prog);
	}
	if (j->argv) {
		free(j->argv);
	}
	if (j->rootdir) {
		free(j->rootdir);
	}
	if (j->workingdir) {
		free(j->workingdir);
	}
	if (j->username) {
		free(j->username);
	}
	if (j->groupname) {
		free(j->groupname);
	}
	if (j->stdinpath) {
		free(j->stdinpath);
	}
	if (j->stdoutpath) {
		free(j->stdoutpath);
	}
	if (j->stderrpath) {
		free(j->stderrpath);
	}
	if (j->alt_exc_handler) {
		free(j->alt_exc_handler);
	}
	if (j->cfbundleidentifier) {
		free(j->cfbundleidentifier);
	}
#if HAVE_SANDBOX
	if (j->seatbelt_profile) {
		free(j->seatbelt_profile);
	}
	if (j->container_identifier) {
		free(j->container_identifier);
	}
#endif
#if HAVE_QUARANTINE
	if (j->quarantine_data) {
		free(j->quarantine_data);
	}
#endif
	if (j->j_binpref) {
		free(j->j_binpref);
	}
	if (j->start_interval) {
		runtime_del_weak_ref();
		(void)job_assumes_zero_p(j, kevent_mod((uintptr_t)&j->start_interval, EVFILT_TIMER, EV_DELETE, 0, 0, NULL));
	}
	if (j->exit_timeout) {
		/* If this fails, it just means the timer's already fired, so no need to
		 * wrap it in an assumes() macro.
		 */
		(void)kevent_mod((uintptr_t)&j->exit_timeout, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
	}
	if (j->asport != MACH_PORT_NULL) {
		(void)job_assumes_zero(j, launchd_mport_deallocate(j->asport));
	}
	if (!uuid_is_null(j->expected_audit_uuid)) {
		LIST_REMOVE(j, needing_session_sle);
	}
	if (j->embedded_god) {
		_launchd_embedded_god = NULL;
	}
	if (j->embedded_home) {
		_launchd_embedded_home = NULL;
	}
	if (j->shutdown_monitor) {
		_launchd_shutdown_monitor = NULL;
	}

	(void)kevent_mod((uintptr_t)j, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);

	LIST_REMOVE(j, sle);
	LIST_REMOVE(j, label_hash_sle);

	job_t ji = NULL;
	job_t jit = NULL;
	LIST_FOREACH_SAFE(ji, &j->subjobs, subjob_sle, jit) {
		job_remove(ji);
	}

	job_log(j, LOG_DEBUG, "Removed");

	j->kqjob_callback = (kq_callback)0x8badf00d;
	free(j);
}

void
socketgroup_setup(launch_data_t obj, const char *key, void *context)
{
	launch_data_t tmp_oai;
	job_t j = context;
	size_t i, fd_cnt = 1;
	int *fds;

	if (launch_data_get_type(obj) == LAUNCH_DATA_ARRAY) {
		fd_cnt = launch_data_array_get_count(obj);
	}

	fds = alloca(fd_cnt * sizeof(int));

	for (i = 0; i < fd_cnt; i++) {
		if (launch_data_get_type(obj) == LAUNCH_DATA_ARRAY) {
			tmp_oai = launch_data_array_get_index(obj, i);
		} else {
			tmp_oai = obj;
		}

		fds[i] = launch_data_get_fd(tmp_oai);
	}

	socketgroup_new(j, key, fds, fd_cnt);

	ipc_revoke_fds(obj);
}

bool
job_set_global_on_demand(job_t j, bool val)
{
	if (j->forced_peers_to_demand_mode && val) {
		return false;
	} else if (!j->forced_peers_to_demand_mode && !val) {
		return false;
	}

	if ((j->forced_peers_to_demand_mode = val)) {
		j->mgr->global_on_demand_cnt++;
	} else {
		j->mgr->global_on_demand_cnt--;
	}

	if (j->mgr->global_on_demand_cnt == 0) {
		jobmgr_dispatch_all(j->mgr, false);
	}

	return true;
}

bool
job_setup_machport(job_t j)
{
	if (job_assumes_zero(j, launchd_mport_create_recv(&j->j_port)) != KERN_SUCCESS) {
		goto out_bad;
	}

	if (job_assumes_zero(j, runtime_add_mport(j->j_port, job_server)) != KERN_SUCCESS) {
		goto out_bad2;
	}

	if (job_assumes_zero(j, launchd_mport_notify_req(j->j_port, MACH_NOTIFY_NO_SENDERS)) != KERN_SUCCESS) {
		(void)job_assumes_zero(j, launchd_mport_close_recv(j->j_port));
		goto out_bad;
	}

	return true;
out_bad2:
	(void)job_assumes_zero(j, launchd_mport_close_recv(j->j_port));
out_bad:
	return false;
}

kern_return_t
job_setup_exit_port(job_t j)
{
	kern_return_t kr = launchd_mport_create_recv(&j->exit_status_port);
	if (job_assumes_zero(j, kr) != KERN_SUCCESS) {
		return MACH_PORT_NULL;
	}

	struct mach_port_limits limits = {
		.mpl_qlimit = 1,
	};
	kr = mach_port_set_attributes(mach_task_self(), j->exit_status_port, MACH_PORT_LIMITS_INFO, (mach_port_info_t)&limits, sizeof(limits));
	(void)job_assumes_zero(j, kr);

	kr = launchd_mport_make_send_once(j->exit_status_port, &j->exit_status_dest);
	if (job_assumes_zero(j, kr) != KERN_SUCCESS) {
		(void)job_assumes_zero(j, launchd_mport_close_recv(j->exit_status_port));
		j->exit_status_port = MACH_PORT_NULL;
	}

	return kr;
}

job_t 
job_new_via_mach_init(job_t j, const char *cmd, uid_t uid, bool ond)
{
	const char **argv = (const char **)mach_cmd2argv(cmd);
	job_t jr = NULL;

	if (!argv) {
		goto out_bad;
	}

	jr = job_new(j->mgr, AUTO_PICK_LEGACY_LABEL, NULL, argv);
	free(argv);

	// Job creation can be denied during shutdown.
	if (unlikely(jr == NULL)) {
		goto out_bad;
	}

	jr->mach_uid = uid;
	jr->ondemand = ond;
	jr->legacy_mach_job = true;
	jr->abandon_pg = true;
	jr->priv_port_has_senders = true; // the IPC that called us will make-send on this port

	if (!job_setup_machport(jr)) {
		goto out_bad;
	}

	job_log(jr, LOG_INFO, "Legacy%s server created", ond ? " on-demand" : "");

	return jr;

out_bad:
	if (jr) {
		job_remove(jr);
	}
	return NULL;
}

job_t
job_new_anonymous(jobmgr_t jm, pid_t anonpid)
{
	struct proc_bsdshortinfo proc;
	bool shutdown_state;
	job_t jp = NULL, jr = NULL;
	uid_t kp_euid, kp_uid, kp_svuid;
	gid_t kp_egid, kp_gid, kp_svgid;

	if (anonpid == 0) {
		errno = EINVAL;
		return NULL;
	}

	if (anonpid >= 100000) {
		/* The kernel current defines PID_MAX to be 99999, but that define isn't
		 * exported.
		 */
		launchd_syslog(LOG_WARNING, "Did PID_MAX change? Got request from PID: %d", anonpid);
		errno = EINVAL;
		return NULL;
	}

	/* libproc returns the number of bytes written into the buffer upon success,
	 * zero on failure. I'd much rather it return -1 on failure, like sysctl(3).
	 */
	if (proc_pidinfo(anonpid, PROC_PIDT_SHORTBSDINFO, 1, &proc, PROC_PIDT_SHORTBSDINFO_SIZE) == 0) {
		if (errno != ESRCH) {
			(void)jobmgr_assumes_zero(jm, errno);
		}
		return NULL;
	}

	if (proc.pbsi_comm[0] == '\0') {
		launchd_syslog(LOG_WARNING, "Blank command for PID: %d", anonpid);
		errno = EINVAL;
		return NULL;
	}

	if (unlikely(proc.pbsi_status == SZOMB)) {
		jobmgr_log(jm, LOG_DEBUG, "Tried to create an anonymous job for zombie PID %u: %s", anonpid, proc.pbsi_comm);
	}

	if (unlikely(proc.pbsi_flags & P_SUGID)) {
		jobmgr_log(jm, LOG_DEBUG, "Inconsistency: P_SUGID is set on PID %u: %s", anonpid, proc.pbsi_comm);
	}

	kp_euid = proc.pbsi_uid;
	kp_uid = proc.pbsi_ruid;
	kp_svuid = proc.pbsi_svuid;
	kp_egid = proc.pbsi_gid;
	kp_gid = proc.pbsi_rgid;
	kp_svgid = proc.pbsi_svgid;

	if (unlikely(kp_euid != kp_uid || kp_euid != kp_svuid || kp_uid != kp_svuid || kp_egid != kp_gid || kp_egid != kp_svgid || kp_gid != kp_svgid)) {
		jobmgr_log(jm, LOG_DEBUG, "Inconsistency: Mixed credentials (e/r/s UID %u/%u/%u GID %u/%u/%u) detected on PID %u: %s",
				kp_euid, kp_uid, kp_svuid, kp_egid, kp_gid, kp_svgid, anonpid, proc.pbsi_comm);
	}

	/* "Fix" for when the kernel turns the process tree into a weird, cyclic
	 * graph.
	 *
	 * See <rdar://problem/7264615> for the symptom and <rdar://problem/5020256>
	 * as to why this can happen.
	 */
	if ((pid_t)proc.pbsi_ppid == anonpid) {
		jobmgr_log(jm, LOG_WARNING, "Process has become its own parent through ptrace(3). Ignoring: %s", proc.pbsi_comm);
		errno = EINVAL;
		return NULL;
	}

	/* HACK: Normally, job_new() returns an error during shutdown, but anonymous
	 * jobs can pop up during shutdown and need to talk to us.
	 */
	if (unlikely(shutdown_state = jm->shutting_down)) {
		jm->shutting_down = false;
	}

	// We only set requestor_pid for XPC domains.
	const char *whichlabel = (jm->req_pid == anonpid) ? AUTO_PICK_XPC_LABEL : AUTO_PICK_ANONYMOUS_LABEL;
	if ((jr = job_new(jm, whichlabel, proc.pbsi_comm, NULL))) {
		u_int proc_fflags = NOTE_EXEC|NOTE_FORK|NOTE_EXIT;

		total_anon_children++;
		jr->anonymous = true;
		jr->p = anonpid;

		// Anonymous process reaping is messy.
		LIST_INSERT_HEAD(&jm->active_jobs[ACTIVE_JOB_HASH(jr->p)], jr, pid_hash_sle);

		if (unlikely(kevent_mod(jr->p, EVFILT_PROC, EV_ADD, proc_fflags, 0, root_jobmgr) == -1)) {
			if (errno != ESRCH) {
				(void)job_assumes_zero(jr, errno);
			}

			// Zombies interact weirdly with kevent(3).
			job_log(jr, LOG_ERR, "Failed to add kevent for PID %u. Will unload at MIG return", jr->p);
			jr->unload_at_mig_return = true;
		}

		if (unlikely(shutdown_state)) {
			job_log(jr, LOG_APPLEONLY, "This process showed up to the party while all the guests were leaving. Odds are that it will have a miserable time.");
		}

		job_log(jr, LOG_DEBUG, "Created PID %u anonymously by PPID %u%s%s", anonpid, proc.pbsi_ppid, jp ? ": " : "", jp ? jp->label : "");
	} else {
		(void)os_assumes_zero(errno);
	}

	// Undo our hack from above.
	if (unlikely(shutdown_state)) {
		jm->shutting_down = true;
	}

	/* This is down here to prevent infinite recursion due to a process
	 * attaching to its parent through ptrace(3) -- causing a cycle in the
	 * process tree and thereby not making it a tree anymore. We need to make
	 * sure that the anonymous job has been added to the process list so that
	 * we'll find the tracing parent PID of the parent process, which is the
	 * child, when we go looking for it in jobmgr_find_by_pid().
	 *
	 * <rdar://problem/7264615>
	 */
	switch (proc.pbsi_ppid) {
	case 0:
		// The kernel.
		break;
	case 1:
		if (!pid1_magic) {
			break;
		}
		// Fall through.
	default:
		jp = jobmgr_find_by_pid(jm, proc.pbsi_ppid, true);
		if (jobmgr_assumes(jm, jp != NULL)) {
			if (jp && !jp->anonymous && unlikely(!(proc.pbsi_flags & P_EXEC))) {
				job_log(jp, LOG_DEBUG, "Called *fork(). Please switch to posix_spawn*(), pthreads or launchd. Child PID %u", proc.pbsi_pid);
			}
		}
		break;
	}

	return jr;
}

job_t 
job_new_subjob(job_t j, uuid_t identifier)
{
	char label[0];
	uuid_string_t idstr;
	uuid_unparse(identifier, idstr);
	size_t label_sz = snprintf(label, 0, "%s.%s", j->label, idstr);

	job_t nj = (struct job_s *)calloc(1, sizeof(struct job_s) + label_sz + 1);
	if (nj != NULL) {
		nj->kqjob_callback = job_callback;
		nj->original = j;
		nj->mgr = j->mgr;
		nj->min_run_time = j->min_run_time;
		nj->timeout = j->timeout;
		nj->exit_timeout = j->exit_timeout;

		snprintf((char *)nj->label, label_sz + 1, "%s.%s", j->label, idstr);

		// Set all our simple Booleans that are applicable.
		nj->debug = j->debug;
		nj->ondemand = j->ondemand;
		nj->checkedin = true;
		nj->low_pri_io = j->low_pri_io;
		nj->setmask = j->setmask;
		nj->wait4debugger = j->wait4debugger;
		nj->internal_exc_handler = j->internal_exc_handler;
		nj->setnice = j->setnice;
		nj->abandon_pg = j->abandon_pg;
		nj->ignore_pg_at_shutdown = j->ignore_pg_at_shutdown;
		nj->deny_job_creation = j->deny_job_creation;
		nj->enable_transactions = j->enable_transactions;
		nj->needs_kickoff = j->needs_kickoff;
		nj->currently_ignored = true;
		nj->dedicated_instance = true;
		nj->xpc_service = j->xpc_service;
		nj->xpc_bootstrapper = j->xpc_bootstrapper;
		nj->jetsam_priority = j->jetsam_priority;
		nj->jetsam_memlimit = j->jetsam_memlimit;
		nj->psproctype = j->psproctype;

		nj->mask = j->mask;
		uuid_copy(nj->instance_id, identifier);

		// These jobs are purely on-demand Mach jobs.
		// {Hard | Soft}ResourceLimits are not supported.
		// JetsamPriority is not supported.

		if (j->prog) {
			nj->prog = strdup(j->prog);
		}
		if (j->argv) {
			size_t sz = malloc_size(j->argv);
			nj->argv = (char **)malloc(sz);
			if (nj->argv != NULL) {
				// This is the start of our strings.
				char *p = ((char *)nj->argv) + ((j->argc + 1) * sizeof(char *));

				size_t i = 0;
				for (i = 0; i < j->argc; i++) {
					(void)strcpy(p, j->argv[i]);
					nj->argv[i] = p;
					p += (strlen(j->argv[i]) + 1);
				}
				nj->argv[i] = NULL;
			} else {
				(void)job_assumes_zero(nj, errno);
			}

			nj->argc = j->argc;
		}

		struct machservice *msi = NULL;
		SLIST_FOREACH(msi, &j->machservices, sle) {
			/* Only copy MachServices that were actually declared in the plist.
			 * So skip over per-PID ones and ones that were created via
			 * bootstrap_register().
			 */
			if (msi->upfront) {
				mach_port_t mp = MACH_PORT_NULL;
				struct machservice *msj = machservice_new(nj, msi->name, &mp, false);
				if (msj != NULL) {
					msj->reset = msi->reset;
					msj->delete_on_destruction = msi->delete_on_destruction;
					msj->drain_one_on_crash = msi->drain_one_on_crash;
					msj->drain_all_on_crash = msi->drain_all_on_crash;

					kern_return_t kr = mach_port_set_attributes(mach_task_self(), msj->port, MACH_PORT_TEMPOWNER, NULL, 0);
					(void)job_assumes_zero(j, kr);
				} else {
					(void)job_assumes_zero(nj, errno);
				}
			}
		}

		// We ignore global environment variables.
		struct envitem *ei = NULL;
		SLIST_FOREACH(ei, &j->env, sle) {
			if (envitem_new(nj, ei->key, ei->value, false)) {
				(void)job_assumes_zero(nj, errno);
			}
		}
		uuid_string_t val;
		uuid_unparse(identifier, val);
		if (envitem_new(nj, LAUNCH_ENV_INSTANCEID, val, false)) {
			(void)job_assumes_zero(nj, errno);
		}

		if (j->rootdir) {
			nj->rootdir = strdup(j->rootdir);
		}
		if (j->workingdir) {
			nj->workingdir = strdup(j->workingdir);
		}
		if (j->username) {
			nj->username = strdup(j->username);
		}
		if (j->groupname) {
			nj->groupname = strdup(j->groupname);
		}

		/* FIXME: We shouldn't redirect all the output from these jobs to the
		 * same file. We should uniquify the file names. But this hasn't shown
		 * to be a problem in practice.
		 */
		if (j->stdinpath) {
			nj->stdinpath = strdup(j->stdinpath);
		}
		if (j->stdoutpath) {
			nj->stdoutpath = strdup(j->stdinpath);
		}
		if (j->stderrpath) {
			nj->stderrpath = strdup(j->stderrpath);
		}
		if (j->alt_exc_handler) {
			nj->alt_exc_handler = strdup(j->alt_exc_handler);
		}
		if (j->cfbundleidentifier) {
			nj->cfbundleidentifier = strdup(j->cfbundleidentifier);
		}
#if HAVE_SANDBOX
		if (j->seatbelt_profile) {
			nj->seatbelt_profile = strdup(j->seatbelt_profile);
		}
		if (j->container_identifier) {
			nj->container_identifier = strdup(j->container_identifier);
		}
#endif

#if HAVE_QUARANTINE
		if (j->quarantine_data) {
			nj->quarantine_data = strdup(j->quarantine_data);
		}
		nj->quarantine_data_sz = j->quarantine_data_sz;
#endif
		if (j->j_binpref) {
			size_t sz = malloc_size(j->j_binpref);
			nj->j_binpref = (cpu_type_t *)malloc(sz);
			if (nj->j_binpref) {
				memcpy(&nj->j_binpref, &j->j_binpref, sz);
			} else {
				(void)job_assumes_zero(nj, errno);
			}
		}

		if (j->asport != MACH_PORT_NULL) {
			(void)job_assumes_zero(nj, launchd_mport_copy_send(j->asport));
			nj->asport = j->asport;
		}

		LIST_INSERT_HEAD(&nj->mgr->jobs, nj, sle);

		jobmgr_t where2put = root_jobmgr;
		if (j->mgr->properties & BOOTSTRAP_PROPERTY_XPC_DOMAIN) {
			where2put = j->mgr;
		}
		LIST_INSERT_HEAD(&where2put->label_hash[hash_label(nj->label)], nj, label_hash_sle);
		LIST_INSERT_HEAD(&j->subjobs, nj, subjob_sle);
	} else {
		(void)os_assumes_zero(errno);
	}

	return nj;
}

job_t 
job_new(jobmgr_t jm, const char *label, const char *prog, const char *const *argv)
{
	const char *const *argv_tmp = argv;
	char tmp_path[PATH_MAX];
	char auto_label[1000];
	const char *bn = NULL;
	char *co;
	size_t minlabel_len;
	size_t i, cc = 0;
	job_t j;

	__OS_COMPILETIME_ASSERT__(offsetof(struct job_s, kqjob_callback) == 0);

	if (unlikely(jm->shutting_down)) {
		errno = EINVAL;
		return NULL;
	}

	if (unlikely(prog == NULL && argv == NULL)) {
		errno = EINVAL;
		return NULL;
	}

	/* I'd really like to redo this someday. Anonymous jobs carry all the
	 * baggage of managed jobs with them, even though most of it is unused.
	 * Maybe when we have Objective-C objects in libSystem, there can be a base
	 * job type that anonymous and managed jobs inherit from...
	 */
	char *anon_or_legacy = (label == AUTO_PICK_ANONYMOUS_LABEL) ? "anonymous" : "mach_init";
	if (unlikely(label == AUTO_PICK_LEGACY_LABEL || label == AUTO_PICK_ANONYMOUS_LABEL)) {
		if (prog) {
			bn = prog;
		} else {
			strlcpy(tmp_path, argv[0], sizeof(tmp_path));
			// prog for auto labels is kp.kp_kproc.p_comm.
			bn = basename(tmp_path);
		}

		(void)snprintf(auto_label, sizeof(auto_label), "%s.%s.%s", sizeof(void *) == 8 ? "0xdeadbeeffeedface" : "0xbabecafe", anon_or_legacy, bn);
		label = auto_label;
		/* This is so we can do gross things later. See NOTE_EXEC for anonymous
		 * jobs.
		 */
		minlabel_len = strlen(label) + MAXCOMLEN;
	} else {
		if (label == AUTO_PICK_XPC_LABEL) {
			minlabel_len = snprintf(auto_label, sizeof(auto_label), "com.apple.xpc.domain-owner.%s", jm->owner);
		} else {
			minlabel_len = strlen(label);
		}
	}

	j = calloc(1, sizeof(struct job_s) + minlabel_len + 1);

	if (!j) {
		(void)os_assumes_zero(errno);
		return NULL;
	}

	if (unlikely(label == auto_label)) {
		(void)snprintf((char *)j->label, strlen(label) + 1, "%p.%s.%s", j, anon_or_legacy, bn);
	} else {
		(void)strcpy((char *)j->label, (label == AUTO_PICK_XPC_LABEL) ? auto_label : label);
	}

	j->kqjob_callback = job_callback;
	j->mgr = jm;
	j->min_run_time = LAUNCHD_MIN_JOB_RUN_TIME;
	j->timeout = RUNTIME_ADVISABLE_IDLE_TIMEOUT;
	j->exit_timeout = LAUNCHD_DEFAULT_EXIT_TIMEOUT;
	j->currently_ignored = true;
	j->ondemand = true;
	j->checkedin = true;
	j->jetsam_priority = DEFAULT_JETSAM_PRIORITY;
	j->jetsam_memlimit = -1;
	uuid_clear(j->expected_audit_uuid);
#if TARGET_OS_EMBEDDED
	/* Run embedded daemons as background by default. SpringBoard jobs are
	 * Interactive by default. Unfortunately, so many daemons have opted into
	 * this priority band that its usefulness is highly questionable.
	 * 
	 * See <rdar://problem/9539873>.
	 *
	 * Also ensure that daemons have a default memory highwatermark unless
	 * otherwise specified, as per <rdar://problem/10307814>.
	 */
	if (launchd_embedded_handofgod) {
		j->psproctype = POSIX_SPAWN_PROC_TYPE_APP_DEFAULT;
		j->app = true;
	} else {
		j->psproctype = POSIX_SPAWN_PROC_TYPE_DAEMON_BACKGROUND;
		j->jetsam_memlimit = DEFAULT_JETSAM_DAEMON_HIGHWATERMARK;
	}
#else
	/* Jobs on OS X that just come from disk are "standard" by default so that
	 * third-party daemons/agents don't encounter unexpected throttling.
	 */
	j->psproctype = POSIX_SPAWN_PROC_TYPE_DAEMON_STANDARD;
#endif

	if (prog) {
		j->prog = strdup(prog);
		if (!j->prog) {
			(void)os_assumes_zero(errno);
			goto out_bad;
		}
	}

	if (likely(argv)) {
		while (*argv_tmp++) {
			j->argc++;
		}

		for (i = 0; i < j->argc; i++) {
			cc += strlen(argv[i]) + 1;
		}

		j->argv = malloc((j->argc + 1) * sizeof(char *) + cc);
		if (!j->argv) {
			(void)job_assumes_zero(j, errno);
			goto out_bad;
		}

		co = ((char *)j->argv) + ((j->argc + 1) * sizeof(char *));

		for (i = 0; i < j->argc; i++) {
			j->argv[i] = co;
			(void)strcpy(co, argv[i]);
			co += strlen(argv[i]) + 1;
		}
		j->argv[i] = NULL;
	}

	// Sssshhh... don't tell anyone.
	if (strcmp(j->label, "com.apple.WindowServer") == 0) {
		j->has_console = true;
	}

	LIST_INSERT_HEAD(&jm->jobs, j, sle);

	jobmgr_t where2put_label = root_jobmgr;
	if (j->mgr->properties & BOOTSTRAP_PROPERTY_XPC_DOMAIN) {
		where2put_label = j->mgr;
	}
	LIST_INSERT_HEAD(&where2put_label->label_hash[hash_label(j->label)], j, label_hash_sle);
	uuid_clear(j->expected_audit_uuid);

	job_log(j, LOG_DEBUG, "Conceived");

	return j;

out_bad:
	if (j->prog) {
		free(j->prog);
	}
	free(j);

	return NULL;
}

job_t
job_new_alias(jobmgr_t jm, job_t src)
{
	if (job_find(jm, src->label)) {
		errno = EEXIST;
		return NULL;
	}

	job_t j = calloc(1, sizeof(struct job_s) + strlen(src->label) + 1);
	if (!j) {
		(void)os_assumes_zero(errno);
		return NULL;
	}

	(void)strcpy((char *)j->label, src->label);
	LIST_INSERT_HEAD(&jm->jobs, j, sle);
	LIST_INSERT_HEAD(&jm->label_hash[hash_label(j->label)], j, label_hash_sle);
	/* Bad jump address. The kqueue callback for aliases should never be
	 * invoked.
	 */
	j->kqjob_callback = (kq_callback)0xfa1afe1;
	j->alias = src;
	j->mgr = jm;

	struct machservice *msi = NULL;
	SLIST_FOREACH(msi, &src->machservices, sle) {
		if (!machservice_new_alias(j, msi)) {
			jobmgr_log(jm, LOG_ERR, "Failed to alias job: %s", src->label);
			errno = EINVAL;
			job_remove(j);
			j = NULL;
			break;
		}
	}

	if (j) {
		job_log(j, LOG_DEBUG, "Aliased service into domain: %s", jm->name);
	}

	return j;
}

job_t 
job_import(launch_data_t pload)
{
#if TARGET_OS_EMBEDDED
	/* If this is the special payload of default values, handle it here */
	if (unlikely(launch_data_dict_lookup(pload, LAUNCH_JOBKEY_DEFAULTS))) {
		job_import_defaults(pload);
		return NULL;
	}
#endif    
    
	job_t j = jobmgr_import2(root_jobmgr, pload);

	if (unlikely(j == NULL)) {
		return NULL;
	}

	/* Since jobs are effectively stalled until they get security sessions
	 * assigned to them, we may wish to reconsider this behavior of calling the
	 * job "enabled" as far as other jobs with the OtherJobEnabled KeepAlive
	 * criterion set.
	 */
	job_dispatch_curious_jobs(j);
	return job_dispatch(j, false);
}

#if TARGET_OS_EMBEDDED

bool
job_import_defaults(launch_data_t pload)
{
	bool result = false;
	xpc_object_t xd = NULL, defaults;
   
	if (_launchd_defaults_cache) {
		xpc_release(_launchd_defaults_cache);
		_launchd_defaults_cache = NULL;
	}

	xd = ld2xpc(pload);
	if (!xd || xpc_get_type(xd) != XPC_TYPE_DICTIONARY) {
		goto out;
	}

	defaults = xpc_dictionary_get_value(xd, LAUNCHD_JOB_DEFAULTS);
	if (!defaults || xpc_get_type(defaults) != XPC_TYPE_DICTIONARY) {
		goto out;
	}

	_launchd_defaults_cache = xpc_copy(defaults);
	result = true;
out:
	if (xd) {
		xpc_release(xd);
	}

	return result;
}

bool
job_apply_defaults(job_t j) {
	const char *test_prefix = "com.apple.test.";
    
	char *sb_prefix_end, *sb_suffix_start;
	char true_job_label[strlen(j->label)];
	const char *label;

	if (((sb_prefix_end = strchr(j->label, ':')) != NULL) &&
	    ((sb_suffix_start = strchr(sb_prefix_end + 1, '[')) != NULL)) {
 		/*
 		 * Workaround 'UIKitApplication:com.apple.foo[bar]' convention for the processes
 		 * we're interested in. To be removed when <rdar://problem/13066361> is addressed.
 		 */
		snprintf(true_job_label, sb_suffix_start - sb_prefix_end, "%s", sb_prefix_end + 1);
 		label = true_job_label;
	} else {
		/* Just test the standard label */
		label = j->label;
	}
    
	/* Test for cache presence and apply if found */
	if (_launchd_defaults_cache) { 
		xpc_object_t props = xpc_dictionary_get_value(_launchd_defaults_cache, label);
		if (props && xpc_get_type(props) == XPC_TYPE_DICTIONARY) {
			launch_data_t lv = xpc2ld(props);
			launch_data_dict_iterate(lv, job_import_keys, j);
			launch_data_free(lv);
			return true;
		}
	}
    
	/* Limit free? Disable the memory limit if this is a test job; see <rdar://problem/13180697> */
	if (!strncmp(label, test_prefix, strlen(test_prefix))) {
		j->jetsam_memlimit = -1;
		return true;
	}
    
	return false;
}

#endif

launch_data_t
job_import_bulk(launch_data_t pload)
{
	launch_data_t resp = launch_data_alloc(LAUNCH_DATA_ARRAY);
	job_t *ja;
	size_t i, c = launch_data_array_get_count(pload);

	ja = alloca(c * sizeof(job_t));

	for (i = 0; i < c; i++) {
		if ((likely(ja[i] = jobmgr_import2(root_jobmgr, launch_data_array_get_index(pload, i)))) && errno != ENEEDAUTH) {
			errno = 0;
		}
		launch_data_array_set_index(resp, launch_data_new_errno(errno), i);
	}

	for (i = 0; i < c; i++) {
		if (likely(ja[i])) {
			job_dispatch_curious_jobs(ja[i]);
			job_dispatch(ja[i], false);
		}
	}

	return resp;
}

void
job_import_bool(job_t j, const char *key, bool value)
{
	bool found_key = false;

	switch (key[0]) {
	case 'a':
	case 'A':
		if (strcasecmp(key, LAUNCH_JOBKEY_ABANDONPROCESSGROUP) == 0) {
			j->abandon_pg = value;
			found_key = true;
		}
		break;
	case 'b':
	case 'B':
		if (strcasecmp(key, LAUNCH_JOBKEY_BEGINTRANSACTIONATSHUTDOWN) == 0) {
			j->dirty_at_shutdown = value;
			found_key = true;
		}
		break;
	case 'j':
	case 'J':
		if (strcasecmp(key, LAUNCH_JOBKEY_JOINGUISESSION) == 0) {
			j->joins_gui_session = value;
			found_key = true;
		}
		break;
	case 'k':
	case 'K':
		if (strcasecmp(key, LAUNCH_JOBKEY_KEEPALIVE) == 0) {
			j->ondemand = !value;
			found_key = true;
		}
		break;
	case 'o':
	case 'O':
		if (strcasecmp(key, LAUNCH_JOBKEY_ONDEMAND) == 0) {
			j->ondemand = value;
			found_key = true;
		}
		break;
	case 'd':
	case 'D':
		if (strcasecmp(key, LAUNCH_JOBKEY_DEBUG) == 0) {
			j->debug = value;
			found_key = true;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_DISABLED) == 0) {
			(void)job_assumes(j, !value);
			found_key = true;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_DISABLEASLR) == 0) {
			j->disable_aslr = value;
			found_key = true;
		}
		break;
	case 'h':
	case 'H':
		if (strcasecmp(key, LAUNCH_JOBKEY_HOPEFULLYEXITSLAST) == 0) {
			job_log(j, LOG_PERF, "%s has been deprecated. Please use the new %s key instead and add EnableTransactions to your launchd.plist.", LAUNCH_JOBKEY_HOPEFULLYEXITSLAST, LAUNCH_JOBKEY_BEGINTRANSACTIONATSHUTDOWN);
			j->dirty_at_shutdown = value;
			found_key = true;
		}
		break;
	case 's':
	case 'S':
		if (strcasecmp(key, LAUNCH_JOBKEY_SESSIONCREATE) == 0) {
			j->session_create = value;
			found_key = true;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_STARTONMOUNT) == 0) {
			j->start_on_mount = value;
			found_key = true;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_SERVICEIPC) == 0) {
			// this only does something on Mac OS X 10.4 "Tiger"
			found_key = true;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_SHUTDOWNMONITOR) == 0) {
			if (_launchd_shutdown_monitor) {
				job_log(j, LOG_ERR, "Only one job may monitor shutdown.");
			} else {
				j->shutdown_monitor = true;
				_launchd_shutdown_monitor = j;
			}
			found_key = true;
		}
		break;
	case 'l':
	case 'L':
		if (strcasecmp(key, LAUNCH_JOBKEY_LOWPRIORITYIO) == 0) {
			j->low_pri_io = value;
			found_key = true;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_LAUNCHONLYONCE) == 0) {
			j->only_once = value;
			found_key = true;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_LOWPRIORITYBACKGROUNDIO) == 0) {
			j->low_priority_background_io = true;
			found_key = true;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_LEGACYTIMERS) == 0) {
#if !TARGET_OS_EMBEDDED
			j->legacy_timers = value;
#else // !TARGET_OS_EMBEDDED
			job_log(j, LOG_ERR, "This key is not supported on this platform: %s", key);
#endif // !TARGET_OS_EMBEDDED
			found_key = true;
		}
		break;
	case 'm':
	case 'M':
		if (strcasecmp(key, LAUNCH_JOBKEY_MACHEXCEPTIONHANDLER) == 0) {
			j->internal_exc_handler = value;
			found_key = true;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_MULTIPLEINSTANCES) == 0) {
			j->multiple_instances = value;
			found_key = true;
		}
		break;
	case 'i':
	case 'I':
		if (strcasecmp(key, LAUNCH_JOBKEY_INITGROUPS) == 0) {
			if (getuid() != 0) {
				job_log(j, LOG_WARNING, "Ignored this key: %s", key);
				return;
			}
			j->no_init_groups = !value;
			found_key = true;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_IGNOREPROCESSGROUPATSHUTDOWN) == 0) {
			j->ignore_pg_at_shutdown = value;
			found_key = true;
		}
		break;
	case 'r':
	case 'R':
		if (strcasecmp(key, LAUNCH_JOBKEY_RUNATLOAD) == 0) {
			if (value) {
				// We don't want value == false to change j->start_pending
				j->start_pending = true;
			}
			found_key = true;
		}
		break;
	case 'e':
	case 'E':
		if (strcasecmp(key, LAUNCH_JOBKEY_ENABLEGLOBBING) == 0) {
			j->globargv = value;
			found_key = true;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_ENABLETRANSACTIONS) == 0) {
			j->enable_transactions = value;
			found_key = true;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_ENTERKERNELDEBUGGERBEFOREKILL) == 0) {
			j->debug_before_kill = value;
			found_key = true;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_EMBEDDEDPRIVILEGEDISPENSATION) == 0) {
#if TARGET_OS_EMBEDDED
			if (!_launchd_embedded_god) {
				if ((j->embedded_god = value)) {
					_launchd_embedded_god = j;
				}
			} else {
				job_log(j, LOG_ERR, "Job tried to claim %s after it has already been claimed.", key);
			}
#else
			job_log(j, LOG_ERR, "This key is not supported on this platform: %s", key);
#endif
			found_key = true;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_EMBEDDEDHOMESCREEN) == 0) {
#if TARGET_OS_EMBEDDED
			if (!_launchd_embedded_home) {
				if ((j->embedded_home = value)) {
					_launchd_embedded_home = j;
				}
			} else {
				job_log(j, LOG_ERR, "Job tried to claim %s after it has already been claimed.", key);
			}
#else
			job_log(j, LOG_ERR, "This key is not supported on this platform: %s", key);
#endif
		} else if (strcasecmp(key, LAUNCH_JOBKEY_EVENTMONITOR) == 0) {
			if (!_launchd_event_monitor) {
				j->event_monitor = value;
				if (value) {
					_launchd_event_monitor = j;
				}
			} else {
				job_log(j, LOG_NOTICE, "Job tried to steal event monitoring responsibility from: %s", _launchd_event_monitor->label);
			}
			found_key = true;
		}
		break;
	case 'w':
	case 'W':
		if (strcasecmp(key, LAUNCH_JOBKEY_WAITFORDEBUGGER) == 0) {
			j->wait4debugger = value;
			found_key = true;
		}
		break;
	case 'x':
	case 'X':
		if (strcasecmp(key, LAUNCH_JOBKEY_XPCDOMAINBOOTSTRAPPER) == 0) {
			if (pid1_magic) {
				if (_launchd_xpc_bootstrapper) {
					job_log(j, LOG_ERR, "This job tried to steal the XPC domain bootstrapper property from the following job: %s", _launchd_xpc_bootstrapper->label);
				} else {
					_launchd_xpc_bootstrapper = j;
					j->xpc_bootstrapper = value;
				}
			} else {
				job_log(j, LOG_ERR, "Non-daemon tried to claim XPC bootstrapper property.");
			}
		}
		found_key = true;
		break;
	default:
		break;
	}

	if (unlikely(!found_key)) {
		job_log(j, LOG_WARNING, "Unknown key for boolean: %s", key);
	}
}

void
job_import_string(job_t j, const char *key, const char *value)
{
	char **where2put = NULL;

	switch (key[0]) {
	case 'c':
	case 'C':
		if (strcasecmp(key, LAUNCH_JOBKEY_CFBUNDLEIDENTIFIER) == 0) {
			where2put = &j->cfbundleidentifier;
		}
		break;
	case 'm':
	case 'M':
		if (strcasecmp(key, LAUNCH_JOBKEY_MACHEXCEPTIONHANDLER) == 0) {
			where2put = &j->alt_exc_handler;
		}
		break;
	case 'p':
	case 'P':
		if (strcasecmp(key, LAUNCH_JOBKEY_PROGRAM) == 0) {
			return;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_POSIXSPAWNTYPE) == 0
				|| strcasecmp(key, LAUNCH_JOBKEY_PROCESSTYPE) == 0) {
			if (strcasecmp(value, LAUNCH_KEY_POSIXSPAWNTYPE_INTERACTIVE) == 0) {
				j->psproctype = POSIX_SPAWN_PROC_TYPE_DAEMON_INTERACTIVE;
			} else if (strcasecmp(value, LAUNCH_KEY_POSIXSPAWNTYPE_ADAPTIVE) == 0) {
				j->psproctype = POSIX_SPAWN_PROC_TYPE_DAEMON_ADAPTIVE;
			} else if (strcasecmp(value, LAUNCH_KEY_POSIXSPAWNTYPE_STANDARD) == 0) {
				j->psproctype = POSIX_SPAWN_PROC_TYPE_DAEMON_STANDARD;
			} else if (strcasecmp(value, LAUNCH_KEY_POSIXSPAWNTYPE_BACKGROUND) == 0) {
				j->psproctype = POSIX_SPAWN_PROC_TYPE_DAEMON_BACKGROUND;
			} else if (strcasecmp(value, LAUNCH_KEY_POSIXSPAWNTYPE_TALAPP) == 0) {
				j->psproctype = POSIX_SPAWN_PROC_TYPE_APP_TAL;
			} else if (strcasecmp(value, LAUNCH_KEY_POSIXSPAWNTYPE_SYSTEMAPP) == 0) {
				j->psproctype = POSIX_SPAWN_PROC_TYPE_APP_DEFAULT;
				j->system_app = true;
			} else if (strcasecmp(value, LAUNCH_KEY_POSIXSPAWNTYPE_APP) == 0) {
				j->psproctype = POSIX_SPAWN_PROC_TYPE_APP_DEFAULT;
				j->app = true;
			} else {
				job_log(j, LOG_ERR, "Unknown value for key %s: %s", key, value);
			}
			return;
		}
		break;
	case 'l':
	case 'L':
		if (strcasecmp(key, LAUNCH_JOBKEY_LABEL) == 0) {
			return;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_LIMITLOADTOHOSTS) == 0) {
			return;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_LIMITLOADFROMHOSTS) == 0) {
			return;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_LIMITLOADTOSESSIONTYPE) == 0) {
			return;
		}
		break;
	case 'r':
	case 'R':
		if (strcasecmp(key, LAUNCH_JOBKEY_ROOTDIRECTORY) == 0) {
			if (getuid() != 0) {
				job_log(j, LOG_WARNING, "Ignored this key: %s", key);
				return;
			}
			where2put = &j->rootdir;
		}
		break;
	case 'w':
	case 'W':
		if (strcasecmp(key, LAUNCH_JOBKEY_WORKINGDIRECTORY) == 0) {
			where2put = &j->workingdir;
		}
		break;
	case 'u':
	case 'U':
		if (strcasecmp(key, LAUNCH_JOBKEY_USERNAME) == 0) {
			if (getuid() != 0) {
				job_log(j, LOG_WARNING, "Ignored this key: %s", key);
				return;
			} else if (strcmp(value, "root") == 0) {
				return;
			}
			where2put = &j->username;
		}
		break;
	case 'g':
	case 'G':
		if (strcasecmp(key, LAUNCH_JOBKEY_GROUPNAME) == 0) {
			if (getuid() != 0) {
				job_log(j, LOG_WARNING, "Ignored this key: %s", key);
				return;
			} else if (strcmp(value, "wheel") == 0) {
				return;
			}
			where2put = &j->groupname;
		}
		break;
	case 's':
	case 'S':
		if (strcasecmp(key, LAUNCH_JOBKEY_STANDARDOUTPATH) == 0) {
			where2put = &j->stdoutpath;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_STANDARDERRORPATH) == 0) {
			where2put = &j->stderrpath;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_STANDARDINPATH) == 0) {
			where2put = &j->stdinpath;
			j->stdin_fd = _fd(open(value, O_RDONLY|O_CREAT|O_NOCTTY|O_NONBLOCK, DEFFILEMODE));
			if (job_assumes_zero_p(j, j->stdin_fd) != -1) {
				// open() should not block, but regular IO by the job should
				(void)job_assumes_zero_p(j, fcntl(j->stdin_fd, F_SETFL, 0));
				// XXX -- EV_CLEAR should make named pipes happy?
				(void)job_assumes_zero_p(j, kevent_mod(j->stdin_fd, EVFILT_READ, EV_ADD|EV_CLEAR, 0, 0, j));
			} else {
				j->stdin_fd = 0;
			}
#if HAVE_SANDBOX
		} else if (strcasecmp(key, LAUNCH_JOBKEY_SANDBOXPROFILE) == 0) {
			where2put = &j->seatbelt_profile;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_SANDBOXCONTAINER) == 0) {
			where2put = &j->container_identifier;
#endif
		}
		break;
	case 'X':
	case 'x':
		if (strcasecmp(key, LAUNCH_JOBKEY_XPCDOMAIN) == 0) {
			return;
		}
		break;
	default:
		job_log(j, LOG_WARNING, "Unknown key for string: %s", key);
		break;
	}

	if (likely(where2put)) {
		if (!(*where2put = strdup(value))) {
			(void)job_assumes_zero(j, errno);
		}
	} else {
		// See rdar://problem/5496612. These two are okay.
		if (strncmp(key, "SHAuthorizationRight", sizeof("SHAuthorizationRight")) == 0
			|| strncmp(key, "ServiceDescription", sizeof("ServiceDescription")) == 0) {
			job_log(j, LOG_APPLEONLY, "This key is no longer relevant and should be removed: %s", key);
		} else {
			job_log(j, LOG_WARNING, "Unknown key: %s", key);
		}
	}
}

void
job_import_integer(job_t j, const char *key, long long value)
{
	switch (key[0]) {
	case 'a':
	case 'A':
#if TARGET_OS_EMBEDDED
		if (strcasecmp(key, LAUNCH_JOBKEY_ASID) == 0) {
			if (launchd_embedded_handofgod) {
				if (audit_session_port((au_asid_t)value, &j->asport) == -1 && errno != ENOSYS) {
					(void)job_assumes_zero(j, errno);
				}
			}
		}
#endif
	case 'e':
	case 'E':
		if (strcasecmp(key, LAUNCH_JOBKEY_EXITTIMEOUT) == 0) {
			if (unlikely(value < 0)) {
				job_log(j, LOG_WARNING, "%s less than zero. Ignoring.", LAUNCH_JOBKEY_EXITTIMEOUT);
			} else if (unlikely(value > UINT32_MAX)) {
				job_log(j, LOG_WARNING, "%s is too large. Ignoring.", LAUNCH_JOBKEY_EXITTIMEOUT);
			} else {
				j->exit_timeout = (typeof(j->exit_timeout)) value;
			}
		} else if (strcasecmp(key, LAUNCH_JOBKEY_EMBEDDEDMAINTHREADPRIORITY) == 0) {
			j->main_thread_priority = value;
		}
		break;
	case 'j':
	case 'J':
		if (strcasecmp(key, LAUNCH_JOBKEY_JETSAMPRIORITY) == 0) {
			job_log(j, LOG_WARNING | LOG_CONSOLE, "Please change the JetsamPriority key to be in a dictionary named JetsamProperties.");

			launch_data_t pri = launch_data_new_integer(value);
			if (job_assumes(j, pri != NULL)) {
				jetsam_property_setup(pri, LAUNCH_JOBKEY_JETSAMPRIORITY, j);
				launch_data_free(pri);
			}
		}
	case 'n':
	case 'N':
		if (strcasecmp(key, LAUNCH_JOBKEY_NICE) == 0) {
			if (unlikely(value < PRIO_MIN)) {
				job_log(j, LOG_WARNING, "%s less than %d. Ignoring.", LAUNCH_JOBKEY_NICE, PRIO_MIN);
			} else if (unlikely(value > PRIO_MAX)) {
				job_log(j, LOG_WARNING, "%s is greater than %d. Ignoring.", LAUNCH_JOBKEY_NICE, PRIO_MAX);
			} else {
				j->nice = (typeof(j->nice)) value;
				j->setnice = true;
			}
		}
		break;
	case 't':
	case 'T':
		if (strcasecmp(key, LAUNCH_JOBKEY_TIMEOUT) == 0) {
			if (unlikely(value < 0)) {
				job_log(j, LOG_WARNING, "%s less than zero. Ignoring.", LAUNCH_JOBKEY_TIMEOUT);
			} else if (unlikely(value > UINT32_MAX)) {
				job_log(j, LOG_WARNING, "%s is too large. Ignoring.", LAUNCH_JOBKEY_TIMEOUT);
			} else {
				j->timeout = (typeof(j->timeout)) value;
			}
		} else if (strcasecmp(key, LAUNCH_JOBKEY_THROTTLEINTERVAL) == 0) {
			if (value < 0) {
				job_log(j, LOG_WARNING, "%s less than zero. Ignoring.", LAUNCH_JOBKEY_THROTTLEINTERVAL);
			} else if (value > UINT32_MAX) {
				job_log(j, LOG_WARNING, "%s is too large. Ignoring.", LAUNCH_JOBKEY_THROTTLEINTERVAL);
			} else {
				j->min_run_time = (typeof(j->min_run_time)) value;
			}
		}
		break;
	case 'u':
	case 'U':
		if (strcasecmp(key, LAUNCH_JOBKEY_UMASK) == 0) {
			j->mask = value;
			j->setmask = true;
		}
		break;
	case 's':
	case 'S':
		if (strcasecmp(key, LAUNCH_JOBKEY_STARTINTERVAL) == 0) {
			if (unlikely(value <= 0)) {
				job_log(j, LOG_WARNING, "%s is not greater than zero. Ignoring.", LAUNCH_JOBKEY_STARTINTERVAL);
			} else if (unlikely(value > UINT32_MAX)) {
				job_log(j, LOG_WARNING, "%s is too large. Ignoring.", LAUNCH_JOBKEY_STARTINTERVAL);
			} else {
				runtime_add_weak_ref();
				j->start_interval = (typeof(j->start_interval)) value;

				(void)job_assumes_zero_p(j, kevent_mod((uintptr_t)&j->start_interval, EVFILT_TIMER, EV_ADD, NOTE_SECONDS, j->start_interval, j));
			}
#if HAVE_SANDBOX
		} else if (strcasecmp(key, LAUNCH_JOBKEY_SANDBOXFLAGS) == 0) {
			j->seatbelt_flags = value;
#endif
		}

		break;
	default:
		job_log(j, LOG_WARNING, "Unknown key for integer: %s", key);
		break;
	}
}

void
job_import_opaque(job_t j __attribute__((unused)), const char *key, launch_data_t value __attribute__((unused)))
{
	switch (key[0]) {
	case 'q':
	case 'Q':
#if HAVE_QUARANTINE
		if (strcasecmp(key, LAUNCH_JOBKEY_QUARANTINEDATA) == 0) {
			size_t tmpsz = launch_data_get_opaque_size(value);

			if (job_assumes(j, j->quarantine_data = malloc(tmpsz))) {
				memcpy(j->quarantine_data, launch_data_get_opaque(value), tmpsz);
				j->quarantine_data_sz = tmpsz;
			}
		}
#endif
	case 's':
	case 'S':
		if (strcasecmp(key, LAUNCH_JOBKEY_SECURITYSESSIONUUID) == 0) {
			size_t tmpsz = launch_data_get_opaque_size(value);
			if (job_assumes(j, tmpsz == sizeof(uuid_t))) {
				memcpy(j->expected_audit_uuid, launch_data_get_opaque(value), sizeof(uuid_t));
			}
		}
		break;
	default:
		break;
	}
}

static void
policy_setup(launch_data_t obj, const char *key, void *context)
{
	job_t j = context;
	bool found_key = false;

	switch (key[0]) {
	case 'd':
	case 'D':
		if (strcasecmp(key, LAUNCH_JOBPOLICY_DENYCREATINGOTHERJOBS) == 0) {
			j->deny_job_creation = launch_data_get_bool(obj);
			found_key = true;
		}
		break;
	default:
		break;
	}

	if (unlikely(!found_key)) {
		job_log(j, LOG_WARNING, "Unknown policy: %s", key);
	}
}

void
job_import_dictionary(job_t j, const char *key, launch_data_t value)
{
	launch_data_t tmp;

	switch (key[0]) {
	case 'p':
	case 'P':
		if (strcasecmp(key, LAUNCH_JOBKEY_POLICIES) == 0) {
			launch_data_dict_iterate(value, policy_setup, j);
		}
		break;
	case 'k':
	case 'K':
		if (strcasecmp(key, LAUNCH_JOBKEY_KEEPALIVE) == 0) {
			launch_data_dict_iterate(value, semaphoreitem_setup, j);
		}
		break;
	case 'i':
	case 'I':
		if (strcasecmp(key, LAUNCH_JOBKEY_INETDCOMPATIBILITY) == 0) {
			j->inetcompat = true;
			j->abandon_pg = true;
			if ((tmp = launch_data_dict_lookup(value, LAUNCH_JOBINETDCOMPATIBILITY_WAIT))) {
				j->inetcompat_wait = launch_data_get_bool(tmp);
			}
		}
		break;
	case 'j':
	case 'J':
		if (strcasecmp(key, LAUNCH_JOBKEY_JETSAMPROPERTIES) == 0) {
			launch_data_dict_iterate(value, (void (*)(launch_data_t, const char *, void *))jetsam_property_setup, j);
		}
	case 'e':
	case 'E':
		if (strcasecmp(key, LAUNCH_JOBKEY_ENVIRONMENTVARIABLES) == 0) {
			launch_data_dict_iterate(value, envitem_setup, j);
		}		
		break;
	case 'u':
	case 'U':
		if (strcasecmp(key, LAUNCH_JOBKEY_USERENVIRONMENTVARIABLES) == 0) {
			j->importing_global_env = true;
			launch_data_dict_iterate(value, envitem_setup, j);
			j->importing_global_env = false;
		}
		break;
	case 's':
	case 'S':
		if (strcasecmp(key, LAUNCH_JOBKEY_SOCKETS) == 0) {
			launch_data_dict_iterate(value, socketgroup_setup, j);
		} else if (strcasecmp(key, LAUNCH_JOBKEY_STARTCALENDARINTERVAL) == 0) {
			calendarinterval_new_from_obj(j, value);
		} else if (strcasecmp(key, LAUNCH_JOBKEY_SOFTRESOURCELIMITS) == 0) {
			launch_data_dict_iterate(value, limititem_setup, j);
#if HAVE_SANDBOX
		} else if (strcasecmp(key, LAUNCH_JOBKEY_SANDBOXFLAGS) == 0) {
			launch_data_dict_iterate(value, seatbelt_setup_flags, j);
#endif
		}
		break;
	case 'h':
	case 'H':
		if (strcasecmp(key, LAUNCH_JOBKEY_HARDRESOURCELIMITS) == 0) {
			j->importing_hard_limits = true;
			launch_data_dict_iterate(value, limititem_setup, j);
			j->importing_hard_limits = false;
		}
		break;
	case 'm':
	case 'M':
		if (strcasecmp(key, LAUNCH_JOBKEY_MACHSERVICES) == 0) {
			launch_data_dict_iterate(value, machservice_setup, j);
		}
		break;
	case 'l':
	case 'L':
		if (strcasecmp(key, LAUNCH_JOBKEY_LAUNCHEVENTS) == 0) {
			launch_data_dict_iterate(value, eventsystem_setup, j);
		} else {
			if (strcasecmp(key, LAUNCH_JOBKEY_LIMITLOADTOHARDWARE) == 0) {
				return;
			}
			if (strcasecmp(key, LAUNCH_JOBKEY_LIMITLOADFROMHARDWARE) == 0) {
				return;
			}
		}
		break;
	default:
		job_log(j, LOG_WARNING, "Unknown key for dictionary: %s", key);
		break;
	}
}

void
job_import_array(job_t j, const char *key, launch_data_t value)
{
	size_t i, value_cnt = launch_data_array_get_count(value);

	switch (key[0]) {
	case 'p':
	case 'P':
		if (strcasecmp(key, LAUNCH_JOBKEY_PROGRAMARGUMENTS) == 0) {
			return;
		}
		break;
	case 'l':
	case 'L':
		if (strcasecmp(key, LAUNCH_JOBKEY_LIMITLOADTOHOSTS) == 0) {
			return;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_LIMITLOADFROMHOSTS) == 0) {
			return;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_LIMITLOADTOSESSIONTYPE) == 0) {
			job_log(j, LOG_NOTICE, "launchctl should have transformed the \"%s\" array to a string", LAUNCH_JOBKEY_LIMITLOADTOSESSIONTYPE);
			return;
		}
		break;
	case 'b':
	case 'B':
		if (strcasecmp(key, LAUNCH_JOBKEY_BINARYORDERPREFERENCE) == 0) {
			if (job_assumes(j, j->j_binpref = malloc(value_cnt * sizeof(*j->j_binpref)))) {
				j->j_binpref_cnt = value_cnt;
				for (i = 0; i < value_cnt; i++) {
					j->j_binpref[i] = (cpu_type_t) launch_data_get_integer(launch_data_array_get_index(value, i));
				}
			}
		}
		break;
	case 's':
	case 'S':
		if (strcasecmp(key, LAUNCH_JOBKEY_STARTCALENDARINTERVAL) == 0) {
			for (i = 0; i < value_cnt; i++) {
				calendarinterval_new_from_obj(j, launch_data_array_get_index(value, i));
			}
		}
		break;
	default:
		job_log(j, LOG_WARNING, "Unknown key for array: %s", key);
		break;
	}
}

void
job_import_keys(launch_data_t obj, const char *key, void *context)
{
	job_t j = context;
	launch_data_type_t kind;

	if (!obj) {
		launchd_syslog(LOG_ERR, "NULL object given to job_import_keys().");
		return;
	}

	kind = launch_data_get_type(obj);

	switch (kind) {
	case LAUNCH_DATA_BOOL:
		job_import_bool(j, key, launch_data_get_bool(obj));
		break;
	case LAUNCH_DATA_STRING:
		job_import_string(j, key, launch_data_get_string(obj));
		break;
	case LAUNCH_DATA_INTEGER:
		job_import_integer(j, key, launch_data_get_integer(obj));
		break;
	case LAUNCH_DATA_DICTIONARY:
		job_import_dictionary(j, key, obj);
		break;
	case LAUNCH_DATA_ARRAY:
		job_import_array(j, key, obj);
		break;
	case LAUNCH_DATA_OPAQUE:
		job_import_opaque(j, key, obj);
		break;
	default:
		job_log(j, LOG_WARNING, "Unknown value type '%d' for key: %s", kind, key);
		break;
	}
}

job_t
jobmgr_import2(jobmgr_t jm, launch_data_t pload)
{
	launch_data_t tmp, ldpa;
	const char *label = NULL, *prog = NULL;
	const char **argv = NULL;
	job_t j;

	if (!jobmgr_assumes(jm, pload != NULL)) {
		errno = EINVAL;
		return NULL;
	}

	if (unlikely(launch_data_get_type(pload) != LAUNCH_DATA_DICTIONARY)) {
		errno = EINVAL;
		return NULL;
	}

	if (unlikely(!(tmp = launch_data_dict_lookup(pload, LAUNCH_JOBKEY_LABEL)))) {
		errno = EINVAL;
		return NULL;
	}

	if (unlikely(launch_data_get_type(tmp) != LAUNCH_DATA_STRING)) {
		errno = EINVAL;
		return NULL;
	}

	if (unlikely(!(label = launch_data_get_string(tmp)))) {
		errno = EINVAL;
		return NULL;
	}

#if TARGET_OS_EMBEDDED
	if (unlikely(launchd_embedded_handofgod && _launchd_embedded_god)) {
		if (unlikely(!(tmp = launch_data_dict_lookup(pload, LAUNCH_JOBKEY_USERNAME)))) {
			errno = EPERM;
			return NULL;
		}

		const char *username = NULL;
		if (likely(tmp && launch_data_get_type(tmp) == LAUNCH_DATA_STRING)) {
			username = launch_data_get_string(tmp);
		} else {
			errno = EPERM;
			return NULL;
		}

		if (!jobmgr_assumes(jm, _launchd_embedded_god->username != NULL && username != NULL)) {
			errno = EPERM;
			return NULL;
		}

		if (unlikely(strcmp(_launchd_embedded_god->username, username) != 0)) {
			errno = EPERM;
			return NULL;
		}
	} else if (launchd_embedded_handofgod) {
		errno = EINVAL;
		return NULL;
	}
#endif

	if ((tmp = launch_data_dict_lookup(pload, LAUNCH_JOBKEY_PROGRAM))
		&& (launch_data_get_type(tmp) == LAUNCH_DATA_STRING)) {
		prog = launch_data_get_string(tmp);
	}

	int argc = 0;
	if ((ldpa = launch_data_dict_lookup(pload, LAUNCH_JOBKEY_PROGRAMARGUMENTS))) {
		size_t i, c;

		if (launch_data_get_type(ldpa) != LAUNCH_DATA_ARRAY) {
			errno = EINVAL;
			return NULL;
		}

		c = launch_data_array_get_count(ldpa);

		argv = alloca((c + 1) * sizeof(char *));

		for (i = 0; i < c; i++) {
			tmp = launch_data_array_get_index(ldpa, i);

			if (launch_data_get_type(tmp) != LAUNCH_DATA_STRING) {
				errno = EINVAL;
				return NULL;
			}

			argv[i] = launch_data_get_string(tmp);
		}

		argv[i] = NULL;
		argc = i;
	}

	if (!prog && argc == 0) {
		jobmgr_log(jm, LOG_ERR, "Job specifies neither Program nor ProgramArguments: %s", label);
		errno = EINVAL;
		return NULL;
	}

	/* Find the requested session. You cannot load services into XPC domains in
	 * this manner.
	 */
	launch_data_t session = launch_data_dict_lookup(pload, LAUNCH_JOBKEY_LIMITLOADTOSESSIONTYPE);
	if (session) {
		jobmgr_t jmt = NULL;
		if (launch_data_get_type(session) == LAUNCH_DATA_STRING) {
			jmt = jobmgr_find_by_name(jm, launch_data_get_string(session));
			if (!jmt) {
				jobmgr_log(jm, LOG_ERR, "Could not find requested session: %s", launch_data_get_string(session));
			} else {
				jm = jmt;
			}
		} else {
			jobmgr_log(jm, LOG_ERR, "Session type is not a string.");
		}

		if (!jmt) {
			errno = EINVAL;
			return NULL;
		}
	}

	/* For legacy reasons, we have a global hash of all labels in all job
	 * managers. So rather than make it a global, we store it in the root job
	 * manager. But for an XPC domain, we store a local hash of all services in
	 * the domain.
	 */
	jobmgr_t where2look = (jm->properties & BOOTSTRAP_PROPERTY_XPC_DOMAIN) ? jm : root_jobmgr;
	if (unlikely((j = job_find(where2look, label)) != NULL)) {
		if (jm->xpc_singleton) {
			/* There can (and probably will be) multiple attemtps to import the
			 * same XPC service from the same framework. This is okay. It's
			 * treated as a singleton, so just return the existing one so that
			 * it may be aliased into the requesting process' XPC domain.
			 */
			errno = EEXIST;
			return j;
		} else {
			/* If we're not a global XPC domain, then it's an error to try
			 * importing the same job/service multiple times.
			 */
			errno = EEXIST;
			return NULL;
		}
	} else if (unlikely(!jobmgr_label_test(where2look, label))) {
		errno = EINVAL;
		return NULL;
	}
	jobmgr_log(jm, LOG_DEBUG, "Importing %s.", label);

	if (likely(j = job_new(jm, label, prog, argv))) {
#if TARGET_OS_EMBEDDED
		job_apply_defaults(j);
#endif
		launch_data_dict_iterate(pload, job_import_keys, j);
		if (!uuid_is_null(j->expected_audit_uuid)) {
			uuid_string_t uuid_str;
			uuid_unparse(j->expected_audit_uuid, uuid_str);
			job_log(j, LOG_DEBUG, "Imported job. Waiting for session for UUID %s.", uuid_str);
			LIST_INSERT_HEAD(&s_needing_sessions, j, needing_session_sle);
			errno = ENEEDAUTH;
		} else {
			job_log(j, LOG_DEBUG, "No security session specified.");
			j->asport = MACH_PORT_NULL;
		}

		if (pid1_magic && !jm->parentmgr) {
			/* Workaround reentrancy in CF. We don't make this a global variable
			 * because we don't want per-user launchd's to inherit it. So we
			 * just set it for every job that we import into the System session.
			 *
			 * See <rdar://problem/9468837>.
			 */
			envitem_new(j, "__CF_USER_TEXT_ENCODING", "0x0:0:0", false);
		}

		if (j->event_monitor) {
			eventsystem_ping();
		}

#if TARGET_OS_EMBEDDED
		/* SpringBoard and backboardd must run at elevated priority.
		 *
		 * See <rdar://problem/9539873> and <rdar://problem/10984383>.
		 */
		if (j->embedded_god || j->embedded_home) {
			j->psproctype = POSIX_SPAWN_PROC_TYPE_APP_DEFAULT;
		}
#endif
	}

	return j;
}

bool
jobmgr_label_test(jobmgr_t jm, const char *str)
{
	const char *ptr;

	if (str[0] == '\0') {
		jobmgr_log(jm, LOG_ERR, "Empty job labels are not allowed");
		return false;
	}

	for (ptr = str; *ptr; ptr++) {
		if (iscntrl(*ptr)) {
			jobmgr_log(jm, LOG_ERR, "ASCII control characters are not allowed in job labels. Index: %td Value: 0x%hhx", ptr - str, *ptr);
			return false;
		}
	}

	if ((strncasecmp(str, "com.apple.launchd", strlen("com.apple.launchd")) == 0)
		|| (strncasecmp(str, "com.apple.launchctl", strlen("com.apple.launchctl")) == 0)) {
		jobmgr_log(jm, LOG_ERR, "Job labels are not allowed to use a reserved prefix: %s", str);
		return false;
	}

	return true;
}

job_t 
job_find(jobmgr_t jm, const char *label)
{
	job_t ji;

	if (!jm) {
		jm = root_jobmgr;
	}

	LIST_FOREACH(ji, &jm->label_hash[hash_label(label)], label_hash_sle) {
		if (unlikely(ji->removal_pending || ji->mgr->shutting_down)) {
			// 5351245 and 5488633 respectively
			continue;
		}

		if (strcmp(ji->label, label) == 0) {
			return ji;
		}
	}

	errno = ESRCH;
	return NULL;
}

// Should try and consolidate with job_mig_intran2() and jobmgr_find_by_pid().
job_t
jobmgr_find_by_pid_deep(jobmgr_t jm, pid_t p, bool anon_okay)
{
	job_t ji = NULL;
	LIST_FOREACH(ji, &jm->active_jobs[ACTIVE_JOB_HASH(p)], pid_hash_sle) {
		if (ji->p == p && (!ji->anonymous || (ji->anonymous && anon_okay))) {
			return ji;
		}
	}

	jobmgr_t jmi = NULL;
	SLIST_FOREACH(jmi, &jm->submgrs, sle) {
		if ((ji = jobmgr_find_by_pid_deep(jmi, p, anon_okay))) {
			break;
		}
	}

	return ji;
}

job_t
jobmgr_find_by_pid(jobmgr_t jm, pid_t p, bool create_anon)
{
	job_t ji;

	LIST_FOREACH(ji, &jm->active_jobs[ACTIVE_JOB_HASH(p)], pid_hash_sle) {
		if (ji->p == p) {
			return ji;
		}
	}

	return create_anon ? job_new_anonymous(jm, p) : NULL;
}

job_t
managed_job(pid_t p)
{
	job_t ji;

	LIST_FOREACH(ji, &managed_actives[ACTIVE_JOB_HASH(p)], pid_hash_sle) {
		if (ji->p == p) {
			return ji;
		}
	}

	return NULL;
}

job_t 
job_mig_intran2(jobmgr_t jm, mach_port_t mport, pid_t upid)
{
	jobmgr_t jmi;
	job_t ji;

	if (jm->jm_port == mport) {
		return jobmgr_find_by_pid(jm, upid, true);
	}

	SLIST_FOREACH(jmi, &jm->submgrs, sle) {
		job_t jr;

		if ((jr = job_mig_intran2(jmi, mport, upid))) {
			return jr;
		}
	}

	LIST_FOREACH(ji, &jm->jobs, sle) {
		if (ji->j_port == mport) {
			return ji;
		}
	}

	return NULL;
}

job_t 
job_mig_intran(mach_port_t p)
{
	struct ldcred *ldc = runtime_get_caller_creds();
	job_t jr;

	jr = job_mig_intran2(root_jobmgr, p, ldc->pid);

	if (!jr) {
		struct proc_bsdshortinfo proc;
		if (proc_pidinfo(ldc->pid, PROC_PIDT_SHORTBSDINFO, 1, &proc, PROC_PIDT_SHORTBSDINFO_SIZE) == 0) {
			if (errno != ESRCH) {
				(void)jobmgr_assumes_zero(root_jobmgr, errno);
			} else {
				jobmgr_log(root_jobmgr, LOG_ERR, "%s[%i] disappeared out from under us (UID: %u EUID: %u)", proc.pbsi_comm, ldc->pid, ldc->uid, ldc->euid);
			}
		}
	}

	return jr;
}

job_t
job_find_by_service_port(mach_port_t p)
{
	struct machservice *ms;

	LIST_FOREACH(ms, &port_hash[HASH_PORT(p)], port_hash_sle) {
		if (ms->recv && (ms->port == p)) {
			return ms->job;
		}
	}

	return NULL;
}

void
job_mig_destructor(job_t j)
{
	/* The job can go invalid before this point.
	 *
	 * <rdar://problem/5477111>
	 */
	if (unlikely(j && (j != workaround_5477111) && j->unload_at_mig_return)) {
		job_log(j, LOG_NOTICE, "Unloading PID %u at MIG return.", j->p);
		job_remove(j);
	}

	workaround_5477111 = NULL;

	calendarinterval_sanity_check();
}

void
job_export_all2(jobmgr_t jm, launch_data_t where)
{
	jobmgr_t jmi;
	job_t ji;

	SLIST_FOREACH(jmi, &jm->submgrs, sle) {
		job_export_all2(jmi, where);
	}

	LIST_FOREACH(ji, &jm->jobs, sle) {
		launch_data_t tmp;

		if (jobmgr_assumes(jm, (tmp = job_export(ji)) != NULL)) {
			launch_data_dict_insert(where, tmp, ji->label);
		}
	}
}

launch_data_t
job_export_all(void)
{
	launch_data_t resp = launch_data_alloc(LAUNCH_DATA_DICTIONARY);

	if (resp != NULL) {
		job_export_all2(root_jobmgr, resp);
	} else {
		(void)os_assumes_zero(errno);
	}

	return resp;
}

void
job_log_stray_pg(job_t j)
{
	pid_t *pids = NULL;
	size_t len = sizeof(pid_t) * get_kern_max_proc();
	int i = 0, kp_cnt = 0;

	if (!launchd_apple_internal) {
		return;
	}

	runtime_ktrace(RTKT_LAUNCHD_FINDING_STRAY_PG, j->p, 0, 0);

	if (!job_assumes(j, (pids = malloc(len)) != NULL)) {
		return;
	}
	if (job_assumes_zero_p(j, (kp_cnt = proc_listpgrppids(j->p, pids, len))) == -1) {
		goto out;
	}

	for (i = 0; i < kp_cnt; i++) {
		pid_t p_i = pids[i];
		if (p_i == j->p) {
			continue;
		} else if (p_i == 0 || p_i == 1) {
			continue;
		}

		struct proc_bsdshortinfo proc;
		if (proc_pidinfo(p_i, PROC_PIDT_SHORTBSDINFO, 1, &proc, PROC_PIDT_SHORTBSDINFO_SIZE) == 0) {
			if (errno != ESRCH) {
				(void)job_assumes_zero(j, errno);
			}
			continue;
		}

		pid_t pp_i = proc.pbsi_ppid;
		const char *z = (proc.pbsi_status == SZOMB) ? "zombie " : "";
		const char *n = proc.pbsi_comm;

		job_log(j, LOG_WARNING, "Stray %sprocess with PGID equal to this dead job: PID %u PPID %u PGID %u %s", z, p_i, pp_i, proc.pbsi_pgid, n);
	}

out:
	free(pids);
}

#if HAVE_SYSTEMSTATS
static void
systemstats_timer_callback(void)
{
	jobmgr_log_perf_statistics(root_jobmgr, true);
}

static bool
systemstats_is_enabled(void)
{
	static bool systemstats_enabled;

	if (!systemstats_enabled) {
		char *store = launchd_copy_persistent_store(LAUNCHD_PERSISTENT_STORE_LOGS, NULL);
		systemstats_enabled = systemstats_init(SYSTEMSTATS_WRITER_launchd, store);
		free(store);

		uint64_t interval;
		interval = systemstats_get_log_interval(SYSTEMSTATS_WRITER_launchd);

		if (pid1_magic && systemstats_enabled && interval) {
			jobmgr_assumes_zero_p(root_jobmgr, kevent_mod((uintptr_t)systemstats_timer_callback, EVFILT_TIMER, EV_ADD, NOTE_SECONDS, interval, root_jobmgr));
		}
	}

	return systemstats_enabled;
}
#endif // HAVE_SYSTEMSTATS

void
job_reap(job_t j)
{
	bool is_system_bootstrapper = ((j->is_bootstrapper && pid1_magic) && !j->mgr->parentmgr);

	job_log(j, LOG_DEBUG, "Reaping");

	if (unlikely(j->weird_bootstrap)) {
		int64_t junk = 0;
		job_mig_swap_integer(j, VPROC_GSK_WEIRD_BOOTSTRAP, 0, 0, &junk);
	}

	if (j->fork_fd) {
		(void)job_assumes_zero_p(j, runtime_close(j->fork_fd));
		j->fork_fd = 0;
	}

	bool was_dirty = false;
	if (!(j->anonymous || j->implicit_reap)) {
		uint32_t flags = 0;
		(void)job_assumes_zero(j, proc_get_dirty(j->p, &flags));

		j->idle_exit = (flags & PROC_DIRTY_ALLOWS_IDLE_EXIT);
		was_dirty = (flags & PROC_DIRTY_IS_DIRTY);

		job_log(j, LOG_DEBUG, "%sob exited %s.", j->idle_exit ? "Idle-exit j" : "J", was_dirty ? "while dirty" : "cleanly");
	}

	if (j->idle_exit && was_dirty) {
		if (j->jettisoned) {
			job_log(j, LOG_NOTICE, "Idle-exit job was jettisoned while dirty. Will respawn immediately.");
			j->unthrottle = true;
			j->start_pending = true;
		} else {
			job_log(j, LOG_INFO, "Idle-exit job exited while dirty.");
		}
	} else if (j->idle_exit && j->jettisoned) {
		/* If an idle-exit job is jettisoned, then we shouldn't throttle its
		 * next respawn because it could not help when it exited. If it ran for
		 * the minimum runtime, then this doesn't really matter. If it ran for
		 * less than the minimum runtime, it will not be throttled.
		 *
		 * <rdar://problem/12098667>
		 */
		job_log(j, LOG_NOTICE, "Idle-exit job was jettisoned. Will bypass throttle interval for next on-demand launch.");
		j->unthrottle = true;
	}

	if (j->anonymous) {
		j->last_exit_status = 0;
	} else {
		uint64_t rt = runtime_get_nanoseconds_since(j->start_time);
		j->trt += rt;

		job_log(j, LOG_PERF, "Last instance wall time: %06f", (double)rt / (double)NSEC_PER_SEC);
		j->nruns++;

		/* The job is dead. While the PID/PGID is still known to be valid, try
		 * to kill abandoned descendant processes.
		 */
		job_log_stray_pg(j);
		if (!j->abandon_pg) {
			if (unlikely(killpg2(j->p, SIGTERM) == -1 && errno != ESRCH)) {
				job_log(j, LOG_APPLEONLY, "Bug: 5487498");
			}
		}

		int r = -1;
		if (!j->implicit_reap) {
			/* If the shutdown monitor has suspended a task and not resumed it
			 * resumed it before exiting, the kernel will not clean up after the
			 * shutdown monitor. It will, instead, leave the task suspended and
			 * not process any pending signals on the event loop for the task.
			 *
			 * There are a variety of other kernel bugs that could prevent a
			 * process from exiting, usually having to do with faulty hardware
			 * or talking to misbehaving drivers that mark a thread as
			 * uninterruptible and deadlock/hang before unmarking it as such. So
			 * we have to work around that too.
			 *
			 * See <rdar://problem/9284889&9359725>.
			 */
			if (j->workaround9359725) {
				job_log(j, LOG_NOTICE, "Simulated exit: <rdar://problem/9359725>");
				j->last_exit_status = W_EXITCODE(-1, SIGSEGV);
			} else {
#if HAVE_SYSTEMSTATS
				int r2;
				struct rusage_info_v1 ri;
				r2 = job_assumes_zero(j, proc_pid_rusage(j->p, RUSAGE_INFO_V1, (rusage_info_t)&ri));
#endif
				if ((r = wait4(j->p, &j->last_exit_status, 0, NULL)) == -1) {
					job_log(j, LOG_ERR, "Reap failed. Assuming job exited: %d: %s", errno, strerror(errno));
					j->last_exit_status = W_EXITCODE(-1, SIGSEGV);
				}

				if (j->idle_exit && j->jettisoned) {
					// Treat idle-exit jettisons as successful exit.
					//
					// <rdar://problem/13338973>
					(void)job_assumes_zero(j, WTERMSIG(j->last_exit_status));
					j->last_exit_status = W_EXITCODE(0, 0);
				}
#if HAVE_SYSTEMSTATS
				if (r2 == 0) {
					job_log_perf_statistics(j, &ri, j->last_exit_status);
				}
#endif
			}
		} else {
			job_log(j, LOG_INFO, "Job was implicitly reaped by the kernel.");
		}
	}

	if (j->exit_timeout) {
		(void)kevent_mod((uintptr_t)&j->exit_timeout, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
	}

	LIST_REMOVE(j, pid_hash_sle);
	if (!j->anonymous) {
		LIST_REMOVE(j, global_pid_hash_sle);
	}

	if (j->sent_signal_time) {
		uint64_t td_sec, td_usec, td = runtime_get_nanoseconds_since(j->sent_signal_time);

		td_sec = td / NSEC_PER_SEC;
		td_usec = (td % NSEC_PER_SEC) / NSEC_PER_USEC;

		job_log(j, LOG_DEBUG, "Exited %llu.%06llu seconds after the first signal was sent", td_sec, td_usec);
	}

	int exit_status = WEXITSTATUS(j->last_exit_status);
	if (WIFEXITED(j->last_exit_status) && exit_status != 0) {
		if (!j->did_exec && _launchd_support_system) {
			xpc_object_t event = NULL;
			switch (exit_status) {
			case ENOENT:
			case ENOTDIR:
			case ESRCH:
				job_log(j, LOG_NOTICE, "Job failed to exec(3). Setting up event to tell us when to try again: %d: %s", exit_status, strerror(exit_status));
				event = xpc_dictionary_create(NULL, NULL, 0);
				xpc_dictionary_set_string(event, "Executable", j->prog ? j->prog : j->argv[0]);
				if (j->mach_uid) {
					xpc_dictionary_set_uint64(event, "UID", j->mach_uid);
				} else if (j->username) {
					xpc_dictionary_set_string(event, "UserName", j->username);
				}

				if (j->groupname) {
					xpc_dictionary_set_string(event, "GroupName", j->groupname);
				}

				(void)externalevent_new(j, _launchd_support_system, j->label, event, 0);
				xpc_release(event);

				j->waiting4ok = true;
			default:
				job_log(j, LOG_NOTICE, "Job failed to exec(3) for weird reason: %d", exit_status);
			}
		} else {
			int level = LOG_INFO;
			if (exit_status != 0) {
				level = LOG_ERR;
			}

			job_log(j, level, "Exited with code: %d", exit_status);
		}
	}

	if (WIFSIGNALED(j->last_exit_status)) {
		int s = WTERMSIG(j->last_exit_status);
		if ((SIGKILL == s || SIGTERM == s) && !j->stopped) {
			job_log(j, LOG_NOTICE, "Exited: %s", strsignal(s));
		} else if (!(j->stopped || j->clean_kill || j->jettisoned)) {
			switch (s) {
			// Signals which indicate a crash.
			case SIGILL:
			case SIGABRT:
			case SIGFPE:
			case SIGBUS:
			case SIGSEGV:
			case SIGSYS:
			/* If the kernel has posted NOTE_EXIT and the signal sent to the process was
			 * SIGTRAP, assume that it's a crash.
			 */
			case SIGTRAP:
				j->crashed = true;
				job_log(j, LOG_WARNING, "Job appears to have crashed: %s", strsignal(s));
				break;
			default:
				job_log(j, LOG_WARNING, "Exited abnormally: %s", strsignal(s));
				break;
			}

			if (is_system_bootstrapper && j->crashed) {
				job_log(j, LOG_ERR | LOG_CONSOLE, "The %s bootstrapper has crashed: %s", j->mgr->name, strsignal(s));
			}
		}
	}

	j->reaped = true;

	struct machservice *msi = NULL;
	if (j->crashed || !(j->did_exec || j->anonymous)) {
		SLIST_FOREACH(msi, &j->machservices, sle) {
			if (j->crashed && !msi->isActive && (msi->drain_one_on_crash || msi->drain_all_on_crash)) {
				machservice_drain_port(msi);
			}

			if (!j->did_exec && msi->reset && job_assumes(j, !msi->isActive)) {
				machservice_resetport(j, msi);
			}
		}
	}

	/* HACK: Essentially duplicating the logic directly above. But this has
	 * gotten really hairy, and I don't want to try consolidating it right now.
	 */
	if (j->xpc_service && !j->xpcproxy_did_exec) {
		job_log(j, LOG_ERR, "XPC Service could not exec(3). Resetting port.");
		SLIST_FOREACH(msi, &j->machservices, sle) {
			/* Drain the messages but do not reset the port. If xpcproxy could
			 * not exec(3), then we don't want to continue trying, since there
			 * is very likely a serious configuration error with the service.
			 *
			 * The above comment is weird. I originally said we should drain
			 * messages but not reset the port, but that's exactly what we do
			 * below, and I'm not sure which is the mistake, the comment or the
			 * actual behavior.
			 *
			 * Since it's always been this way, I'll assume that the comment is
			 * incorrect, but I'll leave it in place just to remind myself to
			 * actually look into it at some point.
			 *
			 * <rdar://problem/8986802>
			 */
			if (msi->upfront && job_assumes(j, !msi->isActive)) {
				machservice_resetport(j, msi);
			}
		}
	}

	struct suspended_peruser *spi = NULL;
	while ((spi = LIST_FIRST(&j->suspended_perusers))) {
		job_log(j, LOG_ERR, "Job exited before resuming per-user launchd for UID %u. Will forcibly resume.", spi->j->mach_uid);
		spi->j->peruser_suspend_count--;
		if (spi->j->peruser_suspend_count == 0) {
			job_dispatch(spi->j, false);
		}
		LIST_REMOVE(spi, sle);
		free(spi);
	}

	if (j->exit_status_dest) {
		errno = helper_downcall_wait(j->exit_status_dest, j->last_exit_status);
		if (errno && errno != MACH_SEND_INVALID_DEST) {
			(void)job_assumes_zero(j, errno);
		}

		j->exit_status_dest = MACH_PORT_NULL;
	}

	if (j->spawn_reply_port) {
		/* If the child never called exec(3), we must send a spawn() reply so
		 * that the requestor can get exit status from it. If we fail to send
		 * the reply for some reason, we have to deallocate the exit status port
		 * ourselves.
		 */
		kern_return_t kr = job_mig_spawn2_reply(j->spawn_reply_port, BOOTSTRAP_SUCCESS, j->p, j->exit_status_port);
		if (kr) {
			if (kr != MACH_SEND_INVALID_DEST) {
				(void)job_assumes_zero(j, kr);
			}

			(void)job_assumes_zero(j, launchd_mport_close_recv(j->exit_status_port));
		}

		j->exit_status_port = MACH_PORT_NULL;
		j->spawn_reply_port = MACH_PORT_NULL;
	}

	if (j->anonymous) {
		total_anon_children--;
		if (j->holds_ref) {
			job_log(j, LOG_PERF, "Anonymous job exited holding reference.");
			runtime_del_ref();
		}
	} else {
		job_log(j, LOG_PERF, "Job exited.");
		runtime_del_ref();
		total_children--;
	}

	if (j->has_console) {
		launchd_wsp = 0;
	}

	if (j->shutdown_monitor) {
		job_log(j, LOG_NOTICE | LOG_CONSOLE, "Shutdown monitor has exited.");
		_launchd_shutdown_monitor = NULL;
		j->shutdown_monitor = false;
	}

	if (!j->anonymous) {
		j->mgr->normal_active_cnt--;
	}
	j->sent_signal_time = 0;
	j->sent_sigkill = false;
	j->clean_kill = false;
	j->event_monitor_ready2signal = false;
	j->p = 0;
	j->uniqueid = 0;
}

void
jobmgr_dispatch_all(jobmgr_t jm, bool newmounthack)
{
	jobmgr_t jmi, jmn;
	job_t ji, jn;

	if (jm->shutting_down) {
		return;
	}

	SLIST_FOREACH_SAFE(jmi, &jm->submgrs, sle, jmn) {
		jobmgr_dispatch_all(jmi, newmounthack);
	}

	LIST_FOREACH_SAFE(ji, &jm->jobs, sle, jn) {
		if (newmounthack && ji->start_on_mount) {
			ji->start_pending = true;
		}

		job_dispatch(ji, false);
	}
}

void
job_dispatch_curious_jobs(job_t j)
{	
	job_t ji = NULL, jt = NULL;
	SLIST_FOREACH_SAFE(ji, &s_curious_jobs, curious_jobs_sle, jt) {
		struct semaphoreitem *si = NULL;
		SLIST_FOREACH(si, &ji->semaphores, sle) {			
			if (!(si->why == OTHER_JOB_ENABLED || si->why == OTHER_JOB_DISABLED)) {
				continue;
			}

			if (strcmp(si->what, j->label) == 0) {
				job_log(ji, LOG_DEBUG, "Dispatching out of interest in \"%s\".", j->label);

				if (!ji->removing) {
					job_dispatch(ji, false);
				} else {
					job_log(ji, LOG_NOTICE, "The following job is circularly dependent upon this one: %s", j->label);
				}

				/* ji could be removed here, so don't do anything with it or its semaphores
				 * after this point.
				 */
				break;
			}
		}
	}
}

job_t
job_dispatch(job_t j, bool kickstart)
{
	// Don't dispatch a job if it has no audit session set.
	if (!uuid_is_null(j->expected_audit_uuid)) {
		job_log(j, LOG_DEBUG, "Job is still awaiting its audit session UUID. Not dispatching.");
		return NULL;
	}
	if (j->alias) {
		job_log(j, LOG_DEBUG, "Job is an alias. Not dispatching.");
		return NULL;
	}

	if (j->waiting4ok) {
		job_log(j, LOG_DEBUG, "Job cannot exec(3). Not dispatching.");
		return NULL;
	}

#if TARGET_OS_EMBEDDED
	if (launchd_embedded_handofgod && _launchd_embedded_god) {
		if (!job_assumes(j, _launchd_embedded_god->username != NULL && j->username != NULL)) {
			errno = EPERM;
			return NULL;
		}

		if (strcmp(j->username, _launchd_embedded_god->username) != 0) {
			errno = EPERM;
			return NULL;
		}
	} else if (launchd_embedded_handofgod) {
		errno = EINVAL;
		return NULL;
	}
#endif

	/*
	 * The whole job removal logic needs to be consolidated. The fact that
	 * a job can be removed from just about anywhere makes it easy to have
	 * stale pointers left behind somewhere on the stack that might get
	 * used after the deallocation. In particular, during job iteration.
	 *
	 * This is a classic example. The act of dispatching a job may delete it.
	 */	
	if (!job_active(j)) {
		if (job_useless(j)) {
			job_log(j, LOG_DEBUG, "Job is useless. Removing.");
			job_remove(j);
			return NULL;
		}
		if (unlikely(j->per_user && j->peruser_suspend_count > 0)) {
			job_log(j, LOG_DEBUG, "Per-user launchd is suspended. Not dispatching.");
			return NULL;
		}

		if (kickstart || job_keepalive(j)) {
			job_log(j, LOG_DEBUG, "%starting job", kickstart ? "Kicks" : "S");
			job_start(j);
		} else {
			job_log(j, LOG_DEBUG, "Watching job.");
			job_watch(j);
		}
	} else {
		job_log(j, LOG_DEBUG, "Tried to dispatch an already active job: %s.", job_active(j));
	}

	return j;
}

void
job_kill(job_t j)
{
	if (unlikely(!j->p || j->anonymous)) {
		return;
	}

	(void)job_assumes_zero_p(j, kill2(j->p, SIGKILL));

	j->sent_sigkill = true;
	(void)job_assumes_zero_p(j, kevent_mod((uintptr_t)&j->exit_timeout, EVFILT_TIMER, EV_ADD|EV_ONESHOT, NOTE_SECONDS, LAUNCHD_SIGKILL_TIMER, j));

	job_log(j, LOG_DEBUG, "Sent SIGKILL signal");
}

void
job_open_shutdown_transaction(job_t j)
{
	int rv = proc_set_dirty(j->p, true);
	if (rv != 0) {
		job_log(j, LOG_DEBUG, "Job wants to be dirty at shutdown, but it is not Instant Off-compliant. Treating normally.");
		j->dirty_at_shutdown = false;
	}
}

void
job_close_shutdown_transaction(job_t j)
{
	if (j->dirty_at_shutdown) {
		job_log(j, LOG_DEBUG, "Closing shutdown transaction for job.");
		(void)job_assumes_zero(j, proc_set_dirty(j->p, false));
		j->dirty_at_shutdown = false;
	}
}

void
job_log_children_without_exec(job_t j)
{
	pid_t *pids = NULL;
	size_t len = sizeof(pid_t) * get_kern_max_proc();
	int i = 0, kp_cnt = 0;

	if (!launchd_apple_internal || j->anonymous || j->per_user) {
		return;
	}

	if (!job_assumes(j, (pids = malloc(len)) != NULL)) {
		return;
	}
	if (job_assumes_zero_p(j, (kp_cnt = proc_listchildpids(j->p, pids, len))) == -1) {
		goto out;
	}

	for (i = 0; i < kp_cnt; i++) {
		struct proc_bsdshortinfo proc;
		if (proc_pidinfo(pids[i], PROC_PIDT_SHORTBSDINFO, 1, &proc, PROC_PIDT_SHORTBSDINFO_SIZE) == 0) {
			if (errno != ESRCH) {
				(void)job_assumes_zero(j, errno);
			}
			continue;
		}
		if (proc.pbsi_flags & P_EXEC) {
			continue;
		}

		job_log(j, LOG_DEBUG, "Called *fork(). Please switch to posix_spawn*(), pthreads or launchd. Child PID %u", pids[i]);
	}

out:
	free(pids);
}

void
job_callback_proc(job_t j, struct kevent *kev)
{
	bool program_changed = false;
	int fflags = kev->fflags;

	job_log(j, LOG_DEBUG, "EVFILT_PROC event for job.");
	log_kevent_struct(LOG_DEBUG, kev, 0);

	if (fflags & NOTE_EXEC) {
		program_changed = true;

		if (j->anonymous) {
			struct proc_bsdshortinfo proc;
			if (proc_pidinfo(j->p, PROC_PIDT_SHORTBSDINFO, 1, &proc, PROC_PIDT_SHORTBSDINFO_SIZE) > 0) {
				char newlabel[1000];

				snprintf(newlabel, sizeof(newlabel), "%p.anonymous.%s", j, proc.pbsi_comm);

				job_log(j, LOG_INFO, "Program changed. Updating the label to: %s", newlabel);

				LIST_REMOVE(j, label_hash_sle);
				strcpy((char *)j->label, newlabel);

				jobmgr_t where2put = root_jobmgr;
				if (j->mgr->properties & BOOTSTRAP_PROPERTY_XPC_DOMAIN) {
					where2put = j->mgr;
				}
				LIST_INSERT_HEAD(&where2put->label_hash[hash_label(j->label)], j, label_hash_sle);
			} else if (errno != ESRCH) {
				(void)job_assumes_zero(j, errno);
			}
		} else {
			if (j->spawn_reply_port) {
				errno = job_mig_spawn2_reply(j->spawn_reply_port, BOOTSTRAP_SUCCESS, j->p, j->exit_status_port);
				if (errno) {
					if (errno != MACH_SEND_INVALID_DEST) {
						(void)job_assumes_zero(j, errno);
					}
					(void)job_assumes_zero(j, launchd_mport_close_recv(j->exit_status_port));
				}

				j->spawn_reply_port = MACH_PORT_NULL;
				j->exit_status_port = MACH_PORT_NULL;
			}

			if (j->xpc_service && j->did_exec) {
				j->xpcproxy_did_exec = true;
			}

			j->did_exec = true;
			job_log(j, LOG_DEBUG, "Program changed");
		}
	}

	if (fflags & NOTE_FORK) {
		job_log(j, LOG_DEBUG, "fork()ed%s", program_changed ? ". For this message only: We don't know whether this event happened before or after execve()." : "");
		job_log_children_without_exec(j);
	}

	if (fflags & NOTE_EXIT) {
		if (kev->data & NOTE_EXIT_DECRYPTFAIL) {
			j->fpfail = true;
			job_log(j, LOG_WARNING, "FairPlay decryption failed on binary for job.");
		} else if (kev->data & NOTE_EXIT_MEMORY) {
			j->jettisoned = true;
			job_log(j, LOG_INFO, "Job was killed due to memory pressure.");
		}

		job_reap(j);

		if (j->anonymous) {
			job_remove(j);
			j = NULL;
		} else {
			struct waiting4attach *w4ai = NULL;
			struct waiting4attach *w4ait = NULL;
			LIST_FOREACH_SAFE(w4ai, &_launchd_domain_waiters, le, w4ait) {
				if (w4ai->dest == (pid_t)kev->ident) {
					waiting4attach_delete(j->mgr, w4ai);
				}
			}

			(void)job_dispatch(j, false);
		}
	}
}

void
job_callback_timer(job_t j, void *ident)
{
	if (j == ident) {
		job_log(j, LOG_DEBUG, "j == ident (%p)", ident);
		job_dispatch(j, true);
	} else if (&j->semaphores == ident) {
		job_log(j, LOG_DEBUG, "&j->semaphores == ident (%p)", ident);
		job_dispatch(j, false);
	} else if (&j->start_interval == ident) {
		job_log(j, LOG_DEBUG, "&j->start_interval == ident (%p)", ident);
		j->start_pending = true;
		job_dispatch(j, false);
	} else if (&j->exit_timeout == ident) {
		if (!job_assumes(j, j->p != 0)) {
			return;
		}

		if (j->sent_sigkill) {
			uint64_t td = runtime_get_nanoseconds_since(j->sent_signal_time);

			td /= NSEC_PER_SEC;
			td -= j->clean_kill ? 0 : j->exit_timeout;

			job_log(j, LOG_WARNING | LOG_CONSOLE, "Job has not died after being %skilled %llu seconds ago. Simulating exit.", j->clean_kill ? "cleanly " : "", td);
			j->workaround9359725 = true;

			// This basically has to be done off the main thread. We have no
			// mechanism for draining the main queue in our run loop (like CF
			// does), and the kevent mechanism wants an object to be associated
			// as the callback. So we just create a dispatch source and reap the
			// errant PID whenever we can. Note that it is not safe for us to do
			// any logging in this block, since logging requires exclusive
			// access to global data structures that is only protected by the
			// main thread.
			dispatch_source_t hack_13570156 = dispatch_source_create(DISPATCH_SOURCE_TYPE_PROC, j->p, DISPATCH_PROC_EXIT, dispatch_get_global_queue(0, 0));
			dispatch_source_set_event_handler(hack_13570156, ^{
				pid_t pid = (pid_t)dispatch_source_get_handle(hack_13570156);

				int status = 0;
				(void)waitpid(pid, &status, 0);
				dispatch_release(hack_13570156);
			});

			dispatch_resume(hack_13570156);

			if (launchd_trap_sigkill_bugs) {
				job_log(j, LOG_NOTICE | LOG_CONSOLE, "Trapping into kernel debugger. You can continue the machine after it has been debugged, and shutdown will proceed normally.");
				(void)job_assumes_zero(j, host_reboot(mach_host_self(), HOST_REBOOT_DEBUGGER));
			}

			struct kevent bogus_exit;
			EV_SET(&bogus_exit, j->p, EVFILT_PROC, 0, NOTE_EXIT, 0, 0);
			jobmgr_callback(j->mgr, &bogus_exit);
		} else {
			if (unlikely(j->debug_before_kill)) {
				job_log(j, LOG_NOTICE, "Exit timeout elapsed. Entering the kernel debugger");
				(void)job_assumes_zero(j, host_reboot(mach_host_self(), HOST_REBOOT_DEBUGGER));
			}

			job_log(j, LOG_WARNING | LOG_CONSOLE, "Exit timeout elapsed (%u seconds). Killing", j->exit_timeout);
			job_kill(j);
		}
	} else {
		job_log(j, LOG_ERR, "Unrecognized job timer callback: %p", ident);
	}
}

void
job_callback_read(job_t j, int ident)
{
	if (ident == j->stdin_fd) {
		job_dispatch(j, true);
	} else {
		socketgroup_callback(j);
	}
}

void
jobmgr_reap_bulk(jobmgr_t jm, struct kevent *kev)
{
	jobmgr_t jmi;
	job_t j;

	SLIST_FOREACH(jmi, &jm->submgrs, sle) {
		jobmgr_reap_bulk(jmi, kev);
	}

	if ((j = jobmgr_find_by_pid(jm, (pid_t)kev->ident, false))) {
		kev->udata = j;
		job_callback(j, kev);
	}
}

void
jobmgr_callback(void *obj, struct kevent *kev)
{
	jobmgr_t jm = obj;

#if TARGET_OS_EMBEDDED
	int flag2check = VQ_MOUNT;
#else
	int flag2check = VQ_UPDATE;
#endif

	switch (kev->filter) {
	case EVFILT_PROC:
		jobmgr_reap_bulk(jm, kev);
		root_jobmgr = jobmgr_do_garbage_collection(root_jobmgr);
		break;
	case EVFILT_SIGNAL:
		switch (kev->ident) {
		case SIGTERM:			
			jobmgr_log(jm, LOG_DEBUG, "Got SIGTERM. Shutting down.");
			return launchd_shutdown();
		case SIGUSR1:
			return calendarinterval_callback();
		case SIGUSR2:
			// Turn on all logging.
			launchd_log_perf = true;
			launchd_log_debug = true;
			launchd_log_shutdown = true;
			/* Hopefully /var is available by this point. If not, uh, oh well.
			 * It's just a debugging facility.
			 */
			return jobmgr_log_perf_statistics(jm, false);
		case SIGINFO:
			return jobmgr_log_perf_statistics(jm, true);
		default:
			jobmgr_log(jm, LOG_ERR, "Unrecognized signal: %lu: %s", kev->ident, strsignal(kev->ident));
		}
		break;
	case EVFILT_FS:
		if (kev->fflags & flag2check) {
			if (!launchd_var_available) {
				struct stat sb;
				if (stat("/var/log", &sb) == 0 && (sb.st_mode & S_IWUSR)) {
					launchd_var_available = true;
				}
			}
		} else if (kev->fflags & VQ_MOUNT) {
			jobmgr_dispatch_all(jm, true);
		}
		jobmgr_dispatch_all_semaphores(jm);
		break;
	case EVFILT_TIMER:
		if (kev->ident == (uintptr_t)&sorted_calendar_events) {
			calendarinterval_callback();
		} else if (kev->ident == (uintptr_t)jm) {
			jobmgr_log(jm, LOG_DEBUG, "Shutdown timer firing.");
			jobmgr_still_alive_with_check(jm);
		} else if (kev->ident == (uintptr_t)&jm->reboot_flags) {
			jobmgr_do_garbage_collection(jm);
		} else if (kev->ident == (uintptr_t)&launchd_runtime_busy_time) {
			jobmgr_log(jm, LOG_DEBUG, "Idle exit timer fired. Shutting down.");
			if (jobmgr_assumes_zero(jm, runtime_busy_cnt) == 0) {
				return launchd_shutdown();
			}
#if HAVE_SYSTEMSTATS
		} else if (kev->ident == (uintptr_t)systemstats_timer_callback) {
			systemstats_timer_callback();
#endif
		}
		break;
	case EVFILT_VNODE:
		if (kev->ident == (uintptr_t)s_no_hang_fd) {
			int _no_hang_fd = open("/dev/autofs_nowait", O_EVTONLY | O_NONBLOCK);
			if (unlikely(_no_hang_fd != -1)) {
				jobmgr_log(root_jobmgr, LOG_DEBUG, "/dev/autofs_nowait has appeared!");
				(void)jobmgr_assumes_zero_p(root_jobmgr, kevent_mod((uintptr_t)s_no_hang_fd, EVFILT_VNODE, EV_DELETE, 0, 0, NULL));
				(void)jobmgr_assumes_zero_p(root_jobmgr, runtime_close(s_no_hang_fd));
				s_no_hang_fd = _fd(_no_hang_fd);
			}
		} else if (pid1_magic && launchd_console && kev->ident == (uintptr_t)fileno(launchd_console)) {
			int cfd = -1;
			if (jobmgr_assumes_zero_p(jm, cfd = open(_PATH_CONSOLE, O_WRONLY | O_NOCTTY)) != -1) {
				_fd(cfd);
				if (!(launchd_console = fdopen(cfd, "w"))) {
					(void)jobmgr_assumes_zero(jm, errno);
					(void)close(cfd);
				}
			}
		}
		break;
	default:
		jobmgr_log(jm, LOG_ERR, "Unrecognized kevent filter: %hd", kev->filter);
	}
}

void
job_callback(void *obj, struct kevent *kev)
{
	job_t j = obj;

	job_log(j, LOG_DEBUG, "Dispatching kevent callback.");

	switch (kev->filter) {
	case EVFILT_PROC:
		return job_callback_proc(j, kev);
	case EVFILT_TIMER:
		return job_callback_timer(j, (void *) kev->ident);
	case EVFILT_READ:
		return job_callback_read(j, (int) kev->ident);
	case EVFILT_MACHPORT:
		return (void)job_dispatch(j, true);
	default:
		job_log(j, LOG_ERR, "Unrecognized job callback filter: %hd", kev->filter);
	}
}

void
job_start(job_t j)
{
	uint64_t td;
	int spair[2];
	int execspair[2];
	char nbuf[64];
	pid_t c;
	bool sipc = false;
	u_int proc_fflags = NOTE_EXIT|NOTE_FORK|NOTE_EXEC|NOTE_EXIT_DETAIL|NOTE_EXITSTATUS;

	if (!job_assumes(j, j->mgr != NULL)) {
		return;
	}

	if (unlikely(job_active(j))) {
		job_log(j, LOG_DEBUG, "Already started");
		return;
	}

	if (!LIST_EMPTY(&j->mgr->attaches)) {
		job_log(j, LOG_DEBUG, "Looking for attachments for job: %s", j->label);
		(void)waiting4attach_find(j->mgr, j);
	}

	/*
	 * Some users adjust the wall-clock and then expect software to not notice.
	 * Therefore, launchd must use an absolute clock instead of the wall clock
	 * wherever possible.
	 */
	td = runtime_get_nanoseconds_since(j->start_time);
	td /= NSEC_PER_SEC;

	if (j->start_time && (td < j->min_run_time) && !j->legacy_mach_job && !j->inetcompat && !j->unthrottle) {
		time_t respawn_delta = j->min_run_time - (uint32_t)td;
		/* We technically should ref-count throttled jobs to prevent idle exit,
		 * but we're not directly tracking the 'throttled' state at the moment.
		 */
		job_log(j, LOG_NOTICE, "Throttling respawn: Will start in %ld seconds", respawn_delta);
		(void)job_assumes_zero_p(j, kevent_mod((uintptr_t)j, EVFILT_TIMER, EV_ADD|EV_ONESHOT, NOTE_SECONDS, respawn_delta, j));
		job_ignore(j);
		return;
	}

	if (likely(!j->legacy_mach_job)) {
		sipc = ((!SLIST_EMPTY(&j->sockets) || !SLIST_EMPTY(&j->machservices)) && !j->deny_job_creation) || j->embedded_god;
	}

	if (sipc) {
		(void)job_assumes_zero_p(j, socketpair(AF_UNIX, SOCK_STREAM, 0, spair));
	}

	(void)job_assumes_zero_p(j, socketpair(AF_UNIX, SOCK_STREAM, 0, execspair));

	switch (c = runtime_fork(j->weird_bootstrap ? j->j_port : j->mgr->jm_port)) {
	case -1:
		job_log_error(j, LOG_ERR, "fork() failed, will try again in one second");
		(void)job_assumes_zero_p(j, kevent_mod((uintptr_t)j, EVFILT_TIMER, EV_ADD|EV_ONESHOT, NOTE_SECONDS, 1, j));
		job_ignore(j);

		(void)job_assumes_zero(j, runtime_close(execspair[0]));
		(void)job_assumes_zero(j, runtime_close(execspair[1]));
		if (sipc) {
			(void)job_assumes_zero(j, runtime_close(spair[0]));
			(void)job_assumes_zero(j, runtime_close(spair[1]));
		}
		break;
	case 0:
		if (unlikely(_vproc_post_fork_ping())) {
			_exit(EXIT_FAILURE);
		}

		(void)job_assumes_zero(j, runtime_close(execspair[0]));
		// wait for our parent to say they've attached a kevent to us
		read(_fd(execspair[1]), &c, sizeof(c));

		if (sipc) {
			(void)job_assumes_zero(j, runtime_close(spair[0]));
			snprintf(nbuf, sizeof(nbuf), "%d", spair[1]);
			setenv(LAUNCHD_TRUSTED_FD_ENV, nbuf, 1);
		}
		job_start_child(j);
		break;
	default:
		j->start_time = runtime_get_opaque_time();

		job_log(j, LOG_DEBUG, "Started as PID: %u", c);

		j->did_exec = false;
		j->fpfail = false;
		j->jettisoned = false;
		j->xpcproxy_did_exec = false;
		j->checkedin = false;
		j->start_pending = false;
		j->reaped = false;
		j->crashed = false;
		j->stopped = false;
		j->workaround9359725 = false;
		j->implicit_reap = false;
		j->unthrottle = false;
		if (j->needs_kickoff) {
			j->needs_kickoff = false;

			if (SLIST_EMPTY(&j->semaphores)) {
				j->ondemand = false;
			}
		}

		if (j->has_console) {
			launchd_wsp = c;
		}

		job_log(j, LOG_PERF, "Job started.");
		runtime_add_ref();
		total_children++;
		LIST_INSERT_HEAD(&j->mgr->active_jobs[ACTIVE_JOB_HASH(c)], j, pid_hash_sle);
		LIST_INSERT_HEAD(&managed_actives[ACTIVE_JOB_HASH(c)], j, global_pid_hash_sle);
		j->p = c;

		struct proc_uniqidentifierinfo info;
		if (proc_pidinfo(c, PROC_PIDUNIQIDENTIFIERINFO, 0, &info, PROC_PIDUNIQIDENTIFIERINFO_SIZE) != 0) {
			// ignore errors here, kevent_mod below will catch them and clean up
			j->uniqueid = info.p_uniqueid;
		}

		j->mgr->normal_active_cnt++;
		j->fork_fd = _fd(execspair[0]);
		(void)job_assumes_zero(j, runtime_close(execspair[1]));
		if (sipc) {
			(void)job_assumes_zero(j, runtime_close(spair[1]));
			ipc_open(_fd(spair[0]), j);
		}
		if (kevent_mod(c, EVFILT_PROC, EV_ADD, proc_fflags, 0, root_jobmgr ? root_jobmgr : j->mgr) != -1) {
			job_ignore(j);
		} else {
			if (errno == ESRCH) {
				job_log(j, LOG_ERR, "Child was killed before we could attach a kevent.");
			} else {
				(void)job_assumes(j, errno == ESRCH);
			}
			job_reap(j);

			/* If we have reaped this job within this same run loop pass, then
			 * it will be currently ignored. So if there's a failure to attach a
			 * kevent, we need to make sure that we watch the job so that we can
			 * respawn it.
			 *
			 * See <rdar://problem/10140809>.
			 */
			job_watch(j);
		}

#if HAVE_SYSTEMSTATS
		if (systemstats_is_enabled()) {
			/* We don't really *need* to make the full rusage call -- it
			 * will be mostly 0s and very small numbers. We only need
			 * ri_proc_start_abstime, because that's how we disambiguiate
			 * PIDs when they wrap around; and the UUID.
			 * In the future we should use the 64-bit process unique ID,
			 * so there's nothing to disambiguiate, and skip the full
			 * rusage call here.
			 *
			 * Well, the future is now.
			 */
			if (_systemstats_get_property(SYSTEMSTATS_API_VERSION, SYSTEMSTATS_WRITER_launchd, SYSTEMSTATS_PROPERTY_LAUNCHD_SHOULD_LOG_JOB_START)) {
				job_log_perf_statistics(j, NULL, -3);
			}
		}
#endif
		j->wait4debugger_oneshot = false;
		if (likely(!j->stall_before_exec)) {
			job_uncork_fork(j);
		}
		break;
	}
}

void
job_start_child(job_t j)
{
	typeof(posix_spawn) *psf;
	const char *file2exec = "/usr/libexec/launchproxy";
	const char **argv;
	posix_spawnattr_t spattr;
	int gflags = GLOB_NOSORT|GLOB_NOCHECK|GLOB_TILDE|GLOB_DOOFFS;
	glob_t g;
	short spflags = POSIX_SPAWN_SETEXEC;
	int psproctype = POSIX_SPAWN_PROC_TYPE_DAEMON_BACKGROUND;
	size_t binpref_out_cnt = 0;
	size_t i;

	(void)job_assumes_zero(j, posix_spawnattr_init(&spattr));

	job_setup_attributes(j);

	bool use_xpcproxy = false;
	struct waiting4attach *w4a = waiting4attach_find(j->mgr, j);
	if (w4a) {
		(void)setenv(XPC_SERVICE_ENV_ATTACHED, "1", 1);
		if (!j->xpc_service) {
			use_xpcproxy = true;
		}
	}

	if (use_xpcproxy) {
		argv = alloca(3 * sizeof(char *));
		argv[0] = "/usr/libexec/xpcproxy";
		argv[1] = "-debug";
		argv[2] = NULL;

		file2exec = argv[0];
	} else if (unlikely(j->argv && j->globargv)) {
		g.gl_offs = 1;
		for (i = 0; i < j->argc; i++) {
			if (i > 0) {
				gflags |= GLOB_APPEND;
			}
			if (glob(j->argv[i], gflags, NULL, &g) != 0) {
				job_log_error(j, LOG_ERR, "glob(\"%s\")", j->argv[i]);
				exit(EXIT_FAILURE);
			}
		}
		g.gl_pathv[0] = (char *)file2exec;
		argv = (const char **)g.gl_pathv;
	} else if (likely(j->argv)) {
		argv = alloca((j->argc + 2) * sizeof(char *));
		argv[0] = file2exec;
		for (i = 0; i < j->argc; i++) {
			argv[i + 1] = j->argv[i];
		}
		argv[i + 1] = NULL;
	} else {
		argv = alloca(3 * sizeof(char *));
		argv[0] = file2exec;
		argv[1] = j->prog;
		argv[2] = NULL;
	}

	if (likely(!(j->inetcompat || use_xpcproxy))) {
		argv++;
	}

	if (unlikely(j->wait4debugger || j->wait4debugger_oneshot)) {
		if (!j->app) {
			job_log(j, LOG_WARNING, "Spawned and waiting for the debugger to attach before continuing...");
		}
		spflags |= POSIX_SPAWN_START_SUSPENDED;
	}

#if !TARGET_OS_EMBEDDED
	if (unlikely(j->disable_aslr)) {
		spflags |= _POSIX_SPAWN_DISABLE_ASLR;
	}
#endif
	spflags |= j->pstype;

	(void)job_assumes_zero(j, posix_spawnattr_setflags(&spattr, spflags));
	if (unlikely(j->j_binpref_cnt)) {
		(void)job_assumes_zero(j, posix_spawnattr_setbinpref_np(&spattr, j->j_binpref_cnt, j->j_binpref, &binpref_out_cnt));
		(void)job_assumes(j, binpref_out_cnt == j->j_binpref_cnt);
	}

	psproctype = j->psproctype;
	(void)job_assumes_zero(j, posix_spawnattr_setprocesstype_np(&spattr, psproctype));

#if TARGET_OS_EMBEDDED
	/* Set jetsam attributes. POSIX_SPAWN_JETSAM_USE_EFFECTIVE_PRIORITY guards
	 * against a race which arises if, during spawn, an initial jetsam property
	 * update occurs before the values below are applied. In this case, the flag
	 * ensures that the subsequent change is ignored; the explicit update should
	 * be given priority.
	 */
	(void)job_assumes_zero(j, posix_spawnattr_setjetsam(&spattr,
	        POSIX_SPAWN_JETSAM_USE_EFFECTIVE_PRIORITY | (j->jetsam_memory_limit_background ? POSIX_SPAWN_JETSAM_HIWATER_BACKGROUND : 0), 
	        j->jetsam_priority, j->jetsam_memlimit));
#endif

	mach_port_array_t sports = NULL;
	mach_msg_type_number_t sports_cnt = 0;
	kern_return_t kr = vproc_mig_get_listener_port_rights(bootstrap_port, &sports, &sports_cnt);
	if (kr == 0 && sports_cnt) {
		/* For some reason, this SPI takes a count as a signed quantity. */
		(void)posix_spawnattr_set_importancewatch_port_np(&spattr, (int)sports_cnt, sports);

		/* All "count" parameters in MIG are counts of the array. So an array of
		 * mach_port_t containing 10 elements will have a count of ten, but it
		 * will occupy 40 bytes. So we must do the multiplication here to pass
		 * the correct size.
		 *
		 * Note that we do NOT release the send rights. We need them to be valid
		 * at the time they are passed to posix_spawn(2). When we exec(3) using
		 * posix_spawn(2), they'll be cleaned up anyway.
		 */
		mig_deallocate((vm_address_t)sports, sports_cnt * sizeof(sports[0]));
	} else if (kr != BOOTSTRAP_UNKNOWN_SERVICE) {
		(void)job_assumes_zero(j, kr);
	}

#if TARGET_OS_EMBEDDED
	if (!j->app || j->system_app) {
		(void)job_assumes_zero(j, posix_spawnattr_setcpumonitor_default(&spattr));
	}
#else
	(void)job_assumes_zero(j, posix_spawnattr_setcpumonitor_default(&spattr));
#endif

#if !TARGET_OS_EMBEDDED
	struct task_qos_policy qosinfo = {
		.task_latency_qos_tier = LATENCY_QOS_LAUNCH_DEFAULT_TIER,
		.task_throughput_qos_tier = THROUGHPUT_QOS_LAUNCH_DEFAULT_TIER,
	};

	if (!j->legacy_timers) {
		kr = task_policy_set(mach_task_self(), TASK_BASE_QOS_POLICY, (task_policy_t)&qosinfo, TASK_QOS_POLICY_COUNT);
		(void)job_assumes_zero_p(j, kr);
	}
#endif

#if HAVE_RESPONSIBILITY
	/* Specify which process is responsible for the new job.  Per-app XPC
	 * services are the responsibility of the app.  Other processes are
	 * responsible for themselves.  This decision is final and also applies
	 * to the process's children, so don't initialize responsibility when
	 * starting a per-user launchd.
	 */
	if (j->mgr->req_pid) {
		responsibility_init2(j->mgr->req_pid, NULL);
	} else if (!j->per_user) {
		responsibility_init2(getpid(), j->prog ? j->prog : j->argv[0]);
	}
#endif

#if HAVE_QUARANTINE
	if (j->quarantine_data) {
		qtn_proc_t qp;

		if (job_assumes(j, qp = qtn_proc_alloc())) {
			if (job_assumes_zero(j, qtn_proc_init_with_data(qp, j->quarantine_data, j->quarantine_data_sz) == 0)) {
				(void)job_assumes_zero(j, qtn_proc_apply_to_self(qp));
			}
		}
	}
#endif

#if HAVE_SANDBOX
#if TARGET_OS_EMBEDDED
	struct sandbox_spawnattrs sbattrs;
	if (j->seatbelt_profile || j->container_identifier) {
		sandbox_spawnattrs_init(&sbattrs);
		if (j->seatbelt_profile) {
			sandbox_spawnattrs_setprofilename(&sbattrs, j->seatbelt_profile);
		}
		if (j->container_identifier) {
			sandbox_spawnattrs_setcontainer(&sbattrs, j->container_identifier);
		}
		(void)job_assumes_zero(j, posix_spawnattr_setmacpolicyinfo_np(&spattr, "Sandbox", &sbattrs, sizeof(sbattrs)));
	}
#else
	if (j->seatbelt_profile) {
		char *seatbelt_err_buf = NULL;

		if (job_assumes_zero_p(j, sandbox_init(j->seatbelt_profile, j->seatbelt_flags, &seatbelt_err_buf)) == -1) {
			if (seatbelt_err_buf) {
				job_log(j, LOG_ERR, "Sandbox failed to init: %s", seatbelt_err_buf);
			}
			goto out_bad;
		}
	}
#endif
#endif

	psf = j->prog ? posix_spawn : posix_spawnp;

	if (likely(!(j->inetcompat || use_xpcproxy))) {
		file2exec = j->prog ? j->prog : argv[0];
	}

	errno = psf(NULL, file2exec, NULL, &spattr, (char *const *)argv, environ);

#if HAVE_SANDBOX && !TARGET_OS_EMBEDDED
out_bad:
#endif
	_exit(errno);
}

void
jobmgr_export_env_from_other_jobs(jobmgr_t jm, launch_data_t dict)
{
	launch_data_t tmp;
	struct envitem *ei;
	job_t ji;

	if (jm->parentmgr) {
		jobmgr_export_env_from_other_jobs(jm->parentmgr, dict);
	} else {
		char **tmpenviron = environ;
		for (; *tmpenviron; tmpenviron++) {
			char envkey[1024];
			launch_data_t s = launch_data_alloc(LAUNCH_DATA_STRING);
			launch_data_set_string(s, strchr(*tmpenviron, '=') + 1);
			strncpy(envkey, *tmpenviron, sizeof(envkey));
			*(strchr(envkey, '=')) = '\0';
			launch_data_dict_insert(dict, s, envkey);
		}
	}

	LIST_FOREACH(ji, &jm->jobs, sle) {
		SLIST_FOREACH(ei, &ji->global_env, sle) {
			if ((tmp = launch_data_new_string(ei->value))) {
				launch_data_dict_insert(dict, tmp, ei->key);
			}
		}
	}
}

void
jobmgr_setup_env_from_other_jobs(jobmgr_t jm)
{
	struct envitem *ei;
	job_t ji;

	if (jm->parentmgr) {
		jobmgr_setup_env_from_other_jobs(jm->parentmgr);
	}

	LIST_FOREACH(ji, &jm->global_env_jobs, global_env_sle) {
		SLIST_FOREACH(ei, &ji->global_env, sle) {
			setenv(ei->key, ei->value, 1);
		}
	}
}

void
job_log_pids_with_weird_uids(job_t j)
{
	size_t len = sizeof(pid_t) * get_kern_max_proc();
	pid_t *pids = NULL;
	uid_t u = j->mach_uid;
	int i = 0, kp_cnt = 0;

	if (!launchd_apple_internal) {
		return;
	}

	pids = malloc(len);
	if (!job_assumes(j, pids != NULL)) {
		return;
	}

	runtime_ktrace(RTKT_LAUNCHD_FINDING_WEIRD_UIDS, j->p, u, 0);

	/* libproc actually has some serious performance drawbacks when used over sysctl(3) in
	 * scenarios like this. Whereas sysctl(3) can give us back all the kinfo_proc's in
	 * one kernel call, libproc requires that we get a list of PIDs we're interested in
	 * (in this case, all PIDs on the system) and then get a single proc_bsdshortinfo
	 * struct back in a single call for each one.
	 *
	 * This kind of thing is also more inherently racy than sysctl(3). While sysctl(3)
	 * returns a snapshot, it returns the whole shebang at once. Any PIDs given to us by
	 * libproc could go stale before we call proc_pidinfo().
	 *
	 * Note that proc_list*() APIs return the number of PIDs given back, not the number
	 * of bytes written to the buffer.
	 */
	if (job_assumes_zero_p(j, (kp_cnt = proc_listallpids(pids, len))) == -1) {
		goto out;
	}

	for (i = 0; i < kp_cnt; i++) {
		struct proc_bsdshortinfo proc;
		/* We perhaps should not log a bug here if we get ESRCH back, due to the race
		 * detailed above.
		 */
		if (proc_pidinfo(pids[i], PROC_PIDT_SHORTBSDINFO, 1, &proc, PROC_PIDT_SHORTBSDINFO_SIZE) == 0) {
			if (errno != ESRCH) {
				(void)job_assumes_zero(j, errno);
			}
			continue;
		}

		uid_t i_euid = proc.pbsi_uid;
		uid_t i_uid = proc.pbsi_ruid;
		uid_t i_svuid = proc.pbsi_svuid;
		pid_t i_pid = pids[i];

		if (i_euid != u && i_uid != u && i_svuid != u) {
			continue;
		}

		job_log(j, LOG_ERR, "PID %u \"%s\" has no account to back it! Real/effective/saved UIDs: %u/%u/%u", i_pid, proc.pbsi_comm, i_uid, i_euid, i_svuid);

// Temporarily disabled due to 5423935 and 4946119.
#if 0
		// Ask the accountless process to exit.
		(void)job_assumes_zero_p(j, kill2(i_pid, SIGTERM));
#endif
	}

out:
	free(pids);
}

static struct passwd *
job_getpwnam(job_t j, const char *name)
{
	/*
	 * methodology for system daemons
	 *
	 * first lookup user record without any opendirectoryd interaction,
	 * we don't know what interprocess dependencies might be in flight.
	 * if that fails, we re-enable opendirectoryd interaction and 
	 * re-issue the lookup.  We have to disable the libinfo L1 cache
	 * otherwise libinfo will return the negative cache entry on the retry
	 */
#if !TARGET_OS_EMBEDDED
	struct passwd *pw = NULL;

	if (pid1_magic && j->mgr == root_jobmgr) {
		// 1 == SEARCH_MODULE_FLAG_DISABLED
		si_search_module_set_flags("ds", 1);
		gL1CacheEnabled = false;

		pw = getpwnam(name);
		si_search_module_set_flags("ds", 0);
	}

	if (pw == NULL) {
		pw = getpwnam(name);
	}

	return pw;
#else
#pragma unused (j)
	return getpwnam(name);
#endif
}

static struct group *
job_getgrnam(job_t j, const char *name)
{
#if !TARGET_OS_EMBEDDED
    struct group *gr = NULL;

    if (pid1_magic && j->mgr == root_jobmgr) {
        si_search_module_set_flags("ds", 1);
        gL1CacheEnabled = false;

        gr = getgrnam(name);

        si_search_module_set_flags("ds", 0);
    }

    if (gr == NULL) {
        gr = getgrnam(name);
    }

    return gr;
#else
#pragma unused (j)
    return getgrnam(name);
#endif
}

void
job_postfork_test_user(job_t j)
{
	// This function is all about 5201578

	const char *home_env_var = getenv("HOME");
	const char *user_env_var = getenv("USER");
	const char *logname_env_var = getenv("LOGNAME");
	uid_t tmp_uid, local_uid = getuid();
	gid_t tmp_gid, local_gid = getgid();
	char shellpath[PATH_MAX];
	char homedir[PATH_MAX];
	char loginname[2000];
	struct passwd *pwe;


	if (!job_assumes(j, home_env_var && user_env_var && logname_env_var
				&& strcmp(user_env_var, logname_env_var) == 0)) {
		goto out_bad;
	}

	if ((pwe = job_getpwnam(j, user_env_var)) == NULL) {
		job_log(j, LOG_ERR, "The account \"%s\" has been deleted out from under us!", user_env_var);
		goto out_bad;
	}

	/*
	 * We must copy the results of getpw*().
	 *
	 * Why? Because subsequent API calls may call getpw*() as a part of
	 * their implementation. Since getpw*() returns a [now thread scoped]
	 * global, we must therefore cache the results before continuing.
	 */

	tmp_uid = pwe->pw_uid;
	tmp_gid = pwe->pw_gid;

	strlcpy(shellpath, pwe->pw_shell, sizeof(shellpath));
	strlcpy(loginname, pwe->pw_name, sizeof(loginname));
	strlcpy(homedir, pwe->pw_dir, sizeof(homedir));

	if (strcmp(loginname, logname_env_var) != 0) {
		job_log(j, LOG_ERR, "The %s environmental variable changed out from under us!", "USER");
		goto out_bad;
	}
	if (strcmp(homedir, home_env_var) != 0) {
		job_log(j, LOG_ERR, "The %s environmental variable changed out from under us!", "HOME");
		goto out_bad;
	}
	if (local_uid != tmp_uid) {
		job_log(j, LOG_ERR, "The %cID of the account (%u) changed out from under us (%u)!",
				'U', tmp_uid, local_uid);
		goto out_bad;
	}
	if (local_gid != tmp_gid) {
		job_log(j, LOG_ERR, "The %cID of the account (%u) changed out from under us (%u)!",
				'G', tmp_gid, local_gid);
		goto out_bad;
	}

	return;
out_bad:
#if 0
	(void)job_assumes_zero_p(j, kill2(getppid(), SIGTERM));
	_exit(EXIT_FAILURE);
#else
	job_log(j, LOG_WARNING, "In a future build of the OS, this error will be fatal.");
#endif
}

void
job_postfork_become_user(job_t j)
{
	char loginname[2000];
	char tmpdirpath[PATH_MAX];
	char shellpath[PATH_MAX];
	char homedir[PATH_MAX];
	struct passwd *pwe;
	size_t r;
	gid_t desired_gid = -1;
	uid_t desired_uid = -1;

	if (getuid() != 0) {
		return job_postfork_test_user(j);
	}

	/*
	 * I contend that having UID == 0 and GID != 0 is of dubious value.
	 * Nevertheless, this used to work in Tiger. See: 5425348
	 */
	if (j->groupname && !j->username) {
		j->username = "root";
	}

	if (j->username) {
		if ((pwe = job_getpwnam(j, j->username)) == NULL) {
			job_log(j, LOG_ERR, "getpwnam(\"%s\") failed", j->username);
			_exit(ESRCH);
		}
	} else if (j->mach_uid) {
		if ((pwe = getpwuid(j->mach_uid)) == NULL) {
			job_log(j, LOG_ERR, "getpwuid(\"%u\") failed", j->mach_uid);
			job_log_pids_with_weird_uids(j);
			_exit(ESRCH);
		}
	} else {
		return;
	}

	/*
	 * We must copy the results of getpw*().
	 *
	 * Why? Because subsequent API calls may call getpw*() as a part of
	 * their implementation. Since getpw*() returns a [now thread scoped]
	 * global, we must therefore cache the results before continuing.
	 */

	desired_uid = pwe->pw_uid;
	desired_gid = pwe->pw_gid;

	strlcpy(shellpath, pwe->pw_shell, sizeof(shellpath));
	strlcpy(loginname, pwe->pw_name, sizeof(loginname));
	strlcpy(homedir, pwe->pw_dir, sizeof(homedir));

	if (unlikely(pwe->pw_expire && time(NULL) >= pwe->pw_expire)) {
		job_log(j, LOG_ERR, "Expired account");
		_exit(EXIT_FAILURE);
	}


	if (unlikely(j->username && strcmp(j->username, loginname) != 0)) {
		job_log(j, LOG_WARNING, "Suspicious setup: User \"%s\" maps to user: %s", j->username, loginname);
	} else if (unlikely(j->mach_uid && (j->mach_uid != desired_uid))) {
		job_log(j, LOG_WARNING, "Suspicious setup: UID %u maps to UID %u", j->mach_uid, desired_uid);
	}

	if (j->groupname) {
		struct group *gre;

		if (unlikely((gre = job_getgrnam(j, j->groupname)) == NULL)) {
			job_log(j, LOG_ERR, "getgrnam(\"%s\") failed", j->groupname);
			_exit(ESRCH);
		}

		desired_gid = gre->gr_gid;
	}

	if (job_assumes_zero_p(j, setlogin(loginname)) == -1) {
		_exit(EXIT_FAILURE);
	}

	if (job_assumes_zero_p(j, setgid(desired_gid)) == -1) {
		_exit(EXIT_FAILURE);
	}

	/*
	 * The kernel team and the DirectoryServices team want initgroups()
	 * called after setgid(). See 4616864 for more information.
	 */

	if (likely(!j->no_init_groups)) {
#if 1
		if (job_assumes_zero_p(j, initgroups(loginname, desired_gid)) == -1) {
			_exit(EXIT_FAILURE);
		}
#else
		/* Do our own little initgroups(). We do this to guarantee that we're
		 * always opted into dynamic group resolution in the kernel. initgroups(3)
		 * does not make this guarantee.
		 */
		int groups[NGROUPS], ngroups;

		// A failure here isn't fatal, and we'll still get data we can use.
		(void)job_assumes_zero_p(j, getgrouplist(j->username, desired_gid, groups, &ngroups));

		if (job_assumes_zero_p(j, syscall(SYS_initgroups, ngroups, groups, desired_uid)) == -1) {
			_exit(EXIT_FAILURE);
		}
#endif
	}

	if (job_assumes_zero_p(j, setuid(desired_uid)) == -1) {
		_exit(EXIT_FAILURE);
	}

	r = confstr(_CS_DARWIN_USER_TEMP_DIR, tmpdirpath, sizeof(tmpdirpath));

	if (likely(r > 0 && r < sizeof(tmpdirpath))) {
		setenv("TMPDIR", tmpdirpath, 0);
	}

	setenv("SHELL", shellpath, 0);
	setenv("HOME", homedir, 0);
	setenv("USER", loginname, 0);
	setenv("LOGNAME", loginname, 0);
}

void
job_setup_attributes(job_t j)
{
	struct limititem *li;
	struct envitem *ei;

	if (unlikely(j->setnice)) {
		(void)job_assumes_zero_p(j, setpriority(PRIO_PROCESS, 0, j->nice));
	}

	SLIST_FOREACH(li, &j->limits, sle) {
		struct rlimit rl;

		if (job_assumes_zero_p(j, getrlimit(li->which, &rl) == -1)) {
			continue;
		}

		if (li->sethard) {
			rl.rlim_max = li->lim.rlim_max;
		}
		if (li->setsoft) {
			rl.rlim_cur = li->lim.rlim_cur;
		}

		if (setrlimit(li->which, &rl) == -1) {
			job_log_error(j, LOG_WARNING, "setrlimit()");
		}
	}

	if (unlikely(!j->inetcompat && j->session_create)) {
		launchd_SessionCreate();
	}

	if (unlikely(j->low_pri_io)) {
		(void)job_assumes_zero_p(j, setiopolicy_np(IOPOL_TYPE_DISK, IOPOL_SCOPE_PROCESS, IOPOL_THROTTLE));
	}
	if (j->low_priority_background_io) {
		(void)job_assumes_zero_p(j, setiopolicy_np(IOPOL_TYPE_DISK, IOPOL_SCOPE_DARWIN_BG, IOPOL_THROTTLE));
	}
	if (unlikely(j->rootdir)) {
		(void)job_assumes_zero_p(j, chroot(j->rootdir));
		(void)job_assumes_zero_p(j, chdir("."));
	}

	job_postfork_become_user(j);

	if (unlikely(j->workingdir)) {
		if (chdir(j->workingdir) == -1) {
			if (errno == ENOENT || errno == ENOTDIR) {
				job_log(j, LOG_ERR, "Job specified non-existent working directory: %s", j->workingdir);
			} else {
				(void)job_assumes_zero(j, errno);
			}
		}
	}

	if (unlikely(j->setmask)) {
		umask(j->mask);
	}

	if (j->stdin_fd) {
		(void)job_assumes_zero_p(j, dup2(j->stdin_fd, STDIN_FILENO));
	} else {
		job_setup_fd(j, STDIN_FILENO, j->stdinpath, O_RDONLY|O_CREAT);
	}
	job_setup_fd(j, STDOUT_FILENO, j->stdoutpath, O_WRONLY|O_CREAT|O_APPEND);
	job_setup_fd(j, STDERR_FILENO, j->stderrpath, O_WRONLY|O_CREAT|O_APPEND);

	jobmgr_setup_env_from_other_jobs(j->mgr);

	SLIST_FOREACH(ei, &j->env, sle) {
		setenv(ei->key, ei->value, 1);
	}

#if !TARGET_OS_EMBEDDED	
	if (j->jetsam_properties) {
		(void)job_assumes_zero(j, proc_setpcontrol(PROC_SETPC_TERMINATE));
	}
#endif

#if TARGET_OS_EMBEDDED
	if (j->main_thread_priority != 0) {
		struct sched_param params;
		bzero(&params, sizeof(params));
		params.sched_priority = j->main_thread_priority;
		(void)job_assumes_zero_p(j, pthread_setschedparam(pthread_self(), SCHED_OTHER, &params));
	}
#endif

	/*
	 * We'd like to call setsid() unconditionally, but we have reason to
	 * believe that prevents launchd from being able to send signals to
	 * setuid children. We'll settle for process-groups.
	 */
	if (getppid() != 1) {
		(void)job_assumes_zero_p(j, setpgid(0, 0));
	} else {
		(void)job_assumes_zero_p(j, setsid());
	}
}

void
job_setup_fd(job_t j, int target_fd, const char *path, int flags)
{
	int fd;

	if (!path) {
		return;
	}

	if ((fd = open(path, flags|O_NOCTTY, DEFFILEMODE)) == -1) {
		job_log_error(j, LOG_WARNING, "open(\"%s\", ...)", path);
		return;
	}

	(void)job_assumes_zero_p(j, dup2(fd, target_fd));
	(void)job_assumes_zero(j, runtime_close(fd));
}

void
calendarinterval_setalarm(job_t j, struct calendarinterval *ci)
{
	struct calendarinterval *ci_iter, *ci_prev = NULL;
	time_t later, head_later;

	later = cronemu(ci->when.tm_mon, ci->when.tm_mday, ci->when.tm_hour, ci->when.tm_min);

	if (ci->when.tm_wday != -1) {
		time_t otherlater = cronemu_wday(ci->when.tm_wday, ci->when.tm_hour, ci->when.tm_min);

		if (ci->when.tm_mday == -1) {
			later = otherlater;
		} else {
			later = later < otherlater ? later : otherlater;
		}
	}

	ci->when_next = later;

	LIST_FOREACH(ci_iter, &sorted_calendar_events, global_sle) {
		if (ci->when_next < ci_iter->when_next) {
			LIST_INSERT_BEFORE(ci_iter, ci, global_sle);
			break;
		}

		ci_prev = ci_iter;
	}

	if (ci_iter == NULL) {
		// ci must want to fire after every other timer, or there are no timers

		if (LIST_EMPTY(&sorted_calendar_events)) {
			LIST_INSERT_HEAD(&sorted_calendar_events, ci, global_sle);
		} else {
			LIST_INSERT_AFTER(ci_prev, ci, global_sle);
		}
	}

	head_later = LIST_FIRST(&sorted_calendar_events)->when_next;

	if (job_assumes_zero_p(j, kevent_mod((uintptr_t)&sorted_calendar_events, EVFILT_TIMER, EV_ADD, NOTE_ABSOLUTE|NOTE_SECONDS, head_later, root_jobmgr)) != -1) {
		char time_string[100];
		size_t time_string_len;

		ctime_r(&later, time_string);
		time_string_len = strlen(time_string);

		if (likely(time_string_len && time_string[time_string_len - 1] == '\n')) {
			time_string[time_string_len - 1] = '\0';
		}

		job_log(j, LOG_INFO, "Scheduled to run again at %s", time_string);
	}
}

bool
jobmgr_log_bug(_SIMPLE_STRING asl_message __attribute__((unused)), void *ctx, const char *message)
{
	jobmgr_t jm = ctx;
	jobmgr_log(jm, LOG_ERR, "%s", message);

	return true;
}

bool
job_log_bug(_SIMPLE_STRING asl_message __attribute__((unused)), void *ctx, const char *message)
{
	job_t j = ctx;
	job_log(j, LOG_ERR, "%s", message);

	return true;
}

// ri: NULL = please sample j->p; non-NULL = use this sample
void
job_log_perf_statistics(job_t j, struct rusage_info_v1 *ri, int64_t exit_status)
{
#if HAVE_SYSTEMSTATS
	if (j->anonymous || !j->p) {
		return;
	}
	if (!systemstats_is_enabled()) {
		return;
	}
	const char *name;
	if (j->cfbundleidentifier) {
		name = j->cfbundleidentifier;
	} else {
		name = j->label;
	}
	int r = 0;
	struct rusage_info_v1 ris;
	if (ri == NULL) {
		ri = &ris;
		r = proc_pid_rusage(j->p, RUSAGE_INFO_V1, (rusage_info_t)ri);
	}
	if (r == -1) {
		return;
	}
	job_log_systemstats(j->p, j->uniqueid, runtime_get_uniqueid(), j->mgr->req_pid, j->mgr->req_uniqueid, name, ri, exit_status);
#else
#pragma unused (j, ri, exit_status)
#endif
}

#if HAVE_SYSTEMSTATS
// ri: NULL = don't write fields from ri; non-NULL = use this sample
static
void
job_log_systemstats(pid_t pid, uint64_t uniqueid, uint64_t parent_uniqueid, pid_t req_pid, uint64_t req_uniqueid, const char *name, struct rusage_info_v1 *ri, int64_t exit_status)
{
	if (!systemstats_is_enabled()) {
		return;
	}

	struct systemstats_process_usage_s info;
	bzero(&info, sizeof(info));
	info.name = name;
	info.pid = pid;
	info.exit_status = exit_status;
	info.uid = getuid();
	info.ppid = getpid();
	info.responsible_pid = req_pid;

	if (likely(ri)) {
		info.macho_uuid = (const uint8_t *)&ri->ri_uuid;
		info.user_time = ri->ri_user_time;
		info.system_time = ri->ri_system_time;
		info.pkg_idle_wkups = ri->ri_pkg_idle_wkups;
		info.interrupt_wkups = ri->ri_interrupt_wkups;
		info.proc_start_abstime = ri->ri_proc_start_abstime;
		info.proc_exit_abstime = ri->ri_proc_exit_abstime;
#if SYSTEMSTATS_API_VERSION >= 20130319
		info.pageins = ri->ri_pageins;
		info.wired_size = ri->ri_wired_size;
		info.resident_size = ri->ri_resident_size;
		info.phys_footprint = ri->ri_phys_footprint;
		// info.purgeablesize = ???
#endif
#if SYSTEMSTATS_API_VERSION >= 20130328
		info.child_user_time = ri->ri_child_user_time;
		info.child_system_time = ri->ri_child_system_time;
		info.child_pkg_idle_wkups = ri->ri_child_pkg_idle_wkups;
		info.child_interrupt_wkups = ri->ri_child_interrupt_wkups;
		info.child_pageins = ri->ri_child_pageins;
		info.child_elapsed_abstime = ri->ri_child_elapsed_abstime;
#endif
	}
#if SYSTEMSTATS_API_VERSION >= 20130410
	info.uniqueid = uniqueid;
	info.parent_uniqueid = parent_uniqueid;
	info.responsible_uniqueid = req_uniqueid;
#endif
	systemstats_write_process_usage(&info);
}
#endif /* HAVE_SYSTEMSTATS */

struct waiting4attach *
waiting4attach_new(jobmgr_t jm, const char *name, mach_port_t port, pid_t dest, xpc_service_type_t type)
{
	size_t xtra = strlen(name) + 1;

	struct waiting4attach *w4a = malloc(sizeof(*w4a) + xtra);
	if (!w4a) {
		return NULL;
	}

	w4a->port = port;
	w4a->dest = dest;
	w4a->type = type;
	(void)strcpy(w4a->name, name);

	if (dest) {
		LIST_INSERT_HEAD(&_launchd_domain_waiters, w4a, le);
	} else {
		LIST_INSERT_HEAD(&jm->attaches, w4a, le);
	}


	(void)jobmgr_assumes_zero(jm, launchd_mport_notify_req(port, MACH_NOTIFY_DEAD_NAME));
	return w4a;
}

void
waiting4attach_delete(jobmgr_t jm, struct waiting4attach *w4a)
{
	jobmgr_log(jm, LOG_DEBUG, "Canceling dead-name notification for waiter port: 0x%x", w4a->port);

	LIST_REMOVE(w4a, le);

	mach_port_t previous = MACH_PORT_NULL;
	(void)jobmgr_assumes_zero(jm, mach_port_request_notification(mach_task_self(), w4a->port, MACH_NOTIFY_DEAD_NAME, 0, MACH_PORT_NULL, MACH_MSG_TYPE_MOVE_SEND_ONCE, &previous));
	if (previous) {
		(void)jobmgr_assumes_zero(jm, launchd_mport_deallocate(previous));
	}

	jobmgr_assumes_zero(jm, launchd_mport_deallocate(w4a->port));
	free(w4a);
}

struct waiting4attach *
waiting4attach_find(jobmgr_t jm, job_t j)
{
	char *name2use = (char *)j->label;
	if (j->app) {
		struct envitem *ei = NULL;
		SLIST_FOREACH(ei, &j->env, sle) {
			if (strcmp(ei->key, XPC_SERVICE_RENDEZVOUS_TOKEN) == 0) {
				name2use = ei->value;
				break;
			}
		}
	}

	struct waiting4attach *w4ai = NULL;
	LIST_FOREACH(w4ai, &jm->attaches, le) {
		if (strcmp(name2use, w4ai->name) == 0) {
			job_log(j, LOG_DEBUG, "Found attachment: %s", name2use);
			break;
		}
	}

	return w4ai;
}

void
job_logv(job_t j, int pri, int err, const char *msg, va_list ap)
{
	const char *label2use = j ? j->label : "com.apple.launchd.job-unknown";
	const char *mgr2use = j ? j->mgr->name : "com.apple.launchd.jobmanager-unknown";
	char *newmsg;
	int oldmask = 0;
	size_t newmsgsz;

	struct launchd_syslog_attr attr = {
		.from_name = launchd_label,
		.about_name = label2use,
		.session_name = mgr2use,
		.priority = pri,
		.from_uid = getuid(),
		.from_pid = getpid(),
		.about_pid = j ? j->p : 0,
	};

	/* Hack: If bootstrap_port is set, we must be on the child side of a
	 * fork(2), but before the exec*(3). Let's route the log message back to
	 * launchd proper.
	 */
	if (bootstrap_port) {
		return _vproc_logv(pri, err, msg, ap);
	}

	newmsgsz = strlen(msg) + 200;
	newmsg = alloca(newmsgsz);

	if (err) {
#if !TARGET_OS_EMBEDDED
		snprintf(newmsg, newmsgsz, "%s: %d: %s", msg, err, strerror(err));
#else
		snprintf(newmsg, newmsgsz, "(%s) %s: %d: %s", label2use, msg, err, strerror(err));
#endif
	} else {
#if !TARGET_OS_EMBEDDED
		snprintf(newmsg, newmsgsz, "%s", msg);
#else
		snprintf(newmsg, newmsgsz, "(%s) %s", label2use, msg);
#endif
	}

	if (j && unlikely(j->debug)) {
		oldmask = setlogmask(LOG_UPTO(LOG_DEBUG));
	}

	launchd_vsyslog(&attr, newmsg, ap);

	if (j && unlikely(j->debug)) {
		setlogmask(oldmask);
	}
}

void
job_log_error(job_t j, int pri, const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	job_logv(j, pri, errno, msg, ap);
	va_end(ap);
}

void
job_log(job_t j, int pri, const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	job_logv(j, pri, 0, msg, ap);
	va_end(ap);
}

#if 0
void
jobmgr_log_error(jobmgr_t jm, int pri, const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	jobmgr_logv(jm, pri, errno, msg, ap);
	va_end(ap);
}
#endif

void
jobmgr_log_perf_statistics(jobmgr_t jm, bool signal_children)
{
#if HAVE_SYSTEMSTATS
	// Log information for kernel_task and pid 1 launchd.
	if (systemstats_is_enabled() && pid1_magic && jm == root_jobmgr) {
#if SYSTEMSTATS_API_VERSION >= 20130328
		if (_systemstats_get_property(SYSTEMSTATS_API_VERSION, SYSTEMSTATS_WRITER_launchd, SYSTEMSTATS_PROPERTY_SHOULD_LOG_ENERGY_STATISTICS)) {
			systemstats_write_intel_energy_statistics(NULL);
		}
#else
		systemstats_write_intel_energy_statistics(NULL);
#endif
		job_log_systemstats(0, 0, 0, 0, 0, "com.apple.kernel", NULL, -1);
		job_log_systemstats(1, 1, 0, 1, 1, "com.apple.launchd", NULL, -1);
	}
#endif
	jobmgr_t jmi = NULL;
	SLIST_FOREACH(jmi, &jm->submgrs, sle) {
		jobmgr_log_perf_statistics(jmi, signal_children);
	}

	if (jm->xpc_singleton) {
		jobmgr_log(jm, LOG_PERF, "XPC Singleton Domain: %s", jm->shortdesc);
	} else if (jm->properties & BOOTSTRAP_PROPERTY_XPC_DOMAIN) {
		jobmgr_log(jm, LOG_PERF, "XPC Private Domain: %s", jm->owner);
	} else if (jm->properties & BOOTSTRAP_PROPERTY_EXPLICITSUBSET) {
		jobmgr_log(jm, LOG_PERF, "Created via bootstrap_subset()");
	}

	jobmgr_log(jm, LOG_PERF, "Jobs in job manager:");

	job_t ji = NULL;
	LIST_FOREACH(ji, &jm->jobs, sle) {
		job_log_perf_statistics(ji, NULL, -1);
		if (unlikely(signal_children) && unlikely(strstr(ji->label, "com.apple.launchd.peruser.") == ji->label)) {
			jobmgr_log(jm, LOG_PERF, "Sending SIGINFO to peruser launchd %d", ji->p);
			kill(ji->p, SIGINFO);
		}
	}

	jobmgr_log(jm, LOG_PERF, "End of job list.");
}

void
jobmgr_log(jobmgr_t jm, int pri, const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	jobmgr_logv(jm, pri, 0, msg, ap);
	va_end(ap);
}

void
jobmgr_logv(jobmgr_t jm, int pri, int err, const char *msg, va_list ap)
{
	if (!jm) {
		jm = root_jobmgr;
	}

	char *newmsg;
	char *newname;
	size_t i, o, jmname_len = strlen(jm->name), newmsgsz;

	newname = alloca((jmname_len + 1) * 2);
	newmsgsz = (jmname_len + 1) * 2 + strlen(msg) + 100;
	newmsg = alloca(newmsgsz);

	for (i = 0, o = 0; i < jmname_len; i++, o++) {
		if (jm->name[i] == '%') {
			newname[o] = '%';
			o++;
		}
		newname[o] = jm->name[i];
	}
	newname[o] = '\0';

	if (err) {
		snprintf(newmsg, newmsgsz, "%s: %s: %s", newname, msg, strerror(err));
	} else {
		snprintf(newmsg, newmsgsz, "%s: %s", newname, msg);
	}

	if (jm->parentmgr) {
		jobmgr_logv(jm->parentmgr, pri, 0, newmsg, ap);
	} else {
		struct launchd_syslog_attr attr = {
			.from_name = launchd_label,
			.about_name = launchd_label,
			.session_name = jm->name,
			.priority = pri,
			.from_uid = getuid(),
			.from_pid = getpid(),
			.about_pid = getpid(),
		};

		launchd_vsyslog(&attr, newmsg, ap);
	}
}

struct cal_dict_walk {
	job_t j;
	struct tm tmptm;
};

void
calendarinterval_new_from_obj_dict_walk(launch_data_t obj, const char *key, void *context)
{
	struct cal_dict_walk *cdw = context;
	struct tm *tmptm = &cdw->tmptm;
	job_t j = cdw->j;
	int64_t val;

	if (unlikely(LAUNCH_DATA_INTEGER != launch_data_get_type(obj))) {
		// hack to let caller know something went wrong
		tmptm->tm_sec = -1;
		return;
	}

	val = launch_data_get_integer(obj);

	if (val < 0) {
		job_log(j, LOG_WARNING, "The interval for key \"%s\" is less than zero.", key);
	} else if (strcasecmp(key, LAUNCH_JOBKEY_CAL_MINUTE) == 0) {
		if (val > 59) {
			job_log(j, LOG_WARNING, "The interval for key \"%s\" is not between 0 and 59 (inclusive).", key);
			tmptm->tm_sec = -1;
		} else {
			tmptm->tm_min = (typeof(tmptm->tm_min)) val;
		}
	} else if (strcasecmp(key, LAUNCH_JOBKEY_CAL_HOUR) == 0) {
		if (val > 23) {
			job_log(j, LOG_WARNING, "The interval for key \"%s\" is not between 0 and 23 (inclusive).", key);
			tmptm->tm_sec = -1;
		} else {
			tmptm->tm_hour = (typeof(tmptm->tm_hour)) val;
		}
	} else if (strcasecmp(key, LAUNCH_JOBKEY_CAL_DAY) == 0) {
		if (val < 1 || val > 31) {
			job_log(j, LOG_WARNING, "The interval for key \"%s\" is not between 1 and 31 (inclusive).", key);
			tmptm->tm_sec = -1;
		} else {
			tmptm->tm_mday = (typeof(tmptm->tm_mday)) val;
		}
	} else if (strcasecmp(key, LAUNCH_JOBKEY_CAL_WEEKDAY) == 0) {
		if (val > 7) {
			job_log(j, LOG_WARNING, "The interval for key \"%s\" is not between 0 and 7 (inclusive).", key);
			tmptm->tm_sec = -1;
		} else {
			tmptm->tm_wday = (typeof(tmptm->tm_wday)) val;
		}
	} else if (strcasecmp(key, LAUNCH_JOBKEY_CAL_MONTH) == 0) {
		if (val > 12) {
			job_log(j, LOG_WARNING, "The interval for key \"%s\" is not between 0 and 12 (inclusive).", key);
			tmptm->tm_sec = -1;
		} else {
			tmptm->tm_mon = (typeof(tmptm->tm_mon)) val;
			tmptm->tm_mon -= 1; // 4798263 cron compatibility
		}
	}
}

bool
calendarinterval_new_from_obj(job_t j, launch_data_t obj)
{
	struct cal_dict_walk cdw;

	cdw.j = j;
	memset(&cdw.tmptm, 0, sizeof(0));

	cdw.tmptm.tm_min = -1;
	cdw.tmptm.tm_hour = -1;
	cdw.tmptm.tm_mday = -1;
	cdw.tmptm.tm_wday = -1;
	cdw.tmptm.tm_mon = -1;

	if (!job_assumes(j, obj != NULL)) {
		return false;
	}

	if (unlikely(LAUNCH_DATA_DICTIONARY != launch_data_get_type(obj))) {
		return false;
	}

	launch_data_dict_iterate(obj, calendarinterval_new_from_obj_dict_walk, &cdw);

	if (unlikely(cdw.tmptm.tm_sec == -1)) {
		return false;
	}

	return calendarinterval_new(j, &cdw.tmptm);
}

bool
calendarinterval_new(job_t j, struct tm *w)
{
	struct calendarinterval *ci = calloc(1, sizeof(struct calendarinterval));

	if (!job_assumes(j, ci != NULL)) {
		return false;
	}

	ci->when = *w;
	ci->job = j;

	SLIST_INSERT_HEAD(&j->cal_intervals, ci, sle);

	calendarinterval_setalarm(j, ci);

	runtime_add_weak_ref();

	return true;
}

void
calendarinterval_delete(job_t j, struct calendarinterval *ci)
{
	SLIST_REMOVE(&j->cal_intervals, ci, calendarinterval, sle);
	LIST_REMOVE(ci, global_sle);

	free(ci);

	runtime_del_weak_ref();
}

void
calendarinterval_sanity_check(void)
{
	struct calendarinterval *ci = LIST_FIRST(&sorted_calendar_events);
	time_t now = time(NULL);

	if (unlikely(ci && (ci->when_next < now))) {
		(void)jobmgr_assumes_zero_p(root_jobmgr, raise(SIGUSR1));
	}
}

void
calendarinterval_callback(void)
{
	struct calendarinterval *ci, *ci_next;
	time_t now = time(NULL);

	LIST_FOREACH_SAFE(ci, &sorted_calendar_events, global_sle, ci_next) {
		job_t j = ci->job;

		if (ci->when_next > now) {
			break;
		}

		LIST_REMOVE(ci, global_sle);
		calendarinterval_setalarm(j, ci);

		j->start_pending = true;
		job_dispatch(j, false);
	}
}

bool
socketgroup_new(job_t j, const char *name, int *fds, size_t fd_cnt)
{
	struct socketgroup *sg = calloc(1, sizeof(struct socketgroup) + strlen(name) + 1);

	if (!job_assumes(j, sg != NULL)) {
		return false;
	}

	sg->fds = calloc(1, fd_cnt * sizeof(int));
	sg->fd_cnt = fd_cnt;

	if (!job_assumes(j, sg->fds != NULL)) {
		free(sg);
		return false;
	}

	memcpy(sg->fds, fds, fd_cnt * sizeof(int));
	strcpy(sg->name_init, name);

	SLIST_INSERT_HEAD(&j->sockets, sg, sle);

	runtime_add_weak_ref();

	return true;
}

void
socketgroup_delete(job_t j, struct socketgroup *sg)
{
	unsigned int i;

	for (i = 0; i < sg->fd_cnt; i++) {
#if 0
		struct sockaddr_storage ss;
		struct sockaddr_un *sun = (struct sockaddr_un *)&ss;
		socklen_t ss_len = sizeof(ss);

		// 5480306
		if (job_assumes_zero(j, getsockname(sg->fds[i], (struct sockaddr *)&ss, &ss_len) != -1)
				&& job_assumes(j, ss_len > 0) && (ss.ss_family == AF_UNIX)) {
			(void)job_assumes(j, unlink(sun->sun_path) != -1);
			// We might conditionally need to delete a directory here
		}
#endif
		(void)job_assumes_zero_p(j, runtime_close(sg->fds[i]));
	}

	SLIST_REMOVE(&j->sockets, sg, socketgroup, sle);

	free(sg->fds);
	free(sg);

	runtime_del_weak_ref();
}

void
socketgroup_kevent_mod(job_t j, struct socketgroup *sg, bool do_add)
{
	struct kevent kev[sg->fd_cnt];
	char buf[10000];
	unsigned int i, buf_off = 0;

	for (i = 0; i < sg->fd_cnt; i++) {
		EV_SET(&kev[i], sg->fds[i], EVFILT_READ, do_add ? EV_ADD : EV_DELETE, 0, 0, j);
		buf_off += snprintf(buf + buf_off, sizeof(buf) - buf_off, " %d", sg->fds[i]);
	}

	job_log(j, LOG_DEBUG, "%s Sockets:%s", do_add ? "Watching" : "Ignoring", buf);

	(void)job_assumes_zero_p(j, kevent_bulk_mod(kev, sg->fd_cnt));

	for (i = 0; i < sg->fd_cnt; i++) {
		(void)job_assumes(j, kev[i].flags & EV_ERROR);
		errno = (typeof(errno)) kev[i].data;
		(void)job_assumes_zero(j, kev[i].data);
	}
}

void
socketgroup_ignore(job_t j, struct socketgroup *sg)
{
	socketgroup_kevent_mod(j, sg, false);
}

void
socketgroup_watch(job_t j, struct socketgroup *sg)
{
	socketgroup_kevent_mod(j, sg, true);
}

void
socketgroup_callback(job_t j)
{
	job_dispatch(j, true);
}

bool
envitem_new(job_t j, const char *k, const char *v, bool global)
{
	if (global && !launchd_allow_global_dyld_envvars) {
		if (strncmp("DYLD_", k, sizeof("DYLD_") - 1) == 0) {
			job_log(j, LOG_ERR, "Ignoring global environment variable submitted by job (variable=value): %s=%s", k, v);
			return false;
		}
	}

	struct envitem *ei = calloc(1, sizeof(struct envitem) + strlen(k) + 1 + strlen(v) + 1);

	if (!job_assumes(j, ei != NULL)) {
		return false;
	}

	strcpy(ei->key_init, k);
	ei->value = ei->key_init + strlen(k) + 1;
	strcpy(ei->value, v);

	if (global) {
		if (SLIST_EMPTY(&j->global_env)) {
			LIST_INSERT_HEAD(&j->mgr->global_env_jobs, j, global_env_sle);
		}
		SLIST_INSERT_HEAD(&j->global_env, ei, sle);
	} else {
		SLIST_INSERT_HEAD(&j->env, ei, sle);
	}

	job_log(j, LOG_DEBUG, "Added environmental variable: %s=%s", k, v);

	return true;
}

void
envitem_delete(job_t j, struct envitem *ei, bool global)
{
	if (global) {
		SLIST_REMOVE(&j->global_env, ei, envitem, sle);
		if (SLIST_EMPTY(&j->global_env)) {
			LIST_REMOVE(j, global_env_sle);
		}
	} else {
		SLIST_REMOVE(&j->env, ei, envitem, sle);
	}

	free(ei);
}

void
envitem_setup(launch_data_t obj, const char *key, void *context)
{
	job_t j = context;

	if (launch_data_get_type(obj) != LAUNCH_DATA_STRING) {
		return;
	}

	if (strncmp(LAUNCHD_TRUSTED_FD_ENV, key, sizeof(LAUNCHD_TRUSTED_FD_ENV) - 1) != 0) {
		envitem_new(j, key, launch_data_get_string(obj), j->importing_global_env);
	} else {
		job_log(j, LOG_DEBUG, "Ignoring reserved environmental variable: %s", key);
	}
}

bool
limititem_update(job_t j, int w, rlim_t r)
{
	struct limititem *li;

	SLIST_FOREACH(li, &j->limits, sle) {
		if (li->which == w) {
			break;
		}
	}

	if (li == NULL) {
		li = calloc(1, sizeof(struct limititem));

		if (!job_assumes(j, li != NULL)) {
			return false;
		}

		SLIST_INSERT_HEAD(&j->limits, li, sle);

		li->which = w;
	}

	if (j->importing_hard_limits) {
		li->lim.rlim_max = r;
		li->sethard = true;
	} else {
		li->lim.rlim_cur = r;
		li->setsoft = true;
	}

	return true;
}

void
limititem_delete(job_t j, struct limititem *li)
{
	SLIST_REMOVE(&j->limits, li, limititem, sle);

	free(li);
}

#if HAVE_SANDBOX
void
seatbelt_setup_flags(launch_data_t obj, const char *key, void *context)
{
	job_t j = context;

	if (launch_data_get_type(obj) != LAUNCH_DATA_BOOL) {
		job_log(j, LOG_WARNING, "Sandbox flag value must be boolean: %s", key);
		return;
	}

	if (launch_data_get_bool(obj) == false) {
		return;
	}

	if (strcasecmp(key, LAUNCH_JOBKEY_SANDBOX_NAMED) == 0) {
		j->seatbelt_flags |= SANDBOX_NAMED;
	}
}
#endif

void
limititem_setup(launch_data_t obj, const char *key, void *context)
{
	job_t j = context;
	size_t i, limits_cnt = (sizeof(launchd_keys2limits) / sizeof(launchd_keys2limits[0]));
	rlim_t rl;

	if (launch_data_get_type(obj) != LAUNCH_DATA_INTEGER) {
		return;
	}

	rl = launch_data_get_integer(obj);

	for (i = 0; i < limits_cnt; i++) {
		if (strcasecmp(launchd_keys2limits[i].key, key) == 0) {
			break;
		}
	}

	if (i == limits_cnt) {
		return;
	}

	limititem_update(j, launchd_keys2limits[i].val, rl);
}

bool
job_useless(job_t j)
{
	if ((j->legacy_LS_job || j->only_once) && j->start_time != 0) {
		if (j->legacy_LS_job && j->j_port) {
			return false;
		}
		job_log(j, LOG_INFO, "Exited. Was only configured to run once.");
		return true;
	} else if (j->removal_pending) {
		job_log(j, LOG_DEBUG, "Exited while removal was pending.");
		return true;
	} else if (j->shutdown_monitor) {
		return false;
	} else if (j->mgr->shutting_down && !j->mgr->parentmgr) {
		job_log(j, LOG_DEBUG, "Exited while shutdown in progress. Processes remaining: %lu/%lu", total_children, total_anon_children);
		if (total_children == 0 && !j->anonymous) {
			job_log(j, LOG_DEBUG | LOG_CONSOLE, "Job was last to exit during shutdown of: %s.", j->mgr->name);
		}
		return true;
	} else if (j->legacy_mach_job) {
		if (SLIST_EMPTY(&j->machservices)) {
			job_log(j, LOG_INFO, "Garbage collecting");
			return true;
		} else if (!j->checkedin) {
			job_log(j, LOG_WARNING, "Failed to check-in!");
			return true;
		}
	} else {
		/* If the job's executable does not have any valid architectures (for
		 * example, if it's a PowerPC-only job), then we don't even bother
		 * trying to relaunch it, as we have no reasonable expectation that
		 * the situation will change.
		 *
		 * <rdar://problem/9106979>
		 */
		if (!j->did_exec && WEXITSTATUS(j->last_exit_status) == EBADARCH) {
			job_log(j, LOG_ERR, "Job executable does not contain supported architectures. Unloading it. Its plist should be removed.");
			return true;
		}
	}

	return false;
}

bool
job_keepalive(job_t j)
{
	mach_msg_type_number_t statusCnt;
	mach_port_status_t status;
	struct semaphoreitem *si;
	struct machservice *ms;
	bool good_exit = (WIFEXITED(j->last_exit_status) && WEXITSTATUS(j->last_exit_status) == 0);
	bool is_not_kextd = (launchd_apple_internal || (strcmp(j->label, "com.apple.kextd") != 0));

	if (unlikely(j->mgr->shutting_down)) {
		return false;
	}

	/*
	 * 5066316
	 *
	 * We definitely need to revisit this after Leopard ships. Please see
	 * launchctl.c for the other half of this hack.
	 */
	if (unlikely((j->mgr->global_on_demand_cnt > 0) && is_not_kextd)) {
		return false;
	}

	if (unlikely(j->needs_kickoff)) {
		job_log(j, LOG_DEBUG, "KeepAlive check: Job needs to be kicked off on-demand before KeepAlive sets in.");
		return false;
	}

	if (j->start_pending) {
		job_log(j, LOG_DEBUG, "KeepAlive check: Pent-up non-IPC launch criteria.");
		return true;
	}

	if (!j->ondemand) {
		job_log(j, LOG_DEBUG, "KeepAlive check: job configured to run continuously.");
		return true;
	}

	SLIST_FOREACH(ms, &j->machservices, sle) {
		statusCnt = MACH_PORT_RECEIVE_STATUS_COUNT;
		if (mach_port_get_attributes(mach_task_self(), ms->port, MACH_PORT_RECEIVE_STATUS,
					(mach_port_info_t)&status, &statusCnt) != KERN_SUCCESS) {
			continue;
		}
		if (status.mps_msgcount) {
			job_log(j, LOG_DEBUG, "KeepAlive check: %d queued Mach messages on service: %s",
					status.mps_msgcount, ms->name);
			return true;
		}
	}

	/* TODO: Coalesce external events and semaphore items, since they're basically
	 * the same thing.
	 */
	struct externalevent *ei = NULL;
	LIST_FOREACH(ei, &j->events, job_le) {
		if (ei->state == ei->wanted_state) {
			return true;
		}
	}

	SLIST_FOREACH(si, &j->semaphores, sle) {
		bool wanted_state = false;
		job_t other_j;

		switch (si->why) {
		case NETWORK_UP:
			wanted_state = true;
		case NETWORK_DOWN:
			if (network_up == wanted_state) {
				job_log(j, LOG_DEBUG, "KeepAlive: The network is %s.", wanted_state ? "up" : "down");
				return true;
			}
			break;
		case SUCCESSFUL_EXIT:
			wanted_state = true;
		case FAILED_EXIT:
			if (good_exit == wanted_state) {
				job_log(j, LOG_DEBUG, "KeepAlive: The exit state was %s.", wanted_state ? "successful" : "failure");
				return true;
			}
			break;
		case CRASHED:
			wanted_state = true;
		case DID_NOT_CRASH:
			if (j->crashed == wanted_state) {
				return true;
			}
			break;
		case OTHER_JOB_ENABLED:
			wanted_state = true;
		case OTHER_JOB_DISABLED:
			if ((bool)job_find(NULL, si->what) == wanted_state) {
				job_log(j, LOG_DEBUG, "KeepAlive: The following job is %s: %s", wanted_state ? "enabled" : "disabled", si->what);
				return true;
			}
			break;
		case OTHER_JOB_ACTIVE:
			wanted_state = true;
		case OTHER_JOB_INACTIVE:
			if ((other_j = job_find(NULL, si->what))) {
				if ((bool)other_j->p == wanted_state) {
					job_log(j, LOG_DEBUG, "KeepAlive: The following job is %s: %s", wanted_state ? "active" : "inactive", si->what);
					return true;
				}
			}
			break;
		}
	}

	return false;
}

const char *
job_active(job_t j)
{
	if (j->p && j->shutdown_monitor) {
		return "Monitoring shutdown";
	}
	if (j->p) {
		return "PID is still valid";
	}

	if (j->priv_port_has_senders) {
		return "Privileged Port still has outstanding senders";
	}

	struct machservice *ms;
	SLIST_FOREACH(ms, &j->machservices, sle) {
		/* If we've simulated an exit, we mark the job as non-active, even
		 * though doing so will leave it in an unsafe state. We do this so that
		 * shutdown can proceed. See <rdar://problem/11126530>.
		 */
		if (!j->workaround9359725 && ms->recv && machservice_active(ms)) {
			job_log(j, LOG_INFO, "Mach service is still active: %s", ms->name);
			return "Mach service is still active";
		}
	}

	return NULL;
}

void
machservice_watch(job_t j, struct machservice *ms)
{
	if (ms->recv) {
		if (job_assumes_zero(j, runtime_add_mport(ms->port, NULL)) == KERN_INVALID_RIGHT) {
			ms->recv_race_hack = true;
		}
	}
}

void
machservice_ignore(job_t j, struct machservice *ms)
{
	/* We only add ports whose receive rights we control into the port set, so
	 * don't attempt to remove te service from the port set if we didn't put it
	 * there in the first place. Otherwise, we could wind up trying to access a
	 * bogus index (like MACH_PORT_DEAD) or zeroing a valid one out.
	 *
	 * <rdar://problem/10898014>
	 */
	if (ms->recv) {
		(void)job_assumes_zero(j, runtime_remove_mport(ms->port));
	}
}

void
machservice_resetport(job_t j, struct machservice *ms)
{
	LIST_REMOVE(ms, port_hash_sle);
	(void)job_assumes_zero(j, launchd_mport_close_recv(ms->port));
	(void)job_assumes_zero(j, launchd_mport_deallocate(ms->port));

	ms->gen_num++;
	(void)job_assumes_zero(j, launchd_mport_create_recv(&ms->port));
	(void)job_assumes_zero(j, launchd_mport_make_send(ms->port));
	LIST_INSERT_HEAD(&port_hash[HASH_PORT(ms->port)], ms, port_hash_sle);
}

void
machservice_stamp_port(job_t j, struct machservice *ms)
{
	mach_port_context_t ctx = 0;
	char *where2get = j->prog ? j->prog : j->argv[0];

	char *prog = NULL;
	if ((prog = strrchr(where2get, '/'))) {
		prog++;
	} else {
		prog = where2get;
	}

	(void)strncpy((char *)&ctx, prog, sizeof(ctx));
#if __LITTLE_ENDIAN__
#if __LP64__
	ctx = OSSwapBigToHostInt64(ctx);
#else
	ctx = OSSwapBigToHostInt32(ctx);
#endif
#endif

	(void)job_assumes_zero(j, mach_port_set_context(mach_task_self(), ms->port, ctx));
}

struct machservice *
machservice_new(job_t j, const char *name, mach_port_t *serviceport, bool pid_local)
{
	/* Don't create new MachServices for dead ports. This is primarily for
	 * clients who use bootstrap_register2(). They can pass in a send right, but
	 * then that port can immediately go dead. Hilarity ensues.
	 *
	 * <rdar://problem/10898014>
	 */
	if (*serviceport == MACH_PORT_DEAD) {
		return NULL;
	}

	struct machservice *ms = calloc(1, sizeof(struct machservice) + strlen(name) + 1);
	if (!job_assumes(j, ms != NULL)) {
		return NULL;
	}

	strcpy((char *)ms->name, name);
	ms->job = j;
	ms->gen_num = 1;
	ms->per_pid = pid_local;

	if (likely(*serviceport == MACH_PORT_NULL)) {
		if (job_assumes_zero(j, launchd_mport_create_recv(&ms->port)) != KERN_SUCCESS) {
			goto out_bad;
		}

		if (job_assumes_zero(j, launchd_mport_make_send(ms->port)) != KERN_SUCCESS) {
			goto out_bad2;
		}
		*serviceport = ms->port;
		ms->recv = true;
	} else {
		ms->port = *serviceport;
		ms->isActive = true;
	}

	SLIST_INSERT_HEAD(&j->machservices, ms, sle);

	jobmgr_t where2put = j->mgr;
	// XPC domains are separate from Mach bootstraps.
	if (!(j->mgr->properties & BOOTSTRAP_PROPERTY_XPC_DOMAIN)) {
		if (launchd_flat_mach_namespace && !(j->mgr->properties & BOOTSTRAP_PROPERTY_EXPLICITSUBSET)) {
			where2put = root_jobmgr;
		}
	}

	/* Don't allow MachServices added by multiple-instance jobs to be looked up
	 * by others. We could just do this with a simple bit, but then we'd have to
	 * uniquify the names ourselves to avoid collisions. This is just easier.
	 */
	if (!j->dedicated_instance) {
		LIST_INSERT_HEAD(&where2put->ms_hash[hash_ms(ms->name)], ms, name_hash_sle);	
	}
	LIST_INSERT_HEAD(&port_hash[HASH_PORT(ms->port)], ms, port_hash_sle);

	if (ms->recv) {
		machservice_stamp_port(j, ms);
	}

	job_log(j, LOG_DEBUG, "Mach service added%s: %s", (j->mgr->properties & BOOTSTRAP_PROPERTY_EXPLICITSUBSET) ? " to private namespace" : "", name);

	return ms;
out_bad2:
	(void)job_assumes_zero(j, launchd_mport_close_recv(ms->port));
out_bad:
	free(ms);
	return NULL;
}

struct machservice *
machservice_new_alias(job_t j, struct machservice *orig)
{
	struct machservice *ms = calloc(1, sizeof(struct machservice) + strlen(orig->name) + 1);
	if (job_assumes(j, ms != NULL)) {
		strcpy((char *)ms->name, orig->name);
		ms->alias = orig;
		ms->job = j;

		LIST_INSERT_HEAD(&j->mgr->ms_hash[hash_ms(ms->name)], ms, name_hash_sle);
		SLIST_INSERT_HEAD(&j->machservices, ms, sle);
		jobmgr_log(j->mgr, LOG_DEBUG, "Service aliased into job manager: %s", orig->name);
	}

	return ms;
}

bootstrap_status_t
machservice_status(struct machservice *ms)
{
	ms = ms->alias ? ms->alias : ms;
	if (ms->isActive) {
		return BOOTSTRAP_STATUS_ACTIVE;
	} else if (ms->job->ondemand) {
		return BOOTSTRAP_STATUS_ON_DEMAND;
	} else {
		return BOOTSTRAP_STATUS_INACTIVE;
	}
}

void
job_setup_exception_port(job_t j, task_t target_task)
{
	struct machservice *ms;
	thread_state_flavor_t f = 0;
	mach_port_t exc_port = the_exception_server;

	if (unlikely(j->alt_exc_handler)) {
		ms = jobmgr_lookup_service(j->mgr, j->alt_exc_handler, true, 0);
		if (likely(ms)) {
			exc_port = machservice_port(ms);
		} else {
			job_log(j, LOG_WARNING, "Falling back to default Mach exception handler. Could not find: %s", j->alt_exc_handler);
		}
	} else if (unlikely(j->internal_exc_handler)) {
		exc_port = runtime_get_kernel_port();
	} else if (unlikely(!exc_port)) {
		return;
	}

#if defined (__ppc__) || defined(__ppc64__)
	f = PPC_THREAD_STATE64;
#elif defined(__i386__) || defined(__x86_64__)
	f = x86_THREAD_STATE;
#elif defined(__arm__)
	f = ARM_THREAD_STATE;
#else
#error "unknown architecture"
#endif

	if (likely(target_task)) {
		kern_return_t kr = task_set_exception_ports(target_task, EXC_MASK_CRASH | EXC_MASK_GUARD | EXC_MASK_RESOURCE, exc_port, EXCEPTION_STATE_IDENTITY | MACH_EXCEPTION_CODES, f);
		if (kr) {
			if (kr != MACH_SEND_INVALID_DEST) {
				(void)job_assumes_zero(j, kr);
			} else {
				job_log(j, LOG_WARNING, "Task died before exception port could be set.");
			}
		}
	} else if (pid1_magic && the_exception_server) {
		mach_port_t mhp = mach_host_self();
		(void)job_assumes_zero(j, host_set_exception_ports(mhp, EXC_MASK_CRASH | EXC_MASK_GUARD | EXC_MASK_RESOURCE, the_exception_server, EXCEPTION_STATE_IDENTITY | MACH_EXCEPTION_CODES, f));
		(void)job_assumes_zero(j, launchd_mport_deallocate(mhp));
	}
}

void
job_set_exception_port(job_t j, mach_port_t port)
{
	if (unlikely(!the_exception_server)) {
		the_exception_server = port;
		job_setup_exception_port(j, 0);
	} else {
		job_log(j, LOG_WARNING, "The exception server is already claimed!");
	}
}

void
machservice_setup_options(launch_data_t obj, const char *key, void *context)
{
	struct machservice *ms = context;
	mach_port_t mhp = mach_host_self();
	int which_port;
	bool b;

	if (!job_assumes(ms->job, mhp != MACH_PORT_NULL)) {
		return;
	}

	switch (launch_data_get_type(obj)) {
	case LAUNCH_DATA_INTEGER:
		which_port = (int)launch_data_get_integer(obj); // XXX we should bound check this...
		if (strcasecmp(key, LAUNCH_JOBKEY_MACH_TASKSPECIALPORT) == 0) {
			switch (which_port) {
			case TASK_KERNEL_PORT:
			case TASK_HOST_PORT:
			case TASK_NAME_PORT:
			case TASK_BOOTSTRAP_PORT:
			/* I find it a little odd that zero isn't reserved in the header.
			 * Normally Mach is fairly good about this convention...
			 */
			case 0:
				job_log(ms->job, LOG_WARNING, "Tried to set a reserved task special port: %d", which_port);
				break;
			default:
				ms->special_port_num = which_port;
				SLIST_INSERT_HEAD(&special_ports, ms, special_port_sle);
				break;
			}
		} else if (strcasecmp(key, LAUNCH_JOBKEY_MACH_HOSTSPECIALPORT) == 0 && pid1_magic) {
			if (which_port > HOST_MAX_SPECIAL_KERNEL_PORT) {
				(void)job_assumes_zero(ms->job, (errno = host_set_special_port(mhp, which_port, ms->port)));
			} else {
				job_log(ms->job, LOG_WARNING, "Tried to set a reserved host special port: %d", which_port);
			}
		}
	case LAUNCH_DATA_BOOL:
		b = launch_data_get_bool(obj);
		if (strcasecmp(key, LAUNCH_JOBKEY_MACH_ENTERKERNELDEBUGGERONCLOSE) == 0) {
			ms->debug_on_close = b;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_MACH_RESETATCLOSE) == 0) {
			ms->reset = b;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_MACH_HIDEUNTILCHECKIN) == 0) {
			ms->hide = b;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_MACH_EXCEPTIONSERVER) == 0) {
			job_set_exception_port(ms->job, ms->port);
		} else if (strcasecmp(key, LAUNCH_JOBKEY_MACH_KUNCSERVER) == 0) {
			ms->kUNCServer = b;
			(void)job_assumes_zero(ms->job, host_set_UNDServer(mhp, ms->port));
		}
		break;
	case LAUNCH_DATA_STRING:
		if (strcasecmp(key, LAUNCH_JOBKEY_MACH_DRAINMESSAGESONCRASH) == 0) {
			const char *option = launch_data_get_string(obj);
			if (strcasecmp(option, "One") == 0) {
				ms->drain_one_on_crash = true;
			} else if (strcasecmp(option, "All") == 0) {
				ms->drain_all_on_crash = true;
			}
		}
		break;
	case LAUNCH_DATA_DICTIONARY:
		if (launch_data_dict_get_count(obj) == 0) {
			job_set_exception_port(ms->job, ms->port);
		}
		break;
	default:
		break;
	}

	(void)job_assumes_zero(ms->job, launchd_mport_deallocate(mhp));
}

void
machservice_setup(launch_data_t obj, const char *key, void *context)
{
	job_t j = context;
	struct machservice *ms;
	mach_port_t p = MACH_PORT_NULL;

	if (unlikely(ms = jobmgr_lookup_service(j->mgr, key, false, 0))) {
		job_log(j, LOG_WARNING, "Conflict with job: %s over Mach service: %s", ms->job->label, key);
		return;
	}

	if (!job_assumes(j, (ms = machservice_new(j, key, &p, false)) != NULL)) {
		return;
	}

	ms->isActive = false;
	ms->upfront = true;

	if (launch_data_get_type(obj) == LAUNCH_DATA_DICTIONARY) {
		launch_data_dict_iterate(obj, machservice_setup_options, ms);
	}

	kern_return_t kr = mach_port_set_attributes(mach_task_self(), ms->port, MACH_PORT_TEMPOWNER, NULL, 0);
	(void)job_assumes_zero(j, kr);
}

jobmgr_t
jobmgr_do_garbage_collection(jobmgr_t jm)
{
	jobmgr_t jmi = NULL, jmn = NULL;
	SLIST_FOREACH_SAFE(jmi, &jm->submgrs, sle, jmn) {
		jobmgr_do_garbage_collection(jmi);
	}

	if (!jm->shutting_down) {
		return jm;
	}

	if (SLIST_EMPTY(&jm->submgrs)) {
		jobmgr_log(jm, LOG_DEBUG, "No submanagers left.");
	} else {
		jobmgr_log(jm, LOG_DEBUG, "Still have submanagers.");
		SLIST_FOREACH(jmi, &jm->submgrs, sle) {
			jobmgr_log(jm, LOG_DEBUG, "Submanager: %s", jmi->name);
		}
	}

	size_t actives = 0;
	job_t ji = NULL, jn = NULL;
	LIST_FOREACH_SAFE(ji, &jm->jobs, sle, jn) {
		if (ji->anonymous) {
			continue;
		}

		// Let the shutdown monitor be up until the very end.
		if (ji->shutdown_monitor) {
			continue;
		}

		/* On our first pass through, open a transaction for all the jobs that
		 * need to be dirty at shutdown. We'll close these transactions once the
		 * jobs that do not need to be dirty at shutdown have all exited.
		 */
		if (ji->dirty_at_shutdown && !jm->shutdown_jobs_dirtied) {
			job_open_shutdown_transaction(ji);
		}

		const char *active = job_active(ji);
		if (!active) {
			job_remove(ji);
		} else {
			job_log(ji, LOG_DEBUG, "Job is active: %s", active);
			job_stop(ji);

			if (!ji->dirty_at_shutdown) {
				actives++;
			}

			if (ji->clean_kill) {
				job_log(ji, LOG_DEBUG, "Job was killed cleanly.");
			} else {
				job_log(ji, LOG_DEBUG, "Job was sent SIGTERM%s.", ji->sent_sigkill ? " and SIGKILL" : "");
			}
		}
	}

	jm->shutdown_jobs_dirtied = true;
	if (actives == 0) {
		if (!jm->shutdown_jobs_cleaned) {
			/* Once all normal jobs have exited, we clean the dirty-at-shutdown
			 * jobs and make them into normal jobs so that the above loop will
			 * handle them appropriately.
			 */
			LIST_FOREACH(ji, &jm->jobs, sle) {
				if (ji->anonymous) {
					continue;
				}

				if (!job_active(ji)) {
					continue;
				}
				
				if (ji->shutdown_monitor) {
					continue;
				}

				job_close_shutdown_transaction(ji);
				actives++;
			}

			jm->shutdown_jobs_cleaned = true;
		}

		if (SLIST_EMPTY(&jm->submgrs) && actives == 0) {
			/* We may be in a situation where the shutdown monitor is all that's
			 * left, in which case we want to stop it. Like dirty-at-shutdown
			 * jobs, we turn it back into a normal job so that the main loop
			 * treats it appropriately.
			 *
			 * See:
			 * <rdar://problem/10756306>
			 * <rdar://problem/11034971>
			 * <rdar://problem/11549541>
			 */
			if (jm->monitor_shutdown && _launchd_shutdown_monitor) {
				/* The rest of shutdown has completed, so we can kill the shutdown
				 * monitor now like it was any other job.
				 */
				_launchd_shutdown_monitor->shutdown_monitor = false;
				
				job_log(_launchd_shutdown_monitor, LOG_NOTICE | LOG_CONSOLE, "Stopping shutdown monitor.");
				job_stop(_launchd_shutdown_monitor);
				_launchd_shutdown_monitor = NULL;
			} else {
				jobmgr_log(jm, LOG_DEBUG, "Removing.");
				jobmgr_remove(jm);
				return NULL;
			}
		}
	}

	return jm;
}

void
jobmgr_kill_stray_children(jobmgr_t jm, pid_t *p, size_t np)
{
	/* I maintain that stray processes should be at the mercy of launchd during
	 * shutdown, but nevertheless, things like diskimages-helper can stick
	 * around, and SIGKILLing them can result in data loss. So we send SIGTERM
	 * to all the strays and don't wait for them to exit before moving on.
	 * 
	 * See rdar://problem/6562592
	 */
	size_t i = 0;
	for (i = 0; i < np; i++) {
		if (p[i] != 0) {
			jobmgr_log(jm, LOG_DEBUG | LOG_CONSOLE, "Sending SIGTERM to PID %u and continuing...", p[i]);
			(void)jobmgr_assumes_zero_p(jm, kill2(p[i], SIGTERM));
		}
	}
}

void
jobmgr_log_stray_children(jobmgr_t jm, bool kill_strays)
{
	size_t kp_skipped = 0, len = sizeof(pid_t) * get_kern_max_proc();
	pid_t *pids = NULL;
	int i = 0, kp_cnt = 0;

	if (likely(jm->parentmgr || !pid1_magic)) {
		return;
	}

	if (!jobmgr_assumes(jm, (pids = malloc(len)) != NULL)) {
		return;
	}

	runtime_ktrace0(RTKT_LAUNCHD_FINDING_ALL_STRAYS);

	if (jobmgr_assumes_zero_p(jm, (kp_cnt = proc_listallpids(pids, len))) == -1) {
		goto out;
	}

	pid_t *ps = (pid_t *)calloc(sizeof(pid_t), kp_cnt);
	for (i = 0; i < kp_cnt; i++) {
		struct proc_bsdshortinfo proc;
		if (proc_pidinfo(pids[i], PROC_PIDT_SHORTBSDINFO, 1, &proc, PROC_PIDT_SHORTBSDINFO_SIZE) == 0) {
			if (errno != ESRCH) {
				(void)jobmgr_assumes_zero(jm, errno);
			}

			kp_skipped++;
			continue;
		}

		pid_t p_i = pids[i];
		pid_t pp_i = proc.pbsi_ppid;
		pid_t pg_i = proc.pbsi_pgid;
		const char *z = (proc.pbsi_status == SZOMB) ? "zombie " : "";
		const char *n = proc.pbsi_comm;

		if (unlikely(p_i == 0 || p_i == 1)) {
			kp_skipped++;
			continue;
		}

		if (_launchd_shutdown_monitor && pp_i == _launchd_shutdown_monitor->p) {
			kp_skipped++;
			continue;
		}

		// We might have some jobs hanging around that we've decided to shut down in spite of.
		job_t j = jobmgr_find_by_pid(jm, p_i, false);
		if (!j || (j && j->anonymous)) {
			jobmgr_log(jm, LOG_INFO | LOG_CONSOLE, "Stray %s%s at shutdown: PID %u PPID %u PGID %u %s", z, j ? "anonymous job" : "process", p_i, pp_i, pg_i, n);

			int status = 0;
			if (pp_i == getpid() && !jobmgr_assumes(jm, proc.pbsi_status != SZOMB)) {
				if (jobmgr_assumes_zero(jm, waitpid(p_i, &status, WNOHANG)) == 0) {
					jobmgr_log(jm, LOG_INFO | LOG_CONSOLE, "Unreaped zombie stray exited with status %i.", WEXITSTATUS(status));
				}
				kp_skipped++;
			} else {
				job_t leader = jobmgr_find_by_pid(jm, pg_i, false);
				/* See rdar://problem/6745714. Some jobs have child processes that back kernel state,
				 * so we don't want to terminate them. Long-term, I'd really like to provide shutdown
				 * hints to the kernel along the way, so that it could shutdown certain subsystems when
				 * their userspace emissaries go away, before the call to reboot(2).
				 */
				if (leader && leader->ignore_pg_at_shutdown) {
					kp_skipped++;
				} else {
					ps[i] = p_i;
				}
			}
		} else {
			kp_skipped++;
		}
	}

	if ((kp_cnt - kp_skipped > 0) && kill_strays) {
		jobmgr_kill_stray_children(jm, ps, kp_cnt - kp_skipped);
	}

	free(ps);
out:
	free(pids);
}

jobmgr_t 
jobmgr_parent(jobmgr_t jm)
{
	return jm->parentmgr;
}

void
job_uncork_fork(job_t j)
{
	pid_t c = j->p;

	job_log(j, LOG_DEBUG, "Uncorking the fork().");
	/* this unblocks the child and avoids a race
	 * between the above fork() and the kevent_mod() */
	(void)job_assumes(j, write(j->fork_fd, &c, sizeof(c)) == sizeof(c));
	(void)job_assumes_zero_p(j, runtime_close(j->fork_fd));
	j->fork_fd = 0;
}

jobmgr_t 
jobmgr_new(jobmgr_t jm, mach_port_t requestorport, mach_port_t transfer_port, bool sflag, const char *name, bool skip_init, mach_port_t asport)
{
	job_t bootstrapper = NULL;
	jobmgr_t jmr;

	__OS_COMPILETIME_ASSERT__(offsetof(struct jobmgr_s, kqjobmgr_callback) == 0);

	if (unlikely(jm && requestorport == MACH_PORT_NULL)) {
		jobmgr_log(jm, LOG_ERR, "Mach sub-bootstrap create request requires a requester port");
		return NULL;
	}

	jmr = calloc(1, sizeof(struct jobmgr_s) + (name ? (strlen(name) + 1) : NAME_MAX + 1));

	if (!jobmgr_assumes(jm, jmr != NULL)) {
		return NULL;
	}

	if (jm == NULL) {
		root_jobmgr = jmr;
	}

	jmr->kqjobmgr_callback = jobmgr_callback;
	strcpy(jmr->name_init, name ? name : "Under construction");

	jmr->req_port = requestorport;

	if ((jmr->parentmgr = jm)) {
		SLIST_INSERT_HEAD(&jm->submgrs, jmr, sle);
	}

	if (jm && jobmgr_assumes_zero(jmr, launchd_mport_notify_req(jmr->req_port, MACH_NOTIFY_DEAD_NAME)) != KERN_SUCCESS) {
		goto out_bad;
	}

	if (transfer_port != MACH_PORT_NULL) {
		(void)jobmgr_assumes(jmr, jm != NULL);
		jmr->jm_port = transfer_port;
	} else if (!jm && !pid1_magic) {
		char *trusted_fd = getenv(LAUNCHD_TRUSTED_FD_ENV);
		name_t service_buf;

		snprintf(service_buf, sizeof(service_buf), "com.apple.launchd.peruser.%u", getuid());

		if (jobmgr_assumes_zero(jmr, bootstrap_check_in(bootstrap_port, service_buf, &jmr->jm_port)) != 0) {
			goto out_bad;
		}

		if (trusted_fd) {
			int dfd, lfd = (int) strtol(trusted_fd, NULL, 10);

			if ((dfd = dup(lfd)) >= 0) {
				(void)jobmgr_assumes_zero_p(jmr, runtime_close(dfd));
				(void)jobmgr_assumes_zero_p(jmr, runtime_close(lfd));
			}

			unsetenv(LAUNCHD_TRUSTED_FD_ENV);
		}

		// cut off the Libc cache, we don't want to deadlock against ourself
		inherited_bootstrap_port = bootstrap_port;
		bootstrap_port = MACH_PORT_NULL;
		os_assert_zero(launchd_mport_notify_req(inherited_bootstrap_port, MACH_NOTIFY_DEAD_NAME));

		// We set this explicitly as we start each child
		os_assert_zero(launchd_set_bport(MACH_PORT_NULL));
	} else if (jobmgr_assumes_zero(jmr, launchd_mport_create_recv(&jmr->jm_port)) != KERN_SUCCESS) {
		goto out_bad;
	}

	if (!name) {
		sprintf(jmr->name_init, "%u", MACH_PORT_INDEX(jmr->jm_port));
	}

	if (!jm) {
		(void)jobmgr_assumes_zero_p(jmr, kevent_mod(SIGTERM, EVFILT_SIGNAL, EV_ADD, 0, 0, jmr));
		(void)jobmgr_assumes_zero_p(jmr, kevent_mod(SIGUSR1, EVFILT_SIGNAL, EV_ADD, 0, 0, jmr));
		(void)jobmgr_assumes_zero_p(jmr, kevent_mod(SIGUSR2, EVFILT_SIGNAL, EV_ADD, 0, 0, jmr));
		(void)jobmgr_assumes_zero_p(jmr, kevent_mod(SIGINFO, EVFILT_SIGNAL, EV_ADD, 0, 0, jmr));
		(void)jobmgr_assumes_zero_p(jmr, kevent_mod(0, EVFILT_FS, EV_ADD, VQ_MOUNT|VQ_UNMOUNT|VQ_UPDATE, 0, jmr));
	}

	if (name && !skip_init) {
		bootstrapper = jobmgr_init_session(jmr, name, sflag);
	}

	if (!bootstrapper || !bootstrapper->weird_bootstrap) {
		if (jobmgr_assumes_zero(jmr, runtime_add_mport(jmr->jm_port, job_server)) != KERN_SUCCESS) {
			goto out_bad;
		}
	}

	jobmgr_log(jmr, LOG_DEBUG, "Created job manager%s%s", jm ? " with parent: " : ".", jm ? jm->name : "");

	if (bootstrapper) {
		bootstrapper->asport = asport;

		jobmgr_log(jmr, LOG_DEBUG, "Bootstrapping new job manager with audit session %u", asport);
		(void)jobmgr_assumes(jmr, job_dispatch(bootstrapper, true) != NULL);
	} else {
		jmr->req_asport = asport;
	}

	if (asport != MACH_PORT_NULL) {
		(void)jobmgr_assumes_zero(jmr, launchd_mport_copy_send(asport));
	}

	if (jmr->parentmgr) {
		runtime_add_weak_ref();
	}

	return jmr;

out_bad:
	if (jmr) {
		jobmgr_remove(jmr);
		if (jm == NULL) {
			root_jobmgr = NULL;
		}
	}
	return NULL;
}

jobmgr_t
jobmgr_new_xpc_singleton_domain(jobmgr_t jm, name_t name)
{
	jobmgr_t new = NULL;

	/* These job managers are basically singletons, so we use the root Mach
	 * bootstrap port as their requestor ports so they'll never go away.
	 */
	mach_port_t req_port = root_jobmgr->jm_port;
	if (jobmgr_assumes_zero(jm, launchd_mport_make_send(req_port)) == KERN_SUCCESS) {
		new = jobmgr_new(root_jobmgr, req_port, MACH_PORT_NULL, false, name, true, MACH_PORT_NULL);
		if (new) {
			new->properties |= BOOTSTRAP_PROPERTY_XPC_SINGLETON;
			new->properties |= BOOTSTRAP_PROPERTY_XPC_DOMAIN;
			new->xpc_singleton = true;
		}
	}

	return new;
}

jobmgr_t
jobmgr_find_xpc_per_user_domain(jobmgr_t jm, uid_t uid)
{
	jobmgr_t jmi = NULL;
	LIST_FOREACH(jmi, &_s_xpc_user_domains, xpc_le) {
		if (jmi->req_euid == uid) {
			return jmi;
		}
	}

	name_t name;
	(void)snprintf(name, sizeof(name), "com.apple.xpc.domain.peruser.%u", uid);
	jmi = jobmgr_new_xpc_singleton_domain(jm, name);
	if (jobmgr_assumes(jm, jmi != NULL)) {
		/* We need to create a per-user launchd for this UID if there isn't one
		 * already so we can grab the bootstrap port.
		 */
		job_t puj = jobmgr_lookup_per_user_context_internal(NULL, uid, &jmi->req_bsport);
		if (jobmgr_assumes(jmi, puj != NULL)) {
			(void)jobmgr_assumes_zero(jmi, launchd_mport_copy_send(puj->asport));
			(void)jobmgr_assumes_zero(jmi, launchd_mport_copy_send(jmi->req_bsport));
			jmi->shortdesc = "per-user";
			jmi->req_asport = puj->asport;
			jmi->req_asid = puj->asid;
			jmi->req_euid = uid;
			jmi->req_egid = -1;

			LIST_INSERT_HEAD(&_s_xpc_user_domains, jmi, xpc_le);
		} else {
			jobmgr_remove(jmi);
		}
	}

	return jmi;
}

jobmgr_t
jobmgr_find_xpc_per_session_domain(jobmgr_t jm, au_asid_t asid)
{
	jobmgr_t jmi = NULL;
	LIST_FOREACH(jmi, &_s_xpc_session_domains, xpc_le) {
		if (jmi->req_asid == asid) {
			return jmi;
		}
	}

	name_t name;
	(void)snprintf(name, sizeof(name), "com.apple.xpc.domain.persession.%i", asid);
	jmi = jobmgr_new_xpc_singleton_domain(jm, name);
	if (jobmgr_assumes(jm, jmi != NULL)) {
		(void)jobmgr_assumes_zero(jmi, launchd_mport_make_send(root_jobmgr->jm_port));
		jmi->shortdesc = "per-session";
		jmi->req_bsport = root_jobmgr->jm_port;
		(void)jobmgr_assumes_zero(jmi, audit_session_port(asid, &jmi->req_asport));
		jmi->req_asid = asid;
		jmi->req_euid = -1;
		jmi->req_egid = -1;

		LIST_INSERT_HEAD(&_s_xpc_session_domains, jmi, xpc_le);
	} else {
		jobmgr_remove(jmi);
	}

	return jmi;
}

job_t
jobmgr_init_session(jobmgr_t jm, const char *session_type, bool sflag)
{
	const char *bootstrap_tool[] = { "/bin/launchctl", "bootstrap", "-S", session_type, sflag ? "-s" : NULL, NULL };
	char thelabel[1000];
	job_t bootstrapper;

	snprintf(thelabel, sizeof(thelabel), "com.apple.launchctl.%s", session_type);
	bootstrapper = job_new(jm, thelabel, NULL, bootstrap_tool);

	if (jobmgr_assumes(jm, bootstrapper != NULL) && (jm->parentmgr || !pid1_magic)) {
		bootstrapper->is_bootstrapper = true;
		char buf[100];

		// <rdar://problem/5042202> launchd-201: can't ssh in with AFP OD account (hangs)
		snprintf(buf, sizeof(buf), "0x%X:0:0", getuid());
		envitem_new(bootstrapper, "__CF_USER_TEXT_ENCODING", buf, false);
		bootstrapper->weird_bootstrap = true;
		(void)jobmgr_assumes(jm, job_setup_machport(bootstrapper));
	} else if (bootstrapper && strncmp(session_type, VPROCMGR_SESSION_SYSTEM, sizeof(VPROCMGR_SESSION_SYSTEM)) == 0) {
#if TARGET_OS_EMBEDDED
		bootstrapper->psproctype = POSIX_SPAWN_PROC_TYPE_DAEMON_INTERACTIVE;
#endif
		bootstrapper->is_bootstrapper = true;
		if (jobmgr_assumes(jm, pid1_magic)) {
			// Have our system bootstrapper print out to the console.
			bootstrapper->stdoutpath = strdup(_PATH_CONSOLE);
			bootstrapper->stderrpath = strdup(_PATH_CONSOLE);

			if (launchd_console) {
				(void)jobmgr_assumes_zero_p(jm, kevent_mod((uintptr_t)fileno(launchd_console), EVFILT_VNODE, EV_ADD | EV_ONESHOT, NOTE_REVOKE, 0, jm));
			}
		}
	}

	jm->session_initialized = true;
	return bootstrapper;
}

jobmgr_t
jobmgr_delete_anything_with_port(jobmgr_t jm, mach_port_t port)
{
	struct machservice *ms, *next_ms;
	jobmgr_t jmi, jmn;

	/* Mach ports, unlike Unix descriptors, are reference counted. In other
	 * words, when some program hands us a second or subsequent send right to a
	 * port we already have open, the Mach kernel gives us the same port number
	 * back and increments an reference count associated with the port. This
	 * This forces us, when discovering that a receive right at the other end
	 * has been deleted, to wander all of our objects to see what weird places
	 * clients might have handed us the same send right to use.
	 */

	if (jm == root_jobmgr) {
		if (port == inherited_bootstrap_port) {
			(void)jobmgr_assumes_zero(jm, launchd_mport_deallocate(port));
			inherited_bootstrap_port = MACH_PORT_NULL;

			return jobmgr_shutdown(jm);
		}

		LIST_FOREACH_SAFE(ms, &port_hash[HASH_PORT(port)], port_hash_sle, next_ms) {
			if (ms->port == port && !ms->recv) {
				machservice_delete(ms->job, ms, true);
			}
		}
	}

	SLIST_FOREACH_SAFE(jmi, &jm->submgrs, sle, jmn) {
		jobmgr_delete_anything_with_port(jmi, port);
	}

	if (jm->req_port == port) {
		jobmgr_log(jm, LOG_DEBUG, "Request port died: %i", MACH_PORT_INDEX(port));
		return jobmgr_shutdown(jm);
	}

	struct waiting4attach *w4ai = NULL;
	struct waiting4attach *w4ait = NULL;
	LIST_FOREACH_SAFE(w4ai, &jm->attaches, le, w4ait) {
		if (port == w4ai->port) {
			waiting4attach_delete(jm, w4ai);
			break;
		}
	}

	return jm;
}

struct machservice *
jobmgr_lookup_service(jobmgr_t jm, const char *name, bool check_parent, pid_t target_pid)
{
	struct machservice *ms;
	job_t target_j;

	jobmgr_log(jm, LOG_DEBUG, "Looking up %sservice %s", target_pid ? "per-PID " : "", name);

	if (target_pid) {
		/* This is a hack to let FileSyncAgent look up per-PID Mach services from the Background
		 * bootstrap in other bootstraps.
		 */

		// Start in the given bootstrap.
		if (unlikely((target_j = jobmgr_find_by_pid(jm, target_pid, false)) == NULL)) {
			// If we fail, do a deep traversal.
			if (unlikely((target_j = jobmgr_find_by_pid_deep(root_jobmgr, target_pid, true)) == NULL)) {
				jobmgr_log(jm, LOG_DEBUG, "Didn't find PID %i", target_pid);
				return NULL;
			}
		}

		SLIST_FOREACH(ms, &target_j->machservices, sle) {
			if (ms->per_pid && strcmp(name, ms->name) == 0) {
				return ms;
			}
		}

		job_log(target_j, LOG_DEBUG, "Didn't find per-PID Mach service: %s", name);
		return NULL;
	}

	jobmgr_t where2look = jm;
	// XPC domains are separate from Mach bootstraps.
	if (!(jm->properties & BOOTSTRAP_PROPERTY_XPC_DOMAIN)) {
		if (launchd_flat_mach_namespace && !(jm->properties & BOOTSTRAP_PROPERTY_EXPLICITSUBSET)) {
			where2look = root_jobmgr;
		}
	}

	LIST_FOREACH(ms, &where2look->ms_hash[hash_ms(name)], name_hash_sle) {
		if (!ms->per_pid && strcmp(name, ms->name) == 0) {
			return ms;
		}
	}

	if (jm->parentmgr == NULL || !check_parent) {
		return NULL;
	}

	return jobmgr_lookup_service(jm->parentmgr, name, true, 0);
}

mach_port_t
machservice_port(struct machservice *ms)
{
	return ms->port;
}

job_t 
machservice_job(struct machservice *ms)
{
	return ms->job;
}

bool
machservice_hidden(struct machservice *ms)
{
	return ms->hide;
}

bool
machservice_active(struct machservice *ms)
{
	return ms->isActive;
}

const char *
machservice_name(struct machservice *ms)
{
	return ms->name;
}

void
machservice_drain_port(struct machservice *ms)
{
	bool drain_one = ms->drain_one_on_crash;
	bool drain_all = ms->drain_all_on_crash;

	if (!job_assumes(ms->job, (drain_one || drain_all) == true)) {
		return;
	}

	job_log(ms->job, LOG_INFO, "Draining %s...", ms->name);

	char req_buff[sizeof(union __RequestUnion__catch_mach_exc_subsystem) * 2];
	char rep_buff[sizeof(union __ReplyUnion__catch_mach_exc_subsystem)];
	mig_reply_error_t *req_hdr = (mig_reply_error_t *)&req_buff;
	mig_reply_error_t *rep_hdr = (mig_reply_error_t *)&rep_buff;

	mach_msg_return_t mr = ~MACH_MSG_SUCCESS;

	do {
		/* This should be a direct check on the Mach service to see if it's an exception-handling
		 * port, and it will break things if ReportCrash or SafetyNet start advertising other
		 * Mach services. But for now, it should be okay.
		 */
		if (ms->job->alt_exc_handler || ms->job->internal_exc_handler) {
			mr = launchd_exc_runtime_once(ms->port, sizeof(req_buff), sizeof(rep_buff), req_hdr, rep_hdr, 0);
		} else {
			mach_msg_options_t options =	MACH_RCV_MSG		|
											MACH_RCV_TIMEOUT	;

			mr = mach_msg((mach_msg_header_t *)req_hdr, options, 0, sizeof(req_buff), ms->port, 0, MACH_PORT_NULL);
			switch (mr) {
			case MACH_MSG_SUCCESS:
				mach_msg_destroy((mach_msg_header_t *)req_hdr);
				break;
			case MACH_RCV_TIMED_OUT:
				break;
			case MACH_RCV_TOO_LARGE:
				launchd_syslog(LOG_WARNING, "Tried to receive message that was larger than %lu bytes", sizeof(req_buff));
				break;
			default:
				break;
			}
		}
	} while (drain_all && mr != MACH_RCV_TIMED_OUT);
}

void
machservice_delete(job_t j, struct machservice *ms, bool port_died)
{
	if (ms->alias) {
		/* HACK: Egregious code duplication. But dealing with aliases is a
		 * pretty simple affair since they can't and shouldn't have any complex
		 * behaviors associated with them.
		 */
		LIST_REMOVE(ms, name_hash_sle);
		SLIST_REMOVE(&j->machservices, ms, machservice, sle);
		free(ms);
		return;
	}

	if (unlikely(ms->debug_on_close)) {
		job_log(j, LOG_NOTICE, "About to enter kernel debugger because of Mach port: 0x%x", ms->port);
		(void)job_assumes_zero(j, host_reboot(mach_host_self(), HOST_REBOOT_DEBUGGER));
	}

	if (ms->recv && job_assumes(j, !machservice_active(ms))) {
		job_log(j, LOG_DEBUG, "Closing receive right for %s", ms->name);
		(void)job_assumes_zero(j, launchd_mport_close_recv(ms->port));
	}

	(void)job_assumes_zero(j, launchd_mport_deallocate(ms->port));

	if (unlikely(ms->port == the_exception_server)) {
		the_exception_server = 0;
	}

	job_log(j, LOG_DEBUG, "Mach service deleted%s: %s", port_died ? " (port died)" : "", ms->name);

	if (ms->special_port_num) {
		SLIST_REMOVE(&special_ports, ms, machservice, special_port_sle);
	}
	SLIST_REMOVE(&j->machservices, ms, machservice, sle);

	if (!(j->dedicated_instance || ms->event_channel)) {
		LIST_REMOVE(ms, name_hash_sle);
	}
	LIST_REMOVE(ms, port_hash_sle);

	free(ms);
}

void
machservice_request_notifications(struct machservice *ms)
{
	mach_msg_id_t which = MACH_NOTIFY_DEAD_NAME;

	ms->isActive = true;

	if (ms->recv) {
		which = MACH_NOTIFY_PORT_DESTROYED;
		job_checkin(ms->job);
	}

	(void)job_assumes_zero(ms->job, launchd_mport_notify_req(ms->port, which));
}

#define NELEM(x) (sizeof(x)/sizeof(x[0]))
#define END_OF(x) (&(x)[NELEM(x)])

char **
mach_cmd2argv(const char *string)
{
	char *argv[100], args[1000];
	const char *cp;
	char *argp = args, term, **argv_ret, *co;
	unsigned int nargs = 0, i;

	for (cp = string; *cp;) {
		while (isspace(*cp))
			cp++;
		term = (*cp == '"') ? *cp++ : '\0';
		if (nargs < NELEM(argv)) {
			argv[nargs++] = argp;
		}
		while (*cp && (term ? *cp != term : !isspace(*cp)) && argp < END_OF(args)) {
			if (*cp == '\\') {
				cp++;
			}
			*argp++ = *cp;
			if (*cp) {
				cp++;
			}
		}
		*argp++ = '\0';
	}
	argv[nargs] = NULL;

	if (nargs == 0) {
		return NULL;
	}

	argv_ret = malloc((nargs + 1) * sizeof(char *) + strlen(string) + 1);

	if (!argv_ret) {
		(void)os_assumes_zero(errno);
		return NULL;
	}

	co = (char *)argv_ret + (nargs + 1) * sizeof(char *);

	for (i = 0; i < nargs; i++) {
		strcpy(co, argv[i]);
		argv_ret[i] = co;
		co += strlen(argv[i]) + 1;
	}
	argv_ret[i] = NULL;

	return argv_ret;
}

void
job_checkin(job_t j)
{
	j->checkedin = true;
}

bool job_is_god(job_t j)
{
	return j->embedded_god;
}

bool
job_ack_port_destruction(mach_port_t p)
{
	struct machservice *ms;
	job_t j;

	LIST_FOREACH(ms, &port_hash[HASH_PORT(p)], port_hash_sle) {
		if (ms->recv && (ms->port == p)) {
			break;
		}
	}

	if (!ms) {
		launchd_syslog(LOG_WARNING, "Could not find MachService to match receive right: 0x%x", p);
		return false;
	}

	j = ms->job;

	jobmgr_log(root_jobmgr, LOG_DEBUG, "Receive right returned to us: %s", ms->name);

	/* Without being the exception handler, NOTE_EXIT is our only way to tell if
	 * the job  crashed, and we can't rely on NOTE_EXIT always being processed
	 * after all the job's receive rights have been returned.
	 *
	 * So when we get receive rights back, check to see if the job has been
	 * reaped yet. If not, then we add this service to a list of services to be
	 * drained on crash if it's requested that behavior. So, for a job with N
	 * receive rights all requesting that they be drained on crash, we can
	 * safely handle the following sequence of events.
	 * 
	 * ReceiveRight0Returned
	 * ReceiveRight1Returned
	 * ReceiveRight2Returned
	 * NOTE_EXIT (reap, get exit status)
	 * ReceiveRight3Returned
	 * .
	 * .
	 * .
	 * ReceiveRight(N - 1)Returned
	 */
	if (ms->drain_one_on_crash || ms->drain_all_on_crash) {
		if (j->crashed && j->reaped) {
			job_log(j, LOG_DEBUG, "Job has crashed. Draining port...");
			machservice_drain_port(ms);
		} else if (!(j->crashed || j->reaped)) {
			job_log(j, LOG_DEBUG, "Job's exit status is still unknown. Deferring drain.");
		}
	}

	ms->isActive = false;
	if (ms->delete_on_destruction) {
		machservice_delete(j, ms, false);
	} else if (ms->reset) {
		machservice_resetport(j, ms);
	}

	kern_return_t kr = mach_port_set_attributes(mach_task_self(), ms->port, MACH_PORT_TEMPOWNER, NULL, 0);
	(void)job_assumes_zero(j, kr);
	machservice_stamp_port(j, ms);
	job_dispatch(j, false);

	if (ms->recv_race_hack) {
		ms->recv_race_hack = false;
		machservice_watch(ms->job, ms);
	}

	root_jobmgr = jobmgr_do_garbage_collection(root_jobmgr);

	return true;
}

void
job_ack_no_senders(job_t j)
{
	j->priv_port_has_senders = false;

	(void)job_assumes_zero(j, launchd_mport_close_recv(j->j_port));
	j->j_port = 0;

	job_log(j, LOG_DEBUG, "No more senders on privileged Mach bootstrap port");

	job_dispatch(j, false);
}

bool
semaphoreitem_new(job_t j, semaphore_reason_t why, const char *what)
{
	struct semaphoreitem *si;
	size_t alloc_sz = sizeof(struct semaphoreitem);

	if (what) {
		alloc_sz += strlen(what) + 1;
	}

	if (job_assumes(j, si = calloc(1, alloc_sz)) == NULL) {
		return false;
	}

	si->why = why;

	if (what) {
		strcpy(si->what_init, what);
	}

	SLIST_INSERT_HEAD(&j->semaphores, si, sle);

	if ((why == OTHER_JOB_ENABLED || why == OTHER_JOB_DISABLED) && !j->nosy) {
		job_log(j, LOG_DEBUG, "Job is interested in \"%s\".", what);
		SLIST_INSERT_HEAD(&s_curious_jobs, j, curious_jobs_sle);
		j->nosy = true;
	}

	semaphoreitem_runtime_mod_ref(si, true);

	return true;
}

void
semaphoreitem_runtime_mod_ref(struct semaphoreitem *si, bool add)
{
	/*
	 * External events need to be tracked.
	 * Internal events do NOT need to be tracked.
	 */

	switch (si->why) {
	case SUCCESSFUL_EXIT:
	case FAILED_EXIT:
	case OTHER_JOB_ENABLED:
	case OTHER_JOB_DISABLED:
	case OTHER_JOB_ACTIVE:
	case OTHER_JOB_INACTIVE:
		return;
	default:
		break;
	}

	if (add) {
		runtime_add_weak_ref();
	} else {
		runtime_del_weak_ref();
	}
}

void
semaphoreitem_delete(job_t j, struct semaphoreitem *si)
{
	semaphoreitem_runtime_mod_ref(si, false);

	SLIST_REMOVE(&j->semaphores, si, semaphoreitem, sle);

	// We'll need to rethink this if it ever becomes possible to dynamically add or remove semaphores.
	if ((si->why == OTHER_JOB_ENABLED || si->why == OTHER_JOB_DISABLED) && j->nosy) {
		j->nosy = false;
		SLIST_REMOVE(&s_curious_jobs, j, job_s, curious_jobs_sle);
	}

	free(si);
}

void
semaphoreitem_setup_dict_iter(launch_data_t obj, const char *key, void *context)
{
	struct semaphoreitem_dict_iter_context *sdic = context;
	semaphore_reason_t why;

	why = launch_data_get_bool(obj) ? sdic->why_true : sdic->why_false;

	semaphoreitem_new(sdic->j, why, key);
}

void
semaphoreitem_setup(launch_data_t obj, const char *key, void *context)
{
	struct semaphoreitem_dict_iter_context sdic = { context, 0, 0 };
	job_t j = context;
	semaphore_reason_t why;

	switch (launch_data_get_type(obj)) {
	case LAUNCH_DATA_BOOL:
		if (strcasecmp(key, LAUNCH_JOBKEY_KEEPALIVE_NETWORKSTATE) == 0) {
			why = launch_data_get_bool(obj) ? NETWORK_UP : NETWORK_DOWN;
			semaphoreitem_new(j, why, NULL);
		} else if (strcasecmp(key, LAUNCH_JOBKEY_KEEPALIVE_SUCCESSFULEXIT) == 0) {
			why = launch_data_get_bool(obj) ? SUCCESSFUL_EXIT : FAILED_EXIT;
			semaphoreitem_new(j, why, NULL);
			j->start_pending = true;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_KEEPALIVE_AFTERINITIALDEMAND) == 0) {
			j->needs_kickoff = launch_data_get_bool(obj);
		} else if (strcasecmp(key, LAUNCH_JOBKEY_KEEPALIVE_CRASHED) == 0) {
			why = launch_data_get_bool(obj) ? CRASHED : DID_NOT_CRASH;
			semaphoreitem_new(j, why, NULL);
			j->start_pending = true;
		} else {
			job_log(j, LOG_ERR, "Unrecognized KeepAlive attribute: %s", key);
		}
		break;
	case LAUNCH_DATA_DICTIONARY:
		if (strcasecmp(key, LAUNCH_JOBKEY_KEEPALIVE_OTHERJOBACTIVE) == 0) {
			sdic.why_true = OTHER_JOB_ACTIVE;
			sdic.why_false = OTHER_JOB_INACTIVE;
		} else if (strcasecmp(key, LAUNCH_JOBKEY_KEEPALIVE_OTHERJOBENABLED) == 0) {
			sdic.why_true = OTHER_JOB_ENABLED;
			sdic.why_false = OTHER_JOB_DISABLED;
		} else {
			job_log(j, LOG_ERR, "Unrecognized KeepAlive attribute: %s", key);
			break;
		}

		launch_data_dict_iterate(obj, semaphoreitem_setup_dict_iter, &sdic);
		break;
	default:
		job_log(j, LOG_ERR, "Unrecognized KeepAlive type: %u", launch_data_get_type(obj));
		break;
	}
}

bool
externalevent_new(job_t j, struct eventsystem *sys, const char *evname, xpc_object_t event, uint64_t flags)
{
	if (j->event_monitor) {
		job_log(j, LOG_ERR, "The event monitor job cannot use LaunchEvents or XPC Events.");
		return false;
	}

	struct externalevent *ee = (struct externalevent *)calloc(1, sizeof(struct externalevent) + strlen(evname) + 1);
	if (!ee) {
		return false;
	}

	ee->event = xpc_retain(event);
	(void)strcpy(ee->name, evname);
	ee->job = j;
	ee->id = sys->curid;
	ee->sys = sys;
	ee->state = false;
	ee->wanted_state = true;
	sys->curid++;

	if (flags & XPC_EVENT_FLAG_ENTITLEMENTS) {
		struct ldcred *ldc = runtime_get_caller_creds();
		if (ldc) {
			ee->entitlements = xpc_copy_entitlements_for_pid(ldc->pid);
		}
	}

	if (sys == _launchd_support_system) {
		ee->internal = true;
	}

	LIST_INSERT_HEAD(&j->events, ee, job_le);
	LIST_INSERT_HEAD(&sys->events, ee, sys_le);

	job_log(j, LOG_DEBUG, "New event: %s/%s", sys->name, evname);

	eventsystem_ping();
	return true;
}

void
externalevent_delete(struct externalevent *ee)
{
	xpc_release(ee->event);
	if (ee->entitlements) {
		xpc_release(ee->entitlements);
	}
	LIST_REMOVE(ee, job_le);
	LIST_REMOVE(ee, sys_le);

	free(ee);

	eventsystem_ping();
}

void
externalevent_setup(launch_data_t obj, const char *key, void *context)
{
	/* This method can ONLY be called on the job_import() path, as it assumes
	 * the input is a launch_data_t.
	 */
	struct externalevent_iter_ctx *ctx = (struct externalevent_iter_ctx *)context;

	xpc_object_t xobj = ld2xpc(obj);
	if (xobj) {
		job_log(ctx->j, LOG_DEBUG, "Importing stream/event: %s/%s", ctx->sys->name, key);
		externalevent_new(ctx->j, ctx->sys, key, xobj, 0);
		xpc_release(xobj);
	} else {
		job_log(ctx->j, LOG_ERR, "Could not import event for job: %s", key);
	}
}

struct externalevent *
externalevent_find(const char *sysname, uint64_t id)
{
	struct externalevent *ei = NULL;

	struct eventsystem *es = eventsystem_find(sysname);
	if (es != NULL) {
		LIST_FOREACH(ei, &es->events, sys_le) {
			if (ei->id == id) {
				break;
			}
		}
	} else {
		launchd_syslog(LOG_ERR, "Could not find event system: %s", sysname);
	}

	return ei;
}

struct eventsystem *
eventsystem_new(const char *name)
{
	struct eventsystem *es = (struct eventsystem *)calloc(1, sizeof(struct eventsystem) + strlen(name) + 1);
	if (es != NULL) {
		es->curid = 1;
		(void)strcpy(es->name, name);
		LIST_INSERT_HEAD(&_s_event_systems, es, global_le);
	} else {
		(void)os_assumes_zero(errno);
	}

	return es;
}

void
eventsystem_delete(struct eventsystem *es)
{
	struct externalevent *ei = NULL;
	while ((ei = LIST_FIRST(&es->events))) {
		externalevent_delete(ei);
	}

	LIST_REMOVE(es, global_le);

	free(es);
}

void
eventsystem_setup(launch_data_t obj, const char *key, void *context)
{
	job_t j = (job_t)context;
	if (!job_assumes(j, launch_data_get_type(obj) == LAUNCH_DATA_DICTIONARY)) {
		return;
	}

	struct eventsystem *sys = eventsystem_find(key);
	if (unlikely(sys == NULL)) {
		sys = eventsystem_new(key);
		job_log(j, LOG_DEBUG, "New event system: %s", key);
	}

	if (job_assumes(j, sys != NULL)) {
		struct externalevent_iter_ctx ctx = {
			.j = j,
			.sys = sys,
		};

		job_log(j, LOG_DEBUG, "Importing events for stream: %s", key);
		launch_data_dict_iterate(obj, externalevent_setup, &ctx);
	}
}

struct eventsystem *
eventsystem_find(const char *name)
{
	struct eventsystem *esi = NULL;
	LIST_FOREACH(esi, &_s_event_systems, global_le) {
		if (strcmp(name, esi->name) == 0) {
			break;
		}
	}

	return esi;
}

void
eventsystem_ping(void)
{
	if (!_launchd_event_monitor) {
		return;
	}

	if (!_launchd_event_monitor->p) {
		(void)job_dispatch(_launchd_event_monitor, true);
	} else {
		if (_launchd_event_monitor->event_monitor_ready2signal) {
			(void)job_assumes_zero_p(_launchd_event_monitor, kill(_launchd_event_monitor->p, SIGUSR1));
		}
	}
}

void
jobmgr_dispatch_all_semaphores(jobmgr_t jm)
{
	jobmgr_t jmi, jmn;
	job_t ji, jn;


	SLIST_FOREACH_SAFE(jmi, &jm->submgrs, sle, jmn) {
		jobmgr_dispatch_all_semaphores(jmi);
	}

	LIST_FOREACH_SAFE(ji, &jm->jobs, sle, jn) {
		if (!SLIST_EMPTY(&ji->semaphores)) {
			job_dispatch(ji, false);
		}
	}
}

time_t
cronemu(int mon, int mday, int hour, int min)
{
	struct tm workingtm;
	time_t now;

	now = time(NULL);
	workingtm = *localtime(&now);

	workingtm.tm_isdst = -1;
	workingtm.tm_sec = 0;
	workingtm.tm_min++;

	while (!cronemu_mon(&workingtm, mon, mday, hour, min)) {
		workingtm.tm_year++;
		workingtm.tm_mon = 0;
		workingtm.tm_mday = 1;
		workingtm.tm_hour = 0;
		workingtm.tm_min = 0;
		mktime(&workingtm);
	}

	return mktime(&workingtm);
}

time_t
cronemu_wday(int wday, int hour, int min)
{
	struct tm workingtm;
	time_t now;

	now = time(NULL);
	workingtm = *localtime(&now);

	workingtm.tm_isdst = -1;
	workingtm.tm_sec = 0;
	workingtm.tm_min++;

	if (wday == 7) {
		wday = 0;
	}

	while (!(workingtm.tm_wday == wday && cronemu_hour(&workingtm, hour, min))) {
		workingtm.tm_mday++;
		workingtm.tm_hour = 0;
		workingtm.tm_min = 0;
		mktime(&workingtm);
	}

	return mktime(&workingtm);
}

bool
cronemu_mon(struct tm *wtm, int mon, int mday, int hour, int min)
{
	if (mon == -1) {
		struct tm workingtm = *wtm;
		int carrytest;

		while (!cronemu_mday(&workingtm, mday, hour, min)) {
			workingtm.tm_mon++;
			workingtm.tm_mday = 1;
			workingtm.tm_hour = 0;
			workingtm.tm_min = 0;
			carrytest = workingtm.tm_mon;
			mktime(&workingtm);
			if (carrytest != workingtm.tm_mon) {
				return false;
			}
		}
		*wtm = workingtm;
		return true;
	}

	if (mon < wtm->tm_mon) {
		return false;
	}

	if (mon > wtm->tm_mon) {
		wtm->tm_mon = mon;
		wtm->tm_mday = 1;
		wtm->tm_hour = 0;
		wtm->tm_min = 0;
	}

	return cronemu_mday(wtm, mday, hour, min);
}

bool
cronemu_mday(struct tm *wtm, int mday, int hour, int min)
{
	if (mday == -1) {
		struct tm workingtm = *wtm;
		int carrytest;

		while (!cronemu_hour(&workingtm, hour, min)) {
			workingtm.tm_mday++;
			workingtm.tm_hour = 0;
			workingtm.tm_min = 0;
			carrytest = workingtm.tm_mday;
			mktime(&workingtm);
			if (carrytest != workingtm.tm_mday) {
				return false;
			}
		}
		*wtm = workingtm;
		return true;
	}

	if (mday < wtm->tm_mday) {
		return false;
	}

	if (mday > wtm->tm_mday) {
		wtm->tm_mday = mday;
		wtm->tm_hour = 0;
		wtm->tm_min = 0;
	}

	return cronemu_hour(wtm, hour, min);
}

bool
cronemu_hour(struct tm *wtm, int hour, int min)
{
	if (hour == -1) {
		struct tm workingtm = *wtm;
		int carrytest;

		while (!cronemu_min(&workingtm, min)) {
			workingtm.tm_hour++;
			workingtm.tm_min = 0;
			carrytest = workingtm.tm_hour;
			mktime(&workingtm);
			if (carrytest != workingtm.tm_hour) {
				return false;
			}
		}
		*wtm = workingtm;
		return true;
	}

	if (hour < wtm->tm_hour) {
		return false;
	}

	if (hour > wtm->tm_hour) {
		wtm->tm_hour = hour;
		wtm->tm_min = 0;
	}

	return cronemu_min(wtm, min);
}

bool
cronemu_min(struct tm *wtm, int min)
{
	if (min == -1) {
		return true;
	}

	if (min < wtm->tm_min) {
		return false;
	}

	if (min > wtm->tm_min) {
		wtm->tm_min = min;
	}

	return true;
}

kern_return_t
job_mig_create_server(job_t j, cmd_t server_cmd, uid_t server_uid, boolean_t on_demand, mach_port_t *server_portp)
{
	struct ldcred *ldc = runtime_get_caller_creds();
	job_t js;

	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	if (unlikely(j->deny_job_creation)) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

#if HAVE_SANDBOX
	const char **argv = (const char **)mach_cmd2argv(server_cmd);
	if (unlikely(argv == NULL)) {
		return BOOTSTRAP_NO_MEMORY;
	}
	if (unlikely(sandbox_check(ldc->pid, "job-creation", SANDBOX_FILTER_PATH, argv[0]) > 0)) {
		free(argv);
		return BOOTSTRAP_NOT_PRIVILEGED;
	}
	free(argv);
#endif

	job_log(j, LOG_DEBUG, "Server create attempt: %s", server_cmd);

	if (pid1_magic) {
		if (ldc->euid || ldc->uid) {
			job_log(j, LOG_WARNING, "Server create attempt moved to per-user launchd: %s", server_cmd);
			return VPROC_ERR_TRY_PER_USER;
		}
	} else {
		if (unlikely(server_uid != getuid())) {
			job_log(j, LOG_WARNING, "Server create: \"%s\": As UID %d, we will not be able to switch to UID %d",
					server_cmd, getuid(), server_uid);
		}
		server_uid = 0; // zero means "do nothing"
	}

	js = job_new_via_mach_init(j, server_cmd, server_uid, on_demand);

	if (unlikely(js == NULL)) {
		return BOOTSTRAP_NO_MEMORY;
	}

	*server_portp = js->j_port;
	return BOOTSTRAP_SUCCESS;
}

kern_return_t
job_mig_send_signal(job_t j, mach_port_t srp, name_t targetlabel, int sig)
{
	struct ldcred *ldc = runtime_get_caller_creds();
	job_t otherj;

	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	if (unlikely(ldc->euid != 0 && ldc->euid != getuid()) || j->deny_job_creation) {
#if TARGET_OS_EMBEDDED	
		if (!j->embedded_god) {
			return BOOTSTRAP_NOT_PRIVILEGED;
		}
#else
		return BOOTSTRAP_NOT_PRIVILEGED;
#endif
	}

#if HAVE_SANDBOX
	if (unlikely(sandbox_check(ldc->pid, "job-creation", SANDBOX_FILTER_NONE) > 0)) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}
#endif

	if (unlikely(!(otherj = job_find(NULL, targetlabel)))) {
		return BOOTSTRAP_UNKNOWN_SERVICE;
	}

#if TARGET_OS_EMBEDDED
	if (j->embedded_god) {
		if (j->username && otherj->username) {
			if (strcmp(j->username, otherj->username) != 0) {
				return BOOTSTRAP_NOT_PRIVILEGED;
			}
		} else {
			return BOOTSTRAP_NOT_PRIVILEGED;
		}
	}
#endif	

	if (sig == VPROC_MAGIC_UNLOAD_SIGNAL) {
		bool do_block = otherj->p;

		if (otherj->anonymous) {
			return BOOTSTRAP_NOT_PRIVILEGED;
		}

		job_remove(otherj);

		if (do_block) {
			job_log(j, LOG_DEBUG, "Blocking MIG return of job_remove(): %s", otherj->label);
			// this is messy. We shouldn't access 'otherj' after job_remove(), but we check otherj->p first...
			(void)job_assumes(otherj, waiting4removal_new(otherj, srp));
			return MIG_NO_REPLY;
		} else {
			return 0;
		}
	} else if (otherj->p) {
		(void)job_assumes_zero_p(j, kill2(otherj->p, sig));
	}

	return 0;
}

kern_return_t
job_mig_log_forward(job_t j, vm_offset_t inval, mach_msg_type_number_t invalCnt)
{
	struct ldcred *ldc = runtime_get_caller_creds();

	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	if (!job_assumes(j, j->per_user)) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

	return launchd_log_forward(ldc->euid, ldc->egid, inval, invalCnt);
}

kern_return_t
job_mig_log_drain(job_t j, mach_port_t srp, vm_offset_t *outval, mach_msg_type_number_t *outvalCnt)
{
	struct ldcred *ldc = runtime_get_caller_creds();

	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	if (unlikely(ldc->euid)) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

	return launchd_log_drain(srp, outval, outvalCnt);
}

kern_return_t
job_mig_swap_complex(job_t j, vproc_gsk_t inkey, vproc_gsk_t outkey,
	vm_offset_t inval, mach_msg_type_number_t invalCnt, vm_offset_t *outval,
	mach_msg_type_number_t *outvalCnt)
{
	const char *action;
	launch_data_t input_obj = NULL, output_obj = NULL;
	size_t data_offset = 0;
	size_t packed_size;
	struct ldcred *ldc = runtime_get_caller_creds();

	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	if (inkey && ldc->pid != j->p) {
		if (ldc->euid && ldc->euid != getuid()) {
			return BOOTSTRAP_NOT_PRIVILEGED;
		}
 	}

	if (unlikely(inkey && outkey && !job_assumes(j, inkey == outkey))) {
		return 1;
	}

	if (inkey && outkey) {
		action = "Swapping";
	} else if (inkey) {
		action = "Setting";
	} else {
		action = "Getting";
	}

	job_log(j, LOG_DEBUG, "%s key: %u", action, inkey ? inkey : outkey);

	*outvalCnt = 20 * 1024 * 1024;
	mig_allocate(outval, *outvalCnt);
	if (!job_assumes(j, *outval != 0)) {
		return 1;
	}

	/* Note to future maintainers: launch_data_unpack() does NOT return a heap
	 * object. The data is decoded in-place. So do not call launch_data_free()
	 * on input_obj.
	 */
	runtime_ktrace0(RTKT_LAUNCHD_DATA_UNPACK);
	if (unlikely(invalCnt && !job_assumes(j, (input_obj = launch_data_unpack((void *)inval, invalCnt, NULL, 0, &data_offset, NULL)) != NULL))) {
		goto out_bad;
	}

	char *store = NULL;
	switch (outkey) {
	case VPROC_GSK_ENVIRONMENT:
		if (!job_assumes(j, (output_obj = launch_data_alloc(LAUNCH_DATA_DICTIONARY)))) {
			goto out_bad;
		}
		jobmgr_export_env_from_other_jobs(j->mgr, output_obj);
		runtime_ktrace0(RTKT_LAUNCHD_DATA_PACK);
		if (!job_assumes(j, launch_data_pack(output_obj, (void *)*outval, *outvalCnt, NULL, NULL) != 0)) {
			goto out_bad;
		}
		launch_data_free(output_obj);
		break;
	case VPROC_GSK_ALLJOBS:
		if (!job_assumes(j, (output_obj = job_export_all()) != NULL)) {
			goto out_bad;
		}
		ipc_revoke_fds(output_obj);
		runtime_ktrace0(RTKT_LAUNCHD_DATA_PACK);
		packed_size = launch_data_pack(output_obj, (void *)*outval, *outvalCnt, NULL, NULL);
		if (!job_assumes(j, packed_size != 0)) {
			goto out_bad;
		}
		launch_data_free(output_obj);
		break;
	case VPROC_GSK_MGR_NAME:
		if (!job_assumes(j, (output_obj = launch_data_new_string(j->mgr->name)) != NULL)) {
			goto out_bad;
		}
		packed_size = launch_data_pack(output_obj, (void *)*outval, *outvalCnt, NULL, NULL);
		if (!job_assumes(j, packed_size != 0)) {
			goto out_bad;
		}

		launch_data_free(output_obj);
		break;
	case VPROC_GSK_JOB_OVERRIDES_DB:
		store = launchd_copy_persistent_store(LAUNCHD_PERSISTENT_STORE_DB, "overrides.plist");
		if (!store || !job_assumes(j, (output_obj = launch_data_new_string(store)) != NULL)) {
			free(store);
			goto out_bad;
		}

		free(store);
		packed_size = launch_data_pack(output_obj, (void *)*outval, *outvalCnt, NULL, NULL);
		if (!job_assumes(j, packed_size != 0)) {
			goto out_bad;
		}

		launch_data_free(output_obj);
		break;
	case VPROC_GSK_ZERO:
		mig_deallocate(*outval, *outvalCnt);
		*outval = 0;
		*outvalCnt = 0;
		break;
	default:
		goto out_bad;
	}

	mig_deallocate(inval, invalCnt);
	return 0;

out_bad:
	mig_deallocate(inval, invalCnt);
	if (*outval) {
		mig_deallocate(*outval, *outvalCnt);
	}
	if (output_obj) {
		launch_data_free(output_obj);
	}

	return 1;
}

kern_return_t
job_mig_swap_integer(job_t j, vproc_gsk_t inkey, vproc_gsk_t outkey, int64_t inval, int64_t *outval)
{
	const char *action;
	kern_return_t kr = 0;
	struct ldcred *ldc = runtime_get_caller_creds();
	int oldmask;

	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	if (inkey && ldc->pid != j->p) {
		if (ldc->euid && ldc->euid != getuid()) {
			return BOOTSTRAP_NOT_PRIVILEGED;
		}
 	}

	if (unlikely(inkey && outkey && !job_assumes(j, inkey == outkey))) {
		return 1;
	}

	if (inkey && outkey) {
		action = "Swapping";
	} else if (inkey) {
		action = "Setting";
	} else {
		action = "Getting";
	}

	job_log(j, LOG_DEBUG, "%s key: %u", action, inkey ? inkey : outkey);

	switch (outkey) {
	case VPROC_GSK_ABANDON_PROCESS_GROUP:
		*outval = j->abandon_pg;
		break;
	case VPROC_GSK_LAST_EXIT_STATUS:
		*outval = j->last_exit_status;
		break;
	case VPROC_GSK_MGR_UID:
		*outval = getuid();
		break;
	case VPROC_GSK_MGR_PID:
		*outval = getpid();
		break;
	case VPROC_GSK_IS_MANAGED:
		*outval = j->anonymous ? 0 : 1;
		break;
	case VPROC_GSK_BASIC_KEEPALIVE:
		*outval = !j->ondemand;
		break;
	case VPROC_GSK_START_INTERVAL:
		*outval = j->start_interval;
		break;
	case VPROC_GSK_IDLE_TIMEOUT:
		*outval = j->timeout;
		break;
	case VPROC_GSK_EXIT_TIMEOUT:
		*outval = j->exit_timeout;
		break;
	case VPROC_GSK_GLOBAL_LOG_MASK:
		oldmask = runtime_setlogmask(LOG_UPTO(LOG_DEBUG));
		*outval = oldmask;
		runtime_setlogmask(oldmask);
		break;
	case VPROC_GSK_GLOBAL_UMASK:
		oldmask = umask(0);
		*outval = oldmask;
		umask(oldmask);
		break;
	case VPROC_GSK_TRANSACTIONS_ENABLED:
		job_log(j, LOG_DEBUG, "Reading EnableTransactions value.");
		*outval = j->enable_transactions;
		break;
	case VPROC_GSK_WAITFORDEBUGGER:
		*outval = j->wait4debugger;
		break;
	case VPROC_GSK_EMBEDDEDROOTEQUIVALENT:
		*outval = j->embedded_god;
		break;
	case VPROC_GSK_ZERO:
		*outval = 0;
		break;
	default:
		kr = 1;
		break;
	}

	switch (inkey) {
	case VPROC_GSK_ABANDON_PROCESS_GROUP:
		j->abandon_pg = (bool)inval;
		break;
	case VPROC_GSK_GLOBAL_ON_DEMAND:
		job_log(j, LOG_DEBUG, "Job has set global on-demand mode to: %s", inval ? "true" : "false");
		kr = job_set_global_on_demand(j, inval);
		break;
	case VPROC_GSK_BASIC_KEEPALIVE:
		j->ondemand = !inval;
		break;
	case VPROC_GSK_START_INTERVAL:
		if (inval > UINT32_MAX || inval < 0) {
			kr = 1;
		} else if (inval) {
			if (j->start_interval == 0) {
				runtime_add_weak_ref();
			}
			j->start_interval = (typeof(j->start_interval)) inval;
			(void)job_assumes_zero_p(j, kevent_mod((uintptr_t)&j->start_interval, EVFILT_TIMER, EV_ADD, NOTE_SECONDS, j->start_interval, j));
		} else if (j->start_interval) {
			(void)job_assumes_zero_p(j, kevent_mod((uintptr_t)&j->start_interval, EVFILT_TIMER, EV_DELETE, 0, 0, NULL));
			if (j->start_interval != 0) {
				runtime_del_weak_ref();
			}
			j->start_interval = 0;
		}
		break;
	case VPROC_GSK_IDLE_TIMEOUT:
		if (inval < 0 || inval > UINT32_MAX) {
			kr = 1;
		} else {
			j->timeout = (typeof(j->timeout)) inval;
		}
		break;
	case VPROC_GSK_EXIT_TIMEOUT:
		if (inval < 0 || inval > UINT32_MAX) {
			kr = 1;
		} else {
			j->exit_timeout = (typeof(j->exit_timeout)) inval;
		}
		break;
	case VPROC_GSK_GLOBAL_LOG_MASK:
		if (inval < 0 || inval > UINT32_MAX) {
			kr = 1;
		} else {
			runtime_setlogmask((int) inval);
		}
		break;
	case VPROC_GSK_GLOBAL_UMASK:
		__OS_COMPILETIME_ASSERT__(sizeof (mode_t) == 2);
		if (inval < 0 || inval > UINT16_MAX) {
			kr = 1;
		} else {
#if HAVE_SANDBOX
			if (unlikely(sandbox_check(ldc->pid, "job-creation", SANDBOX_FILTER_NONE) > 0)) {
				kr = 1;
			} else {
				umask((mode_t) inval);
			}
#endif
		}
		break;
	case VPROC_GSK_TRANSACTIONS_ENABLED:
		/* No-op. */
		break;
	case VPROC_GSK_WEIRD_BOOTSTRAP:
		if (job_assumes(j, j->weird_bootstrap)) {
			job_log(j, LOG_DEBUG, "Unsetting weird bootstrap.");

			mach_msg_size_t mxmsgsz = (typeof(mxmsgsz)) sizeof(union __RequestUnion__job_mig_job_subsystem);

			if (job_mig_job_subsystem.maxsize > mxmsgsz) {
				mxmsgsz = job_mig_job_subsystem.maxsize;
			}

			(void)job_assumes_zero(j, runtime_add_mport(j->mgr->jm_port, job_server));
			j->weird_bootstrap = false;
		}
		break;
	case VPROC_GSK_WAITFORDEBUGGER:
		j->wait4debugger_oneshot = inval;
		break;
	case VPROC_GSK_PERUSER_SUSPEND:
		if (job_assumes(j, pid1_magic && ldc->euid == 0)) {
			mach_port_t junk = MACH_PORT_NULL;
			job_t jpu = jobmgr_lookup_per_user_context_internal(j, (uid_t)inval, &junk);
			if (job_assumes(j, jpu != NULL)) {
				struct suspended_peruser *spi = NULL;
				LIST_FOREACH(spi, &j->suspended_perusers, sle) {
					if ((int64_t)(spi->j->mach_uid) == inval) {
						job_log(j, LOG_WARNING, "Job tried to suspend per-user launchd for UID %lli twice.", inval);
						break;
					}
				}

				if (spi == NULL) {
					job_log(j, LOG_INFO, "Job is suspending the per-user launchd for UID %lli.", inval);
					spi = (struct suspended_peruser *)calloc(sizeof(struct suspended_peruser), 1);
					if (job_assumes(j, spi != NULL)) {
						/* Stop listening for events.
						 *
						 * See <rdar://problem/9014146>.
						 */
						if (jpu->peruser_suspend_count == 0) {
							job_ignore(jpu);
						}

						spi->j = jpu;
						spi->j->peruser_suspend_count++;
						LIST_INSERT_HEAD(&j->suspended_perusers, spi, sle);
						job_stop(spi->j);
						*outval = jpu->p;
					} else {
						kr = BOOTSTRAP_NO_MEMORY;
					}
				}
			}
		} else {
			kr = 1;
		}
		break;
	case VPROC_GSK_PERUSER_RESUME:
		if (job_assumes(j, pid1_magic == true)) {
			struct suspended_peruser *spi = NULL, *spt = NULL;
			LIST_FOREACH_SAFE(spi, &j->suspended_perusers, sle, spt) {
				if ((int64_t)(spi->j->mach_uid) == inval) {
					spi->j->peruser_suspend_count--;
					LIST_REMOVE(spi, sle);
					job_log(j, LOG_INFO, "Job is resuming the per-user launchd for UID %lli.", inval);
					break;
				}
			}

			if (!job_assumes(j, spi != NULL)) {
				job_log(j, LOG_WARNING, "Job tried to resume per-user launchd for UID %lli that it did not suspend.", inval);
				kr = BOOTSTRAP_NOT_PRIVILEGED;
			} else if (spi->j->peruser_suspend_count == 0) {
				job_watch(spi->j);
				job_dispatch(spi->j, false);
				free(spi);
			}
		} else {
			kr = 1;
		}
		break;
	case VPROC_GSK_ZERO:
		break;
	default:
		kr = 1;
		break;
	}

	return kr;
}

kern_return_t
job_mig_post_fork_ping(job_t j, task_t child_task, mach_port_t *asport)
{
	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	job_log(j, LOG_DEBUG, "Post fork ping.");

	struct machservice *ms;
	job_setup_exception_port(j, child_task);
	SLIST_FOREACH(ms, &special_ports, special_port_sle) {
		if (j->per_user && (ms->special_port_num != TASK_ACCESS_PORT)) {
			// The TASK_ACCESS_PORT funny business is to workaround 5325399.
			continue;
		}

		errno = task_set_special_port(child_task, ms->special_port_num, ms->port);
		if (errno) {
			if (errno == MACH_SEND_INVALID_DEST) {
				job_log(j, LOG_WARNING, "Task died before special ports could be set.");
				break;
			}

			int desired_log_level = LOG_ERR;
			if (j->anonymous) {
				// 5338127

				desired_log_level = LOG_WARNING;

				if (ms->special_port_num == TASK_SEATBELT_PORT) {
					desired_log_level = LOG_DEBUG;
				}
			}

			job_log(j, desired_log_level, "Could not setup Mach task special port %u: %s", ms->special_port_num, mach_error_string(errno));
		}
	}

	/* MIG will not zero-initialize this pointer, so we must always do so.
	 *
	 * <rdar://problem/8562593>.
	 */
	*asport = MACH_PORT_NULL;
#if !TARGET_OS_EMBEDDED
	if (!j->anonymous) {
		/* XPC services will spawn into the root security session by default.
		 * xpcproxy will switch them away if needed.
		 */
		if (!(j->mgr->properties & BOOTSTRAP_PROPERTY_XPC_DOMAIN)) {
			job_log(j, LOG_DEBUG, "Returning session port: 0x%x", j->asport);
			*asport = j->asport;
		}
	}
#endif
	(void)job_assumes_zero(j, launchd_mport_deallocate(child_task));

	return 0;
}

kern_return_t
job_mig_get_listener_port_rights(job_t j, mach_port_array_t *sports, mach_msg_type_number_t *sports_cnt)
{
	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	size_t cnt = 0;
	struct machservice *msi = NULL;
	SLIST_FOREACH(msi, &j->machservices, sle) {
		if (msi->upfront && job_assumes(j, msi->recv)) {
			cnt++;
		}
	}

	if (cnt == 0) {
		return BOOTSTRAP_UNKNOWN_SERVICE;
	}

	mach_port_array_t sports2 = NULL;
	mig_allocate((vm_address_t *)&sports2, cnt * sizeof(sports2[0]));
	if (!sports2) {
		return BOOTSTRAP_NO_MEMORY;
	}

	size_t i = 0;
	SLIST_FOREACH(msi, &j->machservices, sle) {
		if (msi->upfront && msi->recv) {
			sports2[i] = msi->port;
			i++;
		}
	}

	*sports = sports2;
	*sports_cnt = cnt;

	return KERN_SUCCESS;
}

kern_return_t
job_mig_register_gui_session(job_t j, mach_port_t asport)
{
	if (!j->per_user) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

	jobmgr_t jm = jobmgr_find_xpc_per_user_domain(root_jobmgr, j->mach_uid);
	if (!jm) {
		return BOOTSTRAP_UNKNOWN_SERVICE;
	}

	if (jm->req_gui_asport) {
		// This job manager persists, so we need to allow the per-user launchd
		// to update the GUI session as it comes and goes.
		jobmgr_assumes_zero(jm, launchd_mport_deallocate(jm->req_gui_asport));
	}

	jm->req_gui_asport = asport;
	return KERN_SUCCESS;
}

kern_return_t
job_mig_reboot2(job_t j, uint64_t flags)
{
	char who_started_the_reboot[2048] = "";
	struct proc_bsdshortinfo proc;
	struct ldcred *ldc = runtime_get_caller_creds();
	pid_t pid_to_log;

	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	if (unlikely(!pid1_magic)) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

#if !TARGET_OS_EMBEDDED
	if (unlikely(ldc->euid)) {
#else
	if (unlikely(ldc->euid) && !j->embedded_god) {
#endif
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

	for (pid_to_log = ldc->pid; pid_to_log; pid_to_log = proc.pbsi_ppid) {
		size_t who_offset;
		if (proc_pidinfo(pid_to_log, PROC_PIDT_SHORTBSDINFO, 1, &proc, PROC_PIDT_SHORTBSDINFO_SIZE) == 0) {
			if (errno != ESRCH) {
				(void)job_assumes_zero(j, errno);
			}
			return 1;
		}

		if (!job_assumes(j, pid_to_log != (pid_t)proc.pbsi_ppid)) {
			job_log(j, LOG_WARNING, "Job which is its own parent started reboot.");
			snprintf(who_started_the_reboot, sizeof(who_started_the_reboot), "%s[%u]->%s[%u]->%s[%u]->...", proc.pbsi_comm, pid_to_log, proc.pbsi_comm, pid_to_log, proc.pbsi_comm, pid_to_log);
			break;
		}

		who_offset = strlen(who_started_the_reboot);
		snprintf(who_started_the_reboot + who_offset, sizeof(who_started_the_reboot) - who_offset,
				" %s[%u]%s", proc.pbsi_comm, pid_to_log, proc.pbsi_ppid ? " ->" : "");
	}

	root_jobmgr->reboot_flags = (int)flags;
	job_log(j, LOG_DEBUG, "reboot2() initiated by:%s", who_started_the_reboot);
	launchd_shutdown();

	return 0;
}

kern_return_t
job_mig_getsocket(job_t j, name_t spr)
{
	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	if (j->deny_job_creation) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

#if HAVE_SANDBOX
	struct ldcred *ldc = runtime_get_caller_creds(); 
	if (unlikely(sandbox_check(ldc->pid, "job-creation", SANDBOX_FILTER_NONE) > 0)) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}
#endif

	ipc_server_init();

	if (unlikely(!sockpath)) {
		return BOOTSTRAP_NO_MEMORY;
	}

	strncpy(spr, sockpath, sizeof(name_t));

	return BOOTSTRAP_SUCCESS;
}

kern_return_t
job_mig_log(job_t j, int pri, int err, logmsg_t msg)
{
	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	if ((errno = err)) {
		job_log_error(j, pri, "%s", msg);
	} else {
		job_log(j, pri, "%s", msg);
	}

	return 0;
}

void
job_setup_per_user_directory(job_t j, uid_t uid, const char *path)
{
	struct stat sb;

	bool created = false;
	int r = stat(path, &sb);
	if ((r == -1 && errno == ENOENT) || (r == 0 && !S_ISDIR(sb.st_mode))) {
		if (r == 0) {
			job_log(j, LOG_NOTICE, "File at location of per-user launchd directory is not a directory. Moving aside: %s", path);

			char old[PATH_MAX];
			snprintf(old, sizeof(old), "%s.movedaside", path);
			(void)job_assumes_zero_p(j, rename(path, old));
		}

		(void)job_assumes_zero_p(j, mkdir(path, S_IRWXU));
		(void)job_assumes_zero_p(j, chown(path, uid, 0));
		created = true;
	}

	if (!created) {
		if (sb.st_uid != uid) {
			job_log(j, LOG_NOTICE, "Per-user launchd directory has improper user ownership. Repairing: %s", path);
			(void)job_assumes_zero_p(j, chown(path, uid, 0));
		}
		if (sb.st_gid != 0) {
			job_log(j, LOG_NOTICE, "Per-user launchd directory has improper group ownership. Repairing: %s", path);
			(void)job_assumes_zero_p(j, chown(path, uid, 0));
		}
		if (sb.st_mode != (S_IRWXU | S_IFDIR)) {
			job_log(j, LOG_NOTICE, "Per-user launchd directory has improper mode. Repairing: %s", path);
			(void)job_assumes_zero_p(j, chmod(path, S_IRWXU));
		}
	}
}

void
job_setup_per_user_directories(job_t j, uid_t uid, const char *label)
{
	char path[PATH_MAX];

	(void)snprintf(path, sizeof(path), LAUNCHD_DB_PREFIX "/%s", label);
	job_setup_per_user_directory(j, uid, path);

	(void)snprintf(path, sizeof(path), LAUNCHD_LOG_PREFIX "/%s", label);
	job_setup_per_user_directory(j, uid, path);
}

job_t
jobmgr_lookup_per_user_context_internal(job_t j, uid_t which_user, mach_port_t *mp)
{
	job_t ji = NULL;
	LIST_FOREACH(ji, &root_jobmgr->jobs, sle) {
		if (!ji->per_user) {
			continue;
		}
		if (ji->mach_uid != which_user) {
			continue;
		}
		if (SLIST_EMPTY(&ji->machservices)) {
			continue;
		}
		if (!SLIST_FIRST(&ji->machservices)->per_user_hack) {
			continue;
		}
		break;
	}

	if (unlikely(ji == NULL)) {
		struct machservice *ms;
		char lbuf[1024];

		job_log(j, LOG_DEBUG, "Creating per user launchd job for UID: %u", which_user);

		sprintf(lbuf, "com.apple.launchd.peruser.%u", which_user);

		ji = job_new(root_jobmgr, lbuf, "/sbin/launchd", NULL);

		if (ji != NULL) {
			auditinfo_addr_t auinfo = {
				.ai_termid = { 
					.at_type = AU_IPv4
				},
				.ai_auid = which_user,
				.ai_asid = AU_ASSIGN_ASID,
			};

			if (setaudit_addr(&auinfo, sizeof(auinfo)) == 0) {
				job_log(ji, LOG_DEBUG, "Created new security session for per-user launchd: %u", auinfo.ai_asid);
				(void)job_assumes(ji, (ji->asport = audit_session_self()) != MACH_PORT_NULL);

				/* Kinda lame that we have to do this, but we can't create an
				 * audit session without joining it.
				 */
				(void)job_assumes(ji, audit_session_join(launchd_audit_port));
				ji->asid = auinfo.ai_asid;
			} else {
				job_log(ji, LOG_WARNING, "Could not set audit session!");
				job_remove(ji);
				return NULL;
			}

			ji->mach_uid = which_user;
			ji->per_user = true;
			ji->enable_transactions = true;
			job_setup_per_user_directories(ji, which_user, lbuf);

			if ((ms = machservice_new(ji, lbuf, mp, false)) == NULL) {
				job_remove(ji);
				ji = NULL;
			} else {
				ms->upfront = true;
				ms->per_user_hack = true;
				ms->hide = true;

				ji = job_dispatch(ji, false);
			}
		}
	} else {
		*mp = machservice_port(SLIST_FIRST(&ji->machservices));
		job_log(j, LOG_DEBUG, "Per user launchd job found for UID: %u", which_user);
	}

	return ji;
}

kern_return_t
job_mig_lookup_per_user_context(job_t j, uid_t which_user, mach_port_t *up_cont)
{
	struct ldcred *ldc = runtime_get_caller_creds();
	job_t jpu;

	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	if (launchd_osinstaller) {
		return BOOTSTRAP_UNKNOWN_SERVICE;
	}

#if TARGET_OS_EMBEDDED
	// There is no need for per-user launchd's on embedded.
	job_log(j, LOG_ERR, "Per-user launchds are not supported on this platform.");
	return BOOTSTRAP_UNKNOWN_SERVICE;
#endif

#if HAVE_SANDBOX
	if (unlikely(sandbox_check(ldc->pid, "mach-per-user-lookup", SANDBOX_FILTER_NONE) > 0)) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}
#endif

	job_log(j, LOG_INFO, "Looking up per user launchd for UID: %u", which_user);

	if (unlikely(!pid1_magic)) {
		job_log(j, LOG_ERR, "Only PID 1 supports per user launchd lookups.");
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

	if (ldc->euid || ldc->uid) {
		which_user = ldc->euid ?: ldc->uid;
	}

	*up_cont = MACH_PORT_NULL;

	jpu = jobmgr_lookup_per_user_context_internal(j, which_user, up_cont);

	return 0;
}

kern_return_t
job_mig_check_in2(job_t j, name_t servicename, mach_port_t *serviceportp, uuid_t instance_id, uint64_t flags)
{
	bool per_pid_service = flags & BOOTSTRAP_PER_PID_SERVICE;
	bool strict = flags & BOOTSTRAP_STRICT_CHECKIN;
	struct ldcred *ldc = runtime_get_caller_creds();
	struct machservice *ms = NULL;
	job_t jo;

	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	if (j->dedicated_instance) {
		struct machservice *msi = NULL;
		SLIST_FOREACH(msi, &j->machservices, sle) {
			if (strncmp(servicename, msi->name, sizeof(name_t) - 1) == 0) {
				uuid_copy(instance_id, j->instance_id);
				ms = msi;
				break;
			}
		}
	} else {
		ms = jobmgr_lookup_service(j->mgr, servicename, false, per_pid_service ? ldc->pid : 0);
	}

	if (strict) {
		if (likely(ms != NULL)) {
			if (ms->job != j) {
				return BOOTSTRAP_NOT_PRIVILEGED;
			} else if (ms->isActive) {
				return BOOTSTRAP_SERVICE_ACTIVE;
			}
		} else {
			return BOOTSTRAP_UNKNOWN_SERVICE;
		}
	} else if (ms == NULL) {
		if (job_assumes(j, !j->dedicated_instance)) {
			*serviceportp = MACH_PORT_NULL;

#if HAVE_SANDBOX
			if (unlikely(sandbox_check(ldc->pid, "mach-register", per_pid_service ? SANDBOX_FILTER_LOCAL_NAME : SANDBOX_FILTER_GLOBAL_NAME, servicename) > 0)) {
				return BOOTSTRAP_NOT_PRIVILEGED;
			}
#endif
			if (unlikely((ms = machservice_new(j, servicename, serviceportp, per_pid_service)) == NULL)) {
				return BOOTSTRAP_NO_MEMORY;
			}

			// Treat this like a legacy job.
			if (!j->legacy_mach_job) {
				ms->isActive = true;
				ms->recv = false;
			}

			if (!(j->anonymous || j->legacy_LS_job || j->legacy_mach_job)) {
				job_log(j, LOG_APPLEONLY, "Please add the following service to the configuration file for this job: %s", servicename);
			}
		} else {
			return BOOTSTRAP_UNKNOWN_SERVICE;
		}
	} else {
		if (unlikely((jo = machservice_job(ms)) != j)) {
			static pid_t last_warned_pid;

			if (last_warned_pid != ldc->pid) {
				job_log(jo, LOG_WARNING, "The following job tried to hijack the service \"%s\" from this job: %s", servicename, j->label);
				last_warned_pid = ldc->pid;
			}

			return BOOTSTRAP_NOT_PRIVILEGED;
		}
		if (unlikely(machservice_active(ms))) {
			job_log(j, LOG_WARNING, "Check-in of Mach service failed. Already active: %s", servicename);
			return BOOTSTRAP_SERVICE_ACTIVE;
		}
	}

	job_checkin(j);
	machservice_request_notifications(ms);

	job_log(j, LOG_INFO, "Check-in of service: %s", servicename);

	*serviceportp = machservice_port(ms);
	return BOOTSTRAP_SUCCESS;
}

kern_return_t
job_mig_register2(job_t j, name_t servicename, mach_port_t serviceport, uint64_t flags)
{
	struct machservice *ms;
	struct ldcred *ldc = runtime_get_caller_creds();
	bool per_pid_service = flags & BOOTSTRAP_PER_PID_SERVICE;

	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	if (!per_pid_service && !j->legacy_LS_job) {
		job_log(j, LOG_APPLEONLY, "Performance: bootstrap_register() is deprecated. Service: %s", servicename);
	}

	job_log(j, LOG_DEBUG, "%sMach service registration attempt: %s", flags & BOOTSTRAP_PER_PID_SERVICE ? "Per PID " : "", servicename);

#if HAVE_SANDBOX
	if (unlikely(sandbox_check(ldc->pid, "mach-register", per_pid_service ? SANDBOX_FILTER_LOCAL_NAME : SANDBOX_FILTER_GLOBAL_NAME, servicename) > 0)) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}
#endif

	// 5641783 for the embedded hack
#if !TARGET_OS_EMBEDDED
	/*
	 * From a per-user/session launchd's perspective, SecurityAgent (UID
	 * 92) is a rogue application (not our UID, not root and not a child of
	 * us). We'll have to reconcile this design friction at a later date.
	 */
	if (unlikely(j->anonymous && j->mgr->parentmgr == NULL && ldc->uid != 0 && ldc->uid != getuid() && ldc->uid != 92)) {
		if (pid1_magic) {
			return VPROC_ERR_TRY_PER_USER;
		} else {
			return BOOTSTRAP_NOT_PRIVILEGED;
		}
	}
#endif

	ms = jobmgr_lookup_service(j->mgr, servicename, false, flags & BOOTSTRAP_PER_PID_SERVICE ? ldc->pid : 0);

	if (unlikely(ms)) {
		if (machservice_job(ms) != j) {
			return BOOTSTRAP_NOT_PRIVILEGED;
		}
		if (machservice_active(ms)) {
			job_log(j, LOG_DEBUG, "Mach service registration failed. Already active: %s", servicename);
			return BOOTSTRAP_SERVICE_ACTIVE;
		}
		if (ms->recv && (serviceport != MACH_PORT_NULL)) {
			job_log(j, LOG_ERR, "bootstrap_register() erroneously called instead of bootstrap_check_in(). Mach service: %s", servicename);
			return BOOTSTRAP_NOT_PRIVILEGED;
		}
		job_checkin(j);
		machservice_delete(j, ms, false);
	}

	if (likely(serviceport != MACH_PORT_NULL)) {
		if (likely(ms = machservice_new(j, servicename, &serviceport, flags & BOOTSTRAP_PER_PID_SERVICE ? true : false))) {
			machservice_request_notifications(ms);
		} else {
			return BOOTSTRAP_NO_MEMORY;
		}
	}


	return BOOTSTRAP_SUCCESS;
}

kern_return_t
job_mig_look_up2(job_t j, mach_port_t srp, name_t servicename, mach_port_t *serviceportp, pid_t target_pid, uuid_t instance_id, uint64_t flags)
{
	struct machservice *ms = NULL;
	struct ldcred *ldc = runtime_get_caller_creds();
	kern_return_t kr;
	bool per_pid_lookup = flags & BOOTSTRAP_PER_PID_SERVICE;
	bool specific_instance = flags & BOOTSTRAP_SPECIFIC_INSTANCE;
	bool strict_lookup = flags & BOOTSTRAP_STRICT_LOOKUP;
	bool privileged = flags & BOOTSTRAP_PRIVILEGED_SERVER;

	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	bool xpc_req = (j->mgr->properties & BOOTSTRAP_PROPERTY_XPC_DOMAIN);

	// 5641783 for the embedded hack
#if !TARGET_OS_EMBEDDED
	if (unlikely(pid1_magic && j->anonymous && j->mgr->parentmgr == NULL && ldc->uid != 0 && ldc->euid != 0)) {
		return VPROC_ERR_TRY_PER_USER;
	}
#endif

#if HAVE_SANDBOX
	/* We don't do sandbox checking for XPC domains because, by definition, all
	 * the services within your domain should be accessible to you.
	 */
	if (!xpc_req && unlikely(sandbox_check(ldc->pid, "mach-lookup", per_pid_lookup ? SANDBOX_FILTER_LOCAL_NAME : SANDBOX_FILTER_GLOBAL_NAME, servicename) > 0)) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}
#endif

	if (per_pid_lookup) {
		ms = jobmgr_lookup_service(j->mgr, servicename, false, target_pid);
	} else {
		if (xpc_req) {
			// Requests from XPC domains stay local.
			ms = jobmgr_lookup_service(j->mgr, servicename, false, 0);
		} else {
			/* A strict lookup which is privileged won't even bother trying to
			 * find a service if we're not hosting the root Mach bootstrap.
			 */
			if (strict_lookup && privileged) {
				if (inherited_bootstrap_port == MACH_PORT_NULL) {
					ms = jobmgr_lookup_service(j->mgr, servicename, true, 0);
				}
			} else {
				ms = jobmgr_lookup_service(j->mgr, servicename, true, 0);
			}
		}
	}

	if (likely(ms)) {
		ms = ms->alias ? ms->alias : ms;
		if (unlikely(specific_instance && ms->job->multiple_instances)) {
			job_t ji = NULL;
			job_t instance = NULL;
			LIST_FOREACH(ji, &ms->job->subjobs, subjob_sle) {
				if (uuid_compare(instance_id, ji->instance_id) == 0) {
					instance = ji;
					break;
				}
			}

			if (unlikely(instance == NULL)) {
				job_log(ms->job, LOG_DEBUG, "Creating new instance of job based on lookup of service %s", ms->name);
				instance = job_new_subjob(ms->job, instance_id);
				if (job_assumes(j, instance != NULL)) {
					/* Disable this support for now. We only support having 
					 * multi-instance jobs within private XPC domains.
					 */
#if 0
					/* If the job is multi-instance, in a singleton XPC domain
					 * and the request is not coming from within that singleton
					 * domain, we need to alias the new job into the requesting
					 * domain.
					 */
					if (!j->mgr->xpc_singleton && xpc_req) {
						(void)job_assumes(instance, job_new_alias(j->mgr, instance));
					}
#endif
					job_dispatch(instance, false);
				}
			}

			ms = NULL;
			if (job_assumes(j, instance != NULL)) {
				struct machservice *msi = NULL;
				SLIST_FOREACH(msi, &instance->machservices, sle) {
					/* sizeof(servicename) will return the size of a pointer,
					 * even though it's an array type, because when passing
					 * arrays as parameters in C, they implicitly degrade to
					 * pointers.
					 */
					if (strncmp(servicename, msi->name, sizeof(name_t) - 1) == 0) {
						ms = msi;
						break;
					}
				}
			}
		} else {
			if (machservice_hidden(ms) && !machservice_active(ms)) {
				ms = NULL;
			} else if (unlikely(ms->per_user_hack)) {
				ms = NULL;
			}
		}
	}

	if (likely(ms)) {
		(void)job_assumes(j, machservice_port(ms) != MACH_PORT_NULL);
		job_log(j, LOG_DEBUG, "%sMach service lookup: %s", per_pid_lookup ? "Per PID " : "", servicename);
		*serviceportp = machservice_port(ms);

		kr = BOOTSTRAP_SUCCESS;
	} else if (strict_lookup && !privileged) {
		/* Hack: We need to simulate XPC's desire not to establish a hierarchy.
		 * So if XPC is doing the lookup, and it's not a privileged lookup, we
		 * won't forward. But if it is a privileged lookup, then we must
		 * forward.
		 */
		return BOOTSTRAP_UNKNOWN_SERVICE;
	} else if (inherited_bootstrap_port != MACH_PORT_NULL) {
		// Requests from within an XPC domain don't get forwarded.
		job_log(j, LOG_DEBUG, "Mach service lookup forwarded: %s", servicename);
		/* Clients potentially check the audit token of the reply to verify that
		 * the returned send right is trustworthy.
		 */
		(void)job_assumes_zero(j, vproc_mig_look_up2_forward(inherited_bootstrap_port, srp, servicename, target_pid, instance_id, flags));
		return MIG_NO_REPLY;
	} else if (pid1_magic && j->anonymous && ldc->euid >= 500 && strcasecmp(j->mgr->name, VPROCMGR_SESSION_LOGINWINDOW) == 0) {
		/* 5240036 Should start background session when a lookup of CCacheServer
		 * occurs
		 *
		 * This is a total hack. We sniff out loginwindow session, and attempt
		 * to guess what it is up to. If we find a EUID that isn't root, we
		 * force it over to the per-user context.
		 */
		return VPROC_ERR_TRY_PER_USER;
	} else {
		job_log(j, LOG_DEBUG, "%sMach service lookup failed: %s", per_pid_lookup ? "Per PID " : "", servicename);
		kr = BOOTSTRAP_UNKNOWN_SERVICE;
	}

	return kr;
}

kern_return_t
job_mig_parent(job_t j, mach_port_t srp, mach_port_t *parentport)
{
	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	job_log(j, LOG_DEBUG, "Requested parent bootstrap port");
	jobmgr_t jm = j->mgr;

	if (jobmgr_parent(jm)) {
		*parentport = jobmgr_parent(jm)->jm_port;
	} else if (MACH_PORT_NULL == inherited_bootstrap_port) {
		*parentport = jm->jm_port;
	} else {
		(void)job_assumes_zero(j, vproc_mig_parent_forward(inherited_bootstrap_port, srp));
		// The previous routine moved the reply port, we're forced to return MIG_NO_REPLY now
		return MIG_NO_REPLY;
	}
	return BOOTSTRAP_SUCCESS;
}

kern_return_t
job_mig_get_root_bootstrap(job_t j, mach_port_t *rootbsp)
{
	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	if (inherited_bootstrap_port == MACH_PORT_NULL) {
		*rootbsp = root_jobmgr->jm_port;
		(void)job_assumes_zero(j, launchd_mport_make_send(root_jobmgr->jm_port));
	} else {
		*rootbsp = inherited_bootstrap_port;
		(void)job_assumes_zero(j, launchd_mport_copy_send(inherited_bootstrap_port));
	}

	return BOOTSTRAP_SUCCESS;
}

kern_return_t
job_mig_info(job_t j, name_array_t *servicenamesp,
	unsigned int *servicenames_cnt, name_array_t *servicejobsp,
	unsigned int *servicejobs_cnt, bootstrap_status_array_t *serviceactivesp,
	unsigned int *serviceactives_cnt, uint64_t flags)
{
	name_array_t service_names = NULL;
	name_array_t service_jobs = NULL;
	bootstrap_status_array_t service_actives = NULL;
	unsigned int cnt = 0, cnt2 = 0;
	jobmgr_t jm;

	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

#if TARGET_OS_EMBEDDED
	struct ldcred *ldc = runtime_get_caller_creds();
	if (ldc->euid) {
		return EPERM;
	}
#endif // TARGET_OS_EMBEDDED

	if (launchd_flat_mach_namespace) {
		if ((j->mgr->properties & BOOTSTRAP_PROPERTY_EXPLICITSUBSET) || (flags & BOOTSTRAP_FORCE_LOCAL)) {
			jm = j->mgr;
		} else {
			jm = root_jobmgr;
		}
	} else {
		jm = j->mgr;
	}

	unsigned int i = 0;
	struct machservice *msi = NULL;
	for (i = 0; i < MACHSERVICE_HASH_SIZE; i++) {
		LIST_FOREACH(msi, &jm->ms_hash[i], name_hash_sle) {
			cnt += !msi->per_pid ? 1 : 0;
		}
	}

	if (cnt == 0) {
		goto out;
	}

	mig_allocate((vm_address_t *)&service_names, cnt * sizeof(service_names[0]));
	if (!job_assumes(j, service_names != NULL)) {
		goto out_bad;
	}

	mig_allocate((vm_address_t *)&service_jobs, cnt * sizeof(service_jobs[0]));
	if (!job_assumes(j, service_jobs != NULL)) {
		goto out_bad;
	}

	mig_allocate((vm_address_t *)&service_actives, cnt * sizeof(service_actives[0]));
	if (!job_assumes(j, service_actives != NULL)) {
		goto out_bad;
	}

	for (i = 0; i < MACHSERVICE_HASH_SIZE; i++) {
		LIST_FOREACH(msi, &jm->ms_hash[i], name_hash_sle) {
			if (!msi->per_pid) {
				strlcpy(service_names[cnt2], machservice_name(msi), sizeof(service_names[0]));
				msi = msi->alias ? msi->alias : msi;
				if (msi->job->mgr->shortdesc) {
					strlcpy(service_jobs[cnt2], msi->job->mgr->shortdesc, sizeof(service_jobs[0]));
				} else {
					strlcpy(service_jobs[cnt2], msi->job->label, sizeof(service_jobs[0]));
				}
				service_actives[cnt2] = machservice_status(msi);
				cnt2++;
			}
		}
	}

	(void)job_assumes(j, cnt == cnt2);

out:
	*servicenamesp = service_names;
	*servicejobsp = service_jobs;
	*serviceactivesp = service_actives;
	*servicenames_cnt = *servicejobs_cnt = *serviceactives_cnt = cnt;

	return BOOTSTRAP_SUCCESS;

out_bad:
	if (service_names) {
		mig_deallocate((vm_address_t)service_names, cnt * sizeof(service_names[0]));
	}
	if (service_jobs) {
		mig_deallocate((vm_address_t)service_jobs, cnt * sizeof(service_jobs[0]));
	}
	if (service_actives) {
		mig_deallocate((vm_address_t)service_actives, cnt * sizeof(service_actives[0]));
	}

	return BOOTSTRAP_NO_MEMORY;
}

kern_return_t
job_mig_lookup_children(job_t j, mach_port_array_t *child_ports,
	mach_msg_type_number_t *child_ports_cnt, name_array_t *child_names,
	mach_msg_type_number_t *child_names_cnt,
	bootstrap_property_array_t *child_properties,
	mach_msg_type_number_t *child_properties_cnt)
{
	kern_return_t kr = BOOTSTRAP_NO_MEMORY;
	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	struct ldcred *ldc = runtime_get_caller_creds();

	/* Only allow root processes to look up children, even if we're in the per-user launchd.
	 * Otherwise, this could be used to cross sessions, which counts as a security vulnerability
	 * in a non-flat namespace.
	 */
	if (ldc->euid != 0) {
		job_log(j, LOG_WARNING, "Attempt to look up children of bootstrap by unprivileged job.");
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

	unsigned int cnt = 0;

	jobmgr_t jmr = j->mgr;
	jobmgr_t jmi = NULL;
	SLIST_FOREACH(jmi, &jmr->submgrs, sle) {
		cnt++;
	}

	// Find our per-user launchds if we're PID 1.
	job_t ji = NULL;
	if (pid1_magic) {
		LIST_FOREACH(ji, &jmr->jobs, sle) {
			cnt += ji->per_user ? 1 : 0;
		}
	}

	if (cnt == 0) {
		return BOOTSTRAP_NO_CHILDREN;
	}

	mach_port_array_t _child_ports = NULL;
	name_array_t _child_names = NULL;
	bootstrap_property_array_t _child_properties = NULL;

	mig_allocate((vm_address_t *)&_child_ports, cnt * sizeof(_child_ports[0]));
	if (!job_assumes(j, _child_ports != NULL)) {
		kr = BOOTSTRAP_NO_MEMORY;
		goto out_bad;
	}

	mig_allocate((vm_address_t *)&_child_names, cnt * sizeof(_child_names[0]));
	if (!job_assumes(j, _child_names != NULL)) {
		kr = BOOTSTRAP_NO_MEMORY;
		goto out_bad;
	}

	mig_allocate((vm_address_t *)&_child_properties, cnt * sizeof(_child_properties[0]));
	if (!job_assumes(j, _child_properties != NULL)) {
		kr = BOOTSTRAP_NO_MEMORY;
		goto out_bad;
	}

	unsigned int cnt2 = 0;
	SLIST_FOREACH(jmi, &jmr->submgrs, sle) {
		if (jobmgr_assumes_zero(jmi, launchd_mport_make_send(jmi->jm_port)) == KERN_SUCCESS) {
			_child_ports[cnt2] = jmi->jm_port;
		} else {
			_child_ports[cnt2] = MACH_PORT_NULL;
		}

		strlcpy(_child_names[cnt2], jmi->name, sizeof(_child_names[0]));
		_child_properties[cnt2] = jmi->properties;

		cnt2++;
	}

	if (pid1_magic) LIST_FOREACH(ji, &jmr->jobs, sle) {
		if (ji->per_user) {
			if (job_assumes(ji, SLIST_FIRST(&ji->machservices)->per_user_hack == true)) {
				mach_port_t port = machservice_port(SLIST_FIRST(&ji->machservices));

				if (job_assumes_zero(ji, launchd_mport_copy_send(port)) == KERN_SUCCESS) {
					_child_ports[cnt2] = port;
				} else {
					_child_ports[cnt2] = MACH_PORT_NULL;
				}
			} else {
				_child_ports[cnt2] = MACH_PORT_NULL;
			}

			strlcpy(_child_names[cnt2], ji->label, sizeof(_child_names[0]));
			_child_properties[cnt2] |= BOOTSTRAP_PROPERTY_PERUSER;

			cnt2++;
		}
	}

	*child_names_cnt = cnt;
	*child_ports_cnt = cnt;
	*child_properties_cnt = cnt;

	*child_names = _child_names;
	*child_ports = _child_ports;
	*child_properties = _child_properties;

	unsigned int i = 0;
	for (i = 0; i < cnt; i++) {
		job_log(j, LOG_DEBUG, "child_names[%u] = %s", i, (char *)_child_names[i]);
	}

	return BOOTSTRAP_SUCCESS;
out_bad:
	if (_child_ports) {
		mig_deallocate((vm_address_t)_child_ports, cnt * sizeof(_child_ports[0]));
	}

	if (_child_names) {
		mig_deallocate((vm_address_t)_child_names, cnt * sizeof(_child_names[0]));
	}

	if (_child_properties) {
		mig_deallocate((vm_address_t)_child_properties, cnt * sizeof(_child_properties[0]));
	}

	return kr;
}

kern_return_t
job_mig_pid_is_managed(job_t j __attribute__((unused)), pid_t p, boolean_t *managed)
{
	struct ldcred *ldc = runtime_get_caller_creds();
	if ((ldc->euid != geteuid()) && (ldc->euid != 0)) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

	/* This is so loginwindow doesn't try to quit GUI apps that have been launched
	 * directly by launchd as agents.
	 */
	job_t j_for_pid = jobmgr_find_by_pid_deep(root_jobmgr, p, false);
	if (j_for_pid && !j_for_pid->anonymous && !j_for_pid->legacy_LS_job) {
		*managed = true;
	}

	return BOOTSTRAP_SUCCESS;
}

kern_return_t
job_mig_port_for_label(job_t j __attribute__((unused)), name_t label, mach_port_t *mp)
{
	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	struct ldcred *ldc = runtime_get_caller_creds();
	kern_return_t kr = BOOTSTRAP_NOT_PRIVILEGED;

#if HAVE_SANDBOX
	if (unlikely(sandbox_check(ldc->pid, "job-creation", SANDBOX_FILTER_NONE) > 0)) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}
#endif

	mach_port_t _mp = MACH_PORT_NULL;
	if (!j->deny_job_creation && (ldc->euid == 0 || ldc->euid == geteuid())) {
		job_t target_j = job_find(NULL, label);
		if (jobmgr_assumes(root_jobmgr, target_j != NULL)) {
			if (target_j->j_port == MACH_PORT_NULL) {
				(void)job_assumes(target_j, job_setup_machport(target_j) == true);
			}

			_mp = target_j->j_port;
			kr = _mp != MACH_PORT_NULL ? BOOTSTRAP_SUCCESS : BOOTSTRAP_NO_MEMORY;
		} else {
			kr = BOOTSTRAP_NO_MEMORY;
		}
	}

	*mp = _mp;
	return kr;
}

kern_return_t
job_mig_set_security_session(job_t j, uuid_t uuid, mach_port_t asport)
{
#if TARGET_OS_EMBEDDED
	return KERN_SUCCESS;
#endif

	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	uuid_string_t uuid_str;
	uuid_unparse(uuid, uuid_str);
	job_log(j, LOG_DEBUG, "Setting session %u for UUID %s...", asport, uuid_str);

	job_t ji = NULL, jt = NULL;
	LIST_FOREACH_SAFE(ji, &s_needing_sessions, sle, jt) {
		uuid_string_t uuid_str2;
		uuid_unparse(ji->expected_audit_uuid, uuid_str2);

		if (uuid_compare(uuid, ji->expected_audit_uuid) == 0) {
			uuid_clear(ji->expected_audit_uuid);
			if (asport != MACH_PORT_NULL) {
				job_log(ji, LOG_DEBUG, "Job should join session with port 0x%x", asport);
				(void)job_assumes_zero(j, launchd_mport_copy_send(asport));
			} else {
				job_log(ji, LOG_DEBUG, "No session to set for job. Using our session.");
			}

			ji->asport = asport;
			LIST_REMOVE(ji, needing_session_sle);

			if (ji->event_monitor) {
				eventsystem_ping();
			} else {
				job_dispatch(ji, false);
			}
		}
	}

	/* Each job that the session port was set for holds a reference. At the end of
	 * the loop, there will be one extra reference belonging to this MiG protocol.
	 * We need to release it so that the session goes away when all the jobs
	 * referencing it are unloaded.
	 */
	(void)job_assumes_zero(j, launchd_mport_deallocate(asport));

	return KERN_SUCCESS;
}

jobmgr_t 
jobmgr_find_by_name(jobmgr_t jm, const char *where)
{
	jobmgr_t jmi, jmi2;

	// NULL is only passed for our custom API for LaunchServices. If that is the case, we do magic.
	if (where == NULL) {
		if (strcasecmp(jm->name, VPROCMGR_SESSION_LOGINWINDOW) == 0) {
			where = VPROCMGR_SESSION_LOGINWINDOW;
		} else {
			where = VPROCMGR_SESSION_AQUA;
		}
	}

	if (strcasecmp(jm->name, where) == 0) {
		return jm;
	}

	if (strcasecmp(where, VPROCMGR_SESSION_BACKGROUND) == 0 && !pid1_magic) {
		jmi = root_jobmgr;
		goto jm_found;
	}

	SLIST_FOREACH(jmi, &root_jobmgr->submgrs, sle) {	
		if (unlikely(jmi->shutting_down)) {
			continue;
		} else if (jmi->properties & BOOTSTRAP_PROPERTY_XPC_DOMAIN) {
			continue;
		} else if (strcasecmp(jmi->name, where) == 0) {
			goto jm_found;
		} else if (strcasecmp(jmi->name, VPROCMGR_SESSION_BACKGROUND) == 0 && pid1_magic) {
			SLIST_FOREACH(jmi2, &jmi->submgrs, sle) {
				if (strcasecmp(jmi2->name, where) == 0) {
					jmi = jmi2;
					goto jm_found;
				}
			}
		}
	}

jm_found:
	return jmi;
}

kern_return_t
job_mig_move_subset(job_t j, mach_port_t target_subset, name_t session_type, mach_port_t asport, uint64_t flags)
{
	mach_msg_type_number_t l2l_i, l2l_port_cnt = 0;
	mach_port_array_t l2l_ports = NULL;
	mach_port_t reqport, rcvright;
	kern_return_t kr = 1;
	launch_data_t out_obj_array = NULL;
	struct ldcred *ldc = runtime_get_caller_creds();
	jobmgr_t jmr = NULL;

	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	if (job_mig_intran2(root_jobmgr, target_subset, ldc->pid)) {
		job_log(j, LOG_ERR, "Moving a session to ourself is bogus.");

		kr = BOOTSTRAP_NOT_PRIVILEGED;
		goto out;
	}

	job_log(j, LOG_DEBUG, "Move subset attempt: 0x%x", target_subset);

	kr = _vproc_grab_subset(target_subset, &reqport, &rcvright, &out_obj_array, &l2l_ports, &l2l_port_cnt);
	if (job_assumes_zero(j, kr) != 0) {
		goto out;
	}

	if (launch_data_array_get_count(out_obj_array) != l2l_port_cnt) {
		os_assert_zero(l2l_port_cnt);
	}

	if (!job_assumes(j, (jmr = jobmgr_new(j->mgr, reqport, rcvright, false, session_type, false, asport)) != NULL)) {
		kr = BOOTSTRAP_NO_MEMORY;
		goto out;
	}

	if (strcmp(session_type, VPROCMGR_SESSION_AQUA) == 0) {
		jobmgr_log(jmr, LOG_NOTICE, "Registering new GUI session.");
		kr = vproc_mig_register_gui_session(inherited_bootstrap_port, asport);
		if (kr) {
			jobmgr_log(jmr, LOG_ERR, "Failed to register GUI session with PID 1: 0x%x/0x%x", inherited_bootstrap_port, kr);
		}
	}

	jmr->properties |= BOOTSTRAP_PROPERTY_MOVEDSUBSET;

	/* This is a hack. We should be doing this in jobmgr_new(), but since we're in the middle of
	 * processing an IPC request, we'll do this action before the new job manager can get any IPC
	 * requests. This serialization is guaranteed since we are single-threaded in that respect.
	 */
	if (flags & LAUNCH_GLOBAL_ON_DEMAND) {
		// This is so awful.
		// Remove the job from its current job manager.
		LIST_REMOVE(j, sle);
		LIST_REMOVE(j, pid_hash_sle);

		// Put the job into the target job manager.
		LIST_INSERT_HEAD(&jmr->jobs, j, sle);
		LIST_INSERT_HEAD(&jmr->active_jobs[ACTIVE_JOB_HASH(j->p)], j, pid_hash_sle);

		j->mgr = jmr;
		job_set_global_on_demand(j, true);

		if (!j->holds_ref) {
			job_log(j, LOG_PERF, "Job moved subset into: %s", j->mgr->name);
			j->holds_ref = true;
			runtime_add_ref();
		}
	}

	for (l2l_i = 0; l2l_i < l2l_port_cnt; l2l_i++) {
		launch_data_t tmp, obj_at_idx;
		struct machservice *ms;
		job_t j_for_service;
		const char *serv_name;
		pid_t target_pid;
		bool serv_perpid;

		(void)job_assumes(j, obj_at_idx = launch_data_array_get_index(out_obj_array, l2l_i));
		(void)job_assumes(j, tmp = launch_data_dict_lookup(obj_at_idx, TAKE_SUBSET_PID));
		target_pid = (pid_t)launch_data_get_integer(tmp);
		(void)job_assumes(j, tmp = launch_data_dict_lookup(obj_at_idx, TAKE_SUBSET_PERPID));
		serv_perpid = launch_data_get_bool(tmp);
		(void)job_assumes(j, tmp = launch_data_dict_lookup(obj_at_idx, TAKE_SUBSET_NAME));
		serv_name = launch_data_get_string(tmp);

		j_for_service = jobmgr_find_by_pid(jmr, target_pid, true);

		if (unlikely(!j_for_service)) {
			// The PID probably exited
			(void)job_assumes_zero(j, launchd_mport_deallocate(l2l_ports[l2l_i]));
			continue;
		}

		if (likely(ms = machservice_new(j_for_service, serv_name, &l2l_ports[l2l_i], serv_perpid))) {
			job_log(j, LOG_DEBUG, "Importing %s into new bootstrap.", serv_name);
			machservice_request_notifications(ms);
		}
	}

	kr = 0;

out:
	if (out_obj_array) {
		launch_data_free(out_obj_array);
	}

	if (l2l_ports) {
		mig_deallocate((vm_address_t)l2l_ports, l2l_port_cnt * sizeof(l2l_ports[0]));
	}

	if (kr == 0) {
		if (target_subset) {
			(void)job_assumes_zero(j, launchd_mport_deallocate(target_subset));
		}
		if (asport) {
			(void)job_assumes_zero(j, launchd_mport_deallocate(asport));
		}
	} else if (jmr) {
		jobmgr_shutdown(jmr);
	}

	return kr;
}

kern_return_t
job_mig_init_session(job_t j, name_t session_type, mach_port_t asport)
{
	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	job_t j2;

	kern_return_t kr = BOOTSTRAP_NO_MEMORY;
	if (j->mgr->session_initialized) {
		job_log(j, LOG_ERR, "Tried to initialize an already setup session!");
		kr = BOOTSTRAP_NOT_PRIVILEGED;
		return kr;
	} else if (strcmp(session_type, VPROCMGR_SESSION_LOGINWINDOW) == 0) {
		jobmgr_t jmi;

		/*
		 * 5330262
		 *
		 * We're working around LoginWindow and the WindowServer.
		 *
		 * In practice, there is only one LoginWindow session. Unfortunately, for certain
		 * scenarios, the WindowServer spawns loginwindow, and in those cases, it frequently
		 * spawns a replacement loginwindow session before cleaning up the previous one.
		 *
		 * We're going to use the creation of a new LoginWindow context as a clue that the
		 * previous LoginWindow context is on the way out and therefore we should just
		 * kick-start the shutdown of it.
		 */

		SLIST_FOREACH(jmi, &root_jobmgr->submgrs, sle) {
			if (unlikely(jmi->shutting_down)) {
				continue;
			} else if (strcasecmp(jmi->name, session_type) == 0) {
				jobmgr_shutdown(jmi);
				break;
			}
		}
	} else if (strcmp(session_type, VPROCMGR_SESSION_AQUA) == 0) {
		(void)job_assumes_zero(j, runtime_remove_mport(j->mgr->jm_port));
 	}

	jobmgr_log(j->mgr, LOG_DEBUG, "Initializing as %s", session_type);
	strcpy(j->mgr->name_init, session_type);

	if (job_assumes(j, (j2 = jobmgr_init_session(j->mgr, session_type, false)))) {
		j2->asport = asport;
		(void)job_assumes(j, job_dispatch(j2, true));
		kr = BOOTSTRAP_SUCCESS;
	}

	return kr;
}

kern_return_t
job_mig_switch_to_session(job_t j, mach_port_t requestor_port, name_t session_name, mach_port_t asport, mach_port_t *new_bsport)
{
	struct ldcred *ldc = runtime_get_caller_creds();
	if (!jobmgr_assumes(root_jobmgr, j != NULL)) {
		jobmgr_log(root_jobmgr, LOG_ERR, "%s() called with NULL job: PID %d", __func__, ldc->pid);
		return BOOTSTRAP_NO_MEMORY;
	}

	if (j->mgr->shutting_down) {
		return BOOTSTRAP_UNKNOWN_SERVICE;
	}

	job_log(j, LOG_DEBUG, "Job wants to move to %s session.", session_name);

	if (!job_assumes(j, pid1_magic == false)) {
		job_log(j, LOG_WARNING, "Switching sessions is not allowed in the system Mach bootstrap.");
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

	if (!j->anonymous) {
		job_log(j, LOG_NOTICE, "Non-anonymous job tried to switch sessions. Please use LimitLoadToSessionType instead.");
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

	jobmgr_t target_jm = jobmgr_find_by_name(root_jobmgr, session_name);
	if (target_jm == j->mgr) {
		job_log(j, LOG_DEBUG, "Job is already in its desired session (%s).", session_name);
		(void)job_assumes_zero(j, launchd_mport_deallocate(asport));
		(void)job_assumes_zero(j, launchd_mport_deallocate(requestor_port));
		*new_bsport = target_jm->jm_port;
		return BOOTSTRAP_SUCCESS;
	}

	if (!target_jm) {
		target_jm = jobmgr_new(j->mgr, requestor_port, MACH_PORT_NULL, false, session_name, false, asport);
		if (target_jm) {
			target_jm->properties |= BOOTSTRAP_PROPERTY_IMPLICITSUBSET;
			(void)job_assumes_zero(j, launchd_mport_deallocate(asport));
		}
	}

	if (!job_assumes(j, target_jm != NULL)) {
		job_log(j, LOG_WARNING, "Could not find %s session!", session_name);
		return BOOTSTRAP_NO_MEMORY;
	}

	// Remove the job from it's current job manager.
	LIST_REMOVE(j, sle);
	LIST_REMOVE(j, pid_hash_sle);

	job_t ji = NULL, jit = NULL;
	LIST_FOREACH_SAFE(ji, &j->mgr->global_env_jobs, global_env_sle, jit) {
		if (ji == j) {
			LIST_REMOVE(ji, global_env_sle);
			break;
		}
	}

	// Put the job into the target job manager.
	LIST_INSERT_HEAD(&target_jm->jobs, j, sle);
	LIST_INSERT_HEAD(&target_jm->active_jobs[ACTIVE_JOB_HASH(j->p)], j, pid_hash_sle);

	if (ji) {
		LIST_INSERT_HEAD(&target_jm->global_env_jobs, j, global_env_sle);
	}

	// Move our Mach services over if we're not in a flat namespace.
	if (!launchd_flat_mach_namespace && !SLIST_EMPTY(&j->machservices)) {
		struct machservice *msi = NULL, *msit = NULL;
		SLIST_FOREACH_SAFE(msi, &j->machservices, sle, msit) {
			LIST_REMOVE(msi, name_hash_sle);
			LIST_INSERT_HEAD(&target_jm->ms_hash[hash_ms(msi->name)], msi, name_hash_sle);
		}
	}

	j->mgr = target_jm;

	if (!j->holds_ref) {
		/* Anonymous jobs which move around are particularly interesting to us, so we want to
		 * stick around while they're still around.
		 * For example, login calls into the PAM launchd module, which moves the process into
		 * the StandardIO session by default. So we'll hold a reference on that job to prevent
		 * ourselves from going away.
		 */
		j->holds_ref = true;
		job_log(j, LOG_PERF, "Job switched into manager: %s", j->mgr->name);
		runtime_add_ref();
	}

	*new_bsport = target_jm->jm_port;

	return KERN_SUCCESS;
}

kern_return_t
job_mig_take_subset(job_t j, mach_port_t *reqport, mach_port_t *rcvright,
		vm_offset_t *outdata, mach_msg_type_number_t *outdataCnt,
		mach_port_array_t *portsp, unsigned int *ports_cnt)
{
	launch_data_t tmp_obj, tmp_dict, outdata_obj_array = NULL;
	mach_port_array_t ports = NULL;
	unsigned int cnt = 0, cnt2 = 0;
	size_t packed_size;
	struct machservice *ms;
	jobmgr_t jm;
	job_t ji;

	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	jm = j->mgr;

	if (unlikely(!pid1_magic)) {
		job_log(j, LOG_ERR, "Only the system launchd will transfer Mach sub-bootstraps.");
		return BOOTSTRAP_NOT_PRIVILEGED;
	}
	if (unlikely(jobmgr_parent(jm) == NULL)) {
		job_log(j, LOG_ERR, "Root Mach bootstrap cannot be transferred.");
		return BOOTSTRAP_NOT_PRIVILEGED;
	}
	if (unlikely(strcasecmp(jm->name, VPROCMGR_SESSION_AQUA) == 0)) {
		job_log(j, LOG_ERR, "Cannot transfer a setup GUI session.");
		return BOOTSTRAP_NOT_PRIVILEGED;
	}
	if (unlikely(!j->anonymous)) {
		job_log(j, LOG_ERR, "Only the anonymous job can transfer Mach sub-bootstraps.");
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

	job_log(j, LOG_DEBUG, "Transferring sub-bootstrap to the per session launchd.");

	outdata_obj_array = launch_data_alloc(LAUNCH_DATA_ARRAY);
	if (!job_assumes(j, outdata_obj_array)) {
		goto out_bad;
	}

	*outdataCnt = 20 * 1024 * 1024;
	mig_allocate(outdata, *outdataCnt);
	if (!job_assumes(j, *outdata != 0)) {
		return 1;
	}

	LIST_FOREACH(ji, &j->mgr->jobs, sle) {
		if (!ji->anonymous) {
			continue;
		}
		SLIST_FOREACH(ms, &ji->machservices, sle) {
			cnt++;
		}
	}

	mig_allocate((vm_address_t *)&ports, cnt * sizeof(ports[0]));
	if (!job_assumes(j, ports != NULL)) {
		goto out_bad;
	}

	LIST_FOREACH(ji, &j->mgr->jobs, sle) {
		if (!ji->anonymous) {
			continue;
		}

		SLIST_FOREACH(ms, &ji->machservices, sle) {
			if (job_assumes(j, (tmp_dict = launch_data_alloc(LAUNCH_DATA_DICTIONARY)))) {
				(void)job_assumes(j, launch_data_array_set_index(outdata_obj_array, tmp_dict, cnt2));
			} else {
				goto out_bad;
			}

			if (job_assumes(j, (tmp_obj = launch_data_new_string(machservice_name(ms))))) {
				(void)job_assumes(j, launch_data_dict_insert(tmp_dict, tmp_obj, TAKE_SUBSET_NAME));
			} else {
				goto out_bad;
			}

			if (job_assumes(j, (tmp_obj = launch_data_new_integer((ms->job->p))))) {
				(void)job_assumes(j, launch_data_dict_insert(tmp_dict, tmp_obj, TAKE_SUBSET_PID));
			} else {
				goto out_bad;
			}

			if (job_assumes(j, (tmp_obj = launch_data_new_bool((ms->per_pid))))) {
				(void)job_assumes(j, launch_data_dict_insert(tmp_dict, tmp_obj, TAKE_SUBSET_PERPID));
			} else {
				goto out_bad;
			}

			ports[cnt2] = machservice_port(ms);

			// Increment the send right by one so we can shutdown the jobmgr cleanly
			(void)jobmgr_assumes_zero(jm, launchd_mport_copy_send(ports[cnt2]));
			cnt2++;
		}
	}

	(void)job_assumes(j, cnt == cnt2);

	runtime_ktrace0(RTKT_LAUNCHD_DATA_PACK);
	packed_size = launch_data_pack(outdata_obj_array, (void *)*outdata, *outdataCnt, NULL, NULL);
	if (!job_assumes(j, packed_size != 0)) {
		goto out_bad;
	}

	launch_data_free(outdata_obj_array);

	*portsp = ports;
	*ports_cnt = cnt;

	*reqport = jm->req_port;
	*rcvright = jm->jm_port;

	jm->req_port = 0;
	jm->jm_port = 0;

	workaround_5477111 = j;

	jobmgr_shutdown(jm);

	return BOOTSTRAP_SUCCESS;

out_bad:
	if (outdata_obj_array) {
		launch_data_free(outdata_obj_array);
	}
	if (*outdata) {
		mig_deallocate(*outdata, *outdataCnt);
	}
	if (ports) {
		mig_deallocate((vm_address_t)ports, cnt * sizeof(ports[0]));
	}

	return BOOTSTRAP_NO_MEMORY;
}

kern_return_t
job_mig_subset(job_t j, mach_port_t requestorport, mach_port_t *subsetportp)
{
	int bsdepth = 0;
	jobmgr_t jmr;

	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}
	if (j->mgr->shutting_down) {
		return BOOTSTRAP_UNKNOWN_SERVICE;
	}

	jmr = j->mgr;

	while ((jmr = jobmgr_parent(jmr)) != NULL) {
		bsdepth++;
	}

	// Since we use recursion, we need an artificial depth for subsets
	if (unlikely(bsdepth > 100)) {
		job_log(j, LOG_ERR, "Mach sub-bootstrap create request failed. Depth greater than: %d", bsdepth);
		return BOOTSTRAP_NO_MEMORY;
	}

	char name[NAME_MAX];
	snprintf(name, sizeof(name), "%s[%i].subset.%i", j->anonymous ? j->prog : j->label, j->p, MACH_PORT_INDEX(requestorport));

	if (!job_assumes(j, (jmr = jobmgr_new(j->mgr, requestorport, MACH_PORT_NULL, false, name, true, j->asport)) != NULL)) {
		if (unlikely(requestorport == MACH_PORT_NULL)) {
			return BOOTSTRAP_NOT_PRIVILEGED;
		}
		return BOOTSTRAP_NO_MEMORY;
	}

	*subsetportp = jmr->jm_port;
	jmr->properties |= BOOTSTRAP_PROPERTY_EXPLICITSUBSET;

	/* A job could create multiple subsets, so only add a reference the first time
	 * it does so we don't have to keep a count.
	 */
	if (j->anonymous && !j->holds_ref) {
		job_log(j, LOG_PERF, "Job created subset: %s", jmr->name);
		j->holds_ref = true;
		runtime_add_ref();
	}

	job_log(j, LOG_DEBUG, "Job created a subset named \"%s\"", jmr->name);
	return BOOTSTRAP_SUCCESS;
}

job_t
_xpc_domain_import_service(jobmgr_t jm, launch_data_t pload)
{
	jobmgr_t where2put = NULL;

	if (launch_data_get_type(pload) != LAUNCH_DATA_DICTIONARY) {
		errno = EINVAL;
		return NULL;
	}

	launch_data_t ldlabel = launch_data_dict_lookup(pload, LAUNCH_JOBKEY_LABEL);
	if (!ldlabel || launch_data_get_type(ldlabel) != LAUNCH_DATA_STRING) {
		errno = EINVAL;
		return NULL;
	}

	const char *label = launch_data_get_string(ldlabel);
	jobmgr_log(jm, LOG_DEBUG, "Importing service: %s", label);

	launch_data_t destname = launch_data_dict_lookup(pload, LAUNCH_JOBKEY_XPCDOMAIN);
	if (destname) {
		bool supported_domain = false;

		if (launch_data_get_type(destname) == LAUNCH_DATA_STRING) {
			const char *str = launch_data_get_string(destname);
			if (strcmp(str, XPC_DOMAIN_TYPE_SYSTEM) == 0) {
				where2put = _s_xpc_system_domain;
			} else if (strcmp(str, XPC_DOMAIN_TYPE_PERUSER) == 0) {
				where2put = jobmgr_find_xpc_per_user_domain(jm, jm->req_euid);
				supported_domain = true;
			} else if (strcmp(str, XPC_DOMAIN_TYPE_PERSESSION) == 0) {
				where2put = jobmgr_find_xpc_per_session_domain(jm, jm->req_asid);
			} else {
				jobmgr_log(jm, LOG_ERR, "Invalid XPC domain type: %s", str);
				errno = EINVAL;
			}
		} else {
			jobmgr_log(jm, LOG_ERR, "XPC domain type is not a string.");
			errno = EINVAL;
		}

		if (where2put && !supported_domain) {
			launch_data_t mi = NULL;
			if ((mi = launch_data_dict_lookup(pload, LAUNCH_JOBKEY_MULTIPLEINSTANCES))) {
				if (launch_data_get_type(mi) == LAUNCH_DATA_BOOL && launch_data_get_bool(mi)) {
					jobmgr_log(where2put, LOG_ERR, "Multiple-instance services are not supported in this domain.");
					where2put = NULL;
					errno = EINVAL;
				}
			}
		}
	} else {
		where2put = jm;
	}

	job_t j = NULL;
	if (where2put) {
		/* Gross. If the service already exists in a singleton domain, then
		 * jobmgr_import2() will return the existing job. But if we fail to alias
		 * this job, we will normally want to remove it. But if we did not create
		 * it in the first place, then we need to avoid removing it. So check
		 * errno against EEXIST in the success case and if it's EEXIST, then do
		 * not remove the original job in the event of a failed alias.
		 *
		 * This really needs to be re-thought, but I think it'll require a larger
		 * evaluation of launchd's data structures. Right now, once a job is
		 * imported into a singleton domain, it won't be removed until the system
		 * shuts down, but that may not always be true. If it ever changes, we'll
		 * have a problem because we'll have to account for all existing aliases
		 * and clean them up somehow. Or just start ref-counting. I knew this
		 * aliasing stuff would be trouble...
		 *
		 * <rdar://problem/10646503>
		 */
		jobmgr_log(where2put, LOG_DEBUG, "Importing service...");

		errno = 0;
		if ((j = jobmgr_import2(where2put, pload))) {
			bool created = (errno != EEXIST);
			j->xpc_service = true;

			if (where2put->xpc_singleton) {
				/* If the service was destined for one of the global domains,
				 * then we have to alias it into our local domain to reserve the
				 * name.
				 */
				job_t ja = NULL;
				if (!(ja = job_new_alias(jm, j))) {
					/* If we failed to alias the job because of a conflict over
					 * the label, then we remove it from the global domain. We
					 * don't want to risk having imported a malicious job into
					 * one of the global domains.
					 */
					if (errno != EEXIST) {
						job_log(j, LOG_ERR, "Failed to alias job into: %s: %d: %s", where2put->name, errno, strerror(errno));
					} else {
						errno = 0;
					}

					if (created) {
						jobmgr_log(jm, LOG_WARNING, "Singleton service already existed in job-local namespace. Removing: %s", j->label);
						job_remove(j);
					}

					j = NULL;
				} else {
					jobmgr_log(jm, LOG_DEBUG, "Aliased service into local domain: %s", j->label);
					(void)job_dispatch(j, false);
					ja->xpc_service = true;
					j = ja;
				}
			} else {
				(void)job_dispatch(j, false);
			}
		}
	} else {
		jobmgr_log(jm, LOG_DEBUG, "Could not find destination for service: %s", label);
	}

	return j;
}

int
_xpc_domain_import_services(job_t j, launch_data_t services)
{
	int error = EINVAL;
	if (launch_data_get_type(services) != LAUNCH_DATA_ARRAY) {
		return error;
	}

	size_t i = 0;
	size_t c = launch_data_array_get_count(services);
	jobmgr_log(j->mgr, LOG_DEBUG, "Importing new services: %lu", c);

	for (i = 0; i < c; i++) {
		jobmgr_log(j->mgr, LOG_DEBUG, "Importing service at index: %lu", i);

		job_t nj = NULL;
		launch_data_t ploadi = launch_data_array_get_index(services, i);
		if (!(nj = _xpc_domain_import_service(j->mgr, ploadi))) {
			if (!j->mgr->session_initialized && errno) {
				/* Service import failures are only fatal if the domain is being
				 * initialized. If we're extending the domain, we can run into
				 * errors with services already existing, so we just ignore them.
				 * In the case of a domain extension, we don't want to halt the
				 * operation if we run into an error with one service.
				 *
				 * <rdar://problem/10842779>
				 */
				jobmgr_log(j->mgr, LOG_ERR, "Failed to import service at index: %lu: %d: %s", i, errno, strerror(errno));
				error = errno;
				break;
			}
		} else {
			jobmgr_log(j->mgr, LOG_DEBUG, "Imported service: %s", nj->label);
		}
	}

	if (i == c) {
		error = 0;
	}

	return error;
}

kern_return_t
xpc_domain_import2(job_t j, mach_port_t reqport, mach_port_t dport)
{
	if (unlikely(!pid1_magic)) {
		job_log(j, LOG_ERR, "XPC domains may only reside in PID 1.");
		return BOOTSTRAP_NOT_PRIVILEGED;
	}
	if (!j || !MACH_PORT_VALID(reqport)) {
		return BOOTSTRAP_UNKNOWN_SERVICE;
	}
	if (root_jobmgr->shutting_down) {
		jobmgr_log(root_jobmgr, LOG_ERR, "Attempt to create new domain while shutting down.");
		return BOOTSTRAP_NOT_PRIVILEGED;
	}
	if (!j->xpc_bootstrapper) {
		job_log(j, LOG_ERR, "Attempt to create new XPC domain by unprivileged job.");
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

	kern_return_t kr = BOOTSTRAP_NO_MEMORY;
	/* All XPC domains are children of the root job manager. What we're creating
	 * here is really just a skeleton. By creating it, we're adding reqp to our
	 * port set. It will have two messages on it. The first specifies the
	 * environment of the originator. This is so we can cache it and hand it to
	 * xpcproxy to bootstrap our services. The second is the set of jobs that is
	 * to be bootstrapped in.
	 */
	jobmgr_t jm = jobmgr_new(root_jobmgr, reqport, dport, false, NULL, true, MACH_PORT_NULL);
	if (job_assumes(j, jm != NULL)) {
		jm->properties |= BOOTSTRAP_PROPERTY_XPC_DOMAIN;
		jm->shortdesc = "private";
		kr = BOOTSTRAP_SUCCESS;
	}

	return kr;
}

kern_return_t
xpc_domain_set_environment(job_t j, mach_port_t rp, mach_port_t bsport, mach_port_t excport, vm_offset_t ctx, mach_msg_type_number_t ctx_sz)
{
	if (!j) {
		/* Due to the whacky nature of XPC service bootstrapping, we can end up
		 * getting this message long after the requesting process has gone away.
		 * See <rdar://problem/8593143>.
		 */
		return BOOTSTRAP_UNKNOWN_SERVICE;
	}

	jobmgr_t jm = j->mgr;
	if (!(jm->properties & BOOTSTRAP_PROPERTY_XPC_DOMAIN)) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

	if (jm->req_asport != MACH_PORT_NULL) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

	struct ldcred *ldc = runtime_get_caller_creds();
	struct proc_bsdinfowithuniqid proc;
	if (proc_pidinfo(ldc->pid, PROC_PIDT_BSDINFOWITHUNIQID, 1, &proc, PROC_PIDT_BSDINFOWITHUNIQID_SIZE) == 0) {
		if (errno != ESRCH) {
			(void)jobmgr_assumes_zero(jm, errno);
		}

		jm->error = errno;
		jobmgr_remove(jm);
		return BOOTSTRAP_NO_MEMORY;
	}

#if !TARGET_OS_EMBEDDED
	if (jobmgr_assumes_zero(jm, audit_session_port(ldc->asid, &jm->req_asport)) != 0) {
		jm->error = EPERM;
		jobmgr_remove(jm);
		job_log(j, LOG_ERR, "Failed to get port for ASID: %u", ldc->asid);
		return BOOTSTRAP_NOT_PRIVILEGED;
	}
#else
	jm->req_asport = MACH_PORT_DEAD;
#endif

	struct waiting4attach *w4ai = NULL;
	struct waiting4attach *w4ait = NULL;
	LIST_FOREACH_SAFE(w4ai, &_launchd_domain_waiters, le, w4ait) {
		if (w4ai->dest == ldc->pid) {
			jobmgr_log(jm, LOG_DEBUG, "Migrating attach for: %s", w4ai->name);
			LIST_REMOVE(w4ai, le);
			LIST_INSERT_HEAD(&jm->attaches, w4ai, le);
			w4ai->dest = 0;
		}
	}

	(void)snprintf(jm->name_init, NAME_MAX, "com.apple.xpc.domain.%s.%d", proc.pbsd.pbi_comm, ldc->pid);
	strlcpy(jm->owner, proc.pbsd.pbi_comm, sizeof(jm->owner));
	jm->req_bsport = bsport;
	jm->req_excport = excport;
	jm->req_rport = rp;
	jm->req_ctx = ctx;
	jm->req_ctx_sz = ctx_sz;
	jm->req_pid = ldc->pid;
	jm->req_euid = ldc->euid;
	jm->req_egid = ldc->egid;
	jm->req_asid = ldc->asid;
	jm->req_uniqueid = proc.p_uniqidentifier.p_uniqueid;

	return KERN_SUCCESS;
}

kern_return_t
xpc_domain_load_services(job_t j, vm_offset_t services_buff, mach_msg_type_number_t services_sz)
{
	if (!j) {
		return BOOTSTRAP_UNKNOWN_SERVICE;
	}

 	job_t rootj = jobmgr_find_by_pid(root_jobmgr, j->p, false);
 	if (!(rootj && rootj->xpc_bootstrapper)) {
		job_log(j, LOG_ERR, "Attempt to load services into XPC domain by unprivileged job.");
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

	// This is just for XPC domains (for now).
	if (!(j->mgr->properties & BOOTSTRAP_PROPERTY_XPC_DOMAIN)) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}
	if (j->mgr->session_initialized) {
		jobmgr_log(j->mgr, LOG_ERR, "Attempt to initialize an already-initialized XPC domain.");
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

	size_t offset = 0;
	launch_data_t services = launch_data_unpack((void *)services_buff, services_sz, NULL, 0, &offset, NULL);
	if (!services) {
		return BOOTSTRAP_NO_MEMORY;
	}

	int error = _xpc_domain_import_services(j, services);
	if (error) {
		j->mgr->error = error;
		jobmgr_log(j->mgr, LOG_ERR, "Obliterating domain.");
		jobmgr_remove(j->mgr);
	} else {
		j->mgr->session_initialized = true;
		(void)jobmgr_assumes_zero(j->mgr, xpc_call_wakeup(j->mgr->req_rport, BOOTSTRAP_SUCCESS));
		j->mgr->req_rport = MACH_PORT_NULL;

		/* Returning a failure code will destroy the message, whereas returning
		 * success will not, so we need to clean up here.
		 */
		mig_deallocate(services_buff, services_sz);
		error = BOOTSTRAP_SUCCESS;
	}

	return error;
}

kern_return_t
xpc_domain_check_in(job_t j, mach_port_t *bsport, mach_port_t *sbsport, 
	mach_port_t *excport, mach_port_t *asport, uint32_t *uid, uint32_t *gid,
	int32_t *asid, vm_offset_t *ctx, mach_msg_type_number_t *ctx_sz)
{
	if (!jobmgr_assumes(root_jobmgr, j != NULL)) {
		return BOOTSTRAP_UNKNOWN_SERVICE;
	}
	jobmgr_t jm = j->mgr;
	if (!(jm->properties & BOOTSTRAP_PROPERTY_XPC_DOMAIN)) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

	if (jm->req_asport == MACH_PORT_NULL) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

	*bsport = jm->req_bsport;
	*sbsport = root_jobmgr->jm_port;
	*excport = jm->req_excport;
	if (j->joins_gui_session) {
		if (jm->req_gui_asport) {
			*asport = jm->req_gui_asport;
		} else {
			job_log(j, LOG_NOTICE, "No GUI session set for UID of user service. This service may not act properly.");
			*asport = jm->req_asport;
		}
	} else {
		*asport = jm->req_asport;
	}

	*uid = jm->req_euid;
	*gid = jm->req_egid;
	*asid = jm->req_asid;

	*ctx = jm->req_ctx;
	*ctx_sz = jm->req_ctx_sz;

	return KERN_SUCCESS;
}

kern_return_t
xpc_domain_get_service_name(job_t j, event_name_t name)
{
	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	if (!j->xpc_service) {
		jobmgr_log(j->mgr, LOG_ERR, "Attempt to get service name by non-XPC service: %s", j->label);
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

	const char *what2find = j->label;
	if (j->dedicated_instance) {
		what2find = j->original->label;
	}

	struct machservice *msi = NULL;
	SLIST_FOREACH(msi, &j->machservices, sle) {
		if (strcmp(msi->name, what2find) == 0) {
			break;
		}
	}

	if (!msi) {
		jobmgr_log(j->mgr, LOG_ERR, "Attempt to get service name that does not exist: %s", j->label);
		return BOOTSTRAP_UNKNOWN_SERVICE;
	}

	(void)strlcpy(name, msi->name, sizeof(event_name_t));
	return BOOTSTRAP_SUCCESS;
}

#if XPC_LPI_VERSION >= 20111216
kern_return_t
xpc_domain_add_services(job_t j, vm_offset_t services_buff, mach_msg_type_number_t services_sz)
{
	if (!j) {
		return BOOTSTRAP_UNKNOWN_SERVICE;
	}

 	job_t rootj = jobmgr_find_by_pid(root_jobmgr, j->p, false);
 	if (!(rootj && rootj->xpc_bootstrapper)) {
		job_log(j, LOG_ERR, "Attempt to add service to XPC domain by unprivileged job.");
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

	if (!(j->mgr->properties & BOOTSTRAP_PROPERTY_XPC_DOMAIN)) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

	size_t offset = 0;
	launch_data_t services = launch_data_unpack((void *)services_buff, services_sz, NULL, 0, &offset, NULL);
	if (!services) {
		return BOOTSTRAP_NO_MEMORY;
	}

	int error = _xpc_domain_import_services(j, services);
	if (!error) {
		mig_deallocate(services_buff, services_sz);
	}

	return error;
}
#endif

#pragma mark XPC Events
int
xpc_event_find_channel(job_t j, const char *stream, struct machservice **ms)
{
	int error = EXNOMEM;
	struct machservice *msi = NULL;
	SLIST_FOREACH(msi, &j->machservices, sle) {
		if (strcmp(stream, msi->name) == 0) {
			break;
		}
	}

	if (!msi) {
		mach_port_t sp = MACH_PORT_NULL;
		msi = machservice_new(j, stream, &sp, false);
		if (!msi) {
			return EXNOMEM;
		}

		job_log(j, LOG_DEBUG, "Creating new MachService for stream: %s", stream);
		/* Hack to keep this from being publicly accessible through
		 * bootstrap_look_up().
		 */
		if (!j->dedicated_instance) {
			LIST_REMOVE(msi, name_hash_sle);
		}
		msi->event_channel = true;

		/* If we call job_dispatch() here before the audit session for the job
		 * has been set, we'll end up not watching this service. But we also have
		 * to take care not to watch the port if the job is active.
		 *
		 * See <rdar://problem/10357855>.
		 */
		if (!j->currently_ignored) {
			machservice_watch(j, msi);
		}

		error = 0;
		*ms = msi;
	} else if (!msi->event_channel) {
		job_log(j, LOG_ERR, "This job registered a MachService name identical to the requested event channel name: %s", stream);
		error = EEXIST;
	} else {
		error = 0;
		*ms = msi;
	}

	return error;
}

int
xpc_event_get_event_name(job_t j, xpc_object_t request, xpc_object_t *reply)
{
	const char *stream = xpc_dictionary_get_string(request, XPC_EVENT_ROUTINE_KEY_STREAM);
	if (!stream) {
		return EXINVAL;
	}

	uint64_t token = xpc_dictionary_get_uint64(request, XPC_EVENT_ROUTINE_KEY_TOKEN);
	if (!token) {
		return EXINVAL;
	}

	job_log(j, LOG_DEBUG, "Getting event name for stream/token: %s/0x%llu", stream, token);

	int result = ESRCH;
	struct externalevent *event = externalevent_find(stream, token);
	if (event && j->event_monitor) {
		xpc_object_t reply2 = xpc_dictionary_create_reply(request);
		xpc_dictionary_set_string(reply2, XPC_EVENT_ROUTINE_KEY_NAME, event->name);
		*reply = reply2;

		job_log(j, LOG_DEBUG, "Found: %s", event->name);
		result = 0;
	}

	return result;
}

int
xpc_event_copy_entitlements(job_t j, xpc_object_t request, xpc_object_t *reply)
{
	const char *stream = xpc_dictionary_get_string(request, XPC_EVENT_ROUTINE_KEY_STREAM);
	if (!stream) {
		return EXINVAL;
	}

	uint64_t token = xpc_dictionary_get_uint64(request, XPC_EVENT_ROUTINE_KEY_TOKEN);
	if (!token) {
		return EXINVAL;
	}

	job_log(j, LOG_DEBUG, "Getting entitlements for stream/token: %s/0x%llu", stream, token);

	int result = ESRCH;
	struct externalevent *event = externalevent_find(stream, token);
	if (event && j->event_monitor) {
		xpc_object_t reply2 = xpc_dictionary_create_reply(request);
		xpc_dictionary_set_value(reply2, XPC_EVENT_ROUTINE_KEY_ENTITLEMENTS, event->entitlements);
		*reply = reply2;

		job_log(j, LOG_DEBUG, "Found: %s", event->name);
		result = 0;
	}

	return result;
}

// TODO - can be removed with rdar://problem/12666150
#ifndef XPC_EVENT_FLAG_ALLOW_UNMANAGED
#define XPC_EVENT_FLAG_ALLOW_UNMANAGED (1 << 1)
#endif
	
int
xpc_event_set_event(job_t j, xpc_object_t request, xpc_object_t *reply)
{
	const char *stream = xpc_dictionary_get_string(request, XPC_EVENT_ROUTINE_KEY_STREAM);
	if (!stream) {
		return EXINVAL;
	}

	const char *key = xpc_dictionary_get_string(request, XPC_EVENT_ROUTINE_KEY_NAME);
	if (!key) {
		return EXINVAL;
	}

	xpc_object_t event = xpc_dictionary_get_value(request, XPC_EVENT_ROUTINE_KEY_EVENT);
	if (event && xpc_get_type(event) != XPC_TYPE_DICTIONARY) {
		return EXINVAL;
	}

	uint64_t flags = xpc_dictionary_get_uint64(request, XPC_EVENT_ROUTINE_KEY_FLAGS);

	/* Don't allow events to be set for anonymous jobs unless specifically
	 * requested in the flags. Only permit this for internal development.
	 */
	if (j->anonymous && ((flags & XPC_EVENT_FLAG_ALLOW_UNMANAGED) == 0 || !launchd_apple_internal)) {
		job_log(j, LOG_ERR, "Unmanaged jobs may not make XPC Events requests.");
		return EPERM;
	}

	job_log(j, LOG_DEBUG, "%s event for stream/key: %s/%s", event ? "Setting" : "Removing", stream, key);

	struct externalevent *eei = NULL;
	LIST_FOREACH(eei, &j->events, job_le) {
		/* If the event for the given key already exists for the job, we need to
		 * remove the old one first.
		 */
		if (strcmp(eei->name, key) == 0 && strcmp(eei->sys->name, stream) == 0) {
			job_log(j, LOG_DEBUG, "Event exists. Removing.");
			externalevent_delete(eei);
			break;
		}
	}

	int result = EXNOMEM;
	if (event) {
		struct eventsystem *es = eventsystem_find(stream);
		if (!es) {
			job_log(j, LOG_DEBUG, "Creating stream.");
			es = eventsystem_new(stream);
		}

		if (es) {
			job_log(j, LOG_DEBUG, "Adding event.");
			if (externalevent_new(j, es, key, event, flags)) {
				job_log(j, LOG_DEBUG, "Added new event for key: %s", key);
				result = 0;
			} else {
				job_log(j, LOG_ERR, "Could not create event for key: %s", key);
			}
		} else {
			job_log(j, LOG_ERR, "Event stream could not be created: %s", stream);
		}
	} else {
		/* If the event was NULL, then we just remove it and return. */
		result = 0;
	}

	if (result == 0) {
		xpc_object_t reply2 = xpc_dictionary_create_reply(request);
		*reply = reply2;
	}

	return result;
}

int
xpc_event_copy_event(job_t j, xpc_object_t request, xpc_object_t *reply)
{
	const char *stream = xpc_dictionary_get_string(request, XPC_EVENT_ROUTINE_KEY_STREAM);
	const char *key = xpc_dictionary_get_string(request, XPC_EVENT_ROUTINE_KEY_NAME);

	bool all_streams = (stream == NULL);
	bool all_events = (key == NULL || strcmp(key, "") == 0); // strcmp for libxpc compatibility
	xpc_object_t events = NULL;

	if (all_streams && !all_events) {
		return EXINVAL;
	}

	if (all_streams || all_events) {
		job_log(j, LOG_DEBUG, "Fetching all events%s%s", stream ? " for stream: " : "", stream ? stream : "");
		events = xpc_dictionary_create(NULL, NULL, 0);
	} else {
		job_log(j, LOG_DEBUG, "Fetching stream/key: %s/%s", stream, key);
	}

	int result = ESRCH;
	struct externalevent *eei = NULL;
	LIST_FOREACH(eei, &j->events, job_le) {
		if (all_streams) {
			xpc_object_t sub = xpc_dictionary_get_value(events, eei->sys->name);
			if (sub == NULL) {
				sub = xpc_dictionary_create(NULL, NULL, 0);
				xpc_dictionary_set_value(events, eei->sys->name, sub);
				xpc_release(sub);
			}
			xpc_dictionary_set_value(sub, eei->name, eei->event);
		} else if (strcmp(eei->sys->name, stream) == 0) {
			if (all_events) {
				xpc_dictionary_set_value(events, eei->name, eei->event);
			} else if (strcmp(eei->name, key) == 0) {
				job_log(j, LOG_DEBUG, "Found event.");
				events = xpc_retain(eei->event);
				break;
			}
		}
	}

	if (events) {
		xpc_object_t reply2 = xpc_dictionary_create_reply(request);
		xpc_dictionary_set_value(reply2, XPC_EVENT_ROUTINE_KEY_EVENT, events);
		xpc_release(events);

		*reply = reply2;
		result = 0;
	}

	return result;
}

int
xpc_event_channel_check_in(job_t j, xpc_object_t request, xpc_object_t *reply)
{
	const char *stream = xpc_dictionary_get_string(request, XPC_EVENT_ROUTINE_KEY_STREAM);
	if (!stream) {
		return EXINVAL;
	}

	job_log(j, LOG_DEBUG, "Checking in stream: %s", stream);

	struct machservice *ms = NULL;
	int error = xpc_event_find_channel(j, stream, &ms);
	if (error) {
		job_log(j, LOG_ERR, "Failed to check in: 0x%x: %s", error, xpc_strerror(error));
	} else if (ms->isActive) {
		job_log(j, LOG_ERR, "Attempt to check in on event channel multiple times: %s", stream);
		error = EBUSY;
	} else {
		machservice_request_notifications(ms);

		xpc_object_t reply2 = xpc_dictionary_create_reply(request);
		xpc_dictionary_set_mach_recv(reply2, XPC_EVENT_ROUTINE_KEY_PORT, ms->port);
		*reply = reply2;
		error = 0;
	}

	return error;
}

int
xpc_event_channel_look_up(job_t j, xpc_object_t request, xpc_object_t *reply)
{
	if (!j->event_monitor) {
		return EPERM;
	}

	const char *stream = xpc_dictionary_get_string(request, XPC_EVENT_ROUTINE_KEY_STREAM);
	if (!stream) {
		return EXINVAL;
	}

	uint64_t token = xpc_dictionary_get_uint64(request, XPC_EVENT_ROUTINE_KEY_TOKEN);
	if (!token) {
		return EXINVAL;
	}

	job_log(j, LOG_DEBUG, "Looking up channel for stream/token: %s/%llu", stream, token);

	struct externalevent *ee = externalevent_find(stream, token);
	if (!ee) {
		return ESRCH;
	}

	struct machservice *ms = NULL;
	int error = xpc_event_find_channel(ee->job, stream, &ms);
	if (!error) {
		job_log(j, LOG_DEBUG, "Found event channel port: 0x%x", ms->port);
		xpc_object_t reply2 = xpc_dictionary_create_reply(request);
		xpc_dictionary_set_mach_send(reply2, XPC_EVENT_ROUTINE_KEY_PORT, ms->port);
		*reply = reply2;
		error = 0;
	} else {
		job_log(j, LOG_ERR, "Could not find event channel for stream/token: %s/%llu: 0x%x: %s", stream, token, error, xpc_strerror(error));
	}

	return error;
}

int
xpc_event_provider_check_in(job_t j, xpc_object_t request, xpc_object_t *reply)
{
	if (!j->event_monitor) {
		return EPERM;
	}

	/* This indicates that the event monitor is now safe to signal. This state
	 * is independent of whether this operation actually succeeds; we just need
	 * it to ignore SIGUSR1.
	 */
	j->event_monitor_ready2signal = true;

	const char *stream = xpc_dictionary_get_string(request, XPC_EVENT_ROUTINE_KEY_STREAM);
	if (!stream) {
		return EXINVAL;
	}

	job_log(j, LOG_DEBUG, "Provider checking in for stream: %s", stream);

	xpc_object_t events = xpc_array_create(NULL, 0);
	struct eventsystem *es = eventsystem_find(stream);
	if (!es) {
		/* If we had to create the event stream, there were no events, so just
		 * give back the empty array.
		 */
		job_log(j, LOG_DEBUG, "Creating event stream.");
		es = eventsystem_new(stream);
		if (!job_assumes(j, es)) {
			xpc_release(events);
			return EXNOMEM;
		}

		if (strcmp(stream, "com.apple.launchd.helper") == 0) {
			_launchd_support_system = es;
		}
	} else {
		job_log(j, LOG_DEBUG, "Filling event array.");

		struct externalevent *ei = NULL;
		LIST_FOREACH(ei, &es->events, sys_le) {
			xpc_array_set_uint64(events, XPC_ARRAY_APPEND, ei->id);
			xpc_array_append_value(events, ei->event);
		}
	}

	xpc_object_t reply2 = xpc_dictionary_create_reply(request);
	xpc_dictionary_set_value(reply2, XPC_EVENT_ROUTINE_KEY_EVENTS, events);
	xpc_release(events);
	*reply = reply2;

	return 0;
}

int
xpc_event_provider_set_state(job_t j, xpc_object_t request, xpc_object_t *reply)
{
	job_t other_j = NULL;

	if (!j->event_monitor) {
		return EPERM;
	}

	const char *stream = xpc_dictionary_get_string(request, XPC_EVENT_ROUTINE_KEY_STREAM);
	if (!stream) {
		return EXINVAL;
	}

	uint64_t token = xpc_dictionary_get_uint64(request, XPC_EVENT_ROUTINE_KEY_TOKEN);
	if (!token) {
		return EXINVAL;
	}

	bool state = false;
	xpc_object_t xstate = xpc_dictionary_get_value(request, XPC_EVENT_ROUTINE_KEY_STATE);
	if (!xstate || xpc_get_type(xstate) != XPC_TYPE_BOOL) {
		return EXINVAL;
	} else {
		state = xpc_bool_get_value(xstate);
	}

	job_log(j, LOG_DEBUG, "Setting event state to %s for stream/token: %s/%llu", state ? "true" : "false", stream, token);

	struct externalevent *ei = externalevent_find(stream, token);
	if (!ei) {
		job_log(j, LOG_ERR, "Could not find stream/token: %s/%llu", stream, token);
		return ESRCH;
	}

	other_j = ei->job;
	ei->state = state;

	if (ei->internal) {
		job_log(ei->job, LOG_NOTICE, "Job should be able to exec(3) now.");
		ei->job->waiting4ok = false;
		externalevent_delete(ei);
	}

	(void)job_dispatch(other_j, false);

	xpc_object_t reply2 = xpc_dictionary_create_reply(request);
	*reply = reply2;

	return 0;
}

bool
xpc_event_demux(mach_port_t p, xpc_object_t request, xpc_object_t *reply)
{
	uint64_t op = xpc_dictionary_get_uint64(request, XPC_EVENT_ROUTINE_KEY_OP);
	if (!op) {
		return false;
	}

	audit_token_t token;
	xpc_dictionary_get_audit_token(request, &token);
	runtime_record_caller_creds(&token);

	struct ldcred *ldc = runtime_get_caller_creds();
	job_t j = managed_job(ldc->pid);
	if (!j) {
		j = job_mig_intran(p);
		if (!j) {
			op = -1;
		}
	}

	job_log(j, LOG_DEBUG, "Incoming XPC event request: %llu", op);

	int error = -1;
	switch (op) {
	case XPC_EVENT_GET_NAME:
		error = xpc_event_get_event_name(j, request, reply);
		break;
	case XPC_EVENT_SET:
		error = xpc_event_set_event(j, request, reply);
		break;
	case XPC_EVENT_COPY:
		error = xpc_event_copy_event(j, request, reply);
		break;
	case XPC_EVENT_CHECK_IN:
		error = xpc_event_channel_check_in(j, request, reply);
		break;
	case XPC_EVENT_LOOK_UP:
		error = xpc_event_channel_look_up(j, request, reply);
		break;
	case XPC_EVENT_PROVIDER_CHECK_IN:
		error = xpc_event_provider_check_in(j, request, reply);
		break;
	case XPC_EVENT_PROVIDER_SET_STATE:
		error = xpc_event_provider_set_state(j, request, reply);
		break;
	case XPC_EVENT_COPY_ENTITLEMENTS:
		error = xpc_event_copy_entitlements(j, request, reply);
		break;
	case -1:
		error = EINVAL;
		break;
	default:
		job_log(j, LOG_ERR, "Bogus opcode.");
		error = EDOM;
	}

	if (error) {
		xpc_object_t reply2 = xpc_dictionary_create_reply(request);
		xpc_dictionary_set_uint64(reply2, XPC_EVENT_ROUTINE_KEY_ERROR, error);
		*reply = reply2;
	}

	return true;
}

uint64_t
xpc_get_jetsam_entitlement(const char *key)
{
	uint64_t entitlement = 0;

	audit_token_t *token = runtime_get_caller_token();
	xpc_object_t value = xpc_copy_entitlement_for_token(key, token);
	if (value) {
		if (xpc_get_type(value) == XPC_TYPE_UINT64) {
			entitlement = xpc_uint64_get_value(value);
		}

		xpc_release(value);
	}
	
	return entitlement;	
}

int
xpc_process_set_jetsam_band(job_t j, xpc_object_t request, xpc_object_t *reply)
{	
	if (!j) {
		return EINVAL;
	}

	const char *label = xpc_dictionary_get_string(request, XPC_PROCESS_ROUTINE_KEY_LABEL);
	if (!label) {
		return EXINVAL;
	}

	xpc_jetsam_band_t entitled_band = -1;
	xpc_jetsam_band_t requested_band = (xpc_jetsam_band_t)xpc_dictionary_get_uint64(request, XPC_PROCESS_ROUTINE_KEY_PRIORITY_BAND);
	if (!requested_band) {
		return EXINVAL;
	}

	if (!(requested_band >= XPC_JETSAM_BAND_SUSPENDED && requested_band < XPC_JETSAM_BAND_LAST)) {
		return EXINVAL;
	}

	uint64_t rcdata = xpc_dictionary_get_uint64(request, XPC_PROCESS_ROUTINE_KEY_RCDATA);
	
	job_t tj = job_find(root_jobmgr, label);
	if (!tj) {
		return EXSRCH;
	}

	boolean_t allow = false;
	if (j->embedded_god) {
		allow = true;
	} else {
		entitled_band = xpc_get_jetsam_entitlement("com.apple.private.jetsam.modify-priority");
		if (entitled_band >= requested_band) {
			allow = true;
		}
	}

	if (!allow) {
		if (launchd_no_jetsam_perm_check) {
			job_log(j, LOG_NOTICE, "Jetsam priority checks disabled; allowing job to set priority: %d", requested_band);
		} else {		
			job_log(j, LOG_ERR, "Job cannot decrease Jetsam priority band (requested/maximum): %d/%d", requested_band, entitled_band);
			return EPERM;
		}
	}

	job_log(j, LOG_INFO, "Setting Jetsam band: %d.", requested_band);
	job_update_jetsam_properties(tj, requested_band, rcdata);

	xpc_object_t reply2 = xpc_dictionary_create_reply(request);
	*reply = reply2;

	return 0;
}

int
xpc_process_set_jetsam_memory_limit(job_t j, xpc_object_t request, xpc_object_t *reply)
{
	if (!j) {
		return EINVAL;
	}

	const char *label = xpc_dictionary_get_string(request, XPC_PROCESS_ROUTINE_KEY_LABEL);
	if (!label) {
		return EXINVAL;
	}

	int32_t entitlement_limit = 0;
	int32_t requested_limit = (int32_t)xpc_dictionary_get_uint64(request, XPC_PROCESS_ROUTINE_KEY_MEMORY_LIMIT);

	job_t tj = job_find(root_jobmgr, label);
	if (!tj) {
		return EXSRCH;
	}

	boolean_t allow = false;
	if (j->embedded_god) {
		allow = true;
	} else {
		entitlement_limit = (int32_t)xpc_get_jetsam_entitlement("com.apple.private.jetsam.memory_limit");		
		if (entitlement_limit >= requested_limit) {
			allow = true;
		}
	}

	if (!allow) {
		if (launchd_no_jetsam_perm_check) {
			job_log(j, LOG_NOTICE, "Jetsam priority checks disabled; allowing job to set memory limit: %d", requested_limit);
		} else {		
			job_log(j, LOG_ERR, "Job cannot set Jetsam memory limit (requested/maximum): %d/%d", requested_limit, entitlement_limit);
			return EPERM;
		}
	}

	job_log(j, LOG_INFO, "Setting Jetsam memory limit: %d.", requested_limit);
	job_update_jetsam_memory_limit(tj, requested_limit);

	xpc_object_t reply2 = xpc_dictionary_create_reply(request);
	*reply = reply2;

	return 0;
}

static jobmgr_t
_xpc_process_find_target_manager(job_t j, xpc_service_type_t type, pid_t pid)
{
	jobmgr_t target = NULL;
	if (type == XPC_SERVICE_TYPE_BUNDLED) {
		job_log(j, LOG_DEBUG, "Bundled service. Searching for XPC domains for PID: %d", pid);

		jobmgr_t jmi = NULL;
		SLIST_FOREACH(jmi, &root_jobmgr->submgrs, sle) {
			if (jmi->req_pid && jmi->req_pid == pid) {
				jobmgr_log(jmi, LOG_DEBUG, "Found job manager for PID.");
				target = jmi;
				break;
			}
		}
	} else if (type == XPC_SERVICE_TYPE_LAUNCHD || type == XPC_SERVICE_TYPE_APP) {
		target = j->mgr;
	}

	return target;
}

static int
xpc_process_attach(job_t j, xpc_object_t request, xpc_object_t *reply)
{
	if (!j) {
		return EINVAL;
	}

	audit_token_t *token = runtime_get_caller_token();
	xpc_object_t entitlement = xpc_copy_entitlement_for_token(XPC_SERVICE_ENTITLEMENT_ATTACH, token);
	if (!entitlement) {
		job_log(j, LOG_ERR, "Job does not have entitlement: %s", XPC_SERVICE_ENTITLEMENT_ATTACH);
		return EPERM;
	}

	if (entitlement != XPC_BOOL_TRUE) {
		char *desc = xpc_copy_description(entitlement);
		job_log(j, LOG_ERR, "Job has bad value for entitlement: %s:\n%s", XPC_SERVICE_ENTITLEMENT_ATTACH, desc);
		free(desc);

		xpc_release(entitlement);
		return EPERM;
	}

	const char *name = xpc_dictionary_get_string(request, XPC_PROCESS_ROUTINE_KEY_NAME);
	if (!name) {
		return EXINVAL;
	}

	xpc_service_type_t type = xpc_dictionary_get_int64(request, XPC_PROCESS_ROUTINE_KEY_TYPE);
	if (!type) {
		return EXINVAL;
	}

	mach_port_t port = xpc_dictionary_copy_mach_send(request, XPC_PROCESS_ROUTINE_KEY_NEW_INSTANCE_PORT);
	if (!MACH_PORT_VALID(port)) {
		return EXINVAL;
	}

	pid_t pid = xpc_dictionary_get_int64(request, XPC_PROCESS_ROUTINE_KEY_HANDLE);

	job_log(j, LOG_DEBUG, "Attaching to service: %s", name);

	xpc_object_t reply2 = xpc_dictionary_create_reply(request);
	jobmgr_t target = _xpc_process_find_target_manager(j, type, pid);
	if (target) {
		jobmgr_log(target, LOG_DEBUG, "Found target job manager for service: %s", name);
		(void)jobmgr_assumes(target, waiting4attach_new(target, name, port, 0, type));

		/* HACK: This is awful. For legacy reasons, launchd job labels are all
		 * stored in a global namespace, which is stored in the root job
		 * manager. But XPC domains have a per-domain namespace. So if we're
		 * looking for a legacy launchd job, we have to redirect any attachment
		 * attempts to the root job manager to find existing instances.
		 *
		 * But because we store attachments on a per-job manager basis, we have
		 * to create the new attachment in the actual target job manager, hence
		 * why we change the target only after we've created the attachment.
		 */
		if (strcmp(target->name, VPROCMGR_SESSION_AQUA) == 0) {
			target = root_jobmgr;
		}

		job_t existing = job_find(target, name);
		if (existing && existing->p) {
			job_log(existing, LOG_DEBUG, "Found existing instance of service.");
			xpc_dictionary_set_int64(reply2, XPC_PROCESS_ROUTINE_KEY_PID, existing->p);
		} else {
			xpc_dictionary_set_uint64(reply2, XPC_PROCESS_ROUTINE_KEY_ERROR, ESRCH);
		}
	} else if (type == XPC_SERVICE_TYPE_BUNDLED) {
		(void)job_assumes(j, waiting4attach_new(target, name, port, pid, type));
		xpc_dictionary_set_uint64(reply2, XPC_PROCESS_ROUTINE_KEY_ERROR, ESRCH);
	} else {
		xpc_dictionary_set_uint64(reply2, XPC_PROCESS_ROUTINE_KEY_ERROR, EXSRCH);
	}

	*reply = reply2;
	return 0;
}

static int
xpc_process_detach(job_t j, xpc_object_t request, xpc_object_t *reply __unused)
{
	if (!j) {
		return EINVAL;
	}

	const char *name = xpc_dictionary_get_string(request, XPC_PROCESS_ROUTINE_KEY_NAME);
	if (!name) {
		return EXINVAL;
	}

	xpc_service_type_t type = xpc_dictionary_get_int64(request, XPC_PROCESS_ROUTINE_KEY_TYPE);
	if (!type) {
		return EXINVAL;
	}

	job_log(j, LOG_DEBUG, "Deatching from service: %s", name);

	pid_t pid = xpc_dictionary_get_int64(request, XPC_PROCESS_ROUTINE_KEY_PID);
	jobmgr_t target = _xpc_process_find_target_manager(j, type, pid);
	if (target) {
		jobmgr_log(target, LOG_DEBUG, "Found target job manager for service: %s", name);

		struct waiting4attach *w4ai = NULL;
		struct waiting4attach *w4ait = NULL;
		LIST_FOREACH_SAFE(w4ai, &target->attaches, le, w4ait) {
			if (strcmp(name, w4ai->name) == 0) {
				jobmgr_log(target, LOG_DEBUG, "Found attachment. Deleting.");
				waiting4attach_delete(target, w4ai);
				break;
			}
		}
	}

	return 0;
}

static int
xpc_process_get_properties(job_t j, xpc_object_t request, xpc_object_t *reply)
{
	if (j->anonymous) {
		/* Total hack. libxpc will send requests to the pipe created out of the
		 * process' bootstrap port, so when job_mig_intran() tries to resolve
		 * the process into a job, it'll wind up creating an anonymous job if
		 * the requestor was an XPC service, whose job manager is an XPC domain.
		 */
		pid_t pid = j->p;
		jobmgr_t jmi = NULL;
		SLIST_FOREACH(jmi, &root_jobmgr->submgrs, sle) {
			if ((j = jobmgr_find_by_pid(jmi, pid, false))) {
				break;
			}
		}
	}

	if (!j || j->anonymous) {
		return EXINVAL;
	}

	struct waiting4attach *w4a = waiting4attach_find(j->mgr, j);
	if (!w4a) {
		return EXINVAL;
	}

	xpc_object_t reply2 = xpc_dictionary_create_reply(request);
	xpc_dictionary_set_uint64(reply2, XPC_PROCESS_ROUTINE_KEY_TYPE, w4a->type);
	xpc_dictionary_set_mach_send(reply2, XPC_PROCESS_ROUTINE_KEY_NEW_INSTANCE_PORT, w4a->port);
	if (j->prog) {
		xpc_dictionary_set_string(reply2, XPC_PROCESS_ROUTINE_KEY_PATH, j->prog);
	} else {
		xpc_dictionary_set_string(reply2, XPC_PROCESS_ROUTINE_KEY_PATH, j->argv[0]);
	}

	if (j->argv) {
		xpc_object_t xargv = xpc_array_create(NULL, 0);

		size_t i = 0;
		for (i = 0; i < j->argc; i++) {
			if (j->argv[i]) {
				xpc_array_set_string(xargv, XPC_ARRAY_APPEND, j->argv[i]);
			}
		}

		xpc_dictionary_set_value(reply2, XPC_PROCESS_ROUTINE_KEY_ARGV, xargv);
		xpc_release(xargv);
	}

	*reply = reply2;
	return 0;
}

static int
xpc_process_service_kill(job_t j, xpc_object_t request, xpc_object_t *reply)
{
#if XPC_LPI_VERSION >= 20130426
	if (!j) {
		return ESRCH;
	}

	jobmgr_t jm = _xpc_process_find_target_manager(j, XPC_SERVICE_TYPE_BUNDLED, j->p);
	if (!jm) {
		return ENOENT;
	}

	const char *name = xpc_dictionary_get_string(request, XPC_PROCESS_ROUTINE_KEY_NAME);
	if (!name) {
		return EINVAL;
	}

	int64_t whichsig = xpc_dictionary_get_int64(request, XPC_PROCESS_ROUTINE_KEY_SIGNAL);
	if (!whichsig) {
		return EINVAL;
	}

	job_t j2kill = job_find(jm, name);
	if (!j2kill) {
		return ESRCH;
	}

	if (j2kill->alias) {
		// Only allow for private instances to be killed.
		return EPERM;
	}

	struct proc_bsdshortinfo proc;
	if (proc_pidinfo(j2kill->p, PROC_PIDT_SHORTBSDINFO, 1, &proc, PROC_PIDT_SHORTBSDINFO_SIZE) == 0) {
		if (errno != ESRCH) {
			(void)jobmgr_assumes_zero(root_jobmgr, errno);
		}

		return errno;
	}

	struct ldcred *ldc = runtime_get_caller_creds();
	if (proc.pbsi_uid != ldc->euid) {
		// Do not allow non-root to kill RoleAccount services running as a
		// different user.
		return EPERM;
	}

	if (!j2kill->p) {
		return EALREADY;
	}

	xpc_object_t reply2 = xpc_dictionary_create_reply(request);
	if (!reply2) {
		return EINVAL;
	}

	int error = 0;
	int ret = kill(j2kill->p, whichsig);
	if (ret) {
		error = errno;
	}

	xpc_dictionary_set_int64(reply2, XPC_PROCESS_ROUTINE_KEY_ERROR, error);
	*reply = reply2;
	return 0;
#else
	return ENOTSUP;
#endif
}

bool
xpc_process_demux(mach_port_t p, xpc_object_t request, xpc_object_t *reply)
{
	uint64_t op = xpc_dictionary_get_uint64(request, XPC_PROCESS_ROUTINE_KEY_OP);
	if (!op) {
		return false;
	}

	audit_token_t token;
	xpc_dictionary_get_audit_token(request, &token);
	runtime_record_caller_creds(&token);

	job_t j = job_mig_intran(p);
	job_log(j, LOG_DEBUG, "Incoming XPC process request: %llu", op);

	int error = -1;
	switch (op) {
	case XPC_PROCESS_JETSAM_SET_BAND:
		error = xpc_process_set_jetsam_band(j, request, reply);
		break;
	case XPC_PROCESS_JETSAM_SET_MEMORY_LIMIT:
		error = xpc_process_set_jetsam_memory_limit(j, request, reply);
		break;
	case XPC_PROCESS_SERVICE_ATTACH:
		error = xpc_process_attach(j, request, reply);
		break;
	case XPC_PROCESS_SERVICE_DETACH:
		error = xpc_process_detach(j, request, reply);
		break;
	case XPC_PROCESS_SERVICE_GET_PROPERTIES:
		error = xpc_process_get_properties(j, request, reply);
		break;
	case XPC_PROCESS_SERVICE_KILL:
		error = xpc_process_service_kill(j, request, reply);
		break;
	default:
		job_log(j, LOG_ERR, "Bogus process opcode.");
		error = EDOM;
	}

	if (error) {
		xpc_object_t reply2 = xpc_dictionary_create_reply(request);
		if (reply2) {
			xpc_dictionary_set_uint64(reply2, XPC_PROCESS_ROUTINE_KEY_ERROR, error);
		}

		*reply = reply2;
	}

	return true;
}

kern_return_t
job_mig_kickstart(job_t j, name_t targetlabel, pid_t *out_pid, unsigned int flags)
{
	struct ldcred *ldc = runtime_get_caller_creds();
	job_t otherj;

	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	if (unlikely(!(otherj = job_find(NULL, targetlabel)))) {
		return BOOTSTRAP_UNKNOWN_SERVICE;
	}

#if TARGET_OS_EMBEDDED
	bool allow_non_root_kickstart = j->username && otherj->username && (strcmp(j->username, otherj->username) == 0);
#else
	bool allow_non_root_kickstart = false;
#endif

	if (ldc->euid != 0 && ldc->euid != geteuid() && !allow_non_root_kickstart) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

#if HAVE_SANDBOX
	if (unlikely(sandbox_check(ldc->pid, "job-creation", SANDBOX_FILTER_NONE) > 0)) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}
#endif

	if (otherj->p && (flags & VPROCFLAG_STALL_JOB_EXEC)) {
		return BOOTSTRAP_SERVICE_ACTIVE;
	}

	otherj->stall_before_exec = (flags & VPROCFLAG_STALL_JOB_EXEC);
	otherj = job_dispatch(otherj, true);

	if (!job_assumes(j, otherj && otherj->p)) {
		// <rdar://problem/6787083> Clear this flag if we failed to start the job.
		otherj->stall_before_exec = false;
		return BOOTSTRAP_NO_MEMORY;
	}

	*out_pid = otherj->p;

	return 0;
}

kern_return_t
job_mig_spawn_internal(job_t j, vm_offset_t indata, mach_msg_type_number_t indataCnt, mach_port_t asport, job_t *outj)
{
	launch_data_t jobdata = NULL;
	size_t data_offset = 0;
	struct ldcred *ldc = runtime_get_caller_creds();
	job_t jr;

	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	if (unlikely(j->deny_job_creation)) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}

#if HAVE_SANDBOX
	if (unlikely(sandbox_check(ldc->pid, "job-creation", SANDBOX_FILTER_NONE) > 0)) {
		return BOOTSTRAP_NOT_PRIVILEGED;
	}
#endif

	if (unlikely(pid1_magic && ldc->euid && ldc->uid)) {
		job_log(j, LOG_DEBUG, "Punting spawn to per-user-context");
		return VPROC_ERR_TRY_PER_USER;
	}

	if (!job_assumes(j, indataCnt != 0)) {
		return 1;
	}

	runtime_ktrace0(RTKT_LAUNCHD_DATA_UNPACK);
	if (!job_assumes(j, (jobdata = launch_data_unpack((void *)indata, indataCnt, NULL, 0, &data_offset, NULL)) != NULL)) {
		return 1;
	}

	jobmgr_t target_jm = jobmgr_find_by_name(j->mgr, NULL);
	if (!jobmgr_assumes(j->mgr, target_jm != NULL)) {
		jobmgr_log(j->mgr, LOG_ERR, "This API can only be used by a process running within an Aqua session.");
		return 1;
	}

	jr = jobmgr_import2(target_jm ?: j->mgr, jobdata);

	launch_data_t label = NULL;
	launch_data_t wait4debugger = NULL;
	if (!jr) {
		switch (errno) {
		case EEXIST:
			/* If EEXIST was returned, we know that there is a label string in
			 * the dictionary. So we don't need to check the types here; that
			 * has already been done.
			 */
			label = launch_data_dict_lookup(jobdata, LAUNCH_JOBKEY_LABEL);
			jr = job_find(NULL, launch_data_get_string(label));
			if (job_assumes(j, jr != NULL) && !jr->p) {
				wait4debugger = launch_data_dict_lookup(jobdata, LAUNCH_JOBKEY_WAITFORDEBUGGER);
				if (wait4debugger && launch_data_get_type(wait4debugger) == LAUNCH_DATA_BOOL) {
					if (launch_data_get_bool(wait4debugger)) {
						/* If the job exists, we're going to kick-start it, but
						 * we need to give the caller the opportunity to start
						 * it suspended if it so desires. But this will only
						 * take effect if the job isn't running.
						 */
						jr->wait4debugger_oneshot = true;
					}
				}
			}

			*outj = jr;
			return BOOTSTRAP_NAME_IN_USE;
		default:
			return BOOTSTRAP_NO_MEMORY;
		}
	}

	if (pid1_magic) {
		jr->mach_uid = ldc->uid;
	}

	// TODO: Consolidate the app and legacy_LS_job bits.
	jr->legacy_LS_job = true;
	jr->abandon_pg = true;
	jr->asport = asport;
	jr->app = true;
	uuid_clear(jr->expected_audit_uuid);
	jr = job_dispatch(jr, true);

	if (!job_assumes(j, jr != NULL)) {
		job_remove(jr);
		return BOOTSTRAP_NO_MEMORY;
	}

	if (!job_assumes(jr, jr->p)) {
		job_remove(jr);
		return BOOTSTRAP_NO_MEMORY;
	}

	job_log(jr, LOG_DEBUG, "Spawned by PID %u: %s", j->p, j->label);
	*outj = jr;

	return BOOTSTRAP_SUCCESS;
}

kern_return_t
job_mig_spawn2(job_t j, mach_port_t rp, vm_offset_t indata, mach_msg_type_number_t indataCnt, mach_port_t asport, pid_t *child_pid, mach_port_t *obsvr_port)
{
	job_t nj = NULL;
	kern_return_t kr = job_mig_spawn_internal(j, indata, indataCnt, asport, &nj);
	if (likely(kr == KERN_SUCCESS)) {
		if (job_setup_exit_port(nj) != KERN_SUCCESS) {
			job_remove(nj);
			kr = BOOTSTRAP_NO_MEMORY;
		} else {
			/* Do not return until the job has called exec(3), thereby making it
			 * safe for the caller to send it SIGCONT.
			 *
			 * <rdar://problem/9042798>
			 */
			nj->spawn_reply_port = rp;
			kr = MIG_NO_REPLY;
		}
	} else if (kr == BOOTSTRAP_NAME_IN_USE) {
		bool was_running = nj->p;
		if (job_dispatch(nj, true)) {
			if (!was_running) {
				job_log(nj, LOG_DEBUG, "Job exists but is not running. Kick-starting.");

				if (job_setup_exit_port(nj) == KERN_SUCCESS) {
					nj->spawn_reply_port = rp;
					kr = MIG_NO_REPLY;
				} else {
					kr = BOOTSTRAP_NO_MEMORY;
				}
			} else {
				*obsvr_port = MACH_PORT_NULL;
				*child_pid = nj->p;
				kr = KERN_SUCCESS;
			}
		} else {
			job_log(nj, LOG_ERR, "Failed to dispatch job, requestor: %s", j->label);
			kr = BOOTSTRAP_UNKNOWN_SERVICE;
		}
	}

	mig_deallocate(indata, indataCnt);
	return kr;
}

launch_data_t
job_do_legacy_ipc_request(job_t j, launch_data_t request, mach_port_t asport __attribute__((unused)))
{
	launch_data_t reply = NULL;

	errno = ENOTSUP;
	if (launch_data_get_type(request) == LAUNCH_DATA_STRING) {
		if (strcmp(launch_data_get_string(request), LAUNCH_KEY_CHECKIN) == 0) {
			reply = job_export(j);
			job_checkin(j);
		}
	}

	return reply;
}

#define LAUNCHD_MAX_LEGACY_FDS 128
#define countof(x) (sizeof((x)) / sizeof((x[0])))

kern_return_t
job_mig_legacy_ipc_request(job_t j, vm_offset_t request, 
	mach_msg_type_number_t requestCnt, mach_port_array_t request_fds,
	mach_msg_type_number_t request_fdsCnt, vm_offset_t *reply,
	mach_msg_type_number_t *replyCnt, mach_port_array_t *reply_fdps,
	mach_msg_type_number_t *reply_fdsCnt, mach_port_t asport)
{
	if (!j) {
		return BOOTSTRAP_NO_MEMORY;
	}

	/* TODO: Once we support actions other than checking in, we must check the
	 * sandbox capabilities and EUID of the requestort.
	 */
	size_t nout_fdps = 0;
	size_t nfds = request_fdsCnt / sizeof(request_fds[0]);
	if (nfds > LAUNCHD_MAX_LEGACY_FDS) {
		job_log(j, LOG_ERR, "Too many incoming descriptors: %lu", nfds);
		return BOOTSTRAP_NO_MEMORY;
	}

	int in_fds[LAUNCHD_MAX_LEGACY_FDS];
	size_t i = 0;
	for (i = 0; i < nfds; i++) {
		in_fds[i] = fileport_makefd(request_fds[i]);
		if (in_fds[i] == -1) {
			job_log(j, LOG_ERR, "Bad descriptor passed in legacy IPC request at index: %lu", i);
		}
	}

	// DON'T goto outbad before this point.
	*reply = 0;
	*reply_fdps = NULL;
	launch_data_t ldreply = NULL;

	size_t dataoff = 0;
	size_t fdoff = 0;
	launch_data_t ldrequest = launch_data_unpack((void *)request, requestCnt, in_fds, nfds, &dataoff, &fdoff);
	if (!ldrequest) {
		job_log(j, LOG_ERR, "Invalid legacy IPC request passed.");
		goto out_bad;
	}

	ldreply = job_do_legacy_ipc_request(j, ldrequest, asport);
	if (!ldreply) {
		ldreply = launch_data_new_errno(errno);
		if (!ldreply) {
			goto out_bad;
		}
	}

	*replyCnt = 10 * 1024 * 1024;
	mig_allocate(reply, *replyCnt);
	if (!*reply) {
		goto out_bad;
	}

	int out_fds[LAUNCHD_MAX_LEGACY_FDS];
	size_t nout_fds = 0;
	size_t sz = launch_data_pack(ldreply, (void *)*reply, *replyCnt, out_fds, &nout_fds);
	if (!sz) {
		job_log(j, LOG_ERR, "Could not pack legacy IPC reply.");
		goto out_bad;
	}

	if (nout_fds) {
		if (nout_fds > 128) {
			job_log(j, LOG_ERR, "Too many outgoing descriptors: %lu", nout_fds);
			goto out_bad;
		}

		*reply_fdsCnt = nout_fds * sizeof((*reply_fdps)[0]);
		mig_allocate((vm_address_t *)reply_fdps, *reply_fdsCnt);
		if (!*reply_fdps) {
			goto out_bad;
		}

		for (i = 0; i < nout_fds; i++) {
			mach_port_t fp = MACH_PORT_NULL;
			/* Whatever. Worst case is that we insert MACH_PORT_NULL. Not a big
			 * deal. Note, these get stuffed into an array whose disposition is
			 * mach_port_move_send_t, so we don't have to worry about them after
			 * returning.
			 */
			if (fileport_makeport(out_fds[i], &fp) != 0) {
				job_log(j, LOG_ERR, "Could not pack response descriptor at index: %lu: %d: %s", i, errno, strerror(errno));
			}
			(*reply_fdps)[i] = fp;
		}

		nout_fdps = nout_fds;
	} else {
		*reply_fdsCnt = 0;
	}

	mig_deallocate(request, requestCnt);
	launch_data_free(ldreply);
	ldreply = NULL;

	// Unused for now.
	(void)launchd_mport_deallocate(asport);

	return BOOTSTRAP_SUCCESS;

out_bad:
	for (i = 0; i < nfds; i++) {
		(void)close(in_fds[i]);
	}

	for (i = 0; i < nout_fds; i++) {
		(void)launchd_mport_deallocate((*reply_fdps)[i]);
	}

	if (*reply) {
		mig_deallocate(*reply, *replyCnt);
	}

	/* We should never hit this since the last goto out is in the case that
	 * allocating this fails.
	 */
	if (*reply_fdps) {
		mig_deallocate((vm_address_t)*reply_fdps, *reply_fdsCnt);
	}

	if (ldreply) {
		launch_data_free(ldreply);
	}

	return BOOTSTRAP_NO_MEMORY;
}

void
jobmgr_init(bool sflag)
{
	const char *root_session_type = pid1_magic ? VPROCMGR_SESSION_SYSTEM : VPROCMGR_SESSION_BACKGROUND;
	SLIST_INIT(&s_curious_jobs);
	LIST_INIT(&s_needing_sessions);

	os_assert((root_jobmgr = jobmgr_new(NULL, MACH_PORT_NULL, MACH_PORT_NULL, sflag, root_session_type, false, MACH_PORT_NULL)) != NULL);
	os_assert((_s_xpc_system_domain = jobmgr_new_xpc_singleton_domain(root_jobmgr, "com.apple.xpc.system")) != NULL);
	_s_xpc_system_domain->req_asid = launchd_audit_session;
	_s_xpc_system_domain->req_asport = launchd_audit_port;
	_s_xpc_system_domain->shortdesc = "system";
	if (pid1_magic) {
		root_jobmgr->monitor_shutdown = true;
	}

	uint32_t fflags = NOTE_ATTRIB | NOTE_LINK | NOTE_REVOKE | NOTE_EXTEND | NOTE_WRITE;
	s_no_hang_fd = open("/dev/autofs_nowait", O_EVTONLY | O_NONBLOCK);
	if (likely(s_no_hang_fd == -1)) {
		if (jobmgr_assumes_zero_p(root_jobmgr, (s_no_hang_fd = open("/dev", O_EVTONLY | O_NONBLOCK))) != -1) {
			(void)jobmgr_assumes_zero_p(root_jobmgr, kevent_mod((uintptr_t)s_no_hang_fd, EVFILT_VNODE, EV_ADD, fflags, 0, root_jobmgr));
		}
	}
	s_no_hang_fd = _fd(s_no_hang_fd);
}

size_t
our_strhash(const char *s)
{
	size_t c, r = 5381;

	/* djb2
	 * This algorithm was first reported by Dan Bernstein many years ago in comp.lang.c
	 */

	while ((c = *s++)) {
		r = ((r << 5) + r) + c; // hash*33 + c
	}

	return r;
}

size_t
hash_label(const char *label)
{
	return our_strhash(label) % LABEL_HASH_SIZE;
}

size_t
hash_ms(const char *msstr)
{
	return our_strhash(msstr) % MACHSERVICE_HASH_SIZE;
}

bool
waiting4removal_new(job_t j, mach_port_t rp)
{
	struct waiting_for_removal *w4r;

	if (!job_assumes(j, (w4r = malloc(sizeof(struct waiting_for_removal))) != NULL)) {
		return false;
	}

	w4r->reply_port = rp;

	SLIST_INSERT_HEAD(&j->removal_watchers, w4r, sle);

	return true;
}

void
waiting4removal_delete(job_t j, struct waiting_for_removal *w4r)
{
	(void)job_assumes_zero(j, job_mig_send_signal_reply(w4r->reply_port, 0));

	SLIST_REMOVE(&j->removal_watchers, w4r, waiting_for_removal, sle);

	free(w4r);
}

size_t
get_kern_max_proc(void)
{
	int mib[] = { CTL_KERN, KERN_MAXPROC };
	int max = 100;
	size_t max_sz = sizeof(max);

	(void)posix_assumes_zero(sysctl(mib, 2, &max, &max_sz, NULL, 0));

	return max;
}

// See rdar://problem/6271234
void
eliminate_double_reboot(void)
{
	if (unlikely(!pid1_magic)) {
		return;
	}

	struct stat sb;
	const char *argv[] = { _PATH_BSHELL, "/etc/rc.deferred_install", NULL };
	int result = -1;

	if (unlikely(stat(argv[1], &sb) != -1)) {
		jobmgr_log(root_jobmgr, LOG_DEBUG | LOG_CONSOLE, "Going to run deferred install script.");

		pid_t p = 0;
		result = posix_spawnp(&p, argv[0], NULL, NULL, (char **)argv, environ);
		if (result == -1) {
			jobmgr_log(root_jobmgr, LOG_WARNING | LOG_CONSOLE, "Couldn't run deferred install script: %d: %s", result, strerror(result));
			goto out;
		}

		int wstatus = 0;
		result = waitpid(p, &wstatus, 0);
		if (result == -1) {
			jobmgr_log(root_jobmgr, LOG_WARNING | LOG_CONSOLE, "Failed to reap deferred install script: %d: %s", errno, strerror(errno));
			goto out;
		}

		if (WIFEXITED(wstatus)) {
			if ((result = WEXITSTATUS(wstatus)) == 0) {
				jobmgr_log(root_jobmgr, LOG_DEBUG | LOG_CONSOLE, "Deferred install script completed successfully.");
			} else {
				jobmgr_log(root_jobmgr, LOG_WARNING | LOG_CONSOLE, "Deferred install script failed with status: %d", WEXITSTATUS(wstatus));
			}
		} else {
			jobmgr_log(root_jobmgr, LOG_WARNING | LOG_CONSOLE, "Weirdness with install script: %d", wstatus);
		}
	}
out:
	if (result == 0) {
		/* If the unlink(2) was to fail, it would be most likely fail with
		 * EBUSY. All the other failure cases for unlink(2) don't apply when
		 * we're running under PID 1 and have verified that the file exists.
		 * Outside of someone deliberately messing with us (like if
		 * /etc/rc.deferredinstall is actually a looping sym-link or a mount
		 * point for a filesystem) and I/O errors, we should be good.
		 */
		if (unlink(argv[1]) == -1) {
			jobmgr_log(root_jobmgr, LOG_WARNING | LOG_CONSOLE, "Failed to remove deferred install script: %d: %s", errno, strerror(errno));
		}
	}
}

void
jetsam_property_setup(launch_data_t obj, const char *key, job_t j)
{
	job_log(j, LOG_DEBUG, "Setting Jetsam properties for job...");
	if (strcasecmp(key, LAUNCH_JOBKEY_JETSAMPRIORITY) == 0 && launch_data_get_type(obj) == LAUNCH_DATA_INTEGER) {
		j->jetsam_priority = (typeof(j->jetsam_priority))launch_data_get_integer(obj);

#if XPC_LPI_VERSION >= 20120810
		if (j->jetsam_priority > XPC_JETSAM_PRIORITY_RESERVED && j->jetsam_priority < XPC_JETSAM_PRIORITY_RESERVED + XPC_JETSAM_BAND_LAST) {
			size_t band = j->jetsam_priority - XPC_JETSAM_PRIORITY_RESERVED;
			j->jetsam_priority = _launchd_priority_map[band - 1].priority;
		}
#endif
		job_log(j, LOG_DEBUG, "Priority: %d", j->jetsam_priority);
	} else if (strcasecmp(key, LAUNCH_JOBKEY_JETSAMMEMORYLIMIT) == 0 && launch_data_get_type(obj) == LAUNCH_DATA_INTEGER) {
		j->jetsam_memlimit = (typeof(j->jetsam_memlimit))launch_data_get_integer(obj);
		job_log(j, LOG_DEBUG, "Memory limit: %d", j->jetsam_memlimit);
	} else if (strcasecmp(key, LAUNCH_JOBKEY_JETSAMMEMORYLIMITBACKGROUND) == 0) {
		j->jetsam_memory_limit_background = true;
		job_log(j, LOG_DEBUG, "Memory limit is for background state only");
	} else if (strcasecmp(key, LAUNCH_KEY_JETSAMFRONTMOST) == 0) {
		/* Ignore. We only recognize this key so we don't complain when we get SpringBoard's request. 
		 * You can't set this in a plist.
		 */
	} else if (strcasecmp(key, LAUNCH_KEY_JETSAMACTIVE) == 0) {
		// Ignore.
	} else if (strcasecmp(key, LAUNCH_KEY_JETSAMLABEL) == 0) {
		/* Ignore. This key is present in SpringBoard's request dictionary, so we don't want to
		 * complain about it.
		 */
	} else {
		job_log(j, LOG_ERR, "Unknown Jetsam key: %s", key);
	}

	if (unlikely(!j->jetsam_properties)) {
		j->jetsam_properties = true;
	}
}

void
job_update_jetsam_properties(job_t j, xpc_jetsam_band_t band, uint64_t user_data)
{
#if TARGET_OS_EMBEDDED
	j->jetsam_priority = _launchd_priority_map[band - 1].priority;
	j->jetsam_properties = true;

	memorystatus_priority_properties_t mjp;
	mjp.priority = j->jetsam_priority;
	mjp.user_data = user_data;

	size_t size = sizeof(mjp);
	int r = memorystatus_control(MEMORYSTATUS_CMD_SET_PRIORITY_PROPERTIES, j->p, 0, &mjp, size);
	if (r == -1 && errno != ESRCH) {
		(void)job_assumes_zero(j, errno);
	}
#else
#pragma unused(j, band, user_data)
#endif
}

void
job_update_jetsam_memory_limit(job_t j, int32_t limit)
{
#if TARGET_OS_EMBEDDED
	j->jetsam_memlimit = limit;
	j->jetsam_properties = true;

	int r = memorystatus_control(MEMORYSTATUS_CMD_SET_JETSAM_HIGH_WATER_MARK, j->p, limit, NULL, 0);
	if (r == -1 && errno != ESRCH) {
		(void)job_assumes_zero(j, errno);
	}
#else
#pragma unused(j, limit)
#endif
}
