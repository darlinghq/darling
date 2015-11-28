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
	Boolean				ok;
	SCNetworkReachabilityRef	target1;
	SCNetworkReachabilityRef	target2;
	SCNetworkReachabilityRef	target3;
	SCNetworkReachabilityRef	target4;
	SCNetworkReachabilityRef	target5;

	_sc_log = FALSE;
	_sc_debug = TRUE;	// extra reachability logging

	target1 = SCNetworkReachabilityCreateWithName(NULL, "web.apple.com");
	ok = __SCNetworkReachabilityServer_targetAdd(target1);
	if (!ok) {
		SCLog(TRUE, LOG_ERR, CFSTR("No reachability server"));
		exit(1);
	}

	target2 = SCNetworkReachabilityCreateWithName(NULL, "www.comcast.net");
	__SCNetworkReachabilityServer_targetAdd(target2);

	target3 = SCNetworkReachabilityCreateWithName(NULL, "www.comcast.net");
	__SCNetworkReachabilityServer_targetAdd(target3);

	target4 = SCNetworkReachabilityCreateWithName(NULL, "www.nonexistenthost.com");
	__SCNetworkReachabilityServer_targetAdd(target4);

	target5 = SCNetworkReachabilityCreateWithName(NULL, "www.washingtonpost.com");
	__SCNetworkReachabilityServer_targetAdd(target5);

	__SCNetworkReachabilityServer_targetStatus(target1);
	__SCNetworkReachabilityServer_targetStatus(target2);
	__SCNetworkReachabilityServer_targetStatus(target3);
	__SCNetworkReachabilityServer_targetStatus(target4);

	__SCNetworkReachabilityServer_targetMonitor(target1, TRUE);
	__SCNetworkReachabilityServer_targetMonitor(target4, TRUE);
	__SCNetworkReachabilityServer_targetMonitor(target5, TRUE);

	__SCNetworkReachabilityServer_targetStatus(target4);
	__SCNetworkReachabilityServer_targetStatus(target3);
	__SCNetworkReachabilityServer_targetStatus(target2);
	__SCNetworkReachabilityServer_targetStatus(target1);

	sleep(2);

	__SCNetworkReachabilityServer_targetStatus(target1);
	__SCNetworkReachabilityServer_targetStatus(target4);
	__SCNetworkReachabilityServer_targetStatus(target1);
	__SCNetworkReachabilityServer_targetStatus(target4);

	__SCNetworkReachabilityServer_targetMonitor(target1, FALSE);

	__SCNetworkReachabilityServer_targetStatus(target1);
	__SCNetworkReachabilityServer_targetStatus(target4);
	__SCNetworkReachabilityServer_targetStatus(target1);
	__SCNetworkReachabilityServer_targetStatus(target4);

	__SCNetworkReachabilityServer_targetMonitor(target4, FALSE);

	__SCNetworkReachabilityServer_targetStatus(target1);
	__SCNetworkReachabilityServer_targetStatus(target4);

//	SCLog(TRUE, LOG_DEBUG, CFSTR("starting CFRunLoop"));
//	CFRunLoopRun();
//	SCLog(TRUE, LOG_DEBUG, CFSTR("CFRunLoop complete"));

	SCLog(TRUE, LOG_DEBUG, CFSTR("sleeping"));
	sleep(60);

	__SCNetworkReachabilityServer_targetStatus(target5);
	__SCNetworkReachabilityServer_targetRemove(target5);
	CFRelease(target5);

	__SCNetworkReachabilityServer_targetRemove(target4);
	CFRelease(target4);

	__SCNetworkReachabilityServer_targetRemove(target3);
	CFRelease(target3);

//	__SCNetworkReachabilityServer_targetRemove(target2);
//	CFRelease(target2);

//	__SCNetworkReachabilityServer_targetRemove(target1);
//	CFRelease(target1);

	exit(0);
}

