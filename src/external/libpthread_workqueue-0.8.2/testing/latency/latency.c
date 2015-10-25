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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifndef _WIN32
# include <unistd.h>
# include <pthread.h>
# include <sys/time.h>
#endif

#include "latency.h"

pthread_workqueue_t workqueues[WORKQUEUE_COUNT]; 
struct wq_statistics workqueue_statistics[WORKQUEUE_COUNT]; 
struct wq_event_generator workqueue_generator[GENERATOR_WORKQUEUE_COUNT];

struct wq_statistics global_statistics;
unsigned int global_stats_used = 0;

pthread_mutex_t generator_mutex;
pthread_cond_t generator_condition;
static unsigned int events_processed;

#define PERCENTILE_COUNT 8 
double percentiles[PERCENTILE_COUNT] = {50.0, 80.0, 98.0, 99.0, 99.5, 99.8, 99.9, 99.99};
mytime_t real_start, real_end;

#ifdef __APPLE__

#include <assert.h>
#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

static mach_timebase_info_data_t    sTimebaseInfo;

// From http://developer.apple.com/library/mac/#qa/qa2004/qa1398.html
unsigned long gettime(void)
{
    return (mach_absolute_time() * sTimebaseInfo.numer / sTimebaseInfo.denom);
}

#else

static mytime_t gettime(void)
{
#ifdef __linux__
	struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
		        fprintf(stderr, "Failed to get high resolution clock! errno = %d\n", errno);   
    return ((ts.tv_sec * NANOSECONDS_PER_SECOND) + ts.tv_nsec);
#elif defined(_WIN32)
	LARGE_INTEGER now;
	LARGE_INTEGER freq;
	if (!QueryPerformanceCounter(&now) )
		fprintf(stderr, "Failed to get performance counter!\n");
	if (!QueryPerformanceFrequency(&freq) )
		fprintf(stderr, "Failed to get performance frequency!\n");

	return (mytime_t)(now.QuadPart * NANOSECONDS_PER_SECOND / freq.QuadPart);
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_HIGHRES, &ts) != 0)
		        fprintf(stderr, "Failed to get high resolution clock! errno = %d\n", errno);   
    return ((ts.tv_sec * NANOSECONDS_PER_SECOND) + ts.tv_nsec);
#endif
}

#endif

#ifdef _WIN32

static void my_sleep(unsigned long nanoseconds) {
	LARGE_INTEGER start, end;
	LARGE_INTEGER freq;
	
	QueryPerformanceCounter(&start);
	QueryPerformanceFrequency(&freq);

	// sleep with ms resolution ...
	Sleep(nanoseconds / 1000000);

	// ... and busy-wait afterwards, until the requested delay was reached
	QueryPerformanceCounter(&end);
	while( (end.QuadPart - start.QuadPart) * NANOSECONDS_PER_SECOND / freq.QuadPart < nanoseconds ){
		YieldProcessor();
		QueryPerformanceCounter(&end);
	}

}

#else

// real resolution on solaris is at best system clock tick, i.e. 100Hz unless having the
// high res system clock (1000Hz in that case)

static void my_sleep(unsigned long nanoseconds)
{
	struct timespec timeout0;
	struct timespec timeout1;
	struct timespec* tmp;
	struct timespec* t0 = &timeout0;
	struct timespec* t1 = &timeout1;
	
	t0->tv_sec = nanoseconds / NANOSECONDS_PER_SECOND;
	t0->tv_nsec = nanoseconds % NANOSECONDS_PER_SECOND;

	while ((nanosleep(t0, t1) == (-1)) && (errno == EINTR))
	{
		tmp = t0;
		t0 = t1;
		t1 = tmp;
	}
    
    return;
}

#endif

static void _process_data(void* context)
{
	struct wq_event *event = (struct wq_event *) context;
	mytime_t elapsed_time;
    
	elapsed_time = gettime() - event->start_time;
    
	workqueue_statistics[event->queue_index].avg = ((workqueue_statistics[event->queue_index].count * workqueue_statistics[event->queue_index].avg) + elapsed_time) / (workqueue_statistics[event->queue_index].count + 1);
	workqueue_statistics[event->queue_index].total += elapsed_time;
	workqueue_statistics[event->queue_index].count += 1;
    
    if (elapsed_time < workqueue_statistics[event->queue_index].min || 
        workqueue_statistics[event->queue_index].min == 0) 
        workqueue_statistics[event->queue_index].min = elapsed_time;
	
    if (elapsed_time > workqueue_statistics[event->queue_index].max)
        workqueue_statistics[event->queue_index].max = elapsed_time;
    
    if ((elapsed_time / 1000) < DISTRIBUTION_BUCKETS)
        workqueue_statistics[event->queue_index].distribution[(int)(elapsed_time / 1000)] += 1;
    else  
        workqueue_statistics[event->queue_index].distribution[DISTRIBUTION_BUCKETS-1] += 1;

    // allow generator thread to continue when all events have been processed
    if (atomic_dec_nv(&events_processed) == 0)
    {
        pthread_mutex_lock(&generator_mutex);
        pthread_cond_signal(&generator_condition);
        pthread_mutex_unlock(&generator_mutex);
    }
    return;
}

// Perform a small microburst for this tick
static void _event_tick(void* context)
{
	struct wq_event *current_event;
	long i, generator_workqueue = (long) context;
    
    for (i = 0; i < EVENTS_GENERATED_PER_TICK; i++)
    {
        current_event = &workqueue_generator[generator_workqueue].wq_events[i];
        current_event->start_time = gettime();
        current_event->queue_index = (current_event->start_time % WORKQUEUE_COUNT);
        
        (void) pthread_workqueue_additem_np(workqueues[current_event->queue_index], _process_data, current_event, NULL, NULL);
    }
    
    return;
}

static void _generate_simulated_events()
{
	unsigned long i = 0, tick;
	mytime_t overhead;
    mytime_t start, current, overhead_start = 0, overhead_end = 0;

    start = current = gettime();

	for (tick = 0; tick < TOTAL_TICKS_TO_RUN; tick++)
	{
        start = current = overhead_end;
        overhead = overhead_end - overhead_start;
        
        // wait until we have waited proper amount of time for current rate
        // we should remove overhead of previous lap to not lag behind in data rate
        // one call to gethrtime() alone is around 211ns on Nehalem 2.93
        // use busy waiting in case the frequency is higher than the supported resolution of nanosleep()

        if (overhead > EVENT_TIME_SLICE)
        {
            printf("Warning: Event processing overhead > event time slice, readjust test parameters.\n");
        }
        else
        if ((EVENT_GENERATION_FREQUENCY > SYSTEM_CLOCK_RESOLUTION) || FORCE_BUSY_LOOP)
        {
            while ((current - start) < (EVENT_TIME_SLICE - overhead))
                current = gettime();            
        }
        else
        {
            my_sleep(EVENT_TIME_SLICE - overhead);
        }
        
        overhead_start = gettime();
        
        events_processed = GENERATOR_WORKQUEUE_COUNT * EVENTS_GENERATED_PER_TICK; // number of items that will be processed
  
#if (LATENCY_RUN_GENERATOR_IN_MAIN_THREAD == 0)
        for (i = 0; i < GENERATOR_WORKQUEUE_COUNT; i++)
            (void) pthread_workqueue_additem_np(workqueue_generator[i].wq, _event_tick, (void *) i, NULL, NULL);
#else
        _event_tick((void *)i);
#endif

        // wait for all events to be processed
        pthread_mutex_lock(&generator_mutex);
        while (events_processed > 0)
            pthread_cond_wait(&generator_condition, &generator_mutex);
        pthread_mutex_unlock(&generator_mutex);

        overhead_end = gettime();
	}	

	return;
}

static void _gather_statistics(unsigned long queue_index)
{
    unsigned long i;
    
    if (workqueue_statistics[queue_index].count > 0)
    {
        global_stats_used ++;
        
        global_statistics.avg = ((global_statistics.count * global_statistics.avg) + (workqueue_statistics[queue_index].avg * workqueue_statistics[queue_index].count)) / (global_statistics.count + workqueue_statistics[queue_index].count);
        global_statistics.total += workqueue_statistics[queue_index].total;
        global_statistics.count += workqueue_statistics[queue_index].count;
        
        if (workqueue_statistics[queue_index].min < global_statistics.min || global_statistics.min == 0) 
            global_statistics.min = workqueue_statistics[queue_index].min;
        
        if (workqueue_statistics[queue_index].max > global_statistics.max)
            global_statistics.max = workqueue_statistics[queue_index].max;
        
        for (i = 0; i < DISTRIBUTION_BUCKETS; i++)
            global_statistics.distribution[i] += workqueue_statistics[queue_index].distribution[i];
    }
    
	return;
}

void _print_statistics()
{
	unsigned long i, j, total_events = 0, last_percentile = 0, accumulated_percentile = 0;
        
	printf("Collecting statistics...\n");
	
	for (i = 0; i < WORKQUEUE_COUNT; i++)
        _gather_statistics(i);
    
	printf("Test is done, run time was %.3f seconds, %.1fM events generated and processed.\n", (double)((double)(real_end - real_start) / (double) NANOSECONDS_PER_SECOND), total_events/1000000.0); 
	
    //FIXME - casting from mytime_t (u_long) to int will truncate the result
	printf("Global dispatch queue aggregate statistics for %d queues: %dM events, min = %d ns, avg = %.1f ns, max = %d ns\n",
           global_stats_used, global_statistics.count/1000000, (int) global_statistics.min, global_statistics.avg, (int) global_statistics.max);
    
    printf("\nDistribution:\n");
    for (i = 0; i < DISTRIBUTION_BUCKETS; i++)
    {                   
        printf("%3ld us: %d ", i, global_statistics.distribution[i]);
        for (j=0; j<(((double) global_statistics.distribution[i] / (double) global_statistics.count) * 400.0); j++)
            printf("*");
        printf("\n");
    }
    
    printf("\nPercentiles:\n");
    
    for (i = 0; i < DISTRIBUTION_BUCKETS; i++)
    {        
        while ((last_percentile < PERCENTILE_COUNT) && ((100.0 * ((double) accumulated_percentile / (double) global_statistics.count)) > percentiles[last_percentile]))
        {
            printf("%.2f < %ld us\n", percentiles[last_percentile], i-1);
            last_percentile++;
        }
        accumulated_percentile += global_statistics.distribution[i];        
    }
    
    while ((last_percentile < PERCENTILE_COUNT) && ((100.0 * ((double) accumulated_percentile / (double) global_statistics.count)) > percentiles[last_percentile]))
    {
        printf("%.2f > %d us\n", percentiles[last_percentile], DISTRIBUTION_BUCKETS-1);
        last_percentile++;
    }

	return;
}	

int main(void)
{
	int i;
    pthread_workqueue_attr_t attr;
    
#ifdef __APPLE__
    (void) mach_timebase_info(&sTimebaseInfo);
#endif
	
#ifdef MAKE_STATIC
	pthread_workqueue_init_np();
#endif
    
	memset(&workqueues, 0, sizeof(workqueues));
	memset(&workqueue_statistics, 0, sizeof(workqueue_statistics));
	memset(&global_statistics, 0, sizeof(global_statistics));
	memset(&workqueue_generator, 0, sizeof(workqueue_generator));

    pthread_mutex_init(&generator_mutex, NULL);
    pthread_cond_init(&generator_condition, NULL);
    
    if (pthread_workqueue_attr_init_np(&attr) != 0)
        fprintf(stderr, "Failed to set workqueue attributes\n");
    
    for (i = 0; i < GENERATOR_WORKQUEUE_COUNT; i++)
    {
        if (pthread_workqueue_attr_setqueuepriority_np(&attr, i) != 0) 
            fprintf(stderr, "Failed to set workqueue priority\n");

        if (pthread_workqueue_attr_setovercommit_np(&attr, 1) != 0)
            fprintf(stderr, "Failed to set workqueue overcommit\n");

        workqueue_generator[i].wq_events = malloc(sizeof(struct wq_event) * EVENTS_GENERATED_PER_TICK);
        memset(workqueue_generator[i].wq_events, 0, (sizeof(struct wq_event) * EVENTS_GENERATED_PER_TICK));
        
        if (pthread_workqueue_create_np(&workqueue_generator[i].wq, &attr) != 0)
            fprintf(stderr, "Failed to create workqueue\n");
    }
    
	for (i = 0; i < WORKQUEUE_COUNT; i++)
	{
        if (pthread_workqueue_attr_init_np(&attr) != 0)
            fprintf(stderr, "Failed to set workqueue attributes\n");
        
        if (pthread_workqueue_attr_setqueuepriority_np(&attr, i) != 0) 
            fprintf(stderr, "Failed to set workqueue priority\n");
        
        if (pthread_workqueue_create_np(&workqueues[i], &attr) != 0)
            fprintf(stderr, "Failed to create workqueue\n");
	}
    
	if (SLEEP_BEFORE_START > 0)
	{
		printf("Sleeping for %d seconds to allow for processor set configuration...\n",SLEEP_BEFORE_START);
		sleep(SLEEP_BEFORE_START); 		
	}
	
    printf("%d workqueues, running for %d seconds at %d Hz, %d events per tick.\n",WORKQUEUE_COUNT, SECONDS_TO_RUN, EVENT_GENERATION_FREQUENCY, EVENTS_GENERATED_PER_TICK);
    
	printf("Running %d generator threads at %dK events/s, the aggregated data rate is %dK events/s. %.2f MB is used for %.2fK events.\n",
           GENERATOR_WORKQUEUE_COUNT,AGGREGATE_DATA_RATE_PER_SECOND/1000, TOTAL_DATA_PER_SECOND/1000,
           (double) GENERATOR_WORKQUEUE_COUNT * ((sizeof(struct wq_event) * EVENTS_GENERATED_PER_TICK + sizeof(workqueues))/(1024.0*1024.0)), 
           GENERATOR_WORKQUEUE_COUNT * EVENTS_GENERATED_PER_TICK/1000.0);

    real_start = gettime();
    
    _generate_simulated_events();

    real_end = gettime();

    _print_statistics();
        
	return 0;
}
