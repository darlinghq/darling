/* All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Apple nor the names of any contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
*/
/*
 * Portions Copyright 2006, Apple Computer, Inc.
 * Christopher Ryan <ryanc@apple.com>
*/

#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "config.h"
#ifndef HAVE_ASPRINTF
#include "asprintf.h"
#endif
#include "macho.h"
#include "util.h"
#include "data.h"
#include "arcmod.h"
#include "xar.h"

#define BIT64 0x01000000
#define PPC   0x00000012
#define I386  0x00000007

struct machexecutables {
	struct mach_header mh;
	struct lc *lc;
	uint32_t curlc;
	uint64_t nextlc;
	char **strings;
	uint32_t stringsz;
	uint8_t byteswapped;
	uint8_t bits;
};

struct _macho_context{
	struct fat_header fath;   /* Need to read and buffer the fat header */
	struct fat_arch *arches;  /* Read and buffer array of arches */
	uint32_t curarch;          /* Current arch being parsed */
	struct machexecutables *me;
	uint64_t nextme;           /* Offset of the next mach header */
	uint32_t curme;            /* Current me being parsed */
	unsigned char buffer[512]; /* Place to store incomplete struct info */
	uint16_t buffersz;         /* Keep track of how much of buffer is used*/
	uint16_t state;            /* Keep track of what we're looking for */
	uint64_t curroffset;       /* Current offset in file */
	uint8_t  byteswapped;      /* Keep track of whether we're byteswapping*/
};

/* Looking for... */
enum {
	lf_fatheader = 0,
	lf_inc_fatheader,
	lf_archheader,
	lf_inc_archheader,
	lf_machheader,
	lf_inc_machheader,
	lf_loadcommand,
	lf_inc_loadcommand,
	lf_lcstr,
	lf_inc_lcstr,
	lf_none
};

#define MACHO_CONTEXT(x) ((struct _macho_context *)(*x))

static const char *macho_cpustr(uint32_t cputype)
{
	const char *cpustr;
	switch(cputype) {
	case PPC: cpustr = "ppc"; break;
	case I386: cpustr = "i386"; break;
	case PPC|BIT64: cpustr = "ppc64"; break;
	default: cpustr = "unknown"; break;
	};
	return cpustr;
}

/* Returns number of bytes consumed during the parsing process */
static int32_t macho_parse(xar_file_t f, void *in, size_t inlen, struct _macho_context *context)
{
	int32_t consumed = 0;

	switch( context->state ) {
	case(lf_fatheader):
		if( inlen >= sizeof(struct fat_header) ) {
			struct fat_header *fh = (struct fat_header *)in;
			if( fh->magic == 0xcafebabe ) {
				context->fath.magic = fh->magic;
				context->fath.nfat_arch = fh->nfat_arch;
				context->arches = calloc(1,sizeof(struct fat_arch) * fh->nfat_arch);
				context->me = calloc(1,sizeof(struct machexecutables) * fh->nfat_arch);
				context->state = lf_archheader;
				context->byteswapped = 0;
				context->curarch = 0;
				consumed = 8;
				xar_prop_set(f, "contents/type", "Mach-O Fat File");
			} else if( fh->magic == 0xbebafeca ) {
				context->fath.magic = xar_swap32(fh->magic);
				context->fath.nfat_arch = xar_swap32(fh->nfat_arch);
				context->arches = calloc(1,sizeof(struct fat_arch) * context->fath.nfat_arch);
				context->me = calloc(1,sizeof(struct machexecutables) * context->fath.nfat_arch);
				context->state = lf_archheader;
				context->byteswapped = 1;
				context->curarch = 0;
				consumed = 8;
				xar_prop_set(f, "contents/type", "Mach-O Fat File");
			} else {
				context->me = calloc(1,sizeof(struct machexecutables));
				context->curme = 0;
				context->state = lf_machheader;
			}
		} else {
			uint32_t *tmp = in;
			if( (*tmp == 0xcafebabe) || (*tmp == 0xbebafeca) ) {
				memcpy(context->buffer, in, inlen);
				context->buffersz = inlen;
				consumed = (int32_t)inlen;
				context->state = lf_inc_fatheader;
			} else {
				context->me = calloc(1,sizeof(struct machexecutables));
				context->curme = 0;
				context->state = lf_machheader;
			}
		}
		break;
	case lf_archheader:
		if( inlen >= sizeof(struct fat_arch) ) {
			struct fat_arch *fa = in;
			if( context->byteswapped ) {
				context->arches[context->curarch].cputype = xar_swap32(fa->cputype);
				context->arches[context->curarch].cpusubtype = xar_swap32(fa->cpusubtype);
				context->arches[context->curarch].offset = xar_swap32(fa->offset);
				context->arches[context->curarch].size = xar_swap32(fa->size);
				context->arches[context->curarch].alighn = xar_swap32(fa->alighn);
			} else {
				memcpy(&context->arches[context->curarch], in, sizeof(struct fat_arch));
			}
			context->curarch++;
			if( context->curarch >=context->fath.nfat_arch ) {
				context->nextme = context->arches[0].offset;
				context->state = lf_machheader;
			}
			consumed = sizeof(struct fat_arch);
		} else {
			memcpy(context->buffer, in, inlen);
			context->buffersz = inlen;
			consumed = (int32_t)inlen;
			context->state = lf_inc_archheader;
		}
		break;
	case lf_machheader:
		if( (context->curroffset+inlen) <= context->nextme )
			consumed = inlen;
		else {
			uint64_t off;
			unsigned char *tmpin = in;
			off = context->nextme - context->curroffset;
			tmpin += off;
			if( (inlen-off) >= sizeof(struct mach_header) ) {
				const char *cpustr;
				char *typestr, *typestr2;
				struct mach_header *mh = (struct mach_header *)tmpin;
				switch(mh->magic) {
				case 0xcffaedfe:
					context->me[context->curme].bits = 64;
					context->me[context->curme].byteswapped = 1;
					break;
				case 0xcefaedfe:
					context->me[context->curme].bits = 32;
					context->me[context->curme].byteswapped = 1;
					break;
				case 0xfeedface:
					context->me[context->curme].bits = 32;
					break;
				case 0xfeedfacf:
					context->me[context->curme].bits = 64;
					break;
				default:
					context->state = lf_none;
					return inlen;
				};
				
				if( context->me[context->curme].byteswapped ) {
					context->me[context->curme].mh.magic = xar_swap32(mh->magic);
					context->me[context->curme].mh.cputype = xar_swap32(mh->cputype);
					context->me[context->curme].mh.cpusubtype = xar_swap32(mh->cpusubtype);
					context->me[context->curme].mh.filetype = xar_swap32(mh->filetype);
					context->me[context->curme].mh.ncmds = xar_swap32(mh->ncmds);
					context->me[context->curme].mh.sizeofcmds = xar_swap32(mh->sizeofcmds);
					context->me[context->curme].mh.flags = xar_swap32(mh->flags);
				} else {
					memcpy(&context->me[context->curme].mh, tmpin, sizeof(struct mach_header));
				}
				
				cpustr = macho_cpustr(context->me[context->curme].mh.cputype);

				switch(context->me[context->curme].mh.filetype) {
				case 0x01: typestr = "Mach-O Object"; break;
				case 0x02: typestr = "Mach-O Executable"; break;
				case 0x03: typestr = "Mach-O Fixed VM Library"; break;
				case 0x04: typestr = "Mach-O core"; break;
				case 0x05: typestr = "Mach-O Preloaded Executable"; break;
				case 0x06: typestr = "Mach-O Dylib"; break;
				case 0x07: typestr = "Mach-O Dylinker"; break;
				case 0x08: typestr = "Mach-O Bundle"; break;
				case 0x09: typestr = "Mach-O Stub"; break;
				default: typestr = "Unknown"; break;
				};

				if( xar_prop_get(f, "contents/type", (const char **)&typestr2) ) {
					xar_prop_set(f, "contents/type", typestr);
				}			
				asprintf(&typestr2, "contents/%s/type", cpustr);
				xar_prop_set(f, typestr2, typestr);
				free(typestr2);

				context->me[context->curme].lc = malloc(sizeof(struct lc) * context->me[context->curme].mh.ncmds);
				context->me[context->curme].strings = malloc(sizeof(char *) * context->me[context->curme].mh.ncmds);
				context->me[context->curme].curlc = 0;
				consumed = off + sizeof(struct mach_header);
				if( context->me[context->curme].bits == 64 )
					consumed += 4;
				context->me[context->curme].nextlc = context->curroffset + consumed;
				context->state = lf_loadcommand;
			} else {
				memcpy(context->buffer, tmpin, inlen-off);
				context->buffersz = inlen-off;
				consumed = (int32_t)inlen;
				context->state = lf_inc_machheader;
			}
		}
		break;
	case lf_loadcommand:
		if( (context->curroffset+inlen) <= context->me[context->curme].nextlc )
			consumed = inlen;
		else {
			uint64_t off;
			unsigned char *tmpin = in;
			off = context->me[context->curme].nextlc - context->curroffset;
			tmpin += off;
			if( (inlen-off) >= sizeof(struct lc) ) {
				if( context->me[context->curme].byteswapped ) {
					struct lc *lc = (struct lc *)tmpin;
					context->me[context->curme].lc[context->me[context->curme].curlc].cmd = xar_swap32(lc->cmd);
					context->me[context->curme].lc[context->me[context->curme].curlc].cmdsize = xar_swap32(lc->cmdsize);
				} else {
					memcpy(&context->me[context->curme].lc[context->me[context->curme].curlc], tmpin, sizeof(struct lc));
				}
				consumed = off + sizeof(struct lc);
				context->me[context->curme].nextlc += context->me[context->curme].lc[context->me[context->curme].curlc].cmdsize;
				if( (context->me[context->curme].lc[context->me[context->curme].curlc].cmd == 0xc) || (context->me[context->curme].lc[context->me[context->curme].curlc].cmd == 0xd) ) {
					context->state = lf_lcstr;
				} else {
					context->me[context->curme].curlc++;
					if( context->me[context->curme].curlc >= context->me[context->curme].mh.ncmds ) {
						context->curme++;
						if( context->fath.nfat_arch ) {
							if( context->curme >= context->fath.nfat_arch ) {
								context->state = lf_none;
							} else {
								context->nextme = context->arches[context->curme].offset;
								context->state = lf_machheader;
							}
						} else {
							context->state = lf_none;
						}
					}
				}
			} else {
				memcpy(context->buffer, ((char *)in)+off, inlen-off);
				context->buffersz = inlen-off;
				consumed = inlen;
				context->state = lf_inc_loadcommand;
			}
		}
		break;
	case lf_lcstr:
		if( inlen >= (context->me[context->curme].lc[context->me[context->curme].curlc].cmdsize-8) ) {
			const char *tmpstr;
			uint32_t cmdsize = context->me[context->curme].lc[context->me[context->curme].curlc].cmdsize;
			uint32_t *offsetp, offset;
			char *lib, *propstr;
			offsetp = in;
			if( context->me[context->curme].byteswapped )
				offset = xar_swap32(*offsetp);
			else
				offset = *offsetp;
			lib = calloc(1,(cmdsize - offset)+1);
			memcpy(lib, ((char *)in)+(offset - 8), cmdsize - offset);
			tmpstr = macho_cpustr(context->me[context->curme].mh.cputype);
			asprintf(&propstr, "contents/%s/library", tmpstr);
			xar_prop_create(f, propstr, lib);
			free(lib);
			free(propstr);

			consumed = cmdsize-8;
			context->state = lf_loadcommand;
			context->me[context->curme].curlc++;
			if( context->me[context->curme].curlc >= context->me[context->curme].mh.ncmds ) {
				context->curme++;
				if( context->fath.nfat_arch ) {
					if( context->curme >= context->fath.nfat_arch ) {
						context->state = lf_none;
					} else {
						context->nextme = context->arches[context->curme].offset;
						context->state = lf_machheader;
					}
				} else {
					context->state = lf_none;
				}
			}
		} else {
			memcpy(context->buffer, in, inlen);
			context->buffersz = inlen;
			consumed = inlen;
			context->state = lf_inc_lcstr;
		}
		break;
	case lf_none:
	default:
		consumed = inlen;
		break;
	};
	return consumed;
}

int32_t xar_macho_in(xar_t x, xar_file_t f, xar_prop_t p, void **in, size_t *inlen, void **context) {
	int32_t consumed = 0, total = 0;

	if( strcmp(xar_prop_getkey(p), "data") != 0 )
		return 0;

	if( !xar_check_prop(x, "contents") )
		return 0;

	if( !*context ) {
		*context = calloc(1,sizeof(struct _macho_context));
	}

	while( total < *inlen ) {
		consumed = macho_parse(f, ((char *)*in)+total, *inlen-total, MACHO_CONTEXT(context));
		total += consumed;
		MACHO_CONTEXT(context)->curroffset += consumed;
	}

	return 0;
}

int32_t xar_macho_done(xar_t x, xar_file_t f, xar_prop_t p, void **context) {

	if( MACHO_CONTEXT(context) ){
		int i;
		if( MACHO_CONTEXT(context)->fath.nfat_arch ) {
			for(i = 0; i < MACHO_CONTEXT(context)->fath.nfat_arch; i++) {
				if( MACHO_CONTEXT(context)->me[i].lc )
					free(MACHO_CONTEXT(context)->me[i].lc);
				if( MACHO_CONTEXT(context)->me[i].strings )
					free(MACHO_CONTEXT(context)->me[i].strings);
			}
		} else {
			if( MACHO_CONTEXT(context)->me ) {
				if( MACHO_CONTEXT(context)->me[0].lc )
					free(MACHO_CONTEXT(context)->me[0].lc);
				if( MACHO_CONTEXT(context)->me[0].strings )
					free(MACHO_CONTEXT(context)->me[0].strings);
			}
		}
		if( MACHO_CONTEXT(context)->me )
			free(MACHO_CONTEXT(context)->me);
		if( MACHO_CONTEXT(context)->arches )
			free(MACHO_CONTEXT(context)->arches);
		free(*context);
	}
	
	return 0;
}
