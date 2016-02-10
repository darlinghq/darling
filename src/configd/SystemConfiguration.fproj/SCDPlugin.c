/*
 * Copyright (c) 2002-2006, 2013 Apple Computer, Inc. All rights reserved.
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

/*
 * Modification History
 *
 * October 12, 2001		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <fcntl.h>
#include <paths.h>
#include <pwd.h>
#include <pthread.h>
#include <unistd.h>
#include <sysexits.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <mach/mach.h>
#include <mach/mach_error.h>

#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SCDPlugin.h>
#include <SystemConfiguration/SCPrivate.h>



typedef struct childInfo *childInfoRef;

struct childInfo {
	pid_t			pid;
	SCDPluginExecCallBack	callout;
	void			*context;
	int			status;
	struct rusage		rusage;
	childInfoRef		next;
};


/*
 * Mach port used to notify runloop when a child process
 * has been reaped.
 */
static CFMachPortRef	childReaped	= NULL;

/*
 * The following dictionaries contain information about child
 * processes, reaped processes, and any associated callback
 * information.
 *
 * Important: Access to these dictionaries should only be
 *            made when in a SIGCHLD handler (or when the
 *            childLock mutex is held *AND* the signal
 *            has been blocked).
 */
static childInfoRef	activeChildren	= NULL;
static pthread_mutex_t	lock		= PTHREAD_MUTEX_INITIALIZER;


static __inline__ void
blockSignal()
{
	sigset_t	mask	= sigmask(SIGCHLD);

	// block SIGCHLD
	if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
		perror("sigprocmask(SIG_BLOCK)");
	}

	return;
}


static __inline__ void
unblockSignal()
{
	sigset_t	mask	= sigmask(SIGCHLD);

	// unblock SIGCHLD
	if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1) {
		perror("sigprocmask(SIG_UNBLOCK)");
	}

	return;
}


static void
reaper(int sigraised)
{
	/*
	 * block additional SIGCHLD's until current children have
	 * been reaped.
	 */
	blockSignal();

	/*
	 * send message to indicate that at least one child is ready
	 * to be reaped.
	 */
	_SC_sendMachMessage(CFMachPortGetPort(childReaped), 0);

	return;
}


static void
childrenReaped(CFMachPortRef port, void *msg, CFIndex size, void *info)
{
	pid_t		pid		= 0;
	childInfoRef	reapedChildren	= NULL;

	do {
		struct rusage	rusage;
		int		status;

		pid = wait4(-1, &status, WNOHANG, &rusage);
		switch (pid) {
			case -1 :	// if error
				if (errno != ECHILD) {
					perror("wait4");
				}
				break;

			case  0 :	// if no more children
				break;

			default : {
				childInfoRef	last;
				childInfoRef	this;

				// grab the activeChildren mutex
				pthread_mutex_lock(&lock);

				last = NULL;
				this = activeChildren;
				while (this) {
					if (this->pid == pid) {
						/* save exit status & usage */
						this->status = status;
						this->rusage = rusage;

						/* remove from activeChildren */
						if (last) {
							last->next = this->next;
						} else {
							activeChildren = this->next;
						}

						/* add to reapedChildren */
						this->next = reapedChildren;
						reapedChildren = this;

						break;
					} else {
						/* if not this child */
						last = this;
						this = this->next;
					}
				}

				// release the activeChildren mutex
				pthread_mutex_unlock(&lock);

				break;
			}
		}
	} while (pid > 0);

	/*
	 * we need to know about any new children waiting to be reaped so
	 * re-enable the SIGCHLD handler.

	 */
	unblockSignal();

	while (reapedChildren) {
		childInfoRef	child = reapedChildren;

		reapedChildren = reapedChildren->next;
		(*child->callout)(child->pid,
				  child->status,
				  &child->rusage,
				  child->context);
		CFAllocatorDeallocate(NULL, child);
	}

	return;
}


static CFStringRef
childReapedMPCopyDescription(const void *info)
{
	return CFStringCreateWithFormat(NULL, NULL, CFSTR("<SIGCHLD MP>"));
}


void
_SCDPluginExecInit()
{
	struct sigaction	act;
	CFMachPortContext	context	= { 0
					  , (void *)1
					  , NULL
					  , NULL
					  , childReapedMPCopyDescription
					  };

	CFRunLoopSourceRef	rls;

	// create the "a child has been reaped" notification port
	childReaped = CFMachPortCreate(NULL, childrenReaped, &context, NULL);

	// set queue limit
	{
		mach_port_limits_t	limits;
		kern_return_t		status;

		limits.mpl_qlimit = 1;
		status = mach_port_set_attributes(mach_task_self(),
						  CFMachPortGetPort(childReaped),
						  MACH_PORT_LIMITS_INFO,
						  (mach_port_info_t)&limits,
						  MACH_PORT_LIMITS_INFO_COUNT);
		if (status != KERN_SUCCESS) {
			perror("mach_port_set_attributes");
		}
	}

	// add to our runloop
	rls = CFMachPortCreateRunLoopSource(NULL, childReaped, 0);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	CFRelease(rls);

	// enable signal handler
	act.sa_handler = reaper;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_RESTART|SA_NOCLDSTOP;
	if (sigaction(SIGCHLD, &act, NULL) == -1) {
		perror("sigaction");
	}

	return;
}


pid_t
_SCDPluginExecCommand2(SCDPluginExecCallBack	callout,
		       void			*context,
		       uid_t			uid,
		       gid_t			gid,
		       const char		*path,
		       char * const 		argv[],
		       SCDPluginExecSetup	setup,
		       void			*setupContext
		       )
{
	char		buf[1024];
	pid_t		pid;
	struct passwd	pwd;
	struct passwd	*result	= NULL;
	char		*username = NULL;

	// grab the activeChildren mutex
	pthread_mutex_lock(&lock);

	// cache the getpwuid_r result here to avoid spinning that can happen
	// when calling it between fork and execv.
	if ((getpwuid_r(uid, &pwd, buf, sizeof(buf), &result) == 0) &&
	    (result != NULL)) {
		username = result->pw_name;
	}

	// if needed, initialize
	if (childReaped == NULL) {
		_SCDPluginExecInit();
	}

	pid = fork();

	switch (pid) {
		case -1 : {	/* if error */

			int	status;

			status = errno;
			printf("fork() failed: %s\n", strerror(status));
			errno  = status;
			break;
		}

		case 0 : {	/* if child */

			gid_t	egid;
			uid_t	euid;
			int	i;
			int	status;

			if (setup != NULL) {
				(setup)(pid, setupContext);
			} else {
				/* close any open FDs */
				for (i = getdtablesize()-1; i>=0; i--) close(i);
				open(_PATH_DEVNULL, O_RDWR, 0);
				dup(0);
				dup(0);
			}

			egid = getegid();
			euid = geteuid();

			if (egid != gid) {
				(void) setgid(gid);
			}

			if (((euid != uid) || (egid != gid)) && username) {
				initgroups(username, gid);
			}

			if (euid != uid) {
				(void) setuid(uid);
			}

			/* ensure that our PATH environment variable is somewhat reasonable */
			if (setenv("PATH", "/bin:/sbin:/usr/bin:/usr/sbin", 0) == -1) {
				printf("setenv() failed: %s\n", strerror(errno));
				exit(EX_OSERR);
			}

			/* execute requested command */
			(void) execv(path, argv);

			/* if the execv failed */
			status = W_EXITCODE(errno, 0);
			_exit (WEXITSTATUS(status));
		}

		default : {	/* if parent */
			if (setup != NULL) {
				(setup)(pid, setupContext);
			}

			if (callout != NULL) {
				childInfoRef	child;

				// create child process info
				child = CFAllocatorAllocate(NULL, sizeof(struct childInfo), 0);
				bzero(child, sizeof(struct childInfo));
				child->pid     = pid;
				child->callout = callout;
				child->context = context;

				// add the new child to the activeChildren list
				child->next = activeChildren;
				activeChildren = child;
			}
			break;
		}
	}

	// release the activeChildren mutex
	pthread_mutex_unlock(&lock);

	return pid;
}


pid_t
_SCDPluginExecCommand(SCDPluginExecCallBack	callout,
		     void			*context,
		     uid_t			uid,
		     gid_t			gid,
		     const char			*path,
		     char * const		argv[])
{
	return _SCDPluginExecCommand2(callout, context, uid, gid, path, argv, NULL, NULL);
}
