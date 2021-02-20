/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#ifndef __FBSD_COMPAT__H_
#define __FBSD_COMPAT__H_

#include <stdint.h>

#define	OFF_MAX		LLONG_MAX
#define	OFF_MIN		LLONG_MIN
#ifndef _BIG_ENDIAN
#define	_BIG_ENDIAN	BIG_ENDIAN
#endif /* _BIG_ENDIAN */
#define	_BYTE_ORDER	BYTE_ORDER
#define	_LITTLE_ENDIAN	LITTLE_ENDIAN
#define	__ct_rune_t	ct_rune_t
#define	__va_list	__darwin_va_list

#define	__isthreaded	1

#ifdef	_FLOCK_DEBUG
#define _FLOCKFILE(x) _flockfile_debug(x, __FILE__, __LINE__)
#else
#define _FLOCKFILE(x) _flockfile(x)
#endif

#define	FLOCKFILE(fp) _FLOCKFILE(fp)
#define	FUNLOCKFILE(fp) _funlockfile(fp)

/*
 * Do the opposite of FreeBSD namespace.h; that is, map the "hidden" names
 * back to the real names.
 */

/*
 * ISO C (C90) section.  Most names in libc aren't in ISO C, so they
 * should be here.  Most aren't here...
 */
#define		_err				err
#define		_warn				warn

#define		_accept				accept
#define		_bind				bind
#define		_close				close
#define		_connect			connect
#define		_creat				creat
#define		_dup				dup
#define		_dup2				dup2
#define		_execve				execve
#define		_fcntl				fcntl
/*#define		_flock				flock */
#define		_flockfile			flockfile
#define		_ftrylockfile			ftrylockfile
#define		_fstat				fstat
#define		_fstatfs			fstatfs
#define		_fsync				fsync
#define		_funlockfile			funlockfile
#define		_getdirentries			getdirentries
/* #define		_getlogin			getlogin */
#define		_getpeername			getpeername
#define		_getprogname			getprogname
#define		_getsockname			getsockname
#define		_getsockopt			getsockopt
#define		_ioctl				ioctl
/* #define		_kevent				kevent */
#define		_listen				listen
#define		_nanosleep			nanosleep
#define		_open				open
#define		_pause				pause
#define		_poll				poll
#define		_pthread_cond_broadcast		pthread_cond_broadcast
#define		_pthread_cond_destroy		pthread_cond_destroy
#define		_pthread_cond_init		pthread_cond_init
#define		_pthread_cond_signal		pthread_cond_signal
#define		_pthread_cond_timedwait		pthread_cond_timedwait
#define		_pthread_cond_wait		pthread_cond_wait
#define		_pthread_exit			pthread_exit
#define		_pthread_getspecific		pthread_getspecific
#define		_pthread_key_create		pthread_key_create
#define		_pthread_key_delete		pthread_key_delete
#define		_pthread_main_np		pthread_main_np
#define		_pthread_mutex_destroy		pthread_mutex_destroy
#define		_pthread_mutex_init		pthread_mutex_init
#define		_pthread_mutex_lock		pthread_mutex_lock
#define		_pthread_mutex_trylock		pthread_mutex_trylock
#define		_pthread_mutex_unlock		pthread_mutex_unlock
#define		_pthread_mutexattr_destroy	pthread_mutexattr_destroy
#define		_pthread_mutexattr_init		pthread_mutexattr_init
#define		_pthread_mutexattr_settype	pthread_mutexattr_settype
#define		_pthread_once			pthread_once
#define		_pthread_rwlock_destroy		pthread_rwlock_destroy
#define		_pthread_rwlock_init		pthread_rwlock_init
#define		_pthread_rwlock_rdlock		pthread_rwlock_rdlock
#define		_pthread_rwlock_wrlock		pthread_rwlock_wrlock
#define		_pthread_rwlock_tryrdlock	pthread_rwlock_tryrdlock
#define		_pthread_rwlock_trywrlock	pthread_rwlock_trywrlock
#define		_pthread_rwlock_unlock		pthread_rwlock_unlock
#define		_pthread_self			pthread_self
#define		_pthread_setspecific		pthread_setspecific
#define		_pthread_sigmask		pthread_sigmask
#define		_read				read
#define		_readv				readv
#define		_recvfrom			recvfrom
#define		_recvmsg			recvmsg
#define		_select				select
#define		_pselect			pselect
#define		_sendmsg			sendmsg
#define		_sendto				sendto
#define		_setsockopt			setsockopt
#define		_sigaction			sigaction
#define		_sigprocmask			sigprocmask
#define		_sigsuspend			sigsuspend
#define		_sleep				sleep
#define		_socket				socket
#define		_socketpair			socketpair
#define		_system				system
#define		_tcdrain			tcdrain
#define 	_usleep				usleep
#define		_wait				wait
#define		_wait4				wait4
#define		_waitpid			waitpid
#define		_write				write
#define		_writev				writev

#define		__creat				creat
#define		__fmtcheck			fmtcheck
#define		__inet_addr			inet_addr
#define		__inet_aton			inet_aton
#define		__inet_lnaof			inet_lnaof
#define		__inet_makeaddr			inet_makeaddr
#define		__inet_net_ntop			inet_net_ntop
#define		__inet_net_pton			inet_net_pton
#define		__inet_neta			inet_neta
#define		__inet_netof			inet_netof
#define		__inet_network			inet_network
#define		__inet_nsap_addr		inet_nsap_addr
#define		__inet_nsap_ntoa		inet_nsap_ntoa
#define		__inet_ntoa			inet_ntoa
#define		__inet_ntop			inet_ntop
#define		__inet_pton			inet_pton
#define		__makecontext			makecontext
#define		__makecontext			makecontext
#define		__pause				pause
#define		__raise				raise
#define		__signalcontext			signalcontext
#define		__sleep				sleep
#define		__strtok_r			strtok_r
#define		__swapcontext			swapcontext
#define		__system			system
#define		__tcdrain			tcdrain
#define 	__usleep			usleep
#define		__vfscanf			vfscanf
#define		__wait				wait
#define		__waitpid			waitpid

/*
 * The _GENERIC_DIRSIZ macro gives the minimum record length which will hold
 * the directory entry.  This returns the amount of space in struct direct
 * without the d_name field, plus enough space for the name with a terminating
 * null byte (dp->d_namlen+1), rounded up to a 4 byte boundary.
  */
#define	_GENERIC_DIRSIZ(dp) \
    (((unsigned long)&((struct dirent *)0)->d_name + (dp)->d_namlen+1 + 3) & ~3)

#endif /* __FBSD_COMPAT__H_ */
