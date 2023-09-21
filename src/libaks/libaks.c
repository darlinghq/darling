#include <uuid/uuid.h>

#include "libaks.h"
#include "libaks_smartcard.h"

const CFStringRef kAKSKeyAcl                     = CFSTR("AKSKeyAcl");
const CFStringRef kAKSKeyAclParamRequirePasscode = CFSTR("AKSKeyAclParamRequirePasscode");
const CFStringRef kAKSKeyOpDefaultAcl            = CFSTR("AKSKeyOpDefaultAcl");
const CFStringRef kAKSKeyOpSign                  = CFSTR("AKSKeyOpSign");
const CFStringRef kAKSKeyOpComputeKey            = CFSTR("AKSKeyOpComputeKey");
const CFStringRef kAKSKeyOpAttest                = CFSTR("AKSKeyOpAttest");
const CFStringRef kAKSKeyOpDecrypt               = CFSTR("AKSKeyOpDecrypt");
const CFStringRef kAKSKeyOpEncrypt               = CFSTR("AKSKeyOpEncrypt");
const CFStringRef kAKSKeyOpDelete                = CFSTR("AKSKeyOpDelete");
const CFStringRef kAKSKeyOpECIESTranscode        = CFSTR("AKSKeyOpECIESTranscode");

void aks_smartcard_unregister(int a)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
}

int aks_smartcard_request_unlock(int a, unsigned char *b, int c, void **d, size_t *e)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
	return 0;
}

int aks_smartcard_unlock(int a, unsigned char *b, int c, unsigned char *d, int e, void **f, size_t *g)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
	return 0;
}

int aks_smartcard_register(int a, unsigned char *b, int c, int d, unsigned char *e, int f, void **g, size_t *h)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
	return 0;
}

int aks_smartcard_get_sc_usk(void *a, int b, const void **c, size_t *d)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
	return 0;
}

int aks_smartcard_get_ec_pub(void *a, int b, const void **c, size_t *d)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
	return 0;
}

kern_return_t aks_assert_hold(keybag_handle_t keybagHandle, AKSAssertionType_t lockAssertType, uint64_t timeout)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
	return KERN_FAILURE;
}

kern_return_t aks_assert_drop(keybag_handle_t keybagHandle, AKSAssertionType_t lockAssertType)
{
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	return KERN_FAILURE;
}

kern_return_t aks_create_bag(uint8_t* secret, int secret_size, int bag_type, keybag_handle_t* handle) {
    printf("STUB %s\n", __PRETTY_FUNCTION__);
	return KERN_FAILURE;
};

kern_return_t aks_save_bag(keybag_handle_t handle, void** bytes, size_t* size) {
    printf("STUB %s\n", __PRETTY_FUNCTION__);
	return KERN_FAILURE;
};

kern_return_t aks_unload_bag(keybag_handle_t handle) {
    printf("STUB %s\n", __PRETTY_FUNCTION__);
	return KERN_FAILURE;
};

kern_return_t aks_unlock_bag(keybag_handle_t handle, const void* passcode, int length) {
    printf("STUB %s\n", __PRETTY_FUNCTION__);
	return KERN_FAILURE;
};

kern_return_t aks_load_bag(const void* data, int length, keybag_handle_t* handle) {
    printf("STUB %s\n", __PRETTY_FUNCTION__);
	return KERN_FAILURE;
};

const uint8_t * aks_ref_key_get_blob(aks_ref_key_t refkey, size_t *out_blob_len) {
    printf("STUB %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

const uint8_t * aks_ref_key_get_external_data(aks_ref_key_t refkey, size_t *out_external_data_len) {
    printf("STUB %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

kern_return_t aks_generation(keybag_handle_t handle, generation_option_t option, uint32_t* current) {
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	return kAKSReturnError;
}

kern_return_t aks_get_bag_uuid(keybag_handle_t handle, uuid_t uuid) {
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	return kAKSReturnError;
}

kern_return_t aks_lock_bag(keybag_handle_t handle) {
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	return kAKSReturnError;
}

int aks_operation_optional_params(const uint8_t * access_groups, size_t access_groups_len, const uint8_t * external_data, size_t external_data_len, const void * acm_handle, int acm_handle_len, void ** out_der, size_t * out_der_len) {
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	return kAKSReturnError;
}

int aks_ref_key_create(keybag_handle_t handle, keyclass_t key_class, aks_key_type_t type, const uint8_t *params, size_t params_len, aks_ref_key_t *ot) {
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	return kAKSReturnError;
}

int aks_ref_key_create_with_blob(keybag_handle_t keybag, const uint8_t *ref_key_blob, size_t ref_key_blob_len, aks_ref_key_t* handle) {
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	return kAKSReturnError;
}

int aks_ref_key_encrypt(aks_ref_key_t handle, const uint8_t *der_params, size_t der_params_len, const void *data, size_t data_len, void **out_der, size_t *out_der_len) {
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	return kAKSReturnError;
}

int aks_ref_key_decrypt(aks_ref_key_t handle, const uint8_t *der_params, size_t der_params_len, const void *data, size_t data_len, void **out_der, size_t *out_der_len) {
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	return kAKSReturnError;
}

int aks_ref_key_free(aks_ref_key_t* refkey) {
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	return kAKSReturnError;
}

int aks_ref_key_delete(aks_ref_key_t handle, const uint8_t *der_params, size_t der_params_len) {
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	return kAKSReturnError;
}

const uint8_t * aks_ref_key_get_public_key(aks_ref_key_t refkey, size_t* out_pub_key_len) {
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	return NULL;
}

kern_return_t aks_wrap_key(const uint8_t *source, uint32_t textLength, keyclass_t keyclass, keybag_handle_t keybag, uint8_t *data, int *dest_len, keyclass_t *actual_class) {
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	return kAKSReturnError;
}

kern_return_t aks_unwrap_key(const uint8_t *source, uint32_t textLength, keyclass_t keyclass, keybag_handle_t keybag, uint8_t *data, int *dest_len) {
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	return kAKSReturnError;
}

bool hwaes_key_available(void) {
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	return false;
}
