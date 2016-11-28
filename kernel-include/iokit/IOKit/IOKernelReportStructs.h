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

#define kIOReportAPIVersion 28

// Drivers participating in IOReporting can advertise channels by
// publishing properties in the I/O Kit registry.  Various helper
// mechanisms exist to produce correctly-formatted legends.
// 12836893 tracks declaring channels in user space.
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
    
// We are currently (internally) limited to 15 (broad!) categories.


/*
   Units / Scaling Factors

   1. Implementation Details
   2. Unit Constants (kIOReportUnit...) for clients

   Please file radars if you need more units (IOReporting | X)
*/

// 1. Implementation Details
// We are likely to someday support IOReporting data as stored binary data.
// Don't change existing values lest that data become unreadable.

typedef uint64_t IOReportUnits;
#define __IOR_MAKEUNIT(quantity, scale) \
        (((IOReportUnits)quantity << 56) | (uint64_t)scale)
#define IOREPORT_GETUNIT_QUANTITY(unit) \
        ((IOReportQuantity)((uint64_t)unit >> 56) & 0xff)
#define IOREPORT_GETUNIT_SCALE(unit) \
        ((IOReportScaleFactor)unit & 0x00ffffffffffffff)

// 8b quantity + 32b const + 8b * 2^10 + 8b * 2^n + 8b cardinal + 8b unused
typedef uint8_t IOReportQuantity;       // SI "quantity" is what's measured
typedef uint64_t IOReportScaleFactor;

// See <http://en.wikipedia.org/wiki/SI_base_unit> for a list
// of quantities and their symbols.
enum {
    // used by state reports, etc
    kIOReportQuantityUndefined = 0,

    kIOReportQuantityTime = 1,          // Seconds
    kIOReportQuantityPower = 2,         // Watts
    kIOReportQuantityEnergy = 3,        // Joules
    kIOReportQuantityCurrent = 4,       // Amperes
    kIOReportQuantityVoltage = 5,       // Volts
    kIOReportQuantityCapacitance = 6,   // Farad
    kIOReportQuantityInductance = 7,    // Henry
    kIOReportQuantityFrequency = 8,     // Hertz
    kIOReportQuantityData = 9,          // bits/bytes (see scale)
    kIOReportQuantityTemperature = 10,  // Celsius (not Kelvin :)

    kIOReportQuantityEventCount = 100,
    kIOReportQuantityPacketCount = 101
};


/* A number of units end up with both IEC (2^n) and SI (10^n) scale factors.
   For example, the "MB" of a 1.44 MB floppy or a 1024MHz clock.  We
   thus support separate 2^n and 10^n factors.  The exponent encoding
   scheme is modeled loosely on single-precision IEEE 754.
 */
#define kIOReportScaleConstMask 0x000000007fffffff      // constant ("uint31")
#define kIOReportScaleOneOver   (1LL << 31)             // 1/constant
#define kIOReportExpBase        (-127)                  // support base^(-n)
#define kIOReportExpZeroOffset  -(kIOReportExpBase)     // max exponent = 128
#define kIOReportScaleSIShift   32                      // * 10^n
#define kIOReportScaleSIMask    0x000000ff00000000
#define kIOReportScaleIECShift  40                      // * 2^n
#define kIOReportScaleIECMask   0x0000ff0000000000
#define kIOReportCardinalShift  48                      // placeholders
#define kIOReportCardinalMask   0x00ff000000000000


/*
   Scales are described as a factor times unity:
   1ms = kIOReportScaleMilli * s

   A value expressed in a scaled unit can be scaled to unity via
   multiplication by the constant:
   100ms * kIOReportScaleMilli [1e-3] = 0.1s.
*/

// SI / decimal
#define kIOReportScalePico  ((-12LL + kIOReportExpZeroOffset)  \
                                        << kIOReportScaleSIShift)
#define kIOReportScaleNano  ((-9LL + kIOReportExpZeroOffset)  \
                                        << kIOReportScaleSIShift)
#define kIOReportScaleMicro ((-6LL + kIOReportExpZeroOffset)  \
                                        << kIOReportScaleSIShift)
#define kIOReportScaleMilli ((-3LL + kIOReportExpZeroOffset)  \
                                        << kIOReportScaleSIShift)
#define kIOReportScaleUnity 0    // 10^0 = 2^0 = 1
// unity = 0 is a special case for which we give up exp = -127
#define kIOReportScaleKilo  ((3LL + kIOReportExpZeroOffset)  \
                                        << kIOReportScaleSIShift)
#define kIOReportScaleMega  ((6LL + kIOReportExpZeroOffset)  \
                                        << kIOReportScaleSIShift)
#define kIOReportScaleGiga  ((9LL + kIOReportExpZeroOffset)  \
                                        << kIOReportScaleSIShift)
#define kIOReportScaleTera  ((12LL + kIOReportExpZeroOffset)  \
                                        << kIOReportScaleSIShift)

// IEC / computer / binary
// It's not clear we'll ever use 2^(-n), but 1..2^~120 should suffice.
#define kIOReportScaleBits  kIOReportScaleUnity
#define kIOReportScaleBytes     ((3LL + kIOReportExpZeroOffset)  \
                                            << kIOReportScaleIECShift)
// (bytes have to be added to the exponents up front, can't just OR in)
#define kIOReportScaleKibi      ((10LL + kIOReportExpZeroOffset)  \
                                            << kIOReportScaleIECShift)
#define kIOReportScaleKiBytes   ((13LL + kIOReportExpZeroOffset)  \
                                            << kIOReportScaleIECShift)
#define kIOReportScaleMebi      ((20LL + kIOReportExpZeroOffset)  \
                                            << kIOReportScaleIECShift)
#define kIOReportScaleMiBytes   ((23LL + kIOReportExpZeroOffset)  \
                                            << kIOReportScaleIECShift)
#define kIOReportScaleGibi      ((30LL + kIOReportExpZeroOffset)  \
                                            << kIOReportScaleIECShift)
#define kIOReportScaleGiBytes   ((33LL + kIOReportExpZeroOffset)  \
                                            << kIOReportScaleIECShift)
#define kIOReportScaleTebi      ((40LL + kIOReportExpZeroOffset)  \
                                            << kIOReportScaleIECShift)
#define kIOReportScaleTiBytes   ((43LL + kIOReportExpZeroOffset)  \
                                            << kIOReportScaleIECShift)
// can't encode more than 2^125 (keeping bits & bytes inside -126..128)
// Also, IOReportScaleValue() is currently limited internally by uint64_t.


// Cardinal values, to be filled in appropriately.
// Add values in increasing order.
#define kIOReportScaleMachHWTicks   (1LL << kIOReportCardinalShift)
#define kIOReportScaleHWPageSize    (2LL << kIOReportCardinalShift)

// page scales: 2 pages * 4ikB/page = 8096 bytes
#define kIOReportScale4KiB      (4 | kIOReportScaleKiBytes)
#define kIOReportScale8KiB      (8 | kIOReportScaleKiBytes)

// Clock frequencies scales (units add seconds).
// 1 GHz ticks are 1 ns: 1000 ticks * 1e-6 = 1e-3s
// The '1' is a no-op, but allows a custom label.
#define kIOReportScale1GHz      (1 | kIOReportScaleNano)
// 24MHz ticks are 1/24 of a microsecond: (1/24 * kIOReportScaleMicro [1e-6])s
// So for example, 240 24Mticks * 1/24 * 1e-6 = .00001s [1e-5]s
#define kIOReportScale24MHz     (kIOReportScaleOneOver|24 |kIOReportScaleMicro)

// --- END: implementation details

// 2. Units Constants 
// --- BEGIN: units constants driver writers might use
#define kIOReportUnitNone       __IOR_MAKEUNIT(kIOReportQuantityUndefined,  \
                                                  kIOReportScaleUnity)
    
#define kIOReportUnit_s         __IOR_MAKEUNIT(kIOReportQuantityTime,  \
                                               kIOReportScaleUnity)
#define kIOReportUnit_ms        __IOR_MAKEUNIT(kIOReportQuantityTime,  \
                                               kIOReportScaleMilli)
#define kIOReportUnit_us        __IOR_MAKEUNIT(kIOReportQuantityTime,  \
                                               kIOReportScaleMicro)
#define kIOReportUnit_ns        __IOR_MAKEUNIT(kIOReportQuantityTime,  \
                                               kIOReportScaleNano)
    
#define kIOReportUnit_J         __IOR_MAKEUNIT(kIOReportQuantityEnergy,  \
                                               kIOReportScaleUnity)
#define kIOReportUnit_mJ        __IOR_MAKEUNIT(kIOReportQuantityEnergy,  \
                                               kIOReportScaleMilli)
#define kIOReportUnit_uJ        __IOR_MAKEUNIT(kIOReportQuantityEnergy,  \
                                               kIOReportScaleMicro)
#define kIOReportUnit_nJ        __IOR_MAKEUNIT(kIOReportQuantityEnergy,  \
                                               kIOReportScaleNano)
#define kIOReportUnit_pJ        __IOR_MAKEUNIT(kIOReportQuantityEnergy,  \
                                               kIOReportScalePico)

#define kIOReportUnitHWTicks    __IOR_MAKEUNIT(kIOReportQuantityTime,  \
                                               kIOReportScaleMachHWTicks)
#define kIOReportUnit24MHzTicks __IOR_MAKEUNIT(kIOReportQuantityTime,  \
                                               kIOReportScale24MHz)
#define kIOReportUnit1GHzTicks  __IOR_MAKEUNIT(kIOReportQuantityTime,  \
                                               kIOReportScale1GHz)

#define kIOReportUnitBits       __IOR_MAKEUNIT(kIOReportQuantityData,  \
                                                kIOReportScaleBits)
#define kIOReportUnitBytes      __IOR_MAKEUNIT(kIOReportQuantityData,  \
                                                kIOReportScaleBytes)
#define kIOReportUnit_KiB       __IOR_MAKEUNIT(kIOReportQuantityData,  \
                                               kIOReportScaleKiBytes)

#define kIOReportUnitEvents     __IOR_MAKEUNIT(kIOReportQuantityEventCount,  \
                                               kIOReportScaleUnity)

#define kIOReportUnitPackets    __IOR_MAKEUNIT(kIOReportQuantityPacketCount,  \
                                               kIOReportScaleUnity)

// Please file radars if you need more units (IOReporting | X)
    
// --- END: unit constants driver writers might use
    
/*  Histogram Segment Configuration
    Currently supports 2 types of scaling to compute bucket upper bounds,
    linear or exponential.
    scale_flag = 0 -> linear scale
                 1 -> exponential scale
    upper_bound[n] = (scale_flag) ? pow(base,(n+1)) : base * (n+1);
*/
#define kIOHistogramScaleLinear 0
#define kIOHistogramScaleExponential 1
typedef struct {
    uint32_t    base_bucket_width;    // segment[0].bucket[0] = [0, base_width]
    uint32_t    scale_flag;           // bit 0 only in current use (see #defs)
    uint32_t    segment_idx;          // for multiple segments histograms
    uint32_t    segment_bucket_count; // number of buckets in this segment
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
