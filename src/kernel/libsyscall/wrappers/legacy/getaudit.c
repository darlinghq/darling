/*
 * Copyright (c) 2011 Apple Computer, Inc. All rights reserved.
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

#include <bsm/audit.h>

/*
 * Wrappers for the legacy getaudit() and setaudit() syscalls.
 */

int
getaudit(struct auditinfo *ainfo)
{
	int err;
	auditinfo_addr_t aia;

	if ((err = getaudit_addr(&aia, sizeof(aia))) != 0)
		return (err);

	ainfo->ai_auid = aia.ai_auid;
	ainfo->ai_mask = aia.ai_mask;
	ainfo->ai_termid.port = aia.ai_termid.at_port;
	ainfo->ai_termid.machine = aia.ai_termid.at_addr[0];
	ainfo->ai_asid = aia.ai_asid;

	return (0);
}

int
setaudit(const struct auditinfo *ainfo)
{
	int err;
	struct auditinfo *ai = (struct auditinfo *)ainfo;
	auditinfo_addr_t aia;

	/* Get the current ai_flags so they are preserved. */
	if ((err = getaudit_addr(&aia, sizeof(aia))) != 0)
		return (err);

	aia.ai_auid = ai->ai_auid;
	aia.ai_mask = ai->ai_mask;
	aia.ai_termid.at_port = ai->ai_termid.port;
	aia.ai_termid.at_type = AU_IPv4;
	aia.ai_termid.at_addr[0] = ai->ai_termid.machine;
	aia.ai_asid = ai->ai_asid;

	if ((err = setaudit_addr(&aia, sizeof(aia))) != 0)
		return (err);

	/* The session ID may have been assigned by kernel so copy that back. */
	ai->ai_asid = aia.ai_asid;

	return (0);
}
