/*
 * Copyright (c) 2004, 2006, 2011, 2014 Apple Inc. All rights reserved.
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
 * August 5, 2004			Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#ifndef _NET_H
#define _NET_H

#include <sys/cdefs.h>

#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>


typedef int (*optionHandler) (CFStringRef		key,
			      const char		*description,
			      void			*info,
			      int			argc,
			      char			**argv,
			      CFMutableDictionaryRef	newConfiguration);

typedef enum {
	isOther,		// use "only" handler function for processing
	isHelp,
	isChooseOne,
	isChooseMultiple,
	isBoolean,
	isNumber,
	isString,
	isStringArray
} optionType;

typedef const struct {
	const CFStringRef	selection;
	const CFStringRef	*key;
	const unsigned int	flags;
} selections;
#define selectionNotAvailable	1<<0	// if you can't "choose" this selection

typedef const struct {
	const char		*option;
	const char		*description;
	optionType		type;
	const CFStringRef	*key;
	optionHandler		handler;
	void			*info;
} options, *optionsRef;


extern CFMutableArrayRef	new_interfaces;

extern CFArrayRef		interfaces;
extern CFArrayRef		services;
extern CFArrayRef		protocols;
extern CFArrayRef		sets;

extern SCNetworkInterfaceRef	net_interface;
extern SCNetworkServiceRef	net_service;
extern SCNetworkProtocolRef	net_protocol;
extern SCNetworkSetRef		net_set;

extern CFNumberRef		CFNumberRef_0;
extern CFNumberRef		CFNumberRef_1;


__BEGIN_DECLS

Boolean		_process_options(optionsRef		options,
				 int			nOptions,
				 int			argc,
				 char			**argv,
				 CFMutableDictionaryRef	newConfiguration);

CF_RETURNS_RETAINED
CFNumberRef	_copy_number	(const char *arg);

CFIndex		_find_option	(const char	*option,
				 optionsRef	options,
				 const int	nOptions);

CFIndex		_find_selection	(CFStringRef 	choice,
				 selections	choises[],
				 unsigned int	*flags);

void		_show_entity	(CFDictionaryRef entity, CFStringRef prefix);

void		do_net_init	(void);
void		do_net_quit	(int argc, char **argv);

void		do_net_open	(int argc, char **argv);
void		do_net_commit	(int argc, char **argv);
void		do_net_apply	(int argc, char **argv);
void		do_net_close	(int argc, char **argv);

void		do_net_create	(int argc, char **argv);
void		do_net_disable	(int argc, char **argv);
void		do_net_enable	(int argc, char **argv);
void		do_net_migrate	(int argc, char **argv);
void		do_net_remove	(int argc, char **argv);
void		do_net_select	(int argc, char **argv);
void		do_net_set	(int argc, char **argv);
void		do_net_show	(int argc, char **argv);
void		do_net_update	(int argc, char **argv);

void		do_net_snapshot	(int argc, char **argv);

__END_DECLS

#endif /* !_NET_H */
