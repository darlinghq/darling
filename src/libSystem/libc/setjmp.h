#ifndef LIBC_SETJMP_H
#define LIBC_SETJMP_H
#include <stdint.h>
#include "signals.h"

//#pragma pack (1)
#if defined(__x86_64__)

struct darwin_jmp_buf_t
{
	int64_t rbp, rbx, r12, r13, r14, r15, rip, rsp;
	int32_t mxcsr;
	int16_t fpstate;
	__darwin_sigset_t sigset;
	stack_t stack;
};

#elif defined(__i386__)

struct darwin_jmp_buf_t
{
	int32_t edi, esi, ebx, ebp, eip, esp;
	int32_t mxcsr;
	int16_t fpstate;
	__darwin_sigset_t sigset;
	stack_t stack;
};

#else
#	error Unsupported platform!
#endif

typedef darwin_jmp_buf_t* darwin_jmp_buf;
typedef darwin_jmp_buf darwin_sigjmp_buf;

extern "C" {

// defined in assembly
int __darwin_setjmp(darwin_jmp_buf buf);
int __darwin_sigsetjmp(darwin_sigjmp_buf buf, int savesigs);
void __darwin_longjmp(darwin_jmp_buf env, int val);
void __darwin_siglongjmp(darwin_sigjmp_buf env, int val);

// internal calls
void __darwin_savesigs(darwin_sigjmp_buf env);
void __darwin_restoresigs(darwin_sigjmp_buf env);

}

#endif

