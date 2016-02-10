#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>

#include "SCNetworkReachabilityInternal.h"

int
main(int argc, char **argv)
{
	_sc_log   = FALSE;	// no syslog
	_sc_debug = TRUE;	// extra reachability logging

	_SCNetworkReachabilityServer_start();

	SCLog(TRUE, LOG_DEBUG, CFSTR("starting CFRunLoop"));
	CFRunLoopRun();
	SCLog(TRUE, LOG_DEBUG, CFSTR("CFRunLoop complete"));

	exit(0);
}
