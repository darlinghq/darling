#include <asm/unistd.h>
#include "darling-config.h"

.text

dyld_path:
.string DYLD_PATH

proc_self_exe:
.string "/proc/self/exe"

failure_msg:
.string "Failed to execute self through Darling dyld.\n"
failure_msg_end:

.globl _start
_start:
        // Get path to self
        leaq    proc_self_exe(%rip), %rdi
        leaq    -256(%rsp), %rsi    // buffer
        movl    $255, %edx          // buffer size
        movl    $__NR_readlink, %eax
        syscall

        cmp     $0, %eax
        jl      fail

        movb    $0, (%rsi, %rax, 1) // null terminate
        movq    %rsi, 8(%rsp)       // orig argv[0]

        // Execute self through dyld
	leaq	dyld_path(%rip), %rdi	// exec path
	movq	%rsp, %rsi				// argv
	movl	0(%rsp), %ecx			// argc
	leaq	16(%rsp, %rcx, 8), %rdx	// envp
	movq	%rdi, (%rsi)			// overwrite argc with dyld_path

	movl	$__NR_execve, %eax
	syscall

fail:
        movl    $2, %edi
        leaq    failure_msg(%rip), %rsi
        movl    $(failure_msg_end - failure_msg), %edx
        movl    $__NR_write, %eax
        syscall

	ud2

