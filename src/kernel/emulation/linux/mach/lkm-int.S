.globl _lkm_call

#if defined(__x86_64__)

_lkm_call:
	movq %rdi, %rax
	movq %rsi, %rcx
	int $0x83
	ret
	
#elif defined(__i386__)

_lkm_call:
	movl 4(%esp), %eax
	movl 8(%esp), %ecx
	int $0x83
	ret

#else
#error Missing implementation
#endif

