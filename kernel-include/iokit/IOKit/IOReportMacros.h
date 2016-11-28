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

#ifndef _IOREPORT_MACROS_H_
#define _IOREPORT_MACROS_H_

#include "IOReportTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef IOREPORT_ABORT
#define IOREPORT_ABORT panic
#endif

/*
    Background

    These macros allow non-I/O Kit software to generate IOReporting
    reports.  Clients must prevent concurrent access to any given
    report buffer from multiple threads.

    While these macros allow non-I/O Kit software to participate
    in IOReporting, an IOService instance must lend its driver ID,
    respond to the appropriate IOService overrides, and shuttle
    data back and forth.  In some cases, it may be useful to have
    the I/O Kit driver initialize the report buffer with the
    appropriate macro.
*/


/* ----- Reporting Single Integers (SimpleReport) ----- */

/*
 * The buffer size required for a SimpleReport.
 */

#define SIMPLEREPORT_BUFSIZE   (sizeof(IOReportElement))


/*
 * Initialize a buffer to hold a SimpleReport.
 *
 *                  void* buffer - ptr to SIMPLEREPORT_BUFSIZE bytes
 *                size_t bufSize - sanity check of buffer's size
 *           uint64_t providerID - registry Entry ID of the reporting service
 *            uint64_t channelID - the report's channel ID
 * IOReportCategories categories - categories of this channel
 *
 * If the buffer is not of sufficient size, the macro calls IOREPORT_ABORT().
 * If that returns, the buffer is filled with 0xbadcafe.
 */

#define SIMPLEREPORT_INIT(buffer, bufSize, providerID, channelID, cats)  \
do {  \
    IOReportElement     *__elem = (IOReportElement *)(buffer);  \
    IOSimpleReportValues *__vals;  \
    if ((bufSize) >= SIMPLEREPORT_BUFSIZE) {  \
        __elem->channel_id = (channelID);  \
        __elem->provider_id = (providerID);  \
        __elem->channel_type.report_format = kIOReportFormatSimple;  \
        __elem->channel_type.reserved = 0;  \
        __elem->channel_type.categories = (cats);  \
        __elem->channel_type.nelements = 1;  \
        __elem->channel_type.element_idx = 0;  \
        __elem->timestamp = 0;  \
        __vals = (IOSimpleReportValues*)&__elem->values;  \
        __vals->simple_value = kIOReportInvalidIntValue;  \
    }  \
    else {  \
        IOREPORT_ABORT("bufSize is smaller than the required size\n");  \
        __POLLUTE_BUF((buffer), (bufSize));  \
    }  \
} while(0)


/*
 * Set a SimpleReport to a new value.
 *
 *    void* simp_buf - ptr to memory initialized by SIMPLEREPORT_INIT()
 * int64_t new_value - new value for the report
 */

#define SIMPLEREPORT_SETVALUE(simp_buf, new_value)  \
do {  \
    IOReportElement *__elem = (IOReportElement *)(simp_buf);  \
    IOSimpleReportValues *__vals;  \
    __vals = (IOSimpleReportValues*)&__elem->values;  \
    __vals->simple_value = (new_value);  \
} while(0)


/*
 * Increment the value of a SimpleReport.
 *
 *    void* simp_buf - ptr to memory initialized by SIMPLEREPORT_INIT()
 * int64_t increment - amount by which to increment the value
 */
#define SIMPLEREPORT_INCREMENTVALUE(simp_buf, new_value)  \
do {  \
    IOReportElement *__elem = (IOReportElement *)(simp_buf);  \
    IOSimpleReportValues *__vals;  \
    __vals = (IOSimpleReportValues*)&__elem->values;  \
    __vals->simple_value += (new_value);  \
} while(0)


/*
 * Prepare a SimpleReport for
 * IOService::updateReport(kIOReportCopyChannelData...)
 *
 * void* simp_buf  - Ptr to memory updated by SIMPLEREPORT_SETVALUE()
 * void* ptr2cpy   - On return, 'ptr2cpy' points to the memory that needs to be
 *                   copied for kIOReportCopyChannelData.
 * size_t size2cpy - On return, 'size2cpy' is set to the size of the report
 *                   data that needs to be copied for kIOReportCopyChannelData.
 */

#define SIMPLEREPORT_UPDATEPREP(simp_buf, ptr2cpy, size2cpy)  \
do {  \
    (ptr2cpy) = (simp_buf);  \
    (size2cpy) = sizeof(IOReportElement);  \
} while(0)


/*
 * Update the result field received as a parameter for
 * kIOReportGetDimensions & kIOReportCopyChannelData actions.
 *
 * IOReportConfigureAction action - configure/updateReport() 'action' param
 *                   void* result - configure/updateReport() 'result' param
 */

#define SIMPLEREPORT_UPDATERES(action, result)  \
do {  \
    if (((action) == kIOReportGetDimensions) || ((action) == kIOReportCopyChannelData)) {  \
        int *__nElements = (int *)(result);  \
        *__nElements += 1;  \
    }  \
} while (0)


/*
 * Get the 64-bit channel ID of a SimpleReport.
 *
 * void* simp_buf - ptr to memory initialized by SIMPLEREPORT_INIT()
 */

#define SIMPLEREPORT_GETCHID(simp_buf)  \
    (((IOReportElement *)(simp_buf))->channel_id)

/*
 * Get the IOReportChannelType of a SimpleReport.
 *
 * void* simp_buf - ptr to memory initialized by SIMPLEREPORT_INIT()
 */

#define SIMPLEREPORT_GETCHTYPE(simp_buf)  \
    (*(uint64_t*)&(((IOReportElement *)(simp_buf))->channel_type))


/*
 * Get the integer value of a SimpleReport.
 *
 * void* simp_buf - memory initialized by SIMPLEREPORT_INIT()
 */

#define SIMPLEREPORT_GETVALUE(simp_buf)  \
    (((IOSimpleReportValues*)&(((IOReportElement*)(simp_buf))->values))  \
            ->simple_value)


/* ----- State Machine Reporting (StateReport) ----- */

// Internal struct for StateReport
typedef struct {
   uint16_t        curr_state;
   uint64_t        update_ts;
   IOReportElement elem[]; // Array of elements
} IOStateReportInfo;

/*
 * Determine the size required for a StateReport buffer.
 *
 * int nstates - number of states to be reported
 */
#define STATEREPORT_BUFSIZE(nstates)  \
    (sizeof(IOStateReportInfo) + (nstates) * sizeof(IOReportElement))


/*
 * Initialize a StateReport buffer.
 *
 *                   int nstates - number of states to be reported
 *                  void* buffer - ptr to STATEREPORT_BUFSIZE(nstates) bytes
 *                size_t bufSize - sanity check of buffer's size
 *           uint64_t providerID - registry Entry ID of the reporting service
 *            uint64_t channelID - ID of this channel, see IOREPORT_MAKEID()
 * IOReportCategories categories - categories of this channel
 *
 * If the buffer is not of sufficient size, the macro invokes IOREPORT_ABORT.
 * If that returns, the buffer is filled with 0xbadcafe.
 */
#define STATEREPORT_INIT(nstates, buf, bufSize, providerID, channelID, cats) \
do {  \
    IOStateReportInfo *__info = (IOStateReportInfo *)(buf);  \
    IOStateReportValues *__rep;  \
    IOReportElement     *__elem;  \
    if ((bufSize) >= STATEREPORT_BUFSIZE(nstates)) {  \
        for (unsigned __no = 0; __no < (nstates); __no++) {  \
            __elem =  &(__info->elem[__no]);  \
            __rep = (IOStateReportValues *) &(__elem->values);  \
            __elem->channel_id = (channelID);  \
            __elem->provider_id = (providerID);  \
            __elem->channel_type.report_format = kIOReportFormatState;  \
            __elem->channel_type.reserved = 0;  \
            __elem->channel_type.categories = (cats);  \
            __elem->channel_type.nelements = (nstates);  \
            __elem->channel_type.element_idx = __no;  \
            __elem->timestamp = 0;  \
            __rep->state_id = __no;  \
            __rep->intransitions = 0;  \
            __rep->upticks = 0;  \
        }  \
        __info->curr_state = 0;  \
        __info->update_ts = 0;  \
    }  \
    else {  \
        IOREPORT_ABORT("bufSize is smaller than the required size\n");  \
        __POLLUTE_BUF((buf), (bufSize));  \
    }  \
} while(0)

/*
 * Initialize the state id field of a state with the specified value.  By
 * default, STATEREPORT_INIT() initializes the state IDs with the index of
 * that state.  This macro can be used to provide a more descriptive state id.
 *
 *   void* state_buf - ptr to memory initialized by STATEREPORT_INIT()
 * unsigned stateIdx - index of the state, out of bounds -> no-op
 *  uint64_t stateID - new state id, see IOREPORT_MAKEID()
 */
#define STATEREPORT_SETSTATEID(state_buf, stateIdx, stateID)  \
do {  \
    IOStateReportInfo *__info = (IOStateReportInfo *)(state_buf);  \
    IOStateReportValues *__rep;  \
    if ((stateIdx) < __info->elem[0].channel_type.nelements) {  \
        __rep = (IOStateReportValues*) &(__info->elem[(stateIdx)].values);  \
        __rep->state_id = (stateID);  \
    }  \
} while (0)


/*
 * Set the state of a StateReport.
 *
 *      void* state_buf - pointer to memory initialized by STATEREPORT_INIT()
 * unsigned newStateIdx - index of new state, out of bounds -> no-op
 *  uint64_t changeTime - time at which the transition occurred
 */
#define STATEREPORT_SETSTATE(state_buf, newStateIdx, changeTime)  \
do {  \
    IOStateReportInfo *__info = (IOStateReportInfo *)(state_buf);  \
    IOStateReportValues *__rep;  \
    if ((newStateIdx) < __info->elem[0].channel_type.nelements ) {  \
        __rep = (IOStateReportValues*) &(__info->elem[__info->curr_state].values);  \
        if (__info->update_ts)  \
            __rep->upticks += (changeTime) - __info->update_ts;  \
        __info->elem[(newStateIdx)].timestamp = (changeTime);  \
        __rep = (IOStateReportValues*) &(__info->elem[(newStateIdx)].values);  \
        __rep->intransitions++;  \
        __info->curr_state = (newStateIdx);  \
        __info->update_ts = (changeTime);  \
    }  \
} while(0)

/*
 * Prepare a StateReport for
 * IOService::updateReport(kIOReportCopyChannelData...)
 *
 *      void* state_buf - ptr to memory initialized by STATEREPORT_INIT()
 * uint64_t currentTime - current timestamp
 *        void* ptr2cpy - filled in with pointer to buffer to be copied out
 *      size_t size2cpy - filled in with the size of the buffer to copy out
 */
#define STATEREPORT_UPDATEPREP(state_buf, currentTime, ptr2cpy, size2cpy)  \
do {  \
    IOStateReportInfo *__info = (IOStateReportInfo *)(state_buf);  \
    IOReportElement     *__elem;  \
    IOStateReportValues *__state;  \
    (size2cpy) = __info->elem[0].channel_type.nelements * sizeof(IOReportElement);  \
    (ptr2cpy) =  (void *) &__info->elem[0];  \
    if (__info->update_ts)  {  \
        __elem = &__info->elem[__info->curr_state];  \
        __state = (IOStateReportValues *)&__elem->values;  \
        __elem->timestamp = (currentTime);  \
        __state->upticks  += (currentTime) - __info->update_ts;  \
        __info->update_ts = (currentTime);  \
    }  \
} while(0)

/*
 * Update the result field received as a parameter for kIOReportGetDimensions &
 * kIOReportCopyChannelData actions.
 *
 *                void* state_buf - memory initialized by STATEREPORT_INIT()
 * IOReportConfigureAction action - configure/updateReport() 'action'
 *                   void* result - configure/updateReport() 'result'
 */

#define STATEREPORT_UPDATERES(state_buf, action, result)  \
do {  \
    IOStateReportInfo *__info = (IOStateReportInfo *)(state_buf);  \
    IOReportElement     *__elem;  \
    int *__nElements = (int *)(result);  \
    if (((action) == kIOReportGetDimensions) || ((action) == kIOReportCopyChannelData)) {  \
        __elem =  &(__info->elem[0]);  \
        *__nElements += __elem->channel_type.nelements;  \
    }  \
} while (0)


/*
 * Get the 64-bit channel ID of a StateReport.
 *
 * void* state_buf - ptr to memory initialized by STATEREPORT_INIT()
 */
#define STATEREPORT_GETCHID(state_buf)  \
    (((IOStateReportInfo *)(state_buf))->elem[0].channel_id)

/*
 * Get the IOReportChannelType of a StateReport.
 *
 * void* state_buf - ptr to memory initialized by STATEREPORT_INIT()
 */
#define STATEREPORT_GETCHTYPE(state_buf)  \
    (*(uint64_t*)&(((IOStateReportInfo *)(state_buf))->elem[0].channel_type))

/*
 * Get the number of transitions into a given state.
 *
 *   void* state_buf - ptr to memory initialized by STATEREPORT_INIT()
 * unsigned stateIdx - index of state, out of bounds -> kIOReportInvalidValue
 *
 */
#define STATEREPORT_GETTRANSITIONS(state_buf, stateIdx)  \
    (((stateIdx) < ((IOStateReportInfo *)(state_buf))->elem[0].channel_type.nelements)  \
        ? ((IOStateReportValues*)&(((IOStateReportInfo*)(state_buf))->elem[(stateIdx)].values))->intransitions  \
        : kIOReportInvalidValue)

/*
 * Get the total number of ticks spent in a given state.
 *
 *   void* state_buf - ptr to memory initialized by STATEREPORT_INIT()
 * unsigned stateIdx - index of state, out of bounds -> kIOReportInvalidValue
 */
#define STATEREPORT_GETTICKS(state_buf, stateIdx)  \
    (((stateIdx) < ((IOStateReportInfo*)(state_buf))->elem[0].channel_type.nelements)  \
        ? ((IOStateReportValues*)&(((IOStateReportInfo*)(state_buf))->elem[(stateIdx)].values))->upticks  \
        : kIOReportInvalidValue)


/* ----- Reporting an Array of Integers (SimpleArrayReport) ----- */

/*
 * Determine the buffer size for a SimpleArrayReport.
 *
 * int nValues - number of values to be reported
 */

#define SIMPLEARRAY_BUFSIZE(nValues) \
    ((((nValues)/IOR_VALUES_PER_ELEMENT) + (((nValues) % IOR_VALUES_PER_ELEMENT) ? 1:0)) \
        * sizeof(IOReportElement))

/*
 * Initialize a buffer for use as a SimpleArrayReport.
 *
 *                   int nValues   - number of elements to be reported
 *                     void* buf   - ptr to SIMPLEARRAY_BUFSIZE(nValues) bytes
 *                size_t bufSize   - sanity check of buffer's size
 *           uint64_t providerID   - registry Entry ID of the reporting service
 *            uint64_t channelID   - ID of this channel, see IOREPORT_MAKEID()
 * IOReportCategories categories   - categories of this channel
 *
 * If the buffer is not of sufficient size, the macro invokes IOREPORT_ABORT()
 * and, if that returns, fills the buffer with 0xbadcafe.
 */

#define SIMPLEARRAY_INIT(nValues, buf, bufSize, providerID, channelID, cats) \
do {  \
    IOSimpleArrayReportValues *__rep;  \
    IOReportElement     *__elem;  \
    uint32_t            __nElems = (((nValues) / IOR_VALUES_PER_ELEMENT) + \
                                    (((nValues) % IOR_VALUES_PER_ELEMENT) ? 1 : 0)); \
    if ((bufSize) >= SIMPLEARRAY_BUFSIZE(nValues)) {  \
        for (unsigned __no = 0; __no < __nElems; __no++) {  \
            __elem =  &(((IOReportElement *)(buf))[__no]);  \
            __rep = (IOSimpleArrayReportValues *) &(__elem->values);  \
            __elem->channel_id = (channelID);  \
            __elem->provider_id = (providerID);  \
            __elem->channel_type.report_format = kIOReportFormatSimpleArray;  \
            __elem->channel_type.reserved = 0;  \
            __elem->channel_type.categories = (cats);  \
            __elem->channel_type.nelements = (__nElems);  \
            __elem->channel_type.element_idx = __no;  \
            __elem->timestamp = 0;  \
            __rep->simple_values[0] = kIOReportInvalidIntValue;  \
            __rep->simple_values[1] = kIOReportInvalidIntValue;  \
            __rep->simple_values[2] = kIOReportInvalidIntValue;  \
            __rep->simple_values[3] = kIOReportInvalidIntValue;  \
        }  \
    }  \
    else {  \
        IOREPORT_ABORT("bufSize is smaller than the required size\n");  \
        __POLLUTE_BUF((buf), (bufSize));  \
    }  \
} while(0)


/* SimpleArrayReport helpers */

    #define __SA_FINDREP(array_buf, idx)  \
        IOSimpleArrayReportValues *__rep;  \
        IOReportElement     *__elem;  \
        unsigned __elemIdx = (idx) / IOR_VALUES_PER_ELEMENT;  \
        unsigned __valueIdx = (idx) % IOR_VALUES_PER_ELEMENT;  \
        __elem = &(((IOReportElement *)(array_buf))[0]);  \
        if (__elemIdx < __elem->channel_type.nelements)  { \
            __elem = &(((IOReportElement *)(array_buf))[__elemIdx]);  \
            __rep = (IOSimpleArrayReportValues *) &(__elem->values);  \

    #define __SA_MAXINDEX(array_buf)  \
        ((((IOReportElement*)(array_buf))->channel_type.nelements)  \
            * IOR_VALUES_PER_ELEMENT) - 1

/*
 * Set a value at a specified index in a SimpleArrayReport.
 *
 *   void* array_bufbuf - ptr to memory initialized by SIMPLEARRAY_INIT()
 *        unsigned idx  - array index, out of bounds -> no-op
 *    uint64_t newValue - new value to be stored at array[idx]
 */
#define SIMPLEARRAY_SETVALUE(array_buf, idx, newValue) \
do {  \
    __SA_FINDREP((array_buf), (idx)) \
        __rep->simple_values[__valueIdx] = (newValue);  \
    } \
} while(0)

/*
 * Increment an array value within a SimpleArrayReport.
 *
 *     void* array_buf - ptr to memory initialized by SIMPLEARRAY_INIT()
 *       unsigned idx  - array index to increment, out of bounds -> no-op
 *      int64_t value  - amount by which to increment array[idx]
 */
#define SIMPLEARRAY_INCREMENTVALUE(array_buf, idx, value)  \
do {  \
    __SA_FINDREP((array_buf), (idx)) \
        __rep->simple_values[__valueIdx] += (value);  \
    } \
} while(0)


/*
 * Prepare a SimpleArrayReport for
 * IOService::updateReport(kIOReportCopyChannelData...)
 *
 *      void* array_buf - ptr to memory initialized by SIMPLEARRAY_INIT()
 *        void* ptr2cpy - filled in with pointer to buffer to be copied out
 *      size_t size2cpy - filled in with the size of the buffer to copy out
 */

#define SIMPLEARRAY_UPDATEPREP(array_buf, ptr2cpy, size2cpy) \
do {  \
    IOReportElement     *__elem;  \
    __elem = &(((IOReportElement *)(array_buf))[0]);  \
    (ptr2cpy) =  (void *) (array_buf);  \
    (size2cpy) = __elem->channel_type.nelements * sizeof(IOReportElement);  \
} while(0)


/*
 * Update the result field received as a parameter for kIOReportGetDimensions &
 * kIOReportCopyChannelData actions.
 *
 *                void* array_buf - memory initialized by SIMPLEARRAY_INIT()
 * IOReportConfigureAction action - configure/updateReport() 'action'
 *                   void* result - configure/updateReport() 'result'
 */

#define SIMPLEARRAY_UPDATERES(array_buf, action, result) \
do {  \
    IOReportElement     *__elem;  \
    int *__nElements = (int *)(result);  \
    __elem = &(((IOReportElement *)(array_buf))[0]);  \
    if (((action) == kIOReportGetDimensions) || ((action) == kIOReportCopyChannelData)) {  \
        *__nElements += __elem->channel_type.nelements;  \
    }  \
} while (0)


/*
 * Get the 64-bit channel ID of a SimpleArrayReport.
 *
 * void* array_buf - ptr to memory initialized by SIMPLEARRAY_INIT()
 */
#define SIMPLEARRAY_GETCHID(array_buf)  \
    (((IOReportElement *)(array_buf))->channel_id)


/*
 * Get the IOReportChannelType of a SimpleArrayReport.
 *
 * void* simp_buf - ptr to memory initialized by SIMPLEREPORT_INIT()
 */
#define SIMPLEARRAY_GETCHTYPE(array_buf)  \
    (*(uint64_t*)&(((IOReportElement *)(array_buf))->channel_type))

/*
 * Get a value from a SimpleArrayReport.
 *
 * void* array_buf - ptr to memory initialized by SIMPLEARRAY_INIT()
 *   unsigned idx  - index of the value, out of bounds -> kIOReportInvalidValue
 */
#define SIMPLEARRAY_GETVALUE(array_buf, idx)  \
    (((idx) > __SA_MAXINDEX(array_buf) || (idx) < 0) ? kIOReportInvalidIntValue :  \
    ((IOSimpleArrayReportValues*)&(  \
        ((IOReportElement*)(array_buf))[(idx) / IOR_VALUES_PER_ELEMENT].values))  \
            ->simple_values[(idx) % IOR_VALUES_PER_ELEMENT])


/* ----- Histogram Reporting (HistogramReport) ----- */

// Internal struct for HistogramReport
typedef struct {
    int             bucketWidth;
   IOReportElement elem[]; // Array of elements
} IOHistReportInfo;

/*
 * Determine the size required for a HistogramReport buffer.
 *
 * int nbuckets - number of buckets in the histogram
 */
#define HISTREPORT_BUFSIZE(nbuckets)  \
    (sizeof(IOHistReportInfo) + ((nbuckets) * sizeof(IOReportElement)))

/*
 * Initialize a HistogramReport buffer. Supports only linear scale histogram.
 *
 *                   int nbuckets - number of buckets data is combined into
 *           uint32_t bucketWidth - size of each bucket
 *                  void* buffer - ptr to HISTREPORT_BUFSIZE(nbuckets) bytes
 *                size_t bufSize - sanity check of buffer's size
 *           uint64_t providerID - registry Entry ID of the reporting service
 *            uint64_t channelID - ID of this channel, see IOREPORT_MAKEID()
 * IOReportCategories categories - categories of this channel
 *
 * If the buffer is not of sufficient size, the macro invokes IOREPORT_ABORT.
 * If that returns, the buffer is filled with 0xbadcafe.
 */
#define HISTREPORT_INIT(nbuckets, bktSize, buf, bufSize, providerID, channelID, cats) \
do {  \
    IOHistReportInfo   *__info = (IOHistReportInfo *)(buf);  \
    IOReportElement         *__elem;  \
    IOHistogramReportValues *__rep;  \
    if ((bufSize) >= HISTREPORT_BUFSIZE(nbuckets)) {  \
        __info->bucketWidth = (bktSize);  \
        for (unsigned __no = 0; __no < (nbuckets); __no++) {  \
            __elem =  &(__info->elem[__no]);  \
            __rep = (IOHistogramReportValues *) &(__elem->values);  \
            __elem->channel_id = (channelID);  \
            __elem->provider_id = (providerID);  \
            __elem->channel_type.report_format = kIOReportFormatHistogram;  \
            __elem->channel_type.reserved = 0;  \
            __elem->channel_type.categories = (cats);  \
            __elem->channel_type.nelements = (nbuckets);  \
            __elem->channel_type.element_idx = __no;  \
            __elem->timestamp = 0;  \
            bzero(__rep, sizeof(IOHistogramReportValues)); \
        }  \
    }  \
    else {  \
        IOREPORT_ABORT("bufSize is smaller than the required size\n");  \
        __POLLUTE_BUF((buf), (bufSize));  \
    }  \
} while (0)

/*
 * Update histogram with a new value.
 *
 *
 *      void* hist_buf - pointer to memory initialized by HISTREPORT_INIT()
 *        int64_t value - new value to add to the histogram
 */
#define HISTREPORT_TALLYVALUE(hist_buf, value) \
do {  \
    IOHistReportInfo   *__info = (IOHistReportInfo *)(hist_buf);  \
    IOReportElement         *__elem;  \
    IOHistogramReportValues *__rep;  \
    for (unsigned __no = 0; __no < __info->elem[0].channel_type.nelements; __no++) {  \
        if ((value) <= __info->bucketWidth * (__no+1)) {  \
            __elem =  &(__info->elem[__no]);  \
            __rep = (IOHistogramReportValues *) &(__elem->values);  \
            if (__rep->bucket_hits == 0) {  \
                __rep->bucket_min = __rep->bucket_max = (value);  \
            }  \
            else if ((value) < __rep->bucket_min) {  \
                __rep->bucket_min = (value);  \
            }  \
            else if ((value) > __rep->bucket_max) {  \
                __rep->bucket_max = (value);  \
            }  \
            __rep->bucket_sum += (value);  \
            __rep->bucket_hits++;  \
            break;  \
        }  \
    }  \
} while (0)

/*
 * Prepare a HistogramReport for
 * IOService::updateReport(kIOReportCopyChannelData...)
 *
 *      void* array_buf - ptr to memory initialized by HISTREPORT_INIT()
 *        void* ptr2cpy - filled in with pointer to buffer to be copied out
 *      size_t size2cpy - filled in with the size of the buffer to copy out
 */

#define HISTREPORT_UPDATEPREP(hist_buf, ptr2cpy, size2cpy) \
do {  \
    IOHistReportInfo   *__info = (IOHistReportInfo *)(hist_buf);  \
    (size2cpy) = __info->elem[0].channel_type.nelements * sizeof(IOReportElement);  \
    (ptr2cpy) =  (void *) &__info->elem[0];  \
} while(0)


/*
 * Update the result field received as a parameter for kIOReportGetDimensions &
 * kIOReportCopyChannelData actions.
 *
 *                void* array_buf - memory initialized by HISTREPORT_INIT()
 * IOReportConfigureAction action - configure/updateReport() 'action'
 *                   void* result - configure/updateReport() 'result'
 */

#define HISTREPORT_UPDATERES(hist_buf, action, result) \
do {  \
    IOHistReportInfo   *__info = (IOHistReportInfo *)(hist_buf);  \
    int *__nElements = (int *)(result);  \
    if (((action) == kIOReportGetDimensions) || ((action) == kIOReportCopyChannelData)) {  \
        *__nElements += __info->elem[0].channel_type.nelements;  \
    }  \
} while (0)

/*
 * Get the 64-bit channel ID of a HistogramReport.
 *
 * void* hist_buf - ptr to memory initialized by HISTREPORT_INIT()
 */
#define HISTREPORT_GETCHID(hist_buf)  \
    (((IOHistReportInfo *)(hist_buf))->elem[0].channel_id)

/*
 * Get the IOReportChannelType of a HistogramReport.
 *
 * void* hist_buf - ptr to memory initialized by HISTREPORT_INIT()
 */
#define HISTREPORT_GETCHTYPE(hist_buf)  \
    (*(uint64_t*)&(((IOHistReportInfo *)(hist_buf))->elem[0].channel_type))



/* generic utilities */

    #define __POLLUTE_BUF(buf, bufSize)  \
    do {  \
        int __cnt = (bufSize)/sizeof(uint32_t);  \
        while (--__cnt >= 0)  \
            ((uint32_t*)(buf))[__cnt] = 0xbadcafe;  \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif // _IOREPORT_MACROS_H_


