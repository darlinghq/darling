/*
 * Copyright (c) 2000-2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */
/*-
 * Copyright (c) 1982, 1986, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)syslog.h	8.1 (Berkeley) 6/2/93
 * $FreeBSD: src/sys/sys/syslog.h,v 1.27.2.1.4.1 2010/06/14 02:09:06 kensmith Exp $
 */

#ifndef _SYS_SYSLOG_H_
#define _SYS_SYSLOG_H_

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>

#define _PATH_LOG       "/var/run/syslog"

/*
 * priorities/facilities are encoded into a single 32-bit quantity, where the
 * bottom 3 bits are the priority (0-7) and the top 28 bits are the facility
 * (0-big number).  Both the priorities and the facilities map roughly
 * one-to-one to strings in the syslogd(8) source code.  This mapping is
 * included in this file.
 *
 * priorities (these are ordered)
 */
#define LOG_EMERG       0       /* system is unusable */
#define LOG_ALERT       1       /* action must be taken immediately */
#define LOG_CRIT        2       /* critical conditions */
#define LOG_ERR         3       /* error conditions */
#define LOG_WARNING     4       /* warning conditions */
#define LOG_NOTICE      5       /* normal but significant condition */
#define LOG_INFO        6       /* informational */
#define LOG_DEBUG       7       /* debug-level messages */

#define LOG_PRIMASK     0x07    /* mask to extract priority part (internal) */
/* extract priority */
#define LOG_PRI(p)      ((p) & LOG_PRIMASK)
#define LOG_MAKEPRI(fac, pri)   ((fac) | (pri))

#ifdef SYSLOG_NAMES
#define INTERNAL_NOPRI  0x10    /* the "no priority" priority */
/* mark "facility" */
#define INTERNAL_MARK   LOG_MAKEPRI((LOG_NFACILITIES<<3), 0)
typedef struct _code {
	const char      *c_name;
	int             c_val;
} CODE;

CODE prioritynames[] = {
	{ "alert", LOG_ALERT, },
	{ "crit", LOG_CRIT, },
	{ "debug", LOG_DEBUG, },
	{ "emerg", LOG_EMERG, },
	{ "err", LOG_ERR, },
	{ "error", LOG_ERR, },                  /* DEPRECATED */
	{ "info", LOG_INFO, },
	{ "none", INTERNAL_NOPRI, },            /* INTERNAL */
	{ "notice", LOG_NOTICE, },
	{ "panic", LOG_EMERG, },                /* DEPRECATED */
	{ "warn", LOG_WARNING, },               /* DEPRECATED */
	{ "warning", LOG_WARNING, },
	{ NULL, -1, }
};
#endif

/* facility codes */
#define LOG_KERN        (0<<3)  /* kernel messages */
#define LOG_USER        (1<<3)  /* random user-level messages */
#define LOG_MAIL        (2<<3)  /* mail system */
#define LOG_DAEMON      (3<<3)  /* system daemons */
#define LOG_AUTH        (4<<3)  /* authorization messages */
#define LOG_SYSLOG      (5<<3)  /* messages generated internally by syslogd */
#define LOG_LPR         (6<<3)  /* line printer subsystem */
#define LOG_NEWS        (7<<3)  /* network news subsystem */
#define LOG_UUCP        (8<<3)  /* UUCP subsystem */
#define LOG_CRON        (9<<3)  /* clock daemon */
#define LOG_AUTHPRIV    (10<<3) /* authorization messages (private) */
/* Facility #10 clashes in DEC UNIX, where */
/* it's defined as LOG_MEGASAFE for AdvFS  */
/* event logging.                          */
#define LOG_FTP         (11<<3) /* ftp daemon */
//#define	LOG_NTP		(12<<3)	/* NTP subsystem */
//#define	LOG_SECURITY	(13<<3) /* security subsystems (firewalling, etc.) */
//#define	LOG_CONSOLE	(14<<3) /* /dev/console output */
#define LOG_NETINFO     (12<<3) /* NetInfo */
#define LOG_REMOTEAUTH  (13<<3) /* remote authentication/authorization */
#define LOG_INSTALL     (14<<3) /* installer subsystem */
#define LOG_RAS         (15<<3) /* Remote Access Service (VPN / PPP) */

/* other codes through 15 reserved for system use */
#define LOG_LOCAL0      (16<<3) /* reserved for local use */
#define LOG_LOCAL1      (17<<3) /* reserved for local use */
#define LOG_LOCAL2      (18<<3) /* reserved for local use */
#define LOG_LOCAL3      (19<<3) /* reserved for local use */
#define LOG_LOCAL4      (20<<3) /* reserved for local use */
#define LOG_LOCAL5      (21<<3) /* reserved for local use */
#define LOG_LOCAL6      (22<<3) /* reserved for local use */
#define LOG_LOCAL7      (23<<3) /* reserved for local use */

#define LOG_LAUNCHD     (24<<3) /* launchd - general bootstrap daemon */

#define LOG_NFACILITIES 25      /* current number of facilities */
#define LOG_FACMASK     0x03f8  /* mask to extract facility part */
/* facility of pri */
#define LOG_FAC(p)      (((p) & LOG_FACMASK) >> 3)

#ifdef SYSLOG_NAMES
CODE facilitynames[] = {
	{ "auth", LOG_AUTH, },
	{ "authpriv", LOG_AUTHPRIV, },
	{ "cron", LOG_CRON, },
	{ "daemon", LOG_DAEMON, },
	{ "ftp", LOG_FTP, },
	{ "install", LOG_INSTALL     },
	{ "kern", LOG_KERN, },
	{ "lpr", LOG_LPR, },
	{ "mail", LOG_MAIL, },
	{ "mark", INTERNAL_MARK, },             /* INTERNAL */
	{ "netinfo", LOG_NETINFO, },
	{ "ras", LOG_RAS         },
	{ "remoteauth", LOG_REMOTEAUTH  },
	{ "news", LOG_NEWS, },
	{ "security", LOG_AUTH        },        /* DEPRECATED */
	{ "syslog", LOG_SYSLOG, },
	{ "user", LOG_USER, },
	{ "uucp", LOG_UUCP, },
	{ "local0", LOG_LOCAL0, },
	{ "local1", LOG_LOCAL1, },
	{ "local2", LOG_LOCAL2, },
	{ "local3", LOG_LOCAL3, },
	{ "local4", LOG_LOCAL4, },
	{ "local5", LOG_LOCAL5, },
	{ "local6", LOG_LOCAL6, },
	{ "local7", LOG_LOCAL7, },
	{ "launchd", LOG_LAUNCHD     },
	{ NULL, -1, }
};
#endif

#ifdef KERNEL
#ifdef __APPLE_API_PRIVATE
#define LOG_PRINTF      -1      /* pseudo-priority to indicate use of printf */
#endif /* __APPLE_API_PRIVATE */
#endif

/*
 * arguments to setlogmask.
 */
#define LOG_MASK(pri)   (1 << (pri))            /* mask for one priority */
#define LOG_UPTO(pri)   ((1 << ((pri)+1)) - 1)  /* all priorities through pri */

/*
 * Option flags for openlog.
 *
 * LOG_ODELAY no longer does anything.
 * LOG_NDELAY is the inverse of what it used to be.
 */
#define LOG_PID         0x01    /* log the pid with each message */
#define LOG_CONS        0x02    /* log on the console if errors in sending */
#define LOG_ODELAY      0x04    /* delay open until first syslog() (default) */
#define LOG_NDELAY      0x08    /* don't delay open */
#define LOG_NOWAIT      0x10    /* don't wait for console forks: DEPRECATED */
#define LOG_PERROR      0x20    /* log to stderr as well */

#ifndef KERNEL

/*
 * Don't use va_list in the vsyslog() prototype.   Va_list is typedef'd in two
 * places (<machine/varargs.h> and <machine/stdarg.h>), so if we include one
 * of them here we may collide with the utility's includes.  It's unreasonable
 * for utilities to have to include one of them to include syslog.h, so we get
 * __va_list from <sys/_types.h> and use it.
 */
#include <sys/_types.h>

__BEGIN_DECLS
void    closelog(void);
void    openlog(const char *, int, int);
int     setlogmask(int);
#if defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && __DARWIN_C_LEVEL >= __DARWIN_C_FULL
void    syslog(int, const char *, ...) __DARWIN_ALIAS_STARTING(__MAC_10_13, __IPHONE_NA, __DARWIN_EXTSN(syslog)) __printflike(2, 3) __not_tail_called;
#else
void    syslog(int, const char *, ...) __printflike(2, 3) __not_tail_called;
#endif
#if __DARWIN_C_LEVEL >= __DARWIN_C_FULL
void    vsyslog(int, const char *, __darwin_va_list) __printflike(2, 0) __not_tail_called;
#endif
__END_DECLS

#else /* !KERNEL */
#ifdef __APPLE_API_OBSOLETE
/*
 * bit field descriptions for printf %r and %R formats
 */

/*
 * printf("%r %R", val, reg_descp);
 * struct reg_desc *reg_descp;
 *
 * the %r and %R formats allow formatted output of bit fields.
 * reg_descp points to an array of reg_desc structures, each element of the
 * array describes a range of bits within val.  the array should have a
 * final element with all structure elements 0.
 * %r outputs a string of the format "<bit field descriptions>"
 * %R outputs a string of the format "0x%x<bit field descriptions>"
 *
 * The fields in a reg_desc are:
 *	unsigned rd_mask;	An appropriate mask to isolate the bit field
 *				within a word, and'ed with val
 *
 *	int rd_shift;		A shift amount to be done to the isolated
 *				bit field.  done before printing the isolate
 *				bit field with rd_format and before searching
 *				for symbolic value names in rd_values
 *
 *	char *rd_name;		If non-null, a bit field name to label any
 *				out from rd_format or searching rd_values.
 *				if neither rd_format or rd_values is non-null
 *				rd_name is printed only if the isolated
 *				bit field is non-null.
 *
 *	char *rd_format;	If non-null, the shifted bit field value
 *				is printed using this format.
 *
 *	struct reg_values *rd_values;	If non-null, a pointer to a table
 *				matching numeric values with symbolic names.
 *				rd_values are searched and the symbolic
 *				value is printed if a match is found, if no
 *				match is found "???" is printed.
 *
 * printf("%n %N", val, reg_valuesp);
 * struct reg_values *reg_valuesp;
 *
 * the %n and %N formats allow formatted output of symbolic constants
 * Reg_valuesp is a pointer to an array of struct reg_values which pairs
 * numeric values (rv_value) with symbolic names (rv_name).  The array is
 * terminated with a reg_values entry that has a null pointer for the
 * rv_name field.  When %n or %N is used rd_values are searched and the
 * symbolic value is printed if a match is found, if no match is found
 * "???" is printed.
 *
 * printf("%C", val);
 * int val;
 *
 * the %C format prints an int as a 4 character string.
 * The most significant byte of the int is printed first, the least
 * significant byte is printed last.
 */

/*
 * register values
 * map between numeric values and symbolic values
 */
struct reg_values {
	unsigned rv_value;
	char *rv_name;
};

/*
 * register descriptors are used for formatted prints of register values
 * rd_mask and rd_shift must be defined, other entries may be null
 */
struct reg_desc {
	unsigned rd_mask;       /* mask to extract field */
	int rd_shift;           /* shift for extracted value, - >>, + << */
	char *rd_name;          /* field name */
	char *rd_format;        /* format to print field */
	struct reg_values *rd_values;   /* symbolic names of values */
};

#endif /* __APPLE_API_OBSOLETE */

#include <stdarg.h>
__BEGIN_DECLS
void    log(int, const char *, ...);
#ifdef XNU_KERNEL_PRIVATE
int     vaddlog(const char *, va_list) __printflike(1, 0);
void    logtime(time_t);
#endif /* XNU_KERNEL_PRIVATE */

__END_DECLS

#endif /* !KERNEL */
#endif /* !_SYS_SYSLOG_H_ */
