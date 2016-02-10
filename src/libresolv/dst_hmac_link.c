#ifdef HMAC_MD5
#ifndef __APPLE__
#ifndef LINT
static const char rcsid[] = "$Header: /Users/Shared/libresolv_2/libresolv/dst_hmac_link.c,v 1.1 2006/03/01 19:01:36 majka Exp $";
#endif
#endif

/*
 * Portions Copyright (c) 1995-1998 by Trusted Information Systems, Inc.
 *
 * Permission to use, copy modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND TRUSTED INFORMATION SYSTEMS
 * DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL
 * TRUSTED INFORMATION SYSTEMS BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THE SOFTWARE.
 */

/* 
 * This file contains an implementation of the HMAC-MD5 algorithm.
 */
#ifndef __APPLE__
#include "port_before.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <sys/param.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

#include "dst_internal.h"
#ifdef USE_MD5
# include "md5.h"
# ifndef _MD5_H_
#  define _MD5_H_ 1	/* make sure we do not include rsaref md5.h file */
# endif
#endif

#ifndef __APPLE__
#include "port_after.h"
#endif


#define HMAC_LEN	64
#define HMAC_IPAD	0x36
#define HMAC_OPAD	0x5c
#define MD5_LEN		16


typedef struct hmackey {
	u_char hk_ipad[64], hk_opad[64];
} HMAC_Key;


/************************************************************************** 
 * dst_hmac_md5_sign
 *     Call HMAC signing functions to sign a block of data.
 *     There are three steps to signing, INIT (initialize structures), 
 *     UPDATE (hash (more) data), FINAL (generate a signature).  This
 *     routine performs one or more of these steps.
 * Parameters
 *     mode	SIG_MODE_INIT, SIG_MODE_UPDATE and/or SIG_MODE_FINAL.
 *     priv_key    key to use for signing.
 *     context   the context to be used in this digest
 *     data	data to be signed.
 *     len	 length in bytes of data.
 *     signature   location to store signature.
 *     sig_len     size of the signature location
 * returns 
 *	N  Success on SIG_MODE_FINAL = returns signature length in bytes
 *	0  Success on SIG_MODE_INIT  and UPDATE
 *	 <0  Failure
 */

static int
dst_hmac_md5_sign(const int mode, DST_KEY *d_key, void **context, 
		  const u_char *data, const int len, 
		  u_char *signature, const int sig_len)
{
	HMAC_Key *key;
	int sign_len = 0;
	MD5_CTX *ctx = NULL;

	if (mode & SIG_MODE_INIT) 
		ctx = (MD5_CTX *) malloc(sizeof(*ctx));
	else if (context)
		ctx = (MD5_CTX *) *context;
	if (ctx == NULL) 
		return (-1);

	if (d_key == NULL || d_key->dk_KEY_struct == NULL)
		return (-1);
	key = (HMAC_Key *) d_key->dk_KEY_struct;

	if (mode & SIG_MODE_INIT) {
		MD5Init(ctx);
		MD5Update(ctx, key->hk_ipad, HMAC_LEN);
	}

	if ((mode & SIG_MODE_UPDATE) && (data && len > 0))
		MD5Update(ctx, data, len);

	if (mode & SIG_MODE_FINAL) {
		if (signature == NULL || sig_len < MD5_LEN)
			return (SIGN_FINAL_FAILURE);
		MD5Final(signature, ctx);

		/* perform outer MD5 */
		MD5Init(ctx);
		MD5Update(ctx, key->hk_opad, HMAC_LEN);
		MD5Update(ctx, signature, MD5_LEN);
		MD5Final(signature, ctx);
		sign_len = MD5_LEN;
		SAFE_FREE(ctx);
	}
	else { 
		if (context == NULL) 
			return (-1);
		*context = (void *) ctx;
	}		
	return (sign_len);
}


/************************************************************************** 
 * dst_hmac_md5_verify() 
 *     Calls HMAC verification routines.  There are three steps to 
 *     verification, INIT (initialize structures), UPDATE (hash (more) data), 
 *     FINAL (generate a signature).  This routine performs one or more of 
 *     these steps.
 * Parameters
 *     mode	SIG_MODE_INIT, SIG_MODE_UPDATE and/or SIG_MODE_FINAL.
 *     dkey	key to use for verify.
 *     data	data signed.
 *     len	 length in bytes of data.
 *     signature   signature.
 *     sig_len     length in bytes of signature.
 * returns 
 *     0  Success 
 *    <0  Failure
 */

static int
dst_hmac_md5_verify(const int mode, DST_KEY *d_key, void **context,
		const u_char *data, const int len,
		const u_char *signature, const int sig_len)
{
	HMAC_Key *key;
	MD5_CTX *ctx = NULL;

	if (mode & SIG_MODE_INIT) 
		ctx = (MD5_CTX *) malloc(sizeof(*ctx));
	else if (context)
		ctx = (MD5_CTX *) *context;
	if (ctx == NULL) 
		return (-1);

	if (d_key == NULL || d_key->dk_KEY_struct == NULL)
		return (-1);

	key = (HMAC_Key *) d_key->dk_KEY_struct;
	if (mode & SIG_MODE_INIT) {
		MD5Init(ctx);
		MD5Update(ctx, key->hk_ipad, HMAC_LEN);
	}
	if ((mode & SIG_MODE_UPDATE) && (data && len > 0))
		MD5Update(ctx, data, len);

	if (mode & SIG_MODE_FINAL) {
		u_char digest[MD5_LEN];
		if (signature == NULL || key == NULL || sig_len != MD5_LEN)
			return (VERIFY_FINAL_FAILURE);
		MD5Final(digest, ctx);

		/* perform outer MD5 */
		MD5Init(ctx);
		MD5Update(ctx, key->hk_opad, HMAC_LEN);
		MD5Update(ctx, digest, MD5_LEN);
		MD5Final(digest, ctx);

		SAFE_FREE(ctx);
		if (memcmp(digest, signature, MD5_LEN) != 0)
			return (VERIFY_FINAL_FAILURE);
	}
	else { 
		if (context == NULL) 
			return (-1);
		*context = (void *) ctx;
	}		
	return (0);
}


/************************************************************************** 
 * dst_buffer_to_hmac_md5
 *     Converts key from raw data to an HMAC Key
 *     This function gets in a pointer to the data
 * Parameters
 *     hkey	the HMAC key to be filled in
 *     key	the key in raw format
 *     keylen	the length of the key
 * Return
 *	0	Success
 *	<0	Failure
 */
static int
dst_buffer_to_hmac_md5(DST_KEY *dkey, const u_char *key, const int keylen)
{
	int i;
	HMAC_Key *hkey = NULL;
	MD5_CTX ctx;
	int local_keylen = keylen;

	if (dkey == NULL || key == NULL || keylen < 0)
		return (-1);

	if ((hkey = (HMAC_Key *) malloc(sizeof(HMAC_Key))) == NULL)
		  return (-2);

	memset(hkey->hk_ipad, 0, sizeof(hkey->hk_ipad));
	memset(hkey->hk_opad, 0, sizeof(hkey->hk_opad));

	/* if key is longer than HMAC_LEN bytes reset it to key=MD5(key) */
	if (keylen > HMAC_LEN) {
		u_char tk[MD5_LEN];
		MD5Init(&ctx);
		MD5Update(&ctx, key, keylen);
		MD5Final(tk, &ctx);
		memset((void *) &ctx, 0, sizeof(ctx));
		key = tk;
		local_keylen = MD5_LEN;
	}
	/* start out by storing key in pads */
	memcpy(hkey->hk_ipad, key, local_keylen);
	memcpy(hkey->hk_opad, key, local_keylen);

	/* XOR key with hk_ipad and opad values */
	for (i = 0; i < HMAC_LEN; i++) {
		hkey->hk_ipad[i] ^= HMAC_IPAD;
		hkey->hk_opad[i] ^= HMAC_OPAD;
	}
	dkey->dk_key_size = local_keylen;
	dkey->dk_KEY_struct = (void *) hkey;
	return (1);
}


/************************************************************************** 
 *  dst_hmac_md5_key_to_file_format
 *	Encodes an HMAC Key into the portable file format.
 *  Parameters 
 *	hkey      HMAC KEY structure 
 *	buff      output buffer
 *	buff_len  size of output buffer 
 *  Return
 *	0  Failure - null input hkey
 *     -1  Failure - not enough space in output area
 *	N  Success - Length of data returned in buff
 */

static int
dst_hmac_md5_key_to_file_format(const DST_KEY *dkey, char *buff,
			    const int buff_len)
{
	char *bp;
	int len, b_len, i, key_len;
	u_char key[HMAC_LEN];
	HMAC_Key *hkey;

	if (dkey == NULL || dkey->dk_KEY_struct == NULL) 
		return (0);
	if (buff == NULL || buff_len <= (int) strlen(key_file_fmt_str))
		return (-1);	/* no OR not enough space in output area */

	hkey = (HMAC_Key *) dkey->dk_KEY_struct;
	memset(buff, 0, buff_len);	/* just in case */
	/* write file header */
	sprintf(buff, key_file_fmt_str, KEY_FILE_FORMAT, KEY_HMAC_MD5, "HMAC");

	bp = (char *) strchr(buff, '\0');
	b_len = buff_len - (bp - buff);

	memset(key, 0, HMAC_LEN);
	for (i = 0; i < HMAC_LEN; i++)
		key[i] = hkey->hk_ipad[i] ^ HMAC_IPAD;
	for (i = HMAC_LEN - 1; i >= 0; i--)
		if (key[i] != 0)
			break;
	key_len = i + 1;

	strcat(bp, "Key: ");
	bp += strlen("Key: ");
	b_len = buff_len - (bp - buff);

	len = b64_ntop(key, key_len, bp, b_len);
	if (len < 0) 
		return (-1);
	bp += len;
	*(bp++) = '\n';
	*bp = '\0';
	b_len = buff_len - (bp - buff);

	return (buff_len - b_len);
}


/************************************************************************** 
 * dst_hmac_md5_key_from_file_format
 *     Converts contents of a key file into an HMAC key. 
 * Parameters 
 *     hkey    structure to put key into 
 *     buff       buffer containing the encoded key 
 *     buff_len   the length of the buffer
 * Return
 *     n >= 0 Foot print of the key converted 
 *     n <  0 Error in conversion 
 */

static int
dst_hmac_md5_key_from_file_format(DST_KEY *dkey, const char *buff,
			      const int buff_len)
{
	const char *p = buff, *eol;
	u_char key[HMAC_LEN+1];	/* b64_pton needs more than 64 bytes do decode
							 * it should probably be fixed rather than doing
							 * this
							 */
	u_char *tmp;
	int key_len, len;

	if (dkey == NULL)
		return (-2);
	if (buff == NULL || buff_len < 0)
		return (-1);

	memset(key, 0, sizeof(key));

	if (!dst_s_verify_str(&p, "Key: "))
		return (-3);

	eol = strchr(p, '\n');
	if (eol == NULL)
		return (-4);
	len = eol - p;
	tmp = malloc(len + 2);
	memcpy(tmp, p, len);
	*(tmp + len) = 0x0;
	key_len = b64_pton((char *)tmp, key, HMAC_LEN+1);	/* see above */
	SAFE_FREE2(tmp, len + 2);

	if (dst_buffer_to_hmac_md5(dkey, key, key_len) < 0) {
		return (-6);
	}
	return (0);
}

/*
 * dst_hmac_md5_to_dns_key() 
 *         function to extract hmac key from DST_KEY structure 
 * intput: 
 *      in_key:  HMAC-MD5 key 
 * output: 
 *	out_str: buffer to write ot
 *      out_len: size of output buffer 
 * returns:
 *      number of bytes written to output buffer 
 */
static int
dst_hmac_md5_to_dns_key(const DST_KEY *in_key, u_char *out_str,
			const int out_len)
{

	HMAC_Key *hkey;
	int i;
	
	if (in_key == NULL || in_key->dk_KEY_struct == NULL ||
	    out_len <= in_key->dk_key_size || out_str == NULL)
		return (-1);

	hkey = (HMAC_Key *) in_key->dk_KEY_struct;
	for (i = 0; i < in_key->dk_key_size; i++)
		out_str[i] = hkey->hk_ipad[i] ^ HMAC_IPAD;
	return (i);
}

/************************************************************************** 
 *  dst_hmac_md5_compare_keys
 *	Compare two keys for equality.
 *  Return
 *	0	  The keys are equal
 *	NON-ZERO   The keys are not equal
 */

static int
dst_hmac_md5_compare_keys(const DST_KEY *key1, const DST_KEY *key2)
{
	HMAC_Key *hkey1 = (HMAC_Key *) key1->dk_KEY_struct;
	HMAC_Key *hkey2 = (HMAC_Key *) key2->dk_KEY_struct;
	return memcmp(hkey1->hk_ipad, hkey2->hk_ipad, HMAC_LEN);
}

/************************************************************************** 
 * dst_hmac_md5_free_key_structure
 *     Frees all (none) dynamically allocated structures in hkey
 */

static void *
dst_hmac_md5_free_key_structure(void *key)
{
	HMAC_Key *hkey = key;
	SAFE_FREE(hkey);
	return (NULL);
}


/*************************************************************************** 
 * dst_hmac_md5_generate_key
 *     Creates a HMAC key of size size with a maximum size of 63 bytes
 *     generating a HMAC key larger than 63 bytes makes no sense as that key 
 *     is digested before use. 
 */

static int
dst_hmac_md5_generate_key(DST_KEY *key, const int nothing)
{
	(void)key;
	(void)nothing;
	return (-1);
}

/*
 * dst_hmac_md5_init()  Function to answer set up function pointers for HMAC
 *	   related functions 
 */
int
dst_hmac_md5_init()
{
	if (dst_t_func[KEY_HMAC_MD5] != NULL)
		return (1);
	dst_t_func[KEY_HMAC_MD5] = malloc(sizeof(struct dst_func));
	if (dst_t_func[KEY_HMAC_MD5] == NULL)
		return (0);
	memset(dst_t_func[KEY_HMAC_MD5], 0, sizeof(struct dst_func));
	dst_t_func[KEY_HMAC_MD5]->sign = dst_hmac_md5_sign;
	dst_t_func[KEY_HMAC_MD5]->verify = dst_hmac_md5_verify;
	dst_t_func[KEY_HMAC_MD5]->compare = dst_hmac_md5_compare_keys;
	dst_t_func[KEY_HMAC_MD5]->generate = dst_hmac_md5_generate_key;
	dst_t_func[KEY_HMAC_MD5]->destroy = dst_hmac_md5_free_key_structure;
	dst_t_func[KEY_HMAC_MD5]->to_dns_key = dst_hmac_md5_to_dns_key;
	dst_t_func[KEY_HMAC_MD5]->from_dns_key = dst_buffer_to_hmac_md5;
	dst_t_func[KEY_HMAC_MD5]->to_file_fmt = dst_hmac_md5_key_to_file_format;
	dst_t_func[KEY_HMAC_MD5]->from_file_fmt = dst_hmac_md5_key_from_file_format;
	return (1);
}

#else 
#define dst_hmac_md5_init res_9_dst_hmac_md5_init
int
dst_hmac_md5_init(){
	return (0);
}
#endif







