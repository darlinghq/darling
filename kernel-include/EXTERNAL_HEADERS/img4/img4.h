/*!
 * @header
 * Image4 interfaces. These interfaces encapsulate the basic concepts required
 * for authenticating and validating Image4 manifests as being authoritative.
 * These concepts are:
 *
 * @section Environment
 * An environment is a description of a host comprised of hardware identifiers
 * and policy configurations. For example, the environment of an iPhone may
 * include the following hardware identifiers (among others):
 *
 *     ChipID
 *     A number identifying the chip design.
 *
 *     BoardID
 *     A number identifying the board.
 *
 *     UniqueChipID / ECID
 *     A number uniquely identifying a specific instance of a chip.
 *
 * The environment also includes policy information derived by previous stages
 * of secure boot. Examples of such policy are:
 *
 *     Mix-n-Match Prevention
 *     Whether firmware payloads from multiple, valid secure boot manifests
 *     should be prevented from being executed on the host environment. The
 *     default is true.
 *
 * @section Manifest
 * An Image4 manifest is a set of constraints that describe a host environment.
 * For example, a manifest may have been signed such that it is only valid for a
 * single host environment. In this case, the manifest may include specific
 * values for ChipID, BoardID, UniqueChipID, etc. Such a manifest is said to be
 * personalized for that environment.
 *
 * If an environment meets the constraints in a manifest, that manifest is said
 * to be authoritative over the environment.
 *
 * The manifest also includes one or more objects which may be executed in the
 * environment.
 *
 * @section Object
 * An object is a description of a payload. An object can describe any payload,
 * not just the payload that is in the Image4. An object describes a payload by
 * means of its digest. Examples of objects present in a secure boot manifest
 * are the kernelcache and the static trust cache.
 *
 * If an authoritative manifest accurately describes an object, then that object
 * may be executed in the host environment. The mechanics of execution typically
 * involve mapping its payload into a privileged memory region. For example,
 * when the kernelcache is executed, its payload bytes are mapped into the range
 * of memory associated with supervisor mode.
 *
 * Payload
 * A payload is the raw sequence of bytes that is described by an object. When
 * described via an Image4 object, payloads are first wrapped in Image4 encoding
 * to associate a tag with them. The resulting series of bytes is what is
 * contained in a .im4p file.
 *
 * An Image4 file may only contain a single payload (even though a manifest may
 * describe multiple payloads through multiple objects).
 *
 * Tag
 * A tag is a FourCC which can identify any of the following:
 *
 *     - an object property (e.g. the 'DGST' property)
 *     - a manifest property (e.g. the 'BORD' property)
 *     - a certificate property
 *     - a type of object (e.g. 'krnl')
 *
 * Tags comprised of all-caps are reserved for the Image4 specification.
 *
 * @section Secure Boot Policy
 * Manifests are evaluated with the Secure Boot evaluation policy. Broadly
 * speaking, this policy:
 *
 *     - enforces that manifest identifiers match the host's silicon
 *       identifiers,
 *     - enforces that the epoch of the certificate which signed the manifest is
 *       greater than or equal to the host silicon's epoch
 *     - enforces that the current manifest is the same one that was used in the
 *       previous stage of Secure Boot unless mix-n-match is allowed
 *
 * For manifests which lack a CHMH property, mix-n-match policy is enforced as
 * follows
 *
 *   (1) If the previous stage of Secure Boot disallows mix-n-match and the
 *       manifest does not possess the AMNM entitlement, the hash of the
 *       manifest will be enforced against the hash of the manifest which was
 *       evaluated by the previous stage of Secure Boot.
 *
 *   (2) If the previous stage of Secure Boot allows mix-n-match or the manifest
 *       possesses the AMNM entitlement, the manifest's constraints will be
 *       enforced on the environment, but the manifest will not be expected to
 *       be consistent with the manifest evaluated in the previous stage of
 *       Secure Boot, i.e. the hash of the previous manifest will not be
 *       enforced against the manifest being evaluated.
 *
 *       Enforcement of the manifest's constraints will include the value of the
 *       BNCH tag in the manifest, if any. Therefore the caller should always
 *       provide a nonce value to the implementation via {@link img4_set_nonce}
 *       if this option is used.
 *
 * For manifests which possess a CHMH property, mix-n-match policy is enforced
 * as follows:
 *
 *   (1) If the previous stage of Secure Boot disallows mix-n-match or the
 *       manifest does not possess the AMNM entitlement, the value of the CHMH
 *       property will be enforced against the hash of the manifest which was
 *       evaluated by the previous stage of Secure Boot.
 *
 *   (2) If the previous stage of Secure Boot allows mix-n-match and the
 *       manifest possesses the AMNM entitlement, all of the manifest's
 *       constraints will be enforced on the environment except for the CHMH
 *       constraint, which will be ignored.
 *
 *       Enforcement of the manifest's constraints will include the value of the
 *       BNCH tag in the manifest, if any. Therefore the caller should always
 *       provide a nonce value to the implementation via {@link img4_set_nonce}
 *       if this option is used.
 *
 * The CHMH policy may be expressed as the following truth table:
 *
 * AMNM [manifest]  Verify Manifest Hash [environment]   Enforce CHMH
 *        0                        0                           Y
 *        0                        1                           Y
 *        1                        0                           N
 *        1                        1                           Y
 */


#ifndef __IMG4_H
#define __IMG4_H

#include <os/base.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/cdefs.h>

#if KERNEL
#if !defined(OS_CLOSED_ENUM)
#define OS_CLOSED_ENUM(...) OS_ENUM(__VA_ARGS__)
#endif

#if !defined(OS_OPTIONS)
#define OS_OPTIONS(...) OS_ENUM(__VA_ARGS__)
#endif

#if !defined(OS_CLOSED_OPTIONS)
#define OS_CLOSED_OPTIONS(...) OS_ENUM(__VA_ARGS__)
#endif
#endif

#define __IMG4_INDIRECT 1

/*
 * When used from the pmap layer, this header pulls in the types from libsa,
 * which conflict with the BSD sys/types.h header that we need to pull in. But
 * we only need it for the errno_t typedef and the vnode_t typedef. So when
 * building MACH_KERNEL_PRIVATE, we do two things:
 *
 *     1. Explicitly pull in <sys/_types/_errno_t.h>, so we get errno_t and
 *        nothing else (no transitive #include's)
 *     2. #define _SYS_TYPES_H_ before #includ'ing <sys/kernel_types.h> so that
 *        we don't get the transitive #include of <sys/types.h> but we still get
 *        the definitions we need
 */
#if MACH_KERNEL_PRIVATE
#define _SYS_TYPES_H_ 1
#include <sys/kernel_types.h>
#include <sys/_types/_errno_t.h>
#else
#include <sys/kernel_types.h>
#include <sys/types.h>
#endif

#if !IMG4_PROJECT_BUILD
#include <img4/api.h>
#endif

__BEGIN_DECLS;

/*!
 * @typedef img4_tag_t
 * A type describing an Image4 tag.
 */
IMG4_API_AVAILABLE_20180112
typedef uint32_t img4_tag_t;

/*!
 * @typedef img4_section_t
 * A type describing the sections of an Image4 object.
 *
 * @const IMG4_SECTION_MANIFEST
 * The manifest section.
 *
 * @const IMG4_SECTION_OBJECT
 * The object section.
 *
 * @const IMG4_SECTION_RESTOREINFO
 * The restore info section.
 */
OS_ENUM(img4_section, uint8_t,
	IMG4_SECTION_MANIFEST,
	IMG4_SECTION_OBJECT,
	IMG4_SECTION_RESTOREINFO,
) IMG4_API_AVAILABLE_20180112;

/*!
 * @typedef img4_destructor_t
 * A type describing a destructor routine for an Image4 object.
 *
 * @param ptr
 * A pointer to the buffer to dispose of.
 *
 * @param len
 * The length of the buffer.
 */
IMG4_API_AVAILABLE_20180112
typedef void (*img4_destructor_t)(
	void *ptr,
	size_t len);

/*!
 * @typedef img4_flags_t
 * A flagset modifying the behavior of an {@link img4_t}.
 *
 * @const I4F_INIT
 * No flags set. This value is suitable for initialization purposes.
 *
 * @const I4F_TRUST_MANIFEST
 * Causes the implementation to bypass trust evaluation for the manifest, i.e.
 * it will not verify that a manifest has been signed by Apple before trusting
 * it.
 *
 * This option is for testing purposes only and is not respected on the RELEASE
 * variant of the implementation.
 *
 * @const I4F_FORCE_MIXNMATCH
 * Causes the implementation to bypass mix-n-match policy evaluation and always
 * allow mix-n-match, irrespective of the previous boot stage's conclusion or
 * manifest policy. This also allows replay of manifests whose personalization
 * has been invalidated by rolling the nonce.
 *
 * This option is for testing purposes only and is not respected on the RELEASE
 * variant of the implementation.
 *
 * @const I4F_FIRST_STAGE
 * Indicates that the manifest being evaluated is the first link in the secure
 * boot chain. This causes the implementation to enforce the manifest directly
 * on the environment rather than requiring that a previous stage has already
 * done so by way of checking the previous stage's boot manifest hash. In effect
 * this disables the mix-n-match enforcement policy.
 *
 * The critical difference between this flag and {@link I4F_FORCE_MIXNMATCH} is
 * that this flag will cause the entire manifest to be enforced on the
 * environment, including the anti-replay token in BNCH.
 * {@link I4F_FORCE_MIXNMATCH} will ignore the nonce.
 *
 * It is illegal to use a manifest which possesses a CHMH tag as a first-stage
 * manifest.
 */
OS_CLOSED_OPTIONS(img4_flags, uint64_t,
	I4F_INIT = 0,
	I4F_TRUST_MANIFEST = (1 << 0),
	I4F_FORCE_MIXNMATCH = (1 << 1),
	I4F_FIRST_STAGE = (1 << 2),
) IMG4_API_AVAILABLE_20180112;

typedef char _img4_opaque_data_64[696];

typedef char _img4_opaque_data_32[520];

/*!
 * @typedef img4_t
 * An opaque structure representing Image4 data. The Image4 data must contain a
 * manifest and may optionally contain a payload. Neither this type nor the APIs
 * APIs which manipulate it are thread-safe.
 */
IMG4_API_AVAILABLE_20180112
typedef struct _img4 {
#if __ILP64__ || __LP64__
	_img4_opaque_data_64 __opaque;
#else
	_img4_opaque_data_32 __opaque;
#endif
} img4_t;

typedef char _img4_payload_opaque_data_64[504];

#if __ARM_ARCH_7A__ || __ARM_ARCH_7S__ || __ARM_ARCH_7K__ || \
		__ARM64_ARCH_8_32__ || __i386__
typedef char _img4_payload_opaque_data_32[328];
#else
typedef char _img4_payload_opaque_data_32[332];
#endif

/*!
 * @typedef img4_payload_t
 * An opaque structure describing Image4 payload data. Neither this type nor the
 * APIs which manipulate it are thread-safe.
 */
IMG4_API_AVAILABLE_20180112
typedef struct _img4_payload {
#if __ILP64__ || __LP64__
	_img4_payload_opaque_data_64 __opaque;
#else
	_img4_payload_opaque_data_32 __opaque;
#endif
} img4_payload_t;

#if !IMG4_PROJECT_BUILD
#include <img4/environment.h>
#include <img4/nonce.h>
#include <img4/payload.h>
#endif

#if IMG4_TAPI
#include "environment.h"
#include "nonce.h"
#include "payload.h"
#endif

/*!
 * @function img4_init
 * Initializes an Image4.
 *
 * @param i4
 * A pointer to the storage to initialize.
 *
 * @param flags
 * Flags to modify initialization.
 *
 * @param bytes
 * The Image4 data from which to initialize. If a destructor is provided,
 * control of this buffer transfers to the Image4.
 *
 * @param len
 * The length of the Image4 data.
 *
 * @param destructor
 * A destructor for the Image4 data. May be NULL if the buffer does not require
 * explicit deallocation (e.g. because the buffer is stack data).
 *
 * @result
 * Upon success, zero is returned. The implementation may also return one of the
 * following error codes directly:
 *
 *     [EILSEQ]     The data is not valid Image4 data
 *     [EFTYPE]     The data does not contain an Image4 manifest
 *
 * @discussion
 * The bytes given to this routine must represent an Image4 manifest. They may
 * optionally also represent an Image4 payload.
 */
#if !XNU_KERNEL_PRIVATE
IMG4_API_AVAILABLE_20180112
OS_EXPORT OS_WARN_RESULT OS_NONNULL1 OS_NONNULL3
errno_t
img4_init(img4_t *i4, img4_flags_t flags, const uint8_t *bytes, size_t len,
		img4_destructor_t destructor);
#else
#define img4_init(...) (img4if->i4if_init(__VA_ARGS__))
#endif

/*!
 * @function img4_set_nonce
 * Sets the anti-reply token to be used during manifest enforcement. This value
 * will be compared against the value of the manifest's BNCH property.
 *
 * @param i4
 * The Image4 to modify.
 *
 * @param bytes
 * The bytes which comprise the anti-replay token.
 *
 * @param len
 * The length of the anti-replay token.
 *
 * @discussion
 * If a nonce is not set prior to a call to either
 * {@link img4_get_trusted_payload} or
 * {@link img4_get_trusted_external_payload}, the implementation will act as
 * though there is no nonce in the environment. Therefore, any manifests which
 * have a BNCH property constraint will fail to validate.
 */
#if !XNU_KERNEL_PRIVATE
IMG4_API_AVAILABLE_20180112
OS_EXPORT OS_NONNULL1 OS_NONNULL2
void
img4_set_nonce(img4_t *i4, const void *bytes, size_t len);
#else
#define img4_set_nonce(...) (img4if->i4if_set_nonce(__VA_ARGS__))
#endif

/*!
 * @function img4_set_nonce_domain
 * Sets the nonce domain to be consulted for the anti-replay token during
 * manifest enforcement.
 *
 * @param i4
 * The Image4 to modify.
 *
 * @param nd
 * The nonce domain to use for anti-replay.
 *
 * @discussion
 * See discussion for {@link img4_set_nonce}.
 */
#if !XNU_KERNEL_PRIVATE
IMG4_API_AVAILABLE_20181106
OS_EXPORT OS_NONNULL1 OS_NONNULL2
void
img4_set_nonce_domain(img4_t *i4, const img4_nonce_domain_t *nd);
#else
#define img4_set_nonce_domain(...) \
		(img4if->i4if_v1.set_nonce_domain(__VA_ARGS__))
#endif

/*!
 * @function img4_get_trusted_payload
 * Obtains the trusted payload bytes from the Image4.
 *
 * @param i4
 * The Image4 to query.
 *
 * @param tag
 * The tag for the payload to obtain.
 *
 * @param env
 * The environment against which to validate the Image4.
 *
 * @param bytes
 * A pointer to the storage where the pointer to the payload buffer will be
 * written on success.
 *
 * @param len
 * A pointer to the storage where the length of the payload buffer will be
 * written on success.
 *
 * @result
 * Upon success, zero is returned. The implementation may also return one of the
 * following error codes directly:
 *
 *     [ENOENT]     The Image4 does not contain a payload for the specified tag
 *     [EAUTH]      The Image4 manifest was not authentic
 *     [EACCES]     The environment given does not satisfy the manifest
 *                  constraints
 *     [ESTALE]     The nonce specified is not valid
 *     [EACCES]     The environment and manifest do not agree on a digest
 *                  algorithm
 *     [EILSEQ]     The payload for the given tag does not match its description
 *                  in the manifest
 *     [EIO]        The payload could not be fetched
 *
 * @discussion
 * This routine will perform the following validation:
 *
 *     1. Validate that the Image4 manifest is authentic (i.e. was signed by
 *        Apple)
 *     2. Validate that the given environment satisfies the constraints in the
 *        manifest
 *     3. Validate that the measurement of the payload for the given tag matches
 *        the measurement in the manifest
 *
 * If any one of these validation checks fails, the payload is considered
 * untrustworthy and is not returned.
 */
#if !XNU_KERNEL_PRIVATE
IMG4_API_AVAILABLE_20180112
OS_EXPORT OS_WARN_RESULT OS_NONNULL1 OS_NONNULL3 OS_NONNULL4 OS_NONNULL5
errno_t
img4_get_trusted_payload(img4_t *i4, img4_tag_t tag,
		const img4_environment_t *env, const uint8_t **bytes, size_t *len);
#else
#define img4_get_trusted_payload(...) \
		(img4if->i4if_get_trusted_payload(__VA_ARGS__))
#endif

/*!
 * @function img4_get_trusted_external_payload
 * Obtains the trusted payload bytes from the external Image4 payload after
 * validating them against the object description in the Image4's manifest.
 *
 * @param i4
 * The Image4 to query.
 *
 * @param payload
 * The payload to validate.
 *
 * @param env
 * The environment against which to validate the Image4.
 *
 * @param bytes
 * A pointer to the storage where the pointer to the payload buffer will be
 * written on success.
 *
 * If the payload objects was initialized with
 * {@link img4_payload_init_with_vnode_4xnu}, this parameter should be NULL, as
 * there will be no in-memory buffer to return.
 *
 * @param len
 * A pointer to the storage where the length of the payload buffer will be
 * written on success.
 *
 * If the payload objects was initialized with
 * {@link img4_payload_init_with_vnode_4xnu}, this parameter should be NULL, as
 * there will be no in-memory buffer to return.
 *
 * @result
 * Upon success, zero is returned. The implementation may also return one of the
 * following error codes directly:
 *
 *     [ENOENT]     The Image4 does not contain an object describing the given
 *                  payload
 *     [EAUTH]      The Image4 manifest was not authentic
 *     [EACCES]     The environment given does not satisfy the manifest
 *                  constraints
 *     [ESTALE]     The nonce specified is not valid
 *     [EACCES]     The environment and manifest do not agree on a digest
 *                  algorithm
 *     [EILSEQ]     The payload for the given tag does not match its description
 *                  in the manifest
 *     [EIO]        The payload could not be fetched
 *     [EIO]        The payload was initialized with
 *                  {@link img4_payload_init_with_vnode_4xnu}, and reading from
 *                  the vnode stalled repeatedly beyond the implementation's
 *                  tolerance
 *
 * If the payload was initialized with
 * {@link img4_payload_init_with_vnode_4xnu}, any error returned by
 * {@link vnode_getattr} or {@link vn_rdwr} may be returned.
 *
 * If the payload was initialized with
 * {@link img4_payload_init_with_fd_4MSM}, any error returned by stat(2),
 * read(2), or malloc(3) may be returned.
 *
 * Otherwise, an error from the underlying Image4 implementation will be
 * returned.
 *
 * @discussion
 * This routine performs the same validation steps as
 * {@link img4_get_trusted_payload} and has the same caveats.
 */
#if !XNU_KERNEL_PRIVATE
IMG4_API_AVAILABLE_20180112
OS_EXPORT OS_WARN_RESULT OS_NONNULL1 OS_NONNULL2 OS_NONNULL3
errno_t
img4_get_trusted_external_payload(img4_t *i4, img4_payload_t *payload,
		const img4_environment_t *env, const uint8_t **bytes, size_t *len);
#else
#define img4_get_trusted_external_payload(...) \
		(img4if->i4if_get_trusted_external_payload(__VA_ARGS__))
#endif

/*!
 * @function img4_destroy
 * Destroys an Image4 and disposes of associated resources.
 *
 * @param i4
 * The Image4 to destroy.
 *
 * @discussion
 * The destructor passed to {@link img4_init} is called as a result of this
 * routine, if any was set.
 */
#if !XNU_KERNEL_PRIVATE
IMG4_API_AVAILABLE_20180112
OS_EXPORT OS_NONNULL1
void
img4_destroy(img4_t *i4);
#else
#define img4_destroy(...) (img4if->i4if_destroy(__VA_ARGS__))
#endif

__END_DECLS;

#endif // __IMG4_H
