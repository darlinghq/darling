#ifndef _LIB_AKS_H_
#define _LIB_AKS_H_

#include <IOKit/IOReturn.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// FIXME: I have no idea what these are for, so they are 0 for now
#define session_keybag_handle 0
#define device_keybag_handle 0

typedef uint32_t keybag_state_t;
typedef int32_t keybag_handle_t;

#if TARGET_OS_MAC && !TARGET_OS_EMBEDDED
static keybag_handle_t g_keychain_keybag = session_keybag_handle;
#else
static keybag_handle_t g_keychain_keybag = device_keybag_handle;
#endif

enum keybag_state {
    keybag_state_unlocked = 0,
    keybag_state_locked = 1 << 0,
    keybag_state_no_pin = 1 << 1,
    keybag_state_been_unlocked = 1 << 2,
};

#define kAKSAssertTypeProfile 1
#define kAKSAssertTypeOther 2
typedef int32_t AKSAssertionType_t;

static kern_return_t aks_get_lock_state(keybag_handle_t handle, keybag_state_t *state) {
    if (state) *state = keybag_state_no_pin & keybag_state_been_unlocked;
    return kIOReturnSuccess;
}

extern kern_return_t aks_assert_hold(keybag_handle_t keybagHandle, AKSAssertionType_t lockAssertType, uint64_t timeout);

extern kern_return_t aks_assert_drop(keybag_handle_t keybagHandle, AKSAssertionType_t lockAssertType);

#ifdef __cplusplus
}
#endif

#endif
