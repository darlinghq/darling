.text
.globl _sig_restorer
.private_extern _sig_restorer

#ifdef __x86_64__
_sig_restorer:
	movl $15, %eax ; rt_sigreturn
	syscall

#elif defined(__i386__)

_sig_restorer:
	movl $173, %eax ; rt_sigreturn
	int $0x80

#elif defined(__arm64__)
_sig_restorer:
	mov w8,#139 ; __NR_rt_sigreturn
	svc #0

#else
#error "Missing assembly!"

#endif

