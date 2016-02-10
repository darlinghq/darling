/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
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

#ifndef	_NET_IF_UTUN_CRYPTO_H_
#define	_NET_IF_UTUN_CRYPTO_H_

// constants used in configuring the crypto context
typedef enum utun_crypto_ver {
	UTUN_CRYPTO_VER_1 = 1,
	UTUN_CRYPTO_VER_MAX,
} utun_crypto_ver_t;

#define UTUN_CRYPTO_KEYS_IPSEC_VER_1                  UTUN_CRYPTO_VER_1
#define UTUN_CRYPTO_IPSEC_VER_1                       UTUN_CRYPTO_VER_1

#define UTUN_CRYPTO_ARGS_VER_MAX                      UTUN_CRYPTO_VER_MAX
#define UTUN_CRYPTO_KEYS_ARGS_VER_MAX                 UTUN_CRYPTO_VER_MAX

typedef enum utun_crypto_dir {
	UTUN_CRYPTO_DIR_IN = 1,
	UTUN_CRYPTO_DIR_OUT,
	UTUN_CRYPTO_DIR_MAX,
} utun_crypto_dir_t;

#define UTUN_CRYPTO_CTX_NUM_DIRS 2

#define BITSTOBYTES(n)                                (n >> 3)
#define BYTESTOBITS(n)                                (n << 3)

#define MAX_KEY_AUTH_LEN_BITS                         512 // corresponds to SHA512
#define MAX_KEY_AUTH_LEN_BYTES                        (BITSTOBYTES(MAX_KEY_AUTH_LEN_BITS))
#define MAX_KEY_ENC_LEN_BITS                          256 // corresponds to AES256
#define MAX_KEY_ENC_LEN_BYTES                         (BITSTOBYTES(MAX_KEY_ENC_LEN_BITS))

typedef enum utun_crypto_type {
	UTUN_CRYPTO_TYPE_IPSEC = 1,
	UTUN_CRYPTO_TYPE_DTLS,
	UTUN_CRYPTO_TYPE_MAX,
} utun_crypto_type_t;

typedef enum if_utun_crypto_ipsec_mode {
	IF_UTUN_CRYPTO_IPSEC_MODE_NONE = 0,
	IF_UTUN_CRYPTO_IPSEC_MODE_TRANSPORT,
	IF_UTUN_CRYPTO_IPSEC_MODE_TUNNEL,
	IF_UTUN_CRYPTO_IPSEC_MODE_MAX,
} if_utun_crypto_ipsec_mode_t;

typedef enum if_utun_crypto_ipsec_proto {
	IF_UTUN_CRYPTO_IPSEC_PROTO_NONE = 0,
	IF_UTUN_CRYPTO_IPSEC_PROTO_ESP,
	IF_UTUN_CRYPTO_IPSEC_PROTO_AH,
	IF_UTUN_CRYPTO_IPSEC_PROTO_MAX,
} if_utun_crypto_ipsec_proto_t;

typedef enum if_utun_crypto_ipsec_auth {
	IF_UTUN_CRYPTO_IPSEC_AUTH_NONE = 0,
	IF_UTUN_CRYPTO_IPSEC_AUTH_MD5,
	IF_UTUN_CRYPTO_IPSEC_AUTH_SHA1,
	IF_UTUN_CRYPTO_IPSEC_AUTH_SHA256,
	IF_UTUN_CRYPTO_IPSEC_AUTH_SHA384,
	IF_UTUN_CRYPTO_IPSEC_AUTH_SHA512,
	IF_UTUN_CRYPTO_IPSEC_AUTH_MAX,
} if_utun_crypto_ipsec_auth_t;

typedef enum if_utun_crypto_ipsec_enc {
	IF_UTUN_CRYPTO_IPSEC_ENC_NONE = 0,
	IF_UTUN_CRYPTO_IPSEC_ENC_DES,
	IF_UTUN_CRYPTO_IPSEC_ENC_3DES,
	IF_UTUN_CRYPTO_IPSEC_ENC_AES128,
	IF_UTUN_CRYPTO_IPSEC_ENC_AES256,
	IF_UTUN_CRYPTO_IPSEC_ENC_MAX,
} if_utun_crypto_ipsec_enc_t;

typedef enum if_utun_crypto_ipsec_keepalive {
	IF_UTUN_CRYPTO_IPSEC_KEEPALIVE_NONE = 0,
	IF_UTUN_CRYPTO_IPSEC_KEEPALIVE_NATT,
	IF_UTUN_CRYPTO_IPSEC_KEEPALIVE_ESP,
	IF_UTUN_CRYPTO_IPSEC_KEEPALIVE_MAX,
} if_utun_crypto_ipsec_keepalive_t;

typedef enum if_utun_crypto_ipsec_natd {
	IF_UTUN_CRYPTO_IPSEC_NATD_NONE = 0,
	IF_UTUN_CRYPTO_IPSEC_NATD_MINE,
	IF_UTUN_CRYPTO_IPSEC_NATD_PEER,
	IF_UTUN_CRYPTO_IPSEC_NATD_BOTH,
	IF_UTUN_CRYPTO_IPSEC_NATD_MAX,
} if_utun_crypto_ipsec_natd_t;

// structures used for storing the App's keying index arguments
typedef struct utun_crypto_keys_idx_ipsec_args_v1 {
	struct sockaddr_storage                       src_addr; // v4 or v6 socket address (ignore port numbers)
	struct sockaddr_storage                       dst_addr; // v4 or v6 socket address (ignore port numbers)
	if_utun_crypto_ipsec_proto_t                  proto;
	if_utun_crypto_ipsec_mode_t                   mode;
	u_int32_t                                     reqid; // policy's reqid, default to 0 for now since we are avoiding policies.
	u_int32_t                                     spi;		  // 0 when requesting the index, otherwise it contains the resulting index
	u_int32_t                                     spirange_min; // default to 0
	u_int32_t                                     spirange_max; // default to 0xffffffff
} __attribute__((packed)) utun_crypto_keys_idx_ipsec_args_v1_t;

typedef struct utun_crypto_keys_idx_dtls_args_v1 {
	// stub for DTLS keying index arguments
	u_int32_t                                     unused; // place holder
} __attribute__((packed)) utun_crypto_keys_idx_dtls_args_v1_t;

// App's parent structure for sending/storing keying index arguments
typedef struct utun_crypto_keys_idx_args {
	utun_crypto_ver_t                             ver;
	utun_crypto_type_t                            type;
	utun_crypto_dir_t                             dir;
	u_int32_t                                     args_ulen;
	u_int32_t                                     varargs_buflen;
	union {
		// don't change the order, number, or size of elements above this line (in this struct). otherwise UTUN_CRYPTO_CTX_IDX_ARGS_HDR_SIZE breaks backwards compatibility
		utun_crypto_keys_idx_ipsec_args_v1_t  ipsec_v1;
		utun_crypto_keys_idx_dtls_args_v1_t   dtls_v1;
		// future (additional) versions of the arguments may be placed here
	} u;
	u_int8_t                                      varargs_buf[0];
} __attribute__((aligned(4), packed)) utun_crypto_keys_idx_args_t;

// structures used for storing the App's keying material arguments
typedef struct utun_crypto_keys_ipsec_args_v1 {
	struct sockaddr_storage                       src_addr; // v4 or v6 socket address (ignore port numbers)
	struct sockaddr_storage                       dst_addr; // v4 or v6 socket address (ignore port numbers)
	if_utun_crypto_ipsec_proto_t                  proto;
	if_utun_crypto_ipsec_mode_t                   mode;
	if_utun_crypto_ipsec_auth_t                   alg_auth;
	if_utun_crypto_ipsec_enc_t                    alg_enc;
	if_utun_crypto_ipsec_keepalive_t              keepalive;
	if_utun_crypto_ipsec_natd_t                   natd;
	u_int8_t                                      replay;   // window size default to 4
	u_int8_t                                      punt_rx_keepalive;
	u_int16_t                                     interval_tx_keepalive;
	u_int16_t                                     key_auth_len; // 128 or 160 or 192 or 256 or 384 or 512
	u_int16_t                                     key_enc_len;  // 64 or 128 or 192 or 256
	u_int16_t                                     natt_port; // if non-zero flags will be set to include SADB_X_EXT_NATT
	u_int16_t                                     unused;
	u_int32_t                                     seq;	  // default to 0
	u_int32_t                                     spi;
	u_int32_t                                     pid;      // vpnagent's process id
	u_int32_t                                     reqid; // policy's reqid, default to 0 for now since we are avoiding policies.
	u_int64_t                                     lifetime_hard; // value in seconds
	u_int64_t                                     lifetime_soft; // value in seconds
	// key_auth and key_enc will actually be stored in utun_crypto_KEYS_args_t.varargs_buf
} __attribute__((packed)) utun_crypto_keys_ipsec_args_v1_t;

typedef struct utun_crypto_ctx_dtls_mat_args_v1 {
	// stub for DTLS keying material arguments
	u_int32_t                                     unused; // place holder
} __attribute__((packed)) utun_crypto_keys_dtls_args_v1_t;

// App's parent structure for sending/storing keying material arguments
typedef struct utun_crypto_keys_args {
	utun_crypto_ver_t                             ver;
	utun_crypto_type_t                            type;
	utun_crypto_dir_t                             dir;
	u_int32_t                                     args_ulen;
	u_int32_t                                     varargs_buflen;
	union {
		// don't change the order, number, or size of elements above this line (in this struct). otherwise UTUN_CRYPTO_KEYS_ARGS_HDR_SIZE breaks backwards compatibility
		utun_crypto_keys_ipsec_args_v1_t      ipsec_v1;
		utun_crypto_keys_dtls_args_v1_t       dtls_v1;
		// future (additional) versions of the arguments may be placed here
	} u;
	u_int8_t                                      varargs_buf[0];
} __attribute__((aligned(4), packed)) utun_crypto_keys_args_t;

// structures used for storing the App's crypto arguments
typedef struct utun_crypto_ipsec_args_v1 {
	// stub for IPSec crypto context arguments
	u_int32_t                                     unused; // place holder
} __attribute__((packed)) utun_crypto_ipsec_args_v1_t;

typedef struct utun_crypto_dtls_args_v1 {
	// stub for DTLS crypto context arguments
	u_int32_t                                     unused; // place holder
} __attribute__((packed)) utun_crypto_dtls_args_v1_t;

// App's parent structure for starting/stopping crypto
typedef struct utun_crypto_args {
	utun_crypto_ver_t                             ver;
	utun_crypto_type_t                            type;
	u_int32_t                                     stop_data_traffic;
	u_int32_t                                     args_ulen;
	u_int32_t                                     varargs_buflen;
	union {
		// don't change the order, number, or size of elements above this line (in this struct). otherwise UTUN_CRYPTO_ARGS_HDR_SIZE breaks backwards compatibility
		utun_crypto_ipsec_args_v1_t           ipsec_v1;
		utun_crypto_dtls_args_v1_t            dtls_v1;
		// future (additional) versions of the arguments may be placed here
	} u;
	u_int8_t                                      varargs_buf[0]; // must be at the end of this struct
} __attribute__((aligned(4), packed)) utun_crypto_args_t;


#endif // _NET_IF_UTUN_CRYPTO_H_
