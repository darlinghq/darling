/*!
 * @header
 * Image4 payload interfaces. These interfaces provide a lightweight type for
 * working with an Image4 payload that is described by a separate manifest (e.g.
 * a .im4p file whose contents are described by an object in a manifest from a
 * .im4m file).
 *
 * No direct access is provided to the raw payload bytes encapsulated by the
 * Image4 payload by design. The intent is that in order to access the raw
 * bytes, the payload object must be validated against a manifest object using
 * the {@link img4_get_trusted_external_payload} interface.
 */
#ifndef __IMG4_PAYLOAD_H
#define __IMG4_PAYLOAD_H

#ifndef __IMG4_INDIRECT
#error "Please #include <img4/img4.h> instead of this file directly"
#endif // __IMG4_INDIRECT

#if IMG4_TAPI
#include "tapi.h"
#endif

/*!
 * @typedef img4_payload_flags_t
 * Flags modifying the behavior of an Image4 payload object.
 *
 * @const I4PLF_INIT
 * No flags set. This value is suitable for initialization purposes.
 *
 * @const I4PLF_UNWRAPPED
 * Indicates that the payload bytes are not wrapped in an Image4 payload object
 * (.im4p file). If this flag is given, the payload tag is ignored.
 *
 * This should be used in scenarios such as x86 SecureBoot, which use Image4 to
 * describe portable executable files which must be fed directly to the firmware
 * and cannot tolerate being wrapped in an intermediary format.
 */
OS_CLOSED_OPTIONS(img4_payload_flags, uint64_t,
	I4PLF_INIT = 0,
	I4PLF_UNWRAPPED = (1 << 0),
);

/*!
 * @function img4_payload_init
 * Initializes an Image4 payload object.
 *
 * @param i4p
 * A pointer to the payload object to initialize.
 *
 * @param tag
 * The expected tag for the payload.
 *
 * @param flags
 * Flags modifying the behavior of the payload object.
 *
 * @param bytes
 * The buffer containing the Image4 payload.
 *
 * @param len
 * The length of the buffer.
 *
 * @param destructor
 * A pointer to a routine to dispose of the buffer. May be NULL if the buffer
 * does not require explicit disposal (e.g. the buffer is stack memory).
 *
 * @result
 * Upon success, zero is returned. Otherwise, one of the following error codes:
 *
 *     [EILSEQ]     The data is not valid Image4 data
 *     [EFTYPE]     The data does not contain an Image4 payload
 *     [ENOENT]     The bytes do not contain a payload for the specified tag
 */
#if !XNU_KERNEL_PRIVATE
IMG4_API_AVAILABLE_20180112
OS_EXPORT OS_WARN_RESULT OS_NONNULL1 OS_NONNULL4
errno_t
img4_payload_init(img4_payload_t *i4p, img4_tag_t tag,
		img4_payload_flags_t flags, const uint8_t *bytes, size_t len,
		img4_destructor_t destructor);
#else
#define img4_payload_init(...) img4if->i4if_payload_init(__VA_ARGS__)
#endif

/*!
 * @function img4_payload_init_with_vnode_4xnu
 * Initializes an Image4 payload object from a vnode.
 *
 * @param i4p
 * A pointer to the payload object to initialize.
 *
 * @param tag
 * The expected tag for the payload.
 *
 * @param vn
 * The vnode from which to initialize the payload.
 *
 * @param flags
 * Flags modifying the behavior of the payload object.
 *
 * @result
 * Upon success, zero is returned. Otherwise, one of the following error codes:
 *
 *     [ENOENT]     The vnode is either dead or in the process of being
 *                  recycled
 *     [EIO]        Reading from the vnode stalled repeatedly beyond the
 *                  implementation's tolerance
 *
 * Additionally, the implementation may return any error that vnode_ref() may
 * return.
 *
 * @discussion
 * Verification of a vnode is performed by reading in chunks of data, updating
 * an ongoing hash operation with that data, and then discarding it. Therefore,
 * payload objects created in this manner can only guarantee their validity at
 * the time the check was performed since the vnode's contents are not kept in
 * memory and may be tampered with after validation has been performed.
 *
 * Additionally, this operation requires the payload to be unwrapped, as it does
 * not parse or recognize any Image4 payload wrapper. Payloads created with this
 * interface are therefore implicitly created with the {@link I4PLF_UNWRAPPED}
 * flag.
 */

#if KERNEL
#if !XNU_KERNEL_PRIVATE
IMG4_API_AVAILABLE_20180112
OS_EXPORT OS_WARN_RESULT OS_NONNULL1 OS_NONNULL3
errno_t
img4_payload_init_with_vnode_4xnu(img4_payload_t *i4p, img4_tag_t tag,
		vnode_t vn, img4_payload_flags_t flags);
#else
#define img4_payload_init_with_vnode_4xnu(...) \
		(img4if->i4if_v2.payload_init_with_vnode_4xnu(__VA_ARGS__))
#endif // !XNU_KERNEL_PRIVATE
#endif // KERNEL

/*!
 * @function img4_payload_init_with_fd_4MSM
 * Initializes an Image4 payload object from a file descriptor.
 *
 * @param i4p
 * A pointer to the payload object to initialize.
 *
 * @param tag
 * The expected tag for the payload.
 *
 * @param fd
 * The file descriptor from which to initialize the payload.
 *
 * @param flags
 * Flags modifying the behavior of the payload object.
 *
 * @result
 * Upon success, zero is returned. Otherwise, the implementation may return any
 * errno that is set by the dup(2) system call.
 *
 * @discussion
 * This interface is a userspace equivalent to
 * {@link img4_payload_init_with_vnode_4xnu}, and all the same caveats apply.
 */

#if !KERNEL
IMG4_API_AVAILABLE_20180112
OS_EXPORT OS_WARN_RESULT OS_NONNULL1
errno_t
img4_payload_init_with_fd_4MSM(img4_payload_t *i4p, img4_tag_t tag,
		int fd, img4_payload_flags_t flags);
#endif // KERNEL

/*!
 * @function img4_payload_destroy
 * Disposes of the resources associated with the payload object.
 *
 * @param i4p
 * The payload object of which to dispose.
 *
 * @discussion
 * This routine does not deallocate the storage for the payload object itself,
 * only the associated resources. This routine will cause the destructor given
 * in {@link img4_payload_init} to be called, if any.
 */
#if !XNU_KERNEL_PRIVATE
IMG4_API_AVAILABLE_20180112
OS_EXPORT OS_NONNULL1
void
img4_payload_destroy(img4_payload_t *i4p);
#else
#define img4_payload_destroy(...) img4if->i4if_payload_destroy(__VA_ARGS__)
#endif

#endif // __IMG4_PAYLOAD_H
