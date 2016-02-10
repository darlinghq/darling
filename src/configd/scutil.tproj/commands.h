/*
 * Copyright (c) 2000, 2001, 2004-2006 Apple Computer, Inc. All rights reserved.
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
 * June 1, 2001			Allan Nathanson <ajn@apple.com>
 * - public API conversion
 *
 * November 9, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#ifndef _COMMANDS_H
#define _COMMANDS_H

#include <sys/cdefs.h>

typedef struct {
	char	*cmd;
	int	minArgs;
	int	maxArgs;
	void	(*func)(int argc, char **argv);
	int	group;
	int	ctype;	/* -1==normal/hidden, 0==normal, 1==limited, 2==private */
	char	*usage;
} cmdInfo;

extern const cmdInfo	commands_store[];
extern const int	nCommands_store;

extern const cmdInfo	commands_net[];
extern const int	nCommands_net;

extern const cmdInfo	commands_prefs[];
extern const int	nCommands_prefs;

extern cmdInfo		*commands;
extern int		nCommands;
extern Boolean		enablePrivateAPI;
extern Boolean		termRequested;

__BEGIN_DECLS

void	do_command		(int argc, char **argv);
void	do_help			(int argc, char **argv);
void	do_quit			(int argc, char **argv);
void	do_readFile		(int argc, char **argv);

__END_DECLS

#endif /* !_COMMANDS_H */
