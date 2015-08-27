#ifndef LINUX_BASE_H
#define LINUX_BASE_H

#define VISIBLE __attribute__ ((visibility ("default")))

#define LINUX_SYSCALL0(nr) linux_syscall(0, 0, 0, 0, 0, 0, nr);
#define LINUX_SYSCALL1(nr, a1) linux_syscall((long)a1, 0, 0, 0, 0, 0, nr);
#define LINUX_SYSCALL2(nr, a1, a2) linux_syscall((long)a1, (long)a2, 0, 0, 0, 0, nr);
#define LINUX_SYSCALL3(nr, a1, a2, a3) linux_syscall((long)a1, (long)a2, (long)a3, 0, 0, 0, nr);
#define LINUX_SYSCALL4(nr, a1, a2, a3, a4) linux_syscall((long)a1, (long)a2, (long)a3, (long)a4, 0, 0, nr);
#define LINUX_SYSCALL5(nr, a1, a2, a3, a4, a5) linux_syscall((long)a1, (long)a2, (long)a3, (long)a4, (long)a5, 0, nr);
#define LINUX_SYSCALL6(nr, a1, a2, a3, a4, a5, a6) linux_syscall((long)a1, (long)a2, (long)a3, (long)a4, (long)a5, (long)a6, nr);

#define __SYSCALL_NARGS_X(a,b,c,d,e,f,g,h,n,...) n
#define __SYSCALL_NARGS(...) __SYSCALL_NARGS_X(__VA_ARGS__,7,6,5,4,3,2,1,0)
#define __SYSCALL_CONCAT_X(a,b) a##b
#define __SYSCALL_CONCAT(a,b) __SYSCALL_CONCAT_X(a,b)
#define LINUX_SYSCALL(...) __SYSCALL_CONCAT(LINUX_SYSCALL,__SYSCALL_NARGS(__VA_ARGS__))(__VA_ARGS__)

long linux_syscall(long a1, long a2, long a3, long a4, long a5, long a6, int nr);

#ifdef __x86_64__
#	define LL_ARG(x) (x)
#else
#	define LL_ARG(x) ((union { long long ll; long l[2]; }){ .ll = x }).l[0], \
			((union { long long ll; long l[2]; }){ .ll = x }).l[1]
#endif

long __darling_bsd_syscall(int nr, ...);

#endif

