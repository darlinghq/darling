/*
 * Copyright (c) 2000-2014 Apple Inc. All rights reserved.
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
 * June 13, 2005		Allan Nathanson <ajn@apple.com>
 * - added SCPreferences support
 *
 * August 4, 2004		Allan Nathanson <ajn@apple.com>
 * - added network configuration (prefs) support
 *
 * September 25, 2002		Allan Nathanson <ajn@apple.com>
 * - added command line history & editing
 *
 * July 9, 2001			Allan Nathanson <ajn@apple.com>
 * - added "-r" option for checking network reachability
 * - added "-w" option to check/wait for the presence of a
 *   dynamic store key.
 *
 * June 1, 2001			Allan Nathanson <ajn@apple.com>
 * - public API conversion
 *
 * November 9, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <TargetConditionals.h>
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sysexits.h>

#ifdef	DEBUG
#include <mach/mach.h>
#include <mach/mach_error.h>
#endif	/* DEBUG */

#include "scutil.h"
#include "commands.h"
#include "dictionary.h"
#include "net.h"
#include "nc.h"
#include "prefs.h"
#include "session.h"
#include "tests.h"


#define LINE_LENGTH 2048


__private_extern__ AuthorizationRef	authorization	= NULL;
__private_extern__ InputRef		currentInput	= NULL;
__private_extern__ Boolean		doDispatch	= FALSE;
__private_extern__ int			nesting		= 0;
__private_extern__ CFRunLoopRef		notifyRl	= NULL;
__private_extern__ CFRunLoopSourceRef	notifyRls	= NULL;
__private_extern__ SCPreferencesRef	prefs		= NULL;
__private_extern__ SCDynamicStoreRef	store		= NULL;
__private_extern__ CFPropertyListRef	value		= NULL;
__private_extern__ CFMutableArrayRef	watchedKeys	= NULL;
__private_extern__ CFMutableArrayRef	watchedPatterns	= NULL;

static const struct option longopts[] = {
//	{ "debug",		no_argument,		NULL,	'd'	},
//	{ "dispatch",		no_argument,		NULL,	'D'	},
//	{ "verbose",		no_argument,		NULL,	'v'	},
//	{ "SPI",		no_argument,		NULL,	'p'	},
//	{ "check-reachability",	required_argument,	NULL,	'r'	},
//	{ "timeout",		required_argument,	NULL,	't'	},
//	{ "wait-key",		required_argument,	NULL,	'w'	},
//	{ "watch-reachability",	no_argument,		NULL,	'W'	},
	{ "dns",		no_argument,		NULL,	0	},
	{ "get",		required_argument,	NULL,	0	},
	{ "error",		required_argument,	NULL,	0	},
	{ "help",		no_argument,		NULL,	'?'	},
	{ "nc",			required_argument,	NULL,	0	},
	{ "net",		no_argument,		NULL,	0	},
	{ "nwi",		no_argument,		NULL,	0	},
	{ "prefs",		no_argument,		NULL,	0	},
	{ "proxy",		no_argument,		NULL,	0	},
	{ "renew",		required_argument,	NULL,	0	},
	{ "set",		required_argument,	NULL,	0	},
	{ "snapshot",		no_argument,		NULL,	0	},
	{ "user",		required_argument,	NULL,	0	},
	{ "password",		required_argument,	NULL,	0	},
	{ "secret",		required_argument,	NULL,	0	},
	{ "log",		required_argument,	NULL,	0	},
	{ NULL,			0,			NULL,	0	}
};


__private_extern__
CFStringRef
_copyStringFromSTDIN(CFStringRef prompt, CFStringRef defaultValue)
{
	char		buf[1024];
	int		i;
	Boolean		is_user_prompt = (prompt != NULL && isatty(STDIN_FILENO) && isatty(STDOUT_FILENO));
	int		len;
	char		*modbuf;
	int		modlen;
	CFStringRef	utf8;

	/* Print out a prompt to user that entry is desired */
	if (is_user_prompt) {
		if (defaultValue != NULL) {
			SCPrint(TRUE, stdout, CFSTR("%@ [%@]: "), prompt, defaultValue);
		} else {
			SCPrint(TRUE, stdout, CFSTR("%@: "), prompt);
		}
	}

	/* Get user input */
	if (fgets(buf, sizeof(buf), stdin) == NULL) {
		return NULL;
	}

	/* Prepare for trim */
	len = (int)strlen(buf);
	modbuf = buf;
	modlen = len;

	/* Trim new-line */
	if ((modlen > 0) && (modbuf[modlen - 1] == '\n')) {
		modbuf[modlen - 1] = '\0';
		modlen--;
	}

	/* If nothing was entered at the user prompt, set default */
	if (is_user_prompt && defaultValue != NULL && modlen == 0) {
		CFRetain(defaultValue);
		return defaultValue;
	}

	/* Trim spaces from front */
	while (modlen > 0 && isspace(modbuf[0])) {
		modbuf = &modbuf[1];
		modlen--;
	}

	/* Trim spaces from back */
	for (i = modlen - 1; i >= 0; i--) {
		if (isspace(buf[i])) {
			buf[i] = '\0';
			modlen--;
		} else {
			break;
		}
	}

	utf8 = CFStringCreateWithBytes(NULL, (UInt8 *)modbuf, modlen, kCFStringEncodingUTF8, TRUE);
	return utf8;
}

static char *
getLine(char *buf, int len, InputRef src)
{
	int	n;

	if (src->el) {
		int		count;
		const char	*line;

		line = el_gets(src->el, &count);
		if (line == NULL)
			return NULL;

		strncpy(buf, line, len);
	} else {
		if (fgets(buf, len, src->fp) == NULL)
			return NULL;
	}

	n = (int)strlen(buf);
	if (buf[n-1] == '\n') {
		/* the entire line fit in the buffer, remove the newline */
		buf[n-1] = '\0';
	} else if (!src->el) {
		/* eat the remainder of the line */
		do {
			n = fgetc(src->fp);
		} while ((n != '\n') && (n != EOF));
	}

	if (src->h && (buf[0] != '\0')) {
		HistEvent	ev;

		history(src->h, &ev, H_ENTER, buf);
	}


	return buf;
}


static char *
getString(char **line)
{
	char *s, *e, c, *string;
	int i, isQuoted = 0, escaped = 0;

	if (*line == NULL) return NULL;
	if (**line == '\0') return NULL;

	/* Skip leading white space */
	while (isspace(**line)) *line += 1;

	/* Grab the next string */
	s = *line;
	if (*s == '\0') {
		return NULL;				/* no string available */
	} else if (*s == '"') {
		isQuoted = 1;				/* it's a quoted string */
		s++;
	}

	for (e = s; (c = *e) != '\0'; e++) {
		if (isQuoted && (c == '"'))
			break;				/* end of quoted string */
		if (c == '\\') {
			e++;
			if (*e == '\0')
				break;			/* if premature end-of-string */
			if ((*e == '"') || isspace(*e))
				escaped++;		/* if escaped quote or white space */
		}
		if (!isQuoted && isspace(c))
			break;				/* end of non-quoted string */
	}

	string = malloc(e - s - escaped + 1);

	for (i = 0; s < e; s++) {
		string[i] = *s;
		if (!((s[0] == '\\') && ((s[1] == '"') || isspace(s[1])))) i++;
	}
	string[i] = '\0';

	if (isQuoted)
		e++;					/* move past end of quoted string */

	*line = e;
	return string;
}


__private_extern__
Boolean
process_line(InputRef src)
{
	char	*arg;
	int	argc			= 0;
	char	**argv			= NULL;
	int	i;
	char	line[LINE_LENGTH];
	char	*s			= line;

	// if end-of-file, exit
	if (getLine(line, sizeof(line), src) == NULL)
		return FALSE;

	if (nesting > 0) {
		SCPrint(TRUE, stdout, CFSTR("%d> %s\n"), nesting, line);
	}

	// break up the input line
	while ((arg = getString(&s)) != NULL) {
		if (argc == 0)
			argv = (char **)malloc(2 * sizeof(char *));
		else
			argv = (char **)reallocf(argv, ((argc + 2) * sizeof(char *)));
		argv[argc++] = arg;
	}

	if (argc == 0) {
		return TRUE;		// if no arguments
	}

	/* process the command */
	if (*argv[0] != '#') {
		argv[argc] = NULL;	// just in case...
		currentInput = src;
		do_command(argc, argv);
	}

	/* free the arguments */
	for (i = 0; i < argc; i++) {
		free(argv[i]);
	}
	free(argv);

	return !termRequested;
}


static void
usage(const char *command)
{
	SCPrint(TRUE, stderr, CFSTR("usage: %s\n"), command);
	SCPrint(TRUE, stderr, CFSTR("\tinteractive access to the dynamic store.\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("   or: %s --prefs [preference-file]\n"), command);
	SCPrint(TRUE, stderr, CFSTR("\tinteractive access to the [raw] stored preferences.\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("   or: %s [-W] -r nodename\n"), command);
	SCPrint(TRUE, stderr, CFSTR("   or: %s [-W] -r address\n"), command);
	SCPrint(TRUE, stderr, CFSTR("   or: %s [-W] -r local-address remote-address\n"), command);
	SCPrint(TRUE, stderr, CFSTR("\tcheck reachability of node, address, or address pair (-W to \"watch\").\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("   or: %s -w dynamic-store-key [ -t timeout ]\n"), command);
	SCPrint(TRUE, stderr, CFSTR("\t-w\twait for presense of dynamic store key\n"));
	SCPrint(TRUE, stderr, CFSTR("\t-t\ttime to wait for key\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("   or: %s --get pref\n"), command);
	SCPrint(TRUE, stderr, CFSTR("   or: %s --set pref [newval]\n"), command);
	SCPrint(TRUE, stderr, CFSTR("   or: %s --get filename path key  \n"), command);
	SCPrint(TRUE, stderr, CFSTR("\tpref\tdisplay (or set) the specified preference.  Valid preferences\n"));
	SCPrint(TRUE, stderr, CFSTR("\t\tinclude:\n"));
	SCPrint(TRUE, stderr, CFSTR("\t\t\tComputerName, LocalHostName, HostName\n"));
	SCPrint(TRUE, stderr, CFSTR("\tnewval\tNew preference value to be set.  If not specified,\n"));
	SCPrint(TRUE, stderr, CFSTR("\t\tthe new value will be read from standard input.\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("   or: %s --dns\n"), command);
	SCPrint(TRUE, stderr, CFSTR("\tshow DNS configuration.\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("   or: %s --proxy\n"), command);
	SCPrint(TRUE, stderr, CFSTR("\tshow \"proxy\" configuration.\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("   or: %s --nwi\n"), command);
	SCPrint(TRUE, stderr, CFSTR("\tshow network information\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("   or: %s --nc\n"), command);
	SCPrint(TRUE, stderr, CFSTR("\tshow VPN network configuration information. Use --nc help for full command list\n"));

	if (_sc_debug) {
		SCPrint(TRUE, stderr, CFSTR("\n"));
		SCPrint(TRUE, stderr, CFSTR("   or: %s --log IPMonitor [off|on]\n"), command);
		SCPrint(TRUE, stderr, CFSTR("\tmanage logging.\n"));
	}

	if (getenv("ENABLE_EXPERIMENTAL_SCUTIL_COMMANDS")) {
		SCPrint(TRUE, stderr, CFSTR("\n"));
		SCPrint(TRUE, stderr, CFSTR("   or: %s --net\n"), command);
		SCPrint(TRUE, stderr, CFSTR("\tmanage network configuration.\n"));
	}

	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("   or: %s --error err#\n"), command);
	SCPrint(TRUE, stderr, CFSTR("\tdisplay a descriptive message for the given error code\n"));

	exit (EX_USAGE);
}


static char *
prompt(EditLine *el)
{
#if	!TARGET_IPHONE_SIMULATOR
	return "> ";
#else	// !TARGET_IPHONE_SIMULATOR
	return "sim> ";
#endif	// !TARGET_IPHONE_SIMULATOR
}


int
main(int argc, char * const argv[])
{
	Boolean			doDNS	= FALSE;
	Boolean			doNet	= FALSE;
	Boolean			doNWI	= FALSE;
	Boolean			doPrefs	= FALSE;
	Boolean			doProxy	= FALSE;
	Boolean			doReach	= FALSE;
	Boolean			doSnap	= FALSE;
	char			*error	= NULL;
	char			*get	= NULL;
	char			*log	= NULL;
	extern int		optind;
	int			opt;
	int			opti;
	const char		*prog	= argv[0];
	char			*renew	= NULL;
	char			*set	= NULL;
	char			*nc_cmd	= NULL;
	InputRef		src;
	int			timeout	= 15;	/* default timeout (in seconds) */
	char			*wait	= NULL;
	Boolean			watch	= FALSE;
	int			xStore	= 0;	/* non dynamic store command line options */

	/* process any arguments */

	while ((opt = getopt_long(argc, argv, "dDvprt:w:W", longopts, &opti)) != -1)
		switch(opt) {
		case 'd':
			_sc_debug = TRUE;
			_sc_log   = FALSE;	/* enable framework logging */
			break;
		case 'D':
			doDispatch = TRUE;
			break;
		case 'v':
			_sc_verbose = TRUE;
			_sc_log     = FALSE;	/* enable framework logging */
			break;
		case 'p':
			enablePrivateAPI = TRUE;
			break;
		case 'r':
			doReach = TRUE;
			xStore++;
			break;
		case 't':
			timeout = atoi(optarg);
			break;
		case 'w':
			wait = optarg;
			xStore++;
			break;
		case 'W':
			watch = TRUE;
			break;
		case 0:
			if        (strcmp(longopts[opti].name, "dns") == 0) {
				doDNS = TRUE;
				xStore++;
			} else if (strcmp(longopts[opti].name, "error") == 0) {
				error = optarg;
				xStore++;
			} else if (strcmp(longopts[opti].name, "get") == 0) {
				get = optarg;
				xStore++;
			} else if (strcmp(longopts[opti].name, "nc") == 0) {
				nc_cmd = optarg;
				xStore++;
			} else if (strcmp(longopts[opti].name, "net") == 0) {
				doNet = TRUE;
				xStore++;
			} else if (strcmp(longopts[opti].name, "nwi") == 0) {
				doNWI = TRUE;
				xStore++;
			} else if (strcmp(longopts[opti].name, "prefs") == 0) {
				doPrefs = TRUE;
				xStore++;
			} else if (strcmp(longopts[opti].name, "proxy") == 0) {
				doProxy = TRUE;
				xStore++;
			} else if (strcmp(longopts[opti].name, "renew") == 0) {
				renew = optarg;
				xStore++;
			} else if (strcmp(longopts[opti].name, "set") == 0) {
				set = optarg;
				xStore++;
			} else if (strcmp(longopts[opti].name, "snapshot") == 0) {
				doSnap = TRUE;
				xStore++;
			} else if (strcmp(longopts[opti].name, "log") == 0) {
				log = optarg;
				xStore++;
			} else if (strcmp(longopts[opti].name, "user") == 0) {
				username = CFStringCreateWithCString(NULL, optarg, kCFStringEncodingUTF8);
			} else if (strcmp(longopts[opti].name, "password") == 0) {
				password = CFStringCreateWithCString(NULL, optarg, kCFStringEncodingUTF8);
			} else if (strcmp(longopts[opti].name, "secret") == 0) {
				sharedsecret = CFStringCreateWithCString(NULL, optarg, kCFStringEncodingUTF8);
			}
			break;
		case '?':
		default :
			usage(prog);
		}
	argc -= optind;
	argv += optind;

	if (xStore > 1) {
		// if we are attempting to process more than one type of request
		usage(prog);
	}

	/* are we checking (or watching) the reachability of a host/address */
	if (doReach) {
		if (argc < 1) {
			usage(prog);
		}
		if (watch) {
			do_watchReachability(argc, (char **)argv);
		} else {
			do_checkReachability(argc, (char **)argv);
		}
		/* NOT REACHED */
	}

	/* are we waiting on the presense of a dynamic store key */
	if (wait) {
		do_wait(wait, timeout);
		/* NOT REACHED */
	}

	/* are we looking up the DNS configuration */
	if (doDNS) {
		if (watch) {
			do_watchDNSConfiguration(argc, (char **)argv);
		} else {
			do_showDNSConfiguration(argc, (char **)argv);
		}
		/* NOT REACHED */
	}

	if (doNWI) {
		if (watch) {
			do_watchNWI(argc, (char**)argv);
		} else {
			do_showNWI(argc, (char**)argv);
		}
		/* NOT REACHED */
	}

	if (doSnap) {
		if (!enablePrivateAPI
#if	!TARGET_IPHONE_SIMULATOR
		    || (geteuid() != 0)
#endif	// !TARGET_IPHONE_SIMULATOR
		   ) {
			usage(prog);
		}

		do_open(0, NULL);	/* open the dynamic store */
		do_snapshot(argc, (char**)argv);
		exit(0);
	}

	/* are we translating error #'s to descriptive text */
	if (error != NULL) {
		int	sc_status	= atoi(error);

		SCPrint(TRUE, stdout, CFSTR("Error: 0x%08x %d %s\n"),
			sc_status,
			sc_status,
			SCErrorString(sc_status));
		exit(0);
	}

	/* are we looking up a preference value */
	if (get) {
		if (argc != 2) {
			if (findPref(get) < 0) {
				usage(prog);
			}
		} else {
			/* need to go back one argument
			 * for the filename */
			argc++;
			argv--;
		}

		do_getPref(get, argc, (char **)argv);
		/* NOT REACHED */
	}

	/* are we looking up the proxy configuration */
	if (doProxy) {
		do_showProxyConfiguration(argc, (char **)argv);
		/* NOT REACHED */
	}

	/* are we changing a preference value */
	if (set) {
		if (findPref(set) < 0) {
			usage(prog);
		}
		do_setPref(set, argc, (char **)argv);
		/* NOT REACHED */
	}

	/* verbose log */
	if (log != NULL) {
		if (strcasecmp(log, "IPMonitor")) {
			usage(prog);
		}
		do_log(log, argc, (char * *)argv);
		/* NOT REACHED */
	}

	/* network connection commands */
	if (nc_cmd) {
		if (find_nc_cmd(nc_cmd) < 0) {
			usage(prog);
		}
		do_nc_cmd(nc_cmd, argc, (char **)argv, watch);
		/* NOT REACHED */
	}

	if (doNet) {
		/* if we are going to be managing the network configuration */
		commands  = (cmdInfo *)commands_net;
		nCommands = nCommands_net;

		if (!getenv("ENABLE_EXPERIMENTAL_SCUTIL_COMMANDS")) {
			usage(prog);
		}

		do_net_init();				/* initialization */
		do_net_open(argc, (char **)argv);	/* open prefs */
	} else if (doPrefs) {
		/* if we are going to be managing the network configuration */
		commands  = (cmdInfo *)commands_prefs;
		nCommands = nCommands_prefs;

		do_dictInit(0, NULL);			/* start with an empty dictionary */
		do_prefs_init();			/* initialization */
		do_prefs_open(argc, (char **)argv);	/* open prefs */
	} else {
		/* if we are going to be managing the dynamic store */
		commands  = (cmdInfo *)commands_store;
		nCommands = nCommands_store;

		do_dictInit(0, NULL);	/* start with an empty dictionary */
		do_open(0, NULL);	/* open the dynamic store */
	}

	/* are we trying to renew a DHCP lease */
	if (renew != NULL) {
		do_renew(renew);
		/* NOT REACHED */
	}

	/* allocate command input stream */
	src = (InputRef)CFAllocatorAllocate(NULL, sizeof(Input), 0);
	src->fp = stdin;
	src->el = NULL;
	src->h  = NULL;

	if (isatty(fileno(src->fp))) {
		int		editmode	= 1;
		HistEvent	ev;
		struct termios	t;

		if (tcgetattr(fileno(src->fp), &t) != -1) {
			if ((t.c_lflag & ECHO) == 0) {
				editmode = 0;
			}
		}
		src->el = el_init(prog, src->fp, stdout, stderr);
		src->h  = history_init();

		(void)history(src->h, &ev, H_SETSIZE, INT_MAX);
		el_set(src->el, EL_HIST, history, src->h);

		if (!editmode) {
			el_set(src->el, EL_EDITMODE, 0);
		}

		el_set(src->el, EL_EDITOR, "emacs");
		el_set(src->el, EL_PROMPT, prompt);

		el_source(src->el, NULL);

		if ((el_get(src->el, EL_EDITMODE, &editmode) != -1) && editmode != 0) {
			el_set(src->el, EL_SIGNAL, 1);
		} else {
			history_end(src->h);
			src->h = NULL;
			el_end(src->el);
			src->el = NULL;
		}
	}

	while (TRUE) {
		Boolean	ok;

		ok = process_line(src);
		if (!ok) {
			break;
		}
	}

	/* close the socket, free resources */
	if (src->h)	history_end(src->h);
	if (src->el)	el_end(src->el);
	(void)fclose(src->fp);
	CFAllocatorDeallocate(NULL, src);

	exit (EX_OK);	// insure the process exit status is 0
	return 0;	// ...and make main fit the ANSI spec.
}
