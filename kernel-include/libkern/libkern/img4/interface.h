/*
 * Copyright (c) 2019 Apple Inc. All rights reserved.
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

/*!
 * @header
 * Interfaces to register the AppleImage4 interface with xnu-proper to avoid a
 * build-time layering inversion.
 */
#ifndef __IMG4_INTERFACE_H
#define __IMG4_INTERFACE_H

#include <os/base.h>
#include <sys/cdefs.h>

/*
 * We rely on img4.h's logic for either including sys/types.h or declaring
 * errno_t ourselves. So when building the kernel, include img4.h from our
 * external headers. Avoid this inclusion if we're building AppleImage4, which
 * will have included its own internal version of the header.
 */
#if MACH_KERNEL_PRIVATE || !_DARWIN_BUILDING_PROJECT_APPLEIMAGE4
#include <img4/img4.h>
#endif

/*!
 * @const IMG4_INTERFACE_VERSION
 * The version of the interface supported by the implementation. As new
 * functions are added to the interface, this value will be incremented so that
 * it can be tested at build-time and not require rev-locked submissions of xnu
 * and AppleImage4.
 */
#define IMG4_INTERFACE_VERSION (4u)

/*!
 * @typedef img4_init_t
 * A type describing a pointer to the {@link img4_init} function.
 */
typedef errno_t (*const img4_init_t)(
	img4_t *i4,
	img4_flags_t flags,
	const uint8_t *bytes,
	size_t len,
	img4_destructor_t destructor
	);

/*!
 * @typedef img4_get_trusted_payload_t
 * A type describing a pointer to the {@link img4_get_trusted_payload} function.
 */
typedef errno_t (*const img4_get_trusted_payload_t)(
	img4_t *i4,
	img4_tag_t tag,
	const img4_environment_t *env,
	const uint8_t **bytes,
	size_t *len
	);

/*!
 * @typedef img4_get_trusted_external_payload_t
 * A type describing a pointer to the {@link img4_get_trusted_external_payload}
 * function.
 */
typedef errno_t (*const img4_get_trusted_external_payload_t)(
	img4_t *img4,
	img4_payload_t *payload,
	const img4_environment_t *env,
	const uint8_t **bytes,
	size_t *len
	);

/*!
 * @typedef img4_set_nonce_t
 * A type describing a pointer to the {@link img4_set_nonce} function.
 */
typedef void (*const img4_set_nonce_t)(img4_t *i4,
    const void *bytes,
    size_t len
    );

/*!
 * @typedef img4_destroy_t
 * A type describing the {@link img4_destroy} function.
 */
typedef void (*const img4_destroy_t)(
	img4_t *i4
	);

/*!
 * @typedef img4_payload_init_t
 * A type describing the {@link img4_payload_init} function.
 */
typedef errno_t (*const img4_payload_init_t)(
	img4_payload_t *i4p,
	img4_tag_t tag,
	img4_payload_flags_t flags,
	const uint8_t *bytes,
	size_t len,
	img4_destructor_t destructor
	);

/*!
 * @typedef img4_payload_destroy_t
 * A type describing the {@link img4_payload_destroy} function.
 */
typedef void (*const img4_payload_destroy_t)(
	img4_payload_t *i4
	);

/*!
 * @typedef img4_payload_destroy_t
 * A type describing the {@link img4_set_nonce_domain} function.
 */
typedef void (*const img4_set_nonce_domain_t)(
	img4_t *i4,
	const img4_nonce_domain_t *nd
	);

/*!
 * @typedef img4_nonce_domain_copy_nonce_t
 * A type describing the {@link img4_nonce_domain_copy_nonce} function.
 */
typedef errno_t (*const img4_nonce_domain_copy_nonce_t)(
	const img4_nonce_domain_t *nd,
	img4_nonce_t *n
	);

/*!
 * @typedef img4_nonce_domain_roll_nonce_t
 * A type describing the {@link img4_nonce_domain_roll_nonce} function.
 */
typedef errno_t (*const img4_nonce_domain_roll_nonce_t)(
	const img4_nonce_domain_t *nd
	);

/*!
 * @typedef img4_payload_init_with_vnode_4xnu_t
 * A type describing the {@link img4_payload_init_with_vnode_4xnu} function.
 */
typedef errno_t (*const img4_payload_init_with_vnode_4xnu_t)(
	img4_payload_t *i4p,
	img4_tag_t tag,
	vnode_t vn,
	img4_payload_flags_t flags
	);

/*!
 * @typedef img4_environment_init_identity_t
 * A type describing the {@link img4_environment_init_identity} function.
 */
typedef errno_t (*const img4_environment_init_identity_t)(
	img4_environment_t *i4e,
	size_t len,
	const img4_identity_t *i4id
	);

/*!
 * @typedef img4_interface_t
 * A structure describing the interface to the AppleImage4 kext.
 *
 * @field i4if_version
 * The version of the structure supported by the implementation.
 *
 * @field i4if_init
 * A pointer to the {@link img4_init} function.
 *
 * @field i4if_get_trusted_payload
 * A pointer to the {@link img4_get_trusted_payload} function.
 *
 * @field i4if_get_trusted_external_payload
 * A pointer to the {@link img4_get_trusted_external_payload} function.
 *
 * @field i4if_destroy
 * A pointer to the {@link img4_destroy} function.
 *
 * @field i4if_payload_init
 * A pointer to the {@link img4_payload_init} function.
 *
 * @field i4if_destroy
 * A pointer to the {@link img4_payload_destroy} function.
 *
 * @field i4if_environment_platform
 * The {@link IMG4_ENVIRONMENT_PLATFORM} global.
 *
 * @field i4if_environment_reserved
 * Reserved for use by the implementation.
 *
 * @field i4if_environment_trust_cache
 * The {@link IMG4_ENVIRONMENT_TRUST_CACHE} global.
 *
 * @field i4if_v1
 * All fields added in version 1 of the structure.
 *
 * @field i4if_v1.set_nonce_domain
 * A pointer to the @{link img4_set_nonce_domain} function.
 *
 * @field i4if_v1.nonce_domain_copy_nonce
 * A pointer to the {@link img4_nonce_domain_copy_nonce} function.
 *
 * @field i4if_v1.nonce_domain_roll_nonce
 * A pointer to the {@link img4_nonce_domain_roll_nonce} function.
 *
 * @field i4if_v1.nonce_domain_trust_cache
 * The {@link IMG4_NONCE_DOMAIN_TRUST_CACHE} global.
 *
 * @field i4if_v2
 * All fields added in version 2 of the structure.
 *
 * @field i4if_v2.payload_init_with_vnode_4xnu
 * A pointer to the {@link img4_payload_init_with_vnode_4xnu} function.
 *
 * @field i4if_v3
 * All fields added in version 3 of the structure.
 *
 * @field i4if_v3.nonce_domain_pdi
 * The {@link IMG4_NONCE_DOMAIN_PDI} global.
 *
 * @field i4if_v3.nonce_domain_cryptex
 * The {@link IMG4_NONCE_DOMAIN_CRYPTEX} global.
 *
 * @field i4if_v4.environment_init_identity
 * A pointer to the {@link img4_environment_init_identity} function.
 */

typedef struct _img4_interface {
	const uint32_t i4if_version;
	img4_init_t i4if_init;
	img4_set_nonce_t i4if_set_nonce;
	img4_get_trusted_payload_t i4if_get_trusted_payload;
	img4_get_trusted_external_payload_t i4if_get_trusted_external_payload;
	img4_destroy_t i4if_destroy;
	img4_payload_init_t i4if_payload_init;
	img4_payload_destroy_t i4if_payload_destroy;
	const img4_environment_t *i4if_environment_platform;
	const img4_environment_t *i4if_environment_reserved;
	const img4_environment_t *i4if_environment_trust_cache;
	struct {
		img4_set_nonce_domain_t set_nonce_domain;
		img4_nonce_domain_copy_nonce_t nonce_domain_copy_nonce;
		img4_nonce_domain_roll_nonce_t nonce_domain_roll_nonce;
		const img4_nonce_domain_t *nonce_domain_trust_cache;
	} i4if_v1;
	struct {
		img4_payload_init_with_vnode_4xnu_t payload_init_with_vnode_4xnu;
	} i4if_v2;
	struct {
		const img4_nonce_domain_t *nonce_domain_pdi;
		const img4_nonce_domain_t *nonce_domain_cryptex;
	} i4if_v3;
	struct {
		const img4_environment_init_identity_t environment_init_identity;
	} i4if_v4;
	void *__reserved[14];
} img4_interface_t;

__BEGIN_DECLS

/*!
 * @const img4if
 * The AppleImage4 interface that was registered.
 */
extern const img4_interface_t *img4if;

/*!
 * @function img4_interface_register
 * Registers the AppleImage4 kext interface for use within the kernel proper.
 *
 * @param i4
 * The interface to register.
 *
 * @discussion
 * This routine may only be called once and must be called before late-const has
 * been applied to kernel memory.
 */
OS_EXPORT OS_NONNULL1
void
img4_interface_register(const img4_interface_t *i4);

__END_DECLS

#endif // __IMG4_INTERFACE_H
