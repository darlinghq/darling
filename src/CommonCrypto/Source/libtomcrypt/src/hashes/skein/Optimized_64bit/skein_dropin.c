/*
 *  skein_dropin.c
 *
 *  Created by Jon Callas on 5/25/10.
 *  Copyright 2010 Apple, Inc. All rights reserved.
 *
 */

#include "skein.h"

#if SKEIN512_BUILD

int Skein_512_128_Init(Skein_512_Ctxt_t *ctx)
{
	return Skein_512_Init(ctx, 128);
}

int Skein_512_160_Init(Skein_512_Ctxt_t *ctx)
{
	return Skein_512_Init(ctx, 160);
}

int Skein_512_224_Init(Skein_512_Ctxt_t *ctx)
{
	return Skein_512_Init(ctx, 224);
}

int Skein_512_256_Init(Skein_512_Ctxt_t *ctx)
{
	return Skein_512_Init(ctx, 256);
}

int Skein_512_384_Init(Skein_512_Ctxt_t *ctx)
{
	return Skein_512_Init(ctx, 384);
}

int Skein_512_512_Init(Skein_512_Ctxt_t *ctx)
{
	return Skein_512_Init(ctx, 512);
}

///// One-shot returns

int Skein_512_128(const u08b_t *msg, size_t msgByteCnt, u08b_t *output)
{
	int result;
	Skein_512_Ctxt_t ctx;
	
	result = Skein_512_Init(&ctx, 128);
	
	if (result == SKEIN_SUCCESS)
		result = Skein_512_Update(&ctx, msg, msgByteCnt);
	
	if (result == SKEIN_SUCCESS)
		result = Skein_512_Final(&ctx, output);
	
	return result;
}

int Skein_512_160(const u08b_t *msg, size_t msgByteCnt, u08b_t *output)
{
	int result;
	Skein_512_Ctxt_t ctx;
	
	result = Skein_512_Init(&ctx, 160);
	
	if (result == SKEIN_SUCCESS)
		result = Skein_512_Update(&ctx, msg, msgByteCnt);
	
	if (result == SKEIN_SUCCESS)
		result = Skein_512_Final(&ctx, output);
	
	return result;
}

int Skein_512_224(const u08b_t *msg, size_t msgByteCnt, u08b_t *output)
{
	int result;
	Skein_512_Ctxt_t ctx;
	
	result = Skein_512_Init(&ctx, 224);
	
	if (result == SKEIN_SUCCESS)
		result = Skein_512_Update(&ctx, msg, msgByteCnt);
	
	if (result == SKEIN_SUCCESS)
		result = Skein_512_Final(&ctx, output);
	
	return result;
}

int Skein_512_256(const u08b_t *msg, size_t msgByteCnt, u08b_t *output)
{
	int result;
	Skein_512_Ctxt_t ctx;
	
	result = Skein_512_Init(&ctx, 256);
	
	if (result == SKEIN_SUCCESS)
		result = Skein_512_Update(&ctx, msg, msgByteCnt);
	
	if (result == SKEIN_SUCCESS)
		result = Skein_512_Final(&ctx, output);
	
	return result;
}

int Skein_512_384(const u08b_t *msg, size_t msgByteCnt, u08b_t *output)
{
	int result;
	Skein_512_Ctxt_t ctx;
	
	result = Skein_512_Init(&ctx, 384);
	
	if (result == SKEIN_SUCCESS)
		result = Skein_512_Update(&ctx, msg, msgByteCnt);
	
	if (result == SKEIN_SUCCESS)
		result = Skein_512_Final(&ctx, output);
	
	return result;
}

int Skein_512_512(const u08b_t *msg, size_t msgByteCnt, u08b_t *output)
{
	int result;
	Skein_512_Ctxt_t ctx;
	
	result = Skein_512_Init(&ctx, 512);
	
	if (result == SKEIN_SUCCESS)
		result = Skein_512_Update(&ctx, msg, msgByteCnt);
	
	if (result == SKEIN_SUCCESS)
		result = Skein_512_Final(&ctx, output);
	
	return result;
}

#endif /* SKEIN512_BUILD */

