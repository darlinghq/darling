// SPDX-License-Identifier: GPL-3.0-or-later
#include <assert.h>
#include <mach/mach.h>
#include <mach/arm/thread_status.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    mach_port_t thread = mach_thread_self();
    arm_debug_state64_t state;
    memset(&state, 0xff, sizeof(state));
    mach_msg_type_number_t count = ARM_DEBUG_STATE64_COUNT;
    assert(thread_get_state(thread, ARM_DEBUG_STATE64, (thread_state_t)&state, &count) == KERN_SUCCESS);
    assert(count == ARM_DEBUG_STATE64_COUNT);
    const unsigned char *bytes = (const unsigned char *)&state;
    for (size_t i = 0; i < sizeof(state); ++i)
        assert(bytes[i] == 0);
    assert(thread_set_state(thread, ARM_DEBUG_STATE64, (thread_state_t)&state, count) == KERN_SUCCESS);

    count = ARM_DEBUG_STATE64_COUNT - 1;
    assert(thread_get_state(thread, ARM_DEBUG_STATE64, (thread_state_t)&state, &count) == KERN_INVALID_ARGUMENT);
    assert(thread_set_state(thread, ARM_DEBUG_STATE64, (thread_state_t)&state, count) == KERN_INVALID_ARGUMENT);
    // Exercise each register independently, including the single-step control.
    for (size_t i = 0; i < sizeof(state); ++i) {
        ((unsigned char *)&state)[i] = 1;
        assert(thread_set_state(thread, ARM_DEBUG_STATE64, (thread_state_t)&state,
                                ARM_DEBUG_STATE64_COUNT) == KERN_NOT_SUPPORTED);
        ((unsigned char *)&state)[i] = 0;
    }
    count = ARM_DEBUG_STATE64_COUNT;
    assert(thread_get_state(thread, ARM_DEBUG_STATE64, (thread_state_t)&state, &count) == KERN_SUCCESS);
    for (size_t i = 0; i < sizeof(state); ++i)
        assert(bytes[i] == 0);
    assert(mach_port_deallocate(mach_task_self(), thread) == KERN_SUCCESS);
    puts("PASS: inactive state capture/reset, counts, and rejection of active debug registers");
    return 0;
}
