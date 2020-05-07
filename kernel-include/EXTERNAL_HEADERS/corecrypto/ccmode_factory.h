/*
 *  ccmode_factory.h
 *  corecrypto
 *
 *  Created on 01/21/2011
 *
 *  Copyright (c) 2011,2012,2013,2014,2015 Apple Inc. All rights reserved.
 *
 */

#ifndef _CORECRYPTO_CCMODE_FACTORY_H_
#define _CORECRYPTO_CCMODE_FACTORY_H_

#include <corecrypto/ccn.h>  /* TODO: Remove dependency on this header. */
#include <corecrypto/ccmode_impl.h>

/* Functions defined in this file are only to be used
 within corecrypto files.
 */

/* Use these function to runtime initialize a ccmode_cbc decrypt object (for
 example if it's part of a larger structure). Normally you would pass a
 ecb decrypt mode implementation of some underlying algorithm as the ecb
 parameter. */
void ccmode_factory_cbc_decrypt(struct ccmode_cbc *cbc,
                                const struct ccmode_ecb *ecb);

/* Use these function to runtime initialize a ccmode_cbc encrypt object (for
 example if it's part of a larger structure). Normally you would pass a
 ecb encrypt mode implementation of some underlying algorithm as the ecb
 parameter. */
void ccmode_factory_cbc_encrypt(struct ccmode_cbc *cbc,
                                const struct ccmode_ecb *ecb);


/* Use these function to runtime initialize a ccmode_cfb decrypt object (for
 example if it's part of a larger structure). Normally you would pass a
 ecb encrypt mode implementation of some underlying algorithm as the ecb
 parameter. */
void ccmode_factory_cfb_decrypt(struct ccmode_cfb *cfb,
                                const struct ccmode_ecb *ecb);

/* Use these function to runtime initialize a ccmode_cfb encrypt object (for
 example if it's part of a larger structure). Normally you would pass a
 ecb encrypt mode implementation of some underlying algorithm as the ecb
 parameter. */
void ccmode_factory_cfb_encrypt(struct ccmode_cfb *cfb,
                                const struct ccmode_ecb *ecb);

/* Use these function to runtime initialize a ccmode_cfb8 decrypt object (for
 example if it's part of a larger structure). Normally you would pass a
 ecb decrypt mode implementation of some underlying algorithm as the ecb
 parameter. */
void ccmode_factory_cfb8_decrypt(struct ccmode_cfb8 *cfb8,
                                 const struct ccmode_ecb *ecb);

/* Use these function to runtime initialize a ccmode_cfb8 encrypt object (for
 example if it's part of a larger structure). Normally you would pass a
 ecb encrypt mode implementation of some underlying algorithm as the ecb
 parameter. */
void ccmode_factory_cfb8_encrypt(struct ccmode_cfb8 *cfb8,
                                 const struct ccmode_ecb *ecb);

/* Use these function to runtime initialize a ccmode_ctr decrypt object (for
 example if it's part of a larger structure). Normally you would pass a
 ecb encrypt mode implementation of some underlying algorithm as the ecb
 parameter. */
void ccmode_factory_ctr_crypt(struct ccmode_ctr *ctr,
                              const struct ccmode_ecb *ecb);

/* Use these function to runtime initialize a ccmode_gcm decrypt object (for
 example if it's part of a larger structure). For GCM you always pass a
 ecb encrypt mode implementation of some underlying algorithm as the ecb
 parameter. */
void ccmode_factory_gcm_decrypt(struct ccmode_gcm *gcm,
                                const struct ccmode_ecb *ecb_encrypt);

/* Use these function to runtime initialize a ccmode_gcm encrypt object (for
 example if it's part of a larger structure). For GCM you always pass a
 ecb encrypt mode implementation of some underlying algorithm as the ecb
 parameter. */
void ccmode_factory_gcm_encrypt(struct ccmode_gcm *gcm,
                                const struct ccmode_ecb *ecb_encrypt);

/* Use these function to runtime initialize a ccmode_ccm decrypt object (for
 example if it's part of a larger structure). For CCM you always pass a
 ecb encrypt mode implementation of some underlying algorithm as the ecb
 parameter. */

void ccmode_factory_ccm_decrypt(struct ccmode_ccm *ccm,
                                const struct ccmode_ecb *ecb_encrypt);

/* Use these function to runtime initialize a ccmode_ccm encrypt object (for
 example if it's part of a larger structure). For CCM you always pass a
 ecb encrypt mode implementation of some underlying algorithm as the ecb
 parameter. */
void ccmode_factory_ccm_encrypt(struct ccmode_ccm *ccm,
                                const struct ccmode_ecb *ecb_encrypt);

/* Use these function to runtime initialize a ccmode_ofb encrypt object (for
 example if it's part of a larger structure). Normally you would pass a
 ecb encrypt mode implementation of some underlying algorithm as the ecb
 parameter. */
void ccmode_factory_ofb_crypt(struct ccmode_ofb *ofb,
                              const struct ccmode_ecb *ecb);

/* Use these function to runtime initialize a ccmode_omac decrypt object (for
 example if it's part of a larger structure). Normally you would pass a
 ecb decrypt mode implementation of some underlying algorithm as the ecb
 parameter. */
void ccmode_factory_omac_decrypt(struct ccmode_omac *omac,
                                 const struct ccmode_ecb *ecb);

/* Use these function to runtime initialize a ccmode_omac encrypt object (for
 example if it's part of a larger structure). Normally you would pass a
 ecb encrypt mode implementation of some underlying algorithm as the ecb
 parameter. */
void ccmode_factory_omac_encrypt(struct ccmode_omac *omac,
                                 const struct ccmode_ecb *ecb);

/* Use these function to runtime initialize a ccmode_xts decrypt object (for
 example if it's part of a larger structure). Normally you would pass a
 ecb decrypt mode implementation of some underlying algorithm as the ecb
 parameter. */
void ccmode_factory_xts_decrypt(struct ccmode_xts *xts,
                                const struct ccmode_ecb *ecb,
                                const struct ccmode_ecb *ecb_encrypt);

/* Use these function to runtime initialize a ccmode_xts encrypt object (for
 example if it's part of a larger structure). Normally you would pass a
 ecb encrypt mode implementation of some underlying algorithm as the ecb
 parameter. */
void ccmode_factory_xts_encrypt(struct ccmode_xts *xts,
                                const struct ccmode_ecb *ecb,
                                const struct ccmode_ecb *ecb_encrypt);

#endif /* _CORECRYPTO_CCMODE_FACTORY_H_ */
