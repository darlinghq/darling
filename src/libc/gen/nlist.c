/*
 * Copyright (c) 1999, 2008 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * Copyright (c) 1989, 1993
 * The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* temporarily comment this file out for LP64, until code can be modified */
#ifndef __LP64__

#include <stdlib.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <malloc_private.h>

/* Stuff lifted from <a.out.h> and <sys/exec.h> since they are gone */
/*
 * Header prepended to each a.out file.
 */
struct exec {
unsigned short  a_machtype;     /* machine type */
unsigned short  a_magic;        /* magic number */
unsigned long	a_text;		/* size of text segment */
unsigned long	a_data;		/* size of initialized data */
unsigned long	a_bss;		/* size of uninitialized data */
unsigned long	a_syms;		/* size of symbol table */
unsigned long	a_entry;	/* entry point */
unsigned long	a_trsize;	/* size of text relocation */
unsigned long	a_drsize;	/* size of data relocation */
};

#define	OMAGIC	0407		/* old impure format */
#define	NMAGIC	0410		/* read-only text */
#define	ZMAGIC	0413		/* demand load format */

#define	N_BADMAG(x) \
    (((x).a_magic)!=OMAGIC && ((x).a_magic)!=NMAGIC && ((x).a_magic)!=ZMAGIC)
#define	N_TXTOFF(x) \
	((x).a_magic==ZMAGIC ? 0 : sizeof (struct exec))
#define N_SYMOFF(x) \
	(N_TXTOFF(x) + (x).a_text+(x).a_data + (x).a_trsize+(x).a_drsize)

#include <mach/mach.h>
#include <mach-o/nlist.h>
#include <stdio.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>

/*
 * CPUSUBTYPE_SUPPORT should be changed to non-zero once the
 * cpusubtype_* routines are available in libc.
 */
#define	CPUSUBTYPE_SUPPORT	0

int __fdnlist(int fd, struct nlist *list);

/*
 * nlist - retreive attributes from name list (string table version)
 */

int
nlist(name, list)
	const char *name;
	struct nlist *list;
{
	int fd, n;

	fd = open(name, O_RDONLY, 0);
	if (fd < 0)
		return (-1);
	n = __fdnlist(fd, list);
	(void)close(fd);
	return (n);
}

/* Note: __fdnlist() is called from kvm_nlist in libkvm's kvm.c */

int
__fdnlist(fd, list)
	int fd;
	struct nlist *list;
{
	register struct nlist *p, *q;
	register char *s1, *s2;
	register int n, m;
	int maxlen, nreq;
	off_t sa;		/* symbol address */
	off_t ss;		/* start of strings */
	struct exec buf;
	struct nlist space[BUFSIZ/sizeof (struct nlist)];
	unsigned  arch_offset = 0;

	maxlen = 0;
	for (q = list, nreq = 0; q->n_un.n_name && q->n_un.n_name[0]; q++, nreq++) {
		q->n_type = 0;
		q->n_value = 0;
		q->n_desc = 0;
		q->n_sect = 0;
		n = strlen(q->n_un.n_name);
		if (n > maxlen)
			maxlen = n;
	}
	if (read(fd, (char *)&buf, sizeof(buf)) != sizeof(buf) ||
	    (N_BADMAG(buf) && *((uint32_t *)&buf) != MH_MAGIC &&
	     OSSwapBigToHostInt32(*((uint32_t *)&buf)) != FAT_MAGIC)) {
		return (-1);
	}

	/* Deal with fat file if necessary */
	if (OSSwapBigToHostInt32(*((uint32_t *)&buf)) == FAT_MAGIC) {
		struct host_basic_info hbi;
		struct fat_header fh;
		struct fat_arch *fat_archs, *fap;
		unsigned i;
		host_t host;

		/* Get our host info */
		host = mach_host_self();
		i = HOST_BASIC_INFO_COUNT;
		if (host_info(host, HOST_BASIC_INFO,
			      (host_info_t)(&hbi), &i) != KERN_SUCCESS) {
			return (-1);
		}
		mach_port_deallocate(mach_task_self(), host);
		  
		/* Read in the fat header */
		lseek(fd, 0, SEEK_SET);
		if (read(fd, (char *)&fh, sizeof(fh)) != sizeof(fh)) {
			return (-1);
		}

		/* Convert fat_narchs to host byte order */
		fh.nfat_arch = OSSwapBigToHostInt32(fh.nfat_arch);

		/* Read in the fat archs */
		fat_archs = (struct fat_arch *)reallocarray(NULL, fh.nfat_arch,
				sizeof(struct fat_arch));
		if (fat_archs == NULL) {
			return (-1);
		}
		if (read(fd, (char *)fat_archs,
			 sizeof(struct fat_arch) * fh.nfat_arch) !=
		    sizeof(struct fat_arch) * fh.nfat_arch) {
			free(fat_archs);
			return (-1);
		}

		/*
		 * Convert archs to host byte ordering (a constraint of
		 * cpusubtype_getbestarch()
		 */
		for (i = 0; i < fh.nfat_arch; i++) {
			fat_archs[i].cputype =
				OSSwapBigToHostInt32(fat_archs[i].cputype);
			fat_archs[i].cpusubtype =
			      OSSwapBigToHostInt32(fat_archs[i].cpusubtype);
			fat_archs[i].offset =
				OSSwapBigToHostInt32(fat_archs[i].offset);
			fat_archs[i].size =
				OSSwapBigToHostInt32(fat_archs[i].size);
			fat_archs[i].align =
				OSSwapBigToHostInt32(fat_archs[i].align);
		}

#if	CPUSUBTYPE_SUPPORT
		fap = cpusubtype_getbestarch(hbi.cpu_type, hbi.cpu_subtype,
					     fat_archs, fh.nfat_arch);
#else
		fap = NULL;
		for (i = 0; i < fh.nfat_arch; i++) {
			if (fat_archs[i].cputype == hbi.cpu_type) {
				fap = &fat_archs[i];
				break;
			}
		}
#endif	/* CPUSUBTYPE_SUPPORT */
		if (!fap) {
			free(fat_archs);
			return (-1);
		}
		arch_offset = fap->offset;
		free(fat_archs);

		/* Read in the beginning of the architecture-specific file */
		lseek(fd, arch_offset, SEEK_SET);
		if (read(fd, (char *)&buf, sizeof(buf)) != sizeof(buf)) {
			return (-1);
		}
	}
		
	if (*((uint32_t *)&buf) == MH_MAGIC) {
	    struct mach_header mh;
	    struct load_command *load_commands, *lcp;
	    struct symtab_command *stp;
	    long i;

		lseek(fd, arch_offset, SEEK_SET);
		if (read(fd, (char *)&mh, sizeof(mh)) != sizeof(mh)) {
			return (-1);
		}
		load_commands = (struct load_command *)malloc(mh.sizeofcmds);
		if (load_commands == NULL) {
			return (-1);
		}
		if (read(fd, (char *)load_commands, mh.sizeofcmds) !=
		    mh.sizeofcmds) {
			free(load_commands);
			return (-1);
		}
		stp = NULL;
		lcp = load_commands;
		for (i = 0; i < mh.ncmds; i++) {
			if (lcp->cmdsize % sizeof(uint32_t) != 0 ||
			    lcp->cmdsize <= 0 ||
			    (char *)lcp + lcp->cmdsize >
			    (char *)load_commands + mh.sizeofcmds) {
				free(load_commands);
				return (-1);
			}
			if (lcp->cmd == LC_SYMTAB) {
				if (lcp->cmdsize !=
				   sizeof(struct symtab_command)) {
					free(load_commands);
					return (-1);
				}
				stp = (struct symtab_command *)lcp;
				break;
			}
			lcp = (struct load_command *)
			      ((char *)lcp + lcp->cmdsize);
		}
		if (stp == NULL) {
			free(load_commands);
			return (-1);
		}
		sa = stp->symoff + arch_offset;
		ss = stp->stroff + arch_offset;
		n = stp->nsyms * sizeof(struct nlist);
		free(load_commands);
	}
	else {
		sa = N_SYMOFF(buf) + arch_offset;
		ss = sa + buf.a_syms + arch_offset;
		n = buf.a_syms;
	}

	lseek(fd, sa, SEEK_SET);
	while (n) {
		off_t savpos;

		m = sizeof (space);
		if (n < m)
			m = n;
		if (read(fd, (char *)space, m) != m)
			break;
		n -= m;
		savpos = lseek(fd, 0, SEEK_CUR);
		for (q = space; (m -= sizeof(struct nlist)) >= 0; q++) {
			char nambuf[BUFSIZ];

			if (q->n_un.n_strx == 0 || q->n_type & N_STAB)
				continue;
			lseek(fd, ss+q->n_un.n_strx, SEEK_SET);
			read(fd, nambuf, maxlen+1);
			for (p = list; p->n_un.n_name && p->n_un.n_name[0]; p++) {
				s1 = p->n_un.n_name;
				s2 = nambuf;
				while (*s1) {
					if (*s1++ != *s2++)
						goto cont;
				}
				if (*s2)
					goto cont;
				p->n_value = q->n_value;
				p->n_type = q->n_type;
				p->n_desc = q->n_desc;
				p->n_sect = q->n_sect;
				if (--nreq == 0)
					return (nreq);
				break;
		cont:		;
			}
		}
		lseek(fd, savpos, SEEK_SET);
	}
	return (nreq);
}

#endif /* !__LP64__ */
