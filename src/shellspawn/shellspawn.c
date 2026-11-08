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
#include "shellspawn-fds.h"
#include "shellspawn-config.h"
#include "duct_signals.h"

#define DBG 0

int g_serverSocket = -1;
struct sigaction sigchld_oldaction;

void setupSocket(void);
void listenForConnections(void);
void spawnShell(int fd);
void setupSigchild(void);
void restoreSigchild(void);
void reapAll(void);

int main(int argc, const char** argv)
{
	// Clients may disconnect at any time; writing to their socket must fail
	// with EPIPE instead of killing shellspawn. The spawned program gets the
	// default disposition back before exec.
	signal(SIGPIPE, SIG_IGN);

	// shellspawn (daemon) --fork()--> shellspawn (child) --fork()--> exec /bin/bash
	// in order to read the exit status of the shell process,
	// we have to allow it to become a zombie, therefore we need to
	// restore the sigaction of SIGCHLD of the child shellspawn
	setupSigchild();

	// The darling launcher waits for the socket before it sends the first
	// command, so binding it only after the system configuration has arrived
	// makes every program started through shellspawn see the configuration.
	waitForSystemConfig();
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
			restoreSigchild();
			fcntl(sock, F_SETFD, FD_CLOEXEC);
			spawnShell(sock);
			exit(EXIT_SUCCESS);
		}
		else
		{
			close(sock);
		}
	}
}

static pid_t get_foreground_pgrp(int ctty_fd, pid_t shell_pid)
{
	pid_t fg = -1;
	if (ctty_fd != -1)
	{
		fg = tcgetpgrp(ctty_fd);
		if (fg > 1)
			return fg;
	}

	if (shell_pid > 1)
	{
		char stat_path[64];
		snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", shell_pid);
		FILE *f = fopen(stat_path, "r");
		if (f)
		{
			char buf[512];
			if (fgets(buf, sizeof(buf), f))
			{
				char *rparen = strrchr(buf, ')');
				if (rparen)
				{
					char state;
					int ppid, pgrp, session, tty_nr, tpgid;
					if (sscanf(rparen + 1, " %c %d %d %d %d %d", &state, &ppid, &pgrp, &session, &tty_nr, &tpgid) == 6)
					{
						if (tpgid > 1)
							fg = tpgid;
					}
				}
			}
			fclose(f);
		}
	}
	return fg;
}

void spawnShell(int fd)
{
	pid_t shell_pid = -1;
	int shellfd[3] = { -1, -1, -1 };
	int pipefd[2] = { -1, -1 };
	int ctty_fd = -1;
	int rv;
	struct pollfd pfd[2];
	char** argv = NULL;
	int argc = 2;
	struct msghdr msg;
	struct iovec iov;
	char cmsgbuf[CMSG_SPACE(sizeof(int)) * 3];
	int kq;
	int wstatus = 0;

	bool read_cmds = true;

	argv = (char**) malloc(sizeof(char*) * 3);
	argv[0] = "/bin/bash";
	argv[1] = "--login";

	char* alloc_exec = NULL;

	// Keep the control socket out of the way of the standard I/O descriptors
	// set up for the shell.
	int moved_fd = shellspawn_move_above_stdio(fd);
	if (moved_fd == -1)
		goto err;
	fd = moved_fd;

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
					// NAME=value with a non-empty name
					char* equals = strchr(param, '=');
					if (equals != NULL && equals != param)
					{
						if (DBG) printf("set env: %s\n", param);
						putenv(param);
					}
					else
					{
						free(param);
					}
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

	for (int i = 0; i < 2; i++)
	{
		int moved = shellspawn_move_above_stdio(pipefd[i]);
		if (moved == -1)
			goto err;
		pipefd[i] = moved;
	}

	shell_pid = fork();
	if (shell_pid == -1)
		goto err;

	if (shell_pid == 0)
	{
		setsid();
		setpgrp();

		close(pipefd[0]);
		fcntl(pipefd[1], F_SETFD, FD_CLOEXEC);

		if (shellspawn_remap_stdio(shellfd) == 0)
		{
			ioctl(STDIN_FILENO, TIOCSCTTY, 0);

			close(fd);
			shellspawn_close_streams(shellfd, STDERR_FILENO + 1);

			signal(SIGPIPE, SIG_DFL);

			// In future, we may support spawning something else than Bash
			// and check the provided shell against /etc/shells
			execv(alloc_exec ? alloc_exec : "/bin/bash", argv);
		}

		rv = errno;
		write(pipefd[1], &rv, sizeof(rv));
		close(pipefd[1]);

		exit(EXIT_FAILURE);
	}

	if (isatty(shellfd[0]))
	{
		ctty_fd = fcntl(shellfd[0], F_DUPFD_CLOEXEC, 0);
	}

	shellspawn_close_streams(shellfd, 0);

	if (alloc_exec)
	{
		free(alloc_exec);
		alloc_exec = NULL;
	}

	// Check that exec succeeded
	close(pipefd[1]); // close the write end
	pipefd[1] = -1;
	if (read(pipefd[0], &rv, sizeof(rv)) == sizeof(rv))
	{
		errno = rv;
		fprintf(stderr, "shellspawn: execv failed: %s (errno=%d)\n", strerror(errno), errno);
		goto err;
	}
	close(pipefd[0]);
	pipefd[0] = -1;

	if (DBG) { puts("shellspawn: execv succeeded, entering poll loop"); fflush(stdout); }

	struct pollfd pfd_in[1];
	pfd_in[0].fd = fd;
	pfd_in[0].events = POLLIN;

	while (true)
	{
		int chk_wstatus = 0;
		pid_t wpid = waitpid(shell_pid, &chk_wstatus, WNOHANG);
		if (wpid == shell_pid)
		{
			wstatus = chk_wstatus;
			goto done_reaped;
		}
		else if (wpid < 0 && errno != EINTR)
		{
			wstatus = 0;
			goto done_reaped;
		}

		int pret = poll(pfd_in, 1, 50);
		if (pret < 0)
		{
			if (errno == EINTR)
				continue;
			break;
		}

		if (pret > 0 && (pfd_in[0].revents & (POLLIN | POLLHUP | POLLERR)))
		{
			if (pfd_in[0].revents & POLLIN)
			{
				struct shellspawn_cmd cmd;

				if (read(fd, &cmd, sizeof(cmd)) != sizeof(cmd))
				{
					if (DBG) { puts("Cannot read cmd (EOF or error)"); fflush(stdout); }
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
							pid_t fg_pgrp = get_foreground_pgrp(ctty_fd, shell_pid);
							if (fg_pgrp > 1)
							{
								if (DBG) printf("fg_pgrp = %d\n", fg_pgrp);
								if (kill(-fg_pgrp, darwin_signal) == -1 && errno == ESRCH)
								{
									if (shell_pid > 1)
										kill(-shell_pid, darwin_signal);
								}
							}
							else if (shell_pid > 1)
							{
								kill(-shell_pid, darwin_signal);
							}
						}

						break;
					}
					default:
						goto err;
				}
			}
			else
			{
				// Client disconnected
				break;
			}
		}
	}

done:
	// Reap the child
	if (waitpid(shell_pid, &wstatus, 0) != shell_pid)
	{
		wstatus = 0;
	}

done_reaped:
	if (WIFEXITED(wstatus))
		wstatus = WEXITSTATUS(wstatus);
	else if (WIFSIGNALED(wstatus))
		wstatus = 128 + WTERMSIG(wstatus);
	else
		wstatus = 0;
	
	// Report exit code back to the client
	write(fd, &wstatus, sizeof(int));

	if (DBG) printf("Shell terminated with exit code %d\n", wstatus);
	close(fd);
	if (ctty_fd != -1)
	{
		close(ctty_fd);
		ctty_fd = -1;
	}

	reapAll();
	return;
err:
	if (DBG) fprintf(stderr, "Error spawning shell: %s\n", strerror(errno));

	if (ctty_fd != -1)
		close(ctty_fd);

	for (int i = 0; i < 2; i++)
	{
		if (pipefd[i] != -1)
			close(pipefd[i]);
	}

	shellspawn_close_streams(shellfd, 0);

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
	sigaction(SIGCHLD, &sigchld_action, &sigchld_oldaction);
}

void restoreSigchild(void)
{
	sigaction(SIGCHLD, &sigchld_oldaction, NULL);
}

void reapAll(void)
{
    while (waitpid((pid_t)(-1), 0, WNOHANG) > 0);
}
