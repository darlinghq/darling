#ifndef _32IN64_H
#define _32IN64_H

#ifdef __x86_64__

	// Jump to a 32-bit address through segment number 0x23 (32-bit). The 64-bit segment is 0x33.
	// Set DS to 0x2B.
#define _64TO32_WITH_STACK(sp, addr) __asm__ volatile("movq %1, %%rsp;" \
												"subq $12, %%rsp;" \
												/*"movq %0, %%rax;"*/ \
												"movl %0, 8(%%rsp);" \
												"movl $0x23, 4(%%rsp);" \
												"leaq 1f, %%rax;" \
												"movl %%eax, (%%rsp);" \
												"lret;" \
												".code32;\n" \
												"1: push $0x2b;" \
												"pop %%ds;" \
												"ret;" \
												:: "r"((uint32_t)addr), "r"(sp) : "rax")

#endif
#endif

