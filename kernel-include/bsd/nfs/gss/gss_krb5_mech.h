/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

#include <corecrypto/ccdigest.h>
#include <corecrypto/cchmac.h>
#include <corecrypto/ccsha1.h>
#include <corecrypto/ccdes.h>
#include <corecrypto/ccaes.h>
#include <corecrypto/ccpad.h>

/*
 * GSS-API things from gssapi.h
 */
/*
 * Copyright 1993 by OpenVision Technologies, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and
 * that both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of OpenVision not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission. OpenVision makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * OPENVISION DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL OPENVISION BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

typedef uint32_t OM_uint32;

#define GSS_S_COMPLETE                  0

/*
 * Some "helper" definitions to make the status code macros obvious.
 * From gssapi.h:
 */
#define GSS_C_CALLING_ERROR_OFFSET 24
#define GSS_C_ROUTINE_ERROR_OFFSET 16
#define GSS_C_SUPPLEMENTARY_OFFSET 0
#define GSS_C_CALLING_ERROR_MASK ((OM_uint32) 0377ul)
#define GSS_C_ROUTINE_ERROR_MASK ((OM_uint32) 0377ul)
#define GSS_C_SUPPLEMENTARY_MASK ((OM_uint32) 0177777ul)

/*
 * The macros that test status codes for error conditions.  Note that the
 * GSS_ERROR() macro has changed slightly from the V1 GSSAPI so that it now
 * evaluates its argument only once.
 */
#define GSS_CALLING_ERROR(x) \
	((x) & (GSS_C_CALLING_ERROR_MASK << GSS_C_CALLING_ERROR_OFFSET))
#define GSS_ROUTINE_ERROR(x) \
	((x) & (GSS_C_ROUTINE_ERROR_MASK << GSS_C_ROUTINE_ERROR_OFFSET))
#define GSS_SUPPLEMENTARY_INFO(x) \
	((x) & (GSS_C_SUPPLEMENTARY_MASK << GSS_C_SUPPLEMENTARY_OFFSET))
#define GSS_ERROR(x) \
	((x) & ((GSS_C_CALLING_ERROR_MASK << GSS_C_CALLING_ERROR_OFFSET) | \
	        (GSS_C_ROUTINE_ERROR_MASK << GSS_C_ROUTINE_ERROR_OFFSET)))

/*
 * Calling errors:
 */
#define GSS_S_CALL_INACCESSIBLE_READ \
	                     (((OM_uint32) 1ul) << GSS_C_CALLING_ERROR_OFFSET)
#define GSS_S_CALL_INACCESSIBLE_WRITE \
	                     (((OM_uint32) 2ul) << GSS_C_CALLING_ERROR_OFFSET)
#define GSS_S_CALL_BAD_STRUCTURE \
	                     (((OM_uint32) 3ul) << GSS_C_CALLING_ERROR_OFFSET)

/*
 * Routine errors:
 */
#define GSS_S_BAD_MECH (((OM_uint32) 1ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_BAD_NAME (((OM_uint32) 2ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_BAD_NAMETYPE (((OM_uint32) 3ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_BAD_BINDINGS (((OM_uint32) 4ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_BAD_STATUS (((OM_uint32) 5ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_BAD_SIG (((OM_uint32) 6ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_NO_CRED (((OM_uint32) 7ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_NO_CONTEXT (((OM_uint32) 8ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_DEFECTIVE_TOKEN (((OM_uint32) 9ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_DEFECTIVE_CREDENTIAL \
     (((OM_uint32) 10ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_CREDENTIALS_EXPIRED \
     (((OM_uint32) 11ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_CONTEXT_EXPIRED \
     (((OM_uint32) 12ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_FAILURE (((OM_uint32) 13ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_BAD_QOP (((OM_uint32) 14ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_UNAUTHORIZED (((OM_uint32) 15ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_UNAVAILABLE (((OM_uint32) 16ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_DUPLICATE_ELEMENT \
     (((OM_uint32) 17ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_NAME_NOT_MN \
     (((OM_uint32) 18ul) << GSS_C_ROUTINE_ERROR_OFFSET)

/*
 * Supplementary info bits:
 */
#define GSS_S_CONTINUE_NEEDED (1 << (GSS_C_SUPPLEMENTARY_OFFSET + 0))
#define GSS_S_DUPLICATE_TOKEN (1 << (GSS_C_SUPPLEMENTARY_OFFSET + 1))
#define GSS_S_OLD_TOKEN (1 << (GSS_C_SUPPLEMENTARY_OFFSET + 2))
#define GSS_S_UNSEQ_TOKEN (1 << (GSS_C_SUPPLEMENTARY_OFFSET + 3))
#define GSS_S_GAP_TOKEN (1 << (GSS_C_SUPPLEMENTARY_OFFSET + 4))

#define GSS_C_QOP_DEFAULT 0

/* end of gssapi.h */

/*
 * The following data structures are genenrated from lucid.x in the gssd project
 * and must be kept in sync with that project. This is a more memory efficient
 * representation of the gss_kerb5_lucid_context_v1_t defined in gssapi_krb5.h
 */
struct lucid_key {
	uint32_t etype;
	struct {
		uint32_t key_len;
		uint8_t *key_val;
	} key;
};
typedef struct lucid_key lucid_key;

struct key_data_1964 {
	uint32_t sign_alg;
	uint32_t seal_alg;
};
typedef struct key_data_1964 key_data_1964;

struct key_data_4121 {
	uint32_t acceptor_subkey;
};
typedef struct key_data_4121 key_data_4121;

struct lucid_protocol {
	uint32_t proto;
	union {
		key_data_1964 data_1964;
		key_data_4121 data_4121;
	} lucid_protocol_u;
};
typedef struct lucid_protocol lucid_protocol;

struct lucid_context {
	uint32_t vers;
	uint32_t initiate;
	uint32_t endtime;
	uint64_t send_seq;
	uint64_t recv_seq;
	lucid_protocol key_data;
	lucid_key ctx_key;
};
typedef struct lucid_context lucid_context;

/* end of lucid.x generated data structures */

typedef struct lucid_context *lucid_context_t;
/*
 * Mask for determining the returned structure version.
 * See example below for usage.
 */
typedef struct lucid_context_version {
	uint32_t        version;
	/* Structure version number */
} *lucid_context_version_t;

typedef enum etypes {
	DES3_CBC_SHA1_KD = 16,
	AES128_CTS_HMAC_SHA1_96 = 17,
	AES256_CTS_HMAC_SHA1_96 = 18,
} etypes;

#define KRB5_USAGE_ACCEPTOR_SEAL        22
#define KRB5_USAGE_ACCEPTOR_SIGN        23
#define KRB5_USAGE_INITIATOR_SEAL       24
#define KRB5_USAGE_INITIATOR_SIGN       25
#define KRB5_USAGE_LEN 5

#define GSS_SND 0
#define GSS_RCV 1
#define GSS_C_QOP_REVERSE 0x80000000    /* Pseudo QOP value to use as input to gss_krb5_unwrap to allow Sender to unwrap */

/*
 * Key schedule is the cbc state for encryption and decryption.
 * For DES3 we always use the session key from the lucid context,
 * and in that case Ekey and Ikey will point to the session key.
 */
struct key_schedule {
	cccbc_ctx *enc;
	cccbc_ctx *dec;
	void *ikey[2];          /* Drived integrity key (same length context key); */
};

/*
 * Crypto context that supports AES and DES3 etypes
 * All supported encryption types use hmac with SHA1
 * All are CBC encryption types
 * des3-cbc-sha1            --  7
 * des3-dbc-sha1-kd         -- 16 ???
 * aes128-cts-hmac-sha1-96  -- 17
 * aes256-cts-hmac-sha1-96  -- 18
 */

typedef struct crypto_ctx {
	uint32_t etype;
	uint32_t mpad;           /* Message padding */
	uint32_t flags;
	lck_mtx_t *lock;
	lucid_context_t gss_ctx;  /* Back pointer to lucid context */
	uint32_t keylen;
	void *key;   /* Points to session key from lucid context */
	const struct ccdigest_info *di;
	const struct ccmode_cbc *enc_mode;
	const struct ccmode_cbc *dec_mode;
	struct key_schedule ks;
	uint32_t digest_size;
	void *ckey[2];  /* Derived checksum key. Same as key for DES3 */
} *crypto_ctx_t;

#define CRYPTO_KS_ALLOCED       0x00001
#define CRYPTO_CTS_ENABLE       0x00002

typedef struct gss_ctx_id_desc {
	lucid_context  gss_lucid_ctx;
	struct crypto_ctx  gss_cryptor;
} *gss_ctx_id_t;

typedef struct gss_buffer_desc_struct {
	size_t length;
	void *value;
} gss_buffer_desc, *gss_buffer_t;

uint32_t
    gss_release_buffer(uint32_t *,    /* minor_status */
    gss_buffer_t);


/* Per message interfaces for kerberos gss mech in the kernel */

typedef uint32_t gss_qop_t;

uint32_t
    gss_krb5_get_mic_mbuf(uint32_t *,   /* minor_status */
    gss_ctx_id_t,                       /* context_handle */
    gss_qop_t,                          /* qop_req */
    mbuf_t,                             /* message mbuf */
    size_t,                             /* offest */
    size_t,                             /* length */
    gss_buffer_t                        /* message_token */
    );

uint32_t
    gss_krb5_get_mic(uint32_t *, /* minor_status */
    gss_ctx_id_t,               /* context_handle */
    gss_qop_t,                  /* qop_req */
    gss_buffer_t,               /* message buffer */
    gss_buffer_t                /* message_token */
    );

uint32_t
    gss_krb5_verify_mic(uint32_t *,     /* minor_status */
    gss_ctx_id_t,                       /* context_handle */
    gss_buffer_t,                       /* message_buffer */
    gss_buffer_t,                       /* message_token */
    gss_qop_t *                         /* qop_state */
    );

uint32_t
    gss_krb5_verify_mic_mbuf(uint32_t *,        /* minor_status */
    gss_ctx_id_t,                               /* context_handle */
    mbuf_t,                                     /* message_buffer */
    size_t,                                     /* offset */
    size_t,                                     /* length */
    gss_buffer_t,                               /* message_token */
    gss_qop_t *                                 /* qop_state */
    );

uint32_t
    gss_krb5_wrap_mbuf(uint32_t *,      /* minor_status */
    gss_ctx_id_t,                       /* context_handle */
    int,                                /* conf_req_flag */
    gss_qop_t,                          /* qop_req */
    mbuf_t *,                           /* input/output message_buffer */
    size_t,                             /* offset */
    size_t,                             /* length */
    int *                               /* conf_state */
    );

uint32_t
    gss_krb5_unwrap_mbuf(uint32_t *,    /* minor_status */
    gss_ctx_id_t,                       /* context_handle */
    mbuf_t *,                           /* input/output message_buffer */
    size_t,                             /* offset */
    size_t,                             /* length */
    int *,                              /* conf_state */
    gss_qop_t *                         /* qop state */
    );

void gss_krb5_destroy_context(gss_ctx_id_t);

gss_ctx_id_t gss_krb5_make_context(void *, uint32_t);

void gss_krb5_mech_init(void);

int corecrypto_available(void);

errno_t gss_normalize_mbuf(mbuf_t, size_t, size_t *, mbuf_t *, mbuf_t *, int);

mbuf_t gss_join_mbuf(mbuf_t, mbuf_t, mbuf_t);

typedef struct hmac_ctx_struct {
	size_t keylen;
	uint8_t *key;
	ccdigest_ctx_t di_ctx;
} hmac_ctx, hmac_ctx_t[1];

void hmac_init(const struct ccdigest_info *, hmac_ctx_t, size_t, void *);
void hmac_update(const struct ccdigest_info *, hmac_ctx_t, size_t, void *);
void hmac_final(const struct ccdigest_info *, hmac_ctx_t, uint8_t *);

void printmbuf(const char *, mbuf_t, uint32_t, uint32_t);

void printgbuf(const char *, gss_buffer_t);
