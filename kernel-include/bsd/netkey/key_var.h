/*	$KAME: key_var.h,v 1.5 2000/02/22 14:06:41 itojun Exp $	*/

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _NETKEY_KEY_VAR_H_
#define _NETKEY_KEY_VAR_H_
#include <sys/appleapiopts.h>

/* sysctl */
#define KEYCTL_DEBUG_LEVEL              1
#define KEYCTL_SPI_TRY                  2
#define KEYCTL_SPI_MIN_VALUE            3
#define KEYCTL_SPI_MAX_VALUE            4
#define KEYCTL_RANDOM_INT               5
#define KEYCTL_LARVAL_LIFETIME          6
#define KEYCTL_BLOCKACQ_COUNT           7
#define KEYCTL_BLOCKACQ_LIFETIME        8
#define KEYCTL_ESP_KEYMIN               9
#define KEYCTL_ESP_AUTH                 10
#define KEYCTL_AH_KEYMIN                11
#define KEYCTL_PREFERED_OLDSA           12
#define KEYCTL_NATT_KEEPALIVE_INTERVAL  13
#define KEYCTL_PFKEYSTAT                14
#define KEYCTL_MAXID                    15

#ifdef BSD_KERNEL_PRIVATE
#define KEYCTL_NAMES { \
	{ 0, 0 }, \
	{ "debug", CTLTYPE_INT }, \
	{ "spi_try", CTLTYPE_INT }, \
	{ "spi_min_value", CTLTYPE_INT }, \
	{ "spi_max_value", CTLTYPE_INT }, \
	{ "random_int", CTLTYPE_INT }, \
	{ "larval_lifetime", CTLTYPE_INT }, \
	{ "blockacq_count", CTLTYPE_INT }, \
	{ "blockacq_lifetime", CTLTYPE_INT }, \
	{ "esp_keymin", CTLTYPE_INT }, \
	{ "esp_auth", CTLTYPE_INT }, \
	{ "ah_keymin", CTLTYPE_INT }, \
	{ "prefered_oldsa", CTLTYPE_INT }, \
	{ "natt_keepalive_interval", CTLTYPE_INT }, \
	{ "pfkeystat", CTLTYPE_STRUCT }, \
}

#define KEYCTL_VARS { \
	0, \
	&key_debug_level, \
	&key_spi_trycnt, \
	&key_spi_minval, \
	&key_spi_maxval, \
	&key_int_random, \
	&key_larval_lifetime, \
	&key_blockacq_count, \
	&key_blockacq_lifetime, \
	&ipsec_esp_keymin, \
	&ipsec_ah_keymin, \
	&ipsec_prefered_oldsa, \
	&natt_keepalive_interval, \
}

#define _ARRAYLEN(p) (sizeof(p)/sizeof(p[0]))
#define _KEYLEN(key) ((u_int)((key)->sadb_key_bits >> 3))
#define _KEYBITS(key) ((u_int)((key)->sadb_key_bits))
#define _KEYBUF(key) ((caddr_t)((caddr_t)(key) + sizeof(struct sadb_key)))

#endif /* BSD_KERNEL_PRIVATE */
#endif /* _NETKEY_KEY_VAR_H_ */
