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
 * Really a C file, but I'd like to have this code available in both
 * the kernel and the application, so I'll put it in a .h file.  This
 * file needs to be included only once in the AIL or ME, into a .c file
 * where it will be compiled.
 */

/* 
 * Since these are debug functions, it doesn't matter which processor macro
 * version I use; I don't mind spoiling cache while I'm debugging.
 */

#include <mach/flipc_cb.h>
/*
 * Print (using printf) all buffers in the communications buffer that
 * are not on any endpoint or on the buffer freelist.  Only active
 * endpoints are checked.
 *
 * Note that no locking is done; this function implicitly assumes the
 * communications buffer is in a quiescent state.  It is expected that
 * this function will normally be called from a debugger.
 *
 * As long as it is at it, this function prints buffers that are
 * doubly owned (valid pointers to them from two places).
 */

/*
 * Given that these functions will normally be called from the debugger,
 * there isn't any need to globally visible prototypes for them.  To
 * eliminate compilation warnings, we include prototypes for the functions
 * here in the file.
 */ 
static void flipcdbg_update_bufferset_bitvec(flipc_comm_buffer_ctl_t,
					     flipc_data_buffer_t);
void flipcdbg_print_unowned_buffers(void);
void flipcdbg_buffer_find_refs(flipc_cb_ptr buffer_cbptr);

#ifdef __GNUC__
__inline__
#endif
static void 
flipcdbg_update_bufferset_bitvec(flipc_comm_buffer_ctl_t cb_ctl,
				 flipc_data_buffer_t buffer)
{
    unsigned char *buffer_base = flipc_cb_base + cb_ctl->data_buffer.start;
    int bitpos = ((((unsigned char *) buffer) - buffer_base)
		  / cb_ctl->data_buffer_size);
    int element = bitpos / (sizeof(unsigned long) * 8);
    int subbitpos = bitpos - element * sizeof(unsigned long) * 8;

    /* Is that position set already?  */
    if (flipc_debug_buffer_bitvec[element] & (1 << subbitpos))
	printf("Buffer 0x%x (idx: %d, cbptr: 0x%x) is multiply referenced.\n",
	       buffer, bitpos, FLIPC_CBPTR(buffer));

    /* Set it.  */
    flipc_debug_buffer_bitvec[element] |= (1 << subbitpos);
}

void
flipcdbg_print_unowned_buffers(void)
{
    flipc_comm_buffer_ctl_t cb_ctl =
	(flipc_comm_buffer_ctl_t) flipc_cb_base;
    int i;
    unsigned long bitvec_length = ((cb_ctl->data_buffer.number + sizeof(unsigned long) * 8)
			    / (sizeof(unsigned int) * 8));
    flipc_data_buffer_t current_buffer;
    flipc_endpoint_t current_endpoint;
    flipc_cb_ptr current_cbptr;
    int header_printed = 0;

    /* Clean out the bitvec.  */
    for (i = 0; i < bitvec_length; i++)
	flipc_debug_buffer_bitvec[i] = 0;

    /* Go through the freelist, setting bits for each buffer.  */
    for (current_cbptr = cb_ctl->data_buffer.free;
	 current_cbptr != FLIPC_CBPTR_NULL;
	 current_cbptr = current_buffer->u.free) {
	int bitpos;
	int element, subbitpos;

	current_buffer = FLIPC_DATA_BUFFER_PTR(current_cbptr);
	flipcdbg_update_bufferset_bitvec(cb_ctl, current_buffer);
    }
    
    /* Go through all the endpoints, again setting bits for each buffer.  */
    for (current_endpoint = FLIPC_ENDPOINT_PTR(cb_ctl->endpoint.start);
	 (current_endpoint
	  < (FLIPC_ENDPOINT_PTR(cb_ctl->endpoint.start)
	     + cb_ctl->endpoint.number));
	 current_endpoint++) {
	if (EXTRACT_ENABLED(current_endpoint->saildm_dpb_or_enabled)) {
	    flipc_cb_ptr current_ptr =
		(EXTRACT_DPB(current_endpoint->saildm_dpb_or_enabled)
		 ? current_endpoint->sme_process_ptr 
		 : current_endpoint->shrd_acquire_ptr);
	    flipc_cb_ptr limit_ptr = current_endpoint->sail_release_ptr;

	    while (current_ptr != limit_ptr) {
		flipc_cb_ptr current_buffer_cbptr =
		    *FLIPC_BUFFERLIST_PTR(current_ptr);
		flipc_data_buffer_t current_buffer =
		    FLIPC_DATA_BUFFER_PTR(current_buffer_cbptr);

		/* Mark this as set.  */
		flipcdbg_update_bufferset_bitvec(cb_ctl, current_buffer);

		/* Increment the current pointer.  */
		current_ptr = NEXT_BUFFERLIST_PTR_ME(current_ptr,
						     current_endpoint);
	    }
	}
    }

    /* Ok, we should have marked every buffer that has a reference.
       Print out all the ones that done have references.  */
    for (i = 0; i < bitvec_length; i++) {
	int this_limit =
	    ((i == bitvec_length - 1)
	     ? cb_ctl->data_buffer.number % (sizeof(unsigned long)*8)
	     : sizeof(unsigned long)*8); 
	if (flipc_debug_buffer_bitvec[i] != (unsigned long) -1) {
	    int j;
	    for (j = 0; j < this_limit; j++) {
		if (!(flipc_debug_buffer_bitvec[i] & (1 << j))) {
		    int buffer_bitpos = i * sizeof(unsigned long) * 8 + j;
		    flipc_cb_ptr buffer_cbptr =
			(buffer_bitpos * cb_ctl->data_buffer_size
			 + cb_ctl->data_buffer.start);
		    flipc_data_buffer_t buffer_ptr =
			FLIPC_DATA_BUFFER_PTR(buffer_cbptr);
		    
		    /* Print header if necessary.  */
		    if (!header_printed) {
			header_printed = 1;
			printf("Unreferenced buffers (ptr,idx,cbptr):");
		    }

		    /* Print buffer.  */
		    printf(" (0x%x,%d,0x%x)", buffer_ptr, buffer_bitpos,
			   buffer_cbptr);
		}
	    }
	}
    }
    if (header_printed)
	printf("\n");
}

void
flipcdbg_buffer_find_refs(flipc_cb_ptr buffer_cbptr)
{
    flipc_comm_buffer_ctl_t cb_ctl =
	(flipc_comm_buffer_ctl_t) flipc_cb_base;
    int found_on_freelist = 0;
    int found_on_endpoints = 0;
    int i;
    flipc_endpoint_t current_endpoint;

    flipc_cb_ptr current_cbptr;
    flipc_data_buffer_t current_buffer;

    /* Go through the freelist, looking for buffer.  */
    for (i = 0, current_cbptr = cb_ctl->data_buffer.free;
	 current_cbptr != FLIPC_CBPTR_NULL;
	 i++, current_cbptr = current_buffer->u.free) {
	if (current_cbptr == buffer_cbptr) {
	    printf("Buffer found on freelist in position %d\n", i);
	    found_on_freelist = 1;
	}
	current_buffer = FLIPC_DATA_BUFFER_PTR(current_cbptr);
	if (i > cb_ctl->data_buffer.number) {
	    printf ("**Some form of corruption following freelist.**");
	    return;
	}
    }
    if (found_on_freelist)
	printf("(Total buffers on freelist: %d/%d)\n", i,
	       cb_ctl->data_buffer.number);
    
    /* Go through all the endpoints, again looking for the buffer.  */
    for (current_endpoint = FLIPC_ENDPOINT_PTR(cb_ctl->endpoint.start);
	 (current_endpoint
	  < (FLIPC_ENDPOINT_PTR(cb_ctl->endpoint.start)
	     + cb_ctl->endpoint.number));
	 current_endpoint++) {
	if (EXTRACT_ENABLED(current_endpoint->saildm_dpb_or_enabled)) {
	    flipc_cb_ptr current_ptr =
		(EXTRACT_DPB(current_endpoint->saildm_dpb_or_enabled)
		 ? current_endpoint->sme_process_ptr 
		 : current_endpoint->shrd_acquire_ptr);
	    flipc_cb_ptr limit_ptr = current_endpoint->sail_release_ptr;

	    while (current_ptr != limit_ptr) {
		current_cbptr = *FLIPC_BUFFERLIST_PTR(current_ptr);

		if (current_cbptr == buffer_cbptr) {
		    printf("Buffer found on endpoint 0x%x (idx: %d)\n",
			   current_endpoint,
			   (current_endpoint
			    - FLIPC_ENDPOINT_PTR(cb_ctl->endpoint.start)));
		    found_on_endpoints = 1;
		}

		/* Increment the current pointer.  */
		current_ptr = NEXT_BUFFERLIST_PTR_ME(current_ptr,
						     current_endpoint);
	    }
	}
    }
}



