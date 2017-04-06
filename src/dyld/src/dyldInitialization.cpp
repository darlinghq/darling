/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2004-2008 Apple Inc. All rights reserved.
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

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <Availability.h>
#include <mach/mach.h>
#include <mach-o/loader.h>
#include <mach-o/ldsyms.h>
#include <mach-o/reloc.h>
#if __x86_64__
	#include <mach-o/x86_64/reloc.h>
#endif
#include "dyld.h"
#include "dyldSyscallInterface.h"

// from dyld_gdb.cpp 
extern void addImagesToAllImages(uint32_t infoCount, const dyld_image_info info[]);
extern void syncProcessInfo();

#ifndef MH_PIE
	#define MH_PIE 0x200000 
#endif

// currently dyld has no initializers, but if some come back, set this to non-zero
#define DYLD_INITIALIZER_SUPPORT  0

#if __LP64__
	#define LC_SEGMENT_COMMAND		LC_SEGMENT_64
	#define macho_segment_command	segment_command_64
	#define macho_section			section_64
	#define RELOC_SIZE				3
#else
	#define LC_SEGMENT_COMMAND		LC_SEGMENT
	#define macho_segment_command	segment_command
	#define macho_section			section
	#define RELOC_SIZE				2
#endif

#if __x86_64__
	#define POINTER_RELOC X86_64_RELOC_UNSIGNED
#else
	#define POINTER_RELOC GENERIC_RELOC_VANILLA
#endif


#if TARGET_IPHONE_SIMULATOR
const dyld::SyscallHelpers* gSyscallHelpers = NULL;
#endif


//
//  Code to bootstrap dyld into a runnable state
//
//

namespace dyldbootstrap {



#if DYLD_INITIALIZER_SUPPORT

typedef void (*Initializer)(int argc, const char* argv[], const char* envp[], const char* apple[]);

extern const Initializer  inits_start  __asm("section$start$__DATA$__mod_init_func");
extern const Initializer  inits_end    __asm("section$end$__DATA$__mod_init_func");

//
// For a regular executable, the crt code calls dyld to run the executables initializers.
// For a static executable, crt directly runs the initializers.
// dyld (should be static) but is a dynamic executable and needs this hack to run its own initializers.
// We pass argc, argv, etc in case libc.a uses those arguments
//
static void runDyldInitializers(const struct macho_header* mh, intptr_t slide, int argc, const char* argv[], const char* envp[], const char* apple[])
{
	for (const Initializer* p = &inits_start; p < &inits_end; ++p) {
		(*p)(argc, argv, envp, apple);
	}
}
#endif // DYLD_INITIALIZER_SUPPORT


//
//  The kernel may have slid a Position Independent Executable
//
static uintptr_t slideOfMainExecutable(const struct macho_header* mh)
{
	const uint32_t cmd_count = mh->ncmds;
	const struct load_command* const cmds = (struct load_command*)(((char*)mh)+sizeof(macho_header));
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		if ( cmd->cmd == LC_SEGMENT_COMMAND ) {
			const struct macho_segment_command* segCmd = (struct macho_segment_command*)cmd;
			if ( (segCmd->fileoff == 0) && (segCmd->filesize != 0)) {
				return (uintptr_t)mh - segCmd->vmaddr;
			}
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	return 0;
}


//
// If the kernel does not load dyld at its preferred address, we need to apply 
// fixups to various initialized parts of the __DATA segment
//
static void rebaseDyld(const struct macho_header* mh, intptr_t slide)
{
	// rebase non-lazy pointers (which all point internal to dyld, since dyld uses no shared libraries)
	// and get interesting pointers into dyld
	const uint32_t cmd_count = mh->ncmds;
	const struct load_command* const cmds = (struct load_command*)(((char*)mh)+sizeof(macho_header));
	const struct load_command* cmd = cmds;
	const struct macho_segment_command* linkEditSeg = NULL;
#if __x86_64__
	const struct macho_segment_command* firstWritableSeg = NULL;
#endif
	const struct dysymtab_command* dynamicSymbolTable = NULL;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_SEGMENT_COMMAND:
				{
					const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
					if ( strcmp(seg->segname, "__LINKEDIT") == 0 )
						linkEditSeg = seg;
					const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
					const struct macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
					for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
						const uint8_t type = sect->flags & SECTION_TYPE;
						if ( type == S_NON_LAZY_SYMBOL_POINTERS ) {
							// rebase non-lazy pointers (which all point internal to dyld, since dyld uses no shared libraries)
							const uint32_t pointerCount = (uint32_t)(sect->size / sizeof(uintptr_t));
							uintptr_t* const symbolPointers = (uintptr_t*)(sect->addr + slide);
							for (uint32_t j=0; j < pointerCount; ++j) {
								symbolPointers[j] += slide;
							}
						}
					}
#if __x86_64__
					if ( (firstWritableSeg == NULL) && (seg->initprot & VM_PROT_WRITE) )
						firstWritableSeg = seg;
#endif
				}
				break;
			case LC_DYSYMTAB:
				dynamicSymbolTable = (struct dysymtab_command *)cmd;
				break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	
	// use reloc's to rebase all random data pointers
#if __x86_64__
	const uintptr_t relocBase = firstWritableSeg->vmaddr + slide;
#else
	const uintptr_t relocBase = (uintptr_t)mh;
#endif
	const relocation_info* const relocsStart = (struct relocation_info*)(linkEditSeg->vmaddr + slide + dynamicSymbolTable->locreloff - linkEditSeg->fileoff);
	const relocation_info* const relocsEnd = &relocsStart[dynamicSymbolTable->nlocrel];
	for (const relocation_info* reloc=relocsStart; reloc < relocsEnd; ++reloc) {
		if ( reloc->r_length != RELOC_SIZE ) 
			throw "relocation in dyld has wrong size";

		if ( reloc->r_type != POINTER_RELOC ) 
			throw "relocation in dyld has wrong type";
		
		// update pointer by amount dyld slid
		*((uintptr_t*)(reloc->r_address + relocBase)) += slide;
	}
}


extern "C" void mach_init();
extern "C" void __guard_setup(const char* apple[]);


//
//  This is code to bootstrap dyld.  This work in normally done for a program by dyld and crt.
//  In dyld we have to do this manually.
//
uintptr_t start(const struct macho_header* appsMachHeader, int argc, const char* argv[], 
				intptr_t slide, const struct macho_header* dyldsMachHeader,
				uintptr_t* startGlue)
{
	// if kernel had to slide dyld, we need to fix up load sensitive locations
	// we have to do this before using any global variables
	if ( slide != 0 ) {
		rebaseDyld(dyldsMachHeader, slide);
	}

	// allow dyld to use mach messaging
	mach_init();

	// kernel sets up env pointer to be just past end of agv array
	const char** envp = &argv[argc+1];
	
	// kernel sets up apple pointer to be just past end of envp array
	const char** apple = envp;
	while(*apple != NULL) { ++apple; }
	++apple;

	// set up random value for stack canary
	__guard_setup(apple);

#if DYLD_INITIALIZER_SUPPORT
	// run all C++ initializers inside dyld
	runDyldInitializers(dyldsMachHeader, slide, argc, argv, envp, apple);
#endif

	// now that we are done bootstrapping dyld, call dyld's main
	uintptr_t appsSlide = slideOfMainExecutable(appsMachHeader);
	return dyld::_main(appsMachHeader, appsSlide, argc, argv, envp, apple, startGlue);
}


#if TARGET_IPHONE_SIMULATOR

extern "C" uintptr_t start_sim(int argc, const char* argv[], const char* envp[], const char* apple[],
							const macho_header* mainExecutableMH, const macho_header* dyldMH, uintptr_t dyldSlide,
							const dyld::SyscallHelpers*, uintptr_t* startGlue);
					
					
uintptr_t start_sim(int argc, const char* argv[], const char* envp[], const char* apple[],
					const macho_header* mainExecutableMH, const macho_header* dyldMH, uintptr_t dyldSlide,
					const dyld::SyscallHelpers* sc, uintptr_t* startGlue)
{
	// if simulator dyld loaded slid, it needs to rebase itself
	// we have to do this before using any global variables
	if ( dyldSlide != 0 ) {
		rebaseDyld(dyldMH, dyldSlide);
	}

	// save table of syscall pointers
	gSyscallHelpers = sc;
	
	// allow dyld to use mach messaging
	mach_init();

	// set up random value for stack canary
	__guard_setup(apple);

	// setup gProcessInfo to point to host dyld's struct
	dyld::gProcessInfo = (struct dyld_all_image_infos*)(sc->getProcessInfo());
	syncProcessInfo();

	// now that we are done bootstrapping dyld, call dyld's main
	uintptr_t appsSlide = slideOfMainExecutable(mainExecutableMH);
	return dyld::_main(mainExecutableMH, appsSlide, argc, argv, envp, apple, startGlue);
}
#endif


} // end of namespace




