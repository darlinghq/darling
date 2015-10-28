#include <dispatch/dispatch.h>
#include <os/assumes.h>
#include "job_reply.h"

#include "launchd.h"
#include "launch_internal.h"
#include "vproc_internal.h"
#include "log.h"

#define ROUND_TO_64BIT_WORD_SIZE(x)	((x + 7) & ~7)
#define LAUNCHD_DEBUG_LOG "launchd-debug.%s.log"
#define LAUNCHD_PERF_LOG "launchd-perf.%s.log"
#define LAUNCHD_SHUTDOWN_LOG "launchd-shutdown.%s.log"
#define LAUNCHD_LOWLEVEL_LOG "launchd-lowlevel.%s.log"

os_redirect_assumes(_launchd_os_redirect);

char *launchd_username = "unknown";
char *launchd_label = "com.apple.launchd.unknown";
mach_port_t launchd_drain_reply_port;
bool launchd_var_available = false;
int64_t launchd_system_start;

static FILE *_launchd_shutdown_log;
static FILE *_launchd_debug_log;
static FILE *_launchd_perf_log;
static STAILQ_HEAD(, logmsg_s) _launchd_logq = STAILQ_HEAD_INITIALIZER(_launchd_logq);
static size_t _launchd_logq_sz;
static size_t _launchd_logq_cnt;
static int _launchd_log_up2 = LOG_UPTO(LOG_NOTICE);

static int64_t _launchd_shutdown_start;

struct _launchd_open_log_ctx_s {
	const char *path;
	FILE **filep;
};

static void
_launchd_open_log_once(void *ctx)
{
	struct _launchd_open_log_ctx_s *ctx2 = ctx;
	const char *path = ctx2->path;
	FILE **filep = ctx2->filep;

	char saved[PATH_MAX];
	snprintf(saved, sizeof(saved), "%s.1", path);
	(void)rename(path, saved);

	FILE *file = fopen(path, "w");
	if (file) {
		(void)_fd(fileno(file));
		*filep = file;
	} else if (launchd_console) {
		fprintf(launchd_console, "Could not open %s: %d: %s\n", path, errno, strerror(errno));
	}
}

static void
_launchd_shutdown_start_once(void *ctx __attribute__((unused)))
{
	_launchd_shutdown_start = runtime_get_wall_time();
}

int
runtime_setlogmask(int maskpri)
{
	_launchd_log_up2 = maskpri;
	return _launchd_log_up2;
}

static bool
_logmsg_add(struct launchd_syslog_attr *attr, int err_num, const char *msg)
{
	size_t lm_sz = sizeof(struct logmsg_s) + strlen(msg) + strlen(attr->from_name) + strlen(attr->about_name) + strlen(attr->session_name) + 4;
	char *data_off;
	struct logmsg_s *lm;

	/* Force the unpacking for the log_drain cause unalignment faults. */
	lm_sz = ROUND_TO_64BIT_WORD_SIZE(lm_sz);

	if (unlikely((lm = calloc(1, lm_sz)) == NULL)) {
		return false;
	}

	data_off = lm->data;

	lm->when = runtime_get_wall_time();
	lm->from_pid = attr->from_pid;
	lm->about_pid = attr->about_pid;
	lm->err_num = err_num;
	lm->pri = attr->priority;
	lm->obj_sz = lm_sz;
	lm->msg = data_off;
	data_off += sprintf(data_off, "%s", msg) + 1;
	lm->from_name = data_off;
	data_off += sprintf(data_off, "%s", attr->from_name) + 1;
	lm->about_name = data_off;
	data_off += sprintf(data_off, "%s", attr->about_name) + 1;
	lm->session_name = data_off;
	data_off += sprintf(data_off, "%s", attr->session_name) + 1;

	STAILQ_INSERT_TAIL(&_launchd_logq, lm, sqe);
	_launchd_logq_sz += lm_sz;
	_launchd_logq_cnt++;

	return true;
}

static void
_logmsg_remove(struct logmsg_s *lm)
{
	STAILQ_REMOVE(&_launchd_logq, lm, logmsg_s, sqe);
	_launchd_logq_sz -= lm->obj_sz;
	_launchd_logq_cnt--;

	free(lm);
}

bool
_launchd_os_redirect(const char *message)
{
	launchd_syslog(LOG_ERR, "%s", message);
	return true;
}

void
launchd_syslog(int pri, const char *message, ...)
{
	struct launchd_syslog_attr attr = {
		.from_name = launchd_label,
		.about_name = launchd_label,
		.session_name = pid1_magic ? "System" : "Background",
		.priority = pri,
		.from_uid = launchd_uid,
		.from_pid = getpid(),
		.about_pid = getpid(),
	};

	va_list ap;

	va_start(ap, message);
	launchd_vsyslog(&attr, message, ap);
	va_end(ap);
}

void
launchd_vsyslog(struct launchd_syslog_attr *attr, const char *fmt, va_list args)
{
	int saved_errno = errno;
	char message[2048];

	static dispatch_once_t perf_once = 0;
	static dispatch_once_t shutdown_once = 0;
	static dispatch_once_t shutdown_start_once = 0;
	static dispatch_once_t debug_once = 0;

	bool echo2console = (attr->priority & LOG_CONSOLE);
	attr->priority &= ~LOG_CONSOLE;
	if (attr->priority == LOG_APPLEONLY && launchd_apple_internal) {
		attr->priority = LOG_NOTICE;
	}

	FILE *log2here = NULL;

	/* launchctl is responsible for mounting /var as read-write. So we need to
	 * wait until /var/log is available before trying to log anything to our log
	 * anything to our auxilliary log files. This is kind of a hack because
	 * we'll lose the first few relevant messages at boot for debug and
	 * performance logging, but the loss isn't too bad. 
	 */
	if (launchd_var_available) {
		/* This file is for logging low-level errors where we can't necessarily be
		 * assured that we can write to the console or use syslog.
		 */
		char *store = launchd_copy_persistent_store(LAUNCHD_PERSISTENT_STORE_LOGS, NULL);
		char path[PATH_MAX];

		if (attr->priority == LOG_PERF) {
			if (launchd_log_perf) {
				(void)snprintf(path, sizeof(path), "%s" LAUNCHD_PERF_LOG, store, launchd_username);

				struct _launchd_open_log_ctx_s ctx2 = {
					.path = path,
					.filep = &_launchd_perf_log,
				};
				dispatch_once_f(&perf_once, &ctx2, _launchd_open_log_once);
				log2here = _launchd_perf_log;
			}

			// Don't log performance messages to the normal syslog store.
			attr->priority = LOG_DEBUG + 1;
		} else {
			if (launchd_shutting_down && launchd_log_shutdown) {
				dispatch_once_f(&shutdown_start_once, NULL, _launchd_shutdown_start_once);

				(void)snprintf(path, sizeof(path), "%s" LAUNCHD_SHUTDOWN_LOG, store, launchd_username);

				struct _launchd_open_log_ctx_s ctx2 = {
					.path = path,
					.filep = &_launchd_shutdown_log,
				};
				dispatch_once_f(&shutdown_once, &ctx2, _launchd_open_log_once);
				log2here = _launchd_shutdown_log;
			} else if (launchd_log_debug) {
				(void)snprintf(path, sizeof(path), "%s" LAUNCHD_DEBUG_LOG, store, launchd_username);

				struct _launchd_open_log_ctx_s ctx2 = {
					.path = path,
					.filep = &_launchd_debug_log,
				};
				dispatch_once_f(&debug_once, &ctx2, _launchd_open_log_once);
				log2here = _launchd_debug_log;
			}
		}

		free(store);

	}

	vsnprintf(message, sizeof(message), fmt, args);
	if (echo2console && launchd_console) {
		fprintf(launchd_console, "%-32s %-8u %-64s %-8u  %s\n", attr->from_name, attr->from_pid, attr->about_name, attr->about_pid, message);
	}

	if (log2here) {
		int64_t delta = 0;
		if (log2here == _launchd_shutdown_log) {
			delta = runtime_get_wall_time() - _launchd_shutdown_start;
		} else {
			delta = runtime_get_wall_time() - launchd_system_start;
		}

		fprintf(log2here, "%-8lld %-32s %-8u %-24s %-8u  %s\n", delta, attr->from_name, attr->from_pid, attr->about_name, attr->about_pid, message);
	}

	if ((LOG_MASK(attr->priority) & _launchd_log_up2)) {
		_logmsg_add(attr, saved_errno, message);
	}
}

static kern_return_t
_launchd_log_pack(vm_offset_t *outval, mach_msg_type_number_t *outvalCnt)
{
	struct logmsg_s *lm;
	void *offset;

	*outvalCnt = _launchd_logq_sz;

	mig_allocate(outval, *outvalCnt);

	if (unlikely(*outval == 0)) {
		return 1;
	}

	offset = (void *)*outval;
	while ((lm = STAILQ_FIRST(&_launchd_logq))) {
		lm->from_name_offset = lm->from_name - (char *)lm;
		lm->about_name_offset = lm->about_name - (char *)lm;
		lm->msg_offset = lm->msg - (char *)lm;
		lm->session_name_offset = lm->session_name - (char *)lm;

		memcpy(offset, lm, lm->obj_sz);

		offset += lm->obj_sz;

		_logmsg_remove(lm);
	}

	return 0;
}

static void
_launchd_log_uncork_pending_drain(void)
{
	mach_msg_type_number_t outvalCnt;
	mach_port_t tmp_port;
	vm_offset_t outval;

	if (!launchd_drain_reply_port) {
		return;
	}

	if (_launchd_logq_cnt == 0) {
		return;
	}

	if (_launchd_log_pack(&outval, &outvalCnt) != 0) {
		return;
	}

	tmp_port = launchd_drain_reply_port;
	launchd_drain_reply_port = MACH_PORT_NULL;

	if (unlikely(errno = job_mig_log_drain_reply(tmp_port, 0, outval, outvalCnt))) {
		if (errno != MACH_SEND_INVALID_DEST) {
			(void)os_assumes_zero(errno);
		}
		(void)os_assumes_zero(launchd_mport_deallocate(tmp_port));
	}

	mig_deallocate(outval, outvalCnt);
}

void
launchd_log_push(void)
{
	vm_offset_t outval = 0;
	mach_msg_type_number_t outvalCnt = 0;

	if (!pid1_magic) {
		if (_launchd_perf_log) {
			(void)fflush(_launchd_perf_log);
		}
		if (_launchd_shutdown_log) {
			(void)fflush(_launchd_shutdown_log);
		}
		if (_launchd_debug_log) {
			(void)fflush(_launchd_debug_log);
		}

		if (_launchd_logq_cnt && _launchd_log_pack(&outval, &outvalCnt) == 0) {
			(void)_vprocmgr_log_forward(inherited_bootstrap_port, (void *)outval, outvalCnt);
			mig_deallocate(outval, outvalCnt);
		}
	} else {
		_launchd_log_uncork_pending_drain();
	}
}

kern_return_t
launchd_log_forward(uid_t forward_uid, gid_t forward_gid, vm_offset_t inval, mach_msg_type_number_t invalCnt)
{
	struct logmsg_s *lm, *lm_walk;
	mach_msg_type_number_t data_left = invalCnt;

	if (inval == 0) {
		return 0;
	}

	for (lm_walk = (struct logmsg_s *)inval; (data_left > 0) && (lm_walk->obj_sz <= data_left); lm_walk = ((void *)lm_walk + lm_walk->obj_sz)) {
		/* malloc(3) returns non-NULL when you ask for zero bytes. If our object
		 * is zero bytes, something is wrong.
		 */
		if (lm_walk->obj_sz == 0) {
			launchd_syslog(LOG_WARNING, "Encountered a log message of size 0 with %u bytes left in forwarded data. Ignoring remaining messages.", data_left);
			break;
		}

		if (lm_walk->obj_sz < sizeof(struct logmsg_s)) {
			launchd_syslog(LOG_WARNING, "Received bytes %llu are less than expected bytes %lu.", lm_walk->obj_sz, sizeof(struct logmsg_s));
			break;
		}

		if (!(lm = malloc(lm_walk->obj_sz))) {
			launchd_syslog(LOG_WARNING, "Failed to allocate %llu bytes for log message with %u bytes left in forwarded data. Ignoring remaining messages.", lm_walk->obj_sz, data_left);
			break;
		}

		memcpy(lm, lm_walk, lm_walk->obj_sz);
		lm->sender_uid = forward_uid;
		lm->sender_gid = forward_gid;

		lm->from_name += (size_t)lm;
		lm->about_name += (size_t)lm;
		lm->msg += (size_t)lm;
		lm->session_name += (size_t)lm;

		STAILQ_INSERT_TAIL(&_launchd_logq, lm, sqe);
		_launchd_logq_sz += lm->obj_sz;
		_launchd_logq_cnt++;

		data_left -= lm->obj_sz;
	}

	mig_deallocate(inval, invalCnt);

	return 0;
}

kern_return_t
launchd_log_drain(mach_port_t srp, vm_offset_t *outval, mach_msg_type_number_t *outvalCnt)
{
	(void)os_assumes_zero(launchd_drain_reply_port);

	if ((_launchd_logq_cnt == 0) || launchd_shutting_down) {
		launchd_drain_reply_port = srp;
		(void)os_assumes_zero(launchd_mport_notify_req(launchd_drain_reply_port, MACH_NOTIFY_DEAD_NAME));

		return MIG_NO_REPLY;
	}

	return _launchd_log_pack(outval, outvalCnt);
}

void
launchd_closelog(void)
{
	launchd_log_push();

	if (_launchd_shutdown_log) {
		(void)fflush(_launchd_shutdown_log);
		(void)runtime_fsync(fileno(_launchd_shutdown_log));
	}
}
