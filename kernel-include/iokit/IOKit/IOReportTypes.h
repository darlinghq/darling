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

#ifndef _IOREPORT_TYPES_H_
#define _IOREPORT_TYPES_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IOR_VALUES_PER_ELEMENT  4

/*! @const      kIOReportInvalidValue
    @const      kIOReportInvalidIntValue
    @abstract   cardinal value used to indicate data errors

    @discussion
        kIOReportInvalidValue and kIOReportInvalidIntValue have the
        same bit pattern so that clients checking for one or the other
        don't have to worry about getting the signedness right.
*/
#define kIOReportInvalidIntValue INT64_MIN
#define kIOReportInvalidValue (uint64_t)kIOReportInvalidIntValue

/*! @typedef    IOReportCategories
    @abstract   encapsulate important, multi-purpose "tags" for channels

    @discussion
        IOReportCategories is the type for the .categories field of
        IOReportChanelType.  These categories are inteded to empower a
        limited number of clients to retrieve a broad range of channels
        without knowing much about them.  They can be OR'd together as
        needed.  Groups and subgroups are a more extensible mechanism
        for aggregating channels produced by different drivers.
*/
typedef uint16_t IOReportCategories;                                                                                                
#define kIOReportCategoryPower           (1 << 1)       // and energy
#define kIOReportCategoryTraffic         (1 << 2)       // I/O at any level
#define kIOReportCategoryPerformance     (1 << 3)       // e.g. cycles/byte
#define kIOReportCategoryPeripheral      (1 << 4)       // not built-in

#define kIOReportCategoryField           (1 << 8)       // consider logging

// future categories TBD
#define kIOReportCategoryInterrupt       (1 << 14)      // TBR: 15850269
#define kIOReportCategoryDebug           (1 << 15)
#define kIOReportInvalidCategory         UINT16_MAX               


// IOReportChannelType.report_format
typedef uint8_t IOReportFormat;
enum {
   kIOReportInvalidFormat = 0,
   kIOReportFormatSimple = 1,
   kIOReportFormatState = 2,
   kIOReportFormatHistogram = 3,
   kIOReportFormatSimpleArray = 4
};

// simple report values
typedef struct {
    int64_t    simple_value;
    uint64_t    reserved1;
    uint64_t    reserved2;
    uint64_t    reserved3;
} __attribute((packed)) IOSimpleReportValues;

// simple value array
typedef struct {
    int64_t    simple_values[IOR_VALUES_PER_ELEMENT];
} __attribute((packed)) IOSimpleArrayReportValues;

// state report values
typedef struct {
    uint64_t    state_id;           // 0..N-1 or 8-char code (see MAKEID())
    uint64_t    intransitions;      // number of transitions into this state
    uint64_t    upticks;            // ticks spent in state (local timebase)
    uint64_t    last_intransition;  // ticks at last in-transition
} __attribute((packed)) IOStateReportValues;

// histogram report values
typedef struct {
    uint64_t    bucket_hits;
    int64_t     bucket_min;
    int64_t     bucket_max;
    int64_t     bucket_sum;
} __attribute((packed)) IOHistogramReportValues;



// configuration actions generally change future behavior
typedef uint32_t IOReportConfigureAction;
enum {
    // basics (in common operational order)
    kIOReportEnable             = 0x01,
    kIOReportGetDimensions      = 0x02,
    kIOReportDisable            = 0x00,

    // Enable/disable modifiers
    kIOReportNotifyHubOnChange  = 0x10,     // triggered polling

    kIOReportTraceOnChange      = 0x20      // kdebug.h tracing
};

// update actions should not have observable side effects
typedef uint32_t IOReportUpdateAction;
enum {
    kIOReportCopyChannelData    = 1,
    kIOReportTraceChannelData   = 2
};

typedef struct {
    uint8_t     report_format;      // Histogram, StateResidency, etc.
    uint8_t     reserved;           // must be zero
    uint16_t    categories;         // power, traffic, etc (omnibus obs.)
    uint16_t    nelements;          // internal size of channel

    // only meaningful in the data pipeline
    int16_t     element_idx;        // 0..nelements-1
                                    // -1..-(nelements) = invalid (13127884)
} __attribute((packed)) IOReportChannelType;

/*!
    @define     IOREPORT_MAKECHID
    @abstract   convert up to 8 printable characters into a 64-bit channel ID
    @param  <char0..char7> - printable chars to be packed into a channel ID
    @result     a 64-bit channel ID with an implicit ASCII name
    @discussion A simple example:
                IOREPORT_MAKECHID('H', 'i', ' ', 'w', 'o', 'r', 'l', 'd');
                will evaluate to 0x686920776f726c64.  Any NUL bytes are
                ignored (by libIOReport) for naming purposes, but will
                appear in the channel ID.  Using a non-NUL non-printable
                character will disable the implicit name.  Putting NUL
                bytes first eliminates trailing zeros when the channel
                ID is printed as hex.  For example:
                IORERPORT_MAKECHID('\0','\0','n','x','f','e','r','s');
                To see the text, use xxd -r -p # not -rp; see 12976241
*/
#define __IOR_lshiftchr(c, chshift)     ((uint64_t)(c) << (8*(chshift)))
#define IOREPORT_MAKEID(A, B, C, D, E, F, G, H) \
    (__IOR_lshiftchr(A, 7) | __IOR_lshiftchr(B, 6) | __IOR_lshiftchr(C, 5) \
    | __IOR_lshiftchr(D, 4) | __IOR_lshiftchr(E, 3) | __IOR_lshiftchr(F, 2) \
    | __IOR_lshiftchr(G, 1) | __IOR_lshiftchr(H, 0))

typedef struct {
    uint64_t                channel_id;
    IOReportChannelType     channel_type;
} IOReportChannel;

typedef struct {
    uint32_t                nchannels;
    IOReportChannel         channels[];
} IOReportChannelList;

typedef struct {
    uint64_t                provider_id;
    IOReportChannel         channel;
} IOReportInterest;

typedef struct {
    uint32_t                ninterests;
    IOReportInterest        interests[];
} IOReportInterestList;

typedef struct {
    uint64_t                v[IOR_VALUES_PER_ELEMENT];
} __attribute((packed)) IOReportElementValues;

typedef struct {
    uint64_t                provider_id;
    uint64_t                channel_id;
    IOReportChannelType     channel_type;
    uint64_t                timestamp;   // mach_absolute_time()
    IOReportElementValues   values;
} __attribute((packed)) IOReportElement;

#ifdef __cplusplus
}
#endif

#endif // _IOREPORT_TYPES_H_
