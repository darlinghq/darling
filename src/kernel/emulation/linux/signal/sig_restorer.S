.text
.globl sig_restorer
.hidden sig_restorer

#ifdef __x86_64__
sig_restorer:
	movl $15, %eax
	syscall

#elif defined(__i386__)

sig_restorer:
	movl $173, %eax
	int $0x80
#endif

