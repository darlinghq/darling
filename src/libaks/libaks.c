#define __STDC_WANT_LIB_EXT1__ 1
#include <uuid/uuid.h>
#include <CommonCrypto/CommonRandom.h>
#include <CommonCrypto/CommonSymmetricKeywrap.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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

kern_return_t aks_save_bag(keybag_handle_t handle, void** bytes, int* size) {
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

/*
 * A software key store behind aks_wrap_key and aks_unwrap_key. There is no
 * Apple key store hardware, so each key class gets one random 256-bit class key,
 * kept in a file that only the owner of the store (the daemon, root) can read, and
 * a key is wrapped with it by the standard AES key wrap (RFC 3394), which makes the
 * wrapped key 8 bytes longer, as the callers expect. Nothing is locked: the keys are
 * available whenever the process may read the file. There is one key bag, the device
 * key bag (handle 0).
 */
#define AKS_STORE_DIRECTORY "/var/db/org.darlinghq.libaks"
#define AKS_CLASS_KEY_SIZE 32
#define AKS_MAX_CLASS 0xff
/* the largest key the callers' int lengths can hold once it is wrapped */
#define AKS_MAX_KEY_SIZE ((uint32_t)INT_MAX - 8)

static pthread_mutex_t aks_store_lock = PTHREAD_MUTEX_INITIALIZER;

/* Clears key material in a way the compiler may not drop. */
static void aks_clear(void* buffer, size_t length)
{
	if (length != 0)
		memset_s(buffer, length, 0, length);
}

/* Reads a class key file; it has to be a regular file of exactly the size of a class key. */
static bool aks_read_class_key(const char* path, uint8_t key[AKS_CLASS_KEY_SIZE])
{
	struct stat st;
	size_t done = 0;
	bool ok = false;

	int fd = open(path, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
	if (fd < 0)
		return false;
	if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode) || st.st_size != AKS_CLASS_KEY_SIZE)
		goto out;
	while (done < AKS_CLASS_KEY_SIZE) {
		ssize_t n = read(fd, key + done, AKS_CLASS_KEY_SIZE - done);
		if (n < 0 && errno == EINTR)
			continue;
		if (n <= 0)
			goto out;
		done += (size_t)n;
	}
	ok = true;
out:
	close(fd);
	if (!ok)
		aks_clear(key, AKS_CLASS_KEY_SIZE);
	return ok;
}

static bool aks_write_all(int fd, const uint8_t* bytes, size_t length)
{
	while (length != 0) {
		ssize_t n = write(fd, bytes, length);
		if (n < 0 && errno == EINTR)
			continue;
		if (n <= 0)
			return false;
		bytes += n;
		length -= (size_t)n;
	}
	return true;
}

/* Makes a new directory entry durable. */
static bool aks_sync_directory(void)
{
	int fd = open(AKS_STORE_DIRECTORY, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
	if (fd < 0)
		return false;
	bool ok = fsync(fd) == 0;
	close(fd);
	return ok;
}

/* The class key of a key class; it is created on first use. */
static bool aks_class_key(keybag_handle_t keybag, keyclass_t keyclass, uint8_t key[AKS_CLASS_KEY_SIZE])
{
	char path[128], temp[160];
	bool ok = false;

	if ((uint32_t)keyclass > AKS_MAX_CLASS)
		return false;
	snprintf(path, sizeof(path), AKS_STORE_DIRECTORY "/bag%d-class%d.key", (int)keybag, (int)keyclass);

	pthread_mutex_lock(&aks_store_lock);
	if (aks_read_class_key(path, key)) {
		ok = true;
		goto out;
	}
	/* A class key file that is there but unusable is not replaced: the items wrapped with it would be lost. */
	if (access(path, F_OK) == 0 || errno != ENOENT)
		goto out;

	if (mkdir(AKS_STORE_DIRECTORY, 0700) != 0 && errno != EEXIST)
		goto out;
	if (CCRandomGenerateBytes(key, AKS_CLASS_KEY_SIZE) != kCCSuccess)
		goto out;

	/*
	 * Written and synced under a unique temporary name and linked in, so that processes racing to create
	 * the key agree on one and a crash leaves either no key or a whole one.
	 */
	snprintf(temp, sizeof(temp), "%s.XXXXXX", path);
	int fd = mkstemp(temp);
	if (fd < 0)
		goto out;
	bool written = fchmod(fd, 0600) == 0 && aks_write_all(fd, key, AKS_CLASS_KEY_SIZE) && fsync(fd) == 0;
	close(fd);
	if (written && (link(temp, path) == 0 || errno == EEXIST) && aks_sync_directory())
		ok = aks_read_class_key(path, key);
	unlink(temp);

out:
	pthread_mutex_unlock(&aks_store_lock);
	if (!ok)
		aks_clear(key, AKS_CLASS_KEY_SIZE);
	return ok;
}

kern_return_t aks_wrap_key(const uint8_t *source, uint32_t textLength, keyclass_t keyclass, keybag_handle_t keybag, uint8_t *data, int *dest_len, keyclass_t *actual_class) {
	uint8_t classKey[AKS_CLASS_KEY_SIZE];
	size_t wrappedLength;

	if (!source || !data || !dest_len || textLength < 16 || textLength % 8 != 0 || textLength > AKS_MAX_KEY_SIZE ||
	    *dest_len < 0 || (uint64_t)*dest_len < (uint64_t)textLength + 8)
		return kAKSReturnBadArgument;
	if (keybag != device_keybag_handle)
		return kAKSReturnBadArgument;
	if (!aks_class_key(keybag, keyclass, classKey))
		return kAKSReturnNotFound;

	wrappedLength = (size_t)textLength + 8;
	CCCryptorStatus status = CCSymmetricKeyWrap(kCCWRAPAES, CCrfc3394_iv, CCrfc3394_ivLen, classKey, sizeof(classKey), source, textLength, data, &wrappedLength);
	aks_clear(classKey, sizeof(classKey));
	if (status != kCCSuccess || wrappedLength != (size_t)textLength + 8) {
		aks_clear(data, (size_t)textLength + 8);
		return kAKSReturnError;
	}

	*dest_len = (int)wrappedLength;
	if (actual_class)
		*actual_class = keyclass;
	return kAKSReturnSuccess;
}

kern_return_t aks_unwrap_key(const uint8_t *source, uint32_t textLength, keyclass_t keyclass, keybag_handle_t keybag, uint8_t *data, int *dest_len) {
	uint8_t classKey[AKS_CLASS_KEY_SIZE];
	size_t unwrappedLength;

	if (!source || !data || !dest_len || textLength < 24 || textLength % 8 != 0 || textLength - 8 > AKS_MAX_KEY_SIZE ||
	    *dest_len < 0 || (uint64_t)*dest_len < (uint64_t)textLength - 8)
		return kAKSReturnBadArgument;
	if (keybag != device_keybag_handle)
		return kAKSReturnBadArgument;
	if (!aks_class_key(keybag, keyclass, classKey))
		return kAKSReturnNotFound;

	unwrappedLength = (size_t)textLength - 8;
	CCCryptorStatus status = CCSymmetricKeyUnwrap(kCCWRAPAES, CCrfc3394_iv, CCrfc3394_ivLen, classKey, sizeof(classKey), source, textLength, data, &unwrappedLength);
	aks_clear(classKey, sizeof(classKey));
	if (status != kCCSuccess || unwrappedLength != (size_t)textLength - 8) {
		/* nothing of a key that failed its integrity check is handed out */
		aks_clear(data, (size_t)textLength - 8);
		return kAKSReturnDecodeError;
	}

	*dest_len = (int)unwrappedLength;
	return kAKSReturnSuccess;
}

bool hwaes_key_available(void) {
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	return false;
}
