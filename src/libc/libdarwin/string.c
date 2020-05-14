/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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
#include "internal.h"

#pragma mark Definitions
#define __ENEG_ONE (-1)
#define __ENONE 0
#define __EBOGUS (ELAST + 1)

#pragma mark Types
OS_ENUM(errno_flags, uint32_t,
	EDF_INIT = 0,
	EDF_NONPOSIX = (1 << 0),
	EDF_SYSEXIT = (1 << 1),
);

typedef struct _errno_desc {
	int ed_error;
	int ed_sysexit;
	errno_flags_t ed_flags;
	const char *ed_sym;
	const char *ed_str;
} errno_desc_t;

#pragma mark Initializers
#define errno_init(__err, __sysexit, __str) \
	[__err] = { \
		.ed_error = (__err), \
		.ed_sysexit = (__sysexit), \
		.ed_flags = 0, \
		.ed_sym = #__err, \
		.ed_str = (__str), \
	}

#define errno_init_np(__err, __sysexit, __str) \
	[__err - EBASE_NP] = { \
		.ed_error = (__err), \
		.ed_sysexit = (__sysexit), \
		.ed_flags = EDF_NONPOSIX, \
		.ed_sym = #__err, \
		.ed_str = (__str), \
	}

#define sysexit_init(__code, __str) \
	[__code - EX__BASE] = { \
		.ed_error = (__code), \
		.ed_sysexit = (__code), \
		.ed_flags = EDF_SYSEXIT, \
		.ed_sym = #__code, \
		.ed_str = (__str), \
	}

#pragma mark Top-Level Statics
static const errno_desc_t _zero = {
	.ed_error = 0,
	.ed_sysexit = 0,
	.ed_flags = 0,
	.ed_sym = "0",
	.ed_str = "successful termination",
};

static const errno_desc_t _negative_one = {
	.ed_error = __ENEG_ONE,
	.ed_sysexit = EXIT_FAILURE,
	.ed_flags = 0,
	.ed_sym = "-1",
	.ed_str = "[return code instead of error]",
};

static const errno_desc_t _badreceipt = {
	.ed_error = EX_BADRECEIPT_NP,
	.ed_sysexit = EX_BADRECEIPT_NP,
	.ed_flags = EDF_SYSEXIT,
	.ed_sym = "EX_BADRECEIPT_NP",
	.ed_str = "bad purchase receipt",
};

static const errno_desc_t _posix_errors[] = {
	errno_init(__ENONE, EX_OK, "[no error]"),
	errno_init(EPERM, EX_NOPERM, "Operation not permitted"),
	errno_init(ENOENT, EX_NOINPUT, "No such file or directory"),
	errno_init(ESRCH, EX_IOERR, "No such process"),
	errno_init(EINTR, EX_OSERR, "Interrupted system call"),
	errno_init(EIO, EX_IOERR, "Input/output error"),
	errno_init(ENXIO, EX_NOINPUT, "Device not configured"),
	errno_init(E2BIG, EX_IOERR, "Argument list too long"),
	errno_init(ENOEXEC, EX_DATAERR, "Exec format error"),
	errno_init(EBADF, EX_OSERR, "Bad file descriptor"),
	errno_init(ECHILD, EX_IOERR, "No child processes"),
	errno_init(EDEADLK, EX_UNAVAILABLE, "Resource deadlock avoided"),
	errno_init(ENOMEM, EX_TEMPFAIL, "Cannot allocate memory"),
	errno_init(EACCES, EX_NOINPUT, "Permission denied"),
	errno_init(EFAULT, EX_OSERR, "Bad address"),
#if defined(ENOTBLK)
	errno_init(ENOTBLK, EX_IOERR, "Block device required"),
#endif
	errno_init(EBUSY, EX_TEMPFAIL, "Device / Resource busy"),
	errno_init(EEXIST, EX_CANTCREAT, "File exists"),
	errno_init(EXDEV, EX_UNAVAILABLE, "Cross-device link"),
	errno_init(ENODEV, EX_IOERR, "Operation not supported by device"),
	errno_init(ENOTDIR, EX_DATAERR, "Not a directory"),
	errno_init(EISDIR, EX_DATAERR, "Is a directory"),
	errno_init(EINVAL, EX_NOINPUT, "Invalid argument"),
	errno_init(ENFILE, EX_OSERR, "Too many open files in system"),
	errno_init(EMFILE, EX_OSERR, "Too many open files"),
	errno_init(ENOTTY, EX_PROTOCOL, "Inappropriate ioctl for device"),
	errno_init(ETXTBSY, EX_TEMPFAIL, "Text file busy"),
	errno_init(EFBIG, EX_NOINPUT, "File too large"),
	errno_init(ENOSPC, EX_CANTCREAT, "No space left on device"),
	errno_init(ESPIPE, EX_IOERR, "Illegal seek"),
	errno_init(EROFS, EX_NOPERM, "Read-only file system"),
	errno_init(EMLINK, EX_OSERR, "Too many links"),
	errno_init(EPIPE, EX_IOERR, "Broken pipe"),
	errno_init(EDOM, EX_DATAERR, "Numerical argument out of domain"),
	errno_init(ERANGE, EX_DATAERR, "Result too large"),
	errno_init(EAGAIN, EX_TEMPFAIL, "Resource temporarily unavailable"),
	errno_init(EINPROGRESS, EX_TEMPFAIL, "Operation now in progress"),
	errno_init(EALREADY, EX_TEMPFAIL, "Operation already in progress"),
	errno_init(ENOTSOCK, EX_PROTOCOL, "Socket operation on non-socket"),
	errno_init(EDESTADDRREQ, EX_PROTOCOL, "Destination address required"),
	errno_init(EMSGSIZE, EX_PROTOCOL, "Message too long"),
	errno_init(EPROTOTYPE, EX_PROTOCOL, "Protocol wrong type for socket"),
	errno_init(ENOPROTOOPT, EX_PROTOCOL, "Protocol not available"),
	errno_init(EPROTONOSUPPORT, EX_PROTOCOL, "Protocol not supported"),
#if defined(ESOCKTNOSUPPORT)
	errno_init(ESOCKTNOSUPPORT, EX_PROTOCOL, "Socket type not supported"),
#endif
	errno_init(ENOTSUP, EX_UNAVAILABLE, "Operation not supported"),
#if defined(EPFNOSUPPORT)
	errno_init(EPFNOSUPPORT, EX_PROTOCOL, "Protocol family not supported"),
#endif
	errno_init(EAFNOSUPPORT, EX_PROTOCOL,
		"Address family not supported by protocol family"
	),
	errno_init(EADDRINUSE, EX_PROTOCOL, "Address already in use"),
	errno_init(EADDRNOTAVAIL, EX_OSERR, "Can't assign requested address"),
	errno_init(ENETDOWN, EX_NOHOST, "Network is down"),
	errno_init(ENETUNREACH, EX_NOHOST, "Network is unreachable"),
	errno_init(ENETRESET, EX_NOHOST, "Network dropped connection on reset"),
	errno_init(ECONNABORTED, EX_OSERR, "Software caused connection abort"),
	errno_init(ECONNRESET, EX_PROTOCOL, "Connection reset by peer"),
	errno_init(ENOBUFS, EX_TEMPFAIL, "No buffer space available"),
	errno_init(EISCONN, EX_PROTOCOL, "Socket is already connected"),
	errno_init(ENOTCONN, EX_PROTOCOL, "Socket is not connected"),
#if defined(ESHUTDOWN)
	errno_init(ESHUTDOWN, EX_PROTOCOL, "Can't send after socket shutdown"),
	errno_init(ETOOMANYREFS, EX_PROTOCOL, "Too many references: can't splice"),
#endif
	errno_init(ETIMEDOUT, EX_TEMPFAIL, "Operation timed out"),
	errno_init(ECONNREFUSED, EX_PROTOCOL, "Connection refused"),
	errno_init(ELOOP, EX_DATAERR, "Too many levels of symbolic links"),
	errno_init(ENAMETOOLONG, EX_DATAERR, "File name too long"),
#if defined(EHOSTDOWN)
	errno_init(EHOSTDOWN, EX_NOHOST, "Host is down"),
#endif
	errno_init(EHOSTUNREACH, EX_NOHOST, "No route to host"),
	errno_init(ENOTEMPTY, EX_DATAERR, "Directory not empty"),
#if defined(EPROCLIM)
	errno_init(EPROCLIM, EX_TEMPFAIL, "Too many processes"),
	errno_init(EUSERS, EX_NOUSER, "Too many users"),
#endif
	errno_init(EDQUOT, EX_CANTCREAT, "Disc quota exceeded"),
	errno_init(ESTALE, EX_NOINPUT, "Stale NFS file handle"),
#if defined(EREMOTE)
	errno_init(EREMOTE, EX_DATAERR, "Too many levels of remote in path"),
	errno_init(EBADRPC, EX_PROTOCOL, "RPC struct is bad"),
	errno_init(ERPCMISMATCH, EX_PROTOCOL, "RPC version wrong"),
	errno_init(EPROGUNAVAIL, EX_PROTOCOL, "RPC prog. not avail"),
	errno_init(EPROGMISMATCH, EX_PROTOCOL, "Program version wrong"),
	errno_init(EPROCUNAVAIL, EX_PROTOCOL, "Bad procedure for program"),
#endif
	errno_init(ENOLCK, EX_OSERR, "No locks available"),
	errno_init(ENOSYS, EX_UNAVAILABLE, "Function not implemented"),
#if defined(EFTYPE)
	errno_init(EFTYPE, EX_DATAERR, "Inappropriate file type or format"),
	errno_init(EAUTH, EX_NOPERM, "Authentication error"),
	errno_init(ENEEDAUTH, EX_NOPERM, "Need authenticator"),
	errno_init(EPWROFF, EX_UNAVAILABLE, "Device power is off"),
	errno_init(EDEVERR, EX_UNAVAILABLE, "Device error, e.g. paper out"),
#endif
	errno_init(EOVERFLOW, EX_DATAERR,
		"Value too large to be stored in data type"
	),
#if defined(EBADEXEC)
	errno_init(EBADEXEC, EX_DATAERR, "Bad executable"),
	errno_init(EBADARCH, EX_DATAERR, "Bad CPU type in executable"),
	errno_init(ESHLIBVERS, EX_DATAERR, "Shared library version mismatch"),
	errno_init(EBADMACHO, EX_DATAERR, "Malformed Macho file"),
#endif
	errno_init(ECANCELED, EX_UNAVAILABLE, "Operation canceled"),
	errno_init(EIDRM, EX_UNAVAILABLE, "Identifier removed"),
	errno_init(ENOMSG, EX_UNAVAILABLE, "No message of desired type"),
	errno_init(EILSEQ, EX_DATAERR, "Illegal byte sequence"),
#if defined(ENOATTR)
	errno_init(ENOATTR, EX_UNAVAILABLE, "Attribute not found"),
#endif
	errno_init(EBADMSG, EX_PROTOCOL, "Bad message"),
	errno_init(EMULTIHOP, EX_UNAVAILABLE, "Reserved"),
	errno_init(ENODATA, EX_NOINPUT, "No message available on STREAM"),
	errno_init(ENOLINK, EX_UNAVAILABLE, "Reserved"),
	errno_init(ENOSR, EX_TEMPFAIL, "No STREAM resources"),
	errno_init(ENOSTR, EX_PROTOCOL, "Not a STREAM"),
	errno_init(EPROTO, EX_PROTOCOL, "Protocol error"),
	errno_init(ETIME, EX_TEMPFAIL, "STREAM ioctl timeout"),
#if defined(EOPNOTSUPP)
	errno_init(EOPNOTSUPP, EX_PROTOCOL, "Operation not supported on socket"),
#endif
	errno_init(ENOPOLICY, EX_UNAVAILABLE, "No such policy registered"),
#if defined(ENOTRECOVERABLE)
	errno_init(ENOTRECOVERABLE, EX_CONFIG, "State not recoverable"),
	errno_init(EOWNERDEAD, EX_NOHOST, "Previous owner died"),
#endif
#if defined(EQFULL)
	errno_init(EQFULL, EX_CANTCREAT, "Interface output queue is full"),
#endif
	errno_init(__EBOGUS, EX_UNAVAILABLE, "[invalid]"),
};

static const errno_desc_t _darwin_errors[] = {
	errno_init_np(ENOTENTITLED_NP, EX_NOPERM, "Requestor not entitled"),
	errno_init_np(ENOTPLATFORM_NP, EX_NOPERM, "Requestor not platform binary"),
	errno_init_np(EROOTLESS_NP, EX_NOPERM,
		"Operation disallowed by System Integrity Protection"
	),
	errno_init_np(ETAINTED_NP, EX_NOPERM,
		"Requestor is tainted and untrusthworthy"
	),
	errno_init_np(EQUARANTINE_NP, EX_NOPERM,
		"Operation not permitted on quarantined file"
	),
	errno_init_np(EBADUSER_NP, EX_NOINPUT, "Invalid username or identifier"),
	errno_init_np(EBADGROUP_NP, EX_NOINPUT, "Invalid group name or identifier"),
	errno_init_np(EOWNERSHIP_NP, EX_NOINPUT,
		"Ownership and/or permissions insufficiently restrictive"
	),
	errno_init_np(ENOOOO_NP, EX_PROTOCOL, "Out-of-order operation rejected"),
	errno_init_np(ENOTBUNDLE_NP, EX_NOINPUT, "Entity not a bundle"),
	errno_init_np(EBADBUNDLE_NP, EX_DATAERR, "Malformed bundle"),
	errno_init_np(EBADPATH_NP, EX_DATAERR, "Malformed path"),
	errno_init_np(EBADPLIST_NP, EX_DATAERR, "Malformed property list"),
	errno_init_np(EBADKEY_NP, EX_DATAERR, "Unrecognized key"),
	errno_init_np(EBADVAL_NP, EX_DATAERR, "Invalid or missing value for key"),
	errno_init_np(EBADSUBSYS_NP, EX_DATAERR,
		"Routine referenced an invalid subsystem"
	),
	errno_init_np(E2BIMPL_NP, EX_UNAVAILABLE, "Routine not yet implemented"),
	errno_init_np(EDEPRECATED_NP, EX_UNAVAILABLE, "Routine deprecated"),
	errno_init_np(EREMOVED_NP, EX_UNAVAILABLE, "Routine removed"),
	errno_init_np(EDROPPED_NP, EX_PROTOCOL, "Routine invocation dropped"),
	errno_init_np(EDEFERRED_NP, EX_TEMPFAIL, "Routine invocation deferred"),
	errno_init_np(EUSAGE_NP, EX_USAGE, "Bad command line usage"),
	errno_init_np(EUNKNOWN_NP, EX_UNAVAILABLE,
		"Unknown error from another subsystem"
	),
};

static const errno_desc_t _sysexits[] = {
	sysexit_init(EX_USAGE, "command line usage error"),
	sysexit_init(EX_DATAERR, "data format error"),
	sysexit_init(EX_NOINPUT, "cannot open input"),
	sysexit_init(EX_NOUSER, "addressee unknown"),
	sysexit_init(EX_NOHOST, "host name unknown"),
	sysexit_init(EX_UNAVAILABLE, "service unavailable"),
	sysexit_init(EX_SOFTWARE, "internal software error"),
	sysexit_init(EX_OSERR, "system error (e.g., can't fork)"),
	sysexit_init(EX_OSFILE, "critical OS file missing"),
	sysexit_init(EX_CANTCREAT, "can't create (user) output file"),
	sysexit_init(EX_IOERR, "input/output error"),
	sysexit_init(EX_TEMPFAIL, "temp failure"),
	sysexit_init(EX_PROTOCOL, "remote error in protocol"),
	sysexit_init(EX_NOPERM, "permission denied"),
	sysexit_init(EX_CONFIG, "configuration error"),
};

#pragma mark Utilities
static const errno_desc_t *
_find_error(int code)
{
	if (code < 0) {
		return &_negative_one;
	}

	if (code >= EBASE_NP && code <= __ELAST_NP) {
		return &_darwin_errors[code - EBASE_NP];
	}

	if (code >= 0 && code <= ELAST) {
		return &_posix_errors[code];
	}

	return &_posix_errors[__EBOGUS];
}

static const errno_desc_t *
_find_sysexit(int code)
{
	if (code == 0) {
		return &_zero;
	}

	if (code == EX_BADRECEIPT_NP) {
		return &_badreceipt;
	}

	if (code < EX__BASE || code > EX__MAX) {
		return &_sysexits[EX_UNAVAILABLE - EX__BASE];
	}

	return &_sysexits[code - EX__BASE];
}

static void
_writeflag(const char *humanflag, const char *sep, char *buff, size_t len,
		char **track_buff, size_t *track_sz)
{
	size_t offs = 0;

	if (*track_buff != buff) {
		offs = zsnprintf_np(*track_buff, *track_sz, "%s", sep);
		*track_buff += offs;
		*track_sz -= offs;
	}

	offs = zsnprintf_np(*track_buff, *track_sz, "%s", humanflag);
	*track_buff += offs;
	*track_sz -= offs;
}

#pragma mark API
int
sysexit_np(int code)
{
	const errno_desc_t *de = _find_error(code);
	return de->ed_sysexit;
}

const char *
strerror_np(int code)
{
	if (code >= 0 && code <= ELAST) {
		return strerror(code);
	}

	return _find_error(code)->ed_str;
}

const char *
strexit_np(int code)
{
	const errno_desc_t *de = _find_sysexit(code);
	return de->ed_str;
}

const char *
symerror_np(int code)
{
	const errno_desc_t *de = _find_error(code);
	return de->ed_sym;
}

const char *
symexit_np(int code)
{
	const errno_desc_t *de = _find_sysexit(code);
	return de->ed_sym;
}

char *
os_flagset_copy_string(const os_flagset_t flagset, uint64_t flags)
{
	size_t i = 0;
	const os_flag_t *fi = NULL;
	char buff[1024] = "[none]";
	char *cursor = buff;
	size_t left = sizeof(buff);

	while (i < OS_FLAGSET_COUNT && (fi = &flagset[i]) && fi->ohf_flag) {
		if (flags & fi->ohf_flag) {
			_writeflag(fi->ohf_human_flag, "|", buff, sizeof(buff),
					&cursor, &left);
		}
		i++;
	}

	return os_strdup(buff);
}

