#if defined(__x86_64__)

.macro xtrace_hook name
	.text
	.globl _\name
	_\name\():
		.space 13, 0x90
		ret
.endm

#elif defined(__i386__)

.macro xtrace_hook name
	.text
	.globl _\name
	_\name\():
		.space 6, 0x90
		ret
.endm

#else

#error Missing xtrace hook dummy function macro for this architecture

#endif

// void _xtrace_thread_exit(void);
xtrace_hook _xtrace_thread_exit

// void _xtrace_execve_inject(const char*** envp_ptr);
xtrace_hook _xtrace_execve_inject

// void _xtrace_postfork_child(void);
xtrace_hook _xtrace_postfork_child
