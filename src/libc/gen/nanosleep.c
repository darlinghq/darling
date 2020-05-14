/*
 * Copyright (c) 1999, 2003, 2006, 2007, 2010 Apple Inc. All rights reserved.
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

#include <errno.h>
#include <sys/time.h>
#include <mach/mach_error.h>
#include <mach/mach_time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <TargetConditionals.h>

#if __DARWIN_UNIX03
#include <mach/clock.h>
#include <pthread.h>
#include <mach/mach.h>
#include <mach/mach_error.h>

#if !defined(BUILDING_VARIANT)
semaphore_t	clock_sem = MACH_PORT_NULL;
mach_port_t	clock_port = MACH_PORT_NULL;

void _init_clock_port(void);

void _init_clock_port(void) {
	kern_return_t kr;
	mach_port_t host = mach_host_self();
	
	/* Get the clock service port for nanosleep */
	kr = host_get_clock_service(host, SYSTEM_CLOCK, &clock_port);
	if (kr != KERN_SUCCESS) {
		abort();
	}
	
	kr = semaphore_create(mach_task_self(), &clock_sem, SYNC_POLICY_FIFO, 0);
	if (kr != KERN_SUCCESS) {
		abort();
	}
	mach_port_deallocate(mach_task_self(), host);
}
#else
extern semaphore_t clock_sem;
extern mach_port_t clock_port;
#endif /* !BUILDING_VARIANT */

extern int __unix_conforming;
#ifdef VARIANT_CANCELABLE
extern int __semwait_signal(int cond_sem, int mutex_sem, int timeout, int relative, __int64_t tv_sec, __int32_t tv_nsec);
#define	SEMWAIT_SIGNAL	__semwait_signal
#else /* !VARIANT_CANCELABLE */
extern int __semwait_signal_nocancel(int cond_sem, int mutex_sem, int timeout, int relative, __int64_t tv_sec, __int32_t tv_nsec);
#define	SEMWAIT_SIGNAL	__semwait_signal_nocancel
#endif /* VARIANT_CANCELABLE */

int
nanosleep(const struct timespec *requested_time, struct timespec *remaining_time) {
    kern_return_t kret;
    int ret;
    mach_timespec_t current;
    mach_timespec_t completion;
   
	if (__unix_conforming == 0)
		__unix_conforming = 1;

#ifdef VARIANT_CANCELABLE
    pthread_testcancel();
#endif /* VARIANT_CANCELABLE */

    if ((requested_time == NULL) || (requested_time->tv_sec < 0) || (requested_time->tv_nsec >= NSEC_PER_SEC)) {
        errno = EINVAL;
        return -1;
    }


    if (remaining_time != NULL) {
	/* once we add requested_time, this will be the completion time */
        kret = clock_get_time(clock_port, &completion);
        if (kret != KERN_SUCCESS) {
            fprintf(stderr, "clock_get_time() failed: %s\n", mach_error_string(kret));
            errno = EINVAL;
            return -1;
        }
    }
    ret = SEMWAIT_SIGNAL(clock_sem, MACH_PORT_NULL, 1, 1, (int64_t)requested_time->tv_sec, (int32_t)requested_time->tv_nsec);    
    if (ret < 0) {
        if (errno == ETIMEDOUT) {
		return 0;
        } else if (errno == EINTR) {
            if (remaining_time != NULL) {
                ret = clock_get_time(clock_port, &current);
                if (ret != KERN_SUCCESS) {
                    fprintf(stderr, "clock_get_time() failed: %s\n", mach_error_string(ret));
                    return -1;
                }
                /* This depends on the layout of a mach_timespec_t and timespec_t being equivalent */
                ADD_MACH_TIMESPEC(&completion, requested_time);
		/* We have to compare first, since mach_timespect_t contains unsigned integers */
		if(CMP_MACH_TIMESPEC(&completion, &current) > 0) {
		    SUB_MACH_TIMESPEC(&completion, &current);
		    remaining_time->tv_sec = completion.tv_sec;
		    remaining_time->tv_nsec = completion.tv_nsec;
		} else {
		    bzero(remaining_time, sizeof(*remaining_time));
		}
            }
        } else {
            errno = EINVAL;
	}
    }
    return -1;
}


#else /* !__DARWIN_UNIX03 */

typedef struct {
    uint64_t high;
    uint64_t low;
} uint128_t;

/* 128-bit addition: acc += add */
static inline void
add128_128(uint128_t *acc, uint128_t *add)
{
    acc->high += add->high;
    acc->low += add->low;
    if(acc->low < add->low)
	acc->high++; // carry
}

/* 128-bit subtraction: acc -= sub */
static inline void
sub128_128(uint128_t *acc, uint128_t *sub)
{
    acc->high -= sub->high;
    if(acc->low < sub->low)
	acc->high--; // borrow
    acc->low -= sub->low;
}

#define TWO64	(((double)(1ULL << 32)) * ((double)(1ULL << 32)))

static inline double
uint128_double(uint128_t *u)
{
    return TWO64 * u->high + u->low; // may loses precision
}

/* 64x64 -> 128 bit multiplication */
static inline void
mul64x64(uint64_t x, uint64_t y, uint128_t *prod)
{
    uint128_t add;
    /*
     * Split the two 64-bit multiplicands into 32-bit parts:
     * x => 2^32 * x1 + x2
     * y => 2^32 * y1 + y2
     */
    uint32_t x1 = (uint32_t)(x >> 32);
    uint32_t x2 = (uint32_t)x;
    uint32_t y1 = (uint32_t)(y >> 32);
    uint32_t y2 = (uint32_t)y;
    /*
     * direct multiplication:
     * x * y => 2^64 * (x1 * y1) + 2^32 (x1 * y2 + x2 * y1) + (x2 * y2)
     * The first and last terms are direct assignmenet into the uint128_t
     * structure.  Then we add the middle two terms separately, to avoid
     * 64-bit overflow.  (We could use the Karatsuba algorithm to save
     * one multiply, but it is harder to deal with 64-bit overflows.)
     */
    prod->high = (uint64_t)x1 * (uint64_t)y1;
    prod->low = (uint64_t)x2 * (uint64_t)y2;
    add.low = (uint64_t)x1 * (uint64_t)y2;
    add.high = (add.low >> 32);
    add.low <<= 32;
    add128_128(prod, &add);
    add.low = (uint64_t)x2 * (uint64_t)y1;
    add.high = (add.low >> 32);
    add.low <<= 32;
    add128_128(prod, &add);
}

/* calculate (x * y / divisor), using 128-bit internal calculations */
static int
muldiv128(uint64_t x, uint64_t y, uint64_t divisor, uint64_t *res)
{
    uint128_t temp;
    uint128_t divisor128 = {0, divisor};
    uint64_t result = 0;
    double recip;

    /* calculate (x * y) */
    mul64x64(x, y, &temp);
    /*
     * Now divide by the divisor.  We use floating point to calculate an
     * approximate answer and update the results.  Then we iterate and
     * calculate a correction from the difference.
     */
    recip = 1.0 / ((double)divisor);
    while(temp.high || temp.low >= divisor) {
	uint128_t backmul;
	uint64_t uapprox;
	double approx = uint128_double(&temp) * recip;

	if(approx > __LONG_LONG_MAX__)
	    return 0; // answer overflows 64-bits
	uapprox = (uint64_t)approx;
	mul64x64(uapprox, divisor, &backmul);
	/*
	 * Because we are using unsigned integers, we need to approach the
	 * answer from the lesser side.  So if our estimate is too large
	 * we need to decrease it until it is smaller.
	 */
	while(backmul.high > temp.high || (backmul.high == temp.high && backmul.low > temp.low)) {
	    sub128_128(&backmul, &divisor128);
	    uapprox--;
	}
	sub128_128(&temp, &backmul);
	result += uapprox;
    }
    *res = result;
    return 1;
}

int
nanosleep(const struct timespec *requested_time, struct timespec *remaining_time) {
    kern_return_t ret;
    uint64_t end, units;
    static struct mach_timebase_info info = {0, 0};
    static int unity;
    
    if ((requested_time == NULL) || (requested_time->tv_sec < 0) || (requested_time->tv_nsec > NSEC_PER_SEC)) {
        errno = EINVAL;
        return -1;
    }

    if (info.denom == 0) {
        ret = mach_timebase_info(&info);
        if (ret != KERN_SUCCESS) {
            fprintf(stderr, "mach_timebase_info() failed: %s\n", mach_error_string(ret));
            errno = EAGAIN;
            return -1;
        }
	/* If numer == denom == 1 (as in intel), no conversion needed */
	unity = (info.numer == info.denom);
    }

    if(unity)
	units = (uint64_t)requested_time->tv_sec * NSEC_PER_SEC;
    else if(!muldiv128((uint64_t)info.denom * NSEC_PER_SEC,
		       (uint64_t)requested_time->tv_sec,
		       (uint64_t)info.numer,
		       &units))
    {
	errno = EINVAL;
	return -1;
    }
    end = mach_absolute_time()
	+ units
	+ (uint64_t)info.denom * requested_time->tv_nsec / info.numer;
    ret = mach_wait_until(end);
    if (ret != KERN_SUCCESS) {
        if (ret == KERN_ABORTED) {
            errno = EINTR;
            if (remaining_time != NULL) {
                uint64_t now = mach_absolute_time();
                if (now >= end) {
		    remaining_time->tv_sec = 0;
		    remaining_time->tv_nsec = 0;
		} else {
		    if(unity)
			units = (end - now);
		    else
			muldiv128((uint64_t)info.numer,
				  (end - now),
				  (uint64_t)info.denom,
				  &units); // this can't overflow
		    remaining_time->tv_sec = units / NSEC_PER_SEC;
		    remaining_time->tv_nsec = units % NSEC_PER_SEC;
		}
            }
        } else {
            errno = EINVAL;
        }
        return -1;
    }
    return 0;
}


#endif /* __DARWIN_UNIX03 */
