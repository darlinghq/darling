/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2004-2011 Apple Inc. All rights reserved.
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

#include <stdlib.h>
#include <string.h>
#include <mach-o/loader.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <pthread.h>
#include <Availability.h>

#include "mach-o/dyld_priv.h"
#include "dyldLibSystemInterface.h"


extern void _ZN4dyld3logEPKcz(const char*, ...);
extern struct LibSystemHelpers* _ZN4dyld17gLibSystemHelpersE;


#if __LP64__
	#define LC_SEGMENT_COMMAND		LC_SEGMENT_64
	#define macho_header			mach_header_64
	#define macho_segment_command	segment_command_64
	#define macho_section			section_64
	#define getsectdatafromheader	getsectdatafromheader_64
#else
	#define LC_SEGMENT_COMMAND		LC_SEGMENT
	#define macho_header			mach_header
	#define macho_segment_command	segment_command
	#define macho_section			section
#endif


//
//  The standard versions of __cxa_get_globals*() from libstdc++-static.a cannot be used.
//  On Mac OS X, they use keymgr which dyld does not implement.
//  On iPhoneOS, they use pthread_key_create which dyld cannot use.
//
//   Implement these ourselves to make upcalls into libSystem to malloc and create a pthread key
//
static pthread_key_t				sCxaKey = 0;
static char							sPreMainCxaGlobals[2*sizeof(long)];

// called by libstdc++.a 
char* __cxa_get_globals() 
{	
	// if libSystem.dylib not yet initialized, or is old libSystem, use shared global
	if ( (_ZN4dyld17gLibSystemHelpersE == NULL) || (_ZN4dyld17gLibSystemHelpersE->version < 7) )
		return sPreMainCxaGlobals;

	if ( sCxaKey == 0 ) {
		// create key
		// we don't need a lock because only one thread can be in dyld at a time
		_ZN4dyld17gLibSystemHelpersE->pthread_key_create(&sCxaKey, &free);
	}
	char* data = (char*)_ZN4dyld17gLibSystemHelpersE->pthread_getspecific(sCxaKey);
	if ( data == NULL ) {
		data = calloc(2,sizeof(void*));
		_ZN4dyld17gLibSystemHelpersE->pthread_setspecific(sCxaKey, data);
	}
	return data; 
}

// called by libstdc++.a 
char* __cxa_get_globals_fast() 
{ 
	// if libSystem.dylib not yet initialized, or is old libSystem, use shared global
	if ( (_ZN4dyld17gLibSystemHelpersE == NULL) || (_ZN4dyld17gLibSystemHelpersE->version < 7) )
		return sPreMainCxaGlobals;

	return _ZN4dyld17gLibSystemHelpersE->pthread_getspecific(sCxaKey); 
}



#if !__USING_SJLJ_EXCEPTIONS__
//
//  When dyld uses zero-cost exceptions it just needs to implement 
//  _dyld_find_unwind_sections to return sections inside dyld proper.
//

extern void*  ehStart  __asm("section$start$__TEXT$__eh_frame");
extern void*  ehEnd    __asm("section$end$__TEXT$__eh_frame");
extern void*  uwStart  __asm("section$start$__TEXT$__unwind_info");
extern void*  uwEnd    __asm("section$end$__TEXT$__unwind_info");

extern void*  textStart __asm("section$start$__TEXT$__text");
extern void*  textEnd   __asm("section$end$__TEXT$__text");

extern void* __dso_handle;

// called by libuwind code to find unwind information sections in dyld
bool _dyld_find_unwind_sections(void* addr, struct dyld_unwind_sections* info)
{
 	if ( ((void*)&textStart < addr) && (addr < (void*)&textEnd) ) { 
        info->mh = (struct mach_header*)&__dso_handle;
        info->dwarf_section = &ehStart;
        info->dwarf_section_length = ((char*)&ehEnd - (char*)&ehStart);
        info->compact_unwind_section = &uwStart;
        info->compact_unwind_section_length = ((char*)&uwEnd - (char*)&uwStart);
		return true;
	}
	else {
		return false;
	}
}

#else
//
//  When dyld uses setjump-longjump exceptions it needs to implement 
//  routines to push and pop a stack of _Unwind_FunctionContext.
//

struct _Unwind_FunctionContext
{
	// next function in stack of handlers
	struct _Unwind_FunctionContext*		prev;

};


static pthread_key_t						sThreadChainKey = 0; 
static struct _Unwind_FunctionContext*		sStaticThreadChain = NULL; 

//
// When libSystem's initializers are run, they call back into dyld's 
// registerThreadHelpers which creates a pthread key and calls 
//  __Unwind_SjLj_SetThreadKey().
//
void __Unwind_SjLj_SetThreadKey(pthread_key_t key)
{
	sThreadChainKey = key;
	//_ZN4dyld3logEPKcz("__Unwind_SjLj_SetThreadKey(key=%d), sStaticThreadChain=%p\n", key, sStaticThreadChain);
	// switch static chain to be per thread
	_ZN4dyld17gLibSystemHelpersE->pthread_setspecific(sThreadChainKey, sStaticThreadChain);
	sStaticThreadChain = NULL;
	//_ZN4dyld3logEPKcz("__Unwind_SjLj_SetThreadKey(key=%d), result=%d, new top=%p\n", key, result, pthread_getspecific(sThreadChainKey));
}


//static void printChain()
//{
//	_ZN4dyld3logEPKcz("chain: ");
//	struct _Unwind_FunctionContext* start = sStaticThreadChain;
//	if ( sThreadChainKey != 0 ) {
//		start = (struct _Unwind_FunctionContext*)pthread_getspecific(sThreadChainKey);
//	}
//	for (struct _Unwind_FunctionContext* p = start; p != NULL; p = p->prev) {
//		_ZN4dyld3logEPKcz("%p -> ", p);
//	}
//	_ZN4dyld3logEPKcz("\n");
//}


struct _Unwind_FunctionContext* __Unwind_SjLj_GetTopOfFunctionStack()
{
	//_ZN4dyld3logEPKcz("__Unwind_SjLj_GetTopOfFunctionStack(),           key=%d, ", sThreadChainKey);
	//printChain();
	if ( sThreadChainKey != 0 ) {
		return (struct _Unwind_FunctionContext*)pthread_getspecific(sThreadChainKey);
	}
	else {
		return sStaticThreadChain;
	}
}

void __Unwind_SjLj_SetTopOfFunctionStack(struct _Unwind_FunctionContext* fc)
{
	//_ZN4dyld3logEPKcz("__Unwind_SjLj_SetTopOfFunctionStack(%p), key=%d, prev=%p\n", 
	//	fc, sThreadChainKey, (fc != NULL) ? fc->prev : NULL);
	if ( sThreadChainKey != 0 )
		_ZN4dyld17gLibSystemHelpersE->pthread_setspecific(sThreadChainKey, fc);
	else
		sStaticThreadChain = fc;
	//_ZN4dyld3logEPKcz("__Unwind_SjLj_SetTopOfFunctionStack(%p), key=%d, ", fc, sThreadChainKey);
	//printChain();
}

#endif



