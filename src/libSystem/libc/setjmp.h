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

#define JMP_BUF_HAS_STACK

typedef darwin_jmp_buf_t* darwin_jmp_buf;
typedef darwin_jmp_buf darwin_sigjmp_buf;

#elif defined(__i386__)

struct darwin_jmp_buf_t
{
	int32_t edi, esi, ebx, ebp, eip, esp;
	int32_t mxcsr;
	int16_t fpstate;
	__darwin_sigset_t sigset;
	stack_t stack;
};

#define JMP_BUF_HAS_STACK

typedef darwin_jmp_buf_t* darwin_jmp_buf;
typedef darwin_jmp_buf darwin_sigjmp_buf;

#elif defined(__arm__)

struct darwin_jmp_buf_t
{
	int32_t r4, r5, r6, r7, r8, r10, r11;
	int32_t sp, lr;
	int32_t d8, d9, d10, d11, d12, d13, d14, d15;
	int32_t sig, fstmx;
	int32_t reserved;
};

struct darwin_sigjmp_buf_t
{
        int32_t r4, r5, r6, r7, r8, r10, r11;
        int32_t sp, lr;
        int32_t d8, d9, d10, d11, d12, d13, d14, d15;
        int32_t sig, fstmx;
        int32_t reserved;
	__darwin_sigset_t sigset;
};

typedef darwin_jmp_buf_t* darwin_jmp_buf;
typedef darwin_sigjmp_buf_t* darwin_sigjmp_buf;

#else
#	error Unsupported platform!
#endif

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

