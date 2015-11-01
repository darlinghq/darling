#ifndef LINUX_WAITID_H
#define LINUX_WAITID_H

struct simple_siginfo
{
	int si_signo;
	int si_errno;
	int si_code;
};

long sys_waitid(int idtype, int id, struct simple_siginfo* si, int options);

#endif

