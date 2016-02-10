#ifndef LINUX_SENDMSG_H
#define LINUX_SENDMSG_H
#include <stdint.h>
#include "recvmsg.h"

long sys_sendmsg(int socket, const struct bsd_msghdr* msg, int flags);
long sys_sendmsg_nocancel(int socket, const struct bsd_msghdr* msg, int flags);

#define LINUX_SYS_SENDMSG	16

#endif

