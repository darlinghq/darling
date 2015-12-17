/*
 *  skein_dropin.h
 *
 *  Created by Jon Callas on 5/25/10.
 *  Copyright 2010 Apple, Inc. All rights reserved.
 *
 */

#ifndef _SKEIN_DROPIN_H_
#define _SKEIN_DROPIN_H_     1

#ifdef __cplusplus
extern "C" {
#endif
	
extern int Skein_512_128_Init(Skein_512_Ctxt_t *ctx);
extern int Skein_512_160_Init(Skein_512_Ctxt_t *ctx);
extern int Skein_512_224_Init(Skein_512_Ctxt_t *ctx);
extern int Skein_512_256_Init(Skein_512_Ctxt_t *ctx);
extern int Skein_512_384_Init(Skein_512_Ctxt_t *ctx);
extern int Skein_512_512_Init(Skein_512_Ctxt_t *ctx);


extern int Skein_512_128(const u08b_t *msg, size_t msgByteCnt, u08b_t *output);
extern int Skein_512_160(const u08b_t *msg, size_t msgByteCnt, u08b_t *output);
extern int Skein_512_224(const u08b_t *msg, size_t msgByteCnt, u08b_t *output);
extern int Skein_512_256(const u08b_t *msg, size_t msgByteCnt, u08b_t *output);
extern int Skein_512_384(const u08b_t *msg, size_t msgByteCnt, u08b_t *output);
extern int Skein_512_512(const u08b_t *msg, size_t msgByteCnt, u08b_t *output);
	
#ifdef __cplusplus
}
#endif


#endif
