#ifndef LINUX_POLL_H
#define LINUX_POLL_H

struct pollfd;
long sys_poll(struct pollfd* fds, unsigned int nfds, int timeout);
long sys_poll_nocancel(struct pollfd* fds, unsigned int nfds, int timeout);

#endif

