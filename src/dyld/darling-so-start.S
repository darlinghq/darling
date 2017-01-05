#include <asm/unistd.h>
#include "darling-config.h"

.text

// For running executables from inside the prefix
dyld_path:
.ascii SYSTEM_ROOT
.string DYLD_PATH

// Additionally, we support running them from the host system
dyld_host_path:
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
        leaq    -512(%rsp), %rsi    // buffer
        movl    $255, %edx          // buffer size
        movl    $__NR_readlink, %eax
        syscall

        cmp     $0, %eax
        jl      fail

        movb    $0, (%rsi, %rax, 1) // null terminate
        movq    8(%rsp), %rbx       // argv[0] to rbx

        // Append '!' to /proc/self/exe name
        // and then append original argv[0]
        xorl    %ecx, %ecx
        movb    $'!', (%rsi, %rax, 1)
        incl    %eax

strcat_start:
        movb    (%rbx, %rcx, 1), %dl
        movb    %dl, (%rsi, %rax, 1)
        movq    %rsi, 8(%rsp)       // orig argv[0]
        testb   %dl, %dl
        jz      strcat_end
        incl    %ecx
        incl    %eax
        jmp     strcat_start
strcat_end:
        movq    %rsi, 8(%rsp)

        // Execute self through dyld
	leaq	dyld_path(%rip), %rdi	// exec path
	movq	%rsp, %rsi				// argv
	movl	0(%rsp), %ecx			// argc
	leaq	16(%rsp, %rcx, 8), %rdx	// envp
	movq	%rdi, (%rsi)			// overwrite argc with dyld_path

	movl	$__NR_execve, %eax
	syscall

        // If we failed to find dyld under the system root,
        // try again without it
	leaq	dyld_host_path(%rip), %rdi	// exec path
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
