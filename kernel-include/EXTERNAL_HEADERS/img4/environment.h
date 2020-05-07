/*!
 * @header
 * Image4 environments.
 */
#ifndef __IMG4_ENVIRONMENT_H
#define __IMG4_ENVIRONMENT_H

#ifndef __IMG4_INDIRECT
#error "Please #include <img4/img4.h> instead of this file directly"
#endif // __IMG4_INDIRECT

#if IMG4_TAPI
#include "tapi.h"
#endif

/*!
 * @const IMG4_IDENTITY_VERSION
 * The version of the {@link img4_identity_t} supported by the implementation.
 */
#define IMG4_IDENTITY_VERSION (0u)

/*!
 * @const IMG4_ENVIRONMENT_LENGTH
 * The minimum length for an allocation which can accommodate an
 * img4_environment_t structure. This is the minimum length which must be given
 * to {@link img4_environment_init_identity}.
 */
#define IMG4_ENVIRONMENT_LENGTH (160ul)

/*!
 * @const IMG4_IDENTITY_CRYPTO_SHA1
 * The device-tree string indicating that the identity requires SHA1.
 */
#define IMG4_IDENTITY_CRYPTO_SHA1 "sha1"

/*!
 * @const IMG4_IDENTITY_CRYPTO_SHA2_384
 * The device-tree string indicating that the identity requires SHA2-384.
 */
#define IMG4_IDENTITY_CRYPTO_SHA2_384 "sha2-384"

/*!
 * @typedef img4_environment_t
 * An opaque type describing an Image4 environment.
 */
typedef struct _img4_environment img4_environment_t;

/*!
 * @typedef img4_identity_t
 * A structure describing a specific Image4 identity comprised of user-supplied
 * identifiers.
 *
 * @field i4id_version
 * The version of the identity structure; initialize to
 * {@link IMG4_IDENTITY_VERSION}
 *
 * @field i4id_algo
 * A string identifying the chosen crypto algorithm as represented in the device
 * tree. Currently valid values are:
 *
 *     - {@link IMG4_IDENTITY_CRYPTO_SHA1}
 *     - {@link IMG4_IDENTITY_CRYPTO_SHA2_384}
 *
 * @field i4id_cepo
 * The minimum certificate epoch required,
 *
 * @field i4id_bord
 * The board identifier.
 *
 * @field i4id_chip
 * The chip identifier.
 *
 * @field i4id_ecid
 * The unique chip identifier.
 *
 * @field i4id_sdom
 * The security domain.
 *
 * @field i4id_cpro
 * The certificate production status.
 *
 * @field i4id_csec
 * The certificate security mode.
 *
 * @field i4id_epro
 * The effective production status.
 *
 * @field i4id_esec
 * The effective security mode.
 */
IMG4_API_AVAILABLE_20191001
typedef struct _img4_identity {
	img4_struct_version_t i4id_version;
	char i4id_algo[12];
	uint32_t i4id_cepo;
	uint32_t i4id_bord;
	uint32_t i4id_chip;
	uint64_t i4id_ecid;
	uint32_t i4id_sdom;
	bool i4id_cpro;
	bool i4id_csec;
	bool i4id_epro;
	bool i4id_esec;
} img4_identity_t;

/*!
 * @const IMG4_ENVIRONMENT_PLATFORM
 * The environment for the host that uses the default platform implementation to
 * resolve the environment. This is the environment against which manifests are
 * personalized.
 */
#if !XNU_KERNEL_PRIVATE
IMG4_API_AVAILABLE_20180112
OS_EXPORT
const struct _img4_environment _img4_environment_platform;
#define IMG4_ENVIRONMENT_PLATFORM (&_img4_environment_platform)
#else
#define IMG4_ENVIRONMENT_PLATFORM (img4if->i4if_environment_platform)
#endif


/*!
 * @const IMG4_ENVIRONMENT_TRUST_CACHE
 * The software environment for globally-signed loadable trust caches. This
 * environment should be used as a fallback when validation against the platform
 * fails, and the caller is handling a loadable trust cache.
 */
#if !XNU_KERNEL_PRIVATE
IMG4_API_AVAILABLE_20181004
OS_EXPORT
const struct _img4_environment _img4_environment_trust_cache;
#define IMG4_ENVIRONMENT_TRUST_CACHE (&_img4_environment_trust_cache)
#else
#define IMG4_ENVIRONMENT_TRUST_CACHE (img4if->i4if_environment_trust_cache)
#endif

/*!
 * @function img4_environment_init_identity
 * Initializes a caller-supplied environment with custom identity information.
 * This may be used for performing test evaluations or evaluations against
 * environments not yet supported by the implementation.
 *
 * @param i4e
 * A pointer to the storage which will hold the custom environment.
 *
 * @param len
 * The length of the storage referenced by {@link i4e}. This must be at least
 * {@link IMG4_ENVIRONMENT_LENGTH} bytes.
 *
 * @param i4id
 * The identity with which to initialize the environment. The resulting
 * environment object will provide these identitifers to the evaluator.
 *
 * @result
 * Upon success, zero is returned. The implementation may also return one of the
 * following error codes directly:
 *
 *     [EOVERFLOW]    The length provided is insufficient to initialize an
 *                    environment structure
 *
 * @discussion
 * When the resulting environment is given to {@link img4_get_trusted_payload}
 * or {@link img4_get_trusted_external_payload}, the trust evaluation proceeds
 * as though it were creating a new chain of trust and therefore acts as though
 * {@link I4F_FIRST_STAGE} was given to {@link img4_init}. No prior stage of
 * secure boot will be consulted for evaluation, and mix-n-match will be
 * presumed to be permitted.
 */
#if !XNU_KERNEL_PRIVATE
IMG4_API_AVAILABLE_20191001
OS_EXPORT OS_WARN_RESULT OS_NONNULL1 OS_NONNULL3
errno_t
img4_environment_init_identity(img4_environment_t *i4e, size_t len,
		const img4_identity_t *i4id);
#else
#define img4_environment_init_identity(...) \
		(img4if->i4if_v4.environment_init_identity(__VA_ARGS__))
#endif

#endif // __IMG4_ENVIRONMENT_H
