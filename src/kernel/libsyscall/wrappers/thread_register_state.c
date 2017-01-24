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
#include <mach/mach.h>

kern_return_t
thread_get_register_pointer_values(thread_t thread, uintptr_t *sp, size_t *length, uintptr_t *values)
{
    if (!length) return KERN_INVALID_ARGUMENT;
    if (*length > 0 && values == NULL) return KERN_INVALID_ARGUMENT;

    size_t in_length = *length;
    size_t out_length = 0;

#if defined(__i386__)
    i386_thread_state_t state = {};
    thread_state_flavor_t flavor = x86_THREAD_STATE32;
    mach_msg_type_number_t count = i386_THREAD_STATE_COUNT;
#elif defined(__x86_64__)
    x86_thread_state64_t state = {};
    thread_state_flavor_t flavor = x86_THREAD_STATE64;
    mach_msg_type_number_t count = x86_THREAD_STATE64_COUNT;
#else
#error thread_get_register_pointer_values not defined for this architecture
#endif

    kern_return_t ret = thread_get_state(thread, flavor, (thread_state_t)&state, &count);
    if (ret != KERN_SUCCESS){
        return ret;
    }

    // If the provided pointer value is > PAGE_SIZE, add it to the output array
    // if there's available space.  (Values between 0 and PAGE_SIZE are the NULL page
    // and not valid pointers.)
#define push_register_value(p) do { \
    if ((uintptr_t)p > PAGE_SIZE) { \
        if (out_length < in_length && values) \
            values[out_length] = p; \
        out_length++; \
    } } while (0)

#if defined(__i386__)
    if (sp) *sp = state.__esp;

    push_register_value(state.__eip);

    push_register_value(state.__eax);
    push_register_value(state.__ebx);
    push_register_value(state.__ecx);
    push_register_value(state.__edx);
    push_register_value(state.__edi);
    push_register_value(state.__esi);
    push_register_value(state.__ebp);
#elif defined(__x86_64__)
    if (sp) *sp = state.__rsp - 128 /* redzone */;

    push_register_value(state.__rip);

    push_register_value(state.__rax);
    push_register_value(state.__rbx);
    push_register_value(state.__rcx);
    push_register_value(state.__rdx);
    push_register_value(state.__rdi);
    push_register_value(state.__rbp);
    push_register_value(state.__r8);
    push_register_value(state.__r9);
    push_register_value(state.__r10);
    push_register_value(state.__r11);
    push_register_value(state.__r12);
    push_register_value(state.__r13);
    push_register_value(state.__r14);
    push_register_value(state.__r15);
#else
#error thread_get_register_pointer_values not defined for this architecture
#endif

    *length = out_length;

    if (in_length == 0 || out_length > in_length){
        return KERN_INSUFFICIENT_BUFFER_SIZE;
    }

	return KERN_SUCCESS;
}
