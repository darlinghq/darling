#include <asm/unistd.h>
#include "darling-config.h"

.text

dyld_path:
.string DYLD_PATH

.globl _start
_start:
	leaq	dyld_path(%rip), %rdi	// exec path
	movq	%rsp, %rsi				// argv
	movl	0(%rsp), %ecx			// argc
	leaq	16(%rsp, %rcx, 8), %rdx	// envp
	movq	%rdi, (%rsi)			// overwrite argc with dyld_path

	movl	$__NR_execve, %eax
	syscall
	ud2

