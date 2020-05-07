/*
 *  ccdrbg.h
 *  corecrypto
 *
 *  Created on 08/17/2010
 *
 *  Copyright (c) 2010,2011,2012,2014,2015 Apple Inc. All rights reserved.
 *
 */

/*!
 @header corecrypto/ccdrbg.h
 @abstract The functions provided in ccdrbg.h implement high-level accessors
 to cryptographically secure random numbers.

 */

#ifndef _CORECRYPTO_CCDRBG_H_
#define _CORECRYPTO_CCDRBG_H_

#include <corecrypto/cc.h>
#include <corecrypto/ccdrbg_impl.h>

/*
 * The maximum length of the entropy_input,  additional_input (max_additional_input_length) , personalization string
 * (max_personalization_string_length) and max_number_of_bits_per_request  are implementation dependent
 * but shall fit in a 32 bit register and be be less than or equal to the specified maximum length for the
 * selected DRBG mechanism (NIST 800-90A Section 10).
 */

#define CCDRBG_MAX_ENTROPY_SIZE         ((uint32_t)1<<16)
#define CCDRBG_MAX_ADDITIONALINPUT_SIZE ((uint32_t)1<<16)
#define CCDRBG_MAX_PSINPUT_SIZE         ((uint32_t)1<<16)
#define CCDRBG_MAX_REQUEST_SIZE         ((uint32_t)1<<16) //this is the absolute maximum in NIST 800-90A
#define CCDRBG_RESEED_INTERVAL          ((uint64_t)1<<48) // must be able to fit the NIST maximum of 2^48


/*
 * The entropyLength is forced to be greater or equal than the security strength.
 * Nonce is not forced. It either needs to have 0.5*security strength entropy. Or, a vale that is repeated
 * less than a 0.5*security strength bit random string.
 * see below or NIST  800-90A for the definition of security strength
 */

CC_INLINE int ccdrbg_init(const struct ccdrbg_info *info,
			struct ccdrbg_state *drbg,
            size_t entropyLength, const void* entropy,
            size_t nonceLength, const void* nonce,
            size_t psLength, const void* ps)
{
	return info->init(info, drbg, entropyLength, entropy, nonceLength, nonce, psLength, ps);
}

/*
 *  The entropyLength is forced to be greater or equal than the security strength.
 */
CC_INLINE int ccdrbg_reseed(const struct ccdrbg_info *info,
       struct ccdrbg_state *drbg,
       size_t entropyLength, const void *entropy,
       size_t additionalLength, const void *additional)
{
    return info->reseed(drbg, entropyLength, entropy, additionalLength, additional);
}


CC_INLINE int ccdrbg_generate(const struct ccdrbg_info *info,
         struct ccdrbg_state *drbg,
         size_t dataOutLength, void *dataOut,
         size_t additionalLength, const void *additional)
{
    return info->generate(drbg, dataOutLength, dataOut, additionalLength, additional);
}

CC_INLINE void ccdrbg_done(const struct ccdrbg_info *info,
		struct ccdrbg_state *drbg)
{
	info->done(drbg);
}

CC_INLINE size_t ccdrbg_context_size(const struct ccdrbg_info *info)
{
    return info->size;
}


/*
 * NIST SP 800-90 CTR_DRBG
 * the maximum security strengh of drbg equals to the block size of the corresponding ECB.
 */
struct ccdrbg_nistctr_custom {
    const struct ccmode_ctr *ctr_info;
    size_t keylen;
    int strictFIPS;
    int use_df;
};

void ccdrbg_factory_nistctr(struct ccdrbg_info *info, const struct ccdrbg_nistctr_custom *custom);

/*
 * NIST SP 800-90 HMAC_DRBG
 * the maximum security strengh of drbg is half of output size of the input hash function and it internally is limited to 256 bits
 */
struct ccdrbg_nisthmac_custom {
    const struct ccdigest_info *di;
    int strictFIPS;
};

void ccdrbg_factory_nisthmac(struct ccdrbg_info *info, const struct ccdrbg_nisthmac_custom *custom);

#endif /* _CORECRYPTO_CCDRBG_H_ */
