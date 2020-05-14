/*
 * Copyright (c) 2016 Apple Inc. All rights reserved.
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
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach_error.h>
#include <mach/mach_time.h>
#include <os/base_private.h>

extern uint64_t __thread_selfusage(void);

#define timeval2nsec(tv) (tv.tv_sec * NSEC_PER_SEC + tv.tv_usec * NSEC_PER_USEC)

static uint64_t
_boottime_fallback_usec(void)
{
    struct timeval tv;
    size_t len = sizeof(tv);
    int ret = sysctlbyname("kern.boottime", &tv, &len, NULL, 0);
    if (ret == -1) return 0;
    return (uint64_t)tv.tv_sec * USEC_PER_SEC + (uint64_t)tv.tv_usec;
}

static int
_mach_boottime_usec(uint64_t *boottime, struct timeval *realtime)
{
    uint64_t bt1 = 0, bt2 = 0;
    int ret;
    do {
        bt1 = mach_boottime_usec();
        if (os_slowpath(bt1 == 0)) bt1 = _boottime_fallback_usec();

        atomic_thread_fence(memory_order_seq_cst);

        ret = gettimeofday(realtime, NULL);
        if (ret != 0) return ret;

        atomic_thread_fence(memory_order_seq_cst);

        bt2 = mach_boottime_usec();
        if (os_slowpath(bt2 == 0)) bt2 = _boottime_fallback_usec();
    } while (os_slowpath(bt1 != bt2));
    *boottime = bt1;
    return 0;
}

uint64_t
clock_gettime_nsec_np(clockid_t clock_id)
{
    switch(clock_id){
    case CLOCK_REALTIME: {
        struct timeval tv;
        int ret = gettimeofday(&tv, NULL);
        if (ret) return 0;
        return timeval2nsec(tv);
    }
    case CLOCK_MONOTONIC: {
        struct timeval tv;
        uint64_t boottime;
        int ret = _mach_boottime_usec(&boottime, &tv);
        if (ret) return 0;
        boottime *= NSEC_PER_USEC;
        return timeval2nsec(tv) - boottime;
    }
    case CLOCK_PROCESS_CPUTIME_ID: {
        struct rusage ru;
        int ret = getrusage(RUSAGE_SELF, &ru);
        if (ret) return 0;
        return timeval2nsec(ru.ru_utime) + timeval2nsec(ru.ru_stime);
    }
    default:
        // calls that use mach_absolute_time units fall through into a common path
        break;
    }

    // Mach Absolute Time unit-based calls
    mach_timebase_info_data_t tb_info;
    if (mach_timebase_info(&tb_info)) return 0;
    uint64_t mach_time;

    switch(clock_id){
    case CLOCK_MONOTONIC_RAW:
        mach_time = mach_continuous_time();
        break;
    case CLOCK_MONOTONIC_RAW_APPROX:
        mach_time = mach_continuous_approximate_time();
        break;
    case CLOCK_UPTIME_RAW:
        mach_time = mach_absolute_time();
        break;
    case CLOCK_UPTIME_RAW_APPROX:
        mach_time = mach_approximate_time();
        break;
    case CLOCK_THREAD_CPUTIME_ID:
        mach_time = __thread_selfusage();
        break;
    default:
        errno = EINVAL;
        return 0;
    }

    return (mach_time * tb_info.numer) / tb_info.denom;
}

int
clock_gettime(clockid_t clk_id, struct timespec *tp)
{
    switch(clk_id){
    case CLOCK_REALTIME: {
        struct timeval tv;
        int ret = gettimeofday(&tv, NULL);
        TIMEVAL_TO_TIMESPEC(&tv, tp);
        return ret;
    }
    case CLOCK_MONOTONIC: {
        struct timeval tv;
        uint64_t boottime_usec;
        int ret = _mach_boottime_usec(&boottime_usec, &tv);
        struct timeval boottime = {
            .tv_sec = boottime_usec / USEC_PER_SEC,
            .tv_usec = boottime_usec % USEC_PER_SEC
        };
        timersub(&tv, &boottime, &tv);
        TIMEVAL_TO_TIMESPEC(&tv, tp);
        return ret;
    }
    case CLOCK_PROCESS_CPUTIME_ID: {
        struct rusage ru;
        int ret = getrusage(RUSAGE_SELF, &ru);
        timeradd(&ru.ru_utime, &ru.ru_stime, &ru.ru_utime);
        TIMEVAL_TO_TIMESPEC(&ru.ru_utime, tp);
        return ret;
    }
    case CLOCK_MONOTONIC_RAW:
    case CLOCK_MONOTONIC_RAW_APPROX:
    case CLOCK_UPTIME_RAW:
    case CLOCK_UPTIME_RAW_APPROX:
    case CLOCK_THREAD_CPUTIME_ID: {
        uint64_t ns = clock_gettime_nsec_np(clk_id);
        if (!ns) return -1;

        tp->tv_sec = ns/NSEC_PER_SEC;
        tp->tv_nsec = ns % NSEC_PER_SEC;
        return 0;
    }
    default:
        errno = EINVAL;
        return -1;
    }
}

int
clock_getres(clockid_t clk_id, struct timespec *res)
{
    switch(clk_id){
    case CLOCK_REALTIME:
    case CLOCK_MONOTONIC:
    case CLOCK_PROCESS_CPUTIME_ID:
        res->tv_nsec = NSEC_PER_USEC;
        res->tv_sec = 0;
        return 0;

    case CLOCK_MONOTONIC_RAW:
    case CLOCK_MONOTONIC_RAW_APPROX:
    case CLOCK_UPTIME_RAW:
    case CLOCK_UPTIME_RAW_APPROX:
    case CLOCK_THREAD_CPUTIME_ID: {
        mach_timebase_info_data_t tb_info;
        if (mach_timebase_info(&tb_info)){
            return -1;
        }
        res->tv_nsec = tb_info.numer / tb_info.denom + (tb_info.numer % tb_info.denom != 0);
        res->tv_sec = 0;
        return 0;
    }

    default:
        errno = EINVAL;
        return -1;
    }
}

int
clock_settime(clockid_t clk_id, const struct timespec *tp)
{
    switch(clk_id){
    case CLOCK_REALTIME: {
        struct timeval tv = {
            .tv_sec = (time_t)tp->tv_sec,
            .tv_usec = (suseconds_t)(tp->tv_nsec / NSEC_PER_USEC)
        };
        return settimeofday(&tv, NULL);
    }
    default:
        errno = EINVAL;
        return -1;
    }
}
