.text
.globl _sig_restorer
.private_extern _sig_restorer

#ifdef __x86_64__
_sig_restorer:
	movl $15, %eax
	syscall

#elif defined(__i386__)

_sig_restorer:
	movl $173, %eax
	int $0x80
#endif

