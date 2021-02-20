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

/*
 * IPMonitorAWDReport.m
 * - C shim layer to interact with AWD to generate and submit a metric
 */


#import <WirelessDiagnostics/WirelessDiagnostics.h>
#import "AWDMetricIds_IPMonitor.h"
#import "AWDIPMonitorInterfaceAdvisoryReport.h"
#import "IPMonitorAWDReport.h"
#import <SystemConfiguration/SCPrivate.h>

#include "symbol_scope.h"

#if defined(TEST_IPMONITOR_AWD_REPORT) || defined(TEST_IPMONITOR_CONTROL)

#define	my_log(__level, __format, ...)	SCPrint(TRUE, stdout, CFSTR(__format "\n"), ## __VA_ARGS__)

#else /* TEST_IPMONITOR_AWD_REPORT || TEST_IPMONITOR_CONTROL */

#define	my_log(__level, __format, ...)	SC_log(__level, __format, ## __VA_ARGS__)

#endif	/* TEST_IPMONITOR_AWD_REPORT || TEST_IPMONITOR_CONTROL */


STATIC AWDServerConnection *
IPMonitorAWDServerConnection(void)
{
    AWDServerConnection * server;

    if ([AWDServerConnection class] == nil) {
	return (nil);
    }
    server = [[AWDServerConnection alloc]
		 initWithComponentId:AWDComponentId_IPMonitor];
    if (server == NULL) {
	my_log(LOG_NOTICE, "Failed to create AWD server connection");
    }
    return (server);
}

STATIC InterfaceAdvisoryReportRef
_InterfaceAdvisoryReportCreate(AWDIPMonitorInterfaceType type)
{
    AWDIPMonitorInterfaceAdvisoryReport *	metric;

    if ([AWDServerConnection class] == nil) {
	return (NULL);
    }
    metric = [[AWDIPMonitorInterfaceAdvisoryReport alloc] init];
    metric.interfaceType = type;

    /* default to zero values */
    metric.flags = 0;
    metric.advisoryCount = 0;

    return ((InterfaceAdvisoryReportRef)metric);
}

PRIVATE_EXTERN InterfaceAdvisoryReportRef
InterfaceAdvisoryReportCreate(AWDIPMonitorInterfaceType type)
{
    InterfaceAdvisoryReportRef	report;

    @autoreleasepool {
	report = _InterfaceAdvisoryReportCreate(type);
    }
    return (report);
}

STATIC void
_InterfaceAdvisoryReportSubmit(InterfaceAdvisoryReportRef report)
{
    AWDMetricContainer * 	container;
    AWDServerConnection * 	server;

    server = IPMonitorAWDServerConnection();
    if (server == NULL) {
	return;
    }
    container = [server newMetricContainerWithIdentifier:
			    AWDMetricId_IPMonitor_InterfaceAdvisoryReport];
    [container setMetric:(AWDIPMonitorInterfaceAdvisoryReport *)report];
    [server submitMetric:container];
    [server release];
    [container release];
    return;
}

PRIVATE_EXTERN void
InterfaceAdvisoryReportSubmit(InterfaceAdvisoryReportRef report)
{
    @autoreleasepool {
	_InterfaceAdvisoryReportSubmit(report);
    }
}

#define INTERFACE_ADVISORY_REPORT_SET_PROP(report, name, value)	\
    @autoreleasepool {						\
	AWDIPMonitorInterfaceAdvisoryReport *	metric;		\
								\
	metric = (AWDIPMonitorInterfaceAdvisoryReport *)report;	\
	metric.name = value;					\
    }

void
InterfaceAdvisoryReportSetFlags(InterfaceAdvisoryReportRef report,
				AWDIPMonitorInterfaceAdvisoryReport_Flags flags)
{
    INTERFACE_ADVISORY_REPORT_SET_PROP(report, flags, flags);
}

void
InterfaceAdvisoryReportSetAdvisoryCount(InterfaceAdvisoryReportRef report,
					uint32_t count)
{
    INTERFACE_ADVISORY_REPORT_SET_PROP(report, advisoryCount, count);
}

#ifdef TEST_IPMONITOR_AWD_REPORT

int
main(int argc, char * argv[])
{
    InterfaceAdvisoryReportRef	report;
    AWDIPMonitorInterfaceType	type;

    type = AWDIPMonitorInterfaceType_IPMONITOR_INTERFACE_TYPE_WIFI;
    report = InterfaceAdvisoryReportCreate(type);
    if (report == NULL) {
	fprintf(stderr, "WirelessDiagnostics framework not available\n");
	exit(1);
    }
    printf("Before setting values:\n");
    CFShow(report);
    fflush(stdout);


    /* set values */
    InterfaceAdvisoryReportSetFlags(report,
				    AWDIPMonitorInterfaceAdvisoryReport_Flags_LINK_LAYER_ISSUE
				    | AWDIPMonitorInterfaceAdvisoryReport_Flags_UPLINK_ISSUE);
    InterfaceAdvisoryReportSetAdvisoryCount(report, 2);

    printf("After setting values:\n");
    CFShow(report);
    fflush(stdout);

    InterfaceAdvisoryReportSubmit(report);
    CFRelease(report);
    if (argc > 1) {
	fprintf(stderr, "pid is %d\n", getpid());
	sleep(120);
    }
}

#endif /* TEST_IPMONITOR_AWD_REPORT */
