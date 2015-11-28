/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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
 * main.c
 * - test harness to test IPMonitorControl client and server
 */

/*
 * Modification History
 *
 * December 16, 2013	Dieter Siegmund (dieter@apple.com)
 * - initial revision
 */

#include <stdlib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SCPrivate.h>

#include "IPMonitorControl.h"
#include "IPMonitorControlServer.h"
#include "symbol_scope.h"

STATIC void 
AssertionsChanged(void * info)
{
    CFDictionaryRef 	assertions = NULL;
    CFArrayRef		changes;

    changes = IPMonitorControlServerCopyInterfaceRankInformation(&assertions);
    SCPrint(TRUE, stdout, CFSTR("Changed interfaces %@\n"), changes);
    if (assertions == NULL) {
	SCPrint(TRUE, stdout, CFSTR("No assertions\n"));
    }
    else {
	SCPrint(TRUE, stdout, CFSTR("Assertions = %@\n"), assertions);
	CFRelease(assertions);
    }
    if (changes != NULL) {
	CFRelease(changes);
    }
    return;
}

int
main(int argc, char * argv[])
{
    if (argc >= 2) {
	int				ch;
	IPMonitorControlRef		control;
	SCNetworkServicePrimaryRank 	rank;
	Boolean				rank_set = FALSE;

	rank = kSCNetworkServicePrimaryRankDefault;
	control = IPMonitorControlCreate();
	if (control == NULL) {
	    fprintf(stderr, "failed to allocate IPMonitorControl\n");
	    exit(1);
	}

	while ((ch = getopt(argc, argv, "i:r:")) != EOF) {
	    CFStringRef			ifname;
	    SCNetworkServicePrimaryRank	existing_rank;

	    switch ((char)ch) {
	    case 'i':
		ifname = CFStringCreateWithCString(NULL, optarg,
						   kCFStringEncodingUTF8);
		existing_rank = IPMonitorControlGetInterfacePrimaryRank(control,
									ifname);
		printf("%s rank was %u\n", optarg, existing_rank);
		if (IPMonitorControlSetInterfacePrimaryRank(control,
							    ifname,
							    rank)) {
		    printf("%s rank set to %u\n", optarg, rank);
		    rank_set = TRUE;
		}
		else {
		    fprintf(stderr, "failed to set rank\n");
		}
		CFRelease(ifname);
		break;
	    case 'r':
		rank = strtoul(optarg, NULL, 0);
		break;
	    default:
		fprintf(stderr, "unexpected option '%c'\n", (char)ch);
		exit(1);
		break;
	    }
	}
	argc -= optind;
	argv += optind;
	if (argc > 0) {
	    fprintf(stderr, "ignoring additional parameters\n");
	}
	if (rank_set == FALSE) {
	    exit(1);
	}
    }
    else {
	CFRunLoopSourceContext 	context;
	CFRunLoopSourceRef	rls;
	STATIC Boolean		verbose = TRUE;

	bzero(&context, sizeof(context));
	context.info = (void *)NULL;
	context.perform = AssertionsChanged;
	rls = CFRunLoopSourceCreate(NULL, 0, &context);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls,
			   kCFRunLoopDefaultMode);
	if (IPMonitorControlServerStart(CFRunLoopGetCurrent(), rls,
					&verbose) == FALSE) {
	    fprintf(stderr, "failed to create connection\n");
	    exit(1);
	}
    }
    CFRunLoopRun();
    exit(0);
    return (0);
}

