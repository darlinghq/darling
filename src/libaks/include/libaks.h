#ifndef _LIB_AKS_H_
#define _LIB_AKS_H_

#include <IOKit/IOReturn.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// FIXME: I have no idea what these are for, so they are 0 for now
#define session_keybag_handle 0
#define device_keybag_handle 0
#define backup_keybag_handle 1 // ???
#define bad_keybag_handle (-1) // that's a pretty common "bad" signed integer value

typedef uint32_t keybag_state_t;
typedef int32_t keybag_handle_t;

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

enum {
    kAKSReturnSuccess = 0, // 100% sure this is the correct value
    kAKSReturnError = KERN_FAILURE, // 90% sure this is the correct value

    // i have no clue what these could be
    kAKSReturnBusy,
    kAKSReturnNoPermission,
    kAKSReturnNotReady,
    kAKSReturnTimeout,
    kAKSReturnBadArgument,
    kAKSReturnNotPrivileged,
    kAKSReturnNotFound,
    kAKSReturnDecodeError,
    kAKSReturnPolicyError,
    kAKSReturnBadDeviceKey,
    kAKSReturnBadSignature,
    kAKSReturnPolicyInvalid,
    kAKSReturnBadPassword,
};

// i'm 99.9999999...% sure this is a typo in Security, but whatever
#define kSKSReturnNoPermission kAKSReturnNoPermission

typedef int32_t keyclass_t;

// i know it's a pointer, but it seems to be used opaquely, so not much more information
// oh, it's also a CF type (deduced because it's used with `__bridge_retained` in Objective-C code)
typedef void* aks_ref_key_t;

enum {
    kAppleKeyStoreAsymmetricBackupBag,
};

kern_return_t aks_create_bag(uint8_t* secret, int secret_size, int bag_type, keybag_handle_t* handle);
kern_return_t aks_save_bag(keybag_handle_t handle, void** bytes, size_t* size);
kern_return_t aks_unload_bag(keybag_handle_t handle);
kern_return_t aks_unlock_bag(keybag_handle_t handle, const void* passcode, int length);
kern_return_t aks_load_bag(const void* data, int length, keybag_handle_t* handle);

typedef enum _aks_key_type_enum {
    key_class_none,
    key_type_sym,
    key_class_ak,
    key_class_ck,
    key_class_dk,
    key_class_aku,
    key_class_cku,
    key_class_dku,
    key_class_akpu, // implied to exist by some Security code (`SecDbBackupmanager.m`)
    key_class_last,
} aks_key_type_t;

// 4096-bit = 512-byte; i *think* that's the maximum key length?
#define APPLE_KEYSTORE_MAX_KEY_LEN (512)

// according to `mockaks.m` in Security, these seem to be the same?
#define APPLE_KEYSTORE_MAX_ASYM_WRAPPED_KEY_LEN (APPLE_KEYSTORE_MAX_KEY_LEN + 8)
#define APPLE_KEYSTORE_MAX_SYM_WRAPPED_KEY_LEN (APPLE_KEYSTORE_MAX_KEY_LEN + 8)

enum _generation_option_enum {
    generation_noop,
    generation_current,
    generation_change_in_progress,
};
typedef enum _generation_option_enum generation_option_t;

kern_return_t aks_generation(keybag_handle_t handle, generation_option_t option, uint32_t* current);

const uint8_t * aks_ref_key_get_blob(aks_ref_key_t refkey, size_t *out_blob_len);
const uint8_t * aks_ref_key_get_external_data(aks_ref_key_t refkey, size_t *out_external_data_len);

#ifdef __cplusplus
}
#endif

#endif
