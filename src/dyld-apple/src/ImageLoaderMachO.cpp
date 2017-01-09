/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2004-2010 Apple Inc. All rights reserved.
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

// work around until conformance work is complete rdar://problem/4508801
#define __srr0	srr0 
#define __eip	eip 
#define __rip	rip 

#define __STDC_LIMIT_MACROS
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/stat.h> 
#include <sys/mman.h>
#include <mach/mach.h>
#include <mach/thread_status.h>
#include <mach-o/loader.h> 
#include <mach-o/nlist.h> 
#include <sys/sysctl.h>
#include <sys/syscall.h>
#include <libkern/OSAtomic.h>
#include <libkern/OSCacheControl.h>
#include <stdint.h>
#include <System/sys/codesign.h>

#include "ImageLoaderMachO.h"
#include "ImageLoaderMachOCompressed.h"
#if SUPPORT_CLASSIC_MACHO
#include "ImageLoaderMachOClassic.h"
#endif
#include "mach-o/dyld_images.h"
#include "dyld.h"

// <rdar://problem/8718137> use stack guard random value to add padding between dylibs
extern "C" long __stack_chk_guard;

#ifndef LC_LOAD_UPWARD_DYLIB
	#define	LC_LOAD_UPWARD_DYLIB (0x23|LC_REQ_DYLD)	/* load of dylib whose initializers run later */
#endif

#ifndef LC_VERSION_MIN_TVOS
	#define LC_VERSION_MIN_TVOS 0x2F
#endif

#ifndef LC_VERSION_MIN_WATCHOS
	#define LC_VERSION_MIN_WATCHOS 0x30
#endif


#if TARGET_IPHONE_SIMULATOR
	#define LIBSYSTEM_DYLIB_PATH "/usr/lib/libSystem.dylib"
#else
	#define LIBSYSTEM_DYLIB_PATH "/usr/lib/libSystem.B.dylib"
#endif

// relocation_info.r_length field has value 3 for 64-bit executables and value 2 for 32-bit executables
#if __LP64__
	#define LC_SEGMENT_COMMAND		LC_SEGMENT_64
	#define LC_ROUTINES_COMMAND		LC_ROUTINES_64
	#define LC_SEGMENT_COMMAND_WRONG LC_SEGMENT
	struct macho_segment_command	: public segment_command_64  {};
	struct macho_section			: public section_64  {};	
	struct macho_routines_command	: public routines_command_64  {};	
#else
	#define LC_SEGMENT_COMMAND		LC_SEGMENT
	#define LC_ROUTINES_COMMAND		LC_ROUTINES
	#define LC_SEGMENT_COMMAND_WRONG LC_SEGMENT_64
	struct macho_segment_command	: public segment_command {};
	struct macho_section			: public section  {};	
	struct macho_routines_command	: public routines_command  {};	
#endif

uint32_t ImageLoaderMachO::fgSymbolTableBinarySearchs = 0;


ImageLoaderMachO::ImageLoaderMachO(const macho_header* mh, const char* path, unsigned int segCount, 
																uint32_t segOffsets[], unsigned int libCount)
 : ImageLoader(path, libCount), fCoveredCodeLength(0), fMachOData((uint8_t*)mh), fLinkEditBase(NULL), fSlide(0),
	fEHFrameSectionOffset(0), fUnwindInfoSectionOffset(0), fDylibIDOffset(0), 
fSegmentsCount(segCount), fIsSplitSeg(false), fInSharedCache(false),
#if TEXT_RELOC_SUPPORT
	fTextSegmentRebases(false),
	fTextSegmentBinds(false),
#endif
#if __i386__
	fReadOnlyImportSegment(false),
#endif
	fHasSubLibraries(false), fHasSubUmbrella(false), fInUmbrella(false), fHasDOFSections(false), fHasDashInit(false),
	fHasInitializers(false), fHasTerminators(false), fNotifyObjC(false), fRetainForObjC(false), fRegisteredAsRequiresCoalescing(false)
{
	fIsSplitSeg = ((mh->flags & MH_SPLIT_SEGS) != 0);        

	// construct SegmentMachO object for each LC_SEGMENT cmd using "placement new" to put 
	// each SegmentMachO object in array at end of ImageLoaderMachO object
	const uint32_t cmd_count = mh->ncmds;
	const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0, segIndex=0; i < cmd_count; ++i) {
		if ( cmd->cmd == LC_SEGMENT_COMMAND ) {
			const struct macho_segment_command* segCmd = (struct macho_segment_command*)cmd;
			// ignore zero-sized segments
			if ( segCmd->vmsize != 0 ) {
				// record offset of load command
				segOffsets[segIndex++] = (uint32_t)((uint8_t*)segCmd - fMachOData);
			}
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}

}

#if __MAC_OS_X_VERSION_MIN_REQUIRED
static uintptr_t pageAlign(uintptr_t value)
{
	return (value + 4095) & (-4096);
}
#endif

// determine if this mach-o file has classic or compressed LINKEDIT and number of segments it has
void ImageLoaderMachO::sniffLoadCommands(const macho_header* mh, const char* path, bool inCache, bool* compressed,
											unsigned int* segCount, unsigned int* libCount, const LinkContext& context,
											const linkedit_data_command** codeSigCmd,
											const encryption_info_command** encryptCmd)
{
	*compressed = false;
	*segCount = 0;
	*libCount = 0;
	*codeSigCmd = NULL;
	*encryptCmd = NULL;

	const uint32_t cmd_count = mh->ncmds;
	const uint32_t sizeofcmds = mh->sizeofcmds;
	if ( sizeofcmds > (MAX_MACH_O_HEADER_AND_LOAD_COMMANDS_SIZE-sizeof(macho_header)) )
		dyld::throwf("malformed mach-o: load commands size (%u) > %u", sizeofcmds, MAX_MACH_O_HEADER_AND_LOAD_COMMANDS_SIZE);
	if ( cmd_count > (sizeofcmds/sizeof(load_command)) )
		dyld::throwf("malformed mach-o: ncmds (%u) too large to fit in sizeofcmds (%u)", cmd_count, sizeofcmds);
	const struct load_command* const startCmds = (struct load_command*)(((uint8_t*)mh) + sizeof(macho_header));
	const struct load_command* const endCmds = (struct load_command*)(((uint8_t*)mh) + sizeof(macho_header) + sizeofcmds);
	const struct load_command* cmd = startCmds;
	bool foundLoadCommandSegment = false;
	const macho_segment_command* linkeditSegCmd = NULL;
	const macho_segment_command* startOfFileSegCmd = NULL;
	const dyld_info_command* dyldInfoCmd = NULL;
	const symtab_command* symTabCmd = NULL;
	const dysymtab_command*	dynSymbTabCmd = NULL;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		uint32_t cmdLength = cmd->cmdsize;
		const macho_segment_command* segCmd;
		const dylib_command* dylibCmd;
		if ( cmdLength < 8 ) {
			dyld::throwf("malformed mach-o image: load command #%d length (%u) too small in %s",
											   i, cmdLength, path);
		}
		const struct load_command* const nextCmd = (const struct load_command*)(((char*)cmd)+cmdLength);
		if ( (nextCmd > endCmds) || (nextCmd < cmd) ) {
			dyld::throwf("malformed mach-o image: load command #%d length (%u) would exceed sizeofcmds (%u) in %s",
											   i, cmdLength, mh->sizeofcmds, path);
		}
		switch (cmd->cmd) {
			case LC_DYLD_INFO:
			case LC_DYLD_INFO_ONLY:
				if ( cmd->cmdsize != sizeof(dyld_info_command) )
					throw "malformed mach-o image: LC_DYLD_INFO size wrong";
				dyldInfoCmd = (struct dyld_info_command*)cmd;
				*compressed = true;
				break;
			case LC_SEGMENT_COMMAND:
				segCmd = (struct macho_segment_command*)cmd;
#if __MAC_OS_X_VERSION_MIN_REQUIRED
				// rdar://problem/19617624 allow unmapped segments on OSX (but not iOS)
				if ( ((segCmd->filesize) > pageAlign(segCmd->vmsize)) && (segCmd->vmsize != 0) )
#else
				// <rdar://problem/19986776> dyld should support non-allocatable __LLVM segment
				if ( (segCmd->filesize > segCmd->vmsize) && ((segCmd->vmsize != 0) || ((segCmd->flags & SG_NORELOC) == 0)) )
#endif
				    dyld::throwf("malformed mach-o image: segment load command %s filesize (0x%0lX) is larger than vmsize (0x%0lX)", segCmd->segname, (long)segCmd->filesize , (long)segCmd->vmsize );
				if ( cmd->cmdsize < sizeof(macho_segment_command) )
					throw "malformed mach-o image: LC_SEGMENT size too small";
				if ( cmd->cmdsize != (sizeof(macho_segment_command) + segCmd->nsects * sizeof(macho_section)) )
					throw "malformed mach-o image: LC_SEGMENT size wrong for number of sections";
				// ignore zero-sized segments
				if ( segCmd->vmsize != 0 )
					*segCount += 1;
				if ( strcmp(segCmd->segname, "__LINKEDIT") == 0 ) {
		#if TARGET_IPHONE_SIMULATOR
					// Note: should check on all platforms that __LINKEDIT is read-only, but <rdar://problem/22637626&22525618>
					if ( segCmd->initprot != VM_PROT_READ )
						throw "malformed mach-o image: __LINKEDIT segment does not have read-only permissions";
		#endif
					if ( segCmd->fileoff ==  0 )
						throw "malformed mach-o image: __LINKEDIT has fileoff==0 which overlaps mach_header";
					if ( linkeditSegCmd != NULL )
						throw "malformed mach-o image: multiple __LINKEDIT segments";
					linkeditSegCmd = segCmd;
				}
				else {
					if ( segCmd->initprot & 0xFFFFFFF8 )
						dyld::throwf("malformed mach-o image: %s segment has invalid permission bits (0x%X) in initprot", segCmd->segname, segCmd->initprot);
					if ( segCmd->maxprot & 0xFFFFFFF8 )
						dyld::throwf("malformed mach-o image: %s segment has invalid permission bits (0x%X) in maxprot", segCmd->segname, segCmd->maxprot);
					if ( (segCmd->initprot != 0) && ((segCmd->initprot & VM_PROT_READ) == 0) )
						dyld::throwf("malformed mach-o image: %s segment is not mapped readable", segCmd->segname);
				}
				if ( (segCmd->fileoff == 0) && (segCmd->filesize != 0) ) {
					if ( (segCmd->initprot & VM_PROT_READ) == 0 )
						dyld::throwf("malformed mach-o image: %s segment maps start of file but is not readable", segCmd->segname);
					if ( (segCmd->initprot & VM_PROT_WRITE) == VM_PROT_WRITE ) {
						if ( context.strictMachORequired )
							dyld::throwf("malformed mach-o image: %s segment maps start of file but is writable", segCmd->segname);
					}
					if ( segCmd->filesize < (sizeof(macho_header) + mh->sizeofcmds) )
						dyld::throwf("malformed mach-o image: %s segment does not map all of load commands", segCmd->segname);
					if ( startOfFileSegCmd != NULL )
						dyld::throwf("malformed mach-o image: multiple segments map start of file: %s %s", startOfFileSegCmd->segname, segCmd->segname);
					startOfFileSegCmd = segCmd;
				}
				if ( context.strictMachORequired ) {
					uintptr_t vmStart   = segCmd->vmaddr;
					uintptr_t vmSize    = segCmd->vmsize;
					uintptr_t vmEnd     = vmStart + vmSize;
					uintptr_t fileStart = segCmd->fileoff;
					uintptr_t fileSize  = segCmd->filesize;
					if ( (intptr_t)(vmSize) < 0 )
						dyld::throwf("malformed mach-o image: segment load command %s vmsize too large in %s", segCmd->segname, path);
					if ( vmStart > vmEnd )
						dyld::throwf("malformed mach-o image: segment load command %s wraps around address space", segCmd->segname);
					if ( vmSize != fileSize ) {
						if ( segCmd->initprot == 0 ) {
							// allow: fileSize == 0 && initprot == 0		e.g. __PAGEZERO
							// allow: vmSize == 0 && initprot == 0			e.g. __LLVM
							if ( (fileSize != 0) && (vmSize != 0) )
								dyld::throwf("malformed mach-o image: unaccessable segment %s has non-zero filesize and vmsize", segCmd->segname);
						}
						else {
							// allow: vmSize > fileSize && initprot != X  e.g. __DATA
							if ( vmSize < fileSize ) {
								dyld::throwf("malformed mach-o image: segment %s has vmsize < filesize", segCmd->segname);
							}
							if ( segCmd->initprot & VM_PROT_EXECUTE ) {
								dyld::throwf("malformed mach-o image: segment %s has vmsize != filesize and is executable", segCmd->segname);
							}
						}
					}
					if ( inCache ) {
						if ( (fileSize != 0) && (segCmd->initprot == (VM_PROT_READ | VM_PROT_EXECUTE)) ) {
							if ( foundLoadCommandSegment )
								throw "load commands in multiple segments";
							foundLoadCommandSegment = true;
						}
					}
					else if ( (fileStart < mh->sizeofcmds) && (fileSize != 0) ) {
						// <rdar://problem/7942521> all load commands must be in an executable segment
						if ( (fileStart != 0) || (fileSize < (mh->sizeofcmds+sizeof(macho_header))) )
							dyld::throwf("malformed mach-o image: segment %s does not span all load commands", segCmd->segname); 
						if ( segCmd->initprot != (VM_PROT_READ | VM_PROT_EXECUTE) ) 
							dyld::throwf("malformed mach-o image: load commands found in segment %s with wrong permissions", segCmd->segname); 
						if ( foundLoadCommandSegment )
							throw "load commands in multiple segments";
						foundLoadCommandSegment = true;
					}

					const struct macho_section* const sectionsStart = (struct macho_section*)((char*)segCmd + sizeof(struct macho_segment_command));
					const struct macho_section* const sectionsEnd = &sectionsStart[segCmd->nsects];
					for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
						if (!inCache && sect->offset != 0 && ((sect->offset + sect->size) > (segCmd->fileoff + segCmd->filesize)))
							dyld::throwf("malformed mach-o image: section %s,%s of '%s' exceeds segment %s booundary", sect->segname, sect->sectname, path, segCmd->segname);
					}
				}
				break;
			case LC_SEGMENT_COMMAND_WRONG:
				dyld::throwf("malformed mach-o image: wrong LC_SEGMENT[_64] for architecture"); 
				break;
			case LC_LOAD_DYLIB:
			case LC_LOAD_WEAK_DYLIB:
			case LC_REEXPORT_DYLIB:
			case LC_LOAD_UPWARD_DYLIB:
				*libCount += 1;
				// fall thru
			case LC_ID_DYLIB:
				dylibCmd = (dylib_command*)cmd;
				if ( dylibCmd->dylib.name.offset > cmdLength )
					dyld::throwf("malformed mach-o image: dylib load command #%d has offset (%u) outside its size (%u)", i, dylibCmd->dylib.name.offset, cmdLength);
				if ( (dylibCmd->dylib.name.offset + strlen((char*)dylibCmd + dylibCmd->dylib.name.offset) + 1) > cmdLength )
					dyld::throwf("malformed mach-o image: dylib load command #%d string extends beyond end of load command", i);
				break;
			case LC_CODE_SIGNATURE:
				if ( cmd->cmdsize != sizeof(linkedit_data_command) )
					throw "malformed mach-o image: LC_CODE_SIGNATURE size wrong";
				// <rdar://problem/22799652> only support one LC_CODE_SIGNATURE per image
				if ( *codeSigCmd != NULL )
					throw "malformed mach-o image: multiple LC_CODE_SIGNATURE load commands";
				*codeSigCmd = (struct linkedit_data_command*)cmd;
				break;
			case LC_ENCRYPTION_INFO:
				if ( cmd->cmdsize != sizeof(encryption_info_command) )
					throw "malformed mach-o image: LC_ENCRYPTION_INFO size wrong";
				// <rdar://problem/22799652> only support one LC_ENCRYPTION_INFO per image
				if ( *encryptCmd != NULL )
					throw "malformed mach-o image: multiple LC_ENCRYPTION_INFO load commands";
				*encryptCmd = (encryption_info_command*)cmd;
				break;
			case LC_ENCRYPTION_INFO_64:
				if ( cmd->cmdsize != sizeof(encryption_info_command_64) )
					throw "malformed mach-o image: LC_ENCRYPTION_INFO_64 size wrong";
				// <rdar://problem/22799652> only support one LC_ENCRYPTION_INFO_64 per image
				if ( *encryptCmd != NULL )
					throw "malformed mach-o image: multiple LC_ENCRYPTION_INFO_64 load commands";
				*encryptCmd = (encryption_info_command*)cmd;
				break;
			case LC_SYMTAB:
				if ( cmd->cmdsize != sizeof(symtab_command) )
					throw "malformed mach-o image: LC_SYMTAB size wrong";
				symTabCmd = (symtab_command*)cmd;
				break;
			case LC_DYSYMTAB:
				if ( cmd->cmdsize != sizeof(dysymtab_command) )
					throw "malformed mach-o image: LC_DYSYMTAB size wrong";
				dynSymbTabCmd = (dysymtab_command*)cmd;
				break;
#if __MAC_OS_X_VERSION_MIN_REQUIRED
			// <rdar://problem/26797345> error when loading iOS Simulator mach-o binary into macOS process
			case LC_VERSION_MIN_WATCHOS:
			case LC_VERSION_MIN_TVOS:
			case LC_VERSION_MIN_IPHONEOS:
				throw "mach-o, but built for simulator (not macOS)";
				break;
#endif
		}
		cmd = nextCmd;
	}

	if ( context.strictMachORequired && !foundLoadCommandSegment )
		throw "load commands not in a segment";
	if ( linkeditSegCmd == NULL )
		throw "malformed mach-o image: missing __LINKEDIT segment";
	if ( startOfFileSegCmd == NULL )
		throw "malformed mach-o image: missing __TEXT segment that maps start of file";
	// <rdar://problem/13145644> verify every segment does not overlap another segment
	if ( context.strictMachORequired ) {
		uintptr_t lastFileStart = 0;
		uintptr_t linkeditFileStart = 0;
		const struct load_command* cmd1 = startCmds;
		for (uint32_t i = 0; i < cmd_count; ++i) {
			if ( cmd1->cmd == LC_SEGMENT_COMMAND ) {
				struct macho_segment_command* segCmd1 = (struct macho_segment_command*)cmd1;
				uintptr_t vmStart1   = segCmd1->vmaddr;
				uintptr_t vmEnd1     = segCmd1->vmaddr + segCmd1->vmsize;
				uintptr_t fileStart1 = segCmd1->fileoff;
				uintptr_t fileEnd1   = segCmd1->fileoff + segCmd1->filesize;

				if (fileStart1 > lastFileStart)
					lastFileStart = fileStart1;

				if ( strcmp(&segCmd1->segname[0], "__LINKEDIT") == 0 ) {
					linkeditFileStart = fileStart1;
				}

				const struct load_command* cmd2 = startCmds;
				for (uint32_t j = 0; j < cmd_count; ++j) {
					if ( cmd2 == cmd1 )
						continue;
					if ( cmd2->cmd == LC_SEGMENT_COMMAND ) {
						struct macho_segment_command* segCmd2 = (struct macho_segment_command*)cmd2;
						uintptr_t vmStart2   = segCmd2->vmaddr;
						uintptr_t vmEnd2     = segCmd2->vmaddr + segCmd2->vmsize;
						uintptr_t fileStart2 = segCmd2->fileoff;
						uintptr_t fileEnd2   = segCmd2->fileoff + segCmd2->filesize;
						if ( ((vmStart2 <= vmStart1) && (vmEnd2 > vmStart1) && (vmEnd1 > vmStart1)) 
						|| ((vmStart2 >= vmStart1) && (vmStart2 < vmEnd1) && (vmEnd2 > vmStart2)) )
							dyld::throwf("malformed mach-o image: segment %s vm overlaps segment %s", segCmd1->segname, segCmd2->segname);
						if ( ((fileStart2 <= fileStart1) && (fileEnd2 > fileStart1) && (fileEnd1 > fileStart1))
						  || ((fileStart2 >= fileStart1) && (fileStart2 < fileEnd1) && (fileEnd2 > fileStart2)) )
							dyld::throwf("malformed mach-o image: segment %s file content overlaps segment %s", segCmd1->segname, segCmd2->segname); 
					}
					cmd2 = (const struct load_command*)(((char*)cmd2)+cmd2->cmdsize);
				}
			}
			cmd1 = (const struct load_command*)(((char*)cmd1)+cmd1->cmdsize);
		}

		if (lastFileStart != linkeditFileStart)
			dyld::throwf("malformed mach-o image: __LINKEDIT must be last segment");
	}

	// validate linkedit content
	if  ( (dyldInfoCmd == NULL) && (symTabCmd == NULL) )
		throw "malformed mach-o image: missing LC_SYMTAB or LC_DYLD_INFO";
	if  ( dynSymbTabCmd == NULL )
		throw "malformed mach-o image: missing LC_DYSYMTAB";

	uint32_t  linkeditFileOffsetStart = (uint32_t)linkeditSegCmd->fileoff;
	uint32_t  linkeditFileOffsetEnd = (uint32_t)linkeditSegCmd->fileoff + (uint32_t)linkeditSegCmd->filesize;

	if ( !inCache && (dyldInfoCmd != NULL) && context.strictMachORequired ) {
		// validate all LC_DYLD_INFO chunks fit in LINKEDIT and don't overlap
		uint32_t offset = linkeditFileOffsetStart;
		if ( dyldInfoCmd->rebase_size != 0 ) {
			if ( dyldInfoCmd->rebase_size & 0x80000000 )
				throw "malformed mach-o image: dyld rebase info size overflow";
			if ( dyldInfoCmd->rebase_off < offset )
				throw "malformed mach-o image: dyld rebase info underruns __LINKEDIT";
			offset = dyldInfoCmd->rebase_off + dyldInfoCmd->rebase_size;
			if ( offset > linkeditFileOffsetEnd )
				throw "malformed mach-o image: dyld rebase info overruns __LINKEDIT";
		}
		if ( dyldInfoCmd->bind_size != 0 ) {
			if ( dyldInfoCmd->bind_size & 0x80000000 )
				throw "malformed mach-o image: dyld bind info size overflow";
			if ( dyldInfoCmd->bind_off < offset )
				throw "malformed mach-o image: dyld bind info overlaps rebase info";
			offset = dyldInfoCmd->bind_off + dyldInfoCmd->bind_size;
			if ( offset > linkeditFileOffsetEnd )
				throw "malformed mach-o image: dyld bind info overruns __LINKEDIT";
		}
		if ( dyldInfoCmd->weak_bind_size != 0 ) {
			if ( dyldInfoCmd->weak_bind_size & 0x80000000 )
				throw "malformed mach-o image: dyld weak bind info size overflow";
			if ( dyldInfoCmd->weak_bind_off < offset )
				throw "malformed mach-o image: dyld weak bind info overlaps bind info";
			offset = dyldInfoCmd->weak_bind_off + dyldInfoCmd->weak_bind_size;
			if ( offset > linkeditFileOffsetEnd )
				throw "malformed mach-o image: dyld weak bind info overruns __LINKEDIT";
		}
		if ( dyldInfoCmd->lazy_bind_size != 0 ) {
			if ( dyldInfoCmd->lazy_bind_size & 0x80000000 )
				throw "malformed mach-o image: dyld lazy bind info size overflow";
			if ( dyldInfoCmd->lazy_bind_off < offset )
				throw "malformed mach-o image: dyld lazy bind info overlaps weak bind info";
			offset = dyldInfoCmd->lazy_bind_off + dyldInfoCmd->lazy_bind_size;
			if ( offset > linkeditFileOffsetEnd )
				throw "malformed mach-o image: dyld lazy bind info overruns __LINKEDIT";
		}
		if ( dyldInfoCmd->export_size != 0 ) {
			if ( dyldInfoCmd->export_size & 0x80000000 )
				throw "malformed mach-o image: dyld export info size overflow";
			if ( dyldInfoCmd->export_off < offset )
				throw "malformed mach-o image: dyld export info overlaps lazy bind info";
			offset = dyldInfoCmd->export_off + dyldInfoCmd->export_size;
			if ( offset > linkeditFileOffsetEnd )
				throw "malformed mach-o image: dyld export info overruns __LINKEDIT";
		}
	}

	if ( symTabCmd != NULL ) {
		// validate symbol table fits in LINKEDIT
		if ( symTabCmd->symoff < linkeditFileOffsetStart )
			throw "malformed mach-o image: symbol table underruns __LINKEDIT";
		if ( symTabCmd->nsyms > 0x10000000 )
			throw "malformed mach-o image: symbol table too large";
		uint32_t symbolsSize = symTabCmd->nsyms * sizeof(macho_nlist);
		if ( symbolsSize > linkeditSegCmd->filesize )
			throw "malformed mach-o image: symbol table overruns __LINKEDIT";
		if ( symTabCmd->symoff + symbolsSize < symTabCmd->symoff )
			throw "malformed mach-o image: symbol table size wraps";
		if ( symTabCmd->symoff + symbolsSize > symTabCmd->stroff )
			throw "malformed mach-o image: symbol table overlaps symbol strings";
		if ( symTabCmd->stroff + symTabCmd->strsize < symTabCmd->stroff )
			throw "malformed mach-o image: symbol string size wraps";
		if ( symTabCmd->stroff + symTabCmd->strsize > linkeditFileOffsetEnd ) {
			// <rdar://problem/24220313> let old apps overflow as long as it stays within mapped page
			if ( context.strictMachORequired || (symTabCmd->stroff + symTabCmd->strsize > ((linkeditFileOffsetEnd + 4095) & (-4096))) )
				throw "malformed mach-o image: symbol strings overrun __LINKEDIT";
		}
		// validate indirect symbol table
		if ( dynSymbTabCmd->nindirectsyms != 0 ) {
			if ( dynSymbTabCmd->indirectsymoff < linkeditFileOffsetStart )
				throw "malformed mach-o image: indirect symbol table underruns __LINKEDIT";
			if ( dynSymbTabCmd->nindirectsyms > 0x10000000 )
				throw "malformed mach-o image: indirect symbol table too large";
			uint32_t indirectTableSize = dynSymbTabCmd->nindirectsyms * sizeof(uint32_t);
			if ( indirectTableSize > linkeditSegCmd->filesize )
				throw "malformed mach-o image: indirect symbol table overruns __LINKEDIT";
			if ( dynSymbTabCmd->indirectsymoff + indirectTableSize < dynSymbTabCmd->indirectsymoff )
				throw "malformed mach-o image: indirect symbol table size wraps";
			if ( context.strictMachORequired && (dynSymbTabCmd->indirectsymoff + indirectTableSize > symTabCmd->stroff)  )
				throw "malformed mach-o image: indirect symbol table overruns string pool";
		}
		if ( (dynSymbTabCmd->nlocalsym > symTabCmd->nsyms) || (dynSymbTabCmd->ilocalsym > symTabCmd->nsyms) )
			throw "malformed mach-o image: indirect symbol table local symbol count exceeds total symbols";
		if ( dynSymbTabCmd->ilocalsym + dynSymbTabCmd->nlocalsym < dynSymbTabCmd->ilocalsym  )
			throw "malformed mach-o image: indirect symbol table local symbol count wraps";
		if ( (dynSymbTabCmd->nextdefsym > symTabCmd->nsyms) || (dynSymbTabCmd->iextdefsym > symTabCmd->nsyms) )
			throw "malformed mach-o image: indirect symbol table extern symbol count exceeds total symbols";
		if ( dynSymbTabCmd->iextdefsym + dynSymbTabCmd->nextdefsym < dynSymbTabCmd->iextdefsym  )
			throw "malformed mach-o image: indirect symbol table extern symbol count wraps";
		if ( (dynSymbTabCmd->nundefsym > symTabCmd->nsyms) || (dynSymbTabCmd->iundefsym > symTabCmd->nsyms) )
			throw "malformed mach-o image: indirect symbol table undefined symbol count exceeds total symbols";
		if ( dynSymbTabCmd->iundefsym + dynSymbTabCmd->nundefsym < dynSymbTabCmd->iundefsym  )
			throw "malformed mach-o image: indirect symbol table undefined symbol count wraps";
	}


	// fSegmentsArrayCount is only 8-bits
	if ( *segCount > 255 )
		dyld::throwf("malformed mach-o image: more than 255 segments in %s", path);

	// fSegmentsArrayCount is only 8-bits
	if ( *libCount > 4095 )
		dyld::throwf("malformed mach-o image: more than 4095 dependent libraries in %s", path);

	if ( needsAddedLibSystemDepency(*libCount, mh) )
		*libCount = 1;
}



// create image for main executable
ImageLoader* ImageLoaderMachO::instantiateMainExecutable(const macho_header* mh, uintptr_t slide, const char* path, const LinkContext& context)
{
	//dyld::log("ImageLoader=%ld, ImageLoaderMachO=%ld, ImageLoaderMachOClassic=%ld, ImageLoaderMachOCompressed=%ld\n",
	//	sizeof(ImageLoader), sizeof(ImageLoaderMachO), sizeof(ImageLoaderMachOClassic), sizeof(ImageLoaderMachOCompressed));
	bool compressed;
	unsigned int segCount;
	unsigned int libCount;
	const linkedit_data_command* codeSigCmd;
	const encryption_info_command* encryptCmd;
	sniffLoadCommands(mh, path, false, &compressed, &segCount, &libCount, context, &codeSigCmd, &encryptCmd);
	// instantiate concrete class based on content of load commands
	if ( compressed ) 
		return ImageLoaderMachOCompressed::instantiateMainExecutable(mh, slide, path, segCount, libCount, context);
	else
#if SUPPORT_CLASSIC_MACHO
		return ImageLoaderMachOClassic::instantiateMainExecutable(mh, slide, path, segCount, libCount, context);
#else
		throw "missing LC_DYLD_INFO load command";
#endif
}


// create image by mapping in a mach-o file
ImageLoader* ImageLoaderMachO::instantiateFromFile(const char* path, int fd, const uint8_t firstPages[], size_t firstPagesSize, uint64_t offsetInFat,
									uint64_t lenInFat, const struct stat& info, const LinkContext& context)
{
	bool compressed;
	unsigned int segCount;
	unsigned int libCount;
	const linkedit_data_command* codeSigCmd;
	const encryption_info_command* encryptCmd;
	sniffLoadCommands((const macho_header*)firstPages, path, false, &compressed, &segCount, &libCount, context, &codeSigCmd, &encryptCmd);
	// instantiate concrete class based on content of load commands
	if ( compressed ) 
		return ImageLoaderMachOCompressed::instantiateFromFile(path, fd, firstPages, firstPagesSize, offsetInFat, lenInFat, info, segCount, libCount, codeSigCmd, encryptCmd, context);
	else
#if SUPPORT_CLASSIC_MACHO
		return ImageLoaderMachOClassic::instantiateFromFile(path, fd, firstPages, firstPagesSize, offsetInFat, lenInFat, info, segCount, libCount, codeSigCmd, context);
#else
		throw "missing LC_DYLD_INFO load command";
#endif
}

// create image by using cached mach-o file
ImageLoader* ImageLoaderMachO::instantiateFromCache(const macho_header* mh, const char* path, long slide, const struct stat& info, const LinkContext& context)
{
	// instantiate right concrete class
	bool compressed;
	unsigned int segCount;
	unsigned int libCount;
	const linkedit_data_command* codeSigCmd;
	const encryption_info_command* encryptCmd;
	sniffLoadCommands(mh, path, true, &compressed, &segCount, &libCount, context, &codeSigCmd, &encryptCmd);
	// instantiate concrete class based on content of load commands
	if ( compressed ) 
		return ImageLoaderMachOCompressed::instantiateFromCache(mh, path, slide, info, segCount, libCount, context);
	else
#if SUPPORT_CLASSIC_MACHO
		return ImageLoaderMachOClassic::instantiateFromCache(mh, path, slide, info, segCount, libCount, context);
#else
		throw "missing LC_DYLD_INFO load command";
#endif
}

// create image by copying an in-memory mach-o file
ImageLoader* ImageLoaderMachO::instantiateFromMemory(const char* moduleName, const macho_header* mh, uint64_t len, const LinkContext& context)
{
	bool compressed;
	unsigned int segCount;
	unsigned int libCount;
	const linkedit_data_command* sigcmd;
	const encryption_info_command* encryptCmd;
	sniffLoadCommands(mh, moduleName, false, &compressed, &segCount, &libCount, context, &sigcmd, &encryptCmd);
	// instantiate concrete class based on content of load commands
	if ( compressed ) 
		return ImageLoaderMachOCompressed::instantiateFromMemory(moduleName, mh, len, segCount, libCount, context);
	else
#if SUPPORT_CLASSIC_MACHO
		return ImageLoaderMachOClassic::instantiateFromMemory(moduleName, mh, len, segCount, libCount, context);
#else
		throw "missing LC_DYLD_INFO load command";
#endif
}


int ImageLoaderMachO::crashIfInvalidCodeSignature()
{
	// Now that segments are mapped in, try reading from first executable segment.
	// If code signing is enabled the kernel will validate the code signature
	// when paging in, and kill the process if invalid.
	for(unsigned int i=0; i < fSegmentsCount; ++i) {
		if ( (segFileOffset(i) == 0) && (segFileSize(i) != 0) ) {
			// return read value to ensure compiler does not optimize away load
			int* p = (int*)segActualLoadAddress(i);
			return *p;
		}
	}
	return 0;
}


void ImageLoaderMachO::parseLoadCmds(const LinkContext& context)
{
	// now that segments are mapped in, get real fMachOData, fLinkEditBase, and fSlide
	for(unsigned int i=0; i < fSegmentsCount; ++i) {
		// set up pointer to __LINKEDIT segment
		if ( strcmp(segName(i),"__LINKEDIT") == 0 ) {
			if ( context.requireCodeSignature && (segFileOffset(i) > fCoveredCodeLength))
				dyld::throwf("cannot load '%s' (segment outside of code signature)", this->getShortName());
			fLinkEditBase = (uint8_t*)(segActualLoadAddress(i) - segFileOffset(i));
		}
#if TEXT_RELOC_SUPPORT
		// __TEXT segment always starts at beginning of file and contains mach_header and load commands
		if ( segExecutable(i) ) {
			if ( segHasRebaseFixUps(i) && (fSlide != 0) )
				fTextSegmentRebases = true;
			if ( segHasBindFixUps(i) )
				fTextSegmentBinds = true;
		}
#endif
#if __i386__
		if ( segIsReadOnlyImport(i) )
			fReadOnlyImportSegment = true;
#endif
		// some segment always starts at beginning of file and contains mach_header and load commands
		if ( (segFileOffset(i) == 0) && (segFileSize(i) != 0) ) {
			fMachOData = (uint8_t*)(segActualLoadAddress(i));
		}
	}
	
	// keep count of prebound images with weak exports
	if ( this->participatesInCoalescing() ) {
		++fgImagesRequiringCoalescing;
		fRegisteredAsRequiresCoalescing = true;
		if ( this->hasCoalescedExports() ) 
			++fgImagesHasWeakDefinitions;
	}

	// keep count of images used in shared cache
	if ( fInSharedCache )
		++fgImagesUsedFromSharedCache;

	// walk load commands (mapped in at start of __TEXT segment)
	const dyld_info_command* dyldInfo = NULL;
	const macho_nlist* symbolTable = NULL;
	const char* symbolTableStrings = NULL;
	const struct load_command* firstUnknownCmd = NULL;
	const struct version_min_command* minOSVersionCmd = NULL;
	const dysymtab_command* dynSymbolTable = NULL;
	const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
	const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_SYMTAB:
				{
					const struct symtab_command* symtab = (struct symtab_command*)cmd;
					symbolTableStrings = (const char*)&fLinkEditBase[symtab->stroff];
					symbolTable = (macho_nlist*)(&fLinkEditBase[symtab->symoff]);
				}
				break;
			case LC_DYSYMTAB:
				dynSymbolTable = (struct dysymtab_command*)cmd;
				break;
			case LC_SUB_UMBRELLA:
				fHasSubUmbrella = true;
				break;
			case LC_SUB_FRAMEWORK:
				fInUmbrella = true;
				break;
			case LC_SUB_LIBRARY:
				fHasSubLibraries = true;
				break;
			case LC_ROUTINES_COMMAND:
				fHasDashInit = true;
				break;
			case LC_DYLD_INFO:
			case LC_DYLD_INFO_ONLY:
				dyldInfo = (struct dyld_info_command*)cmd;
				break;
			case LC_SEGMENT_COMMAND:
				{
					const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
					const bool isTextSeg = (strcmp(seg->segname, "__TEXT") == 0);
		#if __i386__ && __MAC_OS_X_VERSION_MIN_REQUIRED
					const bool isObjCSeg = (strcmp(seg->segname, "__OBJC") == 0);
					if ( isObjCSeg )
						fNotifyObjC = true;
		#else
					const bool isDataSeg = (strncmp(seg->segname, "__DATA", 6) == 0);
		#endif
					const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
					const struct macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
					for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
						const uint8_t type = sect->flags & SECTION_TYPE;
						if ( type == S_MOD_INIT_FUNC_POINTERS )
							fHasInitializers = true;
						else if ( type == S_MOD_TERM_FUNC_POINTERS )
							fHasTerminators = true;
						else if ( type == S_DTRACE_DOF )
							fHasDOFSections = true;
						else if ( isTextSeg && (strcmp(sect->sectname, "__eh_frame") == 0) )
							fEHFrameSectionOffset = (uint32_t)((uint8_t*)sect - fMachOData);
						else if ( isTextSeg && (strcmp(sect->sectname, "__unwind_info") == 0) )
							fUnwindInfoSectionOffset = (uint32_t)((uint8_t*)sect - fMachOData);

		#if __i386__ && __MAC_OS_X_VERSION_MIN_REQUIRED
						else if ( isObjCSeg ) {
							if ( strcmp(sect->sectname, "__image_info") == 0 ) {
								const uint32_t* imageInfo = (uint32_t*)(sect->addr + fSlide);
								uint32_t flags = imageInfo[1];
								if ( (flags & 4) && (((macho_header*)fMachOData)->filetype != MH_EXECUTE) )
									dyld::throwf("cannot load '%s' because Objective-C garbage collection is not supported", getPath());
							}
							else if ( ((macho_header*)fMachOData)->filetype == MH_DYLIB ) {
								fRetainForObjC = true;
							}
						}
		#else
						else if ( isDataSeg && (strncmp(sect->sectname, "__objc_imageinfo", 16) == 0) ) {
			#if __MAC_OS_X_VERSION_MIN_REQUIRED
							const uint32_t* imageInfo = (uint32_t*)(sect->addr + fSlide);
							uint32_t flags = imageInfo[1];
							if ( (flags & 4) && (((macho_header*)fMachOData)->filetype != MH_EXECUTE) )
								dyld::throwf("cannot load '%s' because Objective-C garbage collection is not supported", getPath());
			#endif
							fNotifyObjC = true;
						}
						else if ( isDataSeg && (strncmp(sect->sectname, "__objc_", 7) == 0) && (((macho_header*)fMachOData)->filetype == MH_DYLIB) )
							fRetainForObjC = true;
		#endif
					}
				}
				break;
			case LC_TWOLEVEL_HINTS:
				// no longer supported
				break;
			case LC_ID_DYLIB:
				{
					fDylibIDOffset = (uint32_t)((uint8_t*)cmd - fMachOData);
				}
				break;
			case LC_RPATH:
			case LC_LOAD_WEAK_DYLIB:
		    case LC_REEXPORT_DYLIB:
			case LC_LOAD_UPWARD_DYLIB:
			case LC_MAIN:
				// do nothing, just prevent LC_REQ_DYLD exception from occuring
				break;
			case LC_VERSION_MIN_MACOSX:
			case LC_VERSION_MIN_IPHONEOS:
			case LC_VERSION_MIN_TVOS:
			case LC_VERSION_MIN_WATCHOS:
				minOSVersionCmd = (version_min_command*)cmd;
				break;
			default:
				if ( (cmd->cmd & LC_REQ_DYLD) != 0 ) {
					if ( firstUnknownCmd == NULL )
						firstUnknownCmd = cmd;
				}
				break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	if ( firstUnknownCmd != NULL ) {
		if ( minOSVersionCmd != NULL )  {
			dyld::throwf("cannot load '%s' because it was built for OS version %u.%u (load command 0x%08X is unknown)", 
						 this->getShortName(),
						 minOSVersionCmd->version >> 16, ((minOSVersionCmd->version >> 8) & 0xff), 
						 firstUnknownCmd->cmd);
		}
		else {
			dyld::throwf("cannot load '%s' (load command 0x%08X is unknown)", this->getShortName(), firstUnknownCmd->cmd);
		}
	}
	
	
	if ( dyldInfo != NULL )
		this->setDyldInfo(dyldInfo);
	if ( symbolTable != NULL)
		this->setSymbolTableInfo(symbolTable, symbolTableStrings, dynSymbolTable);
	
}

// don't do this work in destructor because we need object to be full subclass
// for UnmapSegments() to work
void ImageLoaderMachO::destroy()
{
	// update count of images with weak exports
	if ( fRegisteredAsRequiresCoalescing ) {
		--fgImagesRequiringCoalescing;
		if ( this->hasCoalescedExports() ) 
			--fgImagesHasWeakDefinitions;
	}

	// keep count of images used in shared cache
	if ( fInSharedCache )
		--fgImagesUsedFromSharedCache;
		
	// unmap image when done
	UnmapSegments();
}


unsigned int ImageLoaderMachO::segmentCount() const
{
	return fSegmentsCount;
}


const macho_segment_command* ImageLoaderMachO::segLoadCommand(unsigned int segIndex) const
{
	uint32_t* lcOffsets = this->segmentCommandOffsets();
	uint32_t lcOffset =	lcOffsets[segIndex];
	return (macho_segment_command*)(&fMachOData[lcOffset]);
}

const char*	ImageLoaderMachO::segName(unsigned int segIndex) const
{
	return segLoadCommand(segIndex)->segname;
}


uintptr_t ImageLoaderMachO::segSize(unsigned int segIndex) const
{
	return segLoadCommand(segIndex)->vmsize;
}


uintptr_t ImageLoaderMachO::segFileSize(unsigned int segIndex) const
{
	return segLoadCommand(segIndex)->filesize;
}


bool ImageLoaderMachO::segHasTrailingZeroFill(unsigned int segIndex)
{
	return ( segWriteable(segIndex) && (segSize(segIndex) > segFileSize(segIndex)) );
}


uintptr_t ImageLoaderMachO::segFileOffset(unsigned int segIndex) const
{
	return segLoadCommand(segIndex)->fileoff;
}


bool ImageLoaderMachO::segReadable(unsigned int segIndex) const
{
	return ( (segLoadCommand(segIndex)->initprot & VM_PROT_READ) != 0);
}


bool ImageLoaderMachO::segWriteable(unsigned int segIndex) const
{
	return ( (segLoadCommand(segIndex)->initprot & VM_PROT_WRITE) != 0);
}


bool ImageLoaderMachO::segExecutable(unsigned int segIndex) const
{
	return ( (segLoadCommand(segIndex)->initprot & VM_PROT_EXECUTE) != 0);
}


bool ImageLoaderMachO::segUnaccessible(unsigned int segIndex) const
{
	return (segLoadCommand(segIndex)->initprot == 0);
}

bool ImageLoaderMachO::segHasPreferredLoadAddress(unsigned int segIndex) const
{
	return (segLoadCommand(segIndex)->vmaddr != 0);
}

uintptr_t ImageLoaderMachO::segPreferredLoadAddress(unsigned int segIndex) const
{
	return segLoadCommand(segIndex)->vmaddr;
}

uintptr_t ImageLoaderMachO::segActualLoadAddress(unsigned int segIndex) const
{
	return segLoadCommand(segIndex)->vmaddr + fSlide;
}


uintptr_t ImageLoaderMachO::segActualEndAddress(unsigned int segIndex) const
{
	return segActualLoadAddress(segIndex) + segSize(segIndex);
}

bool ImageLoaderMachO::segHasRebaseFixUps(unsigned int segIndex) const
{
#if TEXT_RELOC_SUPPORT
	// scan sections for fix-up bit
	const macho_segment_command* segCmd = segLoadCommand(segIndex);
	const struct macho_section* const sectionsStart = (struct macho_section*)((char*)segCmd + sizeof(struct macho_segment_command));
	const struct macho_section* const sectionsEnd = &sectionsStart[segCmd->nsects];
	for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
		if ( (sect->flags & S_ATTR_LOC_RELOC) != 0 )
			return true;
	}
#endif
	return false;
}

bool ImageLoaderMachO::segHasBindFixUps(unsigned int segIndex) const
{
#if TEXT_RELOC_SUPPORT
	// scan sections for fix-up bit
	const macho_segment_command* segCmd = segLoadCommand(segIndex);
	const struct macho_section* const sectionsStart = (struct macho_section*)((char*)segCmd + sizeof(struct macho_segment_command));
	const struct macho_section* const sectionsEnd = &sectionsStart[segCmd->nsects];
	for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
		if ( (sect->flags & S_ATTR_EXT_RELOC) != 0 )
			return true;
	}
#endif
	return false;
}

#if __i386__
bool ImageLoaderMachO::segIsReadOnlyImport(unsigned int segIndex) const
{
	const macho_segment_command* segCmd = segLoadCommand(segIndex);
	return (    (segCmd->initprot & VM_PROT_EXECUTE) 
			&& ((segCmd->initprot & VM_PROT_WRITE) == 0) 
			&& (strcmp(segCmd->segname, "__IMPORT") == 0) );
}
#endif


void ImageLoaderMachO::UnmapSegments()
{
	// usually unmap image when done
	if ( ! this->leaveMapped() && (this->getState() >= dyld_image_state_mapped) ) {
		// unmap TEXT segment last because it contains load command being inspected
		unsigned int textSegmentIndex = 0;
		for(unsigned int i=0; i < fSegmentsCount; ++i) {
			//dyld::log("unmap %s at 0x%08lX\n", seg->getName(), seg->getActualLoadAddress(this));
			if ( (segFileOffset(i) == 0) && (segFileSize(i) != 0) ) {
				textSegmentIndex = i;
			}
			else {
				// update stats
				--ImageLoader::fgTotalSegmentsMapped;
				ImageLoader::fgTotalBytesMapped -= segSize(i);
				munmap((void*)segActualLoadAddress(i), segSize(i));
			}
		}
		// now unmap TEXT
		--ImageLoader::fgTotalSegmentsMapped;
		ImageLoader::fgTotalBytesMapped -= segSize(textSegmentIndex);
		munmap((void*)segActualLoadAddress(textSegmentIndex), segSize(textSegmentIndex));
	}
}


// prefetch __DATA/__OBJC pages during launch, but not for dynamically loaded code
void ImageLoaderMachO::preFetchDATA(int fd, uint64_t offsetInFat, const LinkContext& context)
{
	if ( context.linkingMainExecutable ) {
		for(unsigned int i=0, e=segmentCount(); i < e; ++i) {
			if ( segWriteable(i) && (segFileSize(i) > 0) ) {
				// prefetch writable segment that have mmap'ed regions
				radvisory advice;
				advice.ra_offset = offsetInFat + segFileOffset(i);
				advice.ra_count = (int)segFileSize(i);
				// limit prefetch to 1MB (256 pages)
				if ( advice.ra_count > 1024*1024 )
					advice.ra_count = 1024*1024;
				// don't prefetch single pages, let them fault in
				fgTotalBytesPreFetched += advice.ra_count;
				fcntl(fd, F_RDADVISE, &advice);
				if ( context.verboseMapping ) {
					dyld::log("%18s prefetching 0x%0lX -> 0x%0lX\n", 
						segName(i), segActualLoadAddress(i), segActualLoadAddress(i)+advice.ra_count-1);
				}
			}
		}
	}
}


bool ImageLoaderMachO::segmentsMustSlideTogether() const 
{
	return true;
}

bool ImageLoaderMachO::segmentsCanSlide() const 
{
	return (this->isDylib() || this->isBundle() || this->isPositionIndependentExecutable());
}

bool ImageLoaderMachO::isBundle() const 
{
	const macho_header* mh = (macho_header*)fMachOData;
	return ( mh->filetype == MH_BUNDLE );
}

bool ImageLoaderMachO::isDylib() const 
{
	const macho_header* mh = (macho_header*)fMachOData;
	return ( mh->filetype == MH_DYLIB );
}

bool ImageLoaderMachO::isExecutable() const 
{
	const macho_header* mh = (macho_header*)fMachOData;
	return ( mh->filetype == MH_EXECUTE );
}

bool ImageLoaderMachO::isPositionIndependentExecutable() const 
{
	const macho_header* mh = (macho_header*)fMachOData;
	return ( (mh->filetype == MH_EXECUTE) && ((mh->flags & MH_PIE) != 0) );
}


bool ImageLoaderMachO::forceFlat() const 
{
	const macho_header* mh = (macho_header*)fMachOData;
	return ( (mh->flags & MH_FORCE_FLAT) != 0 );
}

bool ImageLoaderMachO::usesTwoLevelNameSpace() const
{
	const macho_header* mh = (macho_header*)fMachOData;
	return ( (mh->flags & MH_TWOLEVEL) != 0 );
}

bool ImageLoaderMachO::isPrebindable() const 
{
	const macho_header* mh = (macho_header*)fMachOData;
	return ( (mh->flags & MH_PREBOUND) != 0 );
}

bool ImageLoaderMachO::hasCoalescedExports() const 
{
	const macho_header* mh = (macho_header*)fMachOData;
	return ( (mh->flags & MH_WEAK_DEFINES) != 0 );
}

bool ImageLoaderMachO::hasReferencesToWeakSymbols() const 
{
	const macho_header* mh = (macho_header*)fMachOData;
	return ( (mh->flags & MH_BINDS_TO_WEAK) != 0 );
}

bool ImageLoaderMachO::participatesInCoalescing() const 
{
	const macho_header* mh = (macho_header*)fMachOData;
	// if image is loaded with RTLD_LOCAL, then its symbols' visibility
	// is reduced and it can't coalesce with other images
	if ( this->hasHiddenExports() )
		return false;
	return ( (mh->flags & (MH_WEAK_DEFINES|MH_BINDS_TO_WEAK)) != 0 );
}



void ImageLoaderMachO::setSlide(intptr_t slide)
{
	fSlide = slide;
}

void ImageLoaderMachO::loadCodeSignature(const struct linkedit_data_command* codeSigCmd, int fd,  uint64_t offsetInFatFile, const LinkContext& context)
{
	// if dylib being loaded has no code signature load command
	if ( codeSigCmd == NULL) {
		if (context.requireCodeSignature ) {
			// if we require dylibs to be codesigned there needs to be a signature.
			dyld::throwf("required code signature missing for '%s'\n", this->getPath());
		} else {
			disableCoverageCheck();
		}
	}
	else {
#if __MAC_OS_X_VERSION_MIN_REQUIRED
		// <rdar://problem/13622786> ignore code signatures in binaries built with pre-10.9 tools
		if ( this->sdkVersion() < DYLD_MACOSX_VERSION_10_9 ) {
			return;
		}
#endif
		fsignatures_t siginfo;
		siginfo.fs_file_start=offsetInFatFile;				// start of mach-o slice in fat file 
		siginfo.fs_blob_start=(void*)(long)(codeSigCmd->dataoff);	// start of CD in mach-o file
		siginfo.fs_blob_size=codeSigCmd->datasize;			// size of CD
		int result = fcntl(fd, F_ADDFILESIGS_RETURN, &siginfo);

#if TARGET_IPHONE_SIMULATOR
		// rdar://problem/18759224> check range covered by the code directory after loading
		// Attempt to fallback only if we are in the simulator

		if ( result == -1 ) {
			result = fcntl(fd, F_ADDFILESIGS, &siginfo);
			siginfo.fs_file_start = codeSigCmd->dataoff;
		}
#endif

		if ( result == -1 ) {
			if ( (errno == EPERM) || (errno == EBADEXEC) )
				dyld::throwf("code signature invalid for '%s'\n", this->getPath());
			if ( context.verboseCodeSignatures ) 
				dyld::log("dyld: Failed registering code signature for %s, errno=%d\n", this->getPath(), errno);
			siginfo.fs_file_start = UINT64_MAX;
		} else if ( context.verboseCodeSignatures )  {
			dyld::log("dyld: Registered code signature for %s\n", this->getPath());
		}
		fCoveredCodeLength = siginfo.fs_file_start;

#if __MAC_OS_X_VERSION_MIN_REQUIRED
		if ( context.processUsingLibraryValidation ) {
			fchecklv checkInfo;
			char  messageBuffer[512];
			messageBuffer[0] = '\0';
			checkInfo.lv_file_start = offsetInFatFile;
			checkInfo.lv_error_message_size = sizeof(messageBuffer);
			checkInfo.lv_error_message = messageBuffer;
			int res = fcntl(fd, F_CHECK_LV, &checkInfo);
			if ( res == -1 ) {
				dyld::throwf("code signature in (%s) not valid for use in process using Library Validation: %s", this->getPath(), messageBuffer);
			}
		}
#endif
	}
}

void ImageLoaderMachO::validateFirstPages(const struct linkedit_data_command* codeSigCmd, int fd, const uint8_t *fileData, size_t lenFileData, off_t offsetInFat, const LinkContext& context)
{
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	// rdar://problem/21839703> 15A226d: dyld crashes in mageLoaderMachO::validateFirstPages during dlopen() after encountering an mmap failure
	// We need to ignore older code signatures because they will be bad.
	if ( this->sdkVersion() < DYLD_MACOSX_VERSION_10_9 ) {
		return;
	}
#endif
	if (codeSigCmd != NULL) {
		void *fdata = xmmap(NULL, lenFileData, PROT_READ | PROT_EXEC, MAP_SHARED, fd, offsetInFat);
		if ( fdata == MAP_FAILED ) {
#if __MAC_OS_X_VERSION_MIN_REQUIRED
			if ( context.processUsingLibraryValidation ) {
				dyld::throwf("cannot load image with wrong team ID in process using Library Validation");
			}
			else
#endif
			{
				int errnoCopy = errno;
				if ( errnoCopy == EPERM ) {
					if ( dyld::sandboxBlockedMmap(getPath()) )
						dyld::throwf("file system sandbox blocked mmap() of '%s'", getPath());
					else
						dyld::throwf("code signing blocked mmap() of '%s'", getPath());
				}
				else
					dyld::throwf("mmap() errno=%d validating first page of '%s'", errnoCopy, getPath());
			}
		}
		if ( memcmp(fdata, fileData, lenFileData) != 0 )
			dyld::throwf("mmap() page compare failed for '%s'", getPath());
		munmap(fdata, lenFileData);
	}
}


const char* ImageLoaderMachO::getInstallPath() const
{
	if ( fDylibIDOffset != 0 ) {
		const dylib_command* dylibID = (dylib_command*)(&fMachOData[fDylibIDOffset]);
		return (char*)dylibID + dylibID->dylib.name.offset;
	}
	return NULL;
}

void ImageLoaderMachO::registerInterposing()
{
	// mach-o files advertise interposing by having a __DATA __interpose section
	struct InterposeData { uintptr_t replacement; uintptr_t replacee; };
	const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
	const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_SEGMENT_COMMAND:
				{
					const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
					const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
					const struct macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
					for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
						if ( ((sect->flags & SECTION_TYPE) == S_INTERPOSING) || ((strcmp(sect->sectname, "__interpose") == 0) && (strcmp(seg->segname, "__DATA") == 0)) ) {
							// <rdar://problem/23929217> Ensure section is within segment
							if ( (sect->addr < seg->vmaddr) || (sect->addr+sect->size > seg->vmaddr+seg->vmsize) || (sect->addr+sect->size < sect->addr) )
								dyld::throwf("interpose section has malformed address range for %s\n", this->getPath());
							const InterposeData* interposeArray = (InterposeData*)(sect->addr + fSlide);
							const size_t count = sect->size / sizeof(InterposeData);
							for (size_t j=0; j < count; ++j) {
								ImageLoader::InterposeTuple tuple;
								tuple.replacement		= interposeArray[j].replacement;
								tuple.neverImage		= this;
								tuple.onlyImage		    = NULL;
								tuple.replacee			= interposeArray[j].replacee;
								// <rdar://problem/25686570> ignore interposing on a weak function that does not exist
								if ( tuple.replacee == 0 )
									continue;
								// <rdar://problem/7937695> verify that replacement is in this image
								if ( this->containsAddress((void*)tuple.replacement) ) {
									// chain to any existing interpositions
									for (std::vector<InterposeTuple>::iterator it=fgInterposingTuples.begin(); it != fgInterposingTuples.end(); it++) {
										if ( it->replacee == tuple.replacee ) {
											tuple.replacee = it->replacement;
										}
									}
									ImageLoader::fgInterposingTuples.push_back(tuple);
								}
							}
						}
					}
				}
				break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
}

uint32_t ImageLoaderMachO::sdkVersion(const mach_header* mh)
{
	const uint32_t cmd_count = mh->ncmds;
	const struct load_command* const cmds = (struct load_command*)(((char*)mh) + sizeof(macho_header));
	const struct load_command* cmd = cmds;
	const struct version_min_command* versCmd;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch ( cmd->cmd ) {
			case LC_VERSION_MIN_MACOSX:
			case LC_VERSION_MIN_IPHONEOS:
			case LC_VERSION_MIN_TVOS:
			case LC_VERSION_MIN_WATCHOS:
				versCmd = (version_min_command*)cmd;
				return versCmd->sdk;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	return 0;
}

uint32_t ImageLoaderMachO::sdkVersion() const
{
	return ImageLoaderMachO::sdkVersion(machHeader());
}

uint32_t ImageLoaderMachO::minOSVersion(const mach_header* mh)
{
	const uint32_t cmd_count = mh->ncmds;
	const struct load_command* const cmds = (struct load_command*)(((char*)mh) + sizeof(macho_header));
	const struct load_command* cmd = cmds;
	const struct version_min_command* versCmd;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch ( cmd->cmd ) {
			case LC_VERSION_MIN_MACOSX:
			case LC_VERSION_MIN_IPHONEOS:
			case LC_VERSION_MIN_TVOS:
			case LC_VERSION_MIN_WATCHOS:
				versCmd = (version_min_command*)cmd;
				return versCmd->version;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	return 0;
}

uint32_t ImageLoaderMachO::minOSVersion() const
{
	return ImageLoaderMachO::minOSVersion(machHeader());
}


void* ImageLoaderMachO::getThreadPC() const
{
	const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
	const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		if ( cmd->cmd == LC_MAIN ) {
			entry_point_command* mainCmd = (entry_point_command*)cmd;
			void* entry = (void*)(mainCmd->entryoff + (char*)fMachOData);
			// <rdar://problem/8543820&9228031> verify entry point is in image
			if ( this->containsAddress(entry) )
				return entry;
			else
				throw "LC_MAIN entryoff is out of range";
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	return NULL;
}


void* ImageLoaderMachO::getMain() const
{
	const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
	const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_UNIXTHREAD:
			{
			#if __i386__
				const i386_thread_state_t* registers = (i386_thread_state_t*)(((char*)cmd) + 16);
				void* entry = (void*)(registers->eip + fSlide);
			#elif __x86_64__
				const x86_thread_state64_t* registers = (x86_thread_state64_t*)(((char*)cmd) + 16);
				void* entry = (void*)(registers->rip + fSlide);
			#elif __arm__
				const arm_thread_state_t* registers = (arm_thread_state_t*)(((char*)cmd) + 16);
				void* entry = (void*)(registers->__pc + fSlide);
			#elif __arm64__
				const arm_thread_state64_t* registers = (arm_thread_state64_t*)(((char*)cmd) + 16);
				void* entry = (void*)(registers->__pc + fSlide);
			#else
				#warning need processor specific code
			#endif
				// <rdar://problem/8543820&9228031> verify entry point is in image
				if ( this->containsAddress(entry) ) {
					return entry;
				}
			}
			break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	throw "no valid entry point";
}

bool ImageLoaderMachO::needsAddedLibSystemDepency(unsigned int libCount, const macho_header* mh)
{
	// <rdar://problem/6357561> ensure that every image depends on something which depends on libSystem
	if ( libCount > 1 )
		return false;
		
	// <rdar://problem/6409800> dyld implicit-libSystem breaks valgrind
	if ( mh->filetype == MH_EXECUTE ) 
		return false;
	
	bool isNonOSdylib = false;
	const uint32_t cmd_count = mh->ncmds;
	const struct load_command* const cmds = (struct load_command*)((uint8_t*)mh+sizeof(macho_header));
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_LOAD_DYLIB:
			case LC_LOAD_WEAK_DYLIB:
			case LC_REEXPORT_DYLIB:
			case LC_LOAD_UPWARD_DYLIB:
				return false;
			case LC_ID_DYLIB:
				{
				const dylib_command* dylibID = (dylib_command*)cmd;
				const char* installPath = (char*)cmd + dylibID->dylib.name.offset;
				// It is OK for OS dylibs (libSystem or libmath or Rosetta shims) to have no dependents
				// but all other dylibs must depend on libSystem for initialization to initialize libSystem first
				// <rdar://problem/6497528> rosetta circular dependency spew
				isNonOSdylib = ( (strncmp(installPath, "/usr/lib/", 9) != 0) && (strncmp(installPath, "/usr/libexec/oah/Shims", 9) != 0) );
				}
				break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	return isNonOSdylib;
}


void ImageLoaderMachO::doGetDependentLibraries(DependentLibraryInfo libs[])
{
	if ( needsAddedLibSystemDepency(libraryCount(), (macho_header*)fMachOData) ) {
		DependentLibraryInfo* lib = &libs[0];
		lib->name = LIBSYSTEM_DYLIB_PATH;
		lib->info.checksum = 0;
		lib->info.minVersion = 0;
		lib->info.maxVersion = 0;
		lib->required = false;
		lib->reExported = false;
		lib->upward = false;
	}
	else {
		uint32_t index = 0;
		const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
		const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
		const struct load_command* cmd = cmds;
		for (uint32_t i = 0; i < cmd_count; ++i) {
			switch (cmd->cmd) {
				case LC_LOAD_DYLIB:
				case LC_LOAD_WEAK_DYLIB:
				case LC_REEXPORT_DYLIB:
				case LC_LOAD_UPWARD_DYLIB:
				{
					const struct dylib_command* dylib = (struct dylib_command*)cmd;
					DependentLibraryInfo* lib = &libs[index++];
					lib->name = (char*)cmd + dylib->dylib.name.offset;
					//lib->name = strdup((char*)cmd + dylib->dylib.name.offset);
					lib->info.checksum = dylib->dylib.timestamp;
					lib->info.minVersion = dylib->dylib.compatibility_version;
					lib->info.maxVersion = dylib->dylib.current_version;
					lib->required = (cmd->cmd != LC_LOAD_WEAK_DYLIB);
					lib->reExported = (cmd->cmd == LC_REEXPORT_DYLIB);
					lib->upward = (cmd->cmd == LC_LOAD_UPWARD_DYLIB);
				}
				break;
			}
			cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
		}
	}
}

ImageLoader::LibraryInfo ImageLoaderMachO::doGetLibraryInfo(const LibraryInfo&)
{
	LibraryInfo info;
	if ( fDylibIDOffset != 0 ) {
		const dylib_command* dylibID = (dylib_command*)(&fMachOData[fDylibIDOffset]);
		info.minVersion = dylibID->dylib.compatibility_version;
		info.maxVersion = dylibID->dylib.current_version;
		info.checksum = dylibID->dylib.timestamp;
	}
	else {
		info.minVersion = 0;
		info.maxVersion = 0;		
		info.checksum = 0;
	}
	return info;
}

void ImageLoaderMachO::getRPaths(const LinkContext& context, std::vector<const char*>& paths) const
{
	const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
	const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_RPATH:
				const char* pathToAdd = NULL;
				const char* path = (char*)cmd + ((struct rpath_command*)cmd)->path.offset;
				if ( (strncmp(path, "@loader_path", 12) == 0) && ((path[12] == '/') || (path[12] == '\0')) ) {
#if __MAC_OS_X_VERSION_MIN_REQUIRED
					if ( context.processIsRestricted && (context.mainExecutable == this) ) {
						dyld::warn("LC_RPATH %s in %s being ignored in restricted program because of @loader_path\n", path, this->getPath());
						break;
					}
#endif
					char resolvedPath[PATH_MAX];
					if ( realpath(this->getPath(), resolvedPath) != NULL ) {
						char newRealPath[strlen(resolvedPath) + strlen(path)];
						strcpy(newRealPath, resolvedPath);
						char* addPoint = strrchr(newRealPath,'/');
						if ( addPoint != NULL ) {
							strcpy(addPoint, &path[12]);
							pathToAdd = strdup(newRealPath);
						}
					}
				}
				else if ( (strncmp(path, "@executable_path", 16) == 0) && ((path[16] == '/') || (path[16] == '\0')) ) {
#if __MAC_OS_X_VERSION_MIN_REQUIRED
					if ( context.processIsRestricted ) {
						dyld::warn("LC_RPATH %s in %s being ignored in restricted program because of @executable_path\n", path, this->getPath());
						break;
					}
#endif
					char resolvedPath[PATH_MAX];
					if ( realpath(context.mainExecutable->getPath(), resolvedPath) != NULL ) {
						char newRealPath[strlen(resolvedPath) + strlen(path)];
						strcpy(newRealPath, resolvedPath);
						char* addPoint = strrchr(newRealPath,'/');
						if ( addPoint != NULL ) {
							strcpy(addPoint, &path[16]);
							pathToAdd = strdup(newRealPath);
						}
					}
				}
#if __MAC_OS_X_VERSION_MIN_REQUIRED
				else if ( (path[0] != '/') && context.processIsRestricted ) {
					dyld::warn("LC_RPATH %s in %s being ignored in restricted program because it is a relative path\n", path, this->getPath());
					break;
				}
#endif
#if SUPPORT_ROOT_PATH
				else if ( (path[0] == '/') && (context.rootPaths != NULL) ) {
					// <rdar://problem/5869973> DYLD_ROOT_PATH should apply to LC_RPATH rpaths
					// DYLD_ROOT_PATH can be a list of paths, but at this point we can only support one, so use first combination that exists
					bool found = false;
					for(const char** rp = context.rootPaths; *rp != NULL; ++rp) {
						char newPath[PATH_MAX];
						strlcpy(newPath, *rp, PATH_MAX);
						strlcat(newPath, path, PATH_MAX);
						struct stat stat_buf;
						if ( stat(newPath, &stat_buf) != -1 ) {
							//dyld::log("combined DYLD_ROOT_PATH and LC_RPATH: %s\n", newPath);
							pathToAdd = strdup(newPath);
							found = true;
							break;
						}
					}
					if ( ! found ) {
						// make copy so that all elements of 'paths' can be freed
						pathToAdd = strdup(path);
					}
				}
#endif
				else {
					// make copy so that all elements of 'paths' can be freed
					pathToAdd = strdup(path);
				}
				if ( pathToAdd != NULL )
					paths.push_back(pathToAdd);
				break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
}


bool ImageLoaderMachO::getUUID(uuid_t uuid) const
{
	const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
	const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_UUID:
				uuid_command* uc = (uuid_command*)cmd;
				memcpy(uuid, uc->uuid, 16);
				return true;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	bzero(uuid, 16);
	return false;
}

void ImageLoaderMachO::doRebase(const LinkContext& context)
{
	// <rdar://problem/25329861> Delay calling setNeverUnload() until we know this is not for dlopen_preflight()
	if ( fRetainForObjC )
		this->setNeverUnload();

	// if prebound and loaded at prebound address, then no need to rebase
	if ( this->usablePrebinding(context) ) {
		// skip rebasing because prebinding is valid
		++fgImagesWithUsedPrebinding; // bump totals for statistics
		return;
	}

	// print why prebinding was not used
	if ( context.verbosePrebinding ) {
		if ( !this->isPrebindable() ) {
			dyld::log("dyld: image not prebound, so could not use prebinding in %s\n", this->getPath());
		}
		else if ( fSlide != 0 ) {
			dyld::log("dyld: image slid, so could not use prebinding in %s\n", this->getPath());
		}
		else if ( !this->allDependentLibrariesAsWhenPreBound() ) {
			dyld::log("dyld: dependent libraries changed, so could not use prebinding in %s\n", this->getPath());
		}
		else if ( !this->usesTwoLevelNameSpace() ){
			dyld::log("dyld: image uses flat-namespace so, parts of prebinding ignored %s\n", this->getPath());
		}
		else {
			dyld::log("dyld: environment variable disabled use of prebinding in %s\n", this->getPath());
		}
	}

	//dyld::log("slide=0x%08lX for %s\n", slide, this->getPath());

#if PREBOUND_IMAGE_SUPPORT
	// if prebound and we got here, then prebinding is not valid, so reset all lazy pointers
	// if this image is in the shared cache, do not reset, they will be bound in doBind()
	if ( this->isPrebindable() && !fInSharedCache )
		this->resetPreboundLazyPointers(context);
#endif

	// if loaded at preferred address, no rebasing necessary
	if ( this->fSlide == 0 ) 
		return;

#if TEXT_RELOC_SUPPORT
	// if there are __TEXT fixups, temporarily make __TEXT writable
	if ( fTextSegmentRebases ) 
		this->makeTextSegmentWritable(context, true);
#endif

	// do actual rebasing
	this->rebase(context, fSlide);
			
#if TEXT_RELOC_SUPPORT
	// if there were __TEXT fixups, restore write protection
	if ( fTextSegmentRebases ) 
		this->makeTextSegmentWritable(context, false);
	
#endif	
}

#if TEXT_RELOC_SUPPORT
void ImageLoaderMachO::makeTextSegmentWritable(const LinkContext& context, bool writeable)
{
	for(unsigned int i=0; i < fSegmentsCount; ++i) {
		if ( segExecutable(i) ) {
			if ( writeable ) {
				segMakeWritable(i, context);
			}
			else {
			#if !__i386__ && !__x86_64__
				// some processors require range to be invalidated before it is made executable
				sys_icache_invalidate((void*)segActualLoadAddress(i), segSize(textSegmentIndex));
			#endif
				segProtect(i, context);
			}
		}
	}

}
#endif

const ImageLoader::Symbol* ImageLoaderMachO::findExportedSymbol(const char* name, bool searchReExports, const char* thisPath, const ImageLoader** foundIn) const
{
	// look in this image first
	const ImageLoader::Symbol* result = this->findShallowExportedSymbol(name, foundIn);
	if ( result != NULL )
		return result;
	
	if ( searchReExports ) {
		for(unsigned int i=0; i < libraryCount(); ++i){
			if ( libReExported(i) ) {
				ImageLoader* image = libImage(i);
				if ( image != NULL ) {
					const char* reExPath = libPath(i);
					result = image->findExportedSymbol(name, searchReExports, reExPath, foundIn);
					if ( result != NULL )
						return result;
				}
			}
		}
	}
	

	return NULL;
}



uintptr_t ImageLoaderMachO::getExportedSymbolAddress(const Symbol* sym, const LinkContext& context, 
											const ImageLoader* requestor, bool runResolver, const char* symbolName) const
{
	return this->getSymbolAddress(sym, requestor, context, runResolver);
}

uintptr_t ImageLoaderMachO::getSymbolAddress(const Symbol* sym, const ImageLoader* requestor, 
												const LinkContext& context, bool runResolver) const
{
	uintptr_t result = exportedSymbolAddress(context, sym, requestor, runResolver);
	// check for interposing overrides
	result = interposedAddress(context, result, requestor);
	return result;
}

ImageLoader::DefinitionFlags ImageLoaderMachO::getExportedSymbolInfo(const Symbol* sym) const
{
	if ( exportedSymbolIsWeakDefintion(sym) )
		return kWeakDefinition;
	else
		return kNoDefinitionOptions;
}

const char* ImageLoaderMachO::getExportedSymbolName(const Symbol* sym) const
{
	return exportedSymbolName(sym);
}

uint32_t ImageLoaderMachO::getExportedSymbolCount() const
{
	return exportedSymbolCount();
}


const ImageLoader::Symbol* ImageLoaderMachO::getIndexedExportedSymbol(uint32_t index) const
{
	return exportedSymbolIndexed(index);
}


uint32_t ImageLoaderMachO::getImportedSymbolCount() const
{
	return importedSymbolCount();
}


const ImageLoader::Symbol* ImageLoaderMachO::getIndexedImportedSymbol(uint32_t index) const
{
	return importedSymbolIndexed(index);
}


ImageLoader::ReferenceFlags ImageLoaderMachO::getImportedSymbolInfo(const ImageLoader::Symbol* sym) const
{
	ImageLoader::ReferenceFlags flags = kNoReferenceOptions;
	return flags;
}


const char* ImageLoaderMachO::getImportedSymbolName(const ImageLoader::Symbol* sym) const
{
	return importedSymbolName(sym);
}


bool ImageLoaderMachO::getSectionContent(const char* segmentName, const char* sectionName, void** start, size_t* length)
{
	const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
	const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_SEGMENT_COMMAND:
				{
					const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
					const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
					const struct macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
					for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
						if ( (strcmp(sect->segname, segmentName) == 0) && (strcmp(sect->sectname, sectionName) == 0) ) {
							*start = (uintptr_t*)(sect->addr + fSlide);
							*length = sect->size;
							return true;
						}
					}
				}
				break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	*start = NULL;
	*length = 0;
	return false;
}

void ImageLoaderMachO::getUnwindInfo(dyld_unwind_sections* info)
{
	info->mh = this->machHeader();
	info->dwarf_section = 0;
	info->dwarf_section_length = 0;
	info->compact_unwind_section = 0;
	info->compact_unwind_section_length = 0;
	if ( fEHFrameSectionOffset != 0 ) {
		const macho_section* sect = (macho_section*)&fMachOData[fEHFrameSectionOffset];
		info->dwarf_section = (void*)(sect->addr + fSlide);
		info->dwarf_section_length = sect->size;
	}
	if ( fUnwindInfoSectionOffset != 0 ) {
		const macho_section* sect = (macho_section*)&fMachOData[fUnwindInfoSectionOffset];
		info->compact_unwind_section = (void*)(sect->addr + fSlide);
		info->compact_unwind_section_length = sect->size;
	}
}

intptr_t ImageLoaderMachO::computeSlide(const mach_header* mh)
{
	const uint32_t cmd_count = mh->ncmds;
	const load_command* const cmds = (load_command*)((char*)mh + sizeof(macho_header));
	const load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		if ( cmd->cmd == LC_SEGMENT_COMMAND ) {
			const macho_segment_command* seg = (macho_segment_command*)cmd;
			if ( (seg->fileoff == 0) && (seg->filesize != 0) )
				return (char*)mh - (char*)(seg->vmaddr);
		}
		cmd = (const load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	return 0;
}

bool ImageLoaderMachO::findSection(const mach_header* mh, const char* segmentName, const char* sectionName, void** sectAddress, size_t* sectSize)
{
	const uint32_t cmd_count = mh->ncmds;
	const load_command* const cmds = (load_command*)((char*)mh + sizeof(macho_header));
	const load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_SEGMENT_COMMAND:
				{
					const macho_segment_command* seg = (macho_segment_command*)cmd;
					const macho_section* const sectionsStart = (macho_section*)((char*)seg + sizeof(macho_segment_command));
					const macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
					for (const macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
						if ( (strcmp(sect->segname, segmentName) == 0) && (strcmp(sect->sectname, sectionName) == 0) ) {
							*sectAddress = (void*)(sect->addr + computeSlide(mh));
							*sectSize = sect->size;
							return true;
						}
					}
				}
				break;
		}
		cmd = (const load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	return false;
}


bool ImageLoaderMachO::findSection(const void* imageInterior, const char** segmentName, const char** sectionName, size_t* sectionOffset)
{
	const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
	const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
	const struct load_command* cmd = cmds;
	const uintptr_t unslidInteriorAddress = (uintptr_t)imageInterior - this->getSlide();
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_SEGMENT_COMMAND:
				{
					const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
					if ( (unslidInteriorAddress >= seg->vmaddr) && (unslidInteriorAddress < (seg->vmaddr+seg->vmsize)) ) {
						const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
						const struct macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
						for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
							if ((sect->addr <= unslidInteriorAddress) && (unslidInteriorAddress < (sect->addr+sect->size))) {
								if ( segmentName != NULL )
									*segmentName = sect->segname;
								if ( sectionName != NULL )
									*sectionName = sect->sectname;
								if ( sectionOffset != NULL )
									*sectionOffset = unslidInteriorAddress - sect->addr;
								return true;
							}
						}
					}
				}
				break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	return false;
}

const char* ImageLoaderMachO::libPath(unsigned int index) const
{
	const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
	const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
	const struct load_command* cmd = cmds;
	unsigned count = 0;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch ( cmd->cmd ) {
			case LC_LOAD_DYLIB:
			case LC_LOAD_WEAK_DYLIB:
			case LC_REEXPORT_DYLIB:
			case LC_LOAD_UPWARD_DYLIB:
				if ( index == count ) {
					const struct dylib_command*  dylibCmd = (struct dylib_command*)cmd;
					return (char*)cmd + dylibCmd->dylib.name.offset;
				}
				++count;
				break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}

	// <rdar://problem/24256354> if image linked with nothing and we implicitly added libSystem.dylib, return that
	if ( needsAddedLibSystemDepency(libraryCount(), (macho_header*)fMachOData) ) {
		return LIBSYSTEM_DYLIB_PATH;
	}

	return NULL;
}


void __attribute__((noreturn)) ImageLoaderMachO::throwSymbolNotFound(const LinkContext& context, const char* symbol, 
																	const char* referencedFrom, const char* fromVersMismatch,
																	const char* expectedIn)
{
	// record values for possible use by CrashReporter or Finder
	(*context.setErrorStrings)(DYLD_EXIT_REASON_SYMBOL_MISSING, referencedFrom, expectedIn, symbol);
	dyld::throwf("Symbol not found: %s\n  Referenced from: %s%s\n  Expected in: %s\n",
					symbol, referencedFrom, fromVersMismatch, expectedIn);
}

const mach_header* ImageLoaderMachO::machHeader() const
{
	return (mach_header*)fMachOData;
}

uintptr_t ImageLoaderMachO::getSlide() const
{
	return fSlide;
}

// hmm. maybe this should be up in ImageLoader??
const void* ImageLoaderMachO::getEnd() const
{
	uintptr_t lastAddress = 0;
	for(unsigned int i=0; i < fSegmentsCount; ++i) {
		uintptr_t segEnd = segActualEndAddress(i);
		if ( strcmp(segName(i), "__UNIXSTACK") != 0 ) {
			if ( segEnd > lastAddress )
				lastAddress = segEnd;
		}
	}
	return (const void*)lastAddress;
}


uintptr_t ImageLoaderMachO::bindLocation(const LinkContext& context, uintptr_t location, uintptr_t value, 
										uint8_t type, const char* symbolName,
										intptr_t addend, const char* inPath, const char* toPath, const char* msg)
{
	// log
	if ( context.verboseBind ) {
		if ( addend != 0 )
			dyld::log("dyld: %sbind: %s:0x%08lX = %s:%s, *0x%08lX = 0x%08lX + %ld\n",
						msg, shortName(inPath), (uintptr_t)location,
						((toPath != NULL) ? shortName(toPath) : "<missing weak_import>"),
						symbolName, (uintptr_t)location, value, addend);
		else
			dyld::log("dyld: %sbind: %s:0x%08lX = %s:%s, *0x%08lX = 0x%08lX\n",
						msg, shortName(inPath), (uintptr_t)location,
						((toPath != NULL) ? shortName(toPath) : "<missing weak_import>"),
						 symbolName, (uintptr_t)location, value);
	}
#if LOG_BINDINGS
//	dyld::logBindings("%s: %s\n", targetImage->getShortName(), symbolName);
#endif
	
	// do actual update
	uintptr_t* locationToFix = (uintptr_t*)location;
	uint32_t* loc32;
	uintptr_t newValue = value+addend;
	uint32_t value32;
	switch (type) {
		case BIND_TYPE_POINTER:
			// test first so we don't needless dirty pages
			if ( *locationToFix != newValue )
				*locationToFix = newValue;
			break;
		case BIND_TYPE_TEXT_ABSOLUTE32:
			loc32 = (uint32_t*)locationToFix;
			value32 = (uint32_t)newValue;
			if ( *loc32 != value32 )
				*loc32 = value32;
			break;
		case BIND_TYPE_TEXT_PCREL32:
			loc32 = (uint32_t*)locationToFix;
			value32 = (uint32_t)(newValue - (((uintptr_t)locationToFix) + 4));
			if ( *loc32 != value32 )
				*loc32 = value32;
			break;
		default:
			dyld::throwf("bad bind type %d", type);
	}
	
	// update statistics
	++fgTotalBindFixups;
	
	return newValue;
}





#if SUPPORT_OLD_CRT_INITIALIZATION
// first 16 bytes of "start" in crt1.o
#if __i386__
	static uint8_t sStandardEntryPointInstructions[16] = { 0x6a, 0x00, 0x89, 0xe5, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x10, 0x8b, 0x5d, 0x04, 0x89, 0x5c, 0x24 };
#endif
#endif

struct DATAdyld {
	void*			dyldLazyBinder;		// filled in at launch by dyld to point into dyld to &stub_binding_helper
	void*			dyldFuncLookup;		// filled in at launch by dyld to point into dyld to &_dyld_func_lookup
	// the following only exist in main executables built for 10.5 or later
	ProgramVars		vars;
};

// These are defined in dyldStartup.s
extern "C" void stub_binding_helper();


void ImageLoaderMachO::setupLazyPointerHandler(const LinkContext& context)
{
	const macho_header* mh = (macho_header*)fMachOData;
	const uint32_t cmd_count = mh->ncmds;
	const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
	const struct load_command* cmd;
	// There used to be some optimizations to skip this section scan, but we need to handle the 
	// __dyld section in libdyld.dylib, so everything needs to be scanned for now.
	// <rdar://problem/10910062> CrashTracer: 1,295 crashes in bash at bash: getenv
	if ( true ) {
		cmd = cmds;
		for (uint32_t i = 0; i < cmd_count; ++i) {
			if ( cmd->cmd == LC_SEGMENT_COMMAND ) {
				const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
				if ( strncmp(seg->segname, "__DATA", 6) == 0 ) {
					const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
					const struct macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
					for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
						if ( strcmp(sect->sectname, "__dyld" ) == 0 ) {
							struct DATAdyld* dd = (struct DATAdyld*)(sect->addr + fSlide);
				#if !__arm64__ && !__ARM_ARCH_7K__
							if ( sect->size > offsetof(DATAdyld, dyldLazyBinder) ) {
								if ( dd->dyldLazyBinder != (void*)&stub_binding_helper )
									dd->dyldLazyBinder = (void*)&stub_binding_helper;
							}
				#endif // !__arm64__
							if ( sect->size > offsetof(DATAdyld, dyldFuncLookup) ) {
								if ( dd->dyldFuncLookup != (void*)&_dyld_func_lookup )
									dd->dyldFuncLookup = (void*)&_dyld_func_lookup;
							}
							if ( mh->filetype == MH_EXECUTE ) {
								// there are two ways to get the program variables
								if ( (sect->size > offsetof(DATAdyld, vars)) && (dd->vars.mh == mh) ) {
									// some really old binaries have space for vars, but it is zero filled
									// main executable has 10.5 style __dyld section that has program variable pointers
									context.setNewProgramVars(dd->vars);
								}
								else {
									// main executable is pre-10.5 and requires the symbols names to be looked up
									this->lookupProgramVars(context);
				#if SUPPORT_OLD_CRT_INITIALIZATION
									// If the first 16 bytes of the entry point's instructions do not 
									// match what crt1.o supplies, then the program has a custom entry point.
									// This means it might be doing something that needs to be executed before 
									// initializers are run. 
									if ( memcmp(this->getMain(), sStandardEntryPointInstructions, 16) != 0 ) {
										if ( context.verboseInit )
											dyld::log("dyld: program uses non-standard entry point so delaying running of initializers\n");
										context.setRunInitialzersOldWay();
									}
				#endif
								}
							}
							else if ( mh->filetype == MH_DYLIB ) {
								const char* installPath = this->getInstallPath();
								if ( (installPath != NULL) && (strncmp(installPath, "/usr/lib/", 9) == 0) ) {
									if ( sect->size > offsetof(DATAdyld, vars) ) {
										// use ProgramVars from libdyld.dylib but tweak mh field to correct value
										dd->vars.mh = context.mainExecutable->machHeader();
										context.setNewProgramVars(dd->vars);
									}
								}
							}
						}
						else if ( (strcmp(sect->sectname, "__program_vars" ) == 0) && (mh->filetype == MH_EXECUTE) ) {
							// this is a Mac OS X 10.6 or later main executable 
							struct ProgramVars* pv = (struct ProgramVars*)(sect->addr + fSlide);
							context.setNewProgramVars(*pv);
						}
					}
				}
			}
			cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
		}
	}
}

									
void ImageLoaderMachO::lookupProgramVars(const LinkContext& context) const
{
	ProgramVars vars = context.programVars;
	const ImageLoader::Symbol* sym;
	
	// get mach header directly
	vars.mh = (macho_header*)fMachOData;
	
	// lookup _NXArgc
	sym = this->findShallowExportedSymbol("_NXArgc", NULL);
	if ( sym != NULL )
		vars.NXArgcPtr = (int*)this->getExportedSymbolAddress(sym, context, this, false, NULL);
		
	// lookup _NXArgv
	sym = this->findShallowExportedSymbol("_NXArgv", NULL);
	if ( sym != NULL )
		vars.NXArgvPtr = (const char***)this->getExportedSymbolAddress(sym, context, this, false, NULL);
		
	// lookup _environ
	sym = this->findShallowExportedSymbol("_environ", NULL);
	if ( sym != NULL )
		vars.environPtr = (const char***)this->getExportedSymbolAddress(sym, context, this, false, NULL);
		
	// lookup __progname
	sym = this->findShallowExportedSymbol("___progname", NULL);
	if ( sym != NULL )
		vars.__prognamePtr = (const char**)this->getExportedSymbolAddress(sym, context, this, false, NULL);
		
	context.setNewProgramVars(vars);
}


bool ImageLoaderMachO::usablePrebinding(const LinkContext& context) const
{
	// if prebound and loaded at prebound address, and all libraries are same as when this was prebound, then no need to bind
	if ( ((this->isPrebindable() && (this->getSlide() == 0)) || fInSharedCache)
		&& this->usesTwoLevelNameSpace()
		&& this->allDependentLibrariesAsWhenPreBound() ) {
		// allow environment variables to disable prebinding
		if ( context.bindFlat )
			return false;
		switch ( context.prebindUsage ) {
			case kUseAllPrebinding:
				return true;
			case kUseSplitSegPrebinding:
				return this->fIsSplitSeg;
			case kUseAllButAppPredbinding:
				return (this != context.mainExecutable);
			case kUseNoPrebinding:
				return false;
		}
	}
	return false;
}


void ImageLoaderMachO::doImageInit(const LinkContext& context)
{
	if ( fHasDashInit ) {
		const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
		const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
		const struct load_command* cmd = cmds;
		for (uint32_t i = 0; i < cmd_count; ++i) {
			switch (cmd->cmd) {
				case LC_ROUTINES_COMMAND:
					Initializer func = (Initializer)(((struct macho_routines_command*)cmd)->init_address + fSlide);
					// <rdar://problem/8543820&9228031> verify initializers are in image
					if ( ! this->containsAddress((void*)func) ) {
						dyld::throwf("initializer function %p not in mapped image for %s\n", func, this->getPath());
					}
					if ( ! dyld::gProcessInfo->libSystemInitialized ) {
						// <rdar://problem/17973316> libSystem initializer must run first
						dyld::throwf("-init function in image (%s) that does not link with libSystem.dylib\n", this->getPath());
					}
					if ( context.verboseInit )
						dyld::log("dyld: calling -init function %p in %s\n", func, this->getPath());
					func(context.argc, context.argv, context.envp, context.apple, &context.programVars);
					break;
			}
			cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
		}
	}
}

void ImageLoaderMachO::doModInitFunctions(const LinkContext& context)
{
	if ( fHasInitializers ) {
		const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
		const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
		const struct load_command* cmd = cmds;
		for (uint32_t i = 0; i < cmd_count; ++i) {
			if ( cmd->cmd == LC_SEGMENT_COMMAND ) {
				const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
				const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
				const struct macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
				for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
					const uint8_t type = sect->flags & SECTION_TYPE;
					if ( type == S_MOD_INIT_FUNC_POINTERS ) {
						Initializer* inits = (Initializer*)(sect->addr + fSlide);
						const size_t count = sect->size / sizeof(uintptr_t);
						// <rdar://problem/23929217> Ensure __mod_init_func section is within segment
						if ( (sect->addr < seg->vmaddr) || (sect->addr+sect->size > seg->vmaddr+seg->vmsize) || (sect->addr+sect->size < sect->addr) )
							dyld::throwf("__mod_init_funcs section has malformed address range for %s\n", this->getPath());
						for (size_t j=0; j < count; ++j) {
							Initializer func = inits[j];
							// <rdar://problem/8543820&9228031> verify initializers are in image
							if ( ! this->containsAddress((void*)func) ) {
								dyld::throwf("initializer function %p not in mapped image for %s\n", func, this->getPath());
							}
							if ( ! dyld::gProcessInfo->libSystemInitialized ) {
								// <rdar://problem/17973316> libSystem initializer must run first
								const char* installPath = getInstallPath();
								if ( (installPath == NULL) || (strcmp(installPath, LIBSYSTEM_DYLIB_PATH) != 0) )
									dyld::throwf("initializer in image (%s) that does not link with libSystem.dylib\n", this->getPath());
							}
							if ( context.verboseInit )
								dyld::log("dyld: calling initializer function %p in %s\n", func, this->getPath());
							bool haveLibSystemHelpersBefore = (dyld::gLibSystemHelpers != NULL);
							func(context.argc, context.argv, context.envp, context.apple, &context.programVars);
							bool haveLibSystemHelpersAfter = (dyld::gLibSystemHelpers != NULL);
							if ( !haveLibSystemHelpersBefore && haveLibSystemHelpersAfter ) {
								// now safe to use malloc() and other calls in libSystem.dylib
								dyld::gProcessInfo->libSystemInitialized = true;
							}
						}
					}
				}
			}
			cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
		}
	}
}



void ImageLoaderMachO::doGetDOFSections(const LinkContext& context, std::vector<ImageLoader::DOFInfo>& dofs)
{
	if ( fHasDOFSections ) {
		// walk load commands (mapped in at start of __TEXT segment)
		const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
		const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
		const struct load_command* cmd = cmds;
		for (uint32_t i = 0; i < cmd_count; ++i) {
			switch (cmd->cmd) {
				case LC_SEGMENT_COMMAND:
					{
						const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
						const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
						const struct macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
						for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
							if ( (sect->flags & SECTION_TYPE) == S_DTRACE_DOF ) {
								// <rdar://problem/23929217> Ensure section is within segment
								if ( (sect->addr < seg->vmaddr) || (sect->addr+sect->size > seg->vmaddr+seg->vmsize) || (sect->addr+sect->size < sect->addr) )
									dyld::throwf("DOF section has malformed address range for %s\n", this->getPath());
								ImageLoader::DOFInfo info;
								info.dof			= (void*)(sect->addr + fSlide);
								info.imageHeader	= this->machHeader();
								info.imageShortName = this->getShortName();
								dofs.push_back(info);
							}
						}
					}
					break;
			}
			cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
		}
	}
}	


bool ImageLoaderMachO::doInitialization(const LinkContext& context)
{
	CRSetCrashLogMessage2(this->getPath());

	// mach-o has -init and static initializers
	doImageInit(context);
	doModInitFunctions(context);
	
	CRSetCrashLogMessage2(NULL);
	
	return (fHasDashInit || fHasInitializers);
}

bool ImageLoaderMachO::needsInitialization()
{
	return ( fHasDashInit || fHasInitializers );
}


bool ImageLoaderMachO::needsTermination()
{
	return fHasTerminators;
}


void ImageLoaderMachO::doTermination(const LinkContext& context)
{
	if ( fHasTerminators ) {
		const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
		const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
		const struct load_command* cmd = cmds;
		for (uint32_t i = 0; i < cmd_count; ++i) {
			if ( cmd->cmd == LC_SEGMENT_COMMAND ) {
				const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
				const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
				const struct macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
				for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
					const uint8_t type = sect->flags & SECTION_TYPE;
					if ( type == S_MOD_TERM_FUNC_POINTERS ) {
						// <rdar://problem/23929217> Ensure section is within segment
						if ( (sect->addr < seg->vmaddr) || (sect->addr+sect->size > seg->vmaddr+seg->vmsize) || (sect->addr+sect->size < sect->addr) )
							dyld::throwf("DOF section has malformed address range for %s\n", this->getPath());
						Terminator* terms = (Terminator*)(sect->addr + fSlide);
						const size_t count = sect->size / sizeof(uintptr_t);
						for (size_t j=count; j > 0; --j) {
							Terminator func = terms[j-1];
							// <rdar://problem/8543820&9228031> verify terminators are in image
							if ( ! this->containsAddress((void*)func) ) {
								dyld::throwf("termination function %p not in mapped image for %s\n", func, this->getPath());
							}
							if ( context.verboseInit )
								dyld::log("dyld: calling termination function %p in %s\n", func, this->getPath());
							func();
						}
					}
				}
			}
			cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
		}
	}
}


void ImageLoaderMachO::printStatisticsDetails(unsigned int imageCount, const InitializerTimingList& timingInfo)
{
	ImageLoader::printStatisticsDetails(imageCount, timingInfo);
	dyld::log("total symbol trie searches:    %d\n", fgSymbolTrieSearchs);
	dyld::log("total symbol table binary searches:    %d\n", fgSymbolTableBinarySearchs);
	dyld::log("total images defining weak symbols:  %u\n", fgImagesHasWeakDefinitions);
	dyld::log("total images using weak symbols:  %u\n", fgImagesRequiringCoalescing);
}


intptr_t ImageLoaderMachO::assignSegmentAddresses(const LinkContext& context)
{
	// preflight and calculate slide if needed
	const bool inPIE = (fgNextPIEDylibAddress != 0);
	intptr_t slide = 0;
	if ( this->segmentsCanSlide() && this->segmentsMustSlideTogether() ) {
		bool needsToSlide = false;
		bool imageHasPreferredLoadAddress = segHasPreferredLoadAddress(0);
		uintptr_t lowAddr = (unsigned long)(-1);
		uintptr_t highAddr = 0;
		for(unsigned int i=0, e=segmentCount(); i < e; ++i) {
			const uintptr_t segLow = segPreferredLoadAddress(i);
			const uintptr_t segHigh = dyld_page_round(segLow + segSize(i));
			if ( segLow < highAddr ) {
				if ( dyld_page_size > 4096 )
					dyld::throwf("can't map segments into 16KB pages");
				else
					dyld::throwf("overlapping segments");
			}
			if ( segLow < lowAddr )
				lowAddr = segLow;
			if ( segHigh > highAddr )
				highAddr = segHigh;
				
			if ( needsToSlide || !imageHasPreferredLoadAddress || inPIE || !reserveAddressRange(segPreferredLoadAddress(i), segSize(i)) )
				needsToSlide = true;
		}
		if ( needsToSlide ) {
			// find a chunk of address space to hold all segments
			uintptr_t addr = reserveAnAddressRange(highAddr-lowAddr, context);
			slide = addr - lowAddr;
		}
	} 
	else if ( ! this->segmentsCanSlide() ) {
		for(unsigned int i=0, e=segmentCount(); i < e; ++i) {
			if ( (strcmp(segName(i), "__PAGEZERO") == 0) && (segFileSize(i) == 0) && (segPreferredLoadAddress(i) == 0) )
				continue;
			if ( !reserveAddressRange(segPreferredLoadAddress(i), segSize(i)) )
				dyld::throwf("can't map unslidable segment %s to 0x%lX with size 0x%lX", segName(i), segPreferredLoadAddress(i), segSize(i));
		}
	}
	else {
		throw "mach-o does not support independently sliding segments";
	}
	return slide;
}


uintptr_t ImageLoaderMachO::reserveAnAddressRange(size_t length, const ImageLoader::LinkContext& context)
{
	vm_address_t addr = 0;
	vm_size_t size = length;
	// in PIE programs, load initial dylibs after main executable so they don't have fixed addresses either
	if ( fgNextPIEDylibAddress != 0 ) {
		 // add small (0-3 pages) random padding between dylibs
		addr = fgNextPIEDylibAddress + (__stack_chk_guard/fgNextPIEDylibAddress & (sizeof(long)-1))*dyld_page_size;
		//dyld::log("padding 0x%08llX, guard=0x%08llX\n", (long long)(addr - fgNextPIEDylibAddress), (long long)(__stack_chk_guard));
		kern_return_t r = vm_alloc(&addr, size, VM_FLAGS_FIXED | VM_MAKE_TAG(VM_MEMORY_DYLIB));
		if ( r == KERN_SUCCESS ) {
			fgNextPIEDylibAddress = addr + size;
			return addr;
		}
		fgNextPIEDylibAddress = 0;
	}
	kern_return_t r = vm_alloc(&addr, size, VM_FLAGS_ANYWHERE | VM_MAKE_TAG(VM_MEMORY_DYLIB));
	if ( r != KERN_SUCCESS ) 
		throw "out of address space";
	
	return addr;
}

bool ImageLoaderMachO::reserveAddressRange(uintptr_t start, size_t length)
{
	vm_address_t addr = start;
	vm_size_t size = length;
	kern_return_t r = vm_alloc(&addr, size, VM_FLAGS_FIXED | VM_MAKE_TAG(VM_MEMORY_DYLIB));
	if ( r != KERN_SUCCESS ) 
		return false;
	return true;
}



void ImageLoaderMachO::mapSegments(int fd, uint64_t offsetInFat, uint64_t lenInFat, uint64_t fileLen, const LinkContext& context)
{
	// find address range for image
	intptr_t slide = this->assignSegmentAddresses(context);
	if ( context.verboseMapping ) {
		if ( offsetInFat != 0 )
			dyld::log("dyld: Mapping %s (slice offset=%llu)\n", this->getPath(), (unsigned long long)offsetInFat);
		else
			dyld::log("dyld: Mapping %s\n", this->getPath());
	}
	// map in all segments
	for(unsigned int i=0, e=segmentCount(); i < e; ++i) {
		vm_offset_t fileOffset = segFileOffset(i) + offsetInFat;
		vm_size_t size = segFileSize(i);
		uintptr_t requestedLoadAddress = segPreferredLoadAddress(i) + slide;
		int protection = 0;
		if ( !segUnaccessible(i) ) {
			// If has text-relocs, don't set x-bit initially.
			// Instead set it later after text-relocs have been done.
			if ( segExecutable(i) && !(segHasRebaseFixUps(i) && (slide != 0)) )
				protection   |= PROT_EXEC;
			if ( segReadable(i) )
				protection   |= PROT_READ;
			if ( segWriteable(i) ) {
				protection   |= PROT_WRITE;
				// rdar://problem/22525618 force __LINKEDIT to always be mapped read-only
				if ( strcmp(segName(i), "__LINKEDIT") == 0 )
					protection = PROT_READ;
			}
		}
	#if __i386__
		// initially map __IMPORT segments R/W so dyld can update them
		if ( segIsReadOnlyImport(i) )
			protection |= PROT_WRITE;
	#endif
		// wholly zero-fill segments have nothing to mmap() in
		if ( size > 0 ) {
			if ( (fileOffset+size) > fileLen ) {
				dyld::throwf("truncated mach-o error: segment %s extends to %llu which is past end of file %llu", 
								segName(i), (uint64_t)(fileOffset+size), fileLen);
			}
			void* loadAddress = xmmap((void*)requestedLoadAddress, size, protection, MAP_FIXED | MAP_PRIVATE, fd, fileOffset);
			if ( loadAddress == ((void*)(-1)) ) {
				int mmapErr = errno;
				if ( mmapErr == EPERM ) {
					if ( dyld::sandboxBlockedMmap(getPath()) )
						dyld::throwf("file system sandbox blocked mmap() of '%s'", this->getPath());
					else
						dyld::throwf("code signing blocked mmap() of '%s'", this->getPath());
				}
				else
					dyld::throwf("mmap() errno=%d at address=0x%08lX, size=0x%08lX segment=%s in Segment::map() mapping %s",
						mmapErr, requestedLoadAddress, (uintptr_t)size, segName(i), getPath());
			}
		}
		// update stats
		++ImageLoader::fgTotalSegmentsMapped;
		ImageLoader::fgTotalBytesMapped += size;
		if ( context.verboseMapping )
			dyld::log("%18s at 0x%08lX->0x%08lX with permissions %c%c%c\n", segName(i), requestedLoadAddress, requestedLoadAddress+size-1,
				(protection & PROT_READ) ? 'r' : '.',  (protection & PROT_WRITE) ? 'w' : '.',  (protection & PROT_EXEC) ? 'x' : '.' );
	}

	// update slide to reflect load location			
	this->setSlide(slide);
}

void ImageLoaderMachO::mapSegments(const void* memoryImage, uint64_t imageLen, const LinkContext& context)
{
	// find address range for image
	intptr_t slide = this->assignSegmentAddresses(context);
	if ( context.verboseMapping )
		dyld::log("dyld: Mapping memory %p\n", memoryImage);
	// map in all segments
	for(unsigned int i=0, e=segmentCount(); i < e; ++i) {
		vm_address_t loadAddress = segPreferredLoadAddress(i) + slide;
		vm_address_t srcAddr = (uintptr_t)memoryImage + segFileOffset(i);
		vm_size_t size = segFileSize(i);
		kern_return_t r = vm_copy(mach_task_self(), srcAddr, size, loadAddress);
		if ( r != KERN_SUCCESS ) 
			throw "can't map segment";
		if ( context.verboseMapping )
			dyld::log("%18s at 0x%08lX->0x%08lX\n", segName(i), (uintptr_t)loadAddress, (uintptr_t)loadAddress+size-1);
	}
	// update slide to reflect load location			
	this->setSlide(slide);
	// set R/W permissions on all segments at slide location
	for(unsigned int i=0, e=segmentCount(); i < e; ++i) {
		segProtect(i, context);		
	}
}


void ImageLoaderMachO::segProtect(unsigned int segIndex, const ImageLoader::LinkContext& context)
{
	vm_prot_t protection = 0;
	if ( !segUnaccessible(segIndex) ) {
		if ( segExecutable(segIndex) )
			protection   |= PROT_EXEC;
		if ( segReadable(segIndex) )
			protection   |= PROT_READ;
		if ( segWriteable(segIndex) )
			protection   |= PROT_WRITE;
	}
	vm_address_t addr = segActualLoadAddress(segIndex);
	vm_size_t size = segSize(segIndex);
	const bool setCurrentPermissions = false;
	kern_return_t r = vm_protect(mach_task_self(), addr, size, setCurrentPermissions, protection);
	if ( r != KERN_SUCCESS ) {
        dyld::throwf("vm_protect(0x%08llX, 0x%08llX, false, 0x%02X) failed, result=%d for segment %s in %s",
            (long long)addr, (long long)size, protection, r, segName(segIndex), this->getPath());
    }
	if ( context.verboseMapping ) {
		dyld::log("%18s at 0x%08lX->0x%08lX altered permissions to %c%c%c\n", segName(segIndex), (uintptr_t)addr, (uintptr_t)addr+size-1,
			(protection & PROT_READ) ? 'r' : '.',  (protection & PROT_WRITE) ? 'w' : '.',  (protection & PROT_EXEC) ? 'x' : '.' );
	}
}	

void ImageLoaderMachO::segMakeWritable(unsigned int segIndex, const ImageLoader::LinkContext& context)
{
	vm_address_t addr = segActualLoadAddress(segIndex);
	vm_size_t size = segSize(segIndex);
	const bool setCurrentPermissions = false;
	vm_prot_t protection = VM_PROT_WRITE | VM_PROT_READ;
	if ( segExecutable(segIndex) && !segHasRebaseFixUps(segIndex) )
		protection |= VM_PROT_EXECUTE;
	kern_return_t r = vm_protect(mach_task_self(), addr, size, setCurrentPermissions, protection);
	if ( r != KERN_SUCCESS ) {
        dyld::throwf("vm_protect(0x%08llX, 0x%08llX, false, 0x%02X) failed, result=%d for segment %s in %s",
            (long long)addr, (long long)size, protection, r, segName(segIndex), this->getPath());
    }
	if ( context.verboseMapping ) {
		dyld::log("%18s at 0x%08lX->0x%08lX altered permissions to %c%c%c\n", segName(segIndex), (uintptr_t)addr, (uintptr_t)addr+size-1,
			(protection & PROT_READ) ? 'r' : '.',  (protection & PROT_WRITE) ? 'w' : '.',  (protection & PROT_EXEC) ? 'x' : '.' );
	}
}


const char* ImageLoaderMachO::findClosestSymbol(const mach_header* mh, const void* addr, const void** closestAddr)
{
	// called by dladdr()
	// only works with compressed LINKEDIT if classic symbol table is also present
	const dysymtab_command* dynSymbolTable = NULL;
	const symtab_command* symtab = NULL;
	const macho_segment_command* seg;
	const uint8_t* unslidLinkEditBase = NULL;
	bool linkEditBaseFound = false;
	intptr_t slide = 0;
	const uint32_t cmd_count = mh->ncmds;
	const load_command* const cmds = (load_command*)((char*)mh + sizeof(macho_header));
	const load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_SEGMENT_COMMAND:
				seg = (macho_segment_command*)cmd;
				if ( strcmp(seg->segname, "__LINKEDIT") == 0 ) {
					unslidLinkEditBase = (uint8_t*)(seg->vmaddr - seg->fileoff);
					linkEditBaseFound = true;
				}
				else if ( (seg->fileoff == 0) && (seg->filesize != 0) )
					slide = (uintptr_t)mh - seg->vmaddr;
				break;
			case LC_SYMTAB:
				symtab = (symtab_command*)cmd;
				break;
			case LC_DYSYMTAB:
				dynSymbolTable = (dysymtab_command*)cmd;
				break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	// no symbol table => no lookup by address
	if ( (symtab == NULL) || (dynSymbolTable == NULL) || !linkEditBaseFound )
		return NULL;

	const uint8_t* linkEditBase = unslidLinkEditBase + slide;
	const char* symbolTableStrings = (const char*)&linkEditBase[symtab->stroff];
	const macho_nlist* symbolTable = (macho_nlist*)(&linkEditBase[symtab->symoff]);

	uintptr_t targetAddress = (uintptr_t)addr - slide;
	const struct macho_nlist* bestSymbol = NULL;
	// first walk all global symbols
	const struct macho_nlist* const globalsStart = &symbolTable[dynSymbolTable->iextdefsym];
	const struct macho_nlist* const globalsEnd= &globalsStart[dynSymbolTable->nextdefsym];
	for (const struct macho_nlist* s = globalsStart; s < globalsEnd; ++s) {
 		if ( (s->n_type & N_TYPE) == N_SECT ) {
			if ( bestSymbol == NULL ) {
				if ( s->n_value <= targetAddress )
					bestSymbol = s;
			}
			else if ( (s->n_value <= targetAddress) && (bestSymbol->n_value < s->n_value) ) {
				bestSymbol = s;
			}
		}
	}
	// next walk all local symbols
	const struct macho_nlist* const localsStart = &symbolTable[dynSymbolTable->ilocalsym];
	const struct macho_nlist* const localsEnd= &localsStart[dynSymbolTable->nlocalsym];
	for (const struct macho_nlist* s = localsStart; s < localsEnd; ++s) {
 		if ( ((s->n_type & N_TYPE) == N_SECT) && ((s->n_type & N_STAB) == 0) ) {
			if ( bestSymbol == NULL ) {
				if ( s->n_value <= targetAddress )
					bestSymbol = s;
			}
			else if ( (s->n_value <= targetAddress) && (bestSymbol->n_value < s->n_value) ) {
				bestSymbol = s;
			}
		}
	}
	if ( bestSymbol != NULL ) {
#if __arm__
		if (bestSymbol->n_desc & N_ARM_THUMB_DEF)
			*closestAddr = (void*)((bestSymbol->n_value | 1) + slide);
		else
			*closestAddr = (void*)(bestSymbol->n_value + slide);
#else
		*closestAddr = (void*)(bestSymbol->n_value + slide);
#endif
		return &symbolTableStrings[bestSymbol->n_un.n_strx];
	}
	return NULL;
}

bool ImageLoaderMachO::getLazyBindingInfo(uint32_t& lazyBindingInfoOffset, const uint8_t* lazyInfoStart, const uint8_t* lazyInfoEnd,
													uint8_t* segIndex, uintptr_t* segOffset, int* ordinal, const char** symbolName, bool* doneAfterBind)
{
	if ( lazyBindingInfoOffset > (lazyInfoEnd-lazyInfoStart) )
		return false;
	uint8_t type = BIND_TYPE_POINTER;
	uint8_t symboFlags = 0;
	bool done = false;
	const uint8_t* p = &lazyInfoStart[lazyBindingInfoOffset];
	while ( !done && (p < lazyInfoEnd) ) {
		uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
		uint8_t opcode = *p & BIND_OPCODE_MASK;
		++p;
		switch (opcode) {
			case BIND_OPCODE_DONE:
				*doneAfterBind = false;
				return true;
				break;
			case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
				*ordinal = immediate;
				break;
			case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
				*ordinal = (int)read_uleb128(p, lazyInfoEnd);
				break;
			case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
				// the special ordinals are negative numbers
				if ( immediate == 0 )
					*ordinal = 0;
				else {
					int8_t signExtended = BIND_OPCODE_MASK | immediate;
					*ordinal = signExtended;
				}
				break;
			case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
				*symbolName = (char*)p;
				symboFlags = immediate;
				while (*p != '\0')
					++p;
				++p;
				break;
			case BIND_OPCODE_SET_TYPE_IMM:
				type = immediate;
				break;
			case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
				*segIndex  = immediate;
				*segOffset = read_uleb128(p, lazyInfoEnd);
				break;
			case BIND_OPCODE_DO_BIND:
				*doneAfterBind = ((*p & BIND_OPCODE_MASK) == BIND_OPCODE_DONE);
				lazyBindingInfoOffset += p - &lazyInfoStart[lazyBindingInfoOffset];
				return true;
				break;
			case BIND_OPCODE_SET_ADDEND_SLEB:
			case BIND_OPCODE_ADD_ADDR_ULEB:
			case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
			case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
			case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
			default:
				return false;
		}
	}	
	return false;
}

const dyld_info_command* ImageLoaderMachO::findDyldInfoLoadCommand(const mach_header* mh)
{
	const uint32_t cmd_count = mh->ncmds;
	const load_command* const cmds = (load_command*)((char*)mh + sizeof(macho_header));
	const load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_DYLD_INFO:
			case LC_DYLD_INFO_ONLY:
				return (dyld_info_command*)cmd;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	return NULL;
}


uintptr_t ImageLoaderMachO::segPreferredAddress(const mach_header* mh, unsigned segIndex)
{
	const uint32_t cmd_count = mh->ncmds;
	const load_command* const cmds = (load_command*)((char*)mh + sizeof(macho_header));
	const load_command* cmd = cmds;
	unsigned curSegIndex = 0;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		if ( cmd->cmd == LC_SEGMENT_COMMAND ) {
			if ( segIndex == curSegIndex ) {
				const macho_segment_command* segCmd = (macho_segment_command*)cmd;
				return segCmd->vmaddr;
			}
			++curSegIndex;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	return 0;
}



