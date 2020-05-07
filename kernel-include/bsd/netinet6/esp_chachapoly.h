/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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

#include <sys/appleapiopts.h>

#ifdef BSD_KERNEL_PRIVATE

#ifndef _ESP_CHACHA_POLY_H_
#define _ESP_CHACHA_POLY_H_

#define ESP_CHACHAPOLY_PAD_BOUND                        1
#define ESP_CHACHAPOLY_IV_LEN                           8
#define ESP_CHACHAPOLY_ICV_LEN                          16
#define ESP_CHACHAPOLY_KEYBITS_WITH_SALT        288 /* 32 bytes key + 4 bytes salt */

int esp_chachapoly_schedlen(const struct esp_algorithm *);
int esp_chachapoly_schedule(const struct esp_algorithm *,
    struct secasvar *);
int esp_chachapoly_encrypt(struct mbuf *, size_t, size_t, struct secasvar *,
    const struct esp_algorithm *, int);
int esp_chachapoly_decrypt(struct mbuf *, size_t, struct secasvar *,
    const struct esp_algorithm *, int);
int esp_chachapoly_encrypt_finalize(struct secasvar *, unsigned char *, unsigned int);
int esp_chachapoly_decrypt_finalize(struct secasvar *, unsigned char *, unsigned int);
int esp_chachapoly_mature(struct secasvar *);
int esp_chachapoly_ivlen(const struct esp_algorithm *, struct secasvar *);

#endif /* _ESP_CHACHA_POLY_H_ */
#endif /* BSD_KERNEL_PRIVATE */
