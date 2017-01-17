/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
#ifndef RLD
#include <stdint.h>
#include <libkern/OSByteOrder.h>
#include <mach-o/swap.h>
#include <string.h>

void
swap_fat_header(
struct fat_header *fat_header,
enum NXByteOrder target_byte_sex)
{
	fat_header->magic     = OSSwapInt32(fat_header->magic);
	fat_header->nfat_arch = OSSwapInt32(fat_header->nfat_arch);
}

void
swap_fat_arch(
struct fat_arch *fat_archs,
uint32_t nfat_arch,
enum NXByteOrder target_byte_sex)
{
    uint32_t i;

	for(i = 0; i < nfat_arch; i++){
	    fat_archs[i].cputype    = OSSwapInt32(fat_archs[i].cputype);
	    fat_archs[i].cpusubtype = OSSwapInt32(fat_archs[i].cpusubtype);
	    fat_archs[i].offset     = OSSwapInt32(fat_archs[i].offset);
	    fat_archs[i].size       = OSSwapInt32(fat_archs[i].size);
	    fat_archs[i].align      = OSSwapInt32(fat_archs[i].align);
	}
}

void
swap_fat_arch_64(
struct fat_arch_64 *fat_archs64,
uint32_t nfat_arch,
enum NXByteOrder target_byte_sex)
{
    uint32_t i;

	for(i = 0; i < nfat_arch; i++){
	    fat_archs64[i].cputype    = OSSwapInt32(fat_archs64[i].cputype);
	    fat_archs64[i].cpusubtype = OSSwapInt32(fat_archs64[i].cpusubtype);
	    fat_archs64[i].offset     = OSSwapInt64(fat_archs64[i].offset);
	    fat_archs64[i].size       = OSSwapInt64(fat_archs64[i].size);
	    fat_archs64[i].align      = OSSwapInt32(fat_archs64[i].align);
	    fat_archs64[i].reserved   = OSSwapInt32(fat_archs64[i].reserved);
	}
}

void
swap_mach_header(
struct mach_header *mh,
enum NXByteOrder target_byte_sex)
{
	mh->magic = OSSwapInt32(mh->magic);
	mh->cputype = OSSwapInt32(mh->cputype);
	mh->cpusubtype = OSSwapInt32(mh->cpusubtype);
	mh->filetype = OSSwapInt32(mh->filetype);
	mh->ncmds = OSSwapInt32(mh->ncmds);
	mh->sizeofcmds = OSSwapInt32(mh->sizeofcmds);
	mh->flags = OSSwapInt32(mh->flags);
}

void
swap_mach_header_64(
struct mach_header_64 *mh,
enum NXByteOrder target_byte_sex)
{
	mh->magic = OSSwapInt32(mh->magic);
	mh->cputype = OSSwapInt32(mh->cputype);
	mh->cpusubtype = OSSwapInt32(mh->cpusubtype);
	mh->filetype = OSSwapInt32(mh->filetype);
	mh->ncmds = OSSwapInt32(mh->ncmds);
	mh->sizeofcmds = OSSwapInt32(mh->sizeofcmds);
	mh->flags = OSSwapInt32(mh->flags);
	mh->reserved = OSSwapInt32(mh->reserved);
}

void
swap_load_command(
struct load_command *lc,
enum NXByteOrder target_byte_sex)
{
	lc->cmd = OSSwapInt32(lc->cmd);
	lc->cmdsize = OSSwapInt32(lc->cmdsize);
}

void
swap_segment_command(
struct segment_command *sg,
enum NXByteOrder target_byte_sex)
{
	/* segname[16] */
	sg->cmd = OSSwapInt32(sg->cmd);
	sg->cmdsize = OSSwapInt32(sg->cmdsize);
	sg->vmaddr = OSSwapInt32(sg->vmaddr);
	sg->vmsize = OSSwapInt32(sg->vmsize);
	sg->fileoff = OSSwapInt32(sg->fileoff);
	sg->filesize = OSSwapInt32(sg->filesize);
	sg->maxprot = OSSwapInt32(sg->maxprot);
	sg->initprot = OSSwapInt32(sg->initprot);
	sg->nsects = OSSwapInt32(sg->nsects);
	sg->flags = OSSwapInt32(sg->flags);
}

void 
swap_segment_command_64(
struct segment_command_64* sg,
enum NXByteOrder target_byte_sex)
{
	/* char segname[16] */
	sg->cmd = OSSwapInt32(sg->cmd);
	sg->cmdsize = OSSwapInt32(sg->cmdsize);
	sg->vmaddr = OSSwapInt64(sg->vmaddr);
	sg->vmsize = OSSwapInt64(sg->vmsize);
	sg->fileoff = OSSwapInt64(sg->fileoff);
	sg->filesize = OSSwapInt64(sg->filesize);
	sg->maxprot = OSSwapInt32(sg->maxprot);
	sg->initprot = OSSwapInt32(sg->initprot);
	sg->nsects = OSSwapInt32(sg->nsects);
	sg->flags = OSSwapInt32(sg->flags);
}

void
swap_section(
struct section *s,
uint32_t nsects,
enum NXByteOrder target_byte_sex)
{
    uint32_t i;

	for(i = 0; i < nsects; i++){
	    /* sectname[16] */
	    /* segname[16] */
	    s[i].addr = OSSwapInt32(s[i].addr);
	    s[i].size = OSSwapInt32(s[i].size);
	    s[i].offset = OSSwapInt32(s[i].offset);
	    s[i].align = OSSwapInt32(s[i].align);
	    s[i].reloff = OSSwapInt32(s[i].reloff);
	    s[i].nreloc = OSSwapInt32(s[i].nreloc);
	    s[i].flags = OSSwapInt32(s[i].flags);
	    s[i].reserved1 = OSSwapInt32(s[i].reserved1);
	    s[i].reserved2 = OSSwapInt32(s[i].reserved2);
	}
}

void
swap_section_64(
struct section_64 *s,
uint32_t nsects,
enum NXByteOrder target_byte_sex)
{
    uint32_t i;

	for(i = 0; i < nsects; i++){
	    /* sectname[16] */
	    /* segname[16] */
	    s[i].addr = OSSwapInt64(s[i].addr);
	    s[i].size = OSSwapInt64(s[i].size);
	    s[i].offset = OSSwapInt32(s[i].offset);
	    s[i].align = OSSwapInt32(s[i].align);
	    s[i].reloff = OSSwapInt32(s[i].reloff);
	    s[i].nreloc = OSSwapInt32(s[i].nreloc);
	    s[i].flags = OSSwapInt32(s[i].flags);
	    s[i].reserved1 = OSSwapInt32(s[i].reserved1);
	    s[i].reserved2 = OSSwapInt32(s[i].reserved2);
	    s[i].reserved3 = OSSwapInt32(s[i].reserved3);
	}
}

void
swap_symtab_command(
struct symtab_command *st,
enum NXByteOrder target_byte_sex)
{
	st->cmd = OSSwapInt32(st->cmd);
	st->cmdsize = OSSwapInt32(st->cmdsize);
	st->symoff = OSSwapInt32(st->symoff);
	st->nsyms = OSSwapInt32(st->nsyms);
	st->stroff = OSSwapInt32(st->stroff);
	st->strsize = OSSwapInt32(st->strsize);
}

void
swap_dysymtab_command(
struct dysymtab_command *dyst,
enum NXByteOrder target_byte_sex)
{
	dyst->cmd = OSSwapInt32(dyst->cmd);
	dyst->cmdsize = OSSwapInt32(dyst->cmdsize);
	dyst->ilocalsym = OSSwapInt32(dyst->ilocalsym);
	dyst->nlocalsym = OSSwapInt32(dyst->nlocalsym);
	dyst->iextdefsym = OSSwapInt32(dyst->iextdefsym);
	dyst->nextdefsym = OSSwapInt32(dyst->nextdefsym);
	dyst->iundefsym = OSSwapInt32(dyst->iundefsym);
	dyst->nundefsym = OSSwapInt32(dyst->nundefsym);
	dyst->tocoff = OSSwapInt32(dyst->tocoff);
	dyst->ntoc = OSSwapInt32(dyst->ntoc);
	dyst->modtaboff = OSSwapInt32(dyst->modtaboff);
	dyst->nmodtab = OSSwapInt32(dyst->nmodtab);
	dyst->extrefsymoff = OSSwapInt32(dyst->extrefsymoff);
	dyst->nextrefsyms = OSSwapInt32(dyst->nextrefsyms);
	dyst->indirectsymoff = OSSwapInt32(dyst->indirectsymoff);
	dyst->nindirectsyms = OSSwapInt32(dyst->nindirectsyms);
	dyst->extreloff = OSSwapInt32(dyst->extreloff);
	dyst->nextrel = OSSwapInt32(dyst->nextrel);
	dyst->locreloff = OSSwapInt32(dyst->locreloff);
	dyst->nlocrel = OSSwapInt32(dyst->nlocrel);
}

void
swap_symseg_command(
struct symseg_command *ss,
enum NXByteOrder target_byte_sex)
{
	ss->cmd = OSSwapInt32(ss->cmd);
	ss->cmdsize = OSSwapInt32(ss->cmdsize);
	ss->offset = OSSwapInt32(ss->offset);
	ss->size = OSSwapInt32(ss->size);
}

void
swap_fvmlib_command(
struct fvmlib_command *fl,
enum NXByteOrder target_byte_sex)
{
	fl->cmd = OSSwapInt32(fl->cmd);
	fl->cmdsize = OSSwapInt32(fl->cmdsize);
	fl->fvmlib.name.offset = OSSwapInt32(fl->fvmlib.name.offset);
	fl->fvmlib.minor_version = OSSwapInt32(fl->fvmlib.minor_version);
	fl->fvmlib.header_addr = OSSwapInt32(fl->fvmlib.header_addr);
}

void
swap_dylib_command(
struct dylib_command *dl,
enum NXByteOrder target_byte_sex)
{
	dl->cmd = OSSwapInt32(dl->cmd);
	dl->cmdsize = OSSwapInt32(dl->cmdsize);
	dl->dylib.name.offset = OSSwapInt32(dl->dylib.name.offset);
	dl->dylib.timestamp = OSSwapInt32(dl->dylib.timestamp);
	dl->dylib.current_version = OSSwapInt32(dl->dylib.current_version);
	dl->dylib.compatibility_version =
				OSSwapInt32(dl->dylib.compatibility_version);
}

void
swap_sub_framework_command(
struct sub_framework_command *sub,
enum NXByteOrder target_byte_sex)
{
	sub->cmd = OSSwapInt32(sub->cmd);
	sub->cmdsize = OSSwapInt32(sub->cmdsize);
	sub->umbrella.offset = OSSwapInt32(sub->umbrella.offset);
}

void
swap_sub_umbrella_command(
struct sub_umbrella_command *usub,
enum NXByteOrder target_byte_sex)
{
	usub->cmd = OSSwapInt32(usub->cmd);
	usub->cmdsize = OSSwapInt32(usub->cmdsize);
	usub->sub_umbrella.offset = OSSwapInt32(usub->sub_umbrella.offset);
}

void
swap_sub_library_command(
struct sub_library_command *lsub,
enum NXByteOrder target_byte_sex)
{
	lsub->cmd = OSSwapInt32(lsub->cmd);
	lsub->cmdsize = OSSwapInt32(lsub->cmdsize);
	lsub->sub_library.offset = OSSwapInt32(lsub->sub_library.offset);
}

void
swap_sub_client_command(
struct sub_client_command *csub,
enum NXByteOrder target_byte_sex)
{
	csub->cmd = OSSwapInt32(csub->cmd);
	csub->cmdsize = OSSwapInt32(csub->cmdsize);
	csub->client.offset = OSSwapInt32(csub->client.offset);
}


void
swap_prebound_dylib_command(
struct prebound_dylib_command *pbdylib,
enum NXByteOrder target_byte_sex)
{
	pbdylib->cmd = OSSwapInt32(pbdylib->cmd);
	pbdylib->cmdsize = OSSwapInt32(pbdylib->cmdsize);
	pbdylib->name.offset = OSSwapInt32(pbdylib->name.offset);
	pbdylib->nmodules = OSSwapInt32(pbdylib->nmodules);
	pbdylib->linked_modules.offset =
		OSSwapInt32(pbdylib->linked_modules.offset);
}

void
swap_dylinker_command(
struct dylinker_command *dyld,
enum NXByteOrder target_byte_sex)
{
	dyld->cmd = OSSwapInt32(dyld->cmd);
	dyld->cmdsize = OSSwapInt32(dyld->cmdsize);
	dyld->name.offset = OSSwapInt32(dyld->name.offset);
}

void
swap_fvmfile_command(
struct fvmfile_command *ff,
enum NXByteOrder target_byte_sex)
{
	ff->cmd = OSSwapInt32(ff->cmd);
	ff->cmdsize = OSSwapInt32(ff->cmdsize);
	ff->name.offset = OSSwapInt32(ff->name.offset);
	ff->header_addr = OSSwapInt32(ff->header_addr);
}


void
swap_thread_command(
struct thread_command *ut,
enum NXByteOrder target_byte_sex)
{
	ut->cmd = OSSwapInt32(ut->cmd);
	ut->cmdsize = OSSwapInt32(ut->cmdsize);
}

void
swap_ident_command(
struct ident_command *id_cmd,
enum NXByteOrder target_byte_sex)
{
	id_cmd->cmd = OSSwapInt32(id_cmd->cmd);
	id_cmd->cmdsize = OSSwapInt32(id_cmd->cmdsize);
}

void
swap_routines_command(
struct routines_command *r_cmd,
enum NXByteOrder target_byte_sex)
{
	r_cmd->cmd = OSSwapInt32(r_cmd->cmd);
	r_cmd->cmdsize = OSSwapInt32(r_cmd->cmdsize);
	r_cmd->init_address = OSSwapInt32(r_cmd->init_address);
	r_cmd->init_module = OSSwapInt32(r_cmd->init_module);
	r_cmd->reserved1 = OSSwapInt32(r_cmd->reserved1);
	r_cmd->reserved2 = OSSwapInt32(r_cmd->reserved2);
	r_cmd->reserved3 = OSSwapInt32(r_cmd->reserved3);
	r_cmd->reserved4 = OSSwapInt32(r_cmd->reserved4);
	r_cmd->reserved5 = OSSwapInt32(r_cmd->reserved5);
	r_cmd->reserved6 = OSSwapInt32(r_cmd->reserved6);
}

void
swap_routines_command_64(
struct routines_command_64 *r_cmd,
enum NXByteOrder target_byte_sex)
{
	r_cmd->cmd = OSSwapInt32(r_cmd->cmd);
	r_cmd->cmdsize = OSSwapInt32(r_cmd->cmdsize);
	r_cmd->init_address = OSSwapInt64(r_cmd->init_address);
	r_cmd->init_module = OSSwapInt64(r_cmd->init_module);
	r_cmd->reserved1 = OSSwapInt64(r_cmd->reserved1);
	r_cmd->reserved2 = OSSwapInt64(r_cmd->reserved2);
	r_cmd->reserved3 = OSSwapInt64(r_cmd->reserved3);
	r_cmd->reserved4 = OSSwapInt64(r_cmd->reserved4);
	r_cmd->reserved5 = OSSwapInt64(r_cmd->reserved5);
	r_cmd->reserved6 = OSSwapInt64(r_cmd->reserved6);
}

void
swap_twolevel_hints_command(
struct twolevel_hints_command *hints_cmd,
enum NXByteOrder target_byte_sex)
{
	hints_cmd->cmd = OSSwapInt32(hints_cmd->cmd);
	hints_cmd->cmdsize = OSSwapInt32(hints_cmd->cmdsize);
	hints_cmd->offset = OSSwapInt32(hints_cmd->offset);
	hints_cmd->nhints = OSSwapInt32(hints_cmd->nhints);
}

void
swap_prebind_cksum_command(
struct prebind_cksum_command *cksum_cmd,
enum NXByteOrder target_byte_sex)
{
	cksum_cmd->cmd = OSSwapInt32(cksum_cmd->cmd);
	cksum_cmd->cmdsize = OSSwapInt32(cksum_cmd->cmdsize);
	cksum_cmd->cksum = OSSwapInt32(cksum_cmd->cksum);
}

void
swap_uuid_command(
struct uuid_command *uuid_cmd,
enum NXByteOrder target_byte_sex)
{
	uuid_cmd->cmd = OSSwapInt32(uuid_cmd->cmd);
	uuid_cmd->cmdsize = OSSwapInt32(uuid_cmd->cmdsize);
}

void
swap_linkedit_data_command(
struct linkedit_data_command *ld,
enum NXByteOrder target_byte_sex)
{
	ld->cmd = OSSwapInt32(ld->cmd);
	ld->cmdsize = OSSwapInt32(ld->cmdsize);
	ld->dataoff = OSSwapInt32(ld->dataoff);
	ld->datasize = OSSwapInt32(ld->datasize);
}

void
swap_version_min_command(
struct version_min_command *ver_cmd,
enum NXByteOrder target_byte_sex)
{
	ver_cmd->cmd = OSSwapInt32(ver_cmd->cmd);
	ver_cmd->cmdsize = OSSwapInt32(ver_cmd->cmdsize);
	ver_cmd->version = OSSwapInt32(ver_cmd->version);
}

void
swap_rpath_command(
struct rpath_command *rpath_cmd,
enum NXByteOrder target_byte_sex)
{
	rpath_cmd->cmd = OSSwapInt32(rpath_cmd->cmd);
	rpath_cmd->cmdsize = OSSwapInt32(rpath_cmd->cmdsize);
	rpath_cmd->path.offset = OSSwapInt32(rpath_cmd->path.offset);
}

void
swap_encryption_command(
struct encryption_info_command *ec,
enum NXByteOrder target_byte_sex)
{
	ec->cmd = OSSwapInt32(ec->cmd);
	ec->cmdsize = OSSwapInt32(ec->cmdsize);
	ec->cryptoff = OSSwapInt32(ec->cryptoff);
	ec->cryptsize = OSSwapInt32(ec->cryptsize);
	ec->cryptid = OSSwapInt32(ec->cryptid);
}

void
swap_encryption_command_64(
struct encryption_info_command_64 *ec,
enum NXByteOrder target_byte_sex)
{
	ec->cmd = OSSwapInt32(ec->cmd);
	ec->cmdsize = OSSwapInt32(ec->cmdsize);
	ec->cryptoff = OSSwapInt32(ec->cryptoff);
	ec->cryptsize = OSSwapInt32(ec->cryptsize);
	ec->cryptid = OSSwapInt32(ec->cryptid);
	ec->cryptid = OSSwapInt32(ec->pad);
}

void
swap_linker_option_command(
struct linker_option_command *lo,
enum NXByteOrder target_byte_sex)
{
	lo->cmd = OSSwapInt32(lo->cmd);
	lo->cmdsize = OSSwapInt32(lo->cmdsize);
	lo->count = OSSwapInt32(lo->count);
}

void
swap_dyld_info_command(
struct dyld_info_command *ed,
enum NXByteOrder target_byte_sex)
{
	ed->cmd = OSSwapInt32(ed->cmd);
	ed->cmdsize = OSSwapInt32(ed->cmdsize);
	ed->rebase_off = OSSwapInt32(ed->rebase_off);
	ed->rebase_size = OSSwapInt32(ed->rebase_size);
	ed->bind_off = OSSwapInt32(ed->bind_off);
	ed->bind_size = OSSwapInt32(ed->bind_size);
	ed->weak_bind_off = OSSwapInt32(ed->weak_bind_off);
	ed->weak_bind_size = OSSwapInt32(ed->weak_bind_size);
	ed->lazy_bind_off = OSSwapInt32(ed->lazy_bind_off);
	ed->lazy_bind_size = OSSwapInt32(ed->lazy_bind_size);
	ed->export_off = OSSwapInt32(ed->export_off);
	ed->export_size = OSSwapInt32(ed->export_size);
}

void
swap_entry_point_command(
struct entry_point_command *ep,
enum NXByteOrder target_byte_sex)
{
	ep->cmd = OSSwapInt32(ep->cmd);
	ep->cmdsize = OSSwapInt32(ep->cmdsize);
	ep->entryoff = OSSwapInt64(ep->entryoff);
	ep->stacksize = OSSwapInt64(ep->stacksize);
}

void
swap_source_version_command(
struct source_version_command *sv,
enum NXByteOrder target_byte_sex)
{
	sv->cmd = OSSwapInt32(sv->cmd);
	sv->cmdsize = OSSwapInt32(sv->cmdsize);
	sv->version = OSSwapInt64(sv->version);
}

void
swap_nlist(
struct nlist *symbols,
uint32_t nsymbols,
enum NXByteOrder target_byte_sex)
{
    uint32_t i;

	for(i = 0; i < nsymbols; i++){
	    symbols[i].n_un.n_strx = OSSwapInt32(symbols[i].n_un.n_strx);
	    /* n_type */
	    /* n_sect */
	    symbols[i].n_desc = OSSwapInt16(symbols[i].n_desc);
	    symbols[i].n_value = OSSwapInt32(symbols[i].n_value);
	}
}

void
swap_nlist_64(
struct nlist_64 *symbols,
uint32_t nsymbols,
enum NXByteOrder target_byte_sex)
{
    uint32_t i;

	for(i = 0; i < nsymbols; i++){
	    symbols[i].n_un.n_strx = OSSwapInt32(symbols[i].n_un.n_strx);
	    /* n_type */
	    /* n_sect */
	    symbols[i].n_desc = OSSwapInt16(symbols[i].n_desc);
	    symbols[i].n_value = OSSwapInt64(symbols[i].n_value);
	}
}

void
swap_ranlib(
struct ranlib *ranlibs,
uint32_t nranlibs,
enum NXByteOrder target_byte_sex)
{
    uint32_t i;

	for(i = 0; i < nranlibs; i++){
	    ranlibs[i].ran_un.ran_strx = OSSwapInt32(ranlibs[i].ran_un.ran_strx);
	    ranlibs[i].ran_off = OSSwapInt32(ranlibs[i].ran_off);
	}
}

void
swap_ranlib_64(
struct ranlib_64 *ranlibs,
uint64_t nranlibs,
enum NXByteOrder target_byte_sex)
{
    uint64_t i;

	for(i = 0; i < nranlibs; i++){
	    ranlibs[i].ran_un.ran_strx = OSSwapInt64(ranlibs[i].ran_un.ran_strx);
	    ranlibs[i].ran_off = OSSwapInt64(ranlibs[i].ran_off);
	}
}

void
swap_relocation_info(
struct relocation_info *relocs,
uint32_t nrelocs,
enum NXByteOrder target_byte_sex)
{
    uint32_t i;
    enum NXByteOrder host_byte_sex;
    uint32_t to_host_byte_sex, scattered;

    struct swapped_relocation_info {
	uint32_t r_address;
	union {
	    struct {
		unsigned int
		    r_type:4,
		    r_extern:1,
		    r_length:2,
		    r_pcrel:1,
		    r_symbolnum:24;
	    } fields;
	    uint32_t word;
	} u;
    } sr;

    struct swapped_scattered_relocation_info {
	uint32_t word;
	uint32_t r_value;
    } *ssr;

	host_byte_sex = NXHostByteOrder();
	to_host_byte_sex = target_byte_sex == host_byte_sex;

	for(i = 0; i < nrelocs; i++){
	    if(to_host_byte_sex)
		scattered = (OSSwapInt32(relocs[i].r_address) & R_SCATTERED) != 0;
	    else
		scattered = ((relocs[i].r_address) & R_SCATTERED) != 0;
	    if(scattered == FALSE){
		if(to_host_byte_sex){
		    memcpy(&sr, relocs + i, sizeof(struct relocation_info));
		    sr.r_address = OSSwapInt32(sr.r_address);
		    sr.u.word = OSSwapInt32(sr.u.word);
		    relocs[i].r_address = sr.r_address;
		    relocs[i].r_symbolnum = sr.u.fields.r_symbolnum;
		    relocs[i].r_pcrel = sr.u.fields.r_pcrel;
		    relocs[i].r_length = sr.u.fields.r_length;
		    relocs[i].r_extern = sr.u.fields.r_extern;
		    relocs[i].r_type = sr.u.fields.r_type;
		}
		else{
		    sr.r_address = relocs[i].r_address;
		    sr.u.fields.r_symbolnum = relocs[i].r_symbolnum;
		    sr.u.fields.r_length = relocs[i].r_length;
		    sr.u.fields.r_pcrel = relocs[i].r_pcrel;
		    sr.u.fields.r_extern = relocs[i].r_extern;
		    sr.u.fields.r_type = relocs[i].r_type;
		    sr.r_address = OSSwapInt32(sr.r_address);
		    sr.u.word = OSSwapInt32(sr.u.word);
		    memcpy(relocs + i, &sr, sizeof(struct relocation_info));
		}
	    }
	    else{
		ssr = (struct swapped_scattered_relocation_info *)(relocs + i);
		ssr->word = OSSwapInt32(ssr->word);
		ssr->r_value = OSSwapInt32(ssr->r_value);
	    }
	}
}

void
swap_indirect_symbols(
uint32_t *indirect_symbols,
uint32_t nindirect_symbols,
enum NXByteOrder target_byte_sex)
{
    uint32_t i;

	for(i = 0; i < nindirect_symbols; i++)
	    indirect_symbols[i] = OSSwapInt32(indirect_symbols[i]);
}

void
swap_dylib_reference(
struct dylib_reference *refs,
uint32_t nrefs,
enum NXByteOrder target_byte_sex)
{
    struct swapped_dylib_reference {
	union {
	    struct {
		uint32_t
		    flags:8,
		    isym:24;
	    } fields;
	    uint32_t word;
	} u;
    } sref;

    uint32_t i;
    enum NXByteOrder host_byte_sex;

	host_byte_sex = NXHostByteOrder();

	for(i = 0; i < nrefs; i++){
	    if(target_byte_sex == host_byte_sex){
		memcpy(&sref, refs + i, sizeof(struct swapped_dylib_reference));
		sref.u.word = OSSwapInt32(sref.u.word);
		refs[i].flags = sref.u.fields.flags;
		refs[i].isym = sref.u.fields.isym;
	    }
	    else{
		sref.u.fields.isym = refs[i].isym;
		sref.u.fields.flags = refs[i].flags;
		sref.u.word = OSSwapInt32(sref.u.word);
		memcpy(refs + i, &sref, sizeof(struct swapped_dylib_reference));
	    }
	}

}

void
swap_dylib_module(
struct dylib_module *mods,
uint32_t nmods,
enum NXByteOrder target_byte_sex)
{
    uint32_t i;

	for(i = 0; i < nmods; i++){
	    mods[i].module_name = OSSwapInt32(mods[i].module_name);
	    mods[i].iextdefsym  = OSSwapInt32(mods[i].iextdefsym);
	    mods[i].nextdefsym  = OSSwapInt32(mods[i].nextdefsym);
	    mods[i].irefsym     = OSSwapInt32(mods[i].irefsym);
	    mods[i].nrefsym     = OSSwapInt32(mods[i].nrefsym);
	    mods[i].ilocalsym   = OSSwapInt32(mods[i].ilocalsym);
	    mods[i].nlocalsym   = OSSwapInt32(mods[i].nlocalsym);
	    mods[i].iextrel     = OSSwapInt32(mods[i].iextrel);
	    mods[i].nextrel     = OSSwapInt32(mods[i].nextrel);
	    mods[i].iinit_iterm = OSSwapInt32(mods[i].iinit_iterm);
	    mods[i].ninit_nterm = OSSwapInt32(mods[i].ninit_nterm);
	    mods[i].objc_module_info_size = OSSwapInt32(mods[i].objc_module_info_size);
	    mods[i].objc_module_info_addr = OSSwapInt32(mods[i].objc_module_info_addr);
	}
}

void
swap_dylib_module_64(
struct dylib_module_64 *mods,
uint32_t nmods,
enum NXByteOrder target_byte_sex)
{
    uint32_t i;

	for(i = 0; i < nmods; i++){
	    mods[i].module_name = OSSwapInt32(mods[i].module_name);
	    mods[i].iextdefsym  = OSSwapInt32(mods[i].iextdefsym);
	    mods[i].nextdefsym  = OSSwapInt32(mods[i].nextdefsym);
	    mods[i].irefsym     = OSSwapInt32(mods[i].irefsym);
	    mods[i].nrefsym     = OSSwapInt32(mods[i].nrefsym);
	    mods[i].ilocalsym   = OSSwapInt32(mods[i].ilocalsym);
	    mods[i].nlocalsym   = OSSwapInt32(mods[i].nlocalsym);
	    mods[i].iextrel     = OSSwapInt32(mods[i].iextrel);
	    mods[i].nextrel     = OSSwapInt32(mods[i].nextrel);
	    mods[i].iinit_iterm = OSSwapInt32(mods[i].iinit_iterm);
	    mods[i].ninit_nterm = OSSwapInt32(mods[i].ninit_nterm);
	    mods[i].objc_module_info_size = OSSwapInt32(mods[i].objc_module_info_size);
	    mods[i].objc_module_info_addr = OSSwapInt64(mods[i].objc_module_info_addr);
	}
}

void
swap_dylib_table_of_contents(
struct dylib_table_of_contents *tocs,
uint32_t ntocs,
enum NXByteOrder target_byte_sex)
{
    uint32_t i;

	for(i = 0; i < ntocs; i++){
	    tocs[i].symbol_index = OSSwapInt32(tocs[i].symbol_index);
	    tocs[i].module_index = OSSwapInt32(tocs[i].module_index);
	}
}
#endif /* !defined(RLD) */
