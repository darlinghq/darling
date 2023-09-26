/*
 * This file is part of Darling.
 * 
 * Copyright (C) 2023 Darling Developers
 * 
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/syslimits.h>
#include <errno.h>
#include <pwd.h>

#include <liblaunch/launch_priv.h>
#include <bootstrap_priv.h>
#include <vproc_priv.h>

#include "shellsession.h"
#include "shellspawn.h"

#define MAX_ACCESS_TRIES 10

static void setup_socket_addr(struct sockaddr_un* addr);
static void setup_session_socket_addr(unsigned int uid, struct sockaddr_un* addr);
static int setup_socket(const struct sockaddr_un* addr);
static void listen_for_connections(int server_socket);
static void spawn_session(int sockfd);
static void send_client_socket(int sockfd, int socket_to_send);
static int try_connect_session(const struct sockaddr_un* addr);
static void run_session_manager(unsigned int uid, unsigned int gid);

int main(int argc, const char* const* argv) {
	struct sockaddr_un server_addr;
	int server_socket = -1;

	setup_socket_addr(&server_addr);

	server_socket = setup_socket(&server_addr);
	if (server_socket < 0) {
		exit(EXIT_FAILURE);
	}

	listen_for_connections(server_socket);

	if (server_socket >= 0) {
		close(server_socket);
	}

	return 0;
};

static void setup_socket_addr(struct sockaddr_un* addr) {
	memset(addr, 0, sizeof(*addr));
	addr->sun_family = AF_UNIX;
	snprintf(addr->sun_path, sizeof(addr->sun_path), "%s", SHELLSESSION_SOCKPATH);
};

static void setup_session_socket_addr(unsigned int uid, struct sockaddr_un* addr) {
	memset(addr, 0, sizeof(*addr));
	addr->sun_family = AF_UNIX;
	snprintf(addr->sun_path, sizeof(addr->sun_path), "%s.%d", SHELLSPAWN_SOCKPATH, uid);
};

static int setup_socket(const struct sockaddr_un* addr) {
	int server_socket = -1;

	server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server_socket == -1) {
		perror("Creating unix socket");
		goto err;
	}

	fcntl(server_socket, F_SETFD, FD_CLOEXEC);
	unlink(addr->sun_path);

	if (bind(server_socket, (struct sockaddr*)addr, sizeof(*addr)) == -1) {
		perror("Binding the unix socket");
		goto err;
	}

	chmod(addr->sun_path, S_IRUSR | S_IWUSR);

	if (listen(server_socket, 1) == -1) {
		perror("Listening on unix socket");
		goto err;
	}

	return server_socket;

err:
	if (server_socket >= 0) {
		close(server_socket);
	}
	return -1;
};

static void listen_for_connections(int server_socket) {
	int sock;
	struct sockaddr_un client_addr;
	socklen_t len = sizeof(client_addr);

	while (true) {
		sock = accept(server_socket, (struct sockaddr*)&client_addr, &len);
		if (sock == -1) {
			break;
		}

		if (fork() == 0) {
			// we don't need the server socket
			close(server_socket);
			server_socket = -1;

			fcntl(sock, F_SETFD, FD_CLOEXEC);
			spawn_session(sock);

			exit(EXIT_SUCCESS);
		} else {
			close(sock);
		}
	}
};

void spawn_session(int sockfd) {
	shellsession_cmd_t cmd;
	struct sockaddr_un session_addr;
	int client_socket = -1;
	char session_started_path[PATH_MAX];
	int started_fd = -1;

	if (read(sockfd, &cmd, sizeof(cmd)) != sizeof(cmd)) {
		goto err;
	}

	setup_session_socket_addr(cmd.uid, &session_addr);
	snprintf(session_started_path, sizeof(session_started_path), "%s.%d", SHELLSESSION_STARTED_PATH, cmd.uid);

	started_fd = open(session_started_path, O_RDONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (started_fd < 0 && errno == EEXIST) {
		// we already had a session set up; avoid setting up a new one.

		// wait until shellspawn starts
		for (int i = 0; i < MAX_ACCESS_TRIES; i++) {
			if (access(session_addr.sun_path, F_OK) == 0) {
				break;
			}

			sleep(1);
		}

		client_socket = try_connect_session(&session_addr);
		if (client_socket < 0) {
			goto err;
		}

		// now reply to the client with the session client socket
		send_client_socket(sockfd, client_socket);
		close(sockfd);
		return;
	} else if (started_fd < 0) {
		perror("Failed to create session indicator file");
		goto err;
	}

	// we didn't have a session set up already, so let's create one.

	// we only need to create the session-running file; we don't need to write anything to it
	close(started_fd);

	if (fork() == 0) {
		// this is the new session manager process
		close(sockfd);
		run_session_manager(cmd.uid, cmd.gid);
	} else {
		// wait until shellspawn starts
		for (int i = 0; i < MAX_ACCESS_TRIES; i++) {
			if (access(session_addr.sun_path, F_OK) == 0) {
				break;
			}

			sleep(1);
		}

		client_socket = try_connect_session(&session_addr);
		if (client_socket < 0) {
			fprintf(stderr, "Error connecting to shellspawn session\n");
			exit(EXIT_FAILURE);
		}

		// now reply to the client with the session client socket
		send_client_socket(sockfd, client_socket);
		close(sockfd);
	}

	return;

err:
	close(sockfd);
};

static void send_client_socket(int sockfd, int socket_to_send) {
	char dummy = '\0';
	struct msghdr msg;
	char cmsgbuf[CMSG_SPACE(sizeof(int))];
	struct cmsghdr *cmptr;
	struct iovec iov;

	memset(&msg, 0, sizeof(msg));
	memset(&iov, 0, sizeof(iov));

	iov.iov_base = &dummy;
	iov.iov_len = sizeof(dummy);

	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = cmsgbuf;
	msg.msg_controllen = sizeof(cmsgbuf);

	cmptr = CMSG_FIRSTHDR(&msg);
	cmptr->cmsg_len = CMSG_LEN(sizeof(int));
	cmptr->cmsg_level = SOL_SOCKET;
	cmptr->cmsg_type = SCM_RIGHTS;
	memcpy(CMSG_DATA(cmptr), &socket_to_send, sizeof(socket_to_send));

	if (sendmsg(sockfd, &msg, 0) != sizeof(dummy)) {
		fprintf(stderr, "Error sending reply: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
};

static int try_connect_session(const struct sockaddr_un* addr) {
	int client_socket = -1;

	client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (client_socket < 0) {
		goto err;
	}

	if (connect(client_socket, (struct sockaddr*)addr, sizeof(*addr)) != 0) {
		goto err;
	}

	return client_socket;

err:
	if (client_socket >= 0) {
		close(client_socket);
	}
	return -1;
};

static void run_session_manager(unsigned int uid, unsigned int gid) {
	const char* login = NULL;
	struct passwd* pw = NULL;
	char* homedirTmp = NULL;
	mach_port_t subset = MACH_PORT_NULL;
	mach_port_t dummyService = MACH_PORT_NULL;
	kern_return_t kr = KERN_SUCCESS;

	// we are the shellspawn instance for the user's session. this means we're in charge
	// of the user's session and are responsible for setting it up as macOS would.
	// we are essentially going to be doing the same kind of setup that LoginWindow
	// would do on macOS.

	// switch ourselves to run under the user's UID and GID (but under Darling, this is all fake anyways)
	setgid(gid);
	setuid(uid);

	// fix up env vars to what they should be
	pw = getpwuid(uid);
	if (pw != NULL)
		login = pw->pw_name;

	if (!login)
		login = getlogin();

	setenv("PATH", "/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin", 1);
	setenv("TMPDIR", "/private/tmp", 1);

	asprintf(&homedirTmp, "HOME=/Users/%s", login);
	putenv(homedirTmp);
	homedirTmp = NULL; // `putenv()` assumes ownership of the string

	//
	// IMPORTANT NOTE
	//
	// okay, so i have no clue how this whole session switching is supposed to work.
	// or rather, i do have *some* idea: we need to call `create_and_switch_to_per_session_launchd`
	// to switch to a per-user launchd (which will also trigger LaunchAgents for the user).
	// the problem is that the way this function works internally is that it calls `_vprocmgr_move_subset_to_user`.
	// this function, in turn, moves the current subset into the new per-user launchd session.
	// obviously, this means we must already have a subset prior to calling `create_and_switch_to_per_session_launchd`.
	//
	// the thing is, i can't see how (the modern version of) LoginWindow does this. maybe the API has changed since then
	// and it no longer moves a subset into the new session, but i can't see it creating a new subset at any point during
	// the session setup. but clearly, *we* need to do this, so let's go ahead and do so.
	//

	if ((kr = bootstrap_subset(bootstrap_port, mach_task_self(), &subset)) != KERN_SUCCESS) {
		fprintf(stderr, "Failed to create bootstrap subset: %d\n", kr);
		exit(EXIT_FAILURE);
	}

	// replace the bootstrap port with our subset port
	if ((kr = task_set_bootstrap_port(mach_task_self(), subset)) != KERN_SUCCESS) {
		fprintf(stderr, "Failed to replace bootstrap port: %d\n", kr);
		exit(EXIT_FAILURE);
	}

	// we no longer need the old bootstrap port
	mach_port_deallocate(mach_task_self(), bootstrap_port);

	bootstrap_port = subset;

	//
	// ANOTHER IMPORTANT NOTE
	//
	// launchd requires the new subset to have at least one Mach service registered before switching to the per-user
	// session. to that end, we simply create a bogus service here.
	//
	if ((kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &dummyService)) != KERN_SUCCESS) {
		fprintf(stderr, "Failed to allocate dummy service port: %d\n", kr);
		exit(EXIT_FAILURE);
	}

	if ((kr = mach_port_insert_right(mach_task_self(), dummyService, dummyService, MACH_MSG_TYPE_MAKE_SEND)) != KERN_SUCCESS) {
		fprintf(stderr, "Failed to insert send right into dummy service port: %d\n", kr);
		exit(EXIT_FAILURE);
	}

	if ((kr = bootstrap_register(bootstrap_port, "org.darlinghq.shellsession.dummy-service", dummyService)) != KERN_SUCCESS) {
		fprintf(stderr, "Failed to register dummy service: %d\n", kr);
		exit(EXIT_FAILURE);
	}

	// set up a launchd session
	if (create_and_switch_to_per_session_launchd("shellspawn", LAUNCH_GLOBAL_ON_DEMAND) < 0) {
		fprintf(stderr, "Failed to set up launchd user session.\n");
		exit(EXIT_FAILURE);
	}

	// unset the global-on-demand flag to kickstart jobs that need to run at load
	if (!_vproc_set_global_on_demand(false)) {
		fprintf(stderr, "Failed to unset global-on-demand flag.\n");
		exit(EXIT_FAILURE);
	}

	// now we just need to stay alive for launchd to keep our session alive
	// (launchd will start shellspawn as a LaunchAgent, and that's what the `darling` command will actually send commands to)
	while (true) {
		pause();
	}
};
