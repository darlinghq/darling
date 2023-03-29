#ifndef _DARLING_COREDUMP_X86_64_H_
#define _DARLING_COREDUMP_X86_64_H_

#include <stdint.h>

struct nt_prstatus_registers_i386 {
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t ebp;
	uint32_t eax;
	uint16_t ds;
	uint16_t ds_padding;
	uint16_t es;
	uint16_t es_padding;
	uint16_t fs;
	uint16_t fs_padding;
	uint16_t gs;
	uint16_t gs_padding;
	uint32_t orig_eax;
	uint32_t eip;
	uint16_t cs;
	uint16_t cs_padding;
	uint32_t eflags;
	uint32_t esp;
	uint16_t ss;
	uint16_t ss_padding;
};

struct nt_prstatus_registers_x86_64 {
	unsigned long r15;
	unsigned long r14;
	unsigned long r13;
	unsigned long r12;
	unsigned long bp;
	unsigned long bx;
	unsigned long r11;
	unsigned long r10;
	unsigned long r9;
	unsigned long r8;
	unsigned long ax;
	unsigned long cx;
	unsigned long dx;
	unsigned long si;
	unsigned long di;
	unsigned long orig_ax;
	unsigned long ip;
	unsigned long cs;
	unsigned long flags;
	unsigned long sp;
	unsigned long ss;
	unsigned long fs_base;
	unsigned long gs_base;
	unsigned long ds;
	unsigned long es;
	unsigned long fs;
	unsigned long gs;
};

typedef struct x86_thread_state32 {
	unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;
	unsigned int edi;
	unsigned int esi;
	unsigned int ebp;
	unsigned int esp;
	unsigned int ss;
	unsigned int eflags;
	unsigned int eip;
	unsigned int cs;
	unsigned int ds;
	unsigned int es;
	unsigned int fs;
	unsigned int gs;
} x86_thread_state32_t;

struct fpu_mmst {
	char mmst_reg[10];
	char mmst_rsrv[6];
};

struct fpu_xmm {
	char xmm_reg[16];
};

struct fpu_control {
	unsigned short
		invalid: 1,
		denorm: 1,
		zdiv: 1,
		ovrfl: 1,
		undfl: 1,
		precis: 1,
		: 2,
		pc: 2,
		rc: 2,
		/*inf*/: 1,
		:3;
};

struct fpu_status {
	unsigned short
		invalid: 1,
		denorm: 1,
		zdiv: 1,
		ovrfl: 1,
		undfl: 1,
		precis: 1,
		stkflt: 1,
		errsumm: 1,
		c0: 1,
		c1: 1,
		c2: 1,
		tos: 3,
		c3: 1,
		busy: 1;
};

typedef struct x86_float_state32 {
	int fpu_reserved[2];
	struct fpu_control fpu_fcw;
	struct fpu_status fpu_fsw;
	uint8_t fpu_ftw;
	uint8_t fpu_rsrv1;
	uint16_t fpu_fop;
	uint32_t fpu_ip;
	uint16_t fpu_cs;
	uint16_t fpu_rsrv2;
	uint32_t fpu_dp;
	uint16_t fpu_ds;
	uint16_t fpu_rsrv3;
	uint32_t fpu_mxcsr;
	uint32_t fpu_mxcsrmask;
	struct fpu_mmst fpu_stmm0;
	struct fpu_mmst fpu_stmm1;
	struct fpu_mmst fpu_stmm2;
	struct fpu_mmst fpu_stmm3;
	struct fpu_mmst fpu_stmm4;
	struct fpu_mmst fpu_stmm5;
	struct fpu_mmst fpu_stmm6;
	struct fpu_mmst fpu_stmm7;
	struct fpu_xmm fpu_xmm0;
	struct fpu_xmm fpu_xmm1;
	struct fpu_xmm fpu_xmm2;
	struct fpu_xmm fpu_xmm3;
	struct fpu_xmm fpu_xmm4;
	struct fpu_xmm fpu_xmm5;
	struct fpu_xmm fpu_xmm6;
	struct fpu_xmm fpu_xmm7;
	char fpu_rsrv4[14 * 16];
	int fpu_reserved1;
} x86_float_state32_t;

typedef struct x86_thread_state64 {
	uint64_t rax;
	uint64_t rbx;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rbp;
	uint64_t rsp;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	uint64_t rip;
	uint64_t rflags;
	uint64_t cs;
	uint64_t fs;
	uint64_t gs;
} x86_thread_state64_t;

typedef struct x86_float_state64 {
	int fpu_reserved[2];
	struct fpu_control fpu_fcw;
	struct fpu_status fpu_fsw;
	uint8_t fpu_ftw;
	uint8_t fpu_rsrv1;
	uint16_t fpu_fop;
	uint32_t fpu_ip;
	uint16_t fpu_cs;
	uint16_t fpu_rsrv2;
	uint32_t fpu_dp;
	uint16_t fpu_ds;
	uint16_t fpu_rsrv3;
	uint32_t fpu_mxcsr;
	uint32_t fpu_mxcsrmask;
	struct fpu_mmst fpu_stmm0;
	struct fpu_mmst fpu_stmm1;
	struct fpu_mmst fpu_stmm2;
	struct fpu_mmst fpu_stmm3;
	struct fpu_mmst fpu_stmm4;
	struct fpu_mmst fpu_stmm5;
	struct fpu_mmst fpu_stmm6;
	struct fpu_mmst fpu_stmm7;
	struct fpu_xmm fpu_xmm0;
	struct fpu_xmm fpu_xmm1;
	struct fpu_xmm fpu_xmm2;
	struct fpu_xmm fpu_xmm3;
	struct fpu_xmm fpu_xmm4;
	struct fpu_xmm fpu_xmm5;
	struct fpu_xmm fpu_xmm6;
	struct fpu_xmm fpu_xmm7;
	struct fpu_xmm fpu_xmm8;
	struct fpu_xmm fpu_xmm9;
	struct fpu_xmm fpu_xmm10;
	struct fpu_xmm fpu_xmm11;
	struct fpu_xmm fpu_xmm12;
	struct fpu_xmm fpu_xmm13;
	struct fpu_xmm fpu_xmm14;
	struct fpu_xmm fpu_xmm15;
	char fpu_rsrv4[6 * 16];
	int fpu_reserved1;
} x86_float_state64_t;

#define x86_THREAD_STATE32 1
#define x86_FLOAT_STATE32 2
#define x86_THREAD_STATE64		4
#define x86_FLOAT_STATE64		5

#define x86_THREAD_STATE32_COUNT ((uint32_t)(sizeof(x86_thread_state32_t) / sizeof(int)))
#define x86_FLOAT_STATE32_COUNT ((uint32_t)(sizeof(x86_float_state32_t) / sizeof(unsigned int)))


#define x86_THREAD_STATE64_COUNT ((uint32_t)(sizeof(x86_thread_state64_t) / sizeof(int)))
#define x86_FLOAT_STATE64_COUNT ((uint32_t)(sizeof(x86_float_state64_t) / sizeof(unsigned int)))

#endif // _DARLING_COREDUMP_X86_64_H_
