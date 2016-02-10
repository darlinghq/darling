#ifndef LINUX_SIGALTSTACK_H
#define LINUX_SIGALTSTACK_H

struct bsd_stack
{
	void* ss_sp;
	int ss_size;
	int ss_flags;
};

struct linux_stack
{
	void* ss_sp;
	int ss_flags;
	unsigned long ss_size;
};

long sys_sigaltstack(const struct bsd_stack* ss, struct bsd_stack* oss);

#endif

