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

#include <mach/mach_vm.h>
#include <malloc/malloc.h>
#include <ktrace/ktrace.h>
#include <os/assumes.h>
#include <sys/event.h>
#include <numeric>
#include <notify.h>
#include <dlfcn.h>
#include "malloc_replay.h"
#include <map>
#include <string>
#include <sysexits.h>
#include <perfdata/perfdata.h>
#include <perfcheck_keys.h>

#define capture_thread_counters(x, c) \
    if (c & (CONFIG_REC_COUNTERS | CONFIG_REC_STATS)) { \
        x = thread_instruction_count(); \
    }

#if TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR

// Maximum size to map when reading replay file chunks
#define MAX_REPLAY_FILE_CHUNK_SIZE (100 * 1024 * 1024)

#endif // TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR

static void (*s_funcMagSetThreadIndex)(unsigned int index);

extern "C" int thread_selfcounts(int type, void *buf, size_t nbytes);

//
//Store counter values for each (call, size) tuple.
//
typedef std::pair<int, uint32_t> CallSizePair;
typedef std::vector<uint16_t, ReplayAllocator<uint16_t>> ReplayVector;
static std::map<CallSizePair, ReplayVector, std::less<CallSizePair>, ReplayAllocator<std::pair<const CallSizePair, ReplayVector>>> s_counterDistributions;
static std::map<uint64_t, uint64_t, std::less<uint64_t>, ReplayAllocator<std::pair<const uint64_t, uint64_t> > > s_addressMap;

static uint64_t s_totalEvents = 0;
static uint64_t s_totalLibMallocEvents = 0;
static uint64_t s_totalMallocEvents = 0;
static uint64_t s_totalMalignEvents = 0;
static uint64_t s_totalCallocEvents = 0;
static uint64_t s_totalReallocEvents = 0;
static uint64_t s_totalFreeEvents = 0;
static uint64_t s_totalVallocEvents = 0;
static uint64_t s_totalFailedFreeEvents = 0;
static uint64_t s_totalFailedReallocEvents = 0;

uint64_t call_ins_retired[operation_count] = {0};
uint64_t call_count[operation_count] = {0};

static const char *_DefaultFragMetricName = "DefaultZoneFragmentation";
static const char *_DefaultNanoZone = "DefaultMallocZone";

enum {
	CONFIG_REC_COUNTERS  = 1 << 0,
	CONFIG_REC_STATS     = 1 << 1,
	CONFIG_RUN_REPLAY    = 1 << 2,
	CONFIG_CONVERT_FILE  = 1 << 3,
	CONFIG_PAUSE         = 1 << 4,
};
typedef uint8_t replay_config_t;

//
//Our allocator to allocate from a specific zone.
//
malloc_zone_t* s_zone = NULL;

// The magazine number to use for non-replayed allocations.
#define NON_REPLAY_MAGAZINE 0

static void
configure_ktrace_session(ktrace_session_t s)
{
	ktrace_set_execnames_enabled(s, KTRACE_FEATURE_DISABLED);
	ktrace_set_walltimes_enabled(s, KTRACE_FEATURE_DISABLED);
	ktrace_set_uuid_map_enabled(s, KTRACE_FEATURE_DISABLED);
	ktrace_set_thread_groups_enabled(s, KTRACE_FEATURE_DISABLED);
}

static uint64_t
thread_instruction_count(void)
{
    uint64_t instrCounts[2] = {};
    int err;
    err = thread_selfcounts(1, &instrCounts, sizeof(instrCounts));
    return instrCounts[0];
}

////////////////////////////////////////////////////////////////////////////////
//
// run_ktrace - Takes a nullable input ktrace file path and an output file path.
//              If the input file is NULL, this will setup a ktrace recording
//              session targeted at a file in the output file path. If an input
//              ktrace file path is provided, this will convert the ktrace file
//              to the compressed mtrace format, targeted at the output file
//              path.
//
////////////////////////////////////////////////////////////////////////////////

const int chunk_buffer_size = 16 * 1024 * 1024;

typedef union {
	struct compressed_alloc alloc;
	struct compressed_calloc calloc;
	struct compressed_realloc realloc;
	struct compressed_free free;
	struct compressed_memalign memalign;
} compressed_op_params;

static bool
run_ktrace(const char* inputFile, const char* outputFile)
{
    __block uint32_t blockBytesWritten = 0;

	ktrace_file_t output_file = ktrace_file_create(NULL, outputFile);
	if (!output_file) {
		printf("Couldn't create output file: %s\n", outputFile);
		return false;
	}

	ktrace_session_t s = ktrace_session_create();
	if (inputFile) {
		if (ktrace_set_file(s, inputFile)) {
			printf("Couldn't open file: %s\n", inputFile);
			ktrace_file_close(output_file);
			ktrace_session_destroy(s);
			return false;
		}
	} else {
		assert(outputFile);
		ktrace_set_signal_handler(s);
	}

	configure_ktrace_session(s);

	ktrace_chunk_t events_chunk = ktrace_file_append_start(output_file,
			MALLOC_EVENTS_TAG, MALLOC_EVENTS_V_MAJOR, MALLOC_EVENTS_V_MINOR);
	if (!events_chunk) {
		ktrace_file_close(output_file);
		ktrace_session_destroy(s);
		return false;
	}

	void *buffer = malloc(chunk_buffer_size);
	if (!buffer) {
		printf("Could not allocate buffer for events\n");
		ktrace_file_close(output_file);
		ktrace_session_destroy(s);
		return false;
	}

	__block void *next_ptr = buffer;
	__block size_t space_left = chunk_buffer_size;

	dispatch_group_t g = dispatch_group_create();
	dispatch_queue_t q = dispatch_queue_create("Read Source File", DISPATCH_QUEUE_SERIAL);

	ktrace_events_subclass(s, DBG_UMALLOC, DBG_UMALLOC_EXTERNAL, (^(ktrace_event_t event) {
		s_totalEvents++;
		if (space_left < sizeof(compressed_operation) + sizeof(compressed_op_params)) {
			ktrace_file_append_data(output_file, events_chunk, buffer,
					chunk_buffer_size - space_left);
			blockBytesWritten += chunk_buffer_size - space_left;
			next_ptr = buffer;
			space_left = chunk_buffer_size;
		}
		unsigned int debugid = event->debugid;
		size_t entry_size = 0;

		struct compressed_operation *operation = (struct compressed_operation *)next_ptr;
		operation->core = (uint8_t)event->cpuid;
		operation->opcode = (uint8_t)KDBG_EXTRACT_CODE(debugid);

		switch (debugid) {
		case TRACE_malloc|DBG_FUNC_END:
		case TRACE_valloc|DBG_FUNC_END: {
			s_totalLibMallocEvents++;
			s_totalMallocEvents++;

			struct compressed_alloc *allocp = (struct compressed_alloc *)operation->body;
			allocp->size = (uint32_t)event->arg2;
			allocp->address = event->arg3;
			entry_size = sizeof(compressed_operation) + sizeof(struct compressed_alloc);
			break;
		}
		case TRACE_calloc|DBG_FUNC_END: {
			s_totalLibMallocEvents++;
			s_totalCallocEvents++;

			struct compressed_calloc *callocp = (struct compressed_calloc *)operation->body;
			callocp->count = (uint32_t)event->arg2;
			callocp->size = (uint32_t)event->arg3;
			callocp->address = event->arg4;
			entry_size = sizeof(compressed_operation) + sizeof(struct compressed_calloc);
			break;
		}
		case TRACE_memalign|DBG_FUNC_END: {
			s_totalLibMallocEvents++;
			s_totalMalignEvents++;

			struct compressed_memalign *malignp = (struct compressed_memalign *)operation->body;
			malignp->alignment = (uint32_t)event->arg2;
			malignp->size = (uint32_t)event->arg3;
			malignp->address = event->arg4;
			entry_size = sizeof(compressed_operation) + sizeof(struct compressed_memalign);
			break;
		}
		case TRACE_realloc|DBG_FUNC_END: {
			s_totalLibMallocEvents++;
			s_totalReallocEvents++;

			struct compressed_realloc *reallocp = (struct compressed_realloc *)operation->body;
			reallocp->oldAddress = event->arg2;
			reallocp->size = (uint32_t)event->arg3;
			reallocp->newAddress = event->arg4;
			entry_size = sizeof(compressed_operation) + sizeof(struct compressed_realloc);
			break;
		}
		case TRACE_free: {
			s_totalLibMallocEvents++;
			s_totalFreeEvents++;

			struct compressed_free *freep = (struct compressed_free *)operation->body;
			freep->address = event->arg2;
			entry_size = sizeof(compressed_operation) + sizeof(struct compressed_free);
			break;
		}
		}
		if (entry_size) {
			next_ptr = (char *)next_ptr + entry_size;
			space_left -= entry_size;
		}
	}));
	ktrace_set_completion_handler(s, ^{
		dispatch_group_leave(g);
	});
	dispatch_group_enter(g);

	if (!ktrace_start(s, q)) {
		dispatch_group_wait(g, DISPATCH_TIME_FOREVER);
	} else {
		dispatch_group_leave(g);
	}
	dispatch_release(g);
	dispatch_release(q);

	// Write out any remaining data
	if (space_left < chunk_buffer_size) {
		ktrace_file_append_data(output_file, events_chunk, buffer,
				chunk_buffer_size - space_left);
		blockBytesWritten += chunk_buffer_size - space_left;
	}
	free(buffer);

	if (ktrace_file_append_finish(output_file, events_chunk)) {
		printf("Failed to write events to %s\n", outputFile);
	}
	ktrace_file_close(output_file);
	ktrace_session_destroy(s);

    //
    //Dump out data about how many events we saw.
    //
    printf("TotalMalloc:        %16llu\n"
           "TotalCalloc:        %16llu\n"
           "TotalRealloc:       %16llu\n"
           "TotalMalign:        %16llu\n"
           "TotalFree:          %16llu\n"
           "\n"
           "TotalEvents:        %16llu\n"
           "TotalLibMalloc:     %16llu\n"
           "\n"
           "TotalBytesWritten:  %16u\n",
           s_totalMallocEvents,
           s_totalCallocEvents,
           s_totalReallocEvents,
           s_totalMalignEvents,
           s_totalFreeEvents,
           s_totalEvents,
           s_totalLibMallocEvents,
           blockBytesWritten
	);

    return true;
}


////////////////////////////////////////////////////////////////////////////////
//
// dirty_memory - Writes the minimum number of bytes to dirty a range of memory.
//
////////////////////////////////////////////////////////////////////////////////

static void
dirty_memory(uint8_t* memory, size_t size)
{
	*memory = 0xFF;

	uint8_t* current = (uint8_t*)round_page_kernel((uint64_t)memory);
	size_t good_size = malloc_good_size(size);
	while (current < (memory + good_size)) {
		*current = 0xFF;
		current += vm_kernel_page_size;
	}
}


////////////////////////////////////////////////////////////////////////////////
//
// run_event - Decodes an operation into its actual event type and then calls the
//            proper libmalloc function.  Returns the size of the event type so
//            so the caller can move to the next compressed_operation.
//
////////////////////////////////////////////////////////////////////////////////

static size_t
run_event(const struct compressed_operation* currentOperation,
		size_t remainingMapping, replay_config_t config)
{
    void* event = (void *)currentOperation->body;
    size_t bytesRead = sizeof(compressed_operation);
    remainingMapping -= sizeof(compressed_operation);

    if (s_funcMagSetThreadIndex){
        s_funcMagSetThreadIndex(currentOperation->core);
    }

	uint64_t preICount = 0;
	uint64_t postICount = 0;
	uint32_t reqAllocSize = 0;

    //printf("EVENT : %llx\n", event);
    switch (currentOperation->opcode) {
	case op_malloc: {
		if (remainingMapping < sizeof(struct compressed_alloc)) {
			return 0;
		}
		struct compressed_alloc* alloc = (struct compressed_alloc*)event;
		reqAllocSize = alloc->size;
		capture_thread_counters(preICount, config);
		uint64_t* allocation = (uint64_t*)malloc(alloc->size);
		capture_thread_counters(postICount, config);
		os_assert(allocation);
		dirty_memory((uint8_t*)allocation, alloc->size);
		s_addressMap.insert(std::make_pair(alloc->address, (uint64_t)allocation));
		s_totalMallocEvents++;

		bytesRead += sizeof(struct compressed_alloc);
		break;
	}
	case op_calloc: {
		if (remainingMapping < sizeof(struct compressed_calloc)) {
			return 0;
		}

		struct compressed_calloc* alloc = (struct compressed_calloc*)event;
		reqAllocSize = alloc->size * alloc->count;
		capture_thread_counters(preICount, config);
		uint64_t allocation = (uint64_t)calloc(alloc->count, alloc->size);
		capture_thread_counters(postICount, config);
		os_assert(allocation);
		dirty_memory((uint8_t*)allocation, alloc->size * alloc->count);
		s_addressMap.insert(std::make_pair(alloc->address, allocation));
		s_totalCallocEvents++;

		bytesRead += sizeof(struct compressed_calloc);
		break;
	}
	case op_memalign: {
		if (remainingMapping < sizeof(struct compressed_memalign)) {
			return 0;
		}

		struct compressed_memalign* alloc = (struct compressed_memalign*)event;
		reqAllocSize = alloc->size;
		uint64_t allocation = 0;
		capture_thread_counters(preICount, config);
		posix_memalign((void**)&allocation, alloc->alignment, alloc->size);
		capture_thread_counters(postICount, config);
		os_assert(allocation);
		dirty_memory((uint8_t*)allocation, alloc->size);
		s_addressMap.insert(std::make_pair(alloc->address, allocation));
		s_totalMalignEvents++;

		bytesRead += sizeof(struct compressed_memalign);
		break;
	}
	case op_valloc: {
		if (remainingMapping < sizeof(struct compressed_alloc)) {
			return 0;
		}

		struct compressed_alloc* alloc = (struct compressed_alloc*)event;
		reqAllocSize = alloc->size;
		capture_thread_counters(preICount, config);
		uint64_t allocation = (uint64_t)valloc(alloc->size);
		capture_thread_counters(postICount, config);
		os_assert(allocation);
		dirty_memory((uint8_t*)allocation, alloc->size);
		s_addressMap.insert(std::make_pair(alloc->address, allocation));
		s_totalVallocEvents++;

		bytesRead += sizeof(struct compressed_alloc);
		break;
	}
	case op_free: {
		if (remainingMapping < sizeof(struct compressed_free)) {
			return 0;
		}

		bytesRead += sizeof(struct compressed_free);

		struct compressed_free* freed = (struct compressed_free*)event;
		auto iter = s_addressMap.find(freed->address);
		if (iter == s_addressMap.end()) {
			s_totalFailedFreeEvents++;
			break;
		}
		capture_thread_counters(preICount, config);
		free((void*)iter->second);
		capture_thread_counters(postICount, config);
		s_addressMap.erase(iter);
		s_totalFreeEvents++;
		break;
	}
	case op_realloc: {
		if (remainingMapping < sizeof(struct compressed_realloc)) {
			return 0;
		}

		bytesRead += sizeof(struct compressed_realloc);

		struct compressed_realloc* alloc = (struct compressed_realloc*)event;
		reqAllocSize = alloc->size;
		auto iter = s_addressMap.find(alloc->oldAddress);
		if (iter == s_addressMap.end()) {
			s_totalFailedReallocEvents++;
			break;
		}

		uint64_t oldAddress = iter->second;
		capture_thread_counters(preICount, config);
		uint64_t newAddress = (uint64_t)realloc((void*)oldAddress, alloc->size);
		capture_thread_counters(postICount, config);
		os_assert(newAddress);
		dirty_memory((uint8_t*)newAddress, alloc->size);
		s_addressMap.erase(iter);
		s_addressMap.insert(std::make_pair(alloc->newAddress, newAddress));
		s_totalReallocEvents++;
		break;
	}
	default:
		__builtin_trap();
		break;
    };

	if (s_funcMagSetThreadIndex){
		s_funcMagSetThreadIndex(NON_REPLAY_MAGAZINE);
	}

	if (config & (CONFIG_REC_COUNTERS | CONFIG_REC_STATS)) {
		uint64_t diff = postICount - preICount;
		uint16_t instrCount = diff <= UINT16_MAX ? diff : UINT16_MAX;
		if (config & CONFIG_REC_STATS) {
			call_ins_retired[currentOperation->opcode - 1] += instrCount;
			call_count[currentOperation->opcode - 1]++;
		}
		if ((config & CONFIG_REC_COUNTERS) && reqAllocSize > 0) {
			auto lookup = CallSizePair(currentOperation->opcode, reqAllocSize);
			auto iter = s_counterDistributions.find(lookup);
			if (iter != s_counterDistributions.end()) {
				iter->second.push_back(instrCount);
			} else {
				s_counterDistributions.insert({lookup, ReplayVector(1, instrCount)});
			}
		}
	}

    return bytesRead;
}


////////////////////////////////////////////////////////////////////////////////
//
// setup_private_malloc_zone - Creates a malloc zone for use during actual replay.
//                          We need to do so in order to keep the bookkeeping
//                          separate from the replayed data.  This zone is not
//                          counted when figuring out fragmentation.
//
////////////////////////////////////////////////////////////////////////////////

static bool
setup_private_malloc_zone()
{
    s_zone = malloc_create_zone(0, 0);
    if (!s_zone) {
        printf("Couldn't create zone\n");
        return false;
    }

    malloc_set_zone_name(s_zone, "IGNORE_THIS_ZONE");
    return true;
}


////////////////////////////////////////////////////////////////////////////////
//
// memory_reader - Read from ourselves, instead of a remote process like vmmap
//                 does.
//
////////////////////////////////////////////////////////////////////////////////

static kern_return_t
memory_reader(task_t remote_task, vm_address_t remote_address, vm_size_t size,
		void **local_memory)
{
    if (local_memory) {
        *local_memory = (void*)remote_address;
        return KERN_SUCCESS;
    }

    return KERN_FAILURE;
}


////////////////////////////////////////////////////////////////////////////////
//
// vm_range_recorder - Enumerate all the malloc vm ranges, looking at each page
//                     to figure out if it is resident or not, and dirty or not.
//                     Used to calculate fragmentation.
//
////////////////////////////////////////////////////////////////////////////////

static void
vm_range_recorder(task_t task, void* context, unsigned type, vm_range_t *ranges,
		unsigned count)
{
	for (unsigned currentRange = 0; currentRange < count; currentRange++ ) {
		replay_malloc_magazine magazine = {
			.baseAddress = ranges[currentRange].address,
			.extent = ranges[currentRange].address + ranges[currentRange].size
		};

		for (uint64_t i = magazine.baseAddress; i < magazine.extent; i += vm_kernel_page_size) {
			kern_return_t err = 0;
			integer_t disposition = 0;
			integer_t refCount = 0;

			err = mach_vm_page_query(mach_task_self(), i, &disposition, &refCount);
			if (!err) {
				if (disposition & VM_PAGE_QUERY_PAGE_PRESENT) {
					if (disposition & (VM_PAGE_QUERY_PAGE_COPIED|VM_PAGE_QUERY_PAGE_DIRTY)) {
						magazine.pages_dirty++;
					}
				} else if (disposition & VM_PAGE_QUERY_PAGE_PAGED_OUT) {
					magazine.pages_dirty++;
				}
			}
		}
		((replay_malloc_zone_t)context)->magazines.push_back(magazine);
	}
}


////////////////////////////////////////////////////////////////////////////////
//
// run_malloc_replay - Replay a compressed malloc trace.  The idea here is to replay
//                   the recorded events while forcing a specific CPU.  By doing
//                   so libmalloc will target a specific magazine.  This way we
//                   can see how the current allocator would pack an old allocation
//                   stream.
//
////////////////////////////////////////////////////////////////////////////////

static bool
run_malloc_replay(const char* fileName, pdwriter_t perfDataWriter, replay_config_t config)
{
    if (!setup_private_malloc_zone()) {
        return false;
    }

	ktrace_session_t s = ktrace_session_create();
	if (ktrace_set_file(s, fileName)) {
		printf("Couldn't open file: %s\n", fileName);
		ktrace_session_destroy(s);
        return false;
	}
	configure_ktrace_session(s);

	dispatch_group_t g = dispatch_group_create();
	dispatch_queue_t q = dispatch_queue_create("Read Malloc Trace File",
			DISPATCH_QUEUE_SERIAL);

	ktrace_chunks(s, MALLOC_EVENTS_TAG, ^(ktrace_chunk_t c) {
		if (ktrace_chunk_version_major(c) != MALLOC_EVENTS_V_MAJOR
				|| ktrace_chunk_version_minor(c) != MALLOC_EVENTS_V_MINOR) {
			printf("Invalid replay file: %s\n", fileName);
			exit(1);
		}

		size_t size = (size_t)ktrace_chunk_size(c);
		off_t offset = 0;
		while (size > sizeof(compressed_operation)) {
			void *ptr;
			size_t mapped_size = size;
#ifdef MAX_REPLAY_FILE_CHUNK_SIZE
			mapped_size = MIN(mapped_size, MAX_REPLAY_FILE_CHUNK_SIZE);
#endif // MAX_REPLAY_FILE_CHUNK_SIZE

			// Map as much of the chunk as we can. If we can't map everything,
			// keep halving the requested size until we get to something that
			// works. If nothing works, bail.
			do {
				ptr = ktrace_chunk_map_data(c, offset, mapped_size);
				if (!ptr) {
					mapped_size /= 2;
				}
			} while (!ptr && mapped_size);

			if (!mapped_size) {
				perror("Could not map replay file chunk");
				exit(1);
			}

			struct compressed_operation* event = (struct compressed_operation*)ptr;
			size_t size_left = mapped_size;
			do {
				size_t read = run_event(event, size_left, config);
				if (read == 0) {
					break;
				}
				s_totalLibMallocEvents++;

				size_left -= read;
				size -= read;
				offset += read;

				event = (struct compressed_operation*)((char *)event + read);
			} while (size_left > sizeof(compressed_operation));

			ktrace_chunk_unmap_data(c, ptr, mapped_size);
		}
	});

	ktrace_set_completion_handler(s, ^{
		dispatch_group_leave(g);
	});
	dispatch_group_enter(g);

	ktrace_events_all(s, ^(ktrace_event_t event) { });
	if (!ktrace_start(s, q)) {
		dispatch_group_wait(g, DISPATCH_TIME_FOREVER);
	} else {
		dispatch_group_leave(g);
	}
	dispatch_release(g);
	dispatch_release(q);

	s_addressMap.clear();
	ktrace_session_destroy(s);

	return true;
}

static void
report_results(pdwriter_t perfDataWriter, replay_config_t config)
{
	//
    //If passed a writer, output performance data.
    //
    if (perfDataWriter) {
        pdwriter_new_value(perfDataWriter, "TotalMalloc", PDUNIT_CUSTOM(totalmalloc), s_totalMallocEvents);
        pdwriter_new_value(perfDataWriter, "TotalCalloc", PDUNIT_CUSTOM(totalcalloc), s_totalCallocEvents);
        pdwriter_new_value(perfDataWriter, "TotalRealloc", PDUNIT_CUSTOM(totalrealloc), s_totalReallocEvents);
        pdwriter_new_value(perfDataWriter, "TotalValloc", PDUNIT_CUSTOM(totalvalloc), s_totalVallocEvents);
        pdwriter_new_value(perfDataWriter, "TotalMalign", PDUNIT_CUSTOM(totalmalign), s_totalMalignEvents);
        pdwriter_new_value(perfDataWriter, "TotalFree", PDUNIT_CUSTOM(totalfree), s_totalFreeEvents);
        pdwriter_new_value(perfDataWriter, "FailedRealloc", PDUNIT_CUSTOM(failedrealloc), s_totalFailedReallocEvents);
        pdwriter_new_value(perfDataWriter, "FailedFree", PDUNIT_CUSTOM(failedfree), s_totalFailedFreeEvents);
    }

	printf("TotalMalloc:    %16llu\n"
           "TotalCalloc:    %16llu\n"
           "TotalRealloc:   %16llu\n"
           "TotalValloc:    %16llu\n"
           "TotalMalign:    %16llu\n"
           "TotalFree:      %16llu\n"
           "\n"
           "FailedRealloc:  %16llu\n"
           "FailedFree:     %16llu\n",
           s_totalMallocEvents,
           s_totalCallocEvents,
           s_totalReallocEvents,
           s_totalVallocEvents,
           s_totalMalignEvents,
           s_totalFreeEvents,
           s_totalFailedReallocEvents,
           s_totalFailedFreeEvents
           );

    //
    //Now lets go over the data and find how fragmented we are.
    //
    vm_address_t* addresses = NULL;
    unsigned count = 0;

    printf("\n\n\n");
    printf("Zone:                     BytesDirty      BytesInUse  %%Frag\n");
    printf("===========================================================\n");

	double defaultFrag = 0;

    malloc_get_all_zones(mach_task_self(), memory_reader, &addresses, &count);
    for (unsigned i = 0; i < count; i++) {
        malloc_zone_t* zone = (malloc_zone_t*)addresses[i];
        replay_malloc_zone zoneInfo = { 0 };
        if (strcmp(zone->zone_name, "IGNORE_THIS_ZONE") != 0) {
            malloc_statistics_t stats = {0};
            zone->introspect->enumerator(mach_task_self(), &zoneInfo, MALLOC_PTR_REGION_RANGE_TYPE, (vm_address_t)zone, memory_reader, vm_range_recorder);
            zone->introspect->statistics(zone, &stats);

            uint64_t bytesDirty = 0;

            for (const auto& magazine : zoneInfo.magazines) {
                bytesDirty += magazine.pages_dirty * vm_kernel_page_size;
				//printf("%llx %llx %d\n", magazine.baseAddress, magazine.extent, magazine.pages_dirty);
            }

            double frag = (bytesDirty && (stats.size_in_use < bytesDirty)) ? 100 - (100.0 * stats.size_in_use)/bytesDirty : 0;

            printf("%20s  %14llu  %14lu %6.2f\n", zone->zone_name, bytesDirty, stats.size_in_use, frag);
            if (perfDataWriter) {
                pdwriter_new_value(perfDataWriter, "BytesDirty", pdunit_bytes, bytesDirty);
                pdwriter_record_variable_str(perfDataWriter, "ZoneName", zone->zone_name);
                pdwriter_new_value(perfDataWriter, "BytesInUse", pdunit_bytes, stats.size_in_use);
                pdwriter_record_variable_str(perfDataWriter, "ZoneName", zone->zone_name);
                pdwriter_new_value(perfDataWriter, "Fragmentation", PDUNIT_CUSTOM(FragmentedPercent), frag);
                pdwriter_record_variable_str(perfDataWriter, "ZoneName", zone->zone_name);

				if (strcmp(zone->zone_name, _DefaultNanoZone) == 0) {
					defaultFrag = frag;
				}
            }
        }
    }

	if (perfDataWriter) {
		//
		//Write out the fragmentation in DefaultMallocZone as a primary metric.
		//
		pdwriter_new_value(perfDataWriter, _DefaultFragMetricName, PDUNIT_CUSTOM(FragmentedPercent), defaultFrag);
		pdwriter_record_variable(perfDataWriter, kPCFailureThresholdPctVar, 10);
	} else if (config & CONFIG_REC_STATS) {
		printf("\n\n\n");
		printf("Call       Cycles (mean)\n");
		printf("=====================\n");
	}

	//
	//If we were asked to gather instruction counts, iterate through them and
	//either output the mean for the call or the raw counts for each
	//call:requested-size pair.
	//
	if (config & (CONFIG_REC_COUNTERS | CONFIG_REC_STATS)) {
		json_t jsonW = NULL;
		if (perfDataWriter && (config & CONFIG_REC_COUNTERS)) {
			//
			//Write out the instruction count data. We record into an extension
			//since there's typically a large numbers of counts.
			//
			jsonW = pdwriter_start_extension(perfDataWriter, "libmalloc.instruction_counts");
			if (jsonW) {
				for (auto const &mCallDistribution : s_counterDistributions) {
					//
					//If requested, write the i-counts out to the perfdata.
					//
					char description[16];
					snprintf(description, sizeof(description), "%d:%u", mCallDistribution.first.first, mCallDistribution.first.second);
					json_member_start_object(jsonW, description);
					json_member_int(jsonW, "call", mCallDistribution.first.first);
					json_member_int(jsonW, "size", mCallDistribution.first.second);
					json_member_uint(jsonW, "count", (unsigned int)mCallDistribution.second.size());
					json_member_start_array(jsonW, "values");
					for (uint64_t val : mCallDistribution.second) {
						json_value_uint(jsonW, (unsigned int)val);
					}
					json_end_array(jsonW); // Inner counts
					json_end_object(jsonW);
				}
			}
		}

		//
		//Output the mean number of instructions retired.
		//
		if (config & CONFIG_REC_STATS) {
			for (int i = 0; i < operation_count; i++) {
				if (call_ins_retired[i] > 0 && call_count[i] > 0) {
					uint64_t mean = call_ins_retired[i] / call_count[i];
					if (perfDataWriter) {
						char full_name[16];
						// operation enum is indexed from 1, adjust index for mcall_to_name.
						snprintf(full_name, sizeof(full_name), "%s-mean", mcall_to_name(i + 1));
						pdwriter_new_value(perfDataWriter, full_name, pdunit_instructions, mean);
						pdwriter_record_variable(perfDataWriter, kPCFailureThresholdPctVar, 100);
					} else {
						printf("%9s  %6llu\n", mcall_to_name(i + 1), mean);
					}
				}
			}
		}

		if (jsonW) {
			pdwriter_end_extension(perfDataWriter, jsonW);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// usage - Output help.
//
////////////////////////////////////////////////////////////////////////////////

static void
usage()
{
    printf("libmalloc_replay -r <input mtrace file> [-p] [-j filename] [-t testname] [-c | -s]\n");
    printf("libmalloc_replay [-i <input artrace file>] -o <output mtrace file> [-p]\n");
    printf("\t-p Pause the replay process before exit\n");
    printf("\t-j  <output file>\toutput perfdata V2 formatted file\n");
    printf("\t-t  <test name>\tset the test name for the perfdata V2 formatted output file\n");
    printf("\t-c capture and output instruction counts along with the performance data.\n");
    printf("\t-s capture and output instruction count statistics along with the performance data.\n");
}


////////////////////////////////////////////////////////////////////////////////
//
// main - Yep.
//
////////////////////////////////////////////////////////////////////////////////

int
main(int argc, char** argv)
{
    char * inputMTrace = NULL;
    char * inputKtrace = NULL;
    char * outputMTrace = NULL;
    char * outputPerfData = NULL;
    char * outputTestName = NULL;
    replay_config_t config = 0;
    int c = 0;

    if (argc < 2) {
        usage();
        return -1;
    }

    while ((c = getopt(argc, (char* const*)argv, "phr:i:o:j:t:cs")) != -1) {
      switch (c) {
		case 'r':
			inputMTrace = strdup(optarg);
			config |= CONFIG_RUN_REPLAY;
			break;
		case 'i':
			inputKtrace = strdup(optarg);
			if (inputKtrace && outputMTrace) {
				config |= CONFIG_CONVERT_FILE;
			}
			break;
		case 'o':
			outputMTrace = strdup(optarg);
			if (inputKtrace && outputMTrace) {
				config |= CONFIG_CONVERT_FILE;
			}
			break;
		case 'p':
			config |= CONFIG_PAUSE;
			break;
		case 'j':
			outputPerfData = strdup(optarg);
			break;
		case 't':
			outputTestName = strdup(optarg);
			break;
		case 'c':
			config |= CONFIG_REC_COUNTERS;
			break;
		case 's':
			config |= CONFIG_REC_STATS;
			break;
		case 'h':
		default:
			usage();
			return EX_USAGE;
        }
    }

	if ((config & CONFIG_REC_COUNTERS) && (config & CONFIG_REC_STATS)) {
		printf("Invalid usage: -c and -s\n");
		usage();
		return EX_USAGE;
	}

	timespec beginTime = {0};
	timespec endTime = {0};

    pdwriter_t writer = NULL;
    if (outputPerfData) {
        char dataPath[MAXPATHLEN];

        //
        //Ensure the filename is prepended with libmalloc
        //
        const char *prepend = "libmalloc";
        auto outputFilePath = std::string(outputPerfData);
        const auto namePos = outputFilePath.find_last_of('/') + 1;
        if (outputFilePath.find(prepend, namePos) != namePos) {
            outputFilePath.insert(namePos, prepend);
        }
        int ret = snprintf(dataPath, sizeof(dataPath), "%s.%d.%llx." PD_FILE_EXT,
                           outputFilePath.c_str(), getpid(), mach_absolute_time());
        if (ret < 0) {
            return errno;
        }
		auto perfdataName = std::string("libmalloc.replay.");
		perfdataName += outputTestName ? outputTestName : dataPath;
		writer = pdwriter_open(dataPath, perfdataName.c_str(), 0, 0);
		if (!writer) {
			printf("\n****Couldn't open writer for performance data file. Error: %s\n", strerror(errno));
		} else {
			pdwriter_set_primary_metric(writer, _DefaultFragMetricName);
		}
    }

	if (config & CONFIG_RUN_REPLAY) {
		void *libmalloc = dlopen("/usr/lib/system/libsystem_malloc.dylib", RTLD_NOW);
		if (libmalloc) {
			s_funcMagSetThreadIndex = (void (*)(unsigned int))dlsym(libmalloc, "mag_set_thread_index");
		}

		if (!s_funcMagSetThreadIndex) {
			printf("\n****Couldn't load mag_set_thread_index, replay won't honor core****\n\n");
		} else {
			s_funcMagSetThreadIndex(NON_REPLAY_MAGAZINE);
		}

		clock_gettime(CLOCK_MONOTONIC_RAW, &beginTime);

		if (!run_malloc_replay(inputMTrace, writer, config)) {
			return -1;
		}

		clock_gettime(CLOCK_MONOTONIC_RAW, &endTime);

		report_results(writer, config);
	} else if (config & CONFIG_CONVERT_FILE) {
		clock_gettime(CLOCK_MONOTONIC_RAW, &beginTime);
		if (!run_ktrace(inputKtrace, outputMTrace)) {
			printf("\n****Couldn't record mtrace file.\n");
		}
		clock_gettime(CLOCK_MONOTONIC_RAW, &endTime);
	} else if (outputMTrace) {
		clock_gettime(CLOCK_MONOTONIC_RAW, &beginTime);
		if (!run_ktrace(NULL, outputMTrace)) {
			printf("\n****Couldn't record mtrace file.\n");
		}
		clock_gettime(CLOCK_MONOTONIC_RAW, &endTime);
	}

	if (beginTime.tv_sec) {
		printf("\n\nRuntime: %ld ms\n", ((endTime.tv_sec - beginTime.tv_sec) * 1000) + (endTime.tv_nsec - beginTime.tv_nsec)/1000000);
	}

    if (writer) {
      pdwriter_close(writer);
    }

	if (config & CONFIG_PAUSE) {
		printf("\n\nProcess paused, hit Crtl+C to exit\n");
		pause();
	}

    return 0;
}
