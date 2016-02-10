#ifndef DST_INTERNAL_H
#define DST_INTERNAL_H

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
#include <limits.h>
#include <sys/param.h>
#if (!defined(BSD)) || (BSD < 199306)
# include <sys/bitypes.h>
#else
# include <sys/types.h>
#endif

#ifndef PATH_MAX
# ifdef POSIX_PATH_MAX
#  define PATH_MAX POSIX_PATH_MAX
# else
#  define PATH_MAX 255 /* this is the value of POSIX_PATH_MAX */
# endif
#endif 

#ifndef dst_func
#define dst_func res_9_dst_func
#endif
#define DST_KEY res_9_DST_KEY
typedef struct dst_key {
	char	*dk_key_name;   /* name of the key */
	int	dk_key_size;    /* this is the size of the key in bits */
	int	dk_proto;       /* what protocols this key can be used for */
	int	dk_alg;         /* algorithm number from key record */
	u_int32_t dk_flags;     /* and the flags of the public key */
	u_int16_t dk_id;        /* identifier of the key */
	void	*dk_KEY_struct; /* pointer to key in crypto pkg fmt */
	struct dst_func *dk_func; /* point to cryptto pgk specific function table */
} DST_KEY;
#define HAS_DST_KEY 

#ifdef __APPLE__
#include "dst.h"
#else
#include <isc/dst.h>
#endif

/* 
 * define what crypto systems are supported for RSA, 
 * BSAFE is prefered over RSAREF; only one can be set at any time
 */
#if defined(BSAFE) && defined(RSAREF)
# error "Cannot have both BSAFE and RSAREF defined"
#endif

/* Declare dst_lib specific constants */
#define KEY_FILE_FORMAT "1.2"

/* suffixes for key file names */
#define PRIVATE_KEY		"private"
#define PUBLIC_KEY		"key"

/* error handling */
#ifdef REPORT_ERRORS
#define EREPORT(str)		printf str
#else
#define EREPORT(str)		(void)0
#endif

/* use our own special macro to FRRE memory */

#ifndef SAFE_FREE
#define SAFE_FREE(a) \
do{if(a != NULL){memset(a,0, sizeof(*a)); free(a); a=NULL;}} while (0)
#define SAFE_FREE2(a,s) if (a != NULL && s > 0){memset(a,0, s);free(a); a=NULL;}
#endif

#define dst_func res_9_dst_func
typedef struct dst_func {
	int (*sign)(const int mode, DST_KEY *key, void **context,
		     const u_int8_t *data, const int len,
		     u_int8_t *signature, const int sig_len);
	int (*verify)(const int mode, DST_KEY *key, void **context,
		       const u_int8_t *data, const int len,
		       const u_int8_t *signature, const int sig_len);
	int (*compare)(const DST_KEY *key1, const DST_KEY *key2);
	int (*generate)(DST_KEY *key, int parms);
	void *(*destroy)(void *key);
	/* conversion functions */
	int (*to_dns_key)(const DST_KEY *key, u_int8_t *out,
			   const int out_len);
	int (*from_dns_key)(DST_KEY *key, const u_int8_t *str,
			     const int str_len);
	int (*to_file_fmt)(const DST_KEY *key, char *out,
			    const int out_len);
	int (*from_file_fmt)(DST_KEY *key, const char *out,
			      const int out_len);

} dst_func;

#define dst_t_func res_9_dst_t_func
extern dst_func *dst_t_func[DST_MAX_ALGS];

#define key_file_fmt_str res_9_key_file_fmt_str
extern const char *key_file_fmt_str;

#define dst_path res_9_dst_path
extern const char *dst_path;

#ifndef DST_HASH_SIZE
#define DST_HASH_SIZE 20	/* RIPEMD160 and SHA-1 are 20 bytes MD5 is 16 */
#endif

#define dst_bsafe_init res_9_dst_bsafe_init
int dst_bsafe_init(void);

#define dst_rsaref_init res_9_dst_rsaref_init
int dst_rsaref_init(void);

#define dst_hmac_md5_init res_9_dst_hmac_md5_init
int dst_hmac_md5_init(void);

#define dst_cylink_init res_9_dst_cylink_init
int dst_cylink_init(void);

#define dst_eay_dss_init res_9_dst_eay_dss_init
int dst_eay_dss_init(void);

/* from higher level support routines */
#define dst_s_calculate_bits res_9_dst_s_calculate_bits
int       dst_s_calculate_bits( const u_int8_t *str, const int max_bits);

#define dst_s_verify_str res_9_dst_s_verify_str
int       dst_s_verify_str( const char **buf, const char *str);


/* conversion between dns names and key file names */
#define dst_s_filename_length res_9_dst_s_filename_length
size_t    dst_s_filename_length( const char *name, const char *suffix);

#define dst_s_build_filename res_9_dst_s_build_filename
int       dst_s_build_filename(  char *filename, const char *name, 
			         u_int16_t id, int alg, const char *suffix, 
			         size_t filename_length);

#define dst_s_fopen res_9_dst_s_fopen
FILE      *dst_s_fopen (const char *filename, const char *mode, int perm);

/* 
 * read and write network byte order into u_int?_t  
 *  all of these should be retired
 */
#define dst_s_get_int16 res_9_dst_s_get_int16
u_int16_t dst_s_get_int16( const u_int8_t *buf);

#define dst_s_put_int16 res_9_dst_s_put_int16
void      dst_s_put_int16( u_int8_t *buf, const u_int16_t val);

#define dst_s_get_int32 res_9_dst_s_get_int32
u_int32_t dst_s_get_int32( const u_int8_t *buf);

#define dst_s_put_int32 res_9_dst_s_put_int32
void      dst_s_put_int32( u_int8_t *buf, const u_int32_t val);

#ifdef DUMP
# undef DUMP
# define DUMP(a,b,c,d) dst_s_dump(a,b,c,d)
#else
# define DUMP(a,b,c,d)
#endif
#define dst_s_dump res_9_dst_s_dump
void
dst_s_dump(const int mode, const u_char *data, const int size,
            const char *msg);



#endif /* DST_INTERNAL_H */
