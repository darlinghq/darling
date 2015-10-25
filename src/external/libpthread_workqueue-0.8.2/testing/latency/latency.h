/*
 * Copyright (c) 2011 Joakim Johansson <jocke@tbricks.com>.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#include "pthread_workqueue.h"

#ifdef _WIN32
# include "../../src/windows/platform.h"
#endif

// Run settings
#define SECONDS_TO_RUN 10
#define WORKQUEUE_COUNT 3
#define GENERATOR_WORKQUEUE_COUNT 1
#define SLEEP_BEFORE_START 0
#define FORCE_BUSY_LOOP 0
#define LATENCY_RUN_GENERATOR_IN_MAIN_THREAD 0

// Data rates
#define EVENTS_GENERATED_PER_TICK 100   // simulate some small bursting 
#define EVENT_GENERATION_FREQUENCY 100  // events/s base rate, need to use busy loop = 1 if > 100Hz due to nanosleep resolution 

#define AGGREGATE_DATA_RATE_PER_SECOND (EVENT_GENERATION_FREQUENCY * EVENTS_GENERATED_PER_TICK)
#define EVENTS_TO_GENERATE (SECONDS_TO_RUN * AGGREGATE_DATA_RATE_PER_SECOND)
#define TOTAL_DATA_PER_SECOND (AGGREGATE_DATA_RATE_PER_SECOND*GENERATOR_WORKQUEUE_COUNT)
#define TOTAL_TICKS_TO_RUN (SECONDS_TO_RUN * EVENT_GENERATION_FREQUENCY)

#define NANOSECONDS_PER_SECOND 1000000000
#define DISTRIBUTION_BUCKETS 20 // 1us per bucket
#define EVENT_TIME_SLICE (NANOSECONDS_PER_SECOND / EVENT_GENERATION_FREQUENCY)
#define SYSTEM_CLOCK_RESOLUTION 100

#ifdef _WIN32
typedef unsigned long long mytime_t;
#else
typedef unsigned long mytime_t;
#endif

struct wq_event 
{
	unsigned int queue_index; 
	mytime_t start_time;
};

struct wq_statistics 
{
	mytime_t min; 
	mytime_t max; 
	double avg; 
	mytime_t total; 
	unsigned int count; 
	unsigned int count_over_threshold; 
    unsigned int distribution[DISTRIBUTION_BUCKETS];
};

// We create our own separate workqueues for event generation
struct wq_event_generator 
{
	pthread_workqueue_t wq; 
	struct wq_event *wq_events; 
};

#ifdef __sun
# include <atomic.h>
# define atomic_inc      atomic_inc_32
# define atomic_dec      atomic_dec_32
# define atomic_inc_nv   atomic_inc_32_nv
# define atomic_dec_nv   atomic_dec_32_nv
#elif defined(_WIN32)
# define atomic_inc(p)   (void) InterlockedIncrement((p))
# define atomic_dec(p)   (void) InterlockedDecrement((p))
# define atomic_inc_nv(p)  InterlockedIncrement((p))
# define atomic_dec_nv(p)  InterlockedDecrement((p))
#else
# define atomic_inc(p)   (void) __sync_add_and_fetch((p), 1)
# define atomic_dec(p)   (void) __sync_sub_and_fetch((p), 1)
# define atomic_inc_nv(p)  __sync_add_and_fetch((p), 1)
# define atomic_dec_nv(p)  __sync_sub_and_fetch((p), 1)
#endif
