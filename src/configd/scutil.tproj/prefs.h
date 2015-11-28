/*
 * Copyright (c) 2003, 2005-2007, 2012, 2013 Apple Inc. All rights reserved.
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
 * May 29, 2003			Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#ifndef _PREFS_H
#define _PREFS_H

#include <sys/cdefs.h>
#include <SystemConfiguration/SystemConfiguration.h>


Boolean	_prefs_changed;


__BEGIN_DECLS

AuthorizationRef	_prefs_AuthorizationCreate	(void);
void			_prefs_AuthorizationFree	(AuthorizationRef authorization);

Boolean	_prefs_open		(CFStringRef name, CFStringRef prefsID);
void	_prefs_save		(void);
void	_prefs_close		(void);
Boolean	_prefs_commitRequired	(int argc, char **argv, const char *command);

int	findPref		(char *pref);
void	do_getPref		(char *pref, int argc, char **argv);
void	do_setPref		(char *pref, int argc, char **argv);

void	do_prefs_init		(void);
void	do_prefs_quit		(int argc, char **argv);

void	do_prefs_open		(int argc, char **argv);
void	do_prefs_lock		(int argc, char **argv);
void	do_prefs_unlock		(int argc, char **argv);
void	do_prefs_commit		(int argc, char **argv);
void	do_prefs_apply		(int argc, char **argv);
void	do_prefs_close		(int argc, char **argv);
void	do_prefs_synchronize	(int argc, char **argv);

void	do_prefs_list		(int argc, char **argv);
void	do_prefs_get		(int argc, char **argv);
void	do_prefs_set		(int argc, char **argv);
void	do_prefs_remove		(int argc, char **argv);

void	do_log			(char *pref, int argc, char **argv);
__END_DECLS

#endif /* !_PREFS_H */
