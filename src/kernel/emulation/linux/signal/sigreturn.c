#include "sigreturn.h"
#include "../base.h"
#include "../errno.h"

long sys_sigreturn()
{
	// Apple's libc invokes sigreturn directly,
	// but Linux injects a rt_sigreturn() syscall on its own.
	// Hance we must ignore this call.
	return 0;
}

