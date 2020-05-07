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
 *   @const      kIOReportInvalidIntValue
 *   @abstract   cardinal value used to indicate data errors
 *
 *   @discussion
 *       kIOReportInvalidValue and kIOReportInvalidIntValue have the
 *       same bit pattern so that clients checking for one or the other
 *       don't have to worry about getting the signedness right.
 */
#define kIOReportInvalidIntValue INT64_MIN
#define kIOReportInvalidValue (uint64_t)kIOReportInvalidIntValue

/*! @typedef    IOReportCategories
 *   @abstract   encapsulate important, multi-purpose "tags" for channels
 *
 *   @discussion
 *       IOReportCategories is the type for the .categories field of
 *       IOReportChanelType.  These categories are inteded to empower a
 *       limited number of clients to retrieve a broad range of channels
 *       without knowing much about them.  They can be OR'd together as
 *       needed.  Groups and subgroups are a more extensible mechanism
 *       for aggregating channels produced by different drivers.
 */
typedef uint16_t IOReportCategories;
#define kIOReportCategoryPower           (1 << 1)       // and energy
#define kIOReportCategoryTraffic         (1 << 2)       // I/O at any level
#define kIOReportCategoryPerformance     (1 << 3)       // e.g. cycles/byte
#define kIOReportCategoryPeripheral      (1 << 4)       // not built-in

#define kIOReportCategoryField           (1 << 8)       // consider logging

// future categories TBD
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
	uint64_t    state_id;       // 0..N-1 or 8-char code (see MAKEID())
	uint64_t    intransitions;  // number of transitions into this state
	uint64_t    upticks;        // ticks spent in state (local timebase)
	uint64_t    last_intransition;// ticks at last in-transition
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
	kIOReportNotifyHubOnChange  = 0x10, // triggered polling

	kIOReportTraceOnChange      = 0x20  // kdebug.h tracing
};

// update actions should not have observable side effects
typedef uint32_t IOReportUpdateAction;
enum {
	kIOReportCopyChannelData    = 1,
	kIOReportTraceChannelData   = 2
};

typedef struct {
	uint8_t     report_format;  // Histogram, StateResidency, etc.
	uint8_t     reserved;       // must be zero
	uint16_t    categories;     // power, traffic, etc (omnibus obs.)
	uint16_t    nelements;      // internal size of channel

	// only meaningful in the data pipeline
	int16_t     element_idx;    // 0..nelements-1
	                            // -1..-(nelements) = invalid (13127884)
} __attribute((packed)) IOReportChannelType;

/*!
 *   @define     IOREPORT_MAKECHID
 *   @abstract   convert up to 8 printable characters into a 64-bit channel ID
 *   @param  <char0..char7> - printable chars to be packed into a channel ID
 *   @result     a 64-bit channel ID with an implicit ASCII name
 *   @discussion A simple example:
 *               IOREPORT_MAKECHID('H', 'i', ' ', 'w', 'o', 'r', 'l', 'd');
 *               will evaluate to 0x686920776f726c64.  Any NUL bytes are
 *               ignored (by libIOReport) for naming purposes, but will
 *               appear in the channel ID.  Using a non-NUL non-printable
 *               character will disable the implicit name.  Putting NUL
 *               bytes first eliminates trailing zeros when the channel
 *               ID is printed as hex.  For example:
 *               IORERPORT_MAKECHID('\0','\0','n','x','f','e','r','s');
 *               To see the text, use xxd -r -p # not -rp; see 12976241
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
	uint64_t                timestamp;// mach_absolute_time()
	IOReportElementValues   values;
} __attribute((packed)) IOReportElement;



/*
 *  IOReporting unit type and constants
 */

// 1. Mechanism

// Assume encoded units could be stored in binary format: don't
// change existing values.

typedef uint64_t IOReportUnit;
typedef uint64_t IOReportUnits;     // deprecated typo, please switch
#define __IOR_MAKEUNIT(quantity, scale) \
	(((IOReportUnit)quantity << 56) | (uint64_t)scale)
#define IOREPORT_GETUNIT_QUANTITY(unit) \
	((IOReportQuantity)((uint64_t)unit >> 56) & 0xff)
#define IOREPORT_GETUNIT_SCALE(unit) \
	((IOReportScaleFactor)unit & 0x00ffffffffffffff)

// 8b quantity ID | 32b const val + 8b*2^10 + 8b*2^n | 8b cardinal | 8b unused
typedef uint8_t IOReportQuantity;       // SI "quantity" is what's measured
typedef uint64_t IOReportScaleFactor;

// See <http://en.wikipedia.org/wiki/SI_base_unit> for a list
// of quantities and their symbols.
enum {
	// used by state reports, etc
	kIOReportQuantityUndefined = 0,

	kIOReportQuantityTime           = 1,// Seconds
	kIOReportQuantityPower          = 2,// Watts
	kIOReportQuantityEnergy         = 3,// Joules
	kIOReportQuantityCurrent        = 4,// Amperes
	kIOReportQuantityVoltage        = 5,// Volts
	kIOReportQuantityCapacitance    = 6,// Farad
	kIOReportQuantityInductance     = 7,// Henry
	kIOReportQuantityFrequency      = 8,// Hertz
	kIOReportQuantityData           = 9,// bits/bytes (see scale)
	kIOReportQuantityTemperature    = 10,// Celsius (not Kelvin :)

	kIOReportQuantityEventCount     = 100,
	kIOReportQuantityPacketCount    = 101,
	kIOReportQuantityCPUInstrs      = 102
};


/* A number of units end up with both IEC (2^n) and SI (10^n) scale factors.
 *  For example, the "MB" of a 1.44 MB floppy or a 1024MHz clock.  We
 *  thus support separate 2^n and 10^n factors.  The exponent encoding
 *  scheme is modeled loosely on single-precision IEEE 754.
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
 *  Scales are described as a factor times unity:
 *  1ms = kIOReportScaleMilli * s
 *
 *  A value expressed in a scaled unit can be scaled to unity via
 *  multiplication by the constant:
 *  100ms * kIOReportScaleMilli [1e-3] = 0.1s.
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
#define kIOReportScale16KiB     (16 | kIOReportScaleKiBytes)

// Clock frequency scales (units add seconds).
// 1 GHz ticks are 1 ns: 1000 ticks * 1e-6 = 1e-3s
// This '1' is a no-op for scaling, but allows a custom label.
#define kIOReportScale1GHz      (1 | kIOReportScaleNano)
// 24MHz ticks are 1/24 of a microsecond: (1/24 * kIOReportScaleMicro [1e-6])s
// So for example, 240 24Mticks * 1/24 * 1e-6 = .00001s [1e-5]s
#define kIOReportScale24MHz     (kIOReportScaleOneOver|24 |kIOReportScaleMicro)

// --- END: units mechanism


// 2. Unit constants
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
#define kIOReportUnit_MiB       __IOR_MAKEUNIT(kIOReportQuantityData,  \
	                                       kIOReportScaleMiBytes)
#define kIOReportUnit_GiB       __IOR_MAKEUNIT(kIOReportQuantityData,  \
	                                       kIOReportScaleGiBytes)
#define kIOReportUnit_TiB       __IOR_MAKEUNIT(kIOReportQuantityData,  \
	                                       kIOReportScaleTiBytes)

#define kIOReportUnitEvents     __IOR_MAKEUNIT(kIOReportQuantityEventCount,  \
	                                       kIOReportScaleUnity)

#define kIOReportUnitPackets    __IOR_MAKEUNIT(kIOReportQuantityPacketCount,  \
	                                       kIOReportScaleUnity)

#define kIOReportUnitInstrs     __IOR_MAKEUNIT(kIOReportQuantityCPUInstrs,  \
	                                       kIOReportScaleUnity)
#define kIOReportUnit_KI        __IOR_MAKEUNIT(kIOReportQuantityCPUInstrs,  \
	                                       kIOReportScaleKilo)
#define kIOReportUnit_MI        __IOR_MAKEUNIT(kIOReportQuantityCPUInstrs,  \
	                                       kIOReportScaleMega)
#define kIOReportUnit_GI        __IOR_MAKEUNIT(kIOReportQuantityCPUInstrs,  \
	                                       kIOReportScaleGiga)

// Please file bugs (xnu | IOReporting) for additional units.

// --- END: unit constants


#ifdef __cplusplus
}
#endif

#endif // _IOREPORT_TYPES_H_
