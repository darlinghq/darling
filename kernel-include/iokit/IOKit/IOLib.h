/*
 * Copyright (c) 1998-2016 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1998 Apple Computer, Inc.  All rights reserved.
 *
 * HISTORY
 *
 */

#ifndef __IOKIT_IOLIB_H
#define __IOKIT_IOLIB_H

#ifndef KERNEL
#error IOLib.h is for kernel use only
#endif

#include <stdarg.h>
#include <sys/cdefs.h>
#include <os/overflow.h>

#include <sys/appleapiopts.h>

#include <IOKit/system.h>

#include <IOKit/IOReturn.h>
#include <IOKit/IOTypes.h>
#include <IOKit/IOLocks.h>

#include <libkern/OSAtomic.h>

__BEGIN_DECLS

#include <kern/thread_call.h>
#include <kern/clock.h>

/*
 * min/max macros.
 */

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

/*
 * These are opaque to the user.
 */
typedef thread_t IOThread;
typedef void (*IOThreadFunc)(void *argument);

/*
 * Memory allocation functions.
 */

/*! @function IOMalloc
 *   @abstract Allocates general purpose, wired memory in the kernel map.
 *   @discussion This is a general purpose utility to allocate memory in the kernel. There are no alignment guarantees given on the returned memory, and alignment may vary depending on the kernel configuration. This function may block and so should not be called from interrupt level or while a simple lock is held.
 *   @param size Size of the memory requested.
 *   @result Pointer to the allocated memory, or zero on failure. */

void * IOMalloc(vm_size_t size)      __attribute__((alloc_size(1)));
void * IOMallocZero(vm_size_t size)  __attribute__((alloc_size(1)));

/*! @function IOFree
 *   @abstract Frees memory allocated with IOMalloc.
 *   @discussion This function frees memory allocated with IOMalloc, it may block and so should not be called from interrupt level or while a simple lock is held.
 *   @param address Pointer to the allocated memory. Must be identical to result
 *   @of a prior IOMalloc.
 *   @param size Size of the memory allocated. Must be identical to size of
 *   @the corresponding IOMalloc */

void   IOFree(void * address, vm_size_t size);

/*! @function IOMallocAligned
 *   @abstract Allocates wired memory in the kernel map, with an alignment restriction.
 *   @discussion This is a utility to allocate memory in the kernel, with an alignment restriction which is specified as a byte count. This function may block and so should not be called from interrupt level or while a simple lock is held.
 *   @param size Size of the memory requested.
 *   @param alignment Byte count of the alignment for the memory. For example, pass 256 to get memory allocated at an address with bit 0-7 zero.
 *   @result Pointer to the allocated memory, or zero on failure. */

void * IOMallocAligned(vm_size_t size, vm_offset_t alignment) __attribute__((alloc_size(1)));

/*! @function IOFreeAligned
 *   @abstract Frees memory allocated with IOMallocAligned.
 *   @discussion This function frees memory allocated with IOMallocAligned, it may block and so should not be called from interrupt level or while a simple lock is held.
 *   @param address Pointer to the allocated memory.
 *   @param size Size of the memory allocated. */

void   IOFreeAligned(void * address, vm_size_t size);

/*! @function IOMallocContiguous
 *   @abstract Deprecated - use IOBufferMemoryDescriptor. Allocates wired memory in the kernel map, with an alignment restriction and physically contiguous.
 *   @discussion This is a utility to allocate memory in the kernel, with an alignment restriction which is specified as a byte count, and will allocate only physically contiguous memory. The request may fail if memory is fragmented, and may cause large amounts of paging activity. This function may block and so should not be called from interrupt level or while a simple lock is held.
 *   @param size Size of the memory requested.
 *   @param alignment Byte count of the alignment for the memory. For example, pass 256 to get memory allocated at an address with bits 0-7 zero.
 *   @param physicalAddress IOMallocContiguous returns the physical address of the allocated memory here, if physicalAddress is a non-zero pointer. The physicalAddress argument is deprecated and should be passed as NULL. To obtain the physical address for a memory buffer, use the IODMACommand class in conjunction with the IOMemoryDescriptor or IOBufferMemoryDescriptor classes.
 *   @result Virtual address of the allocated memory, or zero on failure. */

void * IOMallocContiguous(vm_size_t size, vm_size_t alignment,
    IOPhysicalAddress * physicalAddress) __attribute__((deprecated)) __attribute__((alloc_size(1)));

/*! @function IOFreeContiguous
 *   @abstract Deprecated - use IOBufferMemoryDescriptor. Frees memory allocated with IOMallocContiguous.
 *   @discussion This function frees memory allocated with IOMallocContiguous, it may block and so should not be called from interrupt level or while a simple lock is held.
 *   @param address Virtual address of the allocated memory.
 *   @param size Size of the memory allocated. */

void   IOFreeContiguous(void * address, vm_size_t size) __attribute__((deprecated));


/*! @function IOMallocPageable
 *   @abstract Allocates pageable memory in the kernel map.
 *   @discussion This is a utility to allocate pageable memory in the kernel. This function may block and so should not be called from interrupt level or while a simple lock is held.
 *   @param size Size of the memory requested.
 *   @param alignment Byte count of the alignment for the memory. For example, pass 256 to get memory allocated at an address with bits 0-7 zero.
 *   @result Pointer to the allocated memory, or zero on failure. */

void * IOMallocPageable(vm_size_t size, vm_size_t alignment) __attribute__((alloc_size(1)));

/*! @function IOFreePageable
 *   @abstract Frees memory allocated with IOMallocPageable.
 *   @discussion This function frees memory allocated with IOMallocPageable, it may block and so should not be called from interrupt level or while a simple lock is held.
 *   @param address Virtual address of the allocated memory.
 *   @param size Size of the memory allocated. */

void IOFreePageable(void * address, vm_size_t size);

/*
 * Typed memory allocation macros. All may block.
 */

#define IONew(type, count)                              \
({                                                      \
    size_t __size;                                      \
    (os_mul_overflow(sizeof(type), (count), &__size)    \
    ? ((type *) NULL)                                   \
    : ((type *) IOMalloc(__size)));                     \
})

#define IONewZero(type, count)                          \
({                                                      \
    size_t __size;                                      \
    (os_mul_overflow(sizeof(type), (count), &__size)    \
    ? ((type *) NULL)                                   \
    : ((type *) IOMallocZero(__size)));                 \
})

#define IODelete(ptr, type, count)                          \
({                                                          \
    size_t __size;                                          \
    if (!os_mul_overflow(sizeof(type), (count), &__size)) { \
	IOFree(ptr, __size);                                \
    }                                                       \
})

#define IOSafeDeleteNULL(ptr, type, count)              \
    do {                                                \
	if (NULL != (ptr)) {                            \
	    IODelete((ptr), type, count);               \
	    (ptr) = NULL;                               \
	}                                               \
    } while (0)                                         \

/////////////////////////////////////////////////////////////////////////////
//
//
//	These functions are now implemented in IOMapper.cpp
//
//
/////////////////////////////////////////////////////////////////////////////

/*! @function IOMappedRead8
 *   @abstract Read one byte from the desired "Physical" IOSpace address.
 *   @discussion Read one byte from the desired "Physical" IOSpace address.  This function allows the developer to read an address returned from any memory descriptor's getPhysicalSegment routine.  It can then be used by segmenting a physical page slightly to tag the physical page with its kernel space virtual address.
 *   @param address The desired address, as returned by IOMemoryDescriptor::getPhysicalSegment.
 *   @result Data contained at that location */

UInt8 IOMappedRead8(IOPhysicalAddress address);

/*! @function IOMappedRead16
 *   @abstract Read two bytes from the desired "Physical" IOSpace address.
 *   @discussion Read two bytes from the desired "Physical" IOSpace address.  This function allows the developer to read an address returned from any memory descriptor's getPhysicalSegment routine.  It can then be used by segmenting a physical page slightly to tag the physical page with its kernel space virtual address.
 *   @param address The desired address, as returned by IOMemoryDescriptor::getPhysicalSegment.
 *   @result Data contained at that location */

UInt16 IOMappedRead16(IOPhysicalAddress address);

/*! @function IOMappedRead32
 *   @abstract Read four bytes from the desired "Physical" IOSpace address.
 *   @discussion Read four bytes from the desired "Physical" IOSpace address.  This function allows the developer to read an address returned from any memory descriptor's getPhysicalSegment routine.  It can then be used by segmenting a physical page slightly to tag the physical page with its kernel space virtual address.
 *   @param address The desired address, as returned by IOMemoryDescriptor::getPhysicalSegment.
 *   @result Data contained at that location */

UInt32 IOMappedRead32(IOPhysicalAddress address);

/*! @function IOMappedRead64
 *   @abstract Read eight bytes from the desired "Physical" IOSpace address.
 *   @discussion Read eight bytes from the desired "Physical" IOSpace address.  This function allows the developer to read an address returned from any memory descriptor's getPhysicalSegment routine.  It can then be used by segmenting a physical page slightly to tag the physical page with its kernel space virtual address.
 *   @param address The desired address, as returned by IOMemoryDescriptor::getPhysicalSegment.
 *   @result Data contained at that location */

UInt64 IOMappedRead64(IOPhysicalAddress address);

/*! @function IOMappedWrite8
 *   @abstract Write one byte to the desired "Physical" IOSpace address.
 *   @discussion Write one byte to the desired "Physical" IOSpace address.  This function allows the developer to write to an address returned from any memory descriptor's getPhysicalSegment routine.
 *   @param address The desired address, as returned by IOMemoryDescriptor::getPhysicalSegment.
 *   @param value Data to be writen to the desired location */

void IOMappedWrite8(IOPhysicalAddress address, UInt8 value);

/*! @function IOMappedWrite16
 *   @abstract Write two bytes to the desired "Physical" IOSpace address.
 *   @discussion Write two bytes to the desired "Physical" IOSpace address.  This function allows the developer to write to an address returned from any memory descriptor's getPhysicalSegment routine.
 *   @param address The desired address, as returned by IOMemoryDescriptor::getPhysicalSegment.
 *   @param value Data to be writen to the desired location */

void IOMappedWrite16(IOPhysicalAddress address, UInt16 value);

/*! @function IOMappedWrite32
 *   @abstract Write four bytes to the desired "Physical" IOSpace address.
 *   @discussion Write four bytes to the desired "Physical" IOSpace address.  This function allows the developer to write to an address returned from any memory descriptor's getPhysicalSegment routine.
 *   @param address The desired address, as returned by IOMemoryDescriptor::getPhysicalSegment.
 *   @param value Data to be writen to the desired location */

void IOMappedWrite32(IOPhysicalAddress address, UInt32 value);

/*! @function IOMappedWrite64
 *   @abstract Write eight bytes to the desired "Physical" IOSpace address.
 *   @discussion Write eight bytes to the desired "Physical" IOSpace address.  This function allows the developer to write to an address returned from any memory descriptor's getPhysicalSegment routine.
 *   @param address The desired address, as returned by IOMemoryDescriptor::getPhysicalSegment.
 *   @param value Data to be writen to the desired location */

void IOMappedWrite64(IOPhysicalAddress address, UInt64 value);

/* This function is deprecated. Cache settings may be set for allocated memory with the IOBufferMemoryDescriptor api. */

IOReturn IOSetProcessorCacheMode( task_t task, IOVirtualAddress address,
    IOByteCount length, IOOptionBits cacheMode ) __attribute__((deprecated));

/*! @function IOFlushProcessorCache
 *   @abstract Flushes the processor cache for mapped memory.
 *   @discussion This function flushes the processor cache of an already mapped memory range. Note in most cases it is preferable to use IOMemoryDescriptor::prepare and complete to manage cache coherency since they are aware of the architecture's requirements. Flushing the processor cache is not required for coherency in most situations.
 *   @param task Task the memory is mapped into.
 *   @param address Virtual address of the memory.
 *   @param length Length of the range to set.
 *   @result An IOReturn code. */

IOReturn IOFlushProcessorCache( task_t task, IOVirtualAddress address,
    IOByteCount length );

/*! @function IOThreadSelf
 *   @abstract Returns the osfmk identifier for the currently running thread.
 *   @discussion This function returns the current thread (a pointer to the currently active osfmk thread_shuttle). */

#define IOThreadSelf() (current_thread())

/*! @function IOCreateThread
 *   @abstract Deprecated function - use kernel_thread_start(). Create a kernel thread.
 *   @discussion This function creates a kernel thread, and passes the caller supplied argument to the new thread.  Warning: the value returned by this function is not 100% reliable.  There is a race condition where it is possible that the new thread has already terminated before this call returns.  Under that circumstance the IOThread returned will be invalid.  In general there is little that can be done with this value except compare it against 0.  The thread itself can call IOThreadSelf() 100% reliably and that is the prefered mechanism to manipulate the IOThreads state.
 *   @param function A C-function pointer where the thread will begin execution.
 *   @param argument Caller specified data to be passed to the new thread.
 *   @result An IOThread identifier for the new thread, equivalent to an osfmk thread_t. */

IOThread IOCreateThread(IOThreadFunc function, void *argument) __attribute__((deprecated));

/*! @function IOExitThread
 *   @abstract Deprecated function - use thread_terminate(). Terminate execution of current thread.
 *   @discussion This function destroys the currently running thread, and does not return. */

void IOExitThread(void) __attribute__((deprecated));

/*! @function IOSleep
 *   @abstract Sleep the calling thread for a number of milliseconds.
 *   @discussion This function blocks the calling thread for at least the number of specified milliseconds, giving time to other processes.
 *   @param milliseconds The integer number of milliseconds to wait. */

void IOSleep(unsigned milliseconds);

/*! @function IOSleepWithLeeway
 *   @abstract Sleep the calling thread for a number of milliseconds, with a specified leeway the kernel may use for timer coalescing.
 *   @discussion This function blocks the calling thread for at least the number of specified milliseconds, giving time to other processes.  The kernel may also coalesce any timers involved in the delay, using the leeway given as a guideline.
 *   @param intervalMilliseconds The integer number of milliseconds to wait.
 *   @param leewayMilliseconds The integer number of milliseconds to use as a timer coalescing guideline. */

void IOSleepWithLeeway(unsigned intervalMilliseconds, unsigned leewayMilliseconds);

/*! @function IODelay
 *   @abstract Spin delay for a number of microseconds.
 *   @discussion This function spins to delay for at least the number of specified microseconds. Since the CPU is busy spinning no time is made available to other processes; this method of delay should be used only for short periods. Also, the AbsoluteTime based APIs of kern/clock.h provide finer grained and lower cost delays.
 *   @param microseconds The integer number of microseconds to spin wait. */

void IODelay(unsigned microseconds);

/*! @function IOPause
 *   @abstract Spin delay for a number of nanoseconds.
 *   @discussion This function spins to delay for at least the number of specified nanoseconds. Since the CPU is busy spinning no time is made available to other processes; this method of delay should be used only for short periods.
 *   @param nanoseconds The integer number of nanoseconds to spin wait. */

void IOPause(unsigned nanoseconds);

/*! @function IOLog
 *   @abstract Log a message to console in text mode, and /var/log/system.log.
 *   @discussion This function allows a driver to log diagnostic information to the screen during verbose boots, and to a log file found at /var/log/system.log. IOLog should not be called from interrupt context.
 *   @param format A printf() style format string (see printf(3) documentation).
 */

void IOLog(const char *format, ...)
__attribute__((format(printf, 1, 2)));

/*! @function IOLogv
 *   @abstract Log a message to console in text mode, and /var/log/system.log.
 *   @discussion This function allows a driver to log diagnostic information to the screen during verbose boots, and to a log file found at /var/log/system.log. IOLogv should not be called from interrupt context.
 *   @param format A printf() style format string (see printf(3) documentation).
 *   @param ap stdarg(3) style variable arguments. */

void IOLogv(const char *format, va_list ap)
__attribute__((format(printf, 1, 0)));

#ifndef _FN_KPRINTF
#define _FN_KPRINTF
void kprintf(const char *format, ...);
#endif
#ifndef _FN_KPRINTF_DECLARED
#define _FN_KPRINTF_DECLARED
#endif

/*
 * Convert a integer constant (typically a #define or enum) to a string
 * via an array of IONamedValue.
 */
const char *IOFindNameForValue(int value,
    const IONamedValue *namedValueArray);

/*
 * Convert a string to an int via an array of IONamedValue. Returns
 * kIOReturnSuccess of string found, else returns kIOReturnBadArgument.
 */
IOReturn IOFindValueForName(const char *string,
    const IONamedValue *regValueArray,
    int *value);                                /* RETURNED */

/*! @function Debugger
 *   @abstract Enter the kernel debugger.
 *   @discussion This function freezes the kernel and enters the builtin debugger. It may not be possible to exit the debugger without a second machine.
 *   @param reason A C-string to describe why the debugger is being entered. */

void Debugger(const char * reason);
#if __LP64__
#define IOPanic(reason) panic("%s", reason)
#else
void IOPanic(const char *reason) __attribute__((deprecated)) __abortlike;
#endif

#ifdef __cplusplus
class OSDictionary;
#endif

#ifdef __cplusplus
OSDictionary *
#else
struct OSDictionary *
#endif
IOBSDNameMatching( const char * name );

#ifdef __cplusplus
OSDictionary *
#else
struct OSDictionary *
#endif
IOOFPathMatching( const char * path, char * buf, int maxLen ) __attribute__((deprecated));

/*
 * Convert between size and a power-of-two alignment.
 */
IOAlignment IOSizeToAlignment(unsigned int size);
unsigned int IOAlignmentToSize(IOAlignment align);

/*
 * Multiply and divide routines for IOFixed datatype.
 */

static inline IOFixed
IOFixedMultiply(IOFixed a, IOFixed b)
{
	return (IOFixed)((((SInt64) a) * ((SInt64) b)) >> 16);
}

static inline IOFixed
IOFixedDivide(IOFixed a, IOFixed b)
{
	return (IOFixed)((((SInt64) a) << 16) / ((SInt64) b));
}

/*
 * IORound and IOTrunc convenience functions, in the spirit
 * of vm's round_page() and trunc_page().
 */
#define IORound(value, multiple) \
	((((value) + (multiple) - 1) / (multiple)) * (multiple))

#define IOTrunc(value, multiple) \
	(((value) / (multiple)) * (multiple));


#if defined(__APPLE_API_OBSOLETE)

/* The following API is deprecated */

/* The API exported by kern/clock.h
 *  should be used for high resolution timing. */

void IOGetTime( mach_timespec_t * clock_time) __attribute__((deprecated));

#if !defined(__LP64__)

#undef eieio
#define eieio() \
    OSSynchronizeIO()

extern mach_timespec_t IOZeroTvalspec;

#endif /* !defined(__LP64__) */

#endif /* __APPLE_API_OBSOLETE */

#if XNU_KERNEL_PRIVATE
vm_tag_t
IOMemoryTag(vm_map_t map);
#endif

__END_DECLS

#endif /* !__IOKIT_IOLIB_H */
