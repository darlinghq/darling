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
 * IPMonitorAWDReport.h
 * - C shim layer to interact with AWD to generate and submit a metric
 */

#ifndef _S_IPMONITOR_AWD_REPORT_H
#define _S_IPMONITOR_AWD_REPORT_H

/*
 * Modification History
 *
 * June 21, 2018	Dieter Siegmund (dieter@apple.com)
 * - created
 */

#include <CoreFoundation/CFBase.h>
#ifndef NS_ENUM
#define NS_ENUM 	CF_ENUM
#endif

#include "AWDIPMonitorGlobalEnums.h"

/* ugh, duplicate */
#ifndef __OBJC__
typedef NS_ENUM(int32_t, AWDIPMonitorInterfaceAdvisoryReport_Flags) {
    AWDIPMonitorInterfaceAdvisoryReport_Flags_LINK_LAYER_ISSUE = 1,
    AWDIPMonitorInterfaceAdvisoryReport_Flags_UPLINK_ISSUE = 2,
};
#endif

typedef CFTypeRef InterfaceAdvisoryReportRef;

InterfaceAdvisoryReportRef
InterfaceAdvisoryReportCreate(AWDIPMonitorInterfaceType type);

void
InterfaceAdvisoryReportSubmit(InterfaceAdvisoryReportRef report);

void
InterfaceAdvisoryReportSetFlags(InterfaceAdvisoryReportRef report,
				AWDIPMonitorInterfaceAdvisoryReport_Flags flags);
void
InterfaceAdvisoryReportSetAdvisoryCount(InterfaceAdvisoryReportRef report,
					uint32_t count);
#endif /*  _S_IPMONITOR_AWD_REPORT_H */
