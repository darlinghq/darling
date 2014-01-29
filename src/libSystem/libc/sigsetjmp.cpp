#include "setjmp.h"
#include <signal.h>
#include <cstring>
#include <cstdio>

void __darwin_savesigs(darwin_sigjmp_buf env)
{
	__darwin_sigprocmask(SIG_BLOCK, nullptr, &env->sigset);
#ifdef JMP_BUF_HAS_STACK
	sigaltstack(nullptr, &env->stack);
#endif
}

void __darwin_restoresigs(darwin_sigjmp_buf env)
{
#ifdef JMP_BUF_HAS_STACK
	if (env->stack.ss_flags != 0)
	{
		__darwin_sigprocmask(SIG_SETMASK, &env->sigset, nullptr);
		sigaltstack(&env->stack, nullptr);
	}
#else
	__darwin_sigprocmask(SIG_SETMASK, &env->sigset, nullptr);
#endif
}

