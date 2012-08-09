/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
 * @OSF_COPYRIGHT@
 *
 */
/*
 * HISTORY
 * 
 * Revision 1.1.1.1  1998/09/22 21:05:29  wsanchez
 * Import of Mac OS X kernel (~semeria)
 *
 * Revision 1.1.1.1  1998/03/07 02:25:45  wsanchez
 * Import of OSF Mach kernel (~mburg)
 *
 * Revision 1.1.11.1  1996/09/17  16:34:42  bruel
 * 	fixed types.
 * 	[96/09/17            bruel]
 *
 * Revision 1.1.6.1  1995/06/13  18:20:10  sjs
 * 	Merged from flipc_shared.
 * 	[95/06/07            sjs]
 * 
 * Revision 1.1.3.14  1995/05/19  00:58:14  sjs
 * 	Added send_ready to shared area, used for fast check if there is something
 * 	to do (and prevents the cache from getting stirred).
 * 	[95/05/18            sjs]
 * 
 * Revision 1.1.3.13  1995/05/16  20:46:28  randys
 * 	Export performance valid information through performance
 * 	structure rather than kernel configuration section.
 * 	[95/05/16            randys]
 * 
 * 	Added performance (FLIPC_PERF) config information to
 * 	kernel_configuration section of comm buffer, so that user
 * 	programs can find out if this information is being gathered.
 * 	[95/05/16            randys]
 * 
 * Revision 1.1.3.12  1995/05/15  14:26:54  randys
 * 	Updated comments on use of acquire pointer (it's completely
 * 	ignored if dpb is set) and added macros for testing !dpb and
 * 	enabled at the same time.
 * 	[95/05/11            randys]
 * 
 * 	Change pme_process_ptr ==> sme_process_ptr (since it's being read
 * 	by AIL now).
 * 	[95/05/11            randys]
 * 
 * 	Added private copied of release and process pointers.
 * 	[95/05/11            randys]
 * 
 * 	Rearrange endpoint structure to separate data with importantly
 * 	different access patterns into different cache lines.  This
 * 	involved duplicating some (effectively constant) entries, and
 * 	creating two versions of some macros.
 * 	[95/05/11            randys]
 * 
 * Revision 1.1.3.11  1995/05/08  16:06:33  randys
 * 	Added comment explaining that an endpoint bufferlist must always
 * 	have valid buffer pointers in all of its entries, to keep
 * 	FLIPC_endpoint_buffer_available from going off the deep end.  No
 * 	code changes.
 * 	[95/04/18            randys]
 * 
 * Revision 1.1.3.10  1995/04/05  21:21:52  randys
 * 	Added a field to the buffer control structure holding the
 * 	scheduling policy chosen for the allocations lock.
 * 	[95/04/05            randys]
 * 
 * Revision 1.1.3.9  1995/03/23  20:35:19  randys
 * 	Added comments indicating duplication of declarations of
 * 	flipc_cb_base & flipc_cb_size in this file and in flipc_usermsg.h
 * 	Modified declaration of flipc_cb_size to be unsigned long.
 * 	[95/03/21            randys]
 * 
 * Revision 1.1.3.8  1995/02/23  21:32:42  randys
 * 	Added space for kernel configuration in communications buffer
 * 	control structure.
 * 	[95/02/22            randys]
 * 
 * Revision 1.1.3.7  1995/02/21  17:22:58  randys
 * 	Re-indented code to four space indentation
 * 	[1995/02/21  16:25:32  randys]
 * 
 * Revision 1.1.3.6  1995/02/13  22:57:29  randys
 * 	Replaced all of NEXT_{ACQUIRE,RELEASE,PROCESS}_PTR macros with a
 * 	 single NEXT_BUFFERLIST_PTR macro.
 * 	[95/02/03            randys]
 * 
 * Revision 1.1.3.5  1995/01/26  21:01:44  randys
 * 	Add performance structure into CB.
 * 	[1995/01/24  21:14:31  randys]
 * 
 * 	Added flag in epgroup structure to note that epgroup
 * 	has a semaphore associated with it.
 * 	[1995/01/19  23:02:13  randys]
 * 
 * 	Add a space in the comm buffer header for the null_destination
 * 	the ME sets up for the AIL.  Get rid of
 * 	FLIPC_ADDRESS_ENDPOINT_PTR (it isn't used)
 * 	[1995/01/19  20:22:30  randys]
 * 
 * 	Up the comm buffer size to 1 megabyte
 * 	[1995/01/17  22:23:27  randys]
 * 
 * Revision 1.1.3.4  1995/01/12  21:19:01  randys
 * 	Minor commenting changes from dlb
 * 	[1995/01/06  18:18:12  randys]
 * 
 * Revision 1.1.3.3  1994/12/22  16:23:57  randys
 * 	Fixed calculation of number of buffers on an endpoint
 * 	to take size of buffer pointers into account.
 * 	[1994/12/21  16:19:55  randys]
 * 
 * Revision 1.1.3.2  1994/12/20  19:01:56  randys
 * 	Moved definition of flipc_simple_lock to flipc_cb.h
 * 	[1994/12/20  17:34:41  randys]
 * 
 * 	Added a simple lock in the comm buffer to use for the
 * 	allocations lock, along with directions as to how
 * 	to use it (not like a normal simple lock).
 * 	[1994/12/20  15:27:25  randys]
 * 
 * 	Added error log into communications buffer control
 * 	structure, and changed FLIPC_ADDRESS_ENDPOINT_PTR to
 * 	correctly compute the endpoint pointer based on the
 * 	new ctl structure layout.
 * 	[1994/12/19  23:47:45  randys]
 * 
 * 	Added filename in comment at top of each file
 * 	[1994/12/19  20:28:20  randys]
 * 
 * 	Add version field to epgroup to check races on buffer acquire
 * 	from epgroup.
 * 	[1994/12/19  18:05:04  randys]
 * 
 * Revision 1.1.3.1  1994/12/12  17:46:12  randys
 * 	Putting initial flipc implementation under flipc_shared
 * 	[1994/12/12  16:27:46  randys]
 * 
 * Revision 1.1.1.2  1994/12/11  23:11:18  randys
 * 	Initial flipc code checkin
 * 
 * $EndLog$
 */

/*
 * mach/flipc_cb.h
 *
 * This file is intended to be the data structure layout for the flipc
 * communcations buffer, both for the KKT implementation and
 * for the eventual paragon implementation.  This file should include
 * all of the information necessary for either humans or machines to
 * understand the data structure layout.
 *
 * The communications buffer is the wired section of memory used for
 * communication between the flipc applications interface layer and
 * the flipc message engine.  No structure in it are visible to the
 * user; the applications interface layer mediates all user access to
 * the CB.
 */

#ifndef _MACH_FLIPC_CB_H_
#define _MACH_FLIPC_CB_H_

#include <mach/flipc_types.h>

/*
 * Flipc naming and argument ordering conventions (this applies mainly to
 * user-interface.h, but seems inappropriate in a user-visible header file):
 *
 * All objects prefixed with "flipc"; uppercase for user-visible
 * objects, lower case for internal ones.
 *
 * Types created with typedef will have _t suffixes.
 *
 * Words will be separated by '_'.
 *
 * Macro definitions will be all in caps.
 *
 * Enum members will have their initial letter (after Flipc) capitalized.
 *
 *
 * For user-visible routines:
 *
 * The first word following the "flipc" will be the flipc object type that
 * that routine operates on (specifically "domain", "epgroup",
 * "endpoint", or "buffer").
 *
 * The object named by the first word of the call will, if an argument
 * to the call, be the first argument.
 *
 * Output variables passed as pointers in the arglist will come last.
 */

/*
 * The kinds of objects that exist in the communications buffer are:
 *
 * Endpoints -- Used for sending or receiving.
 * Buffers -- Composed of a buffer header and buffer data.
 * Endpoint groups -- Used for collecting multiple numbers of endpoints
 * 	together for a select like operation.
 */

/*
 * We can't use general pointers inside the communications buffer,
 * since the address space on either side of the interface is
 * different.  The places where we could use pointers are:
 *
 *	*) From endpoint sets to endpoints.
 *	*) From endpoints to buffers.
 *
 * The kinds of pointers we could use are:
 *	*) Byte offset from the beginning of the comm buffer.  This
 * 	   is simple, but has the disadvantage of allowing the user to
 * 	   play games with pointing endpoint buffer pointers into data
 * 	   space, & etc.
 *	*) Rigid arrays of each type of object, with the object
 * 	   "pointer" being an index into the array.  This avoids the
 * 	   above problem, but complicates memory allocation (forces
 * 	   allocation to be contiguous, which may force pre-deciding
 * 	   how much space each of the above types will take).
 *
 * Though we appear to be going for the rigid allocation for each type
 * of data structure, I'm still going to do the "simple offset"
 * solution to maintain maximum flexibility into the future.
 * The single exception to this is that FLIPC addresses will be composed of
 * node number and endpoint number, where the endpoint number will be
 * the index into the endpoint array.
 */

typedef unsigned long flipc_cb_ptr;
/* Define a null value, which doesn't point anywhere into the CB.  */
#define FLIPC_CBPTR_NULL ((flipc_cb_ptr) -1)

/*
 * Synchronization between message engine and application.
 *
 * In general, it isn't reasonable to allow locking and unlocking of
 * data structures between message engine and communications buffer,
 * as this requires the message engine to trust arbitrary user
 * threads.  The solution is to arrange all data structures so that
 * they may be accessed by both parties without locking.  The way that
 * this is usually done is that specific variables are considered to
 * be owned by one of the ME or the AIL, and the other party is
 * allowed to read the variable but not to modify it.  With this
 * arrangement, implementing things like producer/consumer circular
 * queues is possible; each agent (ME or AIL) goes around the list
 * doing its thing, and avoids passing the pointer showing where the
 * other agent is working.
 *
 * Following the above, we may divide structure members into five
 * classes, and define prefixes for these five classes.
 *
 *	Description		Prefix
 *      -------------------------------
 *	Private to AIL		pail_
 *	Private to ME		pme_
 *	AIL owned, read by ME	sail_
 *	ME owned, read by AIL	sme_
 * 	Shared in other way	shrd_
 *
 * Shared variables may change their ownership based on their own
 * or someone elses value (these variables may be thought of as
 * being handed back and forth between the two entities) or on a
 * configuration option of the structure (not handed back and forth,
 * but still based on another variables value).
 *
 * In addition, I am going to put variables that are set at endpoint
 * allocation and cleared at deallocation (but read by both sides) in
 * a separate class; they are "AIL owned, read by ME" but are
 * effectively constant over the synchronization protocols we care
 * about.
 *
 * 	Constant after allocation	const_
 *
 * Note that this ignores memory consistency issues (when the two
 * agents are actually on two separate processors).  These issues need
 * to be explored in more detail; for now suffice it to say that the
 * above methods work given a sequentially consistent memory model or
 * a processor consistent memory model.
 *
 * Also note that an optimizing compiler may reorder our memory
 * accesses, playing merry hell with the inter-node synchronization
 * protocols (the compiler doesn't know about the other node, after
 * all).  To avoid this, all structure members used for
 * synchronization will be marked volatile; this will force the
 * compiler to keep the order and number of accesses intact.  This
 * will also force the compiler *not* to optimize way accesses to
 * these variables, so it is wise to explicitly load the variable into
 * a temporary once if you need to do multiple computations with it,
 * and store it back afterwards when you are done.
 */

/*
 * Memory allocation:
 *
 * For maximum simplicity in the first implementation, we need to know
 * at comm buffer allocation time how many endpoints, endpoint_sets,
 * and buffers we will want total, until the end of time.  This
 * masively simplifies memory allocation; there will be a single array
 * of each type of data and the communication buffer will be taken up
 * by the concatenation of these arrays (with some fiddling to make
 * sure that no data crosses a page boundary).
 *
 * For each data type there will be a free list to which pieces of
 * data will be added to or removed from as needed.  Each data type
 * will have a pointer in it to allow it to be linked onto the free
 * list.
 */

/*
 * Multiple thread access to data structures:
 *
 * There are several points in the communications buffer (notably
 * endpoint accesses) when multiple application threads will be
 * attempting operations on data structures at the same time.  To
 * multiplex these operations, we need a per-data structure lock.
 * Lock attributes:
 * 	*) This lock will not be kernel based, as such a lock would be
 * 	   too heavyweight to use for arbitrary sending and receiving
 * 	   operations).
 *	*) Because it is not kernel based, it may not be used to
 * 	   multiplex accesses from threads at different kernel
 * 	   priority levels.  Deadlock would result if a low-priority
 * 	   thread gained the lock and then was prempted by a
 * 	   high-priority thread that wanted to acquire it.
 * 	*) Architecture-dependent interfaces need to be designed to
 * 	   atomically lock and unlock this data structure.
 *
 * These are "simple locks" and are defined in flipc_dep.h.
 */

/*
 * Lock type.  This placement (in flipc_cb.h) is a little bit of a
 * hack, as it really should be defined with the machine dependent lock
 * macros.  But then the machine independent lock macros have problems
 * because they have to include it both before and after the prototypes.
 * So rather than split the machine dependent stuff into multiple
 * files, I'll define it here and hope that this definition works for
 * whatever architectures we're on.
 */
typedef unsigned long flipc_simple_lock;

/*
 * Ownership of data structures.
 *
 * Please note that this is a can of worms, and that I (Randys)
 * consider this (and it's interactions with endpoint group membership)
 * the likeliest place for design bugs in FLIPC.  Any and all should
 * take this as an open invitation and challenge to find bugs in what
 * follows.
 *
 * Rules:
 *
 *	*) If you've disabled a structure and synched with the
 *	   appropriate side of the ME, the ME won't touch it.
 *
 *	*) If you've taken a send endpoint off of the send endpoint
 *	   list and sync'd with the ME, the ME won't touch it.
 *
 *[The rest of this applies to the AIL only; the above rules are the
 * only ones the ME respects.  ]
 *
 *	*) Within the AIL, a disabled structure is owned by:
 *		*) The routine that disabled it, before it is put on
 *		   the free list.
 *		*) The routine that dequeued it from the free list,
 *		   before it is enabled.
 *	   Taking of the simple lock is not required for ownership in
 *	   these cases.  Taking of the simple lock is not required for
 *	   the act of *enabling* the structure (you have ownership and
 *	   are giving it away), however it is required for the act of
 *	   disabling the structure (since it is the only valid way to
 *	   take ownership of an enabled structure, and you can't
 *	   modify the enabled bit without having ownership).
 *
 *	*) The simple lock in a structure always needs to be valid, as
 *	   simple locks may be taken while the structure is in any
 *	   state.  Simiarly, the enabled bit must always be valid,
 *	   both because it's what the ME checks, and because it may be
 *	   checked by the AIL while the structure is free.
 *
 *	*) Holding the simple lock on an enabled structure imparts
 *	   ownership of that structure.  You are allowed to take the
 *	   simple lock of a disabled structure, but ownership is not
 *	   gained by doing so.
 *
 *	*) You are allowed to read the enabled/disabled bit without
 *	   owning the structure (if the structure is disabled, there
 *	   may be no way to gain the ownership).
 *
 *	*) Owning a structure allows you to do what you want with it,
 *	   except:
 *		*) As mentioned above, the simple lock and
 *		   enabled/disabled bit must always be valid.
 *		*) The ownership of the endpoint group related members
 *		   of an endpoint structure is special; see below.
 *	   	*) The allocations lock must be held to manipulate the
 *	   	   next send endpoint field of any endpoint.
 *
 *	*) If an endpoint is on an endpoint group, the ownership of
 *	   the the endpoint group related members of the structure
 *	   (sail_endpoint_group and pail_next_eg_endpoint) go with the
 *	   owndership of the endpoint group, not the endpoint.  For
 *	   this purpose only, membership is defined atomically as the
 *	   sail_endpoint_group pointer being set to an endpoint group.
 *	   Thus one may remove an endpoint from an endpoint group
 *	   without owning the endpoint (change the sail_endpoint_group
 *	   pointer last).  One requires both locks to add an endpoint
 *	   to an endpoint group, however.
 *
 *	   (Part of the motivation for this is that removal and
 *	   addition of endpoints to endpoint groups requires
 *	   modifications of pointers in other endpoint structures).
 *
 *	*) No structure may be put on the free list if marked with any
 *	   association to any other structure.  Specifically, endpoint
 *	   groups may have no endpoints belonging to them, and
 *	   endpoints may not belong to an endpoint group or have
 *	   buffers belonging to them.
 *
 *	*) One consequence of the above is that endpoint groups may
 *	   not be marked as disabled while they have any endpoints on
 *	   them, as freeing an endpoint requires it to be removed from
 *	   its endpoint group, and if ownership of the endpoint group
 *	   cannot be gained, that is impossible.
 *
 *	*) In theory, endpoints *may* be marked disabled while they
 *	   are still on endpoint groups.  In practice, they are not.
 *	   This is relied on by the code which frees endpoint groups,
 *	   in a non-obvious way.  Specifically, that code assumes that
 *	   there is no way that a call to free endpoint will return
 *	   with the endpoint still on the endpoint group.  Since the
 *	   only way for free endpoint to fail is if the endpoint is
 *	   inactive, and since the endpoint is set inactive only after
 *	   free endpoint (presumably a different one) confirms that it
 *	   isn't on any endpoint group, this assumption is true.
 *
 *	   Got that?  Take home lesson: don't allow endpoints to be
 *	   marked disabled while still on endpoint groups until you
 *	   *do* get that, and are willing to take the responsibility
 *	   of changing it so that it works under your new scheme.
 *
 *	*) Ownership of the freelist(s) are gained by holding the
 *	   allocations lock for the buffer, and *only* in that way.
 *	   No modification of freelist, send endpoint list, or send
 *	   side ME sync bits is valid without holding the allocations
 *	   lock.  In other words, while you can read things in the
 *	   main communications buffer control structure at will, you
 *	   may not change them without owning the allocations lock.
 *
 *	*) The state where a structure is disabled but off of the
 *	   freelist may be valid as an intermediate (while an AIL
 *	   routine is orchestrating a transition) but is not a valid
 *	   static state.  This state must not survive the return to
 *	   application code of the thread that disabled the structure.
 */

/*
 * Flipc data buffer management.
 *
 * A buffer (whether being used for sending or receiving) may be in
 * one of three states:
 *
 * READY -- Buffer held by application.
 * PROCESSING -- Buffer held by endpoint, unprocessed.  For receive endpoints,
 * 	   this means that the buffer is empty, waiting to be filled by
 * 	   an incoming message.  For send endpoints, this means tht the
 * 	   buffer is full, waiting to be sent out.
 * COMPLETED -- Buffer held by the endpoint, processed.  For receive
 * 	   endpoints, this means that the buffer is full, with newly
 * 	   received data in it.  For send endpoints, this means that the
 * 	   buffer is empty (*), with it's data having been sent out.
 *
 * 	   (*) In point of fact the data hasn't been touched, though bits
 * 	   may have been fiddled with in the header data structure.  But
 * 	   it's been sent.
 * FREE -- The buffer is in the pool of free buffers, and may be
 * allocated to any newly created endpoint.
 *
 * The transition diagram between these states is relatively simple:
 *
 *
 * 		     release
 *             /-----------------\|
 * +----------+                  -+----------+
 * |  READY   |                   |PROCESSING|<- - - - - -
 * +----------+_                  +----------+		  \
 *      ^     |\ - - - - - - - - /     |    | 		   \endpoint allocate
 *      |	   (processed)		    \endpoint       \
 *      |                              |     \ free         |
 *      | acquire                      /      ------\
 *      |				      	     \      |
 *      |                            / (processed)    >+----------+
 * +----------+				       	       |   FREE   |
 * |COMPLETED |< - - - - - - - - - -          	       +----------+
 * +----------+	                       endpoint allocate    /     ^
 *	|     ^- - - - - - - - - - - - - - - - - - - - - - -	  |
 *	|                                               	 /
 *	\		endpoint free                          	/
 *	 ------------------------------------------------------/
 *
 * (If it doesn't look simple, imagine it without the FREE state; that
 * state doesn't enter into almost any buffer manipulations)
 *
 * For send buffers, release==send, acquire==allocate, and
 * processed==the sending done by the message engine.  For receive buffers,
 * release==release, acquire==receive, and process==the actual
 * arrival of the message handled by the messaging engine.
 *
 * The choice of path from the PROCESSING state is an endpoint
 * specific configuration option; a particular endpoint may leave a
 * processed buffer on the endpoint, or it may release it back to the
 * application by dropping it from the endpoint.
 *
 * Buffers are assigned the PROCESSING state on a newly allocated
 * receive endpoint (to be ready to receive messages) and the
 * COMPLETED state on a newly allocated send endpoint.
 *
 * The state (other than FREE) that a particular buffer is in is
 * determined by its place on a circular queue of buffer pointers that
 * is part of the endpoint structure.  Buffers owned by the
 * application (READY) are not pointed to by pointers on this queue.
 * The buffer is released to the message engine by placement of a
 * pointer to it on this queue.  When the message engine is done
 * processing the buffer, it sets a flag in the buffer header.  If the
 * endpoint is so configured, it then removes the buffer pointer from
 * the queue; otherwise the AIL acquires the buffer (and removes the
 * pointer from the queue) when it chooses.
 *
 *      . . . . . .
 *     .           .
 *    .             .
 *    .             .       AIL releasing
 *    .             .       ^
 *    .         +-------+--/
 *    .         |       |
 *    .         |Buffers|
 *    .         | to be |
 *    .         |Sent or|
 *    .         |Receivd|
 *    .         | Into  |    ^ ME processing
 *    .         +-------+ --/
 *    .         |       |
 *    .  AIL    | Sent  |  (These buffers have a flag set to indicate
 *    .Acquiring|  or   |   that they have been processed.  This
 *    .         |Filled |   section is optional; the endpoint may be
 *    .         |buffers|   configured to drop buffers after processing)
 *    .     ^   |       |
 *    .      \--+-------+
 *    .             .
 *     .           .
 *      . . . . . .
 *
 *
 * The AIL will refuse to acquire a buffer that has not yet been
 * processed by the ME.  Acquire will not work at all on endpoints
 * that have been configured to drop buffers on completion.
 *
 * The buffer_available primitive is coded to avoid doing a
 * (potentially costly) acquiring of the endpoint flipc lock.  Since
 * telling where there is a buffer available requires two operations
 * (comparison of the acquire and release pointers to see if there are
 * any buffers on the endpoint, and then indirection of the acquire
 * pointer to see if that buffer has bee processed yet), there is a
 * potential race that will admit the possibility of indirecting
 * through an invalid pointer.  For this reason, for the life of an
 * endpoint, it is a requirement that all buffer pointers on the
 * bufferlist point *somewhere* (ie. to some existing buffer), so that
 * this indirection will not cause an access error.  The
 * buffer_available primitive may return the wrong result, but (as
 * long as the incorrectness is transitory), this is acceptable.
 */

/* Set up the states so that FLIPC_buffer_processed can just do an
   & and a test.  */
typedef enum {
    flipc_Free = 0x0, flipc_Processing = 0x1,
    flipc_Completed = 0x2, flipc_Ready = 0x3
} flipc_buffer_state_t;
#define FLIPC_BUFFER_PROCESSED_P(state) ((state) & 0x2)

/*
 * Data header/buffer layout.
 *
 * For this implementation, and probably for all time, the header
 * immediately precedes the data in memory, and the mesaging engine
 * will send both header and data.  Our priority is message dispatch
 * speed rather than raw bandwidth (this is the small message side of
 * a transfer mechanism), so we don't mind that we are throwing away
 * some bandwidth by taking up transferred space with header data.
 *
 * The data size will be the maximum size allowed by the underlying
 * transport, minus the header size (available at run time).  The user
 * will be given a pointer to the data buffer, and will use this both
 * for copying data in and out, and as an argument to the underlying
 * flipc routines.  The flipc routines will access appropriately.
 *
 * The header structure follows; the user data type will be offset and
 * cast appropriately to access this.
 */

typedef struct flipc_data_buffer {
    union {
	FLIPC_address_t destination; /* For sending.  */
	flipc_cb_ptr free;	/* Link for header free list.  */
    } u;

    /* ME owned if flipc_Processing, AIL owned otherwise.  May not ever
       assume the state flipc_Ready in an optimized implementation.  */
    volatile flipc_buffer_state_t shrd_state;
} *flipc_data_buffer_t;

/*
 * Endpoint structure.
 *
 * An endpoint is the data structure used for communicating buffers,
 * either send or receive.  Note that all actual circular lists of
 * buffer pointers on the endpoints are in their own array that gets
 * partitioned out to the various endpoints.  This is because we want
 * the endpoint structures themselves to be fixed size for easy
 * indexing upon receit of a message.  This large scale array will be
 * of size (max_buffers_per_endpoint) * (number_of_endpoints).  Both
 * of these values are set during the domain initialization call.
 *
 * Note that the pointers contained in the buffer lists are pointers to
 * buffer *headers*, not to the data.
 */

/*
 * This structure is divided into four cache lines, separated by their
 * usage type:
 *
 *	*) Private data that the AIL scribbles on.
 *	*) Data the AIL writes (regularly) that the ME reads
 *	   (occaisionally).  The canonical example is the release pointer.
 *	*) Private data that the ME scribbles on.
 *	*) Data the ME writes (regularly) that the AIL reads (occaisionally).
 *	   The canonical example is the process pointer.
 *
 * There are a couple of other categories of stuff, that can be shoehorned
 * into the above:
 *	*) Constant data that both sides read regularly.  This can be
 *	   duplicated in the two private areas (actually, it can be
 *	   duplicated in any two areas that stay in the cache of the
 *	   respective processors).
 *	*) Stuff that is not accessed on the critical path; it can go
 *	   almost anywhere (probably in one of the two ping-ponging
 *	   cache lines).
 *	*) Stuff that is read-only for a single processor goes in that
 *	   processors private data section.
 *
 * Duplicate entries have a "p" or a "a" suffixed to the name to
 * indicate that fact.  Note that these will usually, but not always,
 * be "const" variables--they may be "const" variables only from the
 * critical path viewpoint.
 *
 * We take cache line length as being 8 * sizeof(int).
 */

typedef struct flipc_endpoint {

    /* ===Private AIL data===  */
    /* Type of endpoint (send, recv, etc).  Duplicated in private
       ME section.  */
    FLIPC_endpoint_type_t constda_type;

    /* This next value is two variables squeezed into a single word to
     * save on memory accesses (since they are almost always read at
     * the same time.  The two variables are:
     *
     * const_drop_processed_buffers -- Should the message engine drop
     * buffers after processing them (as opposed to leaving them on
     * the endpoint)?
     *
     * sail_enabled (volatile) -- Is the endpoint enabled?  This isn't
     * marked constant because it is used for synchronization on
     * endpoint deallocation.
     *
     * Note that to reduce test and branches, we these two variables
     * are represented by two bits in the word (bit 0 and bit 16).  It
     * is illegal to have bits other than 0 and 16 set in this word.
     * This assumption is used in ENABLED_AND_NOT_DPB_P, and is enforced
     * in DOE_CONSTRUCT (assumed to not be performance critical) below.
     *
     * Duplicated in private ME section.
     */

    volatile unsigned long sailda_dpb_or_enabled;

#define EXTRACT_DPB(dpb_or_enabled) ((dpb_or_enabled) >> 16)
#define EXTRACT_ENABLED(dpb_or_enabled)  ((dpb_or_enabled) & 0xffff)
#define DISABLED_OR_DPB_P(dpb_or_enabled) ((dpb_or_enabled) ^ 0x1)
#define DOE_CONSTRUCT(dpb, enabled) \
    (((dpb) ? 0x10000 : 0) | ((enabled) ? 0x1 : 0))

    flipc_simple_lock pail_lock; /* Simple lock for serializing
				    multiple thread access to
				    structure.  AIL owned.  */
    /* First element in buffer list array that is ours.  Constant
       from communications buffer initialization.  */
    flipc_cb_ptr constda_my_buffer_list;
    /* First element after my_buffer_list that is *not* in my buffer
       list.  Constant from communications buffer initialization.  */
    flipc_cb_ptr constda_next_buffer_list;

    /* First location that has a valid buffer pointer in it.  This may
       contain a pointer to a buffer available for acquisition, or it
       may contain a pointer to a buffer that is still being
       processed; the buffer header or process_ptr needs to be checked
       to be sure.  This location is AIL owned.  It is ignored by all
       (including the ME and initialization code) if
       drop_processed_buffers, above, is set.  */
    volatile flipc_cb_ptr shrd_acquire_ptr;

    /* AIL private copy of process pointer.  This hopefully means that
       the AIL won't need to read the real process pointer (and fault
       in a cache line) very often.  */
    flipc_cb_ptr pail_process_ptr;

    unsigned int pad_pail_7;

    /* ===End of cache line===*/
    /* ===AIL writes, ME occaisionally reads=== */

    /* Next location at which the AIL may insert a buffer pointer.  */
    volatile flipc_cb_ptr sail_release_ptr;
    unsigned int pad_sail_1;
    unsigned int pad_sail_2;
    unsigned int pad_sail_3;
    unsigned int pad_sail_4;
    unsigned int pad_sail_5;
    unsigned int pad_sail_6;
    unsigned int pad_sail_7;

    /* ===End of cache line===*/
    /* ===Private ME data=== */
    /* See above comments (in private ail section).  */

    FLIPC_endpoint_type_t constdm_type;
    volatile unsigned long saildm_dpb_or_enabled;

    volatile unsigned long sme_overruns; /* For a receive endpoint, counter for
				     the number of messages that have
				     arrived when there hasn't been
				     space.  ME owned.   */
    unsigned long pail_overruns_seen;	/* A count of the number of overruns
				   that the AIL has noted and doesn't
				   want to be bothered with again.
				   The user only sees the difference
				   between the previous count and this.  */

    /*
     * For send endpoints; linked into a list that is used by the ME
     * to find stuff to do.  Also used for endpoint free list.
     * Null if at end of list.  Not "const" because it's used as a
     * synchronization variable during setup and teardown
     * of send endpoints.
     */
    volatile flipc_cb_ptr sail_next_send_endpoint;

    /* Constant buffer lsit pointers for ME.  See private ail comments.  */
    flipc_cb_ptr constdm_my_buffer_list;
    flipc_cb_ptr constdm_next_buffer_list;

    /* Private ME copy of release pointer.  This hopefully means that
       the ME won't have to read (and fault in a cache line) the
       release pointer very often.  */

    flipc_cb_ptr pme_release_ptr;
    /* ===End of cache line===*/

    /* ===ME writes, AIL occaisionally reads=== */
    /*
     * For endpoint group membership.
     */
    flipc_cb_ptr pail_next_eg_endpoint; /* Next endpoint in endpoint group.
					   AIL owned.  */
    flipc_cb_ptr sail_epgroup;	/* Direct pointer to endpoint group that
				   we are part of.  FLIPC_CBPTR_NULL
				   if none.  AIL owned.  */

    /* First location that has a buffer pointer available for
       processing. If this value is equal to the release_ptr there are no
       buffers available for processing.  */
    volatile flipc_cb_ptr sme_process_ptr;
    unsigned int pad_sme_3;
    unsigned int pad_sme_4;
    unsigned int pad_sme_5;
    unsigned int pad_sme_6;
    unsigned int pad_sme_7;

    /* ===End of cache line===*/
    /* ===END=== */

    /* The following macros may have possible performance loss in
       multiple accesses (or indirection, but a good compiler will get
       around that).  We need to have versions for each processor so
       that the constant reads are done from the right copy.  */

    /* General bufferlist pointer increment macro, with versions
       for ME and AIL.  */

#define NEXT_BUFFERLIST_PTR(bufferlist_ptr, endpoint, suf)	\
    (((bufferlist_ptr) + sizeof(flipc_data_buffer_t)		\
      == ((endpoint)->const ## suf ## _next_buffer_list)) ? 	\
     ((endpoint)->const ## suf ## _my_buffer_list) :		\
     (bufferlist_ptr) + sizeof(flipc_data_buffer_t))
#define NEXT_BUFFERLIST_PTR_ME(bufferlist_ptr, endpoint) \
    NEXT_BUFFERLIST_PTR(bufferlist_ptr, endpoint, dm)
#define NEXT_BUFFERLIST_PTR_AIL(bufferlist_ptr, endpoint) \
    NEXT_BUFFERLIST_PTR(bufferlist_ptr, endpoint, da)

    /* Macros for each of "can I release onto this buffer?"  "Can I
       acquire from this buffer?" and "Can I process an element on
       this buffer?"  The first two presume they are being executed on
       the main procesor, the third on the co-processor.
       All have three arguments:
       *) A variable which will be set to the release, acquire, or
       process pointer after the macro *if* the operation is ok.
       *) A temporary variable used inside the function.
       *) The endpoint.

       We presume the acquire macro won't be called if drop processed
       buffers is enabled; the process and release macros deal
       appropriately with that issue.  */

    /* In general these macros will:
       *) Not read a volatile structure member more than once.
       *) If a variables owner is the other processor, these macros
          will check a local copy of the variable first before checking
          the other processors.
       *) Will only update the local copy if the remote copy really is
          different from the local one.
	  */

/* This macro implements the synchronization check; local cbptr is
   the pointer owned by the local processor which we want to compare
   with a pointer on the remote processor which we have a copy
   of locally.  Reads the remote pointer zero or one times; other
   reads are as necessary.  

   The algorithm is: 
   *) If the local copy says our pointer and the remote value aren't equal,
      we're done.
   *) Otherwise, check the remote copy.  If it says the values aren't
      equal, update the local copy.  */

#define ENDPOINT_SYNCNE_CHECK(local_cbptr, copy_rmt_cbptr,	\
			      rmt_cbptr, tmp_cbptr)		\
    ((local_cbptr) != (copy_rmt_cbptr)				\
     || ((((tmp_cbptr) = (rmt_cbptr)) != (local_cbptr))		\
	 && (((copy_rmt_cbptr) = (tmp_cbptr)), 1)))

#define ENDPOINT_ACQUIRE_OK(acquire_cbptr, tmp_cbptr, endpoint)		\
    ((acquire_cbptr) = (endpoint)->shrd_acquire_ptr,			\
     ENDPOINT_SYNCNE_CHECK(acquire_cbptr, (endpoint)->pail_process_ptr,	\
			   (endpoint)->sme_process_ptr, tmp_cbptr))

#define ENDPOINT_PROCESS_OK(process_cbptr, tmp_cbptr, endpoint)		\
    ((process_cbptr) = (endpoint)->sme_process_ptr,			\
     ENDPOINT_SYNCNE_CHECK(process_cbptr, (endpoint)->pme_release_ptr,	\
			   (endpoint)->sail_release_ptr, tmp_cbptr))
      
#define NODPB_ENDPOINT_RELEASE_OK(release_cbptr, tmp_cbptr, endpoint)	\
    ((release_cbptr) = (endpoint)->sail_release_ptr,			\
     (tmp_cbptr) = (endpoint)->shrd_acquire_ptr,			\
     (NEXT_BUFFERLIST_PTR_AIL(release_cbptr, endpoint)			\
      != (tmp_cbptr)))	    
	    
/* Don't use NEXT_BUFFERLIST_PTR here to save a temporary variable.  */ 
#define DPB_ENDPOINT_RELEASE_OK(release_cbptr, tmp_cbptr, endpoint)	   \
    (release_cbptr = (endpoint)->sail_release_ptr,			   \
     ((release_cbptr + sizeof(flipc_data_buffer_t) ==			   \
       (endpoint)->constda_next_buffer_list)				   \
      ? ENDPOINT_SYNCNE_CHECK((endpoint)->constda_my_buffer_list,	   \
			      (endpoint)->pail_process_ptr,		   \
			      (endpoint)->sme_process_ptr,		   \
			      tmp_cbptr)				   \
      : ENDPOINT_SYNCNE_CHECK(release_cbptr + sizeof(flipc_data_buffer_t), \
			      (endpoint)->pail_process_ptr,		   \
			      (endpoint)->sme_process_ptr,		   \
			      tmp_cbptr)))

    /* This next is tricky; remember that acquire_ptr points
       to an actual bufferptr on the list, whereas release_ptr does
       not.  This macro is only used in FLIPC_endpoint_query, and so
       doesn't need to have an ME version.  */

#define BUFFERS_ON_ENDPOINT_AIL(acquire_ptr, release_ptr, endpoint)	\
    ((release_ptr) > (acquire_ptr)					\
     ? ((release_ptr) - (acquire_ptr)) / sizeof(flipc_cb_ptr)		\
     : ((((release_ptr) - (endpoint)->constda_my_buffer_list)		\
	 + ((endpoint)->constda_next_buffer_list - acquire_ptr))	\
	/ sizeof(flipc_cb_ptr)))
} *flipc_endpoint_t;


/*
 * Endpoint groups.
 *
 * Used to represent a group of endpoints, for linking sending/receiving
 * with semaphores & etc.  Note that there needs to be a private data
 * structure kept by the kernel that associates with each epgroup
 * a semaphore to be used for wakeups on that endpoint set.
 */

typedef struct flipc_epgroup {
    flipc_simple_lock pail_lock;	/* Lock to synchronize threads (at the
					   same priority level) accessing this
					   structure.  */
    volatile unsigned long sail_enabled;	/* Set if structure is active.  */
    unsigned long const_semaphore_associated; /* Flag to indicate whether or not
					  there is a semaphore associated
					  with this endpoint group in the
					  kernel flipc routines.  */
    volatile unsigned long sail_wakeup_req; /* Incremented when a thread wants to
					be woken.  */
    volatile unsigned long pme_wakeup_del; /* Incremented when the ME delivers a
				       wakeup. */
    unsigned long pail_version;		/* Incremented when epgroup membership
					   is changed; checked when retrieving
					   a buffer from an epgroup.  */
    unsigned long sail_msgs_per_wakeup;	/* How many messages need to arrive
					   before the ME delivers a wakeup.  */
    unsigned long pme_msgs_since_wakeup;	/* How many messages have arrived
					   since the last wakeup.  ME
					   owned.  */

    flipc_cb_ptr pail_first_endpoint; /* First endpoint in the group.  The
					 other endpoints are linked along
					 behind him.  AIL owned.  */
    flipc_cb_ptr pail_free;	/* Used to link this endpoint onto
				   the freelist.  */
} *flipc_epgroup_t;

/*
 * Communication buffer control structure.
 *
 * This is in the communications buffer itself.  Note that any changes
 * in this structure require it to be locked with the allocation lock,
 * as access to this structure is shared by all threads using the CB.
 */

/*
 * Individual data type layout.
 *
 * All we need here is a pointer to the start of each type of data
 * struct, the number of those data structures in the communications
 * buffer, and a pointer to the beginning of the freelist for that data
 * structure.
 *
 * Note that the composite buffer list doesn't have a freelist associated
 * with it, since each section of the buffer list is tightly bound to an
 * endpoint, and is allocated and freed with that endpoint.  We still
 * need the start and number information, though.
 */
struct flipc_cb_type_ctl {
    flipc_cb_ptr start;		/* Where there array of this type of
				   data structure starts.  */
    unsigned long number;		/* How many of them we've got.  */
    flipc_cb_ptr free;		/* Where the beginning of the freelist
				   is.  */
};

/*
 * Synchronization with message engine.
 *
 * At certain times (specifically during structure allocation/free or
 * additions to the send list) you want to know that the messaging
 * engine has picked up your changes.  However, the message engine has
 * (effectively) two threads, one for each of the send and receive
 * sides.  The mechanisms used for synchronizations with the two sides
 * differ.  In an eventual co-processor implementation (with a single
 * thread), only the send side mechanism will be used.
 *
 * To request a cached state flush by the send side of the mesasging
 * engine, you flip the request_sync bit and it responds by flipping
 * the response_sync bit.  The send ME checks this bit once every trip
 * through the send endpoints.
 *
 * On the receive side, since receives take very little time and do
 * not block (unlike sends) when we want to make sure the ME is
 * holding no cached receive side state, we simply spin until we see
 * that the ME receive side is no longer operating.  It sets a
 * variable whenever it is in the process of receiving a message.
 */

/*
 * Proper manipulation of the send endpoint list.
 *
 * Note that synchronizing with the message engine over access to the
 * send endpoint list is especially tricky.  There is no problem with
 * writing new values in all of the locations required to take a send
 * endpoint off of the list.  However, we must be very sure before
 * modifying the pointer *in* the send endpoint that the ME isn't
 * currently working in that send endpoint (else it could be sent off
 * into the void).  Two options here:
 *
 * 	*) Synchronize (using the below variables) for each send
 * 	   endpoint removed, after the removal but before the
 * 	   modification of the data in the internal structure.
 *	*) If we can always be sure that the send endpoint link in the
 *	   endpoint structure has a valid value, we can simply let the
 * 	   chips fall where they may.  It will be null while free, and
 * 	   have a value that points back into the send buffer list
 * 	   when reallocated.  I'm not going to do this; it's sleezy
 * 	   and will partially mess up fairness based on ME send
 * 	   endpoint round-robinning.
 */

/*
 * This entire structure is protected by an kernel level lock so there
 * is no conflict between threads accessing it.  See flipc_kfr.c for
 * details on this lock; how it is implemented and used depends on what
 * kernel base we are on.
 */

/*
 * Note that the last element of this structure is variable sized, so this
 * structure itself is also variable sized.
 */
typedef struct flipc_comm_buffer_ctl {
    /* Kernel flipc configuration that the user must match in order to
       work with this kernel.  Checked as soon as the comm buffer is
       mapped.  */
    struct {
	unsigned int real_time_primitives:1;
	unsigned int message_engine_in_kernel:1;
	unsigned int no_bus_locking:1; /* One way check -- if the kernel doesn't
				   have this and the user does, that's
				   an error.  */
    } kernel_configuration;
    volatile unsigned long	send_ready;	/* A send(s) is ready to go */

    /* These first three structures are constant after communications buffer
       initialization.  */
    unsigned long data_buffer_size; /* Size of the data buffers.  */
    unsigned long local_node_address; /* Local node number.  */
    FLIPC_address_t null_destination; /* Local null destination value.  */

#if REAL_TIME_PRIMITIVES
    /* The scheduling policy used by the task initializing flipc for
       the allocations lock.  */
    int allocations_lock_policy;
#else
    /* A poor substitute for a kernel level allocations lock.
       Note that this *cannot* be used as a regular simple lock;
       instead, try to acquire it, call sleep(1), try again, etc.
       Spinning on this lock will probably waste lots of cycles.  */
    flipc_simple_lock pail_alloc_lock;
#endif

    /* All of the members of these structures except for the free pointer
       are constant after initialization.  The free pointer is ail owned
       and private.  */
    struct flipc_cb_type_ctl endpoint;
    struct flipc_cb_type_ctl epgroup;
    struct flipc_cb_type_ctl bufferlist;
    struct flipc_cb_type_ctl data_buffer;

    /* Global synchronization with the message engine.  On the KKT
       implementation we need one synchronizer for each thread.  */

    /* Send side: */
    volatile unsigned long sail_request_sync; /* request_sync = !request_sync when the
					  AIL wants to synchronize with the
					  CB.  */
    volatile unsigned long sme_respond_sync; /* respond_sync = !respond_sync when
					 the ME has noticed the sync
					 request.  By responding to the
					 sync, the ME is stating that it has
					 no communications buffer state that
					 was cached previous to it noticing
					 the sync.    */

    /* Receive side.  */
    volatile unsigned long sme_receive_in_progress; /* Set by the ME before it looks at
						any data structures; cleared
						afterwards.  A simple spin in
						the user space on this
						variable will suffice, as the
						time that the message
						engine could be receiving
						is low.  */

    /* Send endpoint list starts here.  */
    volatile flipc_cb_ptr sail_send_endpoint_list; /* Null if no send endpoints.
						    */

    /* Keep track of whatever performance information we choose.  */
    struct FLIPC_domain_performance_info performance;

    /* Keep track of various kinds of error information here.  */
    struct FLIPC_domain_errors sme_error_log;

} *flipc_comm_buffer_ctl_t;


/*
 * The communications buffer.
 *
 * The only restriction on the layout of the communications buffer is
 * that the buffers themselves may not cross page boundaries.  So we
 * will place the data buffers at the end of the communications
 * buffer, and the other objects at the beginning, and there may be a
 * little bit of extra space in the middle.
 *
 * Note that this layout may change in future versions of FLIPC.
 *
 * 	+---------------------------+
 * 	|    flipc_comm_buffer_ctl  |
 * 	+---------------------------+
 * 	|       	            |
 * 	|         Endpoints         |
 * 	|       	            |
 * 	+---------------------------+
 * 	|       		    |
 * 	|      Endpoint Groups      |
 * 	|       	            |
 * 	+---------------------------+
 * 	|       	            |
 * 	| Combined Buffer Lists     |
 * 	|       	            |
 * 	+---------------------------+
 *	|			    |
 *	| (Possible empty space)    |
 *	|			    |
 * 	+---------------------------+
 * 	|                           |
 * 	|    Data Buffers	    |
 * 	|       	            |
 * 	+---------------------------+
 */

/* The number of pages that the kernel will reserve for the comm
   buffer.  The AIL needs to know this to know how much to map.  */
#define COMM_BUFFER_SIZE 0x100000

/*
 * These variables are set, in a per-address space context, to the base
 * and length of the communications buffer.  The ME needs to do bounds
 * checking to make sure it isn't overrunning anything.  Note that the
 * existence of these variables implies that an application will only
 * open a single domain.
 *
 * These declarations are duplicated in flipc/flipc_usermsg.h, and
 * should be kept in sync with that file.
 */
unsigned char *flipc_cb_base;
unsigned long flipc_cb_length;		/* In bytes.  */

/*
 * Following is a set of macros to convert back and forth between
 * real address pointers and flipc_cb_ptr's for each data type.  They
 * rely on the flipc_cb_base being set correctly.
 *
 * A possible future improvement might be to have bounds checking occur
 * inside these macros, but I'm not sure what I'd do if it failed.
 */

/* Easy going one way.  */
#define FLIPC_CBPTR(ptr) \
(((unsigned char *) (ptr)) - flipc_cb_base)

/* Need to get the right types going the other way.  */
#define FLIPC_ENDPOINT_PTR(cb_ptr) \
((flipc_endpoint_t) ((cb_ptr) + flipc_cb_base))
#define FLIPC_EPGROUP_PTR(cb_ptr) \
((flipc_epgroup_t) ((cb_ptr) + flipc_cb_base))
#define FLIPC_DATA_BUFFER_PTR(cb_ptr) \
((flipc_data_buffer_t) ((cb_ptr) + flipc_cb_base))
#define FLIPC_BUFFERLIST_PTR(cb_ptr) \
((flipc_cb_ptr *) ((cb_ptr) + flipc_cb_base))


/*
 * Flipc addresses.
 *
 * The addresses used by flipc for communication are defined in the
 * user visible header file as unsigned longs.  These macros pull that
 * information apart for use of the FLIPC internal routines.
 *
 * I assume in the following that endpoints immediately follow the
 * comm buffer control structure, because that makes indexing into
 * them much easier.
 */

#define FLIPC_CREATE_ADDRESS(node, endpoint_idx) \
((node << 16) | (endpoint_idx))
#define FLIPC_ADDRESS_NODE(addr) (((unsigned long) (addr)) >> 16)
#define FLIPC_ADDRESS_ENDPOINT(addr) (((unsigned long) (addr)) & 0xffff)

#endif /* _MACH_FLIPC_CB_H_ */
