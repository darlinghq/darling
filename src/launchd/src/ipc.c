/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
#include "ipc.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/event.h>
#include <sys/stat.h>
#include <sys/ucred.h>
#include <sys/fcntl.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/sysctl.h>
#include <sys/sockio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <paths.h>
#include <string.h>
#include <os/assumes.h>

#include "launch.h"
#include "launch_priv.h"
#include "launchd.h"
#include "runtime.h"
#include "core.h"

extern char **environ;

static LIST_HEAD(, conncb) connections;

static launch_data_t adjust_rlimits(launch_data_t in);

static void ipc_readmsg2(launch_data_t data, const char *cmd, void *context);
static void ipc_readmsg(launch_data_t msg, void *context);

static void ipc_listen_callback(void *obj __attribute__((unused)), struct kevent *kev);

static kq_callback kqipc_listen_callback = ipc_listen_callback;

static pid_t ipc_self = 0;

char *sockpath = NULL;
static char *sockdir = NULL;

static bool ipc_inited = false;

static void
ipc_clean_up(void)
{
	if (ipc_self != getpid()) {
		return;
	}

	if (-1 == unlink(sockpath)) {
		launchd_syslog(LOG_WARNING, "unlink(\"%s\"): %s", sockpath, strerror(errno));
	} else if (-1 == rmdir(sockdir)) {
		launchd_syslog(LOG_WARNING, "rmdir(\"%s\"): %s", sockdir, strerror(errno));
	}
}

void
ipc_server_init(void)
{
	struct sockaddr_un sun;
	mode_t oldmask;
	int r, fd = -1;
	char ourdir[1024];

	if (ipc_inited) {
		return;
	}

	memset(&sun, 0, sizeof(sun));
	sun.sun_family = AF_UNIX;

	if (pid1_magic) {
		strcpy(ourdir, LAUNCHD_SOCK_PREFIX);
		strncpy(sun.sun_path, LAUNCHD_SOCK_PREFIX "/sock", sizeof(sun.sun_path));

		unlink(ourdir);
		if (mkdir(ourdir, S_IRWXU) == -1) {
			if (errno == EROFS) {
				goto out_bad;
			} else if (errno == EEXIST) {
				struct stat sb;
				stat(ourdir, &sb);
				if (!S_ISDIR(sb.st_mode)) {
					errno = EEXIST;
					launchd_syslog(LOG_ERR, "mkdir(\"%s\"): %s", LAUNCHD_SOCK_PREFIX, strerror(errno));
					goto out_bad;
				}
			} else {
				launchd_syslog(LOG_ERR, "mkdir(\"%s\"): %s", ourdir, strerror(errno));
				goto out_bad;
			}
		}
	} else {
		snprintf(ourdir, sizeof(ourdir), _PATH_TMP "launchd-%u.XXXXXX", getpid());
		if (mkdtemp(ourdir) == NULL) {
			launchd_syslog(LOG_ERR, "Could not create critical directory \"%s\": %s", ourdir, strerror(errno));
			goto out_bad;
		}
		snprintf(sun.sun_path, sizeof(sun.sun_path), "%s/sock", ourdir);
	}

	if (unlink(sun.sun_path) == -1 && errno != ENOENT) {
		if (errno != EROFS) {
			launchd_syslog(LOG_ERR, "unlink(\"thesocket\"): %s", strerror(errno));
		}
		goto out_bad;
	}

	if (posix_assumes_zero(fd = _fd(socket(AF_UNIX, SOCK_STREAM, 0))) == -1) {
		goto out_bad;
	}

	oldmask = umask(S_IRWXG|S_IRWXO);
	r = bind(fd, (struct sockaddr *)&sun, sizeof(sun));
	umask(oldmask);

	if (r == -1) {
		if (errno != EROFS) {
			launchd_syslog(LOG_ERR, "bind(\"thesocket\"): %s", strerror(errno));
		}
		goto out_bad;
	}

	if (listen(fd, SOMAXCONN) == -1) {
		launchd_syslog(LOG_ERR, "listen(\"thesocket\"): %s", strerror(errno));
		goto out_bad;
	}

	if (kevent_mod(fd, EVFILT_READ, EV_ADD, 0, 0, &kqipc_listen_callback) == -1) {
		launchd_syslog(LOG_ERR, "kevent_mod(\"thesocket\", EVFILT_READ): %s", strerror(errno));
		goto out_bad;
	}

	ipc_inited = true;

	sockdir = strdup(ourdir);
	sockpath = strdup(sun.sun_path);
	ipc_self = getpid();
	atexit(ipc_clean_up);

out_bad:
	if (!ipc_inited && fd != -1) {
		(void)runtime_close(fd);
	}
}

void
ipc_open(int fd, job_t j)
{
	struct conncb *c = calloc(1, sizeof(struct conncb));

	fcntl(fd, F_SETFL, O_NONBLOCK);

	c->kqconn_callback = ipc_callback;
	if (j) {
		c->conn = launchd_fdopen(-1, fd);
	} else {
		c->conn = launchd_fdopen(fd, -1);
	}

	c->j = j;
	LIST_INSERT_HEAD(&connections, c, sle);
	kevent_mod(fd, EVFILT_READ, EV_ADD, 0, 0, &c->kqconn_callback);
}

void
ipc_listen_callback(void *obj __attribute__((unused)), struct kevent *kev)
{
	struct sockaddr_un sun;
	socklen_t sl = sizeof(sun);
	int cfd;

	if ((cfd = _fd(accept(kev->ident, (struct sockaddr *)&sun, &sl))) == -1) {
		return;
	}

	if (geteuid() == 0) {
		uid_t euid, guid;
		if (getpeereid(cfd, &euid, &guid) == -1) {
			launchd_syslog(LOG_NOTICE | LOG_CONSOLE, "*** launchd[%d] failed to getpeereid on incoming caller (%d)", getpid(), errno);
			(void)runtime_close(cfd);
			return;
		}

		if (euid != geteuid()) {
			launchd_syslog(LOG_NOTICE | LOG_CONSOLE, "*** launchd[%d] failed to euid check on incoming caller (%d != %d)", getpid(), euid, geteuid());
			(void)runtime_close(cfd);
			return;
		}
	}

	ipc_open(cfd, NULL);
}

void
ipc_callback(void *obj, struct kevent *kev)
{
	struct conncb *c = obj;
	int r;

	if (kev->filter == EVFILT_READ) {
		if (launchd_msg_recv(c->conn, ipc_readmsg, c) == -1 && errno != EAGAIN) {
			if (errno != ECONNRESET) {
				launchd_syslog(LOG_DEBUG, "%s(): recv: %s", __func__, strerror(errno));
			}
			ipc_close(c);
		}
	} else if (kev->filter == EVFILT_WRITE) {
		r = launchd_msg_send(c->conn, NULL);
		if (r == -1) {
			if (errno != EAGAIN) {
				launchd_syslog(LOG_DEBUG, "%s(): send: %s", __func__, strerror(errno));
				ipc_close(c);
			}
		} else if (r == 0) {
			kevent_mod(launchd_getfd(c->conn), EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
		}
	} else {
		launchd_syslog(LOG_DEBUG, "%s(): unknown filter type!", __func__);
		ipc_close(c);
	}
}

static void 
set_user_env(launch_data_t obj, const char *key, void *context __attribute__((unused)))
{
	const char *v = launch_data_get_string(obj);
	if (v) {
		setenv(key, v, 1);
	} else {
		launchd_syslog(LOG_WARNING, "Attempt to set NULL environment variable: %s (type = %d)", key, launch_data_get_type(obj));
	}
}

void
ipc_close_all_with_job(job_t j)
{
	struct conncb *ci, *cin;

	LIST_FOREACH_SAFE(ci, &connections, sle, cin) {
		if (ci->j == j) {
			ipc_close(ci);
		}
	}
}

void
ipc_close_fds(launch_data_t o)
{
	size_t i;

	switch (launch_data_get_type(o)) {
	case LAUNCH_DATA_DICTIONARY:
		launch_data_dict_iterate(o, (void (*)(launch_data_t, const char *, void *))ipc_close_fds, NULL);
		break;
	case LAUNCH_DATA_ARRAY:
		for (i = 0; i < launch_data_array_get_count(o); i++)
			ipc_close_fds(launch_data_array_get_index(o, i));
		break;
	case LAUNCH_DATA_FD:
		if (launch_data_get_fd(o) != -1) {
			(void)runtime_close(launch_data_get_fd(o));
		}
		break;
	default:
		break;
	}
}

void
ipc_revoke_fds(launch_data_t o)
{
	size_t i;

	switch (launch_data_get_type(o)) {
	case LAUNCH_DATA_DICTIONARY:
		launch_data_dict_iterate(o, (void (*)(launch_data_t, const char *, void *))ipc_revoke_fds, NULL);
		break;
	case LAUNCH_DATA_ARRAY:
		for (i = 0; i < launch_data_array_get_count(o); i++)
			ipc_revoke_fds(launch_data_array_get_index(o, i));
		break;
	case LAUNCH_DATA_FD:
		launch_data_set_fd(o, -1);
		break;
	default:
		break;
	}
}

struct readmsg_context {
	struct conncb *c;
	launch_data_t resp;
};

void
ipc_readmsg(launch_data_t msg, void *context)
{
	struct readmsg_context rmc = { context, NULL };

	if (LAUNCH_DATA_DICTIONARY == launch_data_get_type(msg)) {
		launch_data_dict_iterate(msg, ipc_readmsg2, &rmc);
	} else if (LAUNCH_DATA_STRING == launch_data_get_type(msg)) {
		ipc_readmsg2(NULL, launch_data_get_string(msg), &rmc);
	} else {
		rmc.resp = launch_data_new_errno(EINVAL);
	}

	if (NULL == rmc.resp) {
		rmc.resp = launch_data_new_errno(ENOSYS);
	}

	ipc_close_fds(msg);

	if (launchd_msg_send(rmc.c->conn, rmc.resp) == -1) {
		if (errno == EAGAIN) {
			kevent_mod(launchd_getfd(rmc.c->conn), EVFILT_WRITE, EV_ADD, 0, 0, &rmc.c->kqconn_callback);
		} else {
			launchd_syslog(LOG_DEBUG, "launchd_msg_send() == -1: %s", strerror(errno));
			ipc_close(rmc.c);
		}
	}
	launch_data_free(rmc.resp);
}

void
ipc_readmsg2(launch_data_t data, const char *cmd, void *context)
{
	struct readmsg_context *rmc = context;
	launch_data_t resp = NULL;
	job_t j;

	if (rmc->resp) {
		return;
	}

	/* Do not allow commands other than check-in to come over the trusted socket
	 * on the Desktop. On Embedded, allow all commands over the trusted socket
	 * if the job has the God Mode key set.
	 */
#if TARGET_OS_EMBEDDED
	bool allow_privileged_ops = (!rmc->c->j || job_is_god(rmc->c->j));
#else
	bool allow_privileged_ops = !rmc->c->j;
#endif

	if (rmc->c->j && strcmp(cmd, LAUNCH_KEY_CHECKIN) == 0) {
		resp = job_export(rmc->c->j);
		job_checkin(rmc->c->j);
	} else if (allow_privileged_ops) {
#if TARGET_OS_EMBEDDED
		launchd_embedded_handofgod = rmc->c->j && job_is_god(rmc->c->j);
#endif
		if (data == NULL) {
			if (!strcmp(cmd, LAUNCH_KEY_SHUTDOWN)) {
				launchd_shutdown();
				resp = launch_data_new_errno(0);
			} else if (!strcmp(cmd, LAUNCH_KEY_GETJOBS)) {
				resp = job_export_all();
				ipc_revoke_fds(resp);
			} else if (!strcmp(cmd, LAUNCH_KEY_GETRESOURCELIMITS)) {
				resp = adjust_rlimits(NULL);
			} else if (!strcmp(cmd, LAUNCH_KEY_GETRUSAGESELF)) {
				struct rusage rusage;
				getrusage(RUSAGE_SELF, &rusage);
				resp = launch_data_new_opaque(&rusage, sizeof(rusage));
			} else if (!strcmp(cmd, LAUNCH_KEY_GETRUSAGECHILDREN)) {
				struct rusage rusage;
				getrusage(RUSAGE_CHILDREN, &rusage);
				resp = launch_data_new_opaque(&rusage, sizeof(rusage));
			}
		} else {
			if (!strcmp(cmd, LAUNCH_KEY_STARTJOB)) {
				if ((j = job_find(NULL, launch_data_get_string(data))) != NULL) {
					errno = job_dispatch(j, true) ? 0 : errno;
				}
				resp = launch_data_new_errno(errno);
			} else if (!strcmp(cmd, LAUNCH_KEY_STOPJOB)) {
				if ((j = job_find(NULL, launch_data_get_string(data))) != NULL) {
					errno = 0;
					job_stop(j);
				}
				resp = launch_data_new_errno(errno);
			} else if (!strcmp(cmd, LAUNCH_KEY_REMOVEJOB)) {
				if ((j = job_find(NULL, launch_data_get_string(data))) != NULL) {
					errno = 0;
					job_remove(j);
				}
				resp = launch_data_new_errno(errno);
			} else if (!strcmp(cmd, LAUNCH_KEY_SUBMITJOB)) {
				if (launch_data_get_type(data) == LAUNCH_DATA_ARRAY) {
					resp = job_import_bulk(data);
				} else {
					if (job_import(data)) {
						errno = 0;
					}
					resp = launch_data_new_errno(errno);
				}
			} else if (!strcmp(cmd, LAUNCH_KEY_UNSETUSERENVIRONMENT)) {
				unsetenv(launch_data_get_string(data));
				resp = launch_data_new_errno(0);
			} else if (!strcmp(cmd, LAUNCH_KEY_SETUSERENVIRONMENT)) {
				launch_data_dict_iterate(data, set_user_env, NULL);
				resp = launch_data_new_errno(0);
			} else if (!strcmp(cmd, LAUNCH_KEY_SETRESOURCELIMITS)) {
				resp = adjust_rlimits(data);
			} else if (!strcmp(cmd, LAUNCH_KEY_GETJOB)) {
				if ((j = job_find(NULL, launch_data_get_string(data))) == NULL) {
					resp = launch_data_new_errno(errno);
				} else {
					resp = job_export(j);
					ipc_revoke_fds(resp);
				}
			}
		}
#if TARGET_OS_EMBEDDED
		launchd_embedded_handofgod = false;
#endif
	} else {
		resp = launch_data_new_errno(EACCES);
	}

	rmc->resp = resp;
}

static int
close_abi_fixup(int fd)
{
	return runtime_close(fd);
}

void
ipc_close(struct conncb *c)
{
	LIST_REMOVE(c, sle);
	launchd_close(c->conn, close_abi_fixup);
	free(c);
}

launch_data_t
adjust_rlimits(launch_data_t in)
{
	/* If I never have to deal with this rlimit nonsense again, I'll be a very
	 * happy man.
	 */
	struct rlimit l[RLIM_NLIMITS];
	struct rlimit *ltmp;
	size_t i,ltmpsz;

	for (i = 0; i < RLIM_NLIMITS; i++) {
		(void)posix_assumes_zero(getrlimit(i, l + i));
	}

	if (in) {
		ltmp = launch_data_get_opaque(in);
		ltmpsz = launch_data_get_opaque_size(in);

		if (ltmpsz > sizeof(l)) {
			launchd_syslog(LOG_WARNING, "Too much rlimit data sent!");
			ltmpsz = sizeof(l);
		}

		for (i = 0; i < (ltmpsz / sizeof(struct rlimit)); i++) {
			if (ltmp[i].rlim_cur == l[i].rlim_cur && ltmp[i].rlim_max == l[i].rlim_max) {
				continue;
			}

			if (/* XXX readcfg_pid && */ pid1_magic && (i == RLIMIT_NOFILE || i == RLIMIT_NPROC)) {
				int gmib[] = { CTL_KERN, KERN_MAXPROC };
				int pmib[] = { CTL_KERN, KERN_MAXPROCPERUID };
				const char *gstr = "kern.maxproc";
				const char *pstr = "kern.maxprocperuid";
				int gval = ltmp[i].rlim_max;
				int pval = ltmp[i].rlim_cur;
				switch (i) {
				case RLIMIT_NOFILE:
					gmib[1] = KERN_MAXFILES;
					pmib[1] = KERN_MAXFILESPERPROC;
					gstr = "kern.maxfiles";
					pstr = "kern.maxfilesperproc";
					break;
				default:
					break;
				}

				if (gval > 0) {
					(void)posix_assumes_zero(sysctl(gmib, 2, NULL, NULL, &gval, sizeof(gval)));
				} else {
					launchd_syslog(LOG_WARNING, "sysctl(\"%s\"): can't be zero", gstr);
				}
				if (pval > 0) {
					(void)posix_assumes_zero(sysctl(pmib, 2, NULL, NULL, &pval, sizeof(pval)));
				} else {
					launchd_syslog(LOG_WARNING, "sysctl(\"%s\"): can't be zero", pstr);
				}
			}
			(void)posix_assumes_zero(setrlimit(i, ltmp + i));
			/* the kernel may have clamped the values we gave it */
			(void)posix_assumes_zero(getrlimit(i, l + i));
		}
	}

	return launch_data_new_opaque(l, sizeof(struct rlimit) * RLIM_NLIMITS);
}
