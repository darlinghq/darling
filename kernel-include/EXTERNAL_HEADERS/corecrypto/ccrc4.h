/*
 *  ccrc4.h
 *  corecrypto
 *
 *  Created on 12/22/2010
 *
 *  Copyright (c) 2010,2011,2012,2013,2014,2015 Apple Inc. All rights reserved.
 *
 */

#ifndef _CORECRYPTO_CCRC4_H_
#define _CORECRYPTO_CCRC4_H_

#include <corecrypto/ccmode.h>

cc_aligned_struct(16) ccrc4_ctx;

/* Declare a rc4 key named _name_.  Pass the size field of a struct ccmode_ecb
 for _size_. */
#define ccrc4_ctx_decl(_size_, _name_) cc_ctx_decl(ccrc4_ctx, _size_, _name_)
#define ccrc4_ctx_clear(_size_, _name_) cc_clear(_size_, _name_)

struct ccrc4_info {
    size_t size;        /* first argument to ccrc4_ctx_decl(). */
    void (*init)(ccrc4_ctx *ctx, size_t key_len, const void *key);
    void (*crypt)(ccrc4_ctx *ctx, size_t nbytes, const void *in, void *out);
};

const struct ccrc4_info *ccrc4(void);

extern const struct ccrc4_info ccrc4_eay;

#endif /* _CORECRYPTO_CCRC4_H_ */
