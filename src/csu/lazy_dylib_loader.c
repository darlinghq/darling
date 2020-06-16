/*
 * Copyright (c) 2008-2012 Apple Inc. All rights reserved.
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
 
 
#include <stddef.h>
#include <string.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>


#ifndef LC_LAZY_LOAD_DYLIB
	#define LC_LAZY_LOAD_DYLIB  0x20
#endif
#ifndef S_LAZY_DYLIB_SYMBOL_POINTERS
	#define S_LAZY_DYLIB_SYMBOL_POINTERS  0x10
#endif
#ifndef LC_LOAD_UPWARD_DYLIB
    #define	LC_LOAD_UPWARD_DYLIB (0x23 | LC_REQ_DYLD) /* load upward dylib */
#endif

#if __LP64__
	#define LC_SEGMENT_COMMAND			LC_SEGMENT_64
	#define LC_ROUTINES_COMMAND			LC_ROUTINES_64
	typedef struct mach_header_64		macho_header;
	typedef struct section_64			macho_section;
	typedef struct nlist_64				macho_nlist;
	typedef struct segment_command_64	macho_segment_command;
#else
	#define LC_SEGMENT_COMMAND		LC_SEGMENT
	#define LC_ROUTINES_COMMAND		LC_ROUTINES
	typedef struct mach_header		macho_header;
	typedef struct section			macho_section;
	typedef struct nlist			macho_nlist;
	typedef struct segment_command	macho_segment_command;
#endif

extern const macho_header __dso_handle;


// This function may be overriden by application code
// to do custom error handling when a lazy symbol cannot be  
// resolved.
int dyld_lazy_dylib_proxy() __attribute__((weak,visibility("hidden")));
int dyld_lazy_dylib_proxy()
{
	return 0;
}


// This function may be overriden by application code
// to dynamically change the path to a loaded lazy dylib. 
const char* dyld_lazy_dylib_path_fix(const char*) __attribute__((weak,visibility("hidden")));
const char* dyld_lazy_dylib_path_fix(const char* path)
{
	return path;
}


static void* getHandleForLazyOrdinal(const macho_header* mh, void* handles[], uint8_t ordinal)
{
	const uint32_t cmd_count = mh->ncmds;
	const struct load_command* const cmds = (struct load_command*)((char*)mh + sizeof(macho_header));
	const struct load_command* cmd = cmds;
	uint8_t loadDylibCount = 0;
	uint8_t loadLazyDylibCount = 0;
	uint32_t i;
	// walk load commands to find LC_LAZY_LOAD_DYLIB that matches ordinal
	for (i = 0; i < cmd_count; ++i) {
		switch ( cmd->cmd ) {
			case LC_LOAD_DYLIB:
			case LC_LOAD_WEAK_DYLIB:
			case LC_LOAD_UPWARD_DYLIB:
				++loadDylibCount;
				break;
			case LC_LAZY_LOAD_DYLIB:
				++loadDylibCount;
				if ( loadDylibCount == ordinal ) {
					if ( handles[loadLazyDylibCount] == NULL ) {
						const struct dylib_command* dylib = (struct dylib_command*)cmd;
						const char* path = (char*)cmd + dylib->dylib.name.offset;
						const char* fixedPath = dyld_lazy_dylib_path_fix(path);
						handles[loadLazyDylibCount] = dlopen(fixedPath, RTLD_LAZY);
					}
					return handles[loadLazyDylibCount];
				}
				++loadLazyDylibCount;
				break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	return NULL;
}



// called by dyld_lazy_dylib_stub_binding_helper
// this function must figure out which function
// lazyPointer is supposed to point to 
// and update it it.
void* lazy_load_dylib(uintptr_t* lazyPointer) __attribute__((visibility("hidden")));
void* lazy_load_dylib(uintptr_t* lazyPointer)
{
	static const macho_header*	mh = NULL;
	static const macho_nlist*	symbolTable = NULL;
	static const char*			stringTable = NULL;
	static const uint8_t*		linkEditBase = NULL;
	static const uint32_t*		indirectSymbolTable = NULL;
	static intptr_t				slide = 0;
	static void*				minHandles[8];
	static void**				handles;
	
	// do this work only on first call
	uint32_t i;
	if ( mh == NULL ) {
		const macho_header* tmh = &__dso_handle;
		// symbol table, indirect symbol table
		const uint32_t cmd_count = tmh->ncmds;
		const struct load_command* const cmds = (struct load_command*)((char*)tmh + sizeof(macho_header));
		const struct load_command* cmd = cmds;
		// first pass at load commands gets linkEditBase
		for (i = 0; i < cmd_count; ++i) {
			if ( cmd->cmd == LC_SEGMENT_COMMAND ) {
				const macho_segment_command* seg = (macho_segment_command*)cmd;
				if ( strcmp(seg->segname,"__TEXT") == 0 ) 
					slide = (uintptr_t)tmh - seg->vmaddr;
				else if ( strcmp(seg->segname,"__LINKEDIT") == 0 ) 
					linkEditBase = (uint8_t*)(seg->vmaddr + slide - seg->fileoff);
			}
			cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
		}
		// next pass at load commands gets symbolTable, stringTable
		uint32_t lazyDylibCount = 0;
		cmd = cmds;
		for (i = 0; i < cmd_count; ++i) {
			switch ( cmd->cmd ) {
				case LC_SYMTAB:
					{
						const struct symtab_command* symtab = (struct symtab_command*)cmd;
						stringTable = (const char*)&linkEditBase[symtab->stroff];
						symbolTable = (macho_nlist*)(&linkEditBase[symtab->symoff]);
					}
					break;
				case LC_DYSYMTAB:
					{
						const struct dysymtab_command* dsymtab = (struct dysymtab_command*)cmd;
						indirectSymbolTable = (uint32_t*)(&linkEditBase[dsymtab->indirectsymoff]);
					}
					break;
				case LC_LAZY_LOAD_DYLIB:
					++lazyDylibCount;
					break;
			}
			cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
		}
		// use static buffer when possible
		if ( lazyDylibCount < 8 )
			handles = minHandles;
		else
			handles = calloc(lazyDylibCount, sizeof(void*));
		
		// save to static global to make this thread safe
		mh = tmh;
	}
	
	// find lazy dylib pointer section
	void* result = &dyld_lazy_dylib_proxy;
	const uint32_t cmd_count = mh->ncmds;
	const struct load_command* const cmds = (struct load_command*)((char*)mh + sizeof(macho_header));
	const struct load_command* cmd = cmds;
	// walk sections to find one with this lazy pointer
	for (i = 0; i < cmd_count; ++i) {
		if ( cmd->cmd == LC_SEGMENT_COMMAND ) {
			const macho_segment_command* seg = (macho_segment_command*)cmd;
			const macho_section* const sectionsStart = (macho_section*)((char*)seg + sizeof(macho_segment_command));
			const macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
			const macho_section* sect;
			for (sect=sectionsStart; sect < sectionsEnd; ++sect) {
				const uint8_t type = sect->flags & SECTION_TYPE;
				if ( type == S_LAZY_DYLIB_SYMBOL_POINTERS ) { // S_LAZY_DYLIB_SYMBOL_POINTERS
					const uint32_t pointerCount = sect->size / sizeof(uintptr_t);
					uintptr_t* const symbolPointers = (uintptr_t*)(sect->addr + slide);
					if ( (lazyPointer >= symbolPointers) && (lazyPointer < &symbolPointers[pointerCount]) ) {
						const uint32_t indirectTableOffset = sect->reserved1;
						const uint32_t lazyIndex = lazyPointer - symbolPointers;
						uint32_t symbolIndex = indirectSymbolTable[indirectTableOffset + lazyIndex];
						if ( symbolIndex != INDIRECT_SYMBOL_ABS && symbolIndex != INDIRECT_SYMBOL_LOCAL ) {
							// found symbol for this lazy pointer, now lookup address
							const char* symbolName = &stringTable[symbolTable[symbolIndex].n_un.n_strx];
							uint8_t ordinal = GET_LIBRARY_ORDINAL(symbolTable[symbolIndex].n_desc);
							void* handle = getHandleForLazyOrdinal(mh, handles, ordinal);
							if ( handle != NULL ) {
								void* addr = dlsym(handle, &symbolName[1]);
								if ( addr != NULL )
									result = addr;
								*lazyPointer = (uintptr_t)result;
								return result;
							}
						}
					}
				}
			}
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}	
	*lazyPointer = (uintptr_t)result;
	return result;
}

