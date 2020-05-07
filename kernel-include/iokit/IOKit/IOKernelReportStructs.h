/*
 * Copyright (c) 2012-2014 Apple Computer, Inc.  All Rights Reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

// Internal data structures to be used by IOReporters and User Space Observers


#ifndef _IOKERNELREPORTSTRUCTS_H_
#define _IOKERNELREPORTSTRUCTS_H_

#include <stdint.h>

#include <IOKit/IOReportTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// Drivers participating in IOReporting can advertise channels by
// publishing properties in the I/O Kit registry.  Various helper
// mechanisms exist to produce correctly-formatted legends.
// 12836893 tracks advertising channels in user space.
#define kIOReportLegendPublicKey        "IOReportLegendPublic"      // bool
#define kIOReportLegendKey              "IOReportLegend"            // arr
#define kIOReportLegendChannelsKey      "IOReportChannels"          // arr
#define kIOReportLegendGroupNameKey     "IOReportGroupName"         // str
#define kIOReportLegendSubGroupNameKey  "IOReportSubGroupName"      // str
#define kIOReportLegendInfoKey          "IOReportChannelInfo"       // dict
#define kIOReportLegendUnitKey          "IOReportChannelUnit"       // num
#define kIOReportLegendConfigKey        "IOReportChannelConfig"     // data
#define kIOReportLegendStateNamesKey    "IOReportChannelStateNames" // str[]

// in an I/O Kit registry legend, a small "array struct" represents a channel
#define kIOReportChannelIDIdx           0       // required
#define kIOReportChannelTypeIdx         1       // required
#define kIOReportChannelNameIdx         2       // optional

/*  Histogram Segment Configuration
 *   Currently supports 2 types of scaling to compute bucket upper bounds,
 *   linear or exponential.
 *   scale_flag = 0 -> linear scale
 *                1 -> exponential scale
 *   upper_bound[n] = (scale_flag) ? pow(base,(n+1)) : base * (n+1);
 */
#define kIOHistogramScaleLinear 0
#define kIOHistogramScaleExponential 1
typedef struct {
	uint32_t    base_bucket_width;// segment[0].bucket[0] = [0, base_width]
	uint32_t    scale_flag;       // bit 0 only in current use (see #defs)
	uint32_t    segment_idx;      // for multiple segments histograms
	uint32_t    segment_bucket_count;// number of buckets in this segment
} __attribute((packed)) IOHistogramSegmentConfig;

// "normalized distribution"(FIXME?) internal format (unused?)
typedef struct {
	uint64_t    samples;
	uint64_t    mean;
	uint64_t    variance;
	uint64_t    reserved;
} __attribute((packed)) IONormDistReportValues;

#ifdef __cplusplus
}
#endif

#endif // _IOKERNELREPORTSTRUCTS_H_
