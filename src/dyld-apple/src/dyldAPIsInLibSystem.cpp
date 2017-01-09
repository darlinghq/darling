/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2004-2012 Apple Inc. All rights reserved.
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
#include <malloc/malloc.h>
#include <sys/mman.h>

#include <crt_externs.h>
#include <Availability.h>
#include <vproc_priv.h>

#include <dirent.h>
#include <sys/stat.h>

#include "mach-o/dyld_images.h"
#include "mach-o/dyld.h"
#include "mach-o/dyld_priv.h"
#include "dyld_cache_format.h"

#include "ImageLoader.h"
#include "dyldLock.h"
#include "start_glue.h"

extern "C" int  __cxa_atexit(void (*func)(void *), void *arg, void *dso);
extern "C" void __cxa_finalize(const void *dso);
extern "C" void __cxa_finalize_ranges(const struct __cxa_range_t ranges[], int count);


#ifndef LC_VERSION_MIN_MACOSX
	#define LC_VERSION_MIN_MACOSX 0x24
	struct version_min_command {
		uint32_t	cmd;		/* LC_VERSION_MIN_MACOSX or
					   LC_VERSION_MIN_IPHONEOS  */
		uint32_t	cmdsize;	/* sizeof(struct min_version_command) */
		uint32_t	version;	/* X.Y.Z is encoded in nibbles xxxx.yy.zz */
		uint32_t	sdk;		/* X.Y.Z is encoded in nibbles xxxx.yy.zz */
	};
#endif

#ifndef LC_VERSION_MIN_IPHONEOS
	#define LC_VERSION_MIN_IPHONEOS 0x25
#endif

#ifndef LC_VERSION_MIN_TVOS
	#define LC_VERSION_MIN_TVOS 0x2F
#endif

#ifndef LC_VERSION_MIN_WATCHOS
	#define LC_VERSION_MIN_WATCHOS 0x30
#endif


#ifndef LC_LOAD_UPWARD_DYLIB
	#define	LC_LOAD_UPWARD_DYLIB (0x23|LC_REQ_DYLD)	/* load of dylib whose initializers run later */
#endif


// deprecated APIs are still availble on Mac OS X, but not on iPhone OS
#if __IPHONE_OS_VERSION_MIN_REQUIRED	
	#define DEPRECATED_APIS_SUPPORTED 0
#else
	#define DEPRECATED_APIS_SUPPORTED 1
#endif

/*
 * names_match() takes an install_name from an LC_LOAD_DYLIB command and a
 * libraryName (which is -lx or -framework Foo argument passed to the static
 * link editor for the same library) and determines if they match.  This depends
 * on conventional use of names including major versioning.
 */
static
bool
names_match(
const char *install_name,
const char* libraryName)
{
    const char *basename;
    unsigned long n;

	/*
	 * Conventional install names have these forms:
	 *	/System/Library/Frameworks/AppKit.framework/Versions/A/Appkit
	 *	/Local/Library/Frameworks/AppKit.framework/Appkit
	 *	/lib/libsys_s.A.dylib
	 *	/usr/lib/libsys_s.dylib
	 */
	basename = strrchr(install_name, '/');
	if(basename == NULL)
	    basename = install_name;
	else
	    basename++;

	/*
	 * By checking the base name matching the library name we take care
	 * of the -framework cases.
	 */
	if(strcmp(basename, libraryName) == 0)
	    return true;

	/*
	 * Now check the base name for "lib" if so proceed to check for the
	 * -lx case dealing with a possible .X.dylib and a .dylib extension.
	 */
	if(strncmp(basename, "lib", 3) ==0){
	    n = strlen(libraryName);
	    if(strncmp(basename+3, libraryName, n) == 0){
		if(strncmp(basename+3+n, ".dylib", 6) == 0)
		    return true;
		if(basename[3+n] == '.' &&
		   basename[3+n+1] != '\0' &&
		   strncmp(basename+3+n+2, ".dylib", 6) == 0)
		    return true;
	    }
	}
	return false;
}

#if DEPRECATED_APIS_SUPPORTED

void NSInstallLinkEditErrorHandlers(
const NSLinkEditErrorHandlers* handlers)
{
	DYLD_LOCK_THIS_BLOCK;
	typedef void (*ucallback_t)(const char* symbol_name);
 	typedef NSModule (*mcallback_t)(NSSymbol s, NSModule old, NSModule newhandler);
	typedef void (*lcallback_t)(NSLinkEditErrors c, int errorNumber,
								const char* fileName, const char* errorString);
	static void (*p)(ucallback_t undefined, mcallback_t multiple, lcallback_t linkEdit) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_install_handlers", (void**)&p);
	mcallback_t m = handlers->multiple;
	p(handlers->undefined, m, handlers->linkEdit);
}

const char* 
NSNameOfModule(
NSModule module)
{
	DYLD_LOCK_THIS_BLOCK;
    static const char*  (*p)(NSModule module) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSNameOfModule", (void**)&p);
	return(p(module));
} 

const char* 
NSLibraryNameForModule(
NSModule module)
{
	DYLD_LOCK_THIS_BLOCK;
    static const char*  (*p)(NSModule module) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSLibraryNameForModule", (void**)&p);
	return(p(module));
}

bool
NSIsSymbolNameDefined(
const char* symbolName)
{
	DYLD_LOCK_THIS_BLOCK;
    static bool (*p)(const char* symbolName) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSIsSymbolNameDefined", (void**)&p);
	return(p(symbolName));
}

bool
NSIsSymbolNameDefinedWithHint(
const char* symbolName,
const char* libraryNameHint)
{
	DYLD_LOCK_THIS_BLOCK;
    static bool (*p)(const char* symbolName,
			  const char* libraryNameHint) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSIsSymbolNameDefinedWithHint", (void**)&p);
	return(p(symbolName, libraryNameHint));
}

bool
NSIsSymbolNameDefinedInImage(
const struct mach_header *image,
const char* symbolName)
{
	DYLD_LOCK_THIS_BLOCK;
    static bool (*p)(const struct mach_header *image,
			  const char* symbolName) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSIsSymbolNameDefinedInImage", (void**)&p);
	return(p(image, symbolName));
}

NSSymbol
NSLookupAndBindSymbol(
const char* symbolName)
{
	DYLD_LOCK_THIS_BLOCK;
    static NSSymbol (*p)(const char* symbolName) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSLookupAndBindSymbol", (void**)&p);
	return(p(symbolName));
}

NSSymbol
NSLookupAndBindSymbolWithHint(
const char* symbolName,
const char* libraryNameHint)
{
	DYLD_LOCK_THIS_BLOCK;
    static NSSymbol (*p)(const char* symbolName,
			 const char* libraryNameHint) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSLookupAndBindSymbolWithHint", (void**)&p);
	return(p(symbolName, libraryNameHint));
}

NSSymbol
NSLookupSymbolInModule(
NSModule module,
const char* symbolName)
{
	DYLD_LOCK_THIS_BLOCK;
    static NSSymbol (*p)(NSModule module, const char* symbolName) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSLookupSymbolInModule", (void**)&p);
	return(p(module, symbolName));
}

NSSymbol
NSLookupSymbolInImage(
const struct mach_header *image,
const char* symbolName,
uint32_t options)
{
 	DYLD_LOCK_THIS_BLOCK;
   static NSSymbol (*p)(const struct mach_header *image,
			 const char* symbolName,
			 uint32_t options) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSLookupSymbolInImage", (void**)&p);
	return(p(image, symbolName, options));
}

const char* 
NSNameOfSymbol(
NSSymbol symbol)
{
	DYLD_LOCK_THIS_BLOCK;
    static char * (*p)(NSSymbol symbol) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSNameOfSymbol",(void**)&p);
	return(p(symbol));
}

void *
NSAddressOfSymbol(
NSSymbol symbol)
{
	DYLD_LOCK_THIS_BLOCK;
    static void * (*p)(NSSymbol symbol) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSAddressOfSymbol", (void**)&p);
	return(p(symbol));
}

NSModule
NSModuleForSymbol(
NSSymbol symbol)
{
	DYLD_LOCK_THIS_BLOCK;
    static NSModule (*p)(NSSymbol symbol) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSModuleForSymbol", (void**)&p);
	return(p(symbol));
}

bool
NSAddLibrary(
const char* pathName)
{
	DYLD_LOCK_THIS_BLOCK;
    static bool (*p)(const char* pathName) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSAddLibrary", (void**)&p);
	return(p(pathName));
}

bool
NSAddLibraryWithSearching(
const char* pathName)
{
	DYLD_LOCK_THIS_BLOCK;
    static bool (*p)(const char* pathName) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSAddLibraryWithSearching", (void**)&p);
	return(p(pathName));
}

const struct mach_header *
NSAddImage(
const char* image_name,
uint32_t options)
{
	DYLD_LOCK_THIS_BLOCK;
    static const struct mach_header * (*p)(const char* image_name,
					   uint32_t options) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSAddImage", (void**)&p);
	return(p(image_name, options));
}
#endif // DEPRECATED_APIS_SUPPORTED

/*
 * This routine returns the current version of the named shared library the
 * executable it was built with.  The libraryName parameter is the same as the
 * -lx or -framework Foo argument passed to the static link editor when building
 * the executable (with -lx it would be "x" and with -framework Foo it would be
 * "Foo").  If this the executable was not built against the specified library
 * it returns -1.  It should be noted that if this only returns the value the
 * current version of the named shared library the executable was built with
 * and not a list of current versions that dependent libraries and bundles the
 * program is using were built with.
 */
int32_t NSVersionOfLinkTimeLibrary(const char* libraryName)
{
	// Lazily call _NSGetMachExecuteHeader() and cache result
#if __LP64__
    static mach_header_64* mh = NULL;
#else
    static mach_header* mh = NULL;
#endif
	if ( mh == NULL )
	    mh = _NSGetMachExecuteHeader();
#if __LP64__
	const load_command* lc = (load_command*)((char*)mh + sizeof(mach_header_64));
#else
	const load_command* lc = (load_command*)((char*)mh + sizeof(mach_header));
#endif
	for(uint32_t i = 0; i < mh->ncmds; i++){
		switch ( lc->cmd ) { 
			case LC_LOAD_DYLIB:
			case LC_LOAD_WEAK_DYLIB:
			case LC_LOAD_UPWARD_DYLIB:
				const dylib_command* dl = (dylib_command *)lc;
				const char* install_name = (char*)dl + dl->dylib.name.offset;
				if ( names_match(install_name, libraryName) )
					return dl->dylib.current_version;
				break;
		}
	    lc = (load_command*)((char*)lc + lc->cmdsize);
	}
	return (-1);
}

/*
 * This routine returns the current version of the named shared library the
 * program it is running against.  The libraryName parameter is the same as
 * would be static link editor using the -lx or -framework Foo flags (with -lx
 * it would be "x" and with -framework Foo it would be "Foo").  If the program
 * is not using the specified library it returns -1.
 */
int32_t NSVersionOfRunTimeLibrary(const char* libraryName)
{
	uint32_t n = _dyld_image_count();
	for(uint32_t i = 0; i < n; i++){
	    const mach_header* mh = _dyld_get_image_header(i);
		if ( mh == NULL )
			continue;
	    if ( mh->filetype != MH_DYLIB )
			continue;
#if __LP64__
	    const load_command* lc = (load_command*)((char*)mh + sizeof(mach_header_64));
#else
	    const load_command* lc = (load_command*)((char*)mh + sizeof(mach_header));
#endif
	    for(uint32_t j = 0; j < mh->ncmds; j++){
			if ( lc->cmd == LC_ID_DYLIB ) {
				const dylib_command* dl = (dylib_command*)lc;
				const char* install_name = (char *)dl + dl->dylib.name.offset;
				if ( names_match(install_name, libraryName) )
					return dl->dylib.current_version;
			}
			lc = (load_command*)((char*)lc + lc->cmdsize);
	    }
	}
	return (-1);
}


#define PACKED_VERSION(major, minor, tiny) ((((major) & 0xffff) << 16) | (((minor) & 0xff) << 8) | ((tiny) & 0xff))


static bool getVersionLoadCommandInfo(const mach_header* mh, uint32_t* loadCommand, uint32_t* minOS, uint32_t* sdk)
{
	const load_command* startCmds = NULL;
	if ( mh->magic == MH_MAGIC_64 )
		startCmds = (load_command*)((char *)mh + sizeof(mach_header_64));
	else if ( mh->magic == MH_MAGIC )
		startCmds = (load_command*)((char *)mh + sizeof(mach_header));
	else
		return false;  // not a mach-o file, or wrong endianness
		
	const load_command* const cmdsEnd = (load_command*)((char*)startCmds + mh->sizeofcmds);
	const load_command* cmd = startCmds;
	for(uint32_t i = 0; i < mh->ncmds; ++i) {
	    const load_command* nextCmd = (load_command*)((char *)cmd + cmd->cmdsize);
		if ( (cmd->cmdsize < 8) || (nextCmd > cmdsEnd) || (nextCmd < startCmds)) {
			return 0;
		}
		const version_min_command* versCmd;
		switch ( cmd->cmd ) {
			case LC_VERSION_MIN_IPHONEOS:
			case LC_VERSION_MIN_MACOSX:
			case LC_VERSION_MIN_TVOS:
			case LC_VERSION_MIN_WATCHOS:
				versCmd = (version_min_command*)cmd;
				*loadCommand = versCmd->cmd;
				*minOS = versCmd->version;
				*sdk = versCmd->sdk;
				return true;
		}
		cmd = nextCmd;
	}
	return false;
}

#if !__WATCH_OS_VERSION_MIN_REQUIRED && !__TV_OS_VERSION_MIN_REQUIRED
static uint32_t deriveSDKVersFromDylibs(const mach_header* mh)
{
	const load_command* startCmds = NULL;
	if ( mh->magic == MH_MAGIC_64 )
		startCmds = (load_command*)((char *)mh + sizeof(mach_header_64));
	else if ( mh->magic == MH_MAGIC )
		startCmds = (load_command*)((char *)mh + sizeof(mach_header));
	else
		return 0;  // not a mach-o file, or wrong endianness
		
	const load_command* const cmdsEnd = (load_command*)((char*)startCmds + mh->sizeofcmds);
	const dylib_command* dylibCmd;
	const load_command* cmd = startCmds;
	const char* dylibName;
  #if __IPHONE_OS_VERSION_MIN_REQUIRED
	uint32_t foundationVers = 0;
  #else
	uint32_t libSystemVers = 0;
  #endif
	for(uint32_t i = 0; i < mh->ncmds; ++i) {
	    const load_command* nextCmd = (load_command*)((char *)cmd + cmd->cmdsize);
		// <rdar://problem/14381579&16050962> sanity check size of command
		if ( (cmd->cmdsize < 8) || (nextCmd > cmdsEnd) || (nextCmd < startCmds)) {
			return 0;
		}
		switch ( cmd->cmd ) {
			case LC_LOAD_DYLIB:
			case LC_LOAD_WEAK_DYLIB:
			case LC_LOAD_UPWARD_DYLIB:
				dylibCmd = (dylib_command*)cmd;
				// sanity check dylib command layout
				if ( dylibCmd->dylib.name.offset > cmd->cmdsize )
					return 0;
				dylibName = (char*)dylibCmd + dylibCmd->dylib.name.offset;
  #if __IPHONE_OS_VERSION_MIN_REQUIRED
				if ( strcmp(dylibName, "/System/Library/Frameworks/Foundation.framework/Foundation") == 0 )
					foundationVers = dylibCmd->dylib.current_version;
  #else
				if ( strcmp(dylibName, "/usr/lib/libSystem.B.dylib") == 0 )
					libSystemVers = dylibCmd->dylib.current_version;
  #endif
				break;
		}
		cmd = nextCmd;
	}

	struct DylibToOSMapping {
		uint32_t dylibVersion;
		uint32_t osVersion;
	};
	
  #if __IPHONE_OS_VERSION_MIN_REQUIRED
	static const DylibToOSMapping foundationMapping[] = {
		{ PACKED_VERSION(678,24,0), DYLD_IOS_VERSION_2_0 },
		{ PACKED_VERSION(678,26,0), DYLD_IOS_VERSION_2_1 },
		{ PACKED_VERSION(678,29,0), DYLD_IOS_VERSION_2_2 },
		{ PACKED_VERSION(678,47,0), DYLD_IOS_VERSION_3_0 },
		{ PACKED_VERSION(678,51,0), DYLD_IOS_VERSION_3_1 },
		{ PACKED_VERSION(678,60,0), DYLD_IOS_VERSION_3_2 },
		{ PACKED_VERSION(751,32,0), DYLD_IOS_VERSION_4_0 },
		{ PACKED_VERSION(751,37,0), DYLD_IOS_VERSION_4_1 },
		{ PACKED_VERSION(751,49,0), DYLD_IOS_VERSION_4_2 },
		{ PACKED_VERSION(751,58,0), DYLD_IOS_VERSION_4_3 },
		{ PACKED_VERSION(881,0,0),  DYLD_IOS_VERSION_5_0 },
		{ PACKED_VERSION(890,1,0),  DYLD_IOS_VERSION_5_1 },
		{ PACKED_VERSION(992,0,0),  DYLD_IOS_VERSION_6_0 },
		{ PACKED_VERSION(993,0,0),  DYLD_IOS_VERSION_6_1 },  
		{ PACKED_VERSION(1038,14,0),DYLD_IOS_VERSION_7_0 },
		{ PACKED_VERSION(0,0,0),    DYLD_IOS_VERSION_7_0 }
		// We don't need to expand this table because all recent
		// binaries have LC_VERSION_MIN_ load command.
	};

	if ( foundationVers != 0 ) {
		uint32_t lastOsVersion = 0;
		for (const DylibToOSMapping* p=foundationMapping; ; ++p) {
			if ( p->dylibVersion == 0 )
				return p->osVersion;
			if ( foundationVers < p->dylibVersion )
				return lastOsVersion;
			lastOsVersion = p->osVersion;
		}
	}

  #else
	// Note: versions are for the GM release.  The last entry should
	// always be zero.  At the start of the next major version,
	// a new last entry needs to be added and the previous zero
	// updated to the GM dylib version.
	static const DylibToOSMapping libSystemMapping[] = {
		{ PACKED_VERSION(88,1,3),   DYLD_MACOSX_VERSION_10_4 },
		{ PACKED_VERSION(111,0,0),  DYLD_MACOSX_VERSION_10_5 },
		{ PACKED_VERSION(123,0,0),  DYLD_MACOSX_VERSION_10_6 },
		{ PACKED_VERSION(159,0,0),  DYLD_MACOSX_VERSION_10_7 },
		{ PACKED_VERSION(169,3,0),  DYLD_MACOSX_VERSION_10_8 },
		{ PACKED_VERSION(1197,0,0), DYLD_MACOSX_VERSION_10_9 },
		{ PACKED_VERSION(0,0,0),    DYLD_MACOSX_VERSION_10_9 }
		// We don't need to expand this table because all recent
		// binaries have LC_VERSION_MIN_ load command.
	};

	if ( libSystemVers != 0 ) {
		uint32_t lastOsVersion = 0;
		for (const DylibToOSMapping* p=libSystemMapping; ; ++p) {
			if ( p->dylibVersion == 0 )
				return p->osVersion;
			if ( libSystemVers < p->dylibVersion )
				return lastOsVersion;
			lastOsVersion = p->osVersion;
		}
	}
  #endif
  return 0;
}
#endif


#if __WATCH_OS_VERSION_MIN_REQUIRED
static uint32_t watchVersToIOSVers(uint32_t vers)
{
	return vers + 0x00070000;
}

uint32_t dyld_get_program_sdk_watch_os_version()
{
	const mach_header* mh = (mach_header*)_NSGetMachExecuteHeader();
	uint32_t loadCommand;
	uint32_t minOS;
	uint32_t sdk;

	if ( getVersionLoadCommandInfo(mh, &loadCommand, &minOS, &sdk) ) {
		if ( loadCommand == LC_VERSION_MIN_WATCHOS )
				return sdk;
	}
	return 0;
}

uint32_t dyld_get_program_min_watch_os_version()
{
	const mach_header* mh = (mach_header*)_NSGetMachExecuteHeader();
	uint32_t loadCommand;
	uint32_t minOS;
	uint32_t sdk;

	if ( getVersionLoadCommandInfo(mh, &loadCommand, &minOS, &sdk) ) {
		if ( loadCommand == LC_VERSION_MIN_WATCHOS )
				return minOS;  // return raw minOS (not mapped to iOS version)
	}
	return 0;
}

#endif

/*
 * Returns the sdk version (encode as nibble XXXX.YY.ZZ) the
 * specified binary was built against.
 *
 * First looks for LC_VERSION_MIN_* in binary and if sdk field is 
 * not zero, return that value.
 * Otherwise, looks for the libSystem.B.dylib the binary linked
 * against and uses a table to convert that to an sdk version.
 */
uint32_t dyld_get_sdk_version(const mach_header* mh)
{
	uint32_t loadCommand;
	uint32_t minOS;
	uint32_t sdk;

	if ( getVersionLoadCommandInfo(mh, &loadCommand, &minOS, &sdk) ) {
		switch (loadCommand) {
#if __WATCH_OS_VERSION_MIN_REQUIRED
			case LC_VERSION_MIN_WATCHOS:
				// new binary. sdk version looks like "2.0" but API wants "9.0"
				return watchVersToIOSVers(sdk);
			case LC_VERSION_MIN_IPHONEOS:
				// old binary. sdk matches API semantics so can return directly.
				return sdk;
#elif __TV_OS_VERSION_MIN_REQUIRED
			case LC_VERSION_MIN_TVOS:
			case LC_VERSION_MIN_IPHONEOS:
				return sdk;
#elif __IPHONE_OS_VERSION_MIN_REQUIRED
			case LC_VERSION_MIN_IPHONEOS:
				if ( sdk != 0 )	// old binaries might not have SDK set
					return sdk;
				break;
#else
			case LC_VERSION_MIN_MACOSX:
				if ( sdk != 0 )	// old binaries might not have SDK set
					return sdk;
				break;
#endif
		}
	}

#if __WATCH_OS_VERSION_MIN_REQUIRED ||__TV_OS_VERSION_MIN_REQUIRED
	// All WatchOS and tv OS binaries should have version load command.
	return 0;
#else
	// MacOSX and iOS have old binaries without version load commmand.
	return deriveSDKVersFromDylibs(mh);
#endif
}

uint32_t dyld_get_program_sdk_version()
{
	return dyld_get_sdk_version((mach_header*)_NSGetMachExecuteHeader());
}

uint32_t dyld_get_min_os_version(const struct mach_header* mh)
{
	uint32_t loadCommand;
	uint32_t minOS;
	uint32_t sdk;

	if ( getVersionLoadCommandInfo(mh, &loadCommand, &minOS, &sdk) ) {
		switch (loadCommand) {
#if __WATCH_OS_VERSION_MIN_REQUIRED
			case LC_VERSION_MIN_WATCHOS:
				// new binary. OS version looks like "2.0" but API wants "9.0"
				return watchVersToIOSVers(minOS);
			case LC_VERSION_MIN_IPHONEOS:
				// old binary. OS matches API semantics so can return directly.
				return minOS;
#elif __TV_OS_VERSION_MIN_REQUIRED
			case LC_VERSION_MIN_TVOS:
			case LC_VERSION_MIN_IPHONEOS:
				return minOS;
#elif __IPHONE_OS_VERSION_MIN_REQUIRED
			case LC_VERSION_MIN_IPHONEOS:
				return minOS;
#else
			case LC_VERSION_MIN_MACOSX:
				return minOS;
#endif
		}
	}
	return 0;
}


uint32_t dyld_get_program_min_os_version()
{
	return dyld_get_min_os_version((mach_header*)_NSGetMachExecuteHeader());
}


bool _dyld_get_image_uuid(const struct mach_header* mh, uuid_t uuid)
{
	const load_command* startCmds = NULL;
	if ( mh->magic == MH_MAGIC_64 )
		startCmds = (load_command*)((char *)mh + sizeof(mach_header_64));
	else if ( mh->magic == MH_MAGIC )
		startCmds = (load_command*)((char *)mh + sizeof(mach_header));
	else
		return false;  // not a mach-o file, or wrong endianness

	const load_command* const cmdsEnd = (load_command*)((char*)startCmds + mh->sizeofcmds);
	const load_command* cmd = startCmds;
	for(uint32_t i = 0; i < mh->ncmds; ++i) {
	    const load_command* nextCmd = (load_command*)((char *)cmd + cmd->cmdsize);
		if ( (cmd->cmdsize < 8) || (nextCmd > cmdsEnd) || (nextCmd < startCmds)) {
			return false;
		}
		if ( cmd->cmd == LC_UUID ) {
			const uuid_command* uuidCmd = (uuid_command*)cmd;
			memcpy(uuid, uuidCmd->uuid, 16);
			return true;
		}
		cmd = nextCmd;
	}
	bzero(uuid, 16);
	return false;
}



#if DEPRECATED_APIS_SUPPORTED
/*
 * NSCreateObjectFileImageFromFile() creates an NSObjectFileImage for the
 * specified file name if the file is a correct Mach-O file that can be loaded
 * with NSloadModule().  For return codes of NSObjectFileImageFailure and
 * NSObjectFileImageFormat an error message is printed to stderr.  All
 * other codes cause no printing. 
 */
NSObjectFileImageReturnCode
NSCreateObjectFileImageFromFile(
const char* pathName,
NSObjectFileImage *objectFileImage)
{
	DYLD_LOCK_THIS_BLOCK;
    static NSObjectFileImageReturnCode (*p)(const char*, NSObjectFileImage*) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSCreateObjectFileImageFromFile", (void**)&p);
	return p(pathName, objectFileImage);
}


/*
 * NSCreateObjectFileImageFromMemory() creates an NSObjectFileImage for the
 * object file mapped into memory at address of size length if the object file
 * is a correct Mach-O file that can be loaded with NSloadModule().  For return
 * codes of NSObjectFileImageFailure and NSObjectFileImageFormat an error
 * message is printed to stderr.  All other codes cause no printing. 
 */
NSObjectFileImageReturnCode
NSCreateObjectFileImageFromMemory(
const void* address,
size_t size, 
NSObjectFileImage *objectFileImage)
{
	DYLD_LOCK_THIS_BLOCK;
    static NSObjectFileImageReturnCode (*p)(const void*, size_t, NSObjectFileImage*) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSCreateObjectFileImageFromMemory", (void**)&p);
	return p(address, size, objectFileImage);
}

#if OBSOLETE_DYLD_API
/*
 * NSCreateCoreFileImageFromFile() creates an NSObjectFileImage for the 
 * specified core file name if the file is a correct Mach-O core file.
 * For return codes of NSObjectFileImageFailure and NSObjectFileImageFormat
 * an error message is printed to stderr.  All other codes cause no printing. 
 */
NSObjectFileImageReturnCode
NSCreateCoreFileImageFromFile(
const char* pathName,
NSObjectFileImage *objectFileImage)
{
	DYLD_LOCK_THIS_BLOCK;
    static NSObjectFileImageReturnCode (*p)(const char*, NSObjectFileImage*) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSCreateCoreFileImageFromFile", (void**)&p);
	return p(pathName, objectFileImage);
}
#endif

bool
NSDestroyObjectFileImage(
NSObjectFileImage objectFileImage)
{
	DYLD_LOCK_THIS_BLOCK;
    static bool (*p)(NSObjectFileImage) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSDestroyObjectFileImage", (void**)&p);
	return p(objectFileImage);
}


NSModule
NSLinkModule(
NSObjectFileImage objectFileImage, 
const char* moduleName,
uint32_t options)
{
	DYLD_LOCK_THIS_BLOCK;
    static NSModule (*p)(NSObjectFileImage, const char*, unsigned long) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSLinkModule", (void**)&p);
		
	return p(objectFileImage, moduleName, options);
}




/*
 * NSSymbolDefinitionCountInObjectFileImage() returns the number of symbol
 * definitions in the NSObjectFileImage.
 */
uint32_t
NSSymbolDefinitionCountInObjectFileImage(
NSObjectFileImage objectFileImage)
{
	DYLD_LOCK_THIS_BLOCK;
    static uint32_t (*p)(NSObjectFileImage) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSSymbolDefinitionCountInObjectFileImage", (void**)&p);
		
	return p(objectFileImage);
}

/*
 * NSSymbolDefinitionNameInObjectFileImage() returns the name of the i'th
 * symbol definitions in the NSObjectFileImage.  If the ordinal specified is
 * outside the range [0..NSSymbolDefinitionCountInObjectFileImage], NULL will
 * be returned.
 */
const char* 
NSSymbolDefinitionNameInObjectFileImage(
NSObjectFileImage objectFileImage,
uint32_t ordinal)
{
	DYLD_LOCK_THIS_BLOCK;
    static const char*  (*p)(NSObjectFileImage, uint32_t) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSSymbolDefinitionNameInObjectFileImage", (void**)&p);
		
	return p(objectFileImage, ordinal);
}

/*
 * NSSymbolReferenceCountInObjectFileImage() returns the number of references
 * to undefined symbols the NSObjectFileImage.
 */
uint32_t
NSSymbolReferenceCountInObjectFileImage(
NSObjectFileImage objectFileImage)
{
	DYLD_LOCK_THIS_BLOCK;
    static uint32_t (*p)(NSObjectFileImage) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSSymbolReferenceCountInObjectFileImage", (void**)&p);
		
	return p(objectFileImage);
}

/*
 * NSSymbolReferenceNameInObjectFileImage() returns the name of the i'th
 * undefined symbol in the NSObjectFileImage. If the ordinal specified is
 * outside the range [0..NSSymbolReferenceCountInObjectFileImage], NULL will be
 * returned.
 */
const char* 
NSSymbolReferenceNameInObjectFileImage(
NSObjectFileImage objectFileImage,
uint32_t ordinal,
bool *tentative_definition) /* can be NULL */
{
	DYLD_LOCK_THIS_BLOCK;
    static const char*  (*p)(NSObjectFileImage, uint32_t, bool*) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSSymbolReferenceNameInObjectFileImage", (void**)&p);
		
	return p(objectFileImage, ordinal, tentative_definition);
}

/*
 * NSIsSymbolDefinedInObjectFileImage() returns TRUE if the specified symbol
 * name has a definition in the NSObjectFileImage and FALSE otherwise.
 */
bool
NSIsSymbolDefinedInObjectFileImage(
NSObjectFileImage objectFileImage,
const char* symbolName)
{
	DYLD_LOCK_THIS_BLOCK;
    static bool (*p)(NSObjectFileImage, const char*) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSIsSymbolDefinedInObjectFileImage", (void**)&p);
		
	return p(objectFileImage, symbolName);
}

/*
 * NSGetSectionDataInObjectFileImage() returns a pointer to the section contents
 * in the NSObjectFileImage for the specified segmentName and sectionName if
 * it exists and it is not a zerofill section.  If not it returns NULL.  If
 * the parameter size is not NULL the size of the section is also returned
 * indirectly through that pointer.
 */
void *
NSGetSectionDataInObjectFileImage(
NSObjectFileImage objectFileImage,
const char* segmentName,
const char* sectionName,
unsigned long *size) /* can be NULL */
{
	DYLD_LOCK_THIS_BLOCK;
    static void* (*p)(NSObjectFileImage, const char*, const char*, unsigned long*) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_NSGetSectionDataInObjectFileImage", (void**)&p);
		
	return p(objectFileImage, segmentName, sectionName, size);
}


void
NSLinkEditError(
NSLinkEditErrors *c,
int *errorNumber, 
const char* *fileName,
const char* *errorString)
{
	DYLD_LOCK_THIS_BLOCK;
    static void (*p)(NSLinkEditErrors *c,
		     int *errorNumber, 
		     const char* *fileName,
		     const char* *errorString) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_link_edit_error", (void**)&p);
	if(p != NULL)
	    p(c, errorNumber, fileName, errorString);
}

bool
NSUnLinkModule(
NSModule module, 
uint32_t options)
{
	DYLD_LOCK_THIS_BLOCK;
    static bool (*p)(NSModule module, uint32_t options) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_unlink_module", (void**)&p);

	return p(module, options);
}

#if OBSOLETE_DYLD_API
NSModule
NSReplaceModule(
NSModule moduleToReplace,
NSObjectFileImage newObjectFileImage, 
uint32_t options)
{
	return(NULL);
}
#endif


#endif // DEPRECATED_APIS_SUPPORTED

/*
 *_NSGetExecutablePath copies the path of the executable into the buffer and
 * returns 0 if the path was successfully copied in the provided buffer. If the
 * buffer is not large enough, -1 is returned and the expected buffer size is
 * copied in *bufsize. Note that _NSGetExecutablePath will return "a path" to
 * the executable not a "real path" to the executable. That is the path may be
 * a symbolic link and not the real file. And with deep directories the total
 * bufsize needed could be more than MAXPATHLEN.
 */
int
_NSGetExecutablePath(
char *buf,
uint32_t *bufsize)
{
	DYLD_NO_LOCK_THIS_BLOCK;
    static int (*p)(char *buf, uint32_t *bufsize) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld__NSGetExecutablePath", (void**)&p);
	return(p(buf, bufsize));
}

#if DEPRECATED_APIS_SUPPORTED
void
_dyld_lookup_and_bind(
const char* symbol_name,
void** address,
NSModule* module)
{
	DYLD_LOCK_THIS_BLOCK;
    static void (*p)(const char*, void** , NSModule*) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_lookup_and_bind", (void**)&p);
	p(symbol_name, address, module);
}

void
_dyld_lookup_and_bind_with_hint(
const char* symbol_name,
const char* library_name_hint,
void** address,
NSModule* module)
{
	DYLD_LOCK_THIS_BLOCK;
    static void (*p)(const char*, const char*, void**, NSModule*) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_lookup_and_bind_with_hint", (void**)&p);
	p(symbol_name, library_name_hint, address, module);
}

#if OBSOLETE_DYLD_API
void
_dyld_lookup_and_bind_objc(
const char* symbol_name,
void** address,
NSModule* module)
{
	DYLD_LOCK_THIS_BLOCK;
    static void (*p)(const char* , void**, NSModule*) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_lookup_and_bind_objc", (void**)&p);
	p(symbol_name, address, module);
}
#endif

void
_dyld_lookup_and_bind_fully(
const char* symbol_name,
void** address,
NSModule* module)
{
	DYLD_LOCK_THIS_BLOCK;
    static void (*p)(const char*, void**, NSModule*) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_lookup_and_bind_fully", (void**)&p);
	p(symbol_name, address, module);
}

bool
_dyld_bind_fully_image_containing_address(
const void* address)
{
	DYLD_LOCK_THIS_BLOCK;
    static bool (*p)(const void*) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_bind_fully_image_containing_address", (void**)&p);
	return p(address);
}
#endif // DEPRECATED_APIS_SUPPORTED


/*
 * _dyld_register_func_for_add_image registers the specified function to be
 * called when a new image is added (a bundle or a dynamic shared library) to
 * the program.  When this function is first registered it is called for once
 * for each image that is currently part of the program.
 */
void
_dyld_register_func_for_add_image(
void (*func)(const struct mach_header *mh, intptr_t vmaddr_slide))
{
	DYLD_LOCK_THIS_BLOCK;
	typedef void (*callback_t)(const struct mach_header *mh, intptr_t vmaddr_slide);
    static void (*p)(callback_t func) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_register_func_for_add_image", (void**)&p);
	p(func);
}

/*
 * _dyld_register_func_for_remove_image registers the specified function to be
 * called when an image is removed (a bundle or a dynamic shared library) from
 * the program.
 */
void
_dyld_register_func_for_remove_image(
void (*func)(const struct mach_header *mh, intptr_t vmaddr_slide))
{
	DYLD_LOCK_THIS_BLOCK;
	typedef void (*callback_t)(const struct mach_header *mh, intptr_t vmaddr_slide);
    static void (*p)(callback_t func) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_register_func_for_remove_image", (void**)&p);
	p(func);
}

#if OBSOLETE_DYLD_API
/*
 * _dyld_register_func_for_link_module registers the specified function to be
 * called when a module is bound into the program.  When this function is first
 * registered it is called for once for each module that is currently bound into
 * the program.
 */
void
_dyld_register_func_for_link_module(
void (*func)(NSModule module))
{
	DYLD_LOCK_THIS_BLOCK;
    static void (*p)(void (*func)(NSModule module)) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_register_func_for_link_module", (void**)&p);
	p(func);
}

/*
 * _dyld_register_func_for_unlink_module registers the specified function to be
 * called when a module is unbound from the program.
 */
void
_dyld_register_func_for_unlink_module(
void (*func)(NSModule module))
{
	DYLD_LOCK_THIS_BLOCK;
    static void (*p)(void (*func)(NSModule module)) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_register_func_for_unlink_module", (void**)&p);
	p(func);
}

/*
 * _dyld_register_func_for_replace_module registers the specified function to be
 * called when a module is to be replace with another module in the program.
 */
void
_dyld_register_func_for_replace_module(
void (*func)(NSModule oldmodule, NSModule newmodule))
{
	DYLD_LOCK_THIS_BLOCK;
    static void (*p)(void (*func)(NSModule oldmodule,
				  NSModule newmodule)) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_register_func_for_replace_module", (void**)&p);
	p(func);
}


/*
 * _dyld_get_objc_module_sect_for_module is passed a module and sets a 
 * pointer to the (__OBJC,__module) section and its size for the specified
 * module.
 */
void
_dyld_get_objc_module_sect_for_module(
NSModule module,
void **objc_module,
unsigned long *size)
{
	DYLD_LOCK_THIS_BLOCK;
    static void (*p)(NSModule module,
		     void **objc_module,
		     unsigned long *size) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_get_objc_module_sect_for_module", (void**)&p);
	p(module, objc_module, size);
}

/*
 * _dyld_bind_objc_module() is passed a pointer to something in an (__OBJC,
 * __module) section and causes the module that is associated with that address
 * to be bound.
 */
void
_dyld_bind_objc_module(const void* objc_module)
{
	DYLD_LOCK_THIS_BLOCK;
    static void (*p)(const void *objc_module) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_bind_objc_module", (void**)&p);
	p(objc_module);
}
#endif

#if DEPRECATED_APIS_SUPPORTED
bool
_dyld_present(void)
{
	// this function exists for compatiblity only
	return true;
}
#endif

uint32_t
_dyld_image_count(void)
{
	DYLD_NO_LOCK_THIS_BLOCK;
    static uint32_t (*p)(void) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_image_count", (void**)&p);
	return(p());
}

const struct mach_header *
_dyld_get_image_header(uint32_t image_index)
{
	DYLD_NO_LOCK_THIS_BLOCK;
    static struct mach_header * (*p)(uint32_t image_index) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_get_image_header", (void**)&p);
	return(p(image_index));
}

intptr_t
_dyld_get_image_vmaddr_slide(uint32_t image_index)
{
	DYLD_NO_LOCK_THIS_BLOCK;
    static unsigned long (*p)(uint32_t image_index) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_get_image_vmaddr_slide", (void**)&p);
	return(p(image_index));
}

const char* 
_dyld_get_image_name(uint32_t image_index)
{
	DYLD_NO_LOCK_THIS_BLOCK;
    static const char*  (*p)(uint32_t image_index) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_get_image_name", (void**)&p);
	return(p(image_index));
}

// SPI in Mac OS X 10.6
intptr_t _dyld_get_image_slide(const struct mach_header* mh)
{
	DYLD_NO_LOCK_THIS_BLOCK;
    static intptr_t (*p)(const struct mach_header*) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_get_image_slide", (void**)&p);
	return(p(mh));
}


bool
_dyld_image_containing_address(const void* address)
{
	DYLD_LOCK_THIS_BLOCK;
    static bool (*p)(const void*) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_image_containing_address", (void**)&p);
	return(p(address));
}

const struct mach_header *
_dyld_get_image_header_containing_address(
const void* address)
{
	DYLD_LOCK_THIS_BLOCK;
    static const struct mach_header * (*p)(const void*) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_get_image_header_containing_address", (void**)&p);
	return p(address);
}


#if DEPRECATED_APIS_SUPPORTED
bool _dyld_launched_prebound(void)
{
	DYLD_LOCK_THIS_BLOCK;
    static bool (*p)(void) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_launched_prebound", (void**)&p);
	return(p());
}

bool _dyld_all_twolevel_modules_prebound(void)
{
	DYLD_LOCK_THIS_BLOCK;
    static bool (*p)(void) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_all_twolevel_modules_prebound", (void**)&p);
	return(p());
}
#endif // DEPRECATED_APIS_SUPPORTED


#include <dlfcn.h>
#include <stddef.h>
#include <pthread.h>
#include <stdlib.h>
#include <mach-o/dyld.h>
#include <servers/bootstrap.h>
#include "dyldLibSystemInterface.h"


// pthread key used to access per-thread dlerror message
static pthread_key_t dlerrorPerThreadKey;
static bool dlerrorPerThreadKeyInitialized = false;

// data kept per-thread
struct dlerrorPerThreadData
{
	size_t		sizeAllocated;
	char		message[1];
};

// function called by dyld to get buffer to store dlerror message
static char* getPerThreadBufferFor_dlerror(size_t sizeRequired)
{
	// ok to create key lazily because this function is called within dyld lock, so there is no race condition
	if (!dlerrorPerThreadKeyInitialized ) {
		// create key and tell pthread package to call free() on any data associated with key if thread dies
		pthread_key_create(&dlerrorPerThreadKey, &free);
		dlerrorPerThreadKeyInitialized = true;
	}

	const size_t size = (sizeRequired < 256) ? 256 : sizeRequired;
	dlerrorPerThreadData* data = (dlerrorPerThreadData*)pthread_getspecific(dlerrorPerThreadKey);
	if ( data == NULL ) {
		//int mallocSize = offsetof(dlerrorPerThreadData, message[size]);
		const size_t mallocSize = sizeof(dlerrorPerThreadData)+size;
		data = (dlerrorPerThreadData*)malloc(mallocSize);
		data->sizeAllocated = size;
		pthread_setspecific(dlerrorPerThreadKey, data);
	}
	else if ( data->sizeAllocated < sizeRequired ) {
		free(data);
		//int mallocSize = offsetof(dlerrorPerThreadData, message[size]);
		const size_t mallocSize = sizeof(dlerrorPerThreadData)+size;
		data = (dlerrorPerThreadData*)malloc(mallocSize);
		data->sizeAllocated = size;
		pthread_setspecific(dlerrorPerThreadKey, data);
	}
	return data->message;
}

// <rdar://problem/10595338> dlerror buffer leak
// Only allocate buffer if an actual error message needs to be set
static bool hasPerThreadBufferFor_dlerror()
{
	if (!dlerrorPerThreadKeyInitialized ) 
		return false;
		
	return (pthread_getspecific(dlerrorPerThreadKey) != NULL);
}

// use non-lazy pointer to vproc_swap_integer so that lazy binding does not recurse
typedef vproc_err_t (*vswapproc)(vproc_t vp, vproc_gsk_t key,int64_t *inval, int64_t *outval);
static vswapproc swapProc = &vproc_swap_integer;

static bool isLaunchdOwned()
{
	int64_t val = 0;
	(*swapProc)(NULL, VPROC_GSK_IS_MANAGED, NULL, &val);
	return ( val != 0 );
}

#if DYLD_SHARED_CACHE_SUPPORT
static void shared_cache_missing()
{
	// leave until dyld's that might call this are rare
}

static void shared_cache_out_of_date()
{
	// leave until dyld's that might call this are rare
}
#endif // DYLD_SHARED_CACHE_SUPPORT


// the table passed to dyld containing thread helpers
static dyld::LibSystemHelpers sHelpers = { 13, &dyldGlobalLockAcquire, &dyldGlobalLockRelease,
									&getPerThreadBufferFor_dlerror, &malloc, &free, &__cxa_atexit,
						#if DYLD_SHARED_CACHE_SUPPORT
									&shared_cache_missing, &shared_cache_out_of_date,
						#else
									NULL, NULL,
						#endif
									NULL, NULL,
									&pthread_key_create, &pthread_setspecific,
									&malloc_size,
									&pthread_getspecific,
									&__cxa_finalize,
									address_of_start,
									&hasPerThreadBufferFor_dlerror,
									&isLaunchdOwned,
									&vm_allocate,
									&mmap,
									&__cxa_finalize_ranges
									};


//
// during initialization of libSystem this routine will run
// and call dyld, registering the helper functions.
//
extern "C" void tlv_initializer();
extern "C" void _dyld_initializer();
void _dyld_initializer()
{	
   void (*p)(dyld::LibSystemHelpers*);

	_dyld_func_lookup("__dyld_register_thread_helpers", (void**)&p);
	if(p != NULL)
		p(&sHelpers);
		
	tlv_initializer();
}


char* dlerror()
{
	DYLD_LOCK_THIS_BLOCK;
    static char* (*p)() = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_dlerror", (void**)&p);
	return(p());
}

int dladdr(const void* addr, Dl_info* info)
{
	DYLD_LOCK_THIS_BLOCK;
    static int (*p)(const void* , Dl_info*) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_dladdr", (void**)&p);
	return(p(addr, info));
}

int dlclose(void* handle)
{
	DYLD_LOCK_THIS_BLOCK;
    static int (*p)(void* handle) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_dlclose", (void**)&p);
	return(p(handle));
}

void* dlopen(const char* path, int mode)
{	
	// dlopen is special. locking is done inside dyld to allow initializer to run without lock
	DYLD_NO_LOCK_THIS_BLOCK;
	
    static void* (*p)(const char* path, int) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_dlopen", (void**)&p);
	void* result = p(path, mode);
	// use asm block to prevent tail call optimization
	// this is needed because dlopen uses __builtin_return_address() and depends on this glue being in the frame chain
	// <rdar://problem/5313172 dlopen() looks too far up stack, can cause crash>
	__asm__ volatile(""); 
	
	return result;
}

bool dlopen_preflight(const char* path)
{
	DYLD_LOCK_THIS_BLOCK;
    static bool (*p)(const char* path) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_dlopen_preflight", (void**)&p);
	return(p(path));
}

void* dlsym(void* handle, const char* symbol)
{
	DYLD_LOCK_THIS_BLOCK;
    static void* (*p)(void* handle, const char* symbol) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_dlsym", (void**)&p);
	return(p(handle, symbol));
}


const struct dyld_all_image_infos* _dyld_get_all_image_infos()
{
	DYLD_NO_LOCK_THIS_BLOCK;
    static struct dyld_all_image_infos* (*p)() = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_get_all_image_infos", (void**)&p);
	return p();
}

#if SUPPORT_ZERO_COST_EXCEPTIONS
bool _dyld_find_unwind_sections(void* addr, dyld_unwind_sections* info)
{
	DYLD_NO_LOCK_THIS_BLOCK;
    static void* (*p)(void*, dyld_unwind_sections*) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_find_unwind_sections", (void**)&p);
	return p(addr, info);
}
#endif


#if __i386__ || __x86_64__ || __arm__ || __arm64__
__attribute__((visibility("hidden"))) 
void* _dyld_fast_stub_entry(void* loadercache, long lazyinfo)
{
	DYLD_NO_LOCK_THIS_BLOCK;
    static void* (*p)(void*, long) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_fast_stub_entry", (void**)&p);
	return p(loadercache, lazyinfo);
}
#endif


const char* dyld_image_path_containing_address(const void* addr)
{
	DYLD_NO_LOCK_THIS_BLOCK;
    static const char* (*p)(const void*) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_image_path_containing_address", (void**)&p);
	return p(addr);
}

const struct mach_header* dyld_image_header_containing_address(const void* addr)
{
	DYLD_NO_LOCK_THIS_BLOCK;
    static const mach_header* (*p)(const void*) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_get_image_header_containing_address", (void**)&p);
	return p(addr);
}


bool dyld_shared_cache_some_image_overridden()
{
	DYLD_NO_LOCK_THIS_BLOCK;
    static bool (*p)() = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_shared_cache_some_image_overridden", (void**)&p);
	return p();
}

bool _dyld_get_shared_cache_uuid(uuid_t uuid)
{
	DYLD_NO_LOCK_THIS_BLOCK;
    static bool (*p)(uuid_t) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_get_shared_cache_uuid", (void**)&p);
	return p(uuid);
}


bool dyld_process_is_restricted()
{
	DYLD_NO_LOCK_THIS_BLOCK;
    static bool (*p)() = NULL;
	
	if(p == NULL)
	    _dyld_func_lookup("__dyld_process_is_restricted", (void**)&p);
	return p();
}

#if DYLD_SHARED_CACHE_SUPPORT
const char* dyld_shared_cache_file_path()
{
	DYLD_NO_LOCK_THIS_BLOCK;
    static const char* (*p)() = NULL;
	
	if(p == NULL)
	    _dyld_func_lookup("__dyld_shared_cache_file_path", (void**)&p);
	return p();
}
#endif

void dyld_dynamic_interpose(const struct mach_header* mh, const struct dyld_interpose_tuple array[], size_t count)
{
	DYLD_LOCK_THIS_BLOCK;
    static void (*p)(const struct mach_header* mh, const struct dyld_interpose_tuple array[], size_t count) = NULL;

	if (p == NULL)
	    _dyld_func_lookup("__dyld_dynamic_interpose", (void**)&p);
	p(mh, array, count);
}


// SPI called __fork
void _dyld_fork_child()
{
	DYLD_NO_LOCK_THIS_BLOCK;
    static void (*p)() = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_fork_child", (void**)&p);
	return p();
}



static void* mapStartOfCache(const char* path, size_t length)
{
	struct stat statbuf;
	if ( ::stat(path, &statbuf) == -1 )
		return NULL;

	if ( statbuf.st_size < length )
		return NULL;

	int cache_fd = ::open(path, O_RDONLY);
	if ( cache_fd < 0 )
		return NULL;

	void* result = ::mmap(NULL, length, PROT_READ, MAP_PRIVATE, cache_fd, 0);
	close(cache_fd);

	if ( result == MAP_FAILED )
		return NULL;

	return result;
}


static const dyld_cache_header* findCacheInDirAndMap(const uuid_t cacheUuid, const char* dirPath)
{
	DIR* dirp = ::opendir(dirPath);
	if ( dirp != NULL) {
		dirent entry;
		dirent* entp = NULL;
		char cachePath[PATH_MAX];
		while ( ::readdir_r(dirp, &entry, &entp) == 0 ) {
			if ( entp == NULL )
				break;
			if ( entp->d_type != DT_REG ) 
				continue;
			if ( strlcpy(cachePath, dirPath, PATH_MAX) >= PATH_MAX )
				continue;
			if ( strlcat(cachePath, "/", PATH_MAX) >= PATH_MAX )
				continue;
			if ( strlcat(cachePath, entp->d_name, PATH_MAX) >= PATH_MAX )
				continue;
			if ( const dyld_cache_header* cacheHeader = (dyld_cache_header*)mapStartOfCache(cachePath, 0x00100000) ) {
				if ( ::memcmp(cacheHeader->uuid, cacheUuid, 16) != 0 ) {
					// wrong uuid, unmap and keep looking
					::munmap((void*)cacheHeader, 0x00100000);
				}
				else {
					// found cache
					closedir(dirp);
					return cacheHeader;
				}
			}
		}
		closedir(dirp);
	}
	return NULL;
}

int dyld_shared_cache_find_iterate_text(const uuid_t cacheUuid, const char* extraSearchDirs[], void (^callback)(const dyld_shared_cache_dylib_text_info* info))
{
	const dyld_cache_header* cacheHeader = NULL;
	bool needToUnmap = true;

	// get info from dyld about this process, to see if requested cache is already mapped into this process
	const dyld_all_image_infos* allInfo = _dyld_get_all_image_infos();
	if ( (allInfo != NULL) && (memcmp(allInfo->sharedCacheUUID, cacheUuid, 16) == 0) ) {
		// requested cache is already mapped, just re-use it
		cacheHeader = (dyld_cache_header*)(SHARED_REGION_BASE + allInfo->sharedCacheSlide);
		needToUnmap = false;
	}
	else {
		// look first is default location for cache files
	#if	__IPHONE_OS_VERSION_MIN_REQUIRED
		const char* defaultSearchDir = IPHONE_DYLD_SHARED_CACHE_DIR;
	#else
		const char* defaultSearchDir = MACOSX_DYLD_SHARED_CACHE_DIR;
	#endif
		cacheHeader = findCacheInDirAndMap(cacheUuid, defaultSearchDir);
		// if not there, look in extra search locations
		if ( cacheHeader == NULL ) {
			for (const char** p = extraSearchDirs; *p != NULL; ++p) {
				cacheHeader = findCacheInDirAndMap(cacheUuid, *p);
				if ( cacheHeader != NULL )
					break;
			}
		}
	}

	if ( cacheHeader == NULL )
		return -1;
	
	if ( cacheHeader->mappingOffset < sizeof(dyld_cache_header) ) {
		// old cache without imagesText array
		if ( needToUnmap )
			::munmap((void*)cacheHeader, 0x00100000);
		return -1;
	}

	// walk imageText table and call callback for each entry
	const dyld_cache_image_text_info* imagesText = (dyld_cache_image_text_info*)((char*)cacheHeader + cacheHeader->imagesTextOffset);
	const dyld_cache_image_text_info* imagesTextEnd = &imagesText[cacheHeader->imagesTextCount];
	for (const dyld_cache_image_text_info* p=imagesText; p < imagesTextEnd; ++p) {
		dyld_shared_cache_dylib_text_info dylibTextInfo;
		dylibTextInfo.version			= 1;
		dylibTextInfo.loadAddressUnslid = p->loadAddress;
		dylibTextInfo.textSegmentSize	= p->textSegmentSize;
		dylibTextInfo.path				= (char*)cacheHeader + p->pathOffset;
		::memcpy(dylibTextInfo.dylibUuid, p->uuid, 16);
		callback(&dylibTextInfo);
	}

	if ( needToUnmap )
		::munmap((void*)cacheHeader, 0x00100000);

	return 0;
}

int dyld_shared_cache_iterate_text(const uuid_t cacheUuid, void (^callback)(const dyld_shared_cache_dylib_text_info* info))
{
	const char* extraSearchDirs[] = { NULL };
	return dyld_shared_cache_find_iterate_text(cacheUuid, extraSearchDirs, callback);
}


bool _dyld_is_memory_immutable(const void* addr, size_t length)
{
	DYLD_NO_LOCK_THIS_BLOCK;
    static bool (*p)(const void*, size_t) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_is_memory_immutable", (void**)&p);
	return p(addr, length);
}


void _dyld_objc_notify_register(_dyld_objc_notify_mapped    mapped,
                                _dyld_objc_notify_init      init,
                                _dyld_objc_notify_unmapped  unmapped)
{
	DYLD_LOCK_THIS_BLOCK;
    static bool (*p)(_dyld_objc_notify_mapped, _dyld_objc_notify_init, _dyld_objc_notify_unmapped) = NULL;

	if(p == NULL)
	    _dyld_func_lookup("__dyld_objc_notify_register", (void**)&p);
	p(mapped, init, unmapped);
}





