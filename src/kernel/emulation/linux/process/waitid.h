#ifndef LINUX_WAITID_H
#define LINUX_WAITID_H

#include <sys/signal.h>

#define LINUX_SI_MAX_SIZE        128
#if __WORDSIZE == 64
# define LINUX_SI_PAD_SIZE        ((LINUX_SI_MAX_SIZE / sizeof (int)) - 4)
#else
# define LINUX_SI_PAD_SIZE        ((LINUX_SI_MAX_SIZE / sizeof (int)) - 3)
#endif

// TODO: see when this is defind in Linux
#ifndef LINUX_SI_ERRNO_THEN_CODE
# define LINUX_SI_ERRNO_THEN_CODE        1
#endif

// This struct is heavily simplified.
typedef struct
{
    int si_signo;                			/* Signal number.  */
#if LINUX_SI_ERRNO_THEN_CODE
    int si_errno;                			/* If non-zero, an errno value associated with
                                   			this signal, as defined in <errno.h>.  */
    int si_code;                			/* Signal code.  */
#else
    int si_code;
    int si_errno;
#endif
#if __WORDSIZE == 64
    int __pad0;                        		/* Explicit padding.  */
#endif
    union
    {
    	int _pad[LINUX_SI_PAD_SIZE];
        /* kill().  */
        struct
		{
			pid_t si_pid;        			/* Sending process ID.  */
			uid_t si_uid;        			/* Real user ID of sending process.  */
		} _kill;
        /* POSIX.1b timers.  */
        struct
		{
			int si_tid;                		/* Timer ID.  */
			int si_overrun;        			/* Overrun count.  */
			union sigval si_sigval;			/* Signal value.  */
		} _timer;
        /* POSIX.1b signals.  */
        struct
		{
			pid_t si_pid;        			/* Sending process ID.  */
			uid_t si_uid;        			/* Real user ID of sending process.  */
			union sigval si_sigval; 		/* Signal value.  */
		} _rt;
        /* SIGCHLD.  */
        struct
		{
			pid_t si_pid;        				/* Which child.         */
			uid_t si_uid;        	/* Real user ID of sending process.  */
			int si_status;        			/* Exit value or signal.  */
		} _sigchld;
        /* SIGILL, SIGFPE, SIGSEGV, SIGBUS.  */
        struct
        {
			void *si_addr;            		/* Faulting insn/memory ref.  */
		} _sigfault;
        /* SIGPOLL.  */
        struct
		{
			long si_band;        			/* Band event for SIGPOLL.  */
			int si_fd;
		} _sigpoll;
        /* SIGSYS.  */
        struct
		{
			void *_call_addr;        		/* Calling user insn.  */
			int _syscall;        			/* Triggering system call number.  */
			unsigned int _arch; 			/* AUDIT_ARCH_* of syscall.  */
		} _sigsys;
	} _sifields;
} linux_siginfo_t;

// We keep only the fields defined in the BSD version.
#define linux_si_pid		_sifields._kill.si_pid
#define linux_si_uid		_sifields._kill.si_uid
#define linux_si_status		_sifields._sigchld.si_status
#define linux_si_value		_sifields._rt.si_sigval
#define linux_si_addr		_sifields._sigfault.si_addr
#define linux_si_band		_sifields._sigpoll.si_band

long sys_waitid(int idtype, int id, siginfo_t* si, int options);

#endif

