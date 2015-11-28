/*
 * Copyright (c) 2000-2011, 2013, 2014 Apple Inc. All rights reserved.
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
 * October 30, 2003		Allan Nathanson <ajn@apple.com>
 * - add plugin "stop()" function support
 *
 * June 1, 2001			Allan Nathanson <ajn@apple.com>
 * - public API conversion
 *
 * 24 March 2000		Allan Nathanson <ajn@apple.com>
 * - created
 */

#define DO_NOT_INFORM

#include <getopt.h>
#include <stdio.h>
#include <sysexits.h>
#include <syslog.h>
#include <unistd.h>
#include <paths.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <objc/objc-runtime.h>
#include <servers/bootstrap.h>
#include <vproc.h>
#include <vproc_priv.h>

#include "configd.h"
#include "configd_server.h"
#include "plugin_support.h"

#if	TARGET_OS_EMBEDDED && !defined(DO_NOT_INFORM)
#include <CoreFoundation/CFUserNotification.h>
#endif	// TARGET_OS_EMBEDDED && !defined(DO_NOT_INFORM)

__private_extern__
Boolean	_configd_verbose		= FALSE;	/* TRUE if verbose logging enabled */

__private_extern__
FILE	*_configd_trace			= NULL;		/* non-NULL if tracing enabled */

__private_extern__
CFMutableSetRef	_plugins_allowed	= NULL;		/* bundle identifiers to allow when loading */

__private_extern__
CFMutableSetRef	_plugins_exclude	= NULL;		/* bundle identifiers to exclude from loading */

__private_extern__
CFMutableSetRef	_plugins_verbose	= NULL;		/* bundle identifiers to enable verbose logging */

static CFMachPortRef termRequested	= NULL;		/* Mach port used to notify runloop of a shutdown request */


static const struct option longopts[] = {
//	{ "include-plugin",	required_argument,	0,	'A' },
//	{ "no-bundles",		no_argument,		0,	'b' },
//	{ "exclude-plugin",	required_argument,	0,	'B' },
//	{ "no-fork",		no_argument,		0,	'd' },
//	{ "test-bundle",	required_argument,      0,	't' },
//	{ "verbose",		no_argument,		0,	'v' },
//	{ "verbose-bundle",	required_argument,	0,	'V' },
	{ "help",		no_argument,		0,	'?' },
	{ 0,			0,                      0,	0 }
};


static void
usage(const char *prog)
{
	SCPrint(TRUE, stderr, CFSTR("%s: [-d] [-v] [-V bundleID] [-b] [-B bundleID] [-A bundleID] [-t bundle-path]\n"), prog);
	SCPrint(TRUE, stderr, CFSTR("options:\n"));
	SCPrint(TRUE, stderr, CFSTR("\t-d\tdisable daemon/run in foreground\n"));
	SCPrint(TRUE, stderr, CFSTR("\t-v\tenable verbose logging\n"));
	SCPrint(TRUE, stderr, CFSTR("\t-V\tenable verbose logging for the specified plug-in\n"));
	SCPrint(TRUE, stderr, CFSTR("\t-f\tload ALL plug-ins in a separate process\n"));
	SCPrint(TRUE, stderr, CFSTR("\t-b\tdisable loading of ALL plug-ins\n"));
	SCPrint(TRUE, stderr, CFSTR("\t-B\tdisable loading of the specified plug-in\n"));
	SCPrint(TRUE, stderr, CFSTR("\t-A\tenable loading of the specified plug-in\n"));
	SCPrint(TRUE, stderr, CFSTR("\t-t\tload/test the specified plug-in\n"));
	SCPrint(TRUE, stderr, CFSTR("\t\t  (Note: only the plug-in will be started)\n"));
	exit (EX_USAGE);
}


static void
catcher(int signum)
{
	switch (signum) {
		case SIGINT :
		case SIGTERM :
			if (termRequested != NULL) {
				if (_sc_log > 0) {
					/*
					 * if we've received a [shutdown] SIGTERM
					 * and we are syslog'ing than it's likely
					 * that syslogd is also being term'd.  As
					 * such, let's also push any remaining log
					 * messages to stdout/stderr.
					 */
					_sc_log = 2;
				}

				/*
				 * send message to indicate that a request has been made
				 * for the daemon to be shutdown.
				 */
				_SC_sendMachMessage(CFMachPortGetPort(termRequested), 0);
			} else {
				_exit(EX_OK);
			}
			break;
	}

	return;
}


static void
term(CFMachPortRef port, void *msg, CFIndex size, void *info)
{
	int	status	= EX_OK;
	Boolean	wait;

	wait = plugin_term(&status);
	if (!wait) {
		// if we are not waiting on a plugin
		status = server_shutdown();
		exit (status);
	}

	return;
}


static void
parent_exit(int i)
{
	_exit (0);
}


static void
init_fds()
{
	int	fd;
	int	i;

	/* close any open FDs */
	for (i = getdtablesize()-1; i>=0; i--) close(i);

	/* set stdin, stdout, stderr */
	fd = open(_PATH_DEVNULL, O_RDWR, 0);
	if (fd != -1) {
		int	ofd;

		// stdin
		(void) dup2(fd, STDIN_FILENO);

		// stdout, stderr
		ofd = open("/var/log/configd.log", O_WRONLY|O_APPEND, 0);
		if (ofd != -1) {
			if (fd > STDIN_FILENO) {
				(void) close(fd);
			}
			fd = ofd;
		}
		(void) dup2(fd, STDOUT_FILENO);
		(void) dup2(fd, STDERR_FILENO);
		if (fd > STDERR_FILENO) {
			(void) close(fd);
		}
	}

	SCTrace(TRUE, stdout, CFSTR("start\n"));
	return;
}


static void
set_trace()
{
	int	fd;

	/* set _configd_trace */
	fd = open("/var/log/configd.trace", O_WRONLY|O_APPEND, 0);
	if (fd != -1) {
		_configd_trace = fdopen(fd, "a");
		SCTrace(TRUE, _configd_trace, CFSTR("start\n"));
	}

	return;
}


static int
fork_child()
{
	int	child_pid;

	signal(SIGTERM, parent_exit);
	child_pid = fork();
	switch (child_pid) {
		case -1: {
			return -1;
		}
		case 0: {
			/* child: becomes the daemon (see below) */
			signal(SIGTERM, SIG_DFL);
			break;
		}
		default: {
			/* parent: wait for signal, then exit */
			int	status;

			(void) wait4(child_pid, (int *)&status, 0, 0);
			if (WIFEXITED(status)) {
				fprintf(stderr,
					"*** configd (daemon) failed to start, exit status=%d\n",
					WEXITSTATUS(status));
			} else {
				fprintf(stderr,
					"*** configd (daemon) failed to start, received signal=%d\n",
					WTERMSIG(status));
			}
			fflush (stderr);
			exit (EX_SOFTWARE);
		}
	}

	if (setsid() == -1)
		return -1;

	(void) chdir("/");

	return 0;
}

static CFStringRef
termMPCopyDescription(const void *info)
{
	return CFStringCreateWithFormat(NULL, NULL, CFSTR("<SIGTERM MP>"));
}


int
main(int argc, char * const argv[])
{
	CFMachPortContext	context		= { 0
						  , (void *)1
						  , NULL
						  , NULL
						  , termMPCopyDescription
						  };
	Boolean			forceForeground	= FALSE;
	Boolean			forcePlugin	= FALSE;
	int64_t			is_launchd_job	= 0;
	mach_port_limits_t	limits;
	Boolean			loadBundles	= TRUE;
	struct sigaction	nact;
	int			opt;
//	extern int		optind;
	const char		*prog		= argv[0];
	CFRunLoopSourceRef	rls;
	kern_return_t		status;
	CFStringRef		str;
	const char		*testBundle	= NULL;

	_plugins_allowed = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
	_plugins_exclude = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
	_plugins_verbose = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);

	/* process any arguments */

	while ((opt = getopt_long(argc, argv, "A:bB:dt:vV:", longopts, NULL)) != -1) {
		switch(opt) {
			case 'A':
				str = CFStringCreateWithCString(NULL, optarg, kCFStringEncodingMacRoman);
				CFSetSetValue(_plugins_allowed, str);
				CFRelease(str);
				break;
			case 'b':
				loadBundles = FALSE;
				break;
			case 'B':
				str = CFStringCreateWithCString(NULL, optarg, kCFStringEncodingMacRoman);
				CFSetSetValue(_plugins_exclude, str);
				CFRelease(str);
				break;
			case 'd':
				forceForeground = TRUE;
				break;
			case 't':
				testBundle = optarg;
				break;
			case 'v':
				_configd_verbose = TRUE;
				break;
			case 'V':
				if (strcmp(optarg, "com.apple.SystemConfiguration") == 0) {
					_sc_verbose = TRUE;
				} else {
					str = CFStringCreateWithCString(NULL, optarg, kCFStringEncodingMacRoman);
					CFSetSetValue(_plugins_verbose, str);
					CFRelease(str);
				}
				break;
			case '?':
			default :
				usage(prog);
		}
	}
//	argc -= optind;
//	argv += optind;

	/* check credentials */
#if	!TARGET_IPHONE_SIMULATOR
	if (getuid() != 0) {
		fprintf(stderr, "%s: permission denied.\n", prog);
		exit (EX_NOPERM);
	}
#endif	// !TARGET_IPHONE_SIMULATOR

	/* check if we have been started by launchd */
	vproc_swap_integer(NULL, VPROC_GSK_IS_MANAGED, NULL, &is_launchd_job);

#if	TARGET_OS_EMBEDDED && !defined(DO_NOT_INFORM)
	// if launchd job, check to see if we have been restarted
	if (is_launchd_job) {
		int64_t	status	= 0;

		vproc_swap_integer(NULL, VPROC_GSK_LAST_EXIT_STATUS, NULL, &status);
		if ((status != 0) && _SC_isAppleInternal()) {
			int	fd;

			// if we've been restarted
			fd = open("/var/run/configd-crash-reported", O_WRONLY|O_CREAT|O_EXCL, 0644);
			if (fd >= 0) {
				// if we have not yet alerted the user
				CFUserNotificationDisplayNotice(0,
								kCFUserNotificationStopAlertLevel,
								NULL,
								NULL,
								NULL,
								CFSTR("\"configd\" restarted"),
								CFSTR("Please collect the crash report and file a Radar."),
								NULL);
				close(fd);
			}
		}
	}
#endif	// TARGET_OS_EMBEDDED && !defined(DO_NOT_INFORM)

	/* ensure that forked plugins behave */
	if ((testBundle != NULL) && (getenv("__FORKED_PLUGIN__") != NULL)) {
		forcePlugin = TRUE;
	}

	/* if needed, daemonize */
	if (!forceForeground && !is_launchd_job) {
		/*
		 * if we haven't been asked to run in the foreground
		 * and have not been started by launchd (i.e. we're
		 * not already running as a Foreground process) then
		 * daemonize ourself.
		 */
		if (fork_child() == -1) {
			fprintf(stderr, "configd: fork() failed, %s\n", strerror(errno));
			exit (1);
		}

		/*
		 * Note: we are now the child process. The parent
		 *       waits/exits in fork_child.
		 */
	}

	/*
	 * close file descriptors, establish stdin/stdout/stderr,
	 * setup logging.
	 */
	if (!forceForeground || forcePlugin) {
		int		facility	= LOG_DAEMON;
		int		logopt		= LOG_CONS|LOG_NDELAY|LOG_PID;
		struct stat	statbuf;

		if (!is_launchd_job && !forcePlugin) {
			init_fds();
		}

		if (_configd_verbose) {
			logopt |= LOG_CONS;
		}

		if (stat("/etc/rc.cdrom", &statbuf) == 0) {
			facility = LOG_INSTALL;
		}

		openlog("configd", logopt, facility);
	} else {
		_sc_log = FALSE;	/* redirect SCLog() to stdout/stderr */
	}

	/* check/enable trace logging */
	set_trace();

	/* add signal handler to catch a SIGHUP */
	nact.sa_handler = catcher;
	sigemptyset(&nact.sa_mask);
	nact.sa_flags = SA_RESTART;
	if (sigaction(SIGHUP, &nact, NULL) == -1) {
		SCLog(_configd_verbose, LOG_ERR,
		       CFSTR("sigaction(SIGHUP, ...) failed: %s"),
		       strerror(errno));
	}

	/* add signal handler to catch a SIGPIPE */
	if (sigaction(SIGPIPE, &nact, NULL) == -1) {
		SCLog(_configd_verbose, LOG_ERR,
		       CFSTR("sigaction(SIGPIPE, ...) failed: %s"),
		       strerror(errno));
	}

	/* add signal handler to catch a SIGTERM */
	if (sigaction(SIGTERM, &nact, NULL) == -1) {
		SCLog(_configd_verbose, LOG_ERR,
		      CFSTR("sigaction(SIGTERM, ...) failed: %s"),
		      strerror(errno));
	}

	/* add signal handler to catch a SIGINT */
	if (sigaction(SIGINT, &nact, NULL) == -1) {
		SCLog(_configd_verbose, LOG_ERR,
		      CFSTR("sigaction(SIGINT, ...) failed: %s"),
		      strerror(errno));
	}

	/* create the "shutdown requested" notification port */
	termRequested = CFMachPortCreate(NULL, term, &context, NULL);

	/* set queue limit */
	limits.mpl_qlimit = 1;
	status = mach_port_set_attributes(mach_task_self(),
					  CFMachPortGetPort(termRequested),
					  MACH_PORT_LIMITS_INFO,
					  (mach_port_info_t)&limits,
					  MACH_PORT_LIMITS_INFO_COUNT);
	if (status != KERN_SUCCESS) {
		perror("mach_port_set_attributes");
	}

	/* add to our runloop */
	rls = CFMachPortCreateRunLoopSource(NULL, termRequested, 0);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	CFRelease(rls);

	if (testBundle == NULL) {
		/* initialize primary (store management) thread */
		server_init();

		if (!forceForeground && !is_launchd_job) {
			/* synchronize with parent process */
			kill(getppid(), SIGTERM);
		}

		/* load/initialize/start bundles into the secondary thread */
		if (loadBundles) {
			/* start plug-in initialization */
			plugin_init();
		}

		/* start primary (store management) thread */
		server_loop();
	} else {
		/* load/initialize/start specified plug-in */
		plugin_exec((void *)testBundle);
	}

	exit (EX_OK);	/* insure the process exit status is 0 */
	return 0;	/* ...and make main fit the ANSI spec. */
}
