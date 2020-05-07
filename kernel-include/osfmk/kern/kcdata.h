/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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


/*
 *
 *                     THE KCDATA MANIFESTO
 *
 *   Kcdata is a self-describing data serialization format.  It is meant to get
 *   nested data structures out of xnu with minimum fuss, but also for that data
 *   to be easy to parse.  It is also meant to allow us to add new fields and
 *   evolve the data format without breaking old parsers.
 *
 *   Kcdata is a permanent data format suitable for long-term storage including
 *   in files.  It is very important that we continue to be able to parse old
 *   versions of kcdata-based formats.  To this end, there are several
 *   invariants you MUST MAINTAIN if you alter this file.
 *
 *     * None of the magic numbers should ever be a byteswap of themselves or
 *       of any of the other magic numbers.
 *
 *     * Never remove any type.
 *
 *     * All kcdata structs must be packed, and must exclusively use fixed-size
 *        types.
 *
 *     * Never change the definition of any type, except to add new fields to
 *      the end.
 *
 *     * If you do add new fields to the end of a type, do not actually change
 *       the definition of the old structure.  Instead, define a new structure
 *       with the new fields.  See thread_snapshot_v3 as an example.  This
 *       provides source compatibility for old readers, and also documents where
 *       the potential size cutoffs are.
 *
 *     * If you change libkdd, or kcdata.py run the unit tests under libkdd.
 *
 *     * If you add a type or extend an existing one, add a sample test to
 *       libkdd/tests so future changes to libkdd will always parse your struct
 *       correctly.
 *
 *       For example to add a field to this:
 *
 *          struct foobar {
 *              uint32_t baz;
 *              uint32_t quux;
 *          } __attribute__ ((packed));
 *
 *       Make it look like this:
 *
 *          struct foobar {
 *              uint32_t baz;
 *              uint32_t quux;
 *              ///////// end version 1 of foobar.  sizeof(struct foobar) was 8 ////////
 *              uint32_t frozzle;
 *          } __attribute__ ((packed));
 *
 *   If you are parsing kcdata formats, you MUST
 *
 *     * Check the length field of each struct, including array elements.   If the
 *       struct is longer than you expect, you must ignore the extra data.
 *
 *     * Ignore any data types you do not understand.
 *
 *   Additionally, we want to be as forward compatible as we can.  Meaning old
 *   tools should still be able to use new data whenever possible.  To this end,
 *   you should:
 *
 *     * Try not to add new versions of types that supplant old ones.  Instead
 *        extend the length of existing types or add supplemental types.
 *
 *     * Try not to remove information from existing kcdata formats, unless
 *        removal was explicitly asked for.  For example it is fine to add a
 *        stackshot flag to remove unwanted information, but you should not
 *        remove it from the default stackshot if the new flag is absent.
 *
 *     * (TBD) If you do break old readers by removing information or
 *        supplanting old structs, then increase the major version number.
 *
 *
 *
 *  The following is a description of the kcdata format.
 *
 *
 * The format for data is setup in a generic format as follows
 *
 * Layout of data structure:
 *
 *   |         8 - bytes         |
 *   |  type = MAGIC |  LENGTH   |
 *   |            0              |
 *   |      type     |  size     |
 *   |          flags            |
 *   |           data            |
 *   |___________data____________|
 *   |      type     |   size    |
 *   |          flags            |
 *   |___________data____________|
 *   |  type = END   |  size=0   |
 *   |            0              |
 *
 *
 * The type field describes what kind of data is passed. For example type = TASK_CRASHINFO_UUID means the following data is a uuid.
 * These types need to be defined in task_corpses.h for easy consumption by userspace inspection tools.
 *
 * Some range of types is reserved for special types like ints, longs etc. A cool new functionality made possible with this
 * extensible data format is that kernel can decide to put more information as required without requiring user space tools to
 * re-compile to be compatible. The case of rusage struct versions could be introduced without breaking existing tools.
 *
 * Feature description: Generic data with description
 * -------------------
 * Further more generic data with description is very much possible now. For example
 *
 *   - kcdata_add_uint64_with_description(cdatainfo, 0x700, "NUM MACH PORTS");
 *   - and more functions that allow adding description.
 * The userspace tools can then look at the description and print the data even if they are not compiled with knowledge of the field apriori.
 *
 *  Example data:
 * 0000  57 f1 ad de 00 00 00 00 00 00 00 00 00 00 00 00  W...............
 * 0010  01 00 00 00 00 00 00 00 30 00 00 00 00 00 00 00  ........0.......
 * 0020  50 49 44 00 00 00 00 00 00 00 00 00 00 00 00 00  PID.............
 * 0030  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
 * 0040  9c 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
 * 0050  01 00 00 00 00 00 00 00 30 00 00 00 00 00 00 00  ........0.......
 * 0060  50 41 52 45 4e 54 20 50 49 44 00 00 00 00 00 00  PARENT PID......
 * 0070  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
 * 0080  01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
 * 0090  ed 58 91 f1
 *
 * Feature description: Container markers for compound data
 * ------------------
 * If a given kernel data type is complex and requires adding multiple optional fields inside a container
 * object for a consumer to understand arbitrary data, we package it using container markers.
 *
 * For example, the stackshot code gathers information and describes the state of a given task with respect
 * to many subsystems. It includes data such as io stats, vm counters, process names/flags and syscall counts.
 *
 * kcdata_add_container_marker(kcdata_p, KCDATA_TYPE_CONTAINER_BEGIN, STACKSHOT_KCCONTAINER_TASK, task_uniqueid);
 * // add multiple data, or add_<type>_with_description()s here
 *
 * kcdata_add_container_marker(kcdata_p, KCDATA_TYPE_CONTAINER_END, STACKSHOT_KCCONTAINER_TASK, task_uniqueid);
 *
 * Feature description: Custom Data formats on demand
 * --------------------
 * With the self describing nature of format, the kernel provider can describe a data type (uniquely identified by a number) and use
 * it in the buffer for sending data. The consumer can parse the type information and have knowledge of describing incoming data.
 * Following is an example of how we can describe a kernel specific struct sample_disk_io_stats in buffer.
 *
 * struct sample_disk_io_stats {
 *     uint64_t        disk_reads_count;
 *     uint64_t        disk_reads_size;
 *     uint64_t        io_priority_count[4];
 *     uint64_t        io_priority_size;
 * } __attribute__ ((packed));
 *
 *
 * struct kcdata_subtype_descriptor disk_io_stats_def[] = {
 *     {KCS_SUBTYPE_FLAGS_NONE, KC_ST_UINT64, 0 * sizeof(uint64_t), sizeof(uint64_t), "disk_reads_count"},
 *     {KCS_SUBTYPE_FLAGS_NONE, KC_ST_UINT64, 1 * sizeof(uint64_t), sizeof(uint64_t), "disk_reads_size"},
 *     {KCS_SUBTYPE_FLAGS_ARRAY, KC_ST_UINT64, 2 * sizeof(uint64_t), KCS_SUBTYPE_PACK_SIZE(4, sizeof(uint64_t)), "io_priority_count"},
 *     {KCS_SUBTYPE_FLAGS_ARRAY, KC_ST_UINT64, (2 + 4) * sizeof(uint64_t), sizeof(uint64_t), "io_priority_size"},
 * };
 *
 * Now you can add this custom type definition into the buffer as
 * kcdata_add_type_definition(kcdata_p, KCTYPE_SAMPLE_DISK_IO_STATS, "sample_disk_io_stats",
 *          &disk_io_stats_def[0], sizeof(disk_io_stats_def)/sizeof(struct kcdata_subtype_descriptor));
 *
 */


#ifndef _KCDATA_H_
#define _KCDATA_H_

#include <stdint.h>
#include <string.h>
#include <uuid/uuid.h>

#define KCDATA_DESC_MAXLEN 32 /* including NULL byte at end */

#define KCDATA_FLAGS_STRUCT_PADDING_MASK 0xf
#define KCDATA_FLAGS_STRUCT_HAS_PADDING 0x80

/*
 * kcdata aligns elements to 16 byte boundaries.
 */
#define KCDATA_ALIGNMENT_SIZE       0x10

struct kcdata_item {
	uint32_t type;
	uint32_t size; /* len(data)  */
	               /* flags.
	                *
	                * For structures:
	                *    padding      = flags & 0xf
	                *    has_padding  = (flags & 0x80) >> 7
	                *
	                * has_padding is needed to disambiguate cases such as
	                * thread_snapshot_v2 and thread_snapshot_v3.  Their
	                * respective sizes are 0x68 and 0x70, and thread_snapshot_v2
	                * was emmitted by old kernels *before* we started recording
	                * padding.  Since legacy thread_snapsht_v2 and modern
	                * thread_snapshot_v3 will both record 0 for the padding
	                * flags, we need some other bit which will be nonzero in the
	                * flags to disambiguate.
	                *
	                * This is why we hardcode a special case for
	                * STACKSHOT_KCTYPE_THREAD_SNAPSHOT into the iterator
	                * functions below.  There is only a finite number of such
	                * hardcodings which will ever be needed.  They can occur
	                * when:
	                *
	                *  * We have a legacy structure that predates padding flags
	                *
	                *  * which we want to extend without changing the kcdata type
	                *
	                *  * by only so many bytes as would fit in the space that
	                *  was previously unused padding.
	                *
	                * For containers:
	                *    container_id = flags
	                *
	                * For arrays:
	                *    element_count = flags & UINT32_MAX
	                *    element_type = (flags >> 32) & UINT32_MAX
	                */
	uint64_t flags;
	char data[]; /* must be at the end */
};

typedef struct kcdata_item * kcdata_item_t;

enum KCDATA_SUBTYPE_TYPES { KC_ST_CHAR = 1, KC_ST_INT8, KC_ST_UINT8, KC_ST_INT16, KC_ST_UINT16, KC_ST_INT32, KC_ST_UINT32, KC_ST_INT64, KC_ST_UINT64 };
typedef enum KCDATA_SUBTYPE_TYPES kctype_subtype_t;

/*
 * A subtype description structure that defines
 * how a compound data is laid out in memory. This
 * provides on the fly definition of types and consumption
 * by the parser.
 */
struct kcdata_subtype_descriptor {
	uint8_t kcs_flags;
#define KCS_SUBTYPE_FLAGS_NONE 0x0
#define KCS_SUBTYPE_FLAGS_ARRAY 0x1
/* Force struct type even if only one element.
 *
 * Normally a kcdata_type_definition is treated as a structure if it has
 * more than one subtype descriptor.  Otherwise it is treated as a simple
 * type.  For example libkdd will represent a simple integer 42 as simply
 * 42, but it will represent a structure containing an integer 42 as
 * {"field_name": 42}..
 *
 * If a kcdata_type_definition has only single subtype, then it will be
 * treated as a structure iff KCS_SUBTYPE_FLAGS_STRUCT is set.  If it has
 * multiple subtypes, it will always be treated as a structure.
 *
 * KCS_SUBTYPE_FLAGS_MERGE has the opposite effect.  If this flag is used then
 * even if there are multiple elements, they will all be treated as individual
 * properties of the parent dictionary.
 */
#define KCS_SUBTYPE_FLAGS_STRUCT 0x2                    /* force struct type even if only one element */
#define KCS_SUBTYPE_FLAGS_MERGE 0x4                     /* treat as multiple elements of parents instead of struct */
	uint8_t kcs_elem_type;                              /* restricted to kctype_subtype_t */
	uint16_t kcs_elem_offset;                           /* offset in struct where data is found */
	uint32_t kcs_elem_size;                             /* size of element (or) packed state for array type */
	char                 kcs_name[KCDATA_DESC_MAXLEN];  /* max 31 bytes for name of field */
};

typedef struct kcdata_subtype_descriptor * kcdata_subtype_descriptor_t;

/*
 * In case of array of basic c types in kctype_subtype_t,
 * size is packed in lower 16 bits and
 * count is packed in upper 16 bits of kcs_elem_size field.
 */
#define KCS_SUBTYPE_PACK_SIZE(e_count, e_size) (((e_count)&0xffffu) << 16 | ((e_size)&0xffffu))

static inline uint32_t
kcs_get_elem_size(kcdata_subtype_descriptor_t d)
{
	if (d->kcs_flags & KCS_SUBTYPE_FLAGS_ARRAY) {
		/* size is composed as ((count &0xffff)<<16 | (elem_size & 0xffff)) */
		return (uint32_t)((d->kcs_elem_size & 0xffff) * ((d->kcs_elem_size & 0xffff0000) >> 16));
	}
	return d->kcs_elem_size;
}

static inline uint32_t
kcs_get_elem_count(kcdata_subtype_descriptor_t d)
{
	if (d->kcs_flags & KCS_SUBTYPE_FLAGS_ARRAY) {
		return (d->kcs_elem_size >> 16) & 0xffff;
	}
	return 1;
}

static inline int
kcs_set_elem_size(kcdata_subtype_descriptor_t d, uint32_t size, uint32_t count)
{
	if (count > 1) {
		/* means we are setting up an array */
		if (size > 0xffff || count > 0xffff) {
			return -1; //invalid argument
		}
		d->kcs_elem_size = ((count & 0xffff) << 16 | (size & 0xffff));
	} else {
		d->kcs_elem_size = size;
	}
	return 0;
}

struct kcdata_type_definition {
	uint32_t kct_type_identifier;
	uint32_t kct_num_elements;
	char kct_name[KCDATA_DESC_MAXLEN];
	struct kcdata_subtype_descriptor kct_elements[];
};


/* chunk type definitions. 0 - 0x7ff are reserved  and defined here
 * NOTE: Please update kcdata/libkdd/kcdtypes.c if you make any changes
 * in STACKSHOT_KCTYPE_* types.
 */

/*
 * Types with description value.
 * these will have KCDATA_DESC_MAXLEN-1 length string description
 * and rest of kcdata_iter_size() - KCDATA_DESC_MAXLEN bytes as data
 */
#define KCDATA_TYPE_INVALID 0x0u
#define KCDATA_TYPE_STRING_DESC 0x1u
#define KCDATA_TYPE_UINT32_DESC 0x2u
#define KCDATA_TYPE_UINT64_DESC 0x3u
#define KCDATA_TYPE_INT32_DESC 0x4u
#define KCDATA_TYPE_INT64_DESC 0x5u
#define KCDATA_TYPE_BINDATA_DESC 0x6u

/*
 * Compound type definitions
 */
#define KCDATA_TYPE_ARRAY 0x11u         /* Array of data OBSOLETE DONT USE THIS*/
#define KCDATA_TYPE_TYPEDEFINTION 0x12u /* Meta type that describes a type on the fly. */
#define KCDATA_TYPE_CONTAINER_BEGIN                                       \
	0x13u /* Container type which has corresponding CONTAINER_END header. \
	       * KCDATA_TYPE_CONTAINER_BEGIN has type in the data segment. \
	       * Both headers have (uint64_t) ID for matching up nested data. \
	       */
#define KCDATA_TYPE_CONTAINER_END 0x14u

#define KCDATA_TYPE_ARRAY_PAD0 0x20u /* Array of data with 0 byte of padding*/
#define KCDATA_TYPE_ARRAY_PAD1 0x21u /* Array of data with 1 byte of padding*/
#define KCDATA_TYPE_ARRAY_PAD2 0x22u /* Array of data with 2 byte of padding*/
#define KCDATA_TYPE_ARRAY_PAD3 0x23u /* Array of data with 3 byte of padding*/
#define KCDATA_TYPE_ARRAY_PAD4 0x24u /* Array of data with 4 byte of padding*/
#define KCDATA_TYPE_ARRAY_PAD5 0x25u /* Array of data with 5 byte of padding*/
#define KCDATA_TYPE_ARRAY_PAD6 0x26u /* Array of data with 6 byte of padding*/
#define KCDATA_TYPE_ARRAY_PAD7 0x27u /* Array of data with 7 byte of padding*/
#define KCDATA_TYPE_ARRAY_PAD8 0x28u /* Array of data with 8 byte of padding*/
#define KCDATA_TYPE_ARRAY_PAD9 0x29u /* Array of data with 9 byte of padding*/
#define KCDATA_TYPE_ARRAY_PADa 0x2au /* Array of data with a byte of padding*/
#define KCDATA_TYPE_ARRAY_PADb 0x2bu /* Array of data with b byte of padding*/
#define KCDATA_TYPE_ARRAY_PADc 0x2cu /* Array of data with c byte of padding*/
#define KCDATA_TYPE_ARRAY_PADd 0x2du /* Array of data with d byte of padding*/
#define KCDATA_TYPE_ARRAY_PADe 0x2eu /* Array of data with e byte of padding*/
#define KCDATA_TYPE_ARRAY_PADf 0x2fu /* Array of data with f byte of padding*/

/*
 * Generic data types that are most commonly used
 */
#define KCDATA_TYPE_LIBRARY_LOADINFO 0x30u   /* struct dyld_uuid_info_32 */
#define KCDATA_TYPE_LIBRARY_LOADINFO64 0x31u /* struct dyld_uuid_info_64 */
#define KCDATA_TYPE_TIMEBASE 0x32u           /* struct mach_timebase_info */
#define KCDATA_TYPE_MACH_ABSOLUTE_TIME 0x33u /* uint64_t */
#define KCDATA_TYPE_TIMEVAL 0x34u            /* struct timeval64 */
#define KCDATA_TYPE_USECS_SINCE_EPOCH 0x35u  /* time in usecs uint64_t */
#define KCDATA_TYPE_PID 0x36u                /* int32_t */
#define KCDATA_TYPE_PROCNAME 0x37u           /* char * */
#define KCDATA_TYPE_NESTED_KCDATA 0x38u      /* nested kcdata buffer */

#define KCDATA_TYPE_BUFFER_END 0xF19158EDu

/* MAGIC numbers defined for each class of chunked data
 *
 * To future-proof against big-endian arches, make sure none of these magic
 * numbers are byteswaps of each other
 */

#define KCDATA_BUFFER_BEGIN_CRASHINFO 0xDEADF157u       /* owner: corpses/task_corpse.h */
                                                        /* type-range: 0x800 - 0x8ff */
#define KCDATA_BUFFER_BEGIN_STACKSHOT 0x59a25807u       /* owner: sys/stackshot.h */
                                                        /* type-range: 0x900 - 0x93f */
#define KCDATA_BUFFER_BEGIN_DELTA_STACKSHOT 0xDE17A59Au /* owner: sys/stackshot.h */
                                                        /* type-range: 0x940 - 0x9ff */
#define KCDATA_BUFFER_BEGIN_OS_REASON 0x53A20900u       /* owner: sys/reason.h */
                                                        /* type-range: 0x1000-0x103f */
#define KCDATA_BUFFER_BEGIN_XNUPOST_CONFIG 0x1e21c09fu  /* owner: osfmk/tests/kernel_tests.c */
                                                        /* type-range: 0x1040-0x105f */

/* next type range number available 0x1060 */
/**************** definitions for XNUPOST *********************/
#define XNUPOST_KCTYPE_TESTCONFIG               0x1040

/**************** definitions for stackshot *********************/

/* This value must always match IO_NUM_PRIORITIES defined in thread_info.h */
#define STACKSHOT_IO_NUM_PRIORITIES     4
/* This value must always match MAXTHREADNAMESIZE used in bsd */
#define STACKSHOT_MAX_THREAD_NAME_SIZE  64

/*
 * NOTE: Please update kcdata/libkdd/kcdtypes.c if you make any changes
 * in STACKSHOT_KCTYPE_* types.
 */
#define STACKSHOT_KCTYPE_IOSTATS                     0x901u /* io_stats_snapshot */
#define STACKSHOT_KCTYPE_GLOBAL_MEM_STATS            0x902u /* struct mem_and_io_snapshot */
#define STACKSHOT_KCCONTAINER_TASK                   0x903u
#define STACKSHOT_KCCONTAINER_THREAD                 0x904u
#define STACKSHOT_KCTYPE_TASK_SNAPSHOT               0x905u /* task_snapshot_v2 */
#define STACKSHOT_KCTYPE_THREAD_SNAPSHOT             0x906u /* thread_snapshot_v2, thread_snapshot_v3 */
#define STACKSHOT_KCTYPE_DONATING_PIDS               0x907u /* int[] */
#define STACKSHOT_KCTYPE_SHAREDCACHE_LOADINFO        0x908u /* same as KCDATA_TYPE_LIBRARY_LOADINFO64 */
#define STACKSHOT_KCTYPE_THREAD_NAME                 0x909u /* char[] */
#define STACKSHOT_KCTYPE_KERN_STACKFRAME             0x90Au /* struct stack_snapshot_frame32 */
#define STACKSHOT_KCTYPE_KERN_STACKFRAME64           0x90Bu /* struct stack_snapshot_frame64 */
#define STACKSHOT_KCTYPE_USER_STACKFRAME             0x90Cu /* struct stack_snapshot_frame32 */
#define STACKSHOT_KCTYPE_USER_STACKFRAME64           0x90Du /* struct stack_snapshot_frame64 */
#define STACKSHOT_KCTYPE_BOOTARGS                    0x90Eu /* boot args string */
#define STACKSHOT_KCTYPE_OSVERSION                   0x90Fu /* os version string */
#define STACKSHOT_KCTYPE_KERN_PAGE_SIZE              0x910u /* kernel page size in uint32_t */
#define STACKSHOT_KCTYPE_JETSAM_LEVEL                0x911u /* jetsam level in uint32_t */
#define STACKSHOT_KCTYPE_DELTA_SINCE_TIMESTAMP       0x912u /* timestamp used for the delta stackshot */
#define STACKSHOT_KCTYPE_KERN_STACKLR                0x913u /* uint32_t */
#define STACKSHOT_KCTYPE_KERN_STACKLR64              0x914u /* uint64_t */
#define STACKSHOT_KCTYPE_USER_STACKLR                0x915u /* uint32_t */
#define STACKSHOT_KCTYPE_USER_STACKLR64              0x916u /* uint64_t */
#define STACKSHOT_KCTYPE_NONRUNNABLE_TIDS            0x917u /* uint64_t */
#define STACKSHOT_KCTYPE_NONRUNNABLE_TASKS           0x918u /* uint64_t */
#define STACKSHOT_KCTYPE_CPU_TIMES                   0x919u /* struct stackshot_cpu_times or stackshot_cpu_times_v2 */
#define STACKSHOT_KCTYPE_STACKSHOT_DURATION          0x91au /* struct stackshot_duration */
#define STACKSHOT_KCTYPE_STACKSHOT_FAULT_STATS       0x91bu /* struct stackshot_fault_stats */
#define STACKSHOT_KCTYPE_KERNELCACHE_LOADINFO        0x91cu /* kernelcache UUID -- same as KCDATA_TYPE_LIBRARY_LOADINFO64 */
#define STACKSHOT_KCTYPE_THREAD_WAITINFO             0x91du /* struct stackshot_thread_waitinfo */
#define STACKSHOT_KCTYPE_THREAD_GROUP_SNAPSHOT       0x91eu /* struct thread_group_snapshot or thread_group_snapshot_v2 */
#define STACKSHOT_KCTYPE_THREAD_GROUP                0x91fu /* uint64_t */
#define STACKSHOT_KCTYPE_JETSAM_COALITION_SNAPSHOT   0x920u /* struct jetsam_coalition_snapshot */
#define STACKSHOT_KCTYPE_JETSAM_COALITION            0x921u /* uint64_t */
#define STACKSHOT_KCTYPE_THREAD_POLICY_VERSION       0x922u /* THREAD_POLICY_INTERNAL_STRUCT_VERSION in uint32 */
#define STACKSHOT_KCTYPE_INSTRS_CYCLES               0x923u /* struct instrs_cycles_snapshot */
#define STACKSHOT_KCTYPE_USER_STACKTOP               0x924u /* struct stack_snapshot_stacktop */
#define STACKSHOT_KCTYPE_ASID                        0x925u /* uint32_t */
#define STACKSHOT_KCTYPE_PAGE_TABLES                 0x926u /* uint64_t */
#define STACKSHOT_KCTYPE_SYS_SHAREDCACHE_LAYOUT      0x927u /* same as KCDATA_TYPE_LIBRARY_LOADINFO64 */
#define STACKSHOT_KCTYPE_THREAD_DISPATCH_QUEUE_LABEL 0x928u /* dispatch queue label */
#define STACKSHOT_KCTYPE_THREAD_TURNSTILEINFO        0x929u /* struct stackshot_thread_turnstileinfo */

#define STACKSHOT_KCTYPE_TASK_DELTA_SNAPSHOT 0x940u   /* task_delta_snapshot_v2 */
#define STACKSHOT_KCTYPE_THREAD_DELTA_SNAPSHOT 0x941u /* thread_delta_snapshot_v* */

struct stack_snapshot_frame32 {
	uint32_t lr;
	uint32_t sp;
};

struct stack_snapshot_frame64 {
	uint64_t lr;
	uint64_t sp;
};

struct dyld_uuid_info_32 {
	uint32_t imageLoadAddress; /* base address image is mapped at */
	uuid_t   imageUUID;
};

struct dyld_uuid_info_64 {
	uint64_t imageLoadAddress; /* XXX image slide */
	uuid_t   imageUUID;
};

struct dyld_uuid_info_64_v2 {
	uint64_t imageLoadAddress; /* XXX image slide */
	uuid_t   imageUUID;
	/* end of version 1 of dyld_uuid_info_64. sizeof v1 was 24 */
	uint64_t imageSlidBaseAddress; /* slid base address of image */
};

struct user32_dyld_uuid_info {
	uint32_t        imageLoadAddress;       /* base address image is mapped into */
	uuid_t                  imageUUID;                      /* UUID of image */
};

struct user64_dyld_uuid_info {
	uint64_t        imageLoadAddress;       /* base address image is mapped into */
	uuid_t                  imageUUID;                      /* UUID of image */
};

enum task_snapshot_flags {
	/* k{User,Kernel}64_p (values 0x1 and 0x2) are defined in generic_snapshot_flags */
	kTaskRsrcFlagged                      = 0x4, // In the EXC_RESOURCE danger zone?
	kTerminatedSnapshot                   = 0x8,
	kPidSuspended                         = 0x10, // true for suspended task
	kFrozen                               = 0x20, // true for hibernated task (along with pidsuspended)
	kTaskDarwinBG                         = 0x40,
	kTaskExtDarwinBG                      = 0x80,
	kTaskVisVisible                       = 0x100,
	kTaskVisNonvisible                    = 0x200,
	kTaskIsForeground                     = 0x400,
	kTaskIsBoosted                        = 0x800,
	kTaskIsSuppressed                     = 0x1000,
	kTaskIsTimerThrottled                 = 0x2000, /* deprecated */
	kTaskIsImpDonor                       = 0x4000,
	kTaskIsLiveImpDonor                   = 0x8000,
	kTaskIsDirty                          = 0x10000,
	kTaskWqExceededConstrainedThreadLimit = 0x20000,
	kTaskWqExceededTotalThreadLimit       = 0x40000,
	kTaskWqFlagsAvailable                 = 0x80000,
	kTaskUUIDInfoFaultedIn                = 0x100000, /* successfully faulted in some UUID info */
	kTaskUUIDInfoMissing                  = 0x200000, /* some UUID info was paged out */
	kTaskUUIDInfoTriedFault               = 0x400000, /* tried to fault in UUID info */
	kTaskSharedRegionInfoUnavailable      = 0x800000,  /* shared region info unavailable */
	kTaskTALEngaged                       = 0x1000000,
	/* 0x2000000 unused */
	kTaskIsDirtyTracked                   = 0x4000000,
	kTaskAllowIdleExit                    = 0x8000000,
};

enum thread_snapshot_flags {
	/* k{User,Kernel}64_p (values 0x1 and 0x2) are defined in generic_snapshot_flags */
	kHasDispatchSerial    = 0x4,
	kStacksPCOnly         = 0x8,    /* Stack traces have no frame pointers. */
	kThreadDarwinBG       = 0x10,   /* Thread is darwinbg */
	kThreadIOPassive      = 0x20,   /* Thread uses passive IO */
	kThreadSuspended      = 0x40,   /* Thread is suspended */
	kThreadTruncatedBT    = 0x80,   /* Unmapped pages caused truncated backtrace */
	kGlobalForcedIdle     = 0x100,  /* Thread performs global forced idle */
	kThreadFaultedBT      = 0x200,  /* Some thread stack pages were faulted in as part of BT */
	kThreadTriedFaultBT   = 0x400,  /* We tried to fault in thread stack pages as part of BT */
	kThreadOnCore         = 0x800,  /* Thread was on-core when we entered debugger context */
	kThreadIdleWorker     = 0x1000, /* Thread is an idle libpthread worker thread */
	kThreadMain           = 0x2000, /* Thread is the main thread */
};

struct mem_and_io_snapshot {
	uint32_t        snapshot_magic;
	uint32_t        free_pages;
	uint32_t        active_pages;
	uint32_t        inactive_pages;
	uint32_t        purgeable_pages;
	uint32_t        wired_pages;
	uint32_t        speculative_pages;
	uint32_t        throttled_pages;
	uint32_t        filebacked_pages;
	uint32_t        compressions;
	uint32_t        decompressions;
	uint32_t        compressor_size;
	int32_t         busy_buffer_count;
	uint32_t        pages_wanted;
	uint32_t        pages_reclaimed;
	uint8_t         pages_wanted_reclaimed_valid; // did mach_vm_pressure_monitor succeed?
} __attribute__((packed));

/* SS_TH_* macros are for ths_state */
#define SS_TH_WAIT 0x01       /* queued for waiting */
#define SS_TH_SUSP 0x02       /* stopped or requested to stop */
#define SS_TH_RUN 0x04        /* running or on runq */
#define SS_TH_UNINT 0x08      /* waiting uninteruptibly */
#define SS_TH_TERMINATE 0x10  /* halted at termination */
#define SS_TH_TERMINATE2 0x20 /* added to termination queue */
#define SS_TH_IDLE 0x80       /* idling processor */

struct thread_snapshot_v2 {
	uint64_t  ths_thread_id;
	uint64_t  ths_wait_event;
	uint64_t  ths_continuation;
	uint64_t  ths_total_syscalls;
	uint64_t  ths_voucher_identifier;
	uint64_t  ths_dqserialnum;
	uint64_t  ths_user_time;
	uint64_t  ths_sys_time;
	uint64_t  ths_ss_flags;
	uint64_t  ths_last_run_time;
	uint64_t  ths_last_made_runnable_time;
	uint32_t  ths_state;
	uint32_t  ths_sched_flags;
	int16_t   ths_base_priority;
	int16_t   ths_sched_priority;
	uint8_t   ths_eqos;
	uint8_t ths_rqos;
	uint8_t ths_rqos_override;
	uint8_t ths_io_tier;
} __attribute__((packed));

struct thread_snapshot_v3 {
	uint64_t ths_thread_id;
	uint64_t ths_wait_event;
	uint64_t ths_continuation;
	uint64_t ths_total_syscalls;
	uint64_t ths_voucher_identifier;
	uint64_t ths_dqserialnum;
	uint64_t ths_user_time;
	uint64_t ths_sys_time;
	uint64_t ths_ss_flags;
	uint64_t ths_last_run_time;
	uint64_t ths_last_made_runnable_time;
	uint32_t ths_state;
	uint32_t ths_sched_flags;
	int16_t ths_base_priority;
	int16_t ths_sched_priority;
	uint8_t ths_eqos;
	uint8_t ths_rqos;
	uint8_t ths_rqos_override;
	uint8_t ths_io_tier;
	uint64_t ths_thread_t;
} __attribute__((packed));


struct thread_snapshot_v4 {
	uint64_t ths_thread_id;
	uint64_t ths_wait_event;
	uint64_t ths_continuation;
	uint64_t ths_total_syscalls;
	uint64_t ths_voucher_identifier;
	uint64_t ths_dqserialnum;
	uint64_t ths_user_time;
	uint64_t ths_sys_time;
	uint64_t ths_ss_flags;
	uint64_t ths_last_run_time;
	uint64_t ths_last_made_runnable_time;
	uint32_t ths_state;
	uint32_t ths_sched_flags;
	int16_t ths_base_priority;
	int16_t ths_sched_priority;
	uint8_t ths_eqos;
	uint8_t ths_rqos;
	uint8_t ths_rqos_override;
	uint8_t ths_io_tier;
	uint64_t ths_thread_t;
	uint64_t ths_requested_policy;
	uint64_t ths_effective_policy;
} __attribute__((packed));


struct thread_group_snapshot {
	uint64_t tgs_id;
	char tgs_name[16];
} __attribute__((packed));

enum thread_group_flags {
	kThreadGroupEfficient = 0x1,
	kThreadGroupUIApp = 0x2
};

struct thread_group_snapshot_v2 {
	uint64_t tgs_id;
	char tgs_name[16];
	uint64_t tgs_flags;
} __attribute__((packed));

enum coalition_flags {
	kCoalitionTermRequested = 0x1,
	kCoalitionTerminated    = 0x2,
	kCoalitionReaped        = 0x4,
	kCoalitionPrivileged    = 0x8,
};

struct jetsam_coalition_snapshot {
	uint64_t jcs_id;
	uint64_t jcs_flags;
	uint64_t jcs_thread_group;
	uint64_t jcs_leader_task_uniqueid;
} __attribute__((packed));

struct instrs_cycles_snapshot {
	uint64_t ics_instructions;
	uint64_t ics_cycles;
} __attribute__((packed));

struct thread_delta_snapshot_v2 {
	uint64_t  tds_thread_id;
	uint64_t  tds_voucher_identifier;
	uint64_t  tds_ss_flags;
	uint64_t  tds_last_made_runnable_time;
	uint32_t  tds_state;
	uint32_t  tds_sched_flags;
	int16_t   tds_base_priority;
	int16_t   tds_sched_priority;
	uint8_t   tds_eqos;
	uint8_t   tds_rqos;
	uint8_t   tds_rqos_override;
	uint8_t   tds_io_tier;
} __attribute__ ((packed));

struct thread_delta_snapshot_v3 {
	uint64_t  tds_thread_id;
	uint64_t  tds_voucher_identifier;
	uint64_t  tds_ss_flags;
	uint64_t  tds_last_made_runnable_time;
	uint32_t  tds_state;
	uint32_t  tds_sched_flags;
	int16_t   tds_base_priority;
	int16_t   tds_sched_priority;
	uint8_t   tds_eqos;
	uint8_t   tds_rqos;
	uint8_t   tds_rqos_override;
	uint8_t   tds_io_tier;
	uint64_t  tds_requested_policy;
	uint64_t  tds_effective_policy;
} __attribute__ ((packed));

struct io_stats_snapshot {
	/*
	 * I/O Statistics
	 * XXX: These fields must be together.
	 */
	uint64_t         ss_disk_reads_count;
	uint64_t         ss_disk_reads_size;
	uint64_t         ss_disk_writes_count;
	uint64_t         ss_disk_writes_size;
	uint64_t         ss_io_priority_count[STACKSHOT_IO_NUM_PRIORITIES];
	uint64_t         ss_io_priority_size[STACKSHOT_IO_NUM_PRIORITIES];
	uint64_t         ss_paging_count;
	uint64_t         ss_paging_size;
	uint64_t         ss_non_paging_count;
	uint64_t         ss_non_paging_size;
	uint64_t         ss_data_count;
	uint64_t         ss_data_size;
	uint64_t         ss_metadata_count;
	uint64_t         ss_metadata_size;
	/* XXX: I/O Statistics end */
} __attribute__ ((packed));

struct task_snapshot_v2 {
	uint64_t  ts_unique_pid;
	uint64_t  ts_ss_flags;
	uint64_t  ts_user_time_in_terminated_threads;
	uint64_t  ts_system_time_in_terminated_threads;
	uint64_t  ts_p_start_sec;
	uint64_t  ts_task_size;
	uint64_t  ts_max_resident_size;
	uint32_t  ts_suspend_count;
	uint32_t  ts_faults;
	uint32_t  ts_pageins;
	uint32_t  ts_cow_faults;
	uint32_t  ts_was_throttled;
	uint32_t  ts_did_throttle;
	uint32_t  ts_latency_qos;
	int32_t   ts_pid;
	char      ts_p_comm[32];
} __attribute__ ((packed));

struct task_delta_snapshot_v2 {
	uint64_t  tds_unique_pid;
	uint64_t  tds_ss_flags;
	uint64_t  tds_user_time_in_terminated_threads;
	uint64_t  tds_system_time_in_terminated_threads;
	uint64_t  tds_task_size;
	uint64_t  tds_max_resident_size;
	uint32_t  tds_suspend_count;
	uint32_t  tds_faults;
	uint32_t  tds_pageins;
	uint32_t  tds_cow_faults;
	uint32_t  tds_was_throttled;
	uint32_t  tds_did_throttle;
	uint32_t  tds_latency_qos;
} __attribute__ ((packed));

struct stackshot_cpu_times {
	uint64_t user_usec;
	uint64_t system_usec;
} __attribute__((packed));

struct stackshot_cpu_times_v2 {
	uint64_t user_usec;
	uint64_t system_usec;
	uint64_t runnable_usec;
} __attribute__((packed));

struct stackshot_duration {
	uint64_t stackshot_duration;
	uint64_t stackshot_duration_outer;
} __attribute__((packed));

struct stackshot_fault_stats {
	uint32_t sfs_pages_faulted_in;      /* number of pages faulted in using KDP fault path */
	uint64_t sfs_time_spent_faulting;   /* MATUs spent faulting */
	uint64_t sfs_system_max_fault_time; /* MATUs fault time limit per stackshot */
	uint8_t  sfs_stopped_faulting;      /* we stopped decompressing because we hit the limit */
} __attribute__((packed));

typedef struct stackshot_thread_waitinfo {
	uint64_t owner;         /* The thread that owns the object */
	uint64_t waiter;        /* The thread that's waiting on the object */
	uint64_t context;       /* A context uniquely identifying the object */
	uint8_t wait_type;      /* The type of object that the thread is waiting on */
} __attribute__((packed)) thread_waitinfo_t;

typedef struct stackshot_thread_turnstileinfo {
	uint64_t waiter;        /* The thread that's waiting on the object */
	uint64_t turnstile_context; /* Associated data (either thread id, or workq addr) */
	uint8_t turnstile_priority;
	uint8_t number_of_hops;
#define STACKSHOT_TURNSTILE_STATUS_UNKNOWN      (1 << 0) /* The final inheritor is unknown (bug?) */
#define STACKSHOT_TURNSTILE_STATUS_LOCKED_WAITQ (1 << 1) /* A waitq was found to be locked */
#define STACKSHOT_TURNSTILE_STATUS_WORKQUEUE    (1 << 2) /* The final inheritor is a workqueue */
#define STACKSHOT_TURNSTILE_STATUS_THREAD       (1 << 3) /* The final inheritor is a thread */
	uint64_t turnstile_flags;
} __attribute__((packed)) thread_turnstileinfo_t;

#define STACKSHOT_WAITOWNER_KERNEL         (UINT64_MAX - 1)
#define STACKSHOT_WAITOWNER_PORT_LOCKED    (UINT64_MAX - 2)
#define STACKSHOT_WAITOWNER_PSET_LOCKED    (UINT64_MAX - 3)
#define STACKSHOT_WAITOWNER_INTRANSIT      (UINT64_MAX - 4)
#define STACKSHOT_WAITOWNER_MTXSPIN        (UINT64_MAX - 5)
#define STACKSHOT_WAITOWNER_THREQUESTED    (UINT64_MAX - 6) /* workloop waiting for a new worker thread */
#define STACKSHOT_WAITOWNER_SUSPENDED      (UINT64_MAX - 7) /* workloop is suspended */


struct stack_snapshot_stacktop {
	uint64_t sp;
	uint8_t stack_contents[8];
};


/**************** definitions for crashinfo *********************/

/*
 * NOTE: Please update kcdata/libkdd/kcdtypes.c if you make any changes
 * in TASK_CRASHINFO_* types.
 */

/* FIXME some of these types aren't clean (fixed width,  packed, and defined *here*) */

struct crashinfo_proc_uniqidentifierinfo {
	uint8_t                 p_uuid[16];             /* UUID of the main executable */
	uint64_t                p_uniqueid;             /* 64 bit unique identifier for process */
	uint64_t                p_puniqueid;            /* unique identifier for process's parent */
	uint64_t                p_reserve2;             /* reserved for future use */
	uint64_t                p_reserve3;             /* reserved for future use */
	uint64_t                p_reserve4;             /* reserved for future use */
} __attribute__((packed));

#define TASK_CRASHINFO_BEGIN                KCDATA_BUFFER_BEGIN_CRASHINFO
#define TASK_CRASHINFO_STRING_DESC          KCDATA_TYPE_STRING_DESC
#define TASK_CRASHINFO_UINT32_DESC          KCDATA_TYPE_UINT32_DESC
#define TASK_CRASHINFO_UINT64_DESC          KCDATA_TYPE_UINT64_DESC

#define TASK_CRASHINFO_EXTMODINFO           0x801
#define TASK_CRASHINFO_BSDINFOWITHUNIQID    0x802 /* struct crashinfo_proc_uniqidentifierinfo */
#define TASK_CRASHINFO_TASKDYLD_INFO        0x803
#define TASK_CRASHINFO_UUID                 0x804
#define TASK_CRASHINFO_PID                  0x805
#define TASK_CRASHINFO_PPID                 0x806
#define TASK_CRASHINFO_RUSAGE               0x807  /* struct rusage DEPRECATED do not use.
	                                            *                                                      This struct has longs in it */
#define TASK_CRASHINFO_RUSAGE_INFO          0x808  /* struct rusage_info_v3 from resource.h */
#define TASK_CRASHINFO_PROC_NAME            0x809  /* char * */
#define TASK_CRASHINFO_PROC_STARTTIME       0x80B  /* struct timeval64 */
#define TASK_CRASHINFO_USERSTACK            0x80C  /* uint64_t */
#define TASK_CRASHINFO_ARGSLEN              0x80D
#define TASK_CRASHINFO_EXCEPTION_CODES      0x80E  /* mach_exception_data_t */
#define TASK_CRASHINFO_PROC_PATH            0x80F  /* string of len MAXPATHLEN */
#define TASK_CRASHINFO_PROC_CSFLAGS         0x810  /* uint32_t */
#define TASK_CRASHINFO_PROC_STATUS          0x811  /* char */
#define TASK_CRASHINFO_UID                  0x812  /* uid_t */
#define TASK_CRASHINFO_GID                  0x813  /* gid_t */
#define TASK_CRASHINFO_PROC_ARGC            0x814  /* int */
#define TASK_CRASHINFO_PROC_FLAGS           0x815  /* unsigned int */
#define TASK_CRASHINFO_CPUTYPE              0x816  /* cpu_type_t */
#define TASK_CRASHINFO_WORKQUEUEINFO        0x817  /* struct proc_workqueueinfo */
#define TASK_CRASHINFO_RESPONSIBLE_PID      0x818  /* pid_t */
#define TASK_CRASHINFO_DIRTY_FLAGS          0x819  /* int */
#define TASK_CRASHINFO_CRASHED_THREADID     0x81A  /* uint64_t */
#define TASK_CRASHINFO_COALITION_ID         0x81B  /* uint64_t */
#define TASK_CRASHINFO_UDATA_PTRS           0x81C  /* uint64_t */
#define TASK_CRASHINFO_MEMORY_LIMIT         0x81D  /* uint64_t */

#define TASK_CRASHINFO_LEDGER_INTERNAL                          0x81E /* uint64_t */
#define TASK_CRASHINFO_LEDGER_INTERNAL_COMPRESSED               0x81F /* uint64_t */
#define TASK_CRASHINFO_LEDGER_IOKIT_MAPPED                      0x820 /* uint64_t */
#define TASK_CRASHINFO_LEDGER_ALTERNATE_ACCOUNTING              0x821 /* uint64_t */
#define TASK_CRASHINFO_LEDGER_ALTERNATE_ACCOUNTING_COMPRESSED   0x822 /* uint64_t */
#define TASK_CRASHINFO_LEDGER_PURGEABLE_NONVOLATILE             0x823 /* uint64_t */
#define TASK_CRASHINFO_LEDGER_PURGEABLE_NONVOLATILE_COMPRESSED  0x824 /* uint64_t */
#define TASK_CRASHINFO_LEDGER_PAGE_TABLE                        0x825 /* uint64_t */
#define TASK_CRASHINFO_LEDGER_PHYS_FOOTPRINT                    0x826 /* uint64_t */
#define TASK_CRASHINFO_LEDGER_PHYS_FOOTPRINT_LIFETIME_MAX       0x827 /* uint64_t */
#define TASK_CRASHINFO_LEDGER_NETWORK_NONVOLATILE               0x828 /* uint64_t */
#define TASK_CRASHINFO_LEDGER_NETWORK_NONVOLATILE_COMPRESSED    0x829 /* uint64_t */
#define TASK_CRASHINFO_LEDGER_WIRED_MEM                         0x82A /* uint64_t */
#define TASK_CRASHINFO_PROC_PERSONA_ID                          0x82B /* uid_t */
#define TASK_CRASHINFO_MEMORY_LIMIT_INCREASE                    0x82C /* uint32_t */



#define TASK_CRASHINFO_END                  KCDATA_TYPE_BUFFER_END

/**************** definitions for os reasons *********************/

#define EXIT_REASON_SNAPSHOT            0x1001
#define EXIT_REASON_USER_DESC           0x1002 /* string description of reason */
#define EXIT_REASON_USER_PAYLOAD        0x1003 /* user payload data */
#define EXIT_REASON_CODESIGNING_INFO    0x1004
#define EXIT_REASON_WORKLOOP_ID         0x1005
#define EXIT_REASON_DISPATCH_QUEUE_NO   0x1006

struct exit_reason_snapshot {
	uint32_t ers_namespace;
	uint64_t ers_code;
	/* end of version 1 of exit_reason_snapshot. sizeof v1 was 12 */
	uint64_t ers_flags;
} __attribute__((packed));

#define EXIT_REASON_CODESIG_PATH_MAX    1024

struct codesigning_exit_reason_info {
	uint64_t  ceri_virt_addr;
	uint64_t  ceri_file_offset;
	char      ceri_pathname[EXIT_REASON_CODESIG_PATH_MAX];
	char      ceri_filename[EXIT_REASON_CODESIG_PATH_MAX];
	uint64_t  ceri_codesig_modtime_secs;
	uint64_t  ceri_codesig_modtime_nsecs;
	uint64_t  ceri_page_modtime_secs;
	uint64_t  ceri_page_modtime_nsecs;
	uint8_t   ceri_path_truncated;
	uint8_t   ceri_object_codesigned;
	uint8_t   ceri_page_codesig_validated;
	uint8_t   ceri_page_codesig_tainted;
	uint8_t   ceri_page_codesig_nx;
	uint8_t   ceri_page_wpmapped;
	uint8_t   ceri_page_slid;
	uint8_t   ceri_page_dirty;
	uint32_t  ceri_page_shadow_depth;
} __attribute__((packed));

#define EXIT_REASON_USER_DESC_MAX_LEN   1024
#define EXIT_REASON_PAYLOAD_MAX_LEN     2048
/**************** safe iterators *********************/

typedef struct kcdata_iter {
	kcdata_item_t item;
	void *end;
} kcdata_iter_t;


static inline
kcdata_iter_t
kcdata_iter(void *buffer, unsigned long size)
{
	kcdata_iter_t iter;
	iter.item = (kcdata_item_t) buffer;
	iter.end = (void*) (((uintptr_t)buffer) + size);
	return iter;
}

static inline
kcdata_iter_t kcdata_iter_unsafe(void *buffer) __attribute__((deprecated));

static inline
kcdata_iter_t
kcdata_iter_unsafe(void *buffer)
{
	kcdata_iter_t iter;
	iter.item = (kcdata_item_t) buffer;
	iter.end = (void*) (uintptr_t) ~0;
	return iter;
}

static const kcdata_iter_t kcdata_invalid_iter = { .item = NULL, .end = NULL };

static inline
int
kcdata_iter_valid(kcdata_iter_t iter)
{
	return
	        ((uintptr_t)iter.item + sizeof(struct kcdata_item) <= (uintptr_t)iter.end) &&
	        ((uintptr_t)iter.item + sizeof(struct kcdata_item) + iter.item->size <= (uintptr_t)iter.end);
}


static inline
kcdata_iter_t
kcdata_iter_next(kcdata_iter_t iter)
{
	iter.item = (kcdata_item_t) (((uintptr_t)iter.item) + sizeof(struct kcdata_item) + (iter.item->size));
	return iter;
}

static inline uint32_t
kcdata_iter_type(kcdata_iter_t iter)
{
	if ((iter.item->type & ~0xfu) == KCDATA_TYPE_ARRAY_PAD0) {
		return KCDATA_TYPE_ARRAY;
	} else {
		return iter.item->type;
	}
}

static inline uint32_t
kcdata_calc_padding(uint32_t size)
{
	/* calculate number of bytes to add to size to get something divisible by 16 */
	return (-size) & 0xf;
}

static inline uint32_t
kcdata_flags_get_padding(uint64_t flags)
{
	return flags & KCDATA_FLAGS_STRUCT_PADDING_MASK;
}

/* see comment above about has_padding */
static inline int
kcdata_iter_is_legacy_item(kcdata_iter_t iter, uint32_t legacy_size)
{
	uint32_t legacy_size_padded = legacy_size + kcdata_calc_padding(legacy_size);
	return iter.item->size == legacy_size_padded &&
	       (iter.item->flags & (KCDATA_FLAGS_STRUCT_PADDING_MASK | KCDATA_FLAGS_STRUCT_HAS_PADDING)) == 0;
}

static inline uint32_t
kcdata_iter_size(kcdata_iter_t iter)
{
	uint32_t legacy_size = 0;

	switch (kcdata_iter_type(iter)) {
	case KCDATA_TYPE_ARRAY:
	case KCDATA_TYPE_CONTAINER_BEGIN:
		return iter.item->size;
	case STACKSHOT_KCTYPE_THREAD_SNAPSHOT: {
		legacy_size = sizeof(struct thread_snapshot_v2);
		if (kcdata_iter_is_legacy_item(iter, legacy_size)) {
			return legacy_size;
		}

		goto not_legacy;
	}
	case STACKSHOT_KCTYPE_SHAREDCACHE_LOADINFO: {
		legacy_size = sizeof(struct dyld_uuid_info_64);
		if (kcdata_iter_is_legacy_item(iter, legacy_size)) {
			return legacy_size;
		}

		goto not_legacy;
	}
not_legacy:
	default:
		if (iter.item->size < kcdata_flags_get_padding(iter.item->flags)) {
			return 0;
		} else {
			return iter.item->size - kcdata_flags_get_padding(iter.item->flags);
		}
	}
}

static inline uint64_t
kcdata_iter_flags(kcdata_iter_t iter)
{
	return iter.item->flags;
}

static inline
void *
kcdata_iter_payload(kcdata_iter_t iter)
{
	return &iter.item->data;
}


static inline
uint32_t
kcdata_iter_array_elem_type(kcdata_iter_t iter)
{
	return (iter.item->flags >> 32) & UINT32_MAX;
}

static inline
uint32_t
kcdata_iter_array_elem_count(kcdata_iter_t iter)
{
	return (iter.item->flags) & UINT32_MAX;
}

/* KCDATA_TYPE_ARRAY is ambiguous about the size of the array elements.  Size is
 * calculated as total_size / elements_count, but total size got padded out to a
 * 16 byte alignment.  New kernels will generate KCDATA_TYPE_ARRAY_PAD* instead
 * to explicitly tell us how much padding was used.  Here we have a fixed, never
 * to be altered list of the sizes of array elements that were used before I
 * discovered this issue.  If you find a KCDATA_TYPE_ARRAY that is not one of
 * these types, treat it as invalid data. */

static inline
uint32_t
kcdata_iter_array_size_switch(kcdata_iter_t iter)
{
	switch (kcdata_iter_array_elem_type(iter)) {
	case KCDATA_TYPE_LIBRARY_LOADINFO:
		return sizeof(struct dyld_uuid_info_32);
	case KCDATA_TYPE_LIBRARY_LOADINFO64:
		return sizeof(struct dyld_uuid_info_64);
	case STACKSHOT_KCTYPE_KERN_STACKFRAME:
	case STACKSHOT_KCTYPE_USER_STACKFRAME:
		return sizeof(struct stack_snapshot_frame32);
	case STACKSHOT_KCTYPE_KERN_STACKFRAME64:
	case STACKSHOT_KCTYPE_USER_STACKFRAME64:
		return sizeof(struct stack_snapshot_frame64);
	case STACKSHOT_KCTYPE_DONATING_PIDS:
		return sizeof(int32_t);
	case STACKSHOT_KCTYPE_THREAD_DELTA_SNAPSHOT:
		return sizeof(struct thread_delta_snapshot_v2);
	// This one is only here to make some unit tests work. It should be OK to
	// remove.
	case TASK_CRASHINFO_CRASHED_THREADID:
		return sizeof(uint64_t);
	default:
		return 0;
	}
}

static inline
int
kcdata_iter_array_valid(kcdata_iter_t iter)
{
	if (!kcdata_iter_valid(iter)) {
		return 0;
	}
	if (kcdata_iter_type(iter) != KCDATA_TYPE_ARRAY) {
		return 0;
	}
	if (kcdata_iter_array_elem_count(iter) == 0) {
		return iter.item->size == 0;
	}
	if (iter.item->type == KCDATA_TYPE_ARRAY) {
		uint32_t elem_size = kcdata_iter_array_size_switch(iter);
		if (elem_size == 0) {
			return 0;
		}
		/* sizes get aligned to the nearest 16. */
		return
		        kcdata_iter_array_elem_count(iter) <= iter.item->size / elem_size &&
		        iter.item->size % kcdata_iter_array_elem_count(iter) < 16;
	} else {
		return
		        (iter.item->type & 0xf) <= iter.item->size &&
		        kcdata_iter_array_elem_count(iter) <= iter.item->size - (iter.item->type & 0xf) &&
		        (iter.item->size - (iter.item->type & 0xf)) % kcdata_iter_array_elem_count(iter) == 0;
	}
}


static inline
uint32_t
kcdata_iter_array_elem_size(kcdata_iter_t iter)
{
	if (iter.item->type == KCDATA_TYPE_ARRAY) {
		return kcdata_iter_array_size_switch(iter);
	}
	if (kcdata_iter_array_elem_count(iter) == 0) {
		return 0;
	}
	return (iter.item->size - (iter.item->type & 0xf)) / kcdata_iter_array_elem_count(iter);
}

static inline
int
kcdata_iter_container_valid(kcdata_iter_t iter)
{
	return
	        kcdata_iter_valid(iter) &&
	        kcdata_iter_type(iter) == KCDATA_TYPE_CONTAINER_BEGIN &&
	        iter.item->size >= sizeof(uint32_t);
}

static inline
uint32_t
kcdata_iter_container_type(kcdata_iter_t iter)
{
	return *(uint32_t *) kcdata_iter_payload(iter);
}

static inline
uint64_t
kcdata_iter_container_id(kcdata_iter_t iter)
{
	return iter.item->flags;
}


#define KCDATA_ITER_FOREACH(iter) for(; kcdata_iter_valid(iter) && iter.item->type != KCDATA_TYPE_BUFFER_END; iter = kcdata_iter_next(iter))
#define KCDATA_ITER_FOREACH_FAILED(iter) (!kcdata_iter_valid(iter) || (iter).item->type != KCDATA_TYPE_BUFFER_END)

static inline
kcdata_iter_t
kcdata_iter_find_type(kcdata_iter_t iter, uint32_t type)
{
	KCDATA_ITER_FOREACH(iter)
	{
		if (kcdata_iter_type(iter) == type) {
			return iter;
		}
	}
	return kcdata_invalid_iter;
}

static inline
int
kcdata_iter_data_with_desc_valid(kcdata_iter_t iter, uint32_t minsize)
{
	return
	        kcdata_iter_valid(iter) &&
	        kcdata_iter_size(iter) >= KCDATA_DESC_MAXLEN + minsize &&
	        ((char*)kcdata_iter_payload(iter))[KCDATA_DESC_MAXLEN - 1] == 0;
}

static inline
char *
kcdata_iter_string(kcdata_iter_t iter, uint32_t offset)
{
	if (offset > kcdata_iter_size(iter)) {
		return NULL;
	}
	uint32_t maxlen = kcdata_iter_size(iter) - offset;
	char *s = ((char*)kcdata_iter_payload(iter)) + offset;
	if (strnlen(s, maxlen) < maxlen) {
		return s;
	} else {
		return NULL;
	}
}

static inline void
kcdata_iter_get_data_with_desc(kcdata_iter_t iter, char **desc_ptr, void **data_ptr, uint32_t *size_ptr)
{
	if (desc_ptr) {
		*desc_ptr = (char *)kcdata_iter_payload(iter);
	}
	if (data_ptr) {
		*data_ptr = (void *)((uintptr_t)kcdata_iter_payload(iter) + KCDATA_DESC_MAXLEN);
	}
	if (size_ptr) {
		*size_ptr = kcdata_iter_size(iter) - KCDATA_DESC_MAXLEN;
	}
}

#endif
