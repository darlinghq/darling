/*
 *  fipspost_trace.h
 *  corecrypto
 *
 *  Created on 01/25/2017
 *
 *  Copyright (c) 2017 Apple Inc. All rights reserved.
 *
 */

#ifndef _CORECRYPTO_FIPSPOST_TRACE_H_
#define _CORECRYPTO_FIPSPOST_TRACE_H_

#if CC_FIPSPOST_TRACE

/*
 * Use this string to separate out tests.
 */
#define FIPSPOST_TRACE_TEST_STR    "?"

int fipspost_trace_is_active(void);
void fipspost_trace_call(const char *fname);

/* Only trace when VERBOSE is set to avoid impacting normal boots. */
#define FIPSPOST_TRACE_EVENT do {                                       \
    if (fipspost_trace_is_active()) {                                   \
        fipspost_trace_call(__FUNCTION__);                              \
    }                                                                   \
} while (0);

#define FIPSPOST_TRACE_MESSAGE(MSG) do {                                \
    if (fipspost_trace_is_active()) {                                   \
        fipspost_trace_call(MSG);                                       \
    }                                                                   \
} while (0);

#else

/* Not building a CC_FIPSPOST_TRACE-enabled, no TRACE operations. */
#define FIPSPOST_TRACE_EVENT
#define FIPSPOST_TRACE_MESSAGE(X)

#endif

#endif
