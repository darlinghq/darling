#ifndef __LAUNCHD_LOG_H__
#define __LAUNCHD_LOG_H__

#include <mach/mach.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <sys/fcntl.h>
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

extern char *launchd_username;
extern char *launchd_label;
extern mach_port_t launchd_drain_reply_port;
extern bool launchd_var_available;
extern int64_t launchd_system_start;

struct launchd_syslog_attr {
	const char *from_name;
	const char *about_name;
	const char *session_name;
	int priority;
	uid_t from_uid;
	pid_t from_pid;
	pid_t about_pid;
};

/* These constants must not have the high bit set so we can safely mask them
 * mask them with LOG_CONSOLE.
 */
#define LOG_PERF 0x5252615d
#define LOG_APPLEONLY 0x4141504c
#define LOG_CONSOLE (1 << 31)

__attribute__((visibility("default")))
__attribute__((used))
extern bool
_launchd_os_redirect(const char *message);

int
runtime_setlogmask(int maskpri);

void
launchd_closelog(void);

__attribute__((format(printf, 2, 3)))
void
launchd_syslog(int pri, const char *message, ...);

__attribute__((format(printf, 2, 0)))
void
launchd_vsyslog(struct launchd_syslog_attr *attr, const char *message, va_list args);

void
launchd_log_push(void);

kern_return_t
launchd_log_forward(uid_t forward_uid, gid_t forward_gid, vm_offset_t inval, mach_msg_type_number_t invalCnt);

kern_return_t
launchd_log_drain(mach_port_t srp, vm_offset_t *outval, mach_msg_type_number_t *outvalCnt);

#endif /* __LAUNCHD_LOG_H__ */
