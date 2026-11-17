/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

/* What gles-functions.c, the generated GL entry points, is made of. */

#include <stdint.h>
#include <string.h>

#include <OpenGLES/gltypes.h>

/* A host function, found the first time it is called. */
struct gles_slot {
	void *address;
	const char *name;
};

void *gles_resolve(struct gles_slot *slot);

#ifdef GLES_FUNCTIONS_C
#if defined(__arm64__)
/* Saves the argument registers around gles_resolve, then jumps to the
 * function: x16 holds the slot. */
__asm__(
	".text\n"
	".p2align 2\n"
	".private_extern _gles_trampoline_common\n"
	"_gles_trampoline_common:\n"
	"\tldr x17, [x16]\n"
	"\tcbz x17, 1f\n"
	"\tbr x17\n"
	"1:\tstp x29, x30, [sp, #-16]!\n"
	"\tmov x29, sp\n"
	"\tsub sp, sp, #208\n"
	"\tstp x0, x1, [sp]\n"
	"\tstp x2, x3, [sp, #16]\n"
	"\tstp x4, x5, [sp, #32]\n"
	"\tstp x6, x7, [sp, #48]\n"
	"\tstp x8, xzr, [sp, #64]\n"
	"\tstp q0, q1, [sp, #80]\n"
	"\tstp q2, q3, [sp, #112]\n"
	"\tstp q4, q5, [sp, #144]\n"
	"\tstp q6, q7, [sp, #176]\n"
	"\tmov x0, x16\n"
	"\tbl _gles_resolve\n"
	"\tmov x17, x0\n"
	"\tldp x0, x1, [sp]\n"
	"\tldp x2, x3, [sp, #16]\n"
	"\tldp x4, x5, [sp, #32]\n"
	"\tldp x6, x7, [sp, #48]\n"
	"\tldr x8, [sp, #64]\n"
	"\tldp q0, q1, [sp, #80]\n"
	"\tldp q2, q3, [sp, #112]\n"
	"\tldp q4, q5, [sp, #144]\n"
	"\tldp q6, q7, [sp, #176]\n"
	"\tmov sp, x29\n"
	"\tldp x29, x30, [sp], #16\n"
	"\tbr x17\n");
#elif defined(__x86_64__)
/* The same on x86_64, where the guest and the host share the System V
 * calling convention: saves the six integer argument registers, the vector
 * count that variadic callers pass in al and xmm0-7 around gles_resolve,
 * then jumps to the function. r11 holds the slot. */
__asm__(
	".text\n"
	".p2align 4\n"
	".private_extern _gles_trampoline_common\n"
	"_gles_trampoline_common:\n"
	"\tmovq (%r11), %r10\n"
	"\ttestq %r10, %r10\n"
	"\tjz 1f\n"
	"\tjmp *%r10\n"
	"1:\tpushq %rbp\n"
	"\tmovq %rsp, %rbp\n"
	"\tsubq $192, %rsp\n"
	"\tmovq %rdi, 0(%rsp)\n"
	"\tmovq %rsi, 8(%rsp)\n"
	"\tmovq %rdx, 16(%rsp)\n"
	"\tmovq %rcx, 24(%rsp)\n"
	"\tmovq %r8, 32(%rsp)\n"
	"\tmovq %r9, 40(%rsp)\n"
	"\tmovq %rax, 48(%rsp)\n"
	"\tmovdqu %xmm0, 64(%rsp)\n"
	"\tmovdqu %xmm1, 80(%rsp)\n"
	"\tmovdqu %xmm2, 96(%rsp)\n"
	"\tmovdqu %xmm3, 112(%rsp)\n"
	"\tmovdqu %xmm4, 128(%rsp)\n"
	"\tmovdqu %xmm5, 144(%rsp)\n"
	"\tmovdqu %xmm6, 160(%rsp)\n"
	"\tmovdqu %xmm7, 176(%rsp)\n"
	"\tmovq %r11, %rdi\n"
	"\tcall _gles_resolve\n"
	"\tmovq %rax, %r10\n"
	"\tmovq 0(%rsp), %rdi\n"
	"\tmovq 8(%rsp), %rsi\n"
	"\tmovq 16(%rsp), %rdx\n"
	"\tmovq 24(%rsp), %rcx\n"
	"\tmovq 32(%rsp), %r8\n"
	"\tmovq 40(%rsp), %r9\n"
	"\tmovq 48(%rsp), %rax\n"
	"\tmovdqu 64(%rsp), %xmm0\n"
	"\tmovdqu 80(%rsp), %xmm1\n"
	"\tmovdqu 96(%rsp), %xmm2\n"
	"\tmovdqu 112(%rsp), %xmm3\n"
	"\tmovdqu 128(%rsp), %xmm4\n"
	"\tmovdqu 144(%rsp), %xmm5\n"
	"\tmovdqu 160(%rsp), %xmm6\n"
	"\tmovdqu 176(%rsp), %xmm7\n"
	"\tmovq %rbp, %rsp\n"
	"\tpopq %rbp\n"
	"\tjmp *%r10\n");
#else
#error "OpenGLES is built for x86_64 and arm64 only"
#endif
#endif

#define GLES_SLOT(name) \
	__attribute__((visibility("hidden"), used)) struct gles_slot gles_slot_##name = { 0, #name };

#if defined(__arm64__)
#define GLES_TRAMPOLINE(name) \
	__asm__(".text\n.p2align 2\n.globl _" #name "\n_" #name ":\n" \
		"\tadrp x16, _gles_slot_" #name "@PAGE\n" \
		"\tadd x16, x16, _gles_slot_" #name "@PAGEOFF\n" \
		"\tb _gles_trampoline_common\n");
#else
#define GLES_TRAMPOLINE(name) \
	__asm__(".text\n.p2align 4\n.globl _" #name "\n_" #name ":\n" \
		"\tleaq _gles_slot_" #name "(%rip), %r11\n" \
		"\tjmp _gles_trampoline_common\n");
#endif

/* A stack argument as the host reads it: in the low bytes of 8. */
static inline uint64_t gles_stack_gp(const void *value, size_t size)
{
	uint64_t slot = 0;
	memcpy(&slot, value, size);
	return slot;
}

static inline double gles_stack_fp(const void *value, size_t size)
{
	double slot = 0;
	memcpy(&slot, value, size);
	return slot;
}
