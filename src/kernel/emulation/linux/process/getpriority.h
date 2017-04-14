#ifndef LINUX_GETPRIORITY_H
#define LINUX_GETPRIORITY_H

#include <sys/_types.h>
#include <sys/_types/_id_t.h>
#include "../errno.h"
#include "../base.h"
#include <linux-syscalls/linux.h>

int sys_getpriority(int which, id_t who);

// Same on BSD and Linux
#define	PRIO_PROCESS	0
#define	PRIO_PGRP	1
#define	PRIO_USER	2

// Unique to Darwin
#define	PRIO_DARWIN_THREAD	3
#define	PRIO_DARWIN_PROCESS	4

// Same on BSD and Linux
#define	PRIO_MIN	-20
#define	PRIO_MAX	20

// Unique to Darwin
#define PRIO_DARWIN_BG 0x1000
#define PRIO_DARWIN_NONUI 0x1001

#endif
