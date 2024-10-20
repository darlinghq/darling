/*
This file is part of Darling.

Copyright (C) 2017 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/event.h>
#include <sys/ioctl.h>
#include <signal.h>
#include "shellspawn.h"
#include "duct_signals.h"

#define DBG 0

int g_serverSocket = -1;

void setupSocket(void);
void listenForConnections(void);
void spawnShell(int fd);
void setupSigchild(void);
void reapAll(void);

int main(int argc, const char** argv)
{
	// in order to read the exit status of the process,
	// we have to allow it to become a zombie, which is prevented
	// when we set the SIGCHLD signal to SA_NOCLDWAIT
	//setupSigchild();
	setupSocket();
	listenForConnections();

	if (g_serverSocket != -1)
		close(g_serverSocket);
	return 0;
}

void setupSocket(void)
{
	struct sockaddr_un addr = {
		.sun_family = AF_UNIX,
		.sun_path = SHELLSPAWN_SOCKPATH
	};

	g_serverSocket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (g_serverSocket == -1)
	{
		perror("Creating unix socket");
		exit(EXIT_FAILURE);
	}

	fcntl(g_serverSocket, F_SETFD, FD_CLOEXEC);
	unlink(SHELLSPAWN_SOCKPATH);

	if (bind(g_serverSocket, (struct sockaddr*) &addr, sizeof(addr)) == -1)
	{
		perror("Binding the unix socket");
		exit(EXIT_FAILURE);
	}

	chmod(addr.sun_path, 0600);

	if (listen(g_serverSocket, 16384) == -1)
	{
		perror("Listening on unix socket");
		exit(EXIT_FAILURE);
	}
}

void listenForConnections(void)
{
	int sock;
	struct sockaddr_un addr;
	socklen_t len = sizeof(addr);

	while (true)
	{
		sock = accept(g_serverSocket, (struct sockaddr*) &addr, &len);
		if (sock == -1)
			break;

		if (fork() == 0)
		{
			fcntl(sock, F_SETFD, FD_CLOEXEC);
			spawnShell(sock);
			exit(EXIT_SUCCESS);
		}
		else
		{
			close(sock);
 			reapAll();
		}
	}
}

void spawnShell(int fd)
{
	pid_t shell_pid = -1;
	int shellfd[3] = { -1, -1, -1 };
	int pipefd[2];
	int rv;
	struct pollfd pfd[2];
	char** argv = NULL;
	int argc = 2;
	struct msghdr msg;
	struct iovec iov;
	char cmsgbuf[CMSG_SPACE(sizeof(int)) * 3];
	int kq;

	bool read_cmds = true;

	argv = (char**) malloc(sizeof(char*) * 3);
	argv[0] = "/bin/bash";
	argv[1] = "--login";

	char* alloc_exec = NULL;

	// Read commands from client
	while (read_cmds)
	{
		struct shellspawn_cmd cmd;
		char* param = NULL;

		memset(&msg, 0, sizeof(msg));
		msg.msg_control = cmsgbuf;
		msg.msg_controllen = sizeof(cmsgbuf);

		iov.iov_base = &cmd;
		iov.iov_len = sizeof(cmd);
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;

		if (recvmsg(fd, &msg, 0) != sizeof(cmd))
		{
			if (DBG) puts("bad recvmsg");
			goto err;
		}

		if (cmd.data_length != 0)
		{
			param = (char*) malloc(cmd.data_length + 1);
			if (read(fd, param, cmd.data_length) != cmd.data_length)
				goto err;
			param[cmd.data_length] = '\0';
		}

		switch (cmd.cmd)
		{
			case SHELLSPAWN_ADDARG:
			{
				if (param != NULL)
				{
					argv = (char**) realloc(argv, sizeof(char*) * (argc + 1));
					argv[argc] = param;
					if (DBG) printf("add arg: %s\n", param);
					argc++;
				}
				break;
			}
			case SHELLSPAWN_SETENV:
			{
				if (param != NULL)
				{
					if (DBG) printf("set env: %s\n", param);
					putenv(param);
				}
				break;
			}
			case SHELLSPAWN_CHDIR:
			{
				if (param != NULL)
				{
					if (DBG) printf("chdir: %s\n", param);
					chdir(param);
					free(param);
				}
				break;
			}
			case SHELLSPAWN_GO:
			{
				struct cmsghdr *cmptr = CMSG_FIRSTHDR(&msg);

				if (cmptr == NULL)
				{
					if (DBG) puts("bad cmptr");
					goto err;
				}
				if (cmptr->cmsg_level != SOL_SOCKET
						|| cmptr->cmsg_type != SCM_RIGHTS)
				{
					if (DBG) puts("bad cmsg level/type");
					goto err;
				}
				if (cmptr->cmsg_len != CMSG_LEN(sizeof(int) * 3))
				{
					if (DBG) printf("bad cmsg_len: %d\n", cmptr->cmsg_len);
					goto err;
				}

				memcpy(shellfd, CMSG_DATA(cmptr), sizeof(int) * 3);

				if (DBG) printf("go, fds={ %d, %d, %d }\n", shellfd[0], shellfd[1], shellfd[2]);
				free(param);
				read_cmds = false;
				break;
			}
			case SHELLSPAWN_SETUIDGID:
			{
				int* ids = (int*) param;
				if (cmd.data_length < 2*sizeof(int))
				{
					free(param);
					break;
				}

				setuid(ids[0]);
				setgid(ids[1]);
				free(param);

				break;
			}
			case SHELLSPAWN_SETEXEC:
			{
				argc = 0;
				argv = realloc(argv, 0);
				alloc_exec = param;
				if (DBG) printf("setexec: %s\n", param);
				break;
			}
		}
	}

	// Add terminating NULL
	argv = (char**) realloc(argv, sizeof(char*) * (argc + 1));
	argv[argc] = NULL;

	if (pipe(pipefd) == -1)
		goto err;

	setsid();
	setpgrp();

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	dup2(shellfd[0], STDIN_FILENO);
	dup2(shellfd[1], STDOUT_FILENO);
	dup2(shellfd[2], STDERR_FILENO);

	ioctl(STDIN_FILENO, TIOCSCTTY, STDIN_FILENO);

	shell_pid = fork();
	if (shell_pid == 0)
	{
		close(fd);

		fcntl(pipefd[1], F_SETFD, FD_CLOEXEC);

		// In future, we may support spawning something else than Bash
		// and check the provided shell against /etc/shells
		execv(alloc_exec ? alloc_exec : "/bin/bash", argv);

		rv = errno;
		write(pipefd[1], &rv, sizeof(rv));
		close(pipefd[1]);

		exit(EXIT_FAILURE);
	}

	if (alloc_exec)
	{
		free(alloc_exec);
		alloc_exec = NULL;
	}

	// Check that exec succeeded
	close(pipefd[1]); // close the write end
	if (read(pipefd[0], &rv, sizeof(rv)) == sizeof(rv))
	{
		errno = rv;
		goto err;
	}
	close(pipefd[0]);

	// Now we start passing signals
	// and check for child process exit

	kq = kqueue();

	{
		struct kevent changes[2];
		EV_SET(&changes[0], fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		EV_SET(&changes[1], shell_pid, EVFILT_PROC, EV_ADD | EV_ENABLE, NOTE_EXIT, 0, NULL);

		if (kevent(kq, changes, 2, NULL, 0, NULL) == -1)
			goto err;
	}

	while (true)
	{
		struct kevent ev;

		if (kevent(kq, NULL, 0, &ev, 1, NULL) <= 0)
		{
			if (errno == EINTR) {
				if (DBG) puts("kevent call interrupted; continuing...");
				continue;
			}
			if (DBG) puts("kevent fail");
			goto err;
		}

		if (ev.filter == EVFILT_PROC && (ev.fflags & NOTE_EXIT))
		{
			if (DBG) puts("subprocess exit");
			break;
		}
		else if (ev.filter == EVFILT_READ)
		{
			struct shellspawn_cmd cmd;

			if (read(fd, &cmd, sizeof(cmd)) != sizeof(cmd))
			{
				if (DBG) puts("Cannot read cmd");
				break;
			}

			switch (cmd.cmd)
			{
				case SHELLSPAWN_SIGNAL:
				{
					int linux_signal, darwin_signal;

					if (cmd.data_length != sizeof(int))
						goto err;

					if (read(fd, &linux_signal, sizeof(int)) != sizeof(int))
						goto err;

					// Convert Linux signal number to Darwin signal number
					darwin_signal = signum_linux_to_bsd(linux_signal);
					if (DBG) printf("rcvd signal %d -> %d\n", linux_signal, darwin_signal);

					if (darwin_signal != 0)
					{
						int fg_pid = tcgetpgrp(shellfd[0]);
						if (fg_pid != -1)
						{
							if (DBG) printf("fg_pid = %d\n", fg_pid);
							kill(fg_pid, darwin_signal);
						}
						else
							kill(-shell_pid, darwin_signal);
					}

					break;
				}
				default:
					goto err;
			}
		}
	}

	// Kill the child process in case it's still running
	kill(shell_pid, SIGKILL);

	// Close shell fds
	for (int i = 0; i < 3; i++)
	{
		if (shellfd[i] != -1)
			close(shellfd[0]);
	}

	// Reap the child
	int wstatus;
	if (waitpid(shell_pid, &wstatus, 0) != shell_pid)
		perror("waitpid");
	wstatus = WEXITSTATUS(wstatus);
	
	// Report exit code back to the client
	write(fd, &wstatus, sizeof(int));

	if (DBG) printf("Shell terminated with exit code %d\n", wstatus);
	close(fd);

	reapAll();
	return;
err:
	if (DBG) fprintf(stderr, "Error spawning shell: %s\n", strerror(errno));

	for (int i = 0; i < 3; i++)
	{
		if (shellfd[i] != -1)
			close(shellfd[0]);
	}

	if (shell_pid != -1)
		kill(shell_pid, SIGKILL);

	close(fd);
	reapAll();
}

void setupSigchild(void)
{
	struct sigaction sigchld_action = {
		.sa_handler = SIG_DFL,
		.sa_flags = SA_NOCLDWAIT
	};
	sigaction(SIGCHLD, &sigchld_action, NULL);
}

void reapAll(void)
{
    while (waitpid((pid_t)(-1), 0, WNOHANG) > 0);
}
