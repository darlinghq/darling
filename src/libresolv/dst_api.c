#ifndef __APPLE__
#ifndef LINT
static const char rcsid[] = "$Header: /Users/Shared/libresolv_2/libresolv/dst_api.c,v 1.1 2006/03/01 19:01:36 majka Exp $";
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
 * This file contains the interface between the DST API and the crypto API.
 * This is the only file that needs to be changed if the crypto system is
 * changed.  Exported functions are:
 * void dst_init()	 Initialize the toolkit
 * int  dst_check_algorithm()   Function to determines if alg is suppored.
 * int  dst_compare_keys()      Function to compare two keys for equality.
 * int  dst_sign_data()         Incremental signing routine.
 * int  dst_verify_data()       Incremental verify routine.
 * int  dst_generate_key()      Function to generate new KEY
 * DST_KEY *dst_read_key()      Function to retrieve private/public KEY.
 * void dst_write_key()         Function to write out a key.
 * DST_KEY *dst_dnskey_to_key() Function to convert DNS KEY RR to a DST
 *				KEY structure.
 * int dst_key_to_dnskey() 	Function to return a public key in DNS 
 *				format binary
 * DST_KEY *dst_buffer_to_key() Converst a data in buffer to KEY
 * int *dst_key_to_buffer()	Writes out DST_KEY key matterial in buffer
 * void dst_free_key()       	Releases all memory referenced by key structure
 */

#ifndef __APPLE__
#include "port_before.h"
#endif

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>
#include <time.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

#include "dst_internal.h"
#ifndef __APPLE__
#include "port_after.h"
#endif

/* static variables */
static int done_init = 0;
dst_func *dst_t_func[DST_MAX_ALGS];
const char *key_file_fmt_str = "Private-key-format: v%s\nAlgorithm: %d (%s)\n";
const char *dst_path = "";

/* internal I/O functions */
#ifdef _UNUSED_API_
static DST_KEY *dst_s_read_public_key(const char *in_name, 
				      const u_int16_t in_id, int in_alg);
static int dst_s_read_private_key_file(char *name, DST_KEY *pk_key,
				       u_int16_t in_id, int in_alg);
static int dst_s_write_public_key(const DST_KEY *key);
static int dst_s_write_private_key(const DST_KEY *key);
#endif

/* internal function to set up data structure */
static DST_KEY *dst_s_get_key_struct(const char *name, const int alg,
				     const int flags, const int protocol,
				     const int bits);

/*
 *  dst_init
 *	This function initializes the Digital Signature Toolkit.
 *	Right now, it just checks the DSTKEYPATH environment variable.
 *  Parameters
 *	none
 *  Returns
 *	none
 */
void
dst_init()
{
	char *s;
	int len;

	if (done_init != 0)
		return;
	done_init = 1;

	s = getenv("DSTKEYPATH");
	len = 0;
	if (s) {
		struct stat statbuf;

		len = strlen(s);
		if (len > PATH_MAX) {
			EREPORT(("%s is longer than %d characters, ignoring\n",
				 s, PATH_MAX));
		} else if (stat(s, &statbuf) != 0 || !S_ISDIR(statbuf.st_mode)) {
			EREPORT(("%s is not a valid directory\n", s));
		} else {
			char *tmp;
			tmp = (char *) malloc(len + 2);
			memcpy(tmp, s, len + 1);
			if (tmp[strlen(tmp) - 1] != '/') {
				tmp[strlen(tmp) + 1] = 0;
				tmp[strlen(tmp)] = '/';
			}
			dst_path = tmp;
		}
	}
	memset(dst_t_func, 0, sizeof(dst_t_func));
	/* first one is selected */
	dst_hmac_md5_init();
}

/*
 *  dst_check_algorithm
 *	This function determines if the crypto system for the specified
 *	algorithm is present.
 *  Parameters
 *	alg     1       KEY_RSA
 *		3       KEY_DSA
 *	      157     KEY_HMAC_MD5
 *		      future algorithms TBD and registered with IANA.
 *  Returns
 *	1 - The algorithm is available.
 *	0 - The algorithm is not available.
 */
#ifdef __APPLE__
static
#endif
int
dst_check_algorithm(const int alg)
{
	return (dst_t_func[alg] != NULL);
}

/* 
 * dst_s_get_key_struct 
 *	This function allocates key structure and fills in some of the 
 *	fields of the structure. 
 * Parameters: 
 *	name:     the name of the key 
 *	alg:      the algorithm number 
 *	flags:    the dns flags of the key
 *	protocol: the dns protocol of the key
 *	bits:     the size of the key
 * Returns:
 *       NULL if error
 *       valid pointer otherwise
 */
static DST_KEY *
dst_s_get_key_struct(const char *name, const int alg, const int flags,
		     const int protocol, const int bits)
{
	DST_KEY *new_key = NULL; 

	if (dst_check_algorithm(alg)) /* make sure alg is available */
		new_key = (DST_KEY *) malloc(sizeof(*new_key));
	if (new_key == NULL)
		return (NULL);

	memset(new_key, 0, sizeof(*new_key));
	new_key->dk_key_name = strdup(name);
	new_key->dk_alg = alg;
	new_key->dk_flags = flags;
	new_key->dk_proto = protocol;
	new_key->dk_KEY_struct = NULL;
	new_key->dk_key_size = bits;
	new_key->dk_func = dst_t_func[alg];
	return (new_key);
}

#ifdef _UNUSED_API_
/*
 *  dst_compare_keys
 *	Compares two keys for equality.
 *  Parameters
 *	key1, key2      Two keys to be compared.
 *  Returns
 *	0	       The keys are equal.
 *	non-zero	The keys are not equal.
 */

int
dst_compare_keys(const DST_KEY *key1, const DST_KEY *key2)
{
	if (key1 == key2)
		return (0);
	if (key1 == NULL || key2 == NULL)
		return (4);
	if (key1->dk_alg != key2->dk_alg)
		return (1);
	if (key1->dk_key_size != key2->dk_key_size)
		return (2);
	if (key1->dk_id != key2->dk_id)
		return (3);
	return (key1->dk_func->compare(key1, key2));
}
#endif

/*
 * dst_sign_data
 *	An incremental signing function.  Data is signed in steps.
 *	First the context must be initialized (SIG_MODE_INIT).
 *	Then data is hashed (SIG_MODE_UPDATE).  Finally the signature
 *	itself is created (SIG_MODE_FINAL).  This function can be called
 *	once with INIT, UPDATE and FINAL modes all set, or it can be
 *	called separately with a different mode set for each step.  The
 *	UPDATE step can be repeated.
 * Parameters
 *	mode    A bit mask used to specify operation(s) to be performed.
 *		  SIG_MODE_INIT	   1   Initialize digest
 *		  SIG_MODE_UPDATE	 2   Add data to digest
 *		  SIG_MODE_FINAL	  4   Generate signature
 *					      from signature
 *		  SIG_MODE_ALL (SIG_MODE_INIT,SIG_MODE_UPDATE,SIG_MODE_FINAL
 *	data    Data to be signed.
 *	len     The length in bytes of data to be signed.
 *	in_key  Contains a private key to sign with.
 *		  KEY structures should be handled (created, converted,
 *		  compared, stored, freed) by the DST.
 *	signature
 *	      The location to which the signature will be written.
 *	sig_len Length of the signature field in bytes.
 * Return
 *	 0      Successfull INIT or Update operation
 *	>0      success FINAL (sign) operation
 *	<0      failure
 */

int
dst_sign_data(const int mode, DST_KEY *in_key, void **context, 
	      const u_char *data, const int len,
	      u_char *signature, const int sig_len)
{
	DUMP(data, mode, len, "dst_sign_data()");

	if (mode & SIG_MODE_FINAL &&
	    (in_key->dk_KEY_struct == NULL || signature == NULL))
		return (MISSING_KEY_OR_SIGNATURE);

	if (in_key->dk_func && in_key->dk_func->sign)
		return (in_key->dk_func->sign(mode, in_key, context, data, len,
					      signature, sig_len));
	return (UNKNOWN_KEYALG);
}


/*
 *  dst_verify_data
 *	An incremental verify function.  Data is verified in steps.
 *	First the context must be initialized (SIG_MODE_INIT).
 *	Then data is hashed (SIG_MODE_UPDATE).  Finally the signature
 *	is verified (SIG_MODE_FINAL).  This function can be called
 *	once with INIT, UPDATE and FINAL modes all set, or it can be
 *	called separately with a different mode set for each step.  The
 *	UPDATE step can be repeated.
 *  Parameters
 *	mode	Operations to perform this time.
 *		      SIG_MODE_INIT       1   Initialize digest
 *		      SIG_MODE_UPDATE     2   add data to digest
 *		      SIG_MODE_FINAL      4   verify signature
 *		      SIG_MODE_ALL
 *			  (SIG_MODE_INIT,SIG_MODE_UPDATE,SIG_MODE_FINAL)
 *	data	Data to pass through the hash function.
 *	len	 Length of the data in bytes.
 *	in_key      Key for verification.
 *	signature   Location of signature.
 *	sig_len     Length of the signature in bytes.
 *  Returns
 *	0	   Verify success
 *	Non-Zero    Verify Failure
 */

int
dst_verify_data(const int mode, DST_KEY *in_key, void **context, 
		const u_char *data, const int len,
		const u_char *signature, const int sig_len)
{
	DUMP(data, mode, len, "dst_verify_data()");
	if (mode & SIG_MODE_FINAL &&
	    (in_key->dk_KEY_struct == NULL || signature == NULL))
		return (MISSING_KEY_OR_SIGNATURE);

	if (in_key->dk_func == NULL || in_key->dk_func->verify == NULL)
		return (UNSUPPORTED_KEYALG);
	return (in_key->dk_func->verify(mode, in_key, context, data, len,
					signature, sig_len));
}

#ifdef _UNUSED_API_
/*
 *  dst_read_private_key
 *	Access a private key.  First the list of private keys that have
 *	already been read in is searched, then the key accessed on disk.
 *	If the private key can be found, it is returned.  If the key cannot
 *	be found, a null pointer is returned.  The options specify required
 *	key characteristics.  If the private key requested does not have
 *	these characteristics, it will not be read.
 *  Parameters
 *	in_keyname  The private key name.
 *	in_id	    The id of the private key.
 *	options     DST_FORCE_READ  Read from disk - don't use a previously
 *				      read key.
 *		  DST_CAN_SIGN    The key must be useable for signing.
 *		  DST_NO_AUTHEN   The key must be useable for authentication.
 *		  DST_STANDARD    Return any key 
 *  Returns
 *	NULL	If there is no key found in the current directory or
 *		      this key has not been loaded before.
 *	!NULL       Success - KEY structure returned.
 */

DST_KEY *
dst_read_key(const char *in_keyname, const u_int16_t in_id, 
	     const int in_alg, const int type)
{
	char keyname[PATH_MAX];
	DST_KEY *dg_key = NULL, *pubkey = NULL;

	if (!dst_check_algorithm(in_alg)) { /* make sure alg is available */
		EREPORT(("dst_read_private_key(): Algorithm %d not suppored\n",
			 in_alg));
		return (NULL);
	}
	if ((type & (DST_PUBLIC | DST_PRIVATE)) == 0) 
		return (NULL);
	if (in_keyname == NULL) {
		EREPORT(("dst_read_private_key(): Null key name passed in\n"));
		return (NULL);
	} else
		strcpy(keyname, in_keyname);

	/* before I read in the public key, check if it is allowed to sign */
	if ((pubkey = dst_s_read_public_key(keyname, in_id, in_alg)) == NULL)
		return (NULL);

	if (type == DST_PUBLIC) 
		return pubkey; 

	if (!(dg_key = dst_s_get_key_struct(keyname, pubkey->dk_alg,
					 pubkey->dk_flags, pubkey->dk_proto,
					    0)))
		return (dg_key);
	/* Fill in private key and some fields in the general key structure */
	if (dst_s_read_private_key_file(keyname, dg_key, pubkey->dk_id,
					pubkey->dk_alg) == 0)
		dg_key = dst_free_key(dg_key);

	pubkey = dst_free_key(pubkey);
	return (dg_key);
}
#endif

#ifdef _UNUSED_API_
int 
dst_write_key(const DST_KEY *key, const int type)
{
	int pub = 0, priv = 0;

	if (key == NULL) 
		return (0);
	if (!dst_check_algorithm(key->dk_alg)) { /* make sure alg is available */
		EREPORT(("dst_write_key(): Algorithm %d not suppored\n", 
			 key->dk_alg));
		return (UNSUPPORTED_KEYALG);
	}
	if ((type & (DST_PRIVATE|DST_PUBLIC)) == 0)
		return (0);

	if (type & DST_PUBLIC) 
		if ((pub = dst_s_write_public_key(key)) < 0)
			return (pub);
	if (type & DST_PRIVATE)
		if ((priv = dst_s_write_private_key(key)) < 0)
			return (priv);
	return (priv+pub);
}
#endif

#ifdef _UNUSED_API_
/*
 *  dst_write_private_key
 *	Write a private key to disk.  The filename will be of the form:
 *	K<key->dk_name>+<key->dk_alg>+<key->dk_id>.<private key suffix>.
 *	If there is already a file with this name, an error is returned.
 *
 *  Parameters
 *	key     A DST managed key structure that contains
 *	      all information needed about a key.
 *  Return
 *	>= 0    Correct behavior.  Returns length of encoded key value
 *		  written to disk.
 *	<  0    error.
 */

static int
dst_s_write_private_key(const DST_KEY *key)
{
	u_char encoded_block[RAW_KEY_SIZE];
	char file[PATH_MAX];
	int len;
	FILE *fp;

	/* First encode the key into the portable key format */
	if (key == NULL)
		return (-1);
	if (key->dk_KEY_struct == NULL)
		return (0);	/* null key has no private key */

	if (key->dk_func == NULL || key->dk_func->to_file_fmt == NULL) {
		EREPORT(("dst_write_private_key(): Unsupported operation %d\n",
			 key->dk_alg));
		return (-5);
	} else if ((len = key->dk_func->to_file_fmt(key, (char *)encoded_block,
					     sizeof(encoded_block))) <= 0) {
		EREPORT(("dst_write_private_key(): Failed encoding private RSA bsafe key %d\n", len));
		return (-8);
	}
	/* Now I can create the file I want to use */
	dst_s_build_filename(file, key->dk_key_name, key->dk_id, key->dk_alg,
			     PRIVATE_KEY, PATH_MAX);

	/* Do not overwrite an existing file */
	if ((fp = dst_s_fopen(file, "w", 0600)) != NULL) {
		int nn;
		if ((nn = fwrite(encoded_block, 1, len, fp)) != len) {
			EREPORT(("dst_write_private_key(): Write failure on %s %d != %d errno=%d\n",
				 file, len, nn, errno));
			return (-5);
		}
		fclose(fp);
	} else {
		EREPORT(("dst_write_private_key(): Can not create file %s\n"
			 ,file));
		return (-6);
	}
	memset(encoded_block, 0, len);
	return (len);
}
#endif

#ifdef _UNUSED_API_
/*
*
 *  dst_read_public_key
 *	Read a public key from disk and store in a DST key structure.
 *  Parameters
 *	in_name	 K<in_name><in_id>.<public key suffix> is the
 *		      filename of the key file to be read.
 *  Returns
 *	NULL	    If the key does not exist or no name is supplied.
 *	NON-NULL	Initialized key structure if the key exists.
 */

static DST_KEY *
dst_s_read_public_key(const char *in_name, const u_int16_t in_id, int in_alg)
{
	int flags, proto, alg, len, dlen;
	int c;
	char name[PATH_MAX], enckey[RAW_KEY_SIZE], *notspace;
	u_char deckey[RAW_KEY_SIZE];
	FILE *fp;

	if (in_name == NULL) {
		EREPORT(("dst_read_public_key(): No key name given\n"));
		return (NULL);
	}
	if (dst_s_build_filename(name, in_name, in_id, in_alg, PUBLIC_KEY,
				 PATH_MAX) == -1) {
		EREPORT(("dst_read_public_key(): Cannot make filename from %s, %d, and %s\n",
			 in_name, in_id, PUBLIC_KEY));
		return (NULL);
	}
	/*
	 * Open the file and read it's formatted contents up to key
	 * File format:
	 *    domain.name [ttl] [IN] KEY  <flags> <protocol> <algorithm> <key>
	 * flags, proto, alg stored as decimal (or hex numbers FIXME).
	 * (FIXME: handle parentheses for line continuation.)
	 */
	if ((fp = dst_s_fopen(name, "r", 0)) == NULL) {
		EREPORT(("dst_read_public_key(): Public Key not found %s\n",
			 name));
		return (NULL);
	}
	/* Skip domain name, which ends at first blank */
	while ((c = getc(fp)) != EOF)
		if (isspace(c))
			break;
	/* Skip blank to get to next field */
	while ((c = getc(fp)) != EOF)
		if (!isspace(c))
			break;

	/* Skip optional TTL -- if initial digit, skip whole word. */
	if (isdigit(c)) {
		while ((c = getc(fp)) != EOF)
			if (isspace(c))
				break;
		while ((c = getc(fp)) != EOF)
			if (!isspace(c))
				break;
	}
	/* Skip optional "IN" */
	if (c == 'I' || c == 'i') {
		while ((c = getc(fp)) != EOF)
			if (isspace(c))
				break;
		while ((c = getc(fp)) != EOF)
			if (!isspace(c))
				break;
	}
	/* Locate and skip "KEY" */
	if (c != 'K' && c != 'k') {
		EREPORT(("\"KEY\" doesn't appear in file: %s", name));
		return NULL;
	}
	while ((c = getc(fp)) != EOF)
		if (isspace(c))
			break;
	while ((c = getc(fp)) != EOF)
		if (!isspace(c))
			break;
	ungetc(c, fp);		/* return the charcter to the input field */
	/* Handle hex!! FIXME.  */

	if (fscanf(fp, "%d %d %d", &flags, &proto, &alg) != 3) {
		EREPORT(("dst_read_public_key(): Can not read flag/proto/alg field from %s\n"
			 ,name));
		return (NULL);
	}
	/* read in the key string */
	fgets(enckey, sizeof(enckey), fp);

	/* If we aren't at end-of-file, something is wrong.  */
	while ((c = getc(fp)) != EOF)
		if (!isspace(c))
			break;
	if (!feof(fp)) {
		EREPORT(("Key too long in file: %s", name));
		return NULL;
	}
	fclose(fp);

	if ((len = strlen(enckey)) <= 0)
		return (NULL);

	/* discard \n */
	enckey[--len] = '\0';

	/* remove leading spaces */
	for (notspace = (char *) enckey; isspace((*notspace)&0xff); len--)
		notspace++;

	dlen = b64_pton(notspace, deckey, sizeof(deckey));
	if (dlen < 0) {
		EREPORT(("dst_read_public_key: bad return from b64_pton = %d",
			 dlen));
		return (NULL);
	}
	/* store key and info in a key structure that is returned */
/*	return dst_store_public_key(in_name, alg, proto, 666, flags, deckey,
				    dlen);*/
	return dst_buffer_to_key(in_name, alg, flags, proto, deckey, dlen);
}
#endif

#ifdef _UNUSED_API_
/*
 *  dst_write_public_key
 *	Write a key to disk in DNS format.
 *  Parameters
 *	key     Pointer to a DST key structure.
 *  Returns
 *	0       Failure
 *	1       Success
 */

static int
dst_s_write_public_key(const DST_KEY *key)
{
	FILE *fp;
	char filename[PATH_MAX];
	u_char out_key[RAW_KEY_SIZE];
	char enc_key[RAW_KEY_SIZE];
	int len = 0;
	int mode;

	memset(out_key, 0, sizeof(out_key));
	if (key == NULL) {
		EREPORT(("dst_write_public_key(): No key specified \n"));
		return (0);
	} else if ((len = dst_key_to_dnskey(key, out_key, sizeof(out_key)))< 0)
		return (0);

	/* Make the filename */
	if (dst_s_build_filename(filename, key->dk_key_name, key->dk_id,
				 key->dk_alg, PUBLIC_KEY, PATH_MAX) == -1) {
		EREPORT(("dst_write_public_key(): Cannot make filename from %s, %d, and %s\n",
			 key->dk_key_name, key->dk_id, PUBLIC_KEY));
		return (0);
	}
	/* XXX in general this should be a check for symmetric keys */
	mode = (key->dk_alg == KEY_HMAC_MD5) ? 0600 : 0644;
	/* create public key file */
	if ((fp = dst_s_fopen(filename, "w+", mode)) == NULL) {
		EREPORT(("DST_write_public_key: open of file:%s failed (errno=%d)\n",
			 filename, errno));
		return (0);
	}
	/*write out key first base64 the key data */
	if (key->dk_flags & DST_EXTEND_FLAG)
		b64_ntop(&out_key[6], len - 6, enc_key, sizeof(enc_key));
	else
		b64_ntop(&out_key[4], len - 4, enc_key, sizeof(enc_key));
	fprintf(fp, "%s IN KEY %d %d %d %s\n",
		key->dk_key_name,
		key->dk_flags, key->dk_proto, key->dk_alg, enc_key);
	fclose(fp);
	return (1);
}
#endif

#ifdef _UNUSED_API_
/*
 *  dst_dnskey_to_public_key
 *	This function converts the contents of a DNS KEY RR into a DST
 *	key structure.
 *  Paramters
 *	len	 Length of the RDATA of the KEY RR RDATA
 *	rdata	 A pointer to the the KEY RR RDATA.
 *	in_name     Key name to be stored in key structure.
 *  Returns
 *	NULL	    Failure
 *	NON-NULL	Success.  Pointer to key structure.
 *			Caller's responsibility to free() it.
 */

DST_KEY *
dst_dnskey_to_key(const char *in_name, const u_char *rdata, const int len)
{
	DST_KEY *key_st;
	int alg ;
	int start = DST_KEY_START;

	if (rdata == NULL || len <= DST_KEY_ALG) /* no data */
		return (NULL);
	alg = (u_int8_t) rdata[DST_KEY_ALG];
	if (!dst_check_algorithm(alg)) { /* make sure alg is available */
		EREPORT(("dst_dnskey_to_key(): Algorithm %d not suppored\n",
			 alg));
		return (NULL);
	}
	if ((key_st = dst_s_get_key_struct(in_name, alg, 0, 0, 0)) == NULL)
		return (NULL);

	if (in_name == NULL)
		return (NULL);
	key_st->dk_id = dst_s_dns_key_id(rdata, len);
	key_st->dk_flags = dst_s_get_int16(rdata);
	key_st->dk_proto = (u_int16_t) rdata[DST_KEY_PROT];
	if (key_st->dk_flags & DST_EXTEND_FLAG) {
		u_int32_t ext_flags;
		ext_flags = (u_int32_t) dst_s_get_int16(&rdata[DST_EXT_FLAG]);
		key_st->dk_flags = key_st->dk_flags | (ext_flags << 16);
		start += 2;
	}
	/*
	 * now point to the begining of the data representing the encoding
	 * of the key
	 */
	if (key_st->dk_func && key_st->dk_func->from_dns_key) {
		if (key_st->dk_func->from_dns_key(key_st, &rdata[start],
						  len - start) > 0)
			return (key_st);
	} else
		EREPORT(("dst_dnskey_to_public_key(): unsuppored alg %d\n",
			 alg));

	SAFE_FREE(key_st);
	return (key_st);
}
#endif

/*
 *  dst_public_key_to_dnskey
 *	Function to encode a public key into DNS KEY wire format 
 *  Parameters
 *	key	     Key structure to encode.
 *	out_storage     Location to write the encoded key to.
 *	out_len	 Size of the output array.
 *  Returns
 *	<0      Failure
 *	>=0     Number of bytes written to out_storage
 */

#ifdef __APPLE__
static
#endif
int
dst_key_to_dnskey(const DST_KEY *key, u_char *out_storage,
			 const int out_len)
{
	u_int16_t val;
	int loc = 0;
	int enc_len = 0;
	if (key == NULL)
		return (-1);

	if (!dst_check_algorithm(key->dk_alg)) { /* make sure alg is available */
		EREPORT(("dst_key_to_dnskey(): Algorithm %d not suppored\n",
			 key->dk_alg));
		return (UNSUPPORTED_KEYALG);
	}
	memset(out_storage, 0, out_len);
	val = (u_int16_t)(key->dk_flags & 0xffff);
	dst_s_put_int16(out_storage, val);
	loc += 2;

	out_storage[loc++] = (u_char) key->dk_proto;
	out_storage[loc++] = (u_char) key->dk_alg;

	if (key->dk_flags > 0xffff) {	/* Extended flags */
		val = (u_int16_t)((key->dk_flags >> 16) & 0xffff);
		dst_s_put_int16(&out_storage[loc], val);
		loc += 2;
	}
	if (key->dk_KEY_struct == NULL)
		return (loc);
	if (key->dk_func && key->dk_func->to_dns_key) {
		enc_len = key->dk_func->to_dns_key(key,
						 (u_char *) &out_storage[loc],
						   out_len - loc);
		if (enc_len > 0)
			return (enc_len + loc);
		else
			return (-1);
	} else
		EREPORT(("dst_key_to_dnskey(): Unsupported ALG %d\n",
			 key->dk_alg));
	return (-1);
}

/*
 *  dst_buffer_to_key
 *	Function to encode a string of raw data into a DST key
 *  Parameters
 *	alg		The algorithm (HMAC only)
 *	key		A pointer to the data
 *	keylen		The length of the data
 *  Returns
 *	NULL	    an error occurred
 *	NON-NULL	the DST key
 */
DST_KEY *
dst_buffer_to_key(const char *key_name,		/* name of the key */
		  const int alg,		/* algorithm */
		  const int flags,		/* dns flags */
		  const int protocol,		/* dns protocol */
		  const u_char *key_buf,	/* key in dns wire fmt */
		  const int key_len)		/* size of key */
{
	
	DST_KEY *dkey = NULL; 
	int dnslen;
	u_char dns[2048];

	if (!dst_check_algorithm(alg)) { /* make sure alg is available */
		EREPORT(("dst_buffer_to_key(): Algorithm %d not suppored\n", alg));
		return (NULL);
	}

	dkey = dst_s_get_key_struct(key_name, alg, flags, 
					     protocol, -1);

	if (dkey == NULL)
		return (NULL);
	if (dkey->dk_func == NULL || dkey->dk_func->from_dns_key == NULL)
		return NULL;

	if (dkey->dk_func->from_dns_key(dkey, key_buf, key_len) < 0) {
		EREPORT(("dst_buffer_to_key(): dst_buffer_to_hmac failed\n"));
		return (dst_free_key(dkey));
	}

	dnslen = dst_key_to_dnskey(dkey, dns, sizeof(dns));
	dkey->dk_id = dst_s_dns_key_id(dns, dnslen);
	return (dkey);
}

#ifdef _UNUSED_API_
int 
dst_key_to_buffer(DST_KEY *key, u_char *out_buff, int buf_len)
{
	int len;
  /* this function will extrac the secret of HMAC into a buffer */
	if (key == NULL) 
		return (0);
	if (key->dk_func != NULL && key->dk_func->to_dns_key != NULL) {
		len = key->dk_func->to_dns_key(key, out_buff, buf_len);
		if (len < 0)
			return (0);
		return (len);
	}
	return (0);
}
#endif

#ifdef _UNUSED_API_
/*
 * dst_s_read_private_key_file
 *     Function reads in private key from a file.
 *     Fills out the KEY structure.
 * Parameters
 *     name    Name of the key to be read.
 *     pk_key  Structure that the key is returned in.
 *     in_id   Key identifier (tag)
 * Return
 *     1 if everthing works
 *     0 if there is any problem
 */

static int
dst_s_read_private_key_file(char *name, DST_KEY *pk_key, u_int16_t in_id,
			    int in_alg)
{
	int cnt, alg, len, major, minor, file_major, file_minor;
	int ret, id;
	char filename[PATH_MAX];
	u_char in_buff[RAW_KEY_SIZE], *p;
	FILE *fp;
	int dnslen;
	u_char dns[2048];

	if (name == NULL || pk_key == NULL) {
		EREPORT(("dst_read_private_key_file(): No key name given\n"));
		return (0);
	}
	/* Make the filename */
	if (dst_s_build_filename(filename, name, in_id, in_alg, PRIVATE_KEY,
				 PATH_MAX) == -1) {
		EREPORT(("dst_read_private_key(): Cannot make filename from %s, %d, and %s\n",
			 name, in_id, PRIVATE_KEY));
		return (0);
	}
	/* first check if we can find the key file */
	if ((fp = dst_s_fopen(filename, "r", 0)) == NULL) {
		EREPORT(("dst_s_read_private_key_file: Could not open file %s in directory %s\n",
			 filename, dst_path[0] ? dst_path :
			 (char *) getcwd(NULL, PATH_MAX - 1)));
		return (0);
	}
	/* now read the header info from the file */
	if ((cnt = fread(in_buff, 1, sizeof(in_buff), fp)) < 5) {
		fclose(fp);
		EREPORT(("dst_s_read_private_key_file: error reading file %s (empty file)\n",
			 filename));
		return (0);
	}
	/* decrypt key */
	fclose(fp);
	if (memcmp(in_buff, "Private-key-format: v", 20) != 0)
		goto fail;
	len = cnt;
	p = in_buff;

	if (!dst_s_verify_str((const char **) &p, "Private-key-format: v")) {
		EREPORT(("dst_s_read_private_key_file(): Not a Key file/Decrypt failed %s\n", name));
		goto fail;
	}
	/* read in file format */
	sscanf((char *)p, "%d.%d", &file_major, &file_minor);
	sscanf(KEY_FILE_FORMAT, "%d.%d", &major, &minor);
	if (file_major < 1) {
		EREPORT(("dst_s_read_private_key_file(): Unknown keyfile %d.%d version for %s\n",
			 file_major, file_minor, name));
		goto fail;
	} else if (file_major > major || file_minor > minor)
		EREPORT((
				"dst_s_read_private_key_file(): Keyfile %s version higher than mine %d.%d MAY FAIL\n",
				name, file_major, file_minor));

	while (*p++ != '\n') ;	/* skip to end of line */

	if (!dst_s_verify_str((const char **) &p, "Algorithm: "))
		goto fail;

	if (sscanf((char *)p, "%d", &alg) != 1)
		goto fail;
	while (*p++ != '\n') ;	/* skip to end of line */

	if (pk_key->dk_key_name && !strcmp(pk_key->dk_key_name, name))
		SAFE_FREE2(pk_key->dk_key_name, strlen(pk_key->dk_key_name));
	pk_key->dk_key_name = (char *) strdup(name);

	/* allocate and fill in key structure */
	if (pk_key->dk_func == NULL || pk_key->dk_func->from_file_fmt == NULL)
		goto fail;

	ret = pk_key->dk_func->from_file_fmt(pk_key, (char *)p, &in_buff[len] - p);
	if (ret < 0)
		goto fail;

	dnslen = dst_key_to_dnskey(pk_key, dns, sizeof(dns));
	id = dst_s_dns_key_id(dns, dnslen);

	/* Make sure the actual key tag matches the input tag used in the filename
	 */
	if (id != in_id) {
		EREPORT(("dst_s_read_private_key_file(): actual tag of key read %d != input tag used to build filename %d.\n", id, in_id));
		goto fail;
	}
	pk_key->dk_id = (u_int16_t) id;
	pk_key->dk_alg = alg;
	memset(in_buff, 0, cnt);
	return (1);

 fail:
	memset(in_buff, 0, cnt);
	return (0);
}
#endif

#ifdef _UNUSED_API_
/*
 *  dst_generate_key
 *	Generate and store a public/private keypair.
 *	Keys will be stored in formatted files.
 *  Parameters
 *	name    Name of the new key.  Used to create key files
 *		  K<name>+<alg>+<id>.public and K<name>+<alg>+<id>.private.
 *	bits    Size of the new key in bits.
 *	exp     What exponent to use:
 *		  0	   use exponent 3
 *		  non-zero    use Fermant4
 *	flags   The default value of the DNS Key flags.
 *		  The DNS Key RR Flag field is defined in RFC 2065,
 *		  section 3.3.  The field has 16 bits.
 *	protocol
 *	      Default value of the DNS Key protocol field.
 *		  The DNS Key protocol field is defined in RFC 2065,
 *		  section 3.4.  The field has 8 bits.
 *	alg     What algorithm to use.  Currently defined:
 *		  KEY_RSA       1
 *		  KEY_DSA       3
 *		  KEY_HMAC    157
 *	out_id The key tag is returned.
 *
 *  Return
 *	NULL		Failure
 *	non-NULL 	the generated key pair
 *			Caller frees the result, and its dk_name pointer.
 */
DST_KEY *
dst_generate_key(const char *name, const int bits, const int exp,
		 const int flags, const int protocol, const int alg)
{
	DST_KEY *new_key = NULL;
	int res;
	int dnslen;
	u_char dns[2048];

	if (name == NULL)
		return (NULL);

	if (!dst_check_algorithm(alg)) { /* make sure alg is available */
		EREPORT(("dst_generate_key(): Algorithm %d not suppored\n", alg));
		return (NULL);
	}

	new_key = dst_s_get_key_struct(name, alg, flags, protocol, bits);
	if (new_key == NULL)
		return (NULL);
	if (bits == 0) /* null key we are done */
		return (new_key);
	if (new_key->dk_func == NULL || new_key->dk_func->generate == NULL) {
		EREPORT(("dst_generate_key_pair():Unsupported algorithm %d\n",
			 alg));
		return (dst_free_key(new_key));
	}
	if ((res = new_key->dk_func->generate(new_key, exp)) <= 0) {
		EREPORT(("dst_generate_key_pair(): Key generation failure %s %d %d %d\n",
			 new_key->dk_key_name, new_key->dk_alg,
			 new_key->dk_key_size, exp));
		return (dst_free_key(new_key));
	}

	dnslen = dst_key_to_dnskey(new_key, dns, sizeof(dns));
	if (dnslen != UNSUPPORTED_KEYALG)
		new_key->dk_id = dst_s_dns_key_id(dns, dnslen);
	else
		new_key->dk_id = 0;

	return (new_key);
}
#endif

/*
 *  dst_free_key
 *	Release all data structures pointed to by a key structure.
 *  Parameters
 *	f_key   Key structure to be freed.
 */

DST_KEY *
dst_free_key(DST_KEY *f_key)
{

	if (f_key == NULL)
		return (f_key);
	if (f_key->dk_func && f_key->dk_func->destroy)
		f_key->dk_KEY_struct =
			f_key->dk_func->destroy(f_key->dk_KEY_struct);
	else {
		EREPORT(("dst_free_key(): Unknown key alg %d\n",
			 f_key->dk_alg));
		free(f_key->dk_KEY_struct);	/* SHOULD NOT happen */
	}
	if (f_key->dk_KEY_struct) {
		free(f_key->dk_KEY_struct);
		f_key->dk_KEY_struct = NULL;
	}
	if (f_key->dk_key_name)
		SAFE_FREE(f_key->dk_key_name);
	SAFE_FREE(f_key);
	return (NULL);
}

#ifdef _UNUSED_API_
/*
 * dst_sig_size
 *	Return the maximim size of signature from the key specified in bytes
 * Parameters
 *      key 
 * Returns
 *     bytes
 */
int
dst_sig_size(DST_KEY *key) {
	switch (key->dk_alg) {
	    case KEY_HMAC_MD5:
		return (16);
	    case KEY_HMAC_SHA1:
		return (20);
	    case KEY_RSA:
		return (key->dk_key_size + 7) / 8;
	    case KEY_DSA:
		return (40);
	    default:
		EREPORT(("dst_sig_size(): Unknown key alg %d\n", key->dk_alg));
		return -1;
	}
}
#endif
