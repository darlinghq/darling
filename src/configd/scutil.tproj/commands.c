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
 * June 1, 2001			Allan Nathanson <ajn@apple.com>
 * - public API conversion
 *
 * November 9, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

#include "scutil.h"
#include "commands.h"
#include "dictionary.h"
#include "session.h"
#include "cache.h"
#include "notifications.h"
#include "tests.h"
#include "net.h"
#include "prefs.h"


__private_extern__
const cmdInfo commands_store[] = {
	/* cmd		minArgs	maxArgs	func			group	ctype			*/
	/* 	usage										*/

	{ "help",	0,	0,	do_help,		0,	0,
		" help                          : list available commands"			},

	{ "f.read",	1,	1,	do_readFile,		0,	0,
		" f.read file                   : process commands from file"			},

	{ "quit",	0,	0,	do_quit,		0,	0,
		" quit                          : quit"						},

	{ "q",		0,	0,	do_quit,		0,	-1,
		NULL										},

	{ "exit",	0,	0,	do_quit,		0,	-1,
		NULL										},

	/* local dictionary manipulation commands */

	{ "d.init",	0,	0,	do_dictInit,		1,	0,
		" d.init                        : initialize (empty) dictionary"		},

	{ "d.show",	0,	0,	do_dictShow,		1,	0,
		" d.show                        : show dictionary contents"			},

	{ "d.add",	2,	101,	do_dictSetKey,		1,	0,
		" d.add key [*#?] val [v2 ...]  : add information to dictionary\n"
		"       (*=array, #=number, ?=boolean, %=hex data)"		                },

	{ "d.remove",	1,	1,	do_dictRemoveKey,	1,	0,
		" d.remove key                  : remove key from dictionary"			},

	/* dynamic store manipulation commands */

	{ "open",	0,	1,	do_open,		2,	1,
		" open [\"temporary\"]            : open a session with \"configd\""		},

	{ "close",	0,	0,	do_close,		2,	1,
		" close                         : close current \"configd\" session"		},

	{ "block",	0,	1,	do_block,		3,	1,
		" block [\"begin\" | \"end\"]     : block multiple data store transactions"	},

	{ "list",	0,	2,	do_list,		4,	0,
		" list [pattern]                : list keys in data store"			},

	{ "add",	1,	2,	do_add,			4,	0,
		" add key [\"temporary\"]         : add key in data store w/current dict"	},

	{ "get",	1,	1,	do_get,			4,	0,
		" get key                       : get dict from data store w/key"		},

	{ "set",	1,	1,	do_set,			4,	0,
		" set key                       : set key in data store w/current dict"		},

	{ "show",	1,	2,	do_show,		4,	0,
		" show key [\"pattern\"]          : show values in data store w/key"		},

	{ "remove",	1,	1,	do_remove,		4,	0,
		" remove key                    : remove key from data store"			},

	{ "notify",	1,	1,	do_notify,		4,	0,
		" notify key                    : notify key in data store"			},

	{ "n.list",	0,	1,	do_notify_list,		5,	0,
		" n.list [\"pattern\"]            : list notification keys"			},

	{ "n.add",	1,	2,	do_notify_add,		5,	0,
		" n.add key [\"pattern\"]         : add notification key"			},

	{ "n.remove",	1,	2,	do_notify_remove,	5,	0,
		" n.remove key [\"pattern\"]      : remove notification key"			},

	{ "n.changes",	0,	0,	do_notify_changes,	5,	0,
		" n.changes                     : list changed keys"				},

	{ "n.watch",	0,	0,	do_notify_watch,	5,	0,
		" n.watch                       : watch for changes"				},

	{ "n.wait",	0,	0,	do_notify_wait,		5,	2,
		" n.wait                        : wait for changes"				},

	{ "n.signal",	1,	2,	do_notify_signal,	5,	2,
		" n.signal sig [pid]            : signal changes"				},

	{ "n.file",	0,	1,	do_notify_file,		5,	2,
		" n.file [identifier]           : watch for changes via file"			},

	{ "n.cancel",	0,	1,	do_notify_cancel,	5,	0,
		" n.cancel                      : cancel notification requests"			},

	{ "snapshot",	0,	0,	do_snapshot,		99,	2,
		" snapshot                      : save snapshot of store and session data"	}
};
__private_extern__
const int nCommands_store = (sizeof(commands_store)/sizeof(cmdInfo));


__private_extern__
const cmdInfo commands_net[] = {
	/* cmd		minArgs	maxArgs	func			group	ctype			*/
	/* 	usage										*/

	{ "help",	0,	0,	do_help,		0,	0,
		" help                          : list available commands"			},

	{ "f.read",	1,	1,	do_readFile,		0,	0,
		" f.read file                   : process commands from file"			},

	{ "quit",	0,	1,	do_net_quit,		0,	0,
		" quit [!]                      : quit"						},

	{ "q",		0,	1,	do_net_quit,		0,	-1,
		NULL										},

	{ "exit",	0,	1,	do_net_quit,		0,	-1,
		NULL										},

	/* network configuration manipulation commands */

	{ "open",	0,	1,	do_net_open,		2,	1,
		" open                          : open the network configuration"		},

	{ "commit",	0,	0,	do_net_commit,		2,	0,
		" commit                        : commit any changes"				},

	{ "apply",	0,	0,	do_net_apply,		2,	0,
		" apply                         : apply any changes"				},

	{ "close",	0,	1,	do_net_close,		2,	1,
		" close [!]                     : close the network configuration"		},

	{ "create",	1,	3, 	do_net_create,		3,	0,
		" create interface <interfaceType> [ <interfaceName> | <interface#> ]\n"
		" create protocol <protocolType>\n"
		" create service [ <interfaceName> | <interface#> [ <serviceName> ]]\n"
		" create set [setName]"								},

	{ "disable",	1,	2,	do_net_disable,		4,	0,
		" disable protocol [ <protocolType> ]\n"
		" disable service  [ <serviceName> | <service#> ]"				},

	{ "enable",	1,	2,	do_net_enable,		5,	0,
		" enable protocol  [ <protocolType> ]\n"
		" enable service   [ <serviceName> | <service#> ]"				},

	{ "migrate",	2,	5,	do_net_migrate,		11,	0,
		" migrate perform <old-configuration> <new-configuration> [<current-configuration>]\n"
		" migrate validate <configuration> <expected-configuration>"},

	{ "remove",	1,	2,	do_net_remove,		6,	0,
		" remove protocol  [ <protocolType> ]\n"
		" remove service   [ <serviceName> | <service#> ]\n"
		" remove set       [ <setName> | <set#> ]"					},

	{ "select",	2,	3,	do_net_select,		7,	0,
		" select interface <interfaceName> | <interface#> | $child | $service | $vlan | $bond <memberName> | $bridge <memberName>\n"
		" select protocol  <protocolType>\n"
		" select service   <serviceName> | <service#>\n"
		" select set       <setName> | <set#>"						},

	{ "set",	2,	101,	do_net_set,		8,	0,
		" set interface context-sensitive-arguments (or ? for help)\n"
		" set protocol  context-sensitive-arguments (or ? for help)\n"
		" set service   [ name <serviceName> ] [ order new-order ] [ rank ("" | First | Last | Never | Scoped ) [temp] ] [ id <serviceID> ]\n"
		" set set       [ name setName ] | [ current ] [ id <setID> ]"			},

	{ "show",	1,	2,	do_net_show,		9,	0,
		" show interfaces\n"
		" show interface [ <interfaceName> | <interface#> ]\n"
		" show protocols\n"
		" show protocol  [ <protocolType> ]\n"
		" show services  [ all ]\n"
		" show service   [ <serviceName> | <service#> ]\n"
		" show sets\n\n"
		" show set       [ <setName> | <set#> ]"					},

	{ "update",	0,	1,	do_net_update,		10,	0,
		" update                        : update the network configuration"		},

	{ "snapshot",	0,	0,	do_net_snapshot,	99,	2,
		" snapshot"									}

};
__private_extern__
const int nCommands_net = (sizeof(commands_net)/sizeof(cmdInfo));


__private_extern__
const cmdInfo commands_prefs[] = {
	/* cmd		minArgs	maxArgs	func			group	ctype			*/
	/* 	usage										*/

	{ "help",	0,	0,	do_help,		0,	0,
		" help                          : list available commands"			},

	{ "f.read",	1,	1,	do_readFile,		0,	0,
		" f.read file                   : process commands from file"			},

	{ "quit",	0,	1,	do_prefs_quit,		0,	0,
		" quit [!]                      : quit"						},

	{ "q",		0,	1,	do_prefs_quit,		0,	-1,
		NULL										},

	{ "exit",	0,	1,	do_prefs_quit,		0,	-1,
		NULL										},

	/* local dictionary manipulation commands */

	{ "d.init",	0,	0,	do_dictInit,		1,	0,
		" d.init                        : initialize (empty) dictionary"		},

	{ "d.show",	0,	0,	do_dictShow,		1,	0,
		" d.show                        : show dictionary contents"			},

	{ "d.add",	2,	101,	do_dictSetKey,		1,	0,
		" d.add key [*#?] val [v2 ...]  : add information to dictionary\n"
		"       (*=array, #=number, ?=boolean, %=hex data)"		                },

	{ "d.remove",	1,	1,	do_dictRemoveKey,	1,	0,
		" d.remove key                  : remove key from dictionary"			},

	/* data store manipulation commands */

	{ "open",	0,	1,	do_prefs_open,		2,	1,
		" open [\"prefsID\"]            : open a \"preferences\" session"		},

	{ "lock",	0,	1,	do_prefs_lock,		3,	1,
		" lock [wait]                   : locks write access to preferences"		},

	{ "commit",	0,	0,	do_prefs_commit,	2,	0,
		" commit                        : commit any changes"				},

	{ "apply",	0,	0,	do_prefs_apply,		2,	0,
		" apply                         : apply any changes"				},

	{ "unlock",	0,	0,	do_prefs_unlock,	3,	1,
		" unlock                        : unlocks write access to preferences"		},

	{ "close",	0,	1,	do_prefs_close,		2,	1,
		" close [!]                     : close current \"preference\" session"		},

	{ "synchronize",0,	1,	do_prefs_synchronize,	2,	0,
		" synchronize            : synchronize a \"preferences\" session"		},

	{ "list",	0,	1,	do_prefs_list,		4,	0,
		" list [path]                   : list preference paths"			},

	{ "get",	1,	1,	do_prefs_get,		4,	0,
		" get path                      : get dict from preferences w/path"		},

	{ "set",	1,	2,	do_prefs_set,		4,	0,
		" set path                      : set path in preferences w/current dict\n"
		" set path link                 : set path in preferences w/link"		},

	{ "remove",	1,	1,	do_prefs_remove,	4,	0,
		" remove path                   : remove path from preferences"			}
};
__private_extern__
const int nCommands_prefs = (sizeof(commands_prefs)/sizeof(cmdInfo));


__private_extern__ cmdInfo	*commands		= NULL;
__private_extern__ int		nCommands		= 0;
__private_extern__ Boolean	enablePrivateAPI	= FALSE;
__private_extern__ Boolean	termRequested		= FALSE;


__private_extern__
void
do_command(int argc, char **argv)
{
	int	i;
	char	*cmd = argv[0];

	for (i = 0; i < nCommands; i++) {
		if ((commands[i].ctype > 1) && !enablePrivateAPI)  {
			continue;	/* if "private" API and access has not been enabled */
		}

		if (strcasecmp(cmd, commands[i].cmd) == 0) {
			--argc;
			argv++;
			if (argc < commands[i].minArgs) {
				SCPrint(TRUE, stdout, CFSTR("%s: too few arguments\n"), cmd);
				return;
			} else if (argc > commands[i].maxArgs) {
				SCPrint(TRUE, stdout, CFSTR("%s: too many arguments\n"), cmd);
				return;
			}
			(*commands[i].func)(argc, argv);
			return;
		}
	}

	SCPrint(TRUE, stdout, CFSTR("%s: unknown, type \"help\" for command info\n"), cmd);
	return;
}


__private_extern__
void
do_help(int argc, char **argv)
{
	int	g = -1;		/* current group */
	int	i;

	SCPrint(TRUE, stdout, CFSTR("\nAvailable commands:\n"));
	for (i = 0; i < nCommands; i++) {
		if (commands[i].ctype < 0)  {
			continue;	/* if "hidden" */
		}

		if ((commands[i].ctype > 0) && !enablePrivateAPI)  {
			continue;	/* if "private" API and access has not been enabled */
		}

		/* check if this is a new command group */
		if (g != commands[i].group) {
			SCPrint(TRUE, stdout, CFSTR("\n"));
			g = commands[i].group;
		}

		/* display the command */
		SCPrint(TRUE, stdout, CFSTR("%s\n"), commands[i].usage);
	}
	SCPrint(TRUE, stdout, CFSTR("\n"));

	return;
}


__private_extern__
void
do_readFile(int argc, char **argv)
{
	InputRef		src;

	/* allocate command input stream */
	src = (InputRef)CFAllocatorAllocate(NULL, sizeof(Input), 0);
	src->el = NULL;
	src->h  = NULL;
	src->fp = fopen(argv[0], "r");

	if (src->fp == NULL) {
		SCPrint(TRUE, stdout, CFSTR("f.read: could not open file (%s).\n"), strerror(errno));
		CFAllocatorDeallocate(NULL, src);
		return;
	}

	/* open file, increase nesting level */
	SCPrint(TRUE, stdout, CFSTR("f.read: reading file (%s).\n"), argv[0]);
	nesting++;

	while (TRUE) {
		Boolean	ok;

		ok = process_line(src);
		if (!ok) {
			break;
		}
	}

	(void)fclose(src->fp);
	CFAllocatorDeallocate(NULL, src);

	return;
}


__private_extern__
void
do_quit(int argc, char **argv)
{
	termRequested = TRUE;
	return;
}
