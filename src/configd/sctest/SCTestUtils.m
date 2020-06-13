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

#import <Foundation/Foundation.h>
#import "SCTest.h"
#import "SCTestUtils.h"
#import <mach/mach_time.h>

NSArray<NSString *> *
getTestClasses()
{
	static NSMutableArray *subclassNames = nil;
	Class base;
	unsigned int classListCount;
	Class *classList;

	if (subclassNames != nil) {
		return subclassNames;
	}

	base = [SCTest class];
	classListCount = 0;
	classList = objc_copyClassList(&classListCount);
	subclassNames = [[NSMutableArray alloc] init];

	if (classList) {
		for (unsigned int i = 0; i < classListCount; i++) {
			Class superClass = class_getSuperclass(classList[i]);
			while (superClass && superClass != base) {
				superClass = class_getSuperclass(superClass);
			}

			if (superClass == base) {
				[subclassNames addObject:@(class_getName(classList[i]))];
			}
		}
		free(classList);
	}

	[subclassNames sortUsingComparator: ^(id obj1, id obj2) {
		NSString *className1 = obj1;
		NSString *className2 = obj2;
		return [className1 compare:className2 options:NSCaseInsensitiveSearch];
	}];

	return subclassNames;
}

NSArray<NSString *> *
getUnitTestListForClass(Class base)
{
	NSMutableArray<NSString *> *unitTestNames = nil;
	unsigned int methodListCount = 0;
	Method *methodList = NULL;

	methodList = class_copyMethodList(base, &methodListCount);
	if (methodList) {
		unitTestNames = [[NSMutableArray alloc] initWithCapacity:methodListCount];
		for (unsigned int i = 0; i < methodListCount; i++) {
			NSString *name = @(sel_getName(method_getName(methodList[i])));
			if ([name isEqualToString:@"unitTest"]) {
				continue;
			} else if (![name hasPrefix:@"unitTest"]) {
				continue;
			}

			[unitTestNames addObject:name];
		}
		free(methodList);
	}

	return unitTestNames;
}

NSDictionary *
getOptionsDictionary(int argc, const char **argv)
{
	NSMutableDictionary *options;
	NSNumberFormatter *numberFormatter;
	int ch;
	int i;
	struct option entries[] = {
		kSCTestOptionEntries
	};

	options = [NSMutableDictionary dictionary];
	optind = 0;
	optreset = 1;
	numberFormatter = [[NSNumberFormatter alloc] init];

	while ((ch = getopt_long_only(argc, (char * const *)argv, "", entries, &i)) == 0) {
		struct option opt = entries[i];
		NSString *optKey = [NSString stringWithFormat:@"%s_Str", opt.name]; // ... "_Str" suffix is standardized across all keys.
		id optVal = nil;

		if (opt.has_arg) {
			// Parse the optarg

			// Attempt string
			optVal = @(optarg);

			if (optVal == nil) {
				// Fall back to NSData
				// WARNING: Doesn't work if it contains '\0'
				optVal = [NSData dataWithBytes:optarg length:strlen(optarg)];
			} else {
				// Use NSNumber if the argument is a number
				NSNumber *number = [numberFormatter numberFromString:optVal];
				if (number) {
					optVal = number;
				}
			}
		} else {
			optVal = @YES;
		}

		// Handle multiple option instances
		id existingValue = options[optKey];
		if (existingValue) {
			if ([existingValue isKindOfClass:[NSMutableArray class]]) {
				[(NSMutableArray *)existingValue addObject:optVal];
				optVal = existingValue;
			} else if ([existingValue isKindOfClass:[NSArray class]]) {
				NSMutableArray *tempArray = [NSMutableArray arrayWithArray:existingValue];
				[tempArray addObject:optVal];
				optVal = tempArray;
			} else {
				optVal = @[existingValue, optVal];
			}
		}

		options[optKey] = optVal;
	}

	if (ch > 0) {
		return nil;
	}

	return options;
}

static void
cpu_routine(CPUUsageInfoInner *usage)
{
	host_name_port_t host;
	host_cpu_load_info_data_t host_load;
	mach_msg_type_number_t count;
	kern_return_t kret;

	if (usage == NULL) {
		return;
	}

	host = mach_host_self();
	count = HOST_CPU_LOAD_INFO_COUNT;
	kret = host_statistics(host, HOST_CPU_LOAD_INFO, (host_info_t)&host_load, &count);
	if (kret) {
		return;
	}

	// ms per tick. 1 tick is 10 ms.
	usage->user = ((uint64_t)host_load.cpu_ticks[CPU_STATE_USER]) * 10;
	usage->sys = ((uint64_t)host_load.cpu_ticks[CPU_STATE_SYSTEM]) * 10;
	usage->idle = ((uint64_t)host_load.cpu_ticks[CPU_STATE_IDLE]) * 10;
}

void
cpuStart(CPUUsageInfo *cpu)
{
	cpu_routine(&cpu->startCPU);
}

void
cpuEnd(CPUUsageInfo *cpu)
{
	cpu_routine(&cpu->endCPU);
}

NSString *
createUsageStringForCPU(CPUUsageInfo *cpu)
{
	uint64_t userelapsed = cpu->endCPU.user - cpu->startCPU.user;
	uint64_t systemelapsed = cpu->endCPU.sys - cpu->startCPU.sys;
	uint64_t idleelapsed = cpu->endCPU.idle - cpu->startCPU.idle;
	uint64_t totalelapsed = userelapsed + systemelapsed + idleelapsed;
	double u = ((double)userelapsed * 100)/totalelapsed;
	double s = ((double)systemelapsed * 100)/totalelapsed;
	double i = ((double)idleelapsed * 100)/totalelapsed;

	return [NSString stringWithFormat:@"%1.02f%% user %1.02f%% sys %1.02f%% idle", u, s, i];
}

static void
timer_routine(struct timespec *ts)
{
	uint64_t diff;
	static uint32_t orwl_timebase_numer = 0;
	static uint32_t orwl_timebase_denom = 0;
	static uint64_t orwl_timestart = 0;
	if (orwl_timestart == 0) {
		mach_timebase_info_data_t tb = { 0, 0 };
		mach_timebase_info(&tb);
		orwl_timebase_numer = tb.numer;
		orwl_timebase_denom = tb.denom;
		orwl_timestart = mach_absolute_time();
	}
	if (0 == orwl_timebase_denom) {
		orwl_timebase_denom = 1;
	}
	diff = ((mach_absolute_time() - orwl_timestart) * orwl_timebase_numer) / orwl_timebase_denom;
	ts->tv_sec = (size_t)(diff / NSEC_PER_SEC);
	ts->tv_nsec = (size_t)(diff - (ts->tv_sec * NSEC_PER_SEC));
}

void
timerStart(timerInfo *timer)
{
	timer_routine(&timer->startTime);
}

void
timerEnd(timerInfo *timer)
{
	timer_routine(&timer->endTime);
}

NSString *
createUsageStringForTimer(timerInfo *timer)
{
	double elapsed;
	int64_t nsecs =  timer->endTime.tv_nsec - timer->startTime.tv_nsec;
	uint64_t secs =  timer->endTime.tv_sec - timer->startTime.tv_sec;
	if (nsecs < 0) {
		nsecs += NSEC_PER_SEC;
		secs -= 1;
	}

	elapsed = (double)secs + (double)nsecs / NSEC_PER_SEC;
	return [NSString stringWithFormat:@"%f", elapsed];
}
