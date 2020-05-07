/*!
 * @header
 * Provides an interface for managing nonces to govern the lifetime of a
 * personalization performed with TSS. A nonce managed by this interface may
 * be used in a TSS signing request as the value for the BNCH tag.
 *
 * These interfaces require the caller to possess the
 *
 *     com.apple.private.security.AppleImage4.user-client
 *
 * entitlement.
 *
 * @section Threat Model
 * The adversary possesses the following:
 *
 *     1. a manifest which was previously valid but has since been invalidated
 *        by rolling the nonce associated with it
 *     2. user-level code execution
 *     3. knowledge of the raw nonce value for the previously-valid manifest
 *
 * The defense against this adversary is a system in which knowledge of the raw
 * nonce is insufficient to impact the evaluation of a personalization. This
 * system has the following characteristics:
 *
 *     1. A nonce seed is stored in an nvram variable which is only writable by
 *        the kernel
 *     2. When making a new signing request, the nonce seed is encrypted by a
 *        UID1-derived key in-kernel and then hashed -- the output of this
 *        operation the nonce to be used in the signing request
 *     3. On boot, AppleImage4 obtains the nonce seed from nvram and stores it
 *        in a data structure which will be covered by KTRR
 *     4. When evaluating a manifest, AppleImage4 reads the raw nonce from the
 *        KTRR-covered data structure and validates it with the same
 *        transformation as was done in (2)
 *     5. When the nonce is to be invalidated, AppleImage4 sets a flag in an
 *        nvram variable which is only writable by the kernel
 *     6. On the subsequent boot, AppleImage4 notices the flag, generates a new
 *        nonce and repeats the procedure in (3)
 *
 * In this system, the raw nonce seed never leaves the kernel, and the nonce
 * itself is a non-reversible representation of the seed.
 */


#ifndef __IMG4_NONCE_H
#define __IMG4_NONCE_H

#ifndef __IMG4_INDIRECT
#error "Please #include <img4/img4.h> instead of this file directly"
#endif // __IMG4_INDIRECT

#if IMG4_TAPI
#include "tapi.h"
#endif

/*!
 * @typedef img4_nonce_domain_t
 * An opaque type describing a nonce domain.
 */
IMG4_API_AVAILABLE_20181106
typedef struct _img4_nonce_domain img4_nonce_domain_t;

/*!
 * @const IMG4_NONCE_VERSION
 * The version of the {@link img4_nonce_t} structure supported by the
 * implementation.
 */
#define IMG4_NONCE_VERSION ((img4_struct_version_t)0)

/*!
 * @const IMG4_NONCE_MAX_LENGTH
 * The maximum length of a nonce. Currently, this is the length of a SHA2-384
 * hash.
 */
#define IMG4_NONCE_MAX_LENGTH (48)

/*!
 * @typedef img4_nonce_t
 * A structure describing a nonce.
 *
 * @field i4n_version
 * The version of the structure. When declaring this structure, you must
 * initialize this field to {@link IMG4_NONCE_VERSION}.
 *
 * @field i4n_nonce
 * The bytes comprising the nonce.
 *
 * @field i4n_length
 * The length of the nonce. Will be at most {@link IMG4_NONCE_MAX_LENGTH}.
 */
IMG4_API_AVAILABLE_20181106
typedef struct _img4_nonce {
	img4_struct_version_t i4n_version;
	const uint8_t i4n_nonce[IMG4_NONCE_MAX_LENGTH];
	uint32_t i4n_length;
} img4_nonce_t;

/*!
 * @const IMG4_NONCE_INIT
 * A convenience initializer for {@link img4_nonce_t} which ensures that the
 * {@link i4n_version} field is properly initialized.
 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define IMG4_NONCE_INIT (img4_nonce_t){.i4n_version = IMG4_NONCE_VERSION}
#elif defined(__cplusplus) && __cplusplus >= 201103L
#define IMG4_NONCE_INIT (img4_nonce_t{IMG4_NONCE_VERSION})
#elif defined(__cplusplus)
#define IMG4_NONCE_INIT \
		(img4_nonce_t((img4_nonce_t){IMG4_NONCE_VERSION}))
#else
#define IMG4_NONCE_INIT {IMG4_NONCE_VERSION}
#endif

/*!
 * @const IMG4_NONCE_DOMAIN_TRUST_CACHE
 * The nonce domain governing trust cache personalizations. Use of this domain
 * requires the
 *
 *     com.apple.private.img4.nonce.trust-cache
 *
 * entitlement.
 */
#if !XNU_KERNEL_PRIVATE
IMG4_API_AVAILABLE_20181106
OS_EXPORT
const struct _img4_nonce_domain _img4_nonce_domain_trust_cache;
#define IMG4_NONCE_DOMAIN_TRUST_CACHE (&_img4_nonce_domain_trust_cache)
#else
#define IMG4_NONCE_DOMAIN_TRUST_CACHE (img4if->i4if_v1.nonce_domain_trust_cache)
#endif

/*!
 * @const IMG4_NONCE_DOMAIN_PDI
 * The nonce domain governing disk image personalizations. Use of this domain
 * requires the
 *
 *     com.apple.private.img4.nonce.pdi
 *
 * entitlement. The nonce for this domain is regenerated once every boot.
 */
#if !XNU_KERNEL_PRIVATE
IMG4_API_AVAILABLE_20181106
OS_EXPORT
const struct _img4_nonce_domain _img4_nonce_domain_pdi;
#define IMG4_NONCE_DOMAIN_PDI (&_img4_nonce_domain_pdi)
#else
#define IMG4_NONCE_DOMAIN_PDI (img4if->i4if_v3.nonce_domain_pdi)
#endif

/*!
 * @const IMG4_NONCE_DOMAIN_CRYPTEX
 * The nonce domain governing cryptex personalizations. Use of this domain
 * requires the
 *
 *     com.apple.private.img4.nonce.cryptex
 *
 * entitlement.
 */
#if !XNU_KERNEL_PRIVATE
IMG4_API_AVAILABLE_20181106
OS_EXPORT
const struct _img4_nonce_domain _img4_nonce_domain_cryptex;
#define IMG4_NONCE_DOMAIN_CRYPTEX (&_img4_nonce_domain_cryptex)
#else
#define IMG4_NONCE_DOMAIN_CRYPTEX (img4if->i4if_v1.nonce_domain_cryptex)
#endif

/*!
 * @function img4_nonce_domain_copy_nonce
 * Copies the current value of the nonce in the given domain.
 *
 * @param nd
 * The nonce domain.
 *
 * @param n
 * Upon successful return, storage that will contain the current nonce. The
 * provided structure's {@link i4n_version} must be initialized to
 * {@link IMG4_NONCE_VERSION}.
 *
 * @result
 * Upon success, zero is returned. The implementation may also return one of the
 * following error codes directly:
 *
 *     [ESTALE]     The nonce for the given domain has been invalidated, and the
 *                  host must reboot in order to generate a new one
 *     [EPERM]      The caller lacked the entitlement necessary to read the
 *                  given nonce
 */
#if !XNU_KERNEL_PRIVATE
IMG4_API_AVAILABLE_20181106
OS_EXPORT OS_WARN_RESULT OS_NONNULL1 OS_NONNULL2
errno_t
img4_nonce_domain_copy_nonce(const img4_nonce_domain_t *nd, img4_nonce_t *n);
#else
#define img4_nonce_domain_copy_nonce(...) \
		(i4if->i4if_v1.nonce_domain_copy_nonce(__VA_ARGS__))
#endif

/*!
 * @function img4_nonce_domain_roll_nonce
 * Invalidates the current nonce for the given domain and forces a re-generation
 * of the domain's nonce seed at the next boot.
 *
 * @param nd
 * The nonce domain.
 *
 * @result
 * Upon success, zero is returned. The kernel implementation will never return
 * a non-zero code. The userspace implementation may return one of the following
 * error codes directly:
 *
 *     [EPERM]      The caller lacked the entitlement necessary to roll the
 *                  given nonce
 */
#if !XNU_KERNEL_PRIVATE
IMG4_API_AVAILABLE_20181106
OS_EXPORT OS_NONNULL1
errno_t
img4_nonce_domain_roll_nonce(const img4_nonce_domain_t *nd);
#else
#define img4_nonce_domain_roll_nonce(...) \
		(i4if->i4if_v1.nonce_domain_roll_nonce(__VA_ARGS__))
#endif

#endif // __IMG4_NONCE_H
