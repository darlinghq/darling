/*-
 * Copyright (c) 2009 Apple Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * $P4: //depot/projects/trustedbsd/openbsm/bsm/auditd_lib.h#4 $
 */

#ifndef _BSM_AUDIT_SESSION_H_
#define	_BSM_AUDIT_SESSION_H_

#include <inttypes.h>		/* Required for audit.h. */
#include <stdio.h>		/* Required for FILE. */

#include <bsm/audit.h>
#include <bsm/audit_kevents.h>	/* Required for AUE_SESSION_* event def's. */

#include <os/availability.h>

/* Defined audit session flags for the ai_flags member of auditinfo_addr.
 * These are opaque to XNU itself, although some may be of interest to certain
 * kernel extensions, notably AU_SESSION_FLAG_HAS_CONSOLE_ACCESS.
 */
enum audit_session_flags {
        /* The initial session created by PID 1. */
        AU_SESSION_FLAG_IS_INITIAL         = 0x0001,

        /* The graphics subsystem (CoreGraphics, etc.) is available. */
        AU_SESSION_FLAG_HAS_GRAPHIC_ACCESS = 0x0010,

        /* /dev/tty is available. */
        AU_SESSION_FLAG_HAS_TTY            = 0x0020,

        /* The session was created for a remote connection. */
        AU_SESSION_FLAG_IS_REMOTE          = 0x1000,

        /* The console and associated devices are available. */
        AU_SESSION_FLAG_HAS_CONSOLE_ACCESS = 0x2000,

        /* An active, authenticated user is associated with the session. */
        AU_SESSION_FLAG_HAS_AUTHENTICATED =  0x4000,
};

/*
 * Audit session device.
 */

#define	AUDIT_SDEV_PATH		"/dev/auditsessions"

/*
 * au_sdev_open() flags
 */
enum au_sdev_open_flags {
	/* Set audit session device to not to block on reads. */
	AU_SDEVF_NONBLOCK		= 0x00000001,


	/* Allow process to monitor all session. (Requires privilege.) */
	AU_SDEVF_ALLSESSIONS		= 0x00010000,
};

__BEGIN_DECLS
/*
 * Audit session device handle.
 */
typedef struct au_sdev_handle {
	FILE		*ash_fp;
	u_char		*ash_buf;
	int		 ash_reclen;
	int		 ash_bytesread;
} au_sdev_handle_t;

/*
 * au_sdev_open()
 *
 * @summary - Open the audit session pseudo device.
 *
 * @param flags - Flags that change the behavior of the device.  The flags
 * specified are formed by or'ing the following flag: AU_SDEVF_NONBLOCK for
 * non-blocking I/O and AU_SDEF_ALLSESSIONS for monitoring all the sessions
 * and not just the session of the current process.
 *
 * @return Upon success returns the audit session device handle.  Otherwise,
 * NULL is returned and the errno is set to indicate the error.
 */
au_sdev_handle_t *au_sdev_open(int flags)
	API_AVAILABLE(macos(10.8)) API_UNAVAILABLE(ios, watchos, tvos);

/*
 * au_sdev_close()
 *
 * @summary - Close the audit session pseudo device.
 *
 * @param ash - Audit session device handle.
 *
 * @return Upon successful completion 0 is returned.  Otherwise, errno is set
 * to indicate the error.
 */
int au_sdev_close(au_sdev_handle_t *ash)
	API_AVAILABLE(macos(10.8)) API_UNAVAILABLE(ios, watchos, tvos);

/*
 * au_sdev_fd()
 *
 * @summary - Get the file descriptor for the audit session device.
 *
 * @param ash - Audit session device handle.
 *
 * @return File descriptor of the audit session device.
 */
int au_sdev_fd(au_sdev_handle_t *ash)
	API_AVAILABLE(macos(10.8)) API_UNAVAILABLE(ios, watchos, tvos);

/*
 * au_sdev_read_aia()
 *
 * @summary - Read a session event and an auditinfo_addr record from kernel.
 *
 * @param ash - Audit session device handle.
 *
 * @param event - A pointer to an integer that will contain the event type:
 * AUE_SESSION_START (start of a new session), AUE_SESSION_UPDATE (the 
 * session information has been changed), AUE_SESSION_END (all the processes in
 * the session have exited), and AUE_SESSION_CLOSE (the session record has been
 * removed from the kernel).
 *
 * @param aia_p - A pointer to an auditinfo_addr structure that will contain the
 * audit session information on a successful return. The audit masks fields
 * (ai_mask), however, does not currently contain correct informaiton.
 *
 * @return Upon sucessful completetion 0 is returned and the event and aia_p 
 * parameters will be populated.  Otherwise, errno is set to indicate the error.
 */
int au_sdev_read_aia(au_sdev_handle_t *ash, int *event, auditinfo_addr_t *aia_p)
	API_AVAILABLE(macos(10.8)) API_UNAVAILABLE(ios, watchos, tvos);

__END_DECLS

#endif /* !_BSM_AUDIT_SESSION_H_ */
