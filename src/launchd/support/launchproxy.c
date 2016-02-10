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
#include <sys/types.h>
#include <sys/select.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <syslog.h>
#include <libgen.h>
#include <getopt.h>
#include <signal.h>
#include <netdb.h>

#if !TARGET_OS_EMBEDDED
#include <bsm/audit.h>
#include <bsm/audit_session.h>
#endif // !TARGET_OS_EMBEDDED

#include "launch.h"

static int kq = 0;

static void find_fds(launch_data_t o, const char *key __attribute__((unused)), void *context __attribute__((unused)))
{
	struct kevent kev;
	size_t i;
	int fd;

	switch (launch_data_get_type(o)) {
	case LAUNCH_DATA_FD:
		fd = launch_data_get_fd(o);
		if (-1 == fd)
			break;
		fcntl(fd, F_SETFD, 1);
		EV_SET(&kev, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
		if (kevent(kq, &kev, 1, NULL, 0, NULL) == -1)
			syslog(LOG_DEBUG, "kevent(%d): %m", fd);
		break;
	case LAUNCH_DATA_ARRAY:
		for (i = 0; i < launch_data_array_get_count(o); i++)
			find_fds(launch_data_array_get_index(o, i), NULL, NULL);
		break;
	case LAUNCH_DATA_DICTIONARY:
		launch_data_dict_iterate(o, find_fds, NULL);
		break;
	default:
		break;
	}
}

int main(int argc __attribute__((unused)), char *argv[])
{
	struct timespec timeout = { 10, 0 };
	struct sockaddr_storage ss;
	socklen_t slen = (socklen_t)sizeof ss;
	struct kevent kev;
	int r, ec = EXIT_FAILURE;
	launch_data_t tmp, resp, msg = launch_data_alloc(LAUNCH_DATA_STRING);
	const char *prog = argv[1];
	bool w = false, dupstdout = true, dupstderr = true;

	launch_data_set_string(msg, LAUNCH_KEY_CHECKIN);

	openlog(getprogname(), LOG_PERROR|LOG_PID|LOG_CONS, LOG_LAUNCHD);

	kq = kqueue();

	if ((resp = launch_msg(msg)) == NULL) {
		syslog(LOG_ERR, "launch_msg(%s): %m", LAUNCH_KEY_CHECKIN);
		goto out;
	}

	launch_data_free(msg);

	tmp = launch_data_dict_lookup(resp, LAUNCH_JOBKEY_SOCKETS);
	if (tmp) {
		find_fds(tmp, NULL, NULL);
	} else {
		syslog(LOG_ERR, "No FDs found to answer requests on!");
		goto out;
	}

	tmp = launch_data_dict_lookup(resp, LAUNCH_JOBKEY_TIMEOUT);
	if (tmp)
		timeout.tv_sec = (int)launch_data_get_integer(tmp);

	tmp = launch_data_dict_lookup(resp, LAUNCH_JOBKEY_PROGRAM);
	if (tmp)
		prog = launch_data_get_string(tmp);

	tmp = launch_data_dict_lookup(resp, LAUNCH_JOBKEY_INETDCOMPATIBILITY);
	if (tmp) {
		tmp = launch_data_dict_lookup(tmp, LAUNCH_JOBINETDCOMPATIBILITY_WAIT);
		if (tmp)
			w = launch_data_get_bool(tmp);
	}

	if (launch_data_dict_lookup(resp, LAUNCH_JOBKEY_STANDARDOUTPATH))
		dupstdout = false;

	if (launch_data_dict_lookup(resp, LAUNCH_JOBKEY_STANDARDERRORPATH))
		dupstderr = false;

	if (!w)
		signal(SIGCHLD, SIG_IGN);

	for (;;) {
		if ((r = kevent(kq, NULL, 0, &kev, 1, &timeout)) == -1) {
			syslog(LOG_DEBUG, "kevent(): %m");
			goto out;
		} else if (r == 0) {
			ec = EXIT_SUCCESS;
			goto out;
		}

		if (w) {
			dup2((int)kev.ident, STDIN_FILENO);
			if (dupstdout)
				dup2((int)kev.ident, STDOUT_FILENO);
			if (dupstderr)
				dup2((int)kev.ident, STDERR_FILENO);
			execv(prog, argv + 1);
			syslog(LOG_ERR, "execv(): %m");
			exit(EXIT_FAILURE);
		}

		if ((r = accept((int)kev.ident, (struct sockaddr *)&ss, &slen)) == -1) {
			if (errno == EWOULDBLOCK)
				continue;
			syslog(LOG_WARNING, "accept(): %m");
			goto out;
		} else {
			if (ss.ss_family == AF_INET || ss.ss_family == AF_INET6) {
				char fromhost[NI_MAXHOST];
				char fromport[NI_MAXSERV];
				int gni_r;

				gni_r = getnameinfo((struct sockaddr *)&ss, slen,
						fromhost, (socklen_t) sizeof fromhost,
						fromport, (socklen_t) sizeof fromport,
						NI_NUMERICHOST | NI_NUMERICSERV);

				if (gni_r) {
					syslog(LOG_WARNING, "%s: getnameinfo(): %s", prog, gai_strerror(gni_r));
				} else {
					syslog(LOG_INFO, "%s: Connection from: %s on port: %s", prog, fromhost, fromport);
				}
			} else {
				syslog(LOG_WARNING, "%s: getnameinfo() only supports IPv4/IPv6. Connection from address family: %u", prog, ss.ss_family);
			}

			switch (fork()) {
			case -1:
				syslog(LOG_WARNING, "fork(): %m");
				if (errno != ENOMEM) {
					continue;
				}
				goto out;
			case 0:
				break;
			default:
				close(r);
				continue;
			}

			setpgid(0, 0);

#if !TARGET_OS_EMBEDDED
			if ((tmp = launch_data_dict_lookup(resp, LAUNCH_JOBKEY_SESSIONCREATE)) && launch_data_get_bool(tmp)) {
				auditinfo_addr_t auinfo = {
					.ai_termid = { .at_type = AU_IPv4 },
					.ai_asid = AU_ASSIGN_ASID,
					.ai_auid = getuid(),
					.ai_flags = 0,
				};
				if (setaudit_addr(&auinfo, sizeof(auinfo)) == 0) {
					char session[16]; 
					snprintf(session, sizeof(session), "%x", auinfo.ai_asid);
					setenv("SECURITYSESSIONID", session, 1);
				} else {
					syslog(LOG_NOTICE, "%s: Setting Audit Session ID failed: %d", prog, errno);
				}
			}
#endif // !TARGET_OS_EMBEDDED
			fcntl(r, F_SETFL, 0);
			fcntl(r, F_SETFD, 1);
			dup2(r, STDIN_FILENO);
			if (dupstdout)
				dup2(r, STDOUT_FILENO);
			if (dupstderr)
				dup2(r, STDERR_FILENO);
			signal(SIGCHLD, SIG_DFL);
			execv(prog, argv + 1);
			syslog(LOG_ERR, "execv(): %m");
			exit(EXIT_FAILURE);
		}
	}

out:
	exit(ec);
}
