#include "setjmp.h"
#include <signal.h>
#include <cstring>
#include <cstdio>

void __darwin_savesigs(darwin_sigjmp_buf env)
{
	__darwin_sigprocmask(SIG_BLOCK, nullptr, &env->sigset);
	sigaltstack(nullptr, &env->stack);
}

void __darwin_restoresigs(darwin_sigjmp_buf env)
{
	if (env->stack.ss_flags != 0)
	{
		__darwin_sigprocmask(SIG_SETMASK, &env->sigset, nullptr);
		sigaltstack(&env->stack, nullptr);
	}
}

