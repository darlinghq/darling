/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2004-2009 Apple Inc. All rights reserved.
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

//
// This file implements that API's in <mach-o/dyld.h>
//
//

#define __STDC_LIMIT_MACROS
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <Availability.h>


#include <vector>
#include <map>
#include <algorithm>

#include <mach/mach.h>
#include <sys/time.h>
#include <sys/sysctl.h>
#include <mach/mach_traps.h> // for task_self_trap()


#include "mach-o/dyld_images.h"
#include "mach-o/dyld.h"
#include "mach-o/dyld_priv.h"

#include "ImageLoader.h"
#include "ImageLoaderMachO.h"
#include "dyld.h"
#include "dyldLibSystemInterface.h"

#undef _POSIX_C_SOURCE
#include "dlfcn.h"

// from dyldExceptions.c
extern "C" void __Unwind_SjLj_SetThreadKey(pthread_key_t key);

// from dyld_gdb.cpp 
extern void addImagesToAllImages(uint32_t infoCount, const dyld_image_info info[]);
extern uint32_t allImagesCount();
extern const mach_header* allImagesIndexedMachHeader(uint32_t index);
extern const char* allImagesIndexedPath(uint32_t index);


// deprecated APIs are still availble on Mac OS X, but not on iPhone OS
#if __IPHONE_OS_VERSION_MIN_REQUIRED	
	#define DEPRECATED_APIS_SUPPORTED 0
#else
	#define DEPRECATED_APIS_SUPPORTED 1
#endif

static bool sDynamicInterposing = false;

#if DEPRECATED_APIS_SUPPORTED
static char sLastErrorFilePath[1024];
static NSLinkEditErrors sLastErrorFileCode;
static int sLastErrorNo;
#endif

// In 10.3.x and earlier all the NSObjectFileImage API's were implemeneted in libSystem.dylib
// Beginning in 10.4 the NSObjectFileImage API's are implemented in dyld and libSystem just forwards
// This conditional keeps support for old libSystem's which needed some help implementing the API's
#define OLD_LIBSYSTEM_SUPPORT (__i386__)

// The following functions have no prototype in any header.  They are special cases
// where _dyld_func_lookup() is used directly.
static void _dyld_make_delayed_module_initializer_calls();
static void registerThreadHelpers(const dyld::LibSystemHelpers*);
#if DEPRECATED_APIS_SUPPORTED
static void _dyld_install_handlers(void* undefined, void* multiple, void* linkEdit);
#if OLD_LIBSYSTEM_SUPPORT
static NSModule _dyld_link_module(NSObjectFileImage object_addr, size_t object_size, const char* moduleName, uint32_t options);
#endif
static void _dyld_register_binding_handler(void * (*)(const char *, const char *, void *), ImageLoader::BindingOptions);
static bool NSMakePrivateModulePublic(NSModule module);
static void _dyld_call_module_initializers_for_dylib(const struct mach_header* mh_dylib_header);

// The following functions are dyld API's, but since dyld links with a static copy of libc.a
// the public name cannot be used.
static void		client_dyld_lookup_and_bind(const char* symbolName, void** address, NSModule* module);
static bool		client_NSIsSymbolNameDefined(const char* symbolName);
#endif // DEPRECATED_APIS_SUPPORTED
#if SUPPORT_ZERO_COST_EXCEPTIONS
static bool client_dyld_find_unwind_sections(void* addr, dyld_unwind_sections* info);
#endif
#if DEPRECATED_APIS_SUPPORTED
#endif

static void unimplemented()
{
	dyld::halt("unimplemented dyld function\n");
}

struct dyld_func {
    const char* name;
    void*		implementation;
};

static struct dyld_func dyld_funcs[] = {
    {"__dyld_register_func_for_add_image",				(void*)_dyld_register_func_for_add_image },
    {"__dyld_register_func_for_remove_image",			(void*)_dyld_register_func_for_remove_image },
    {"__dyld_dladdr",									(void*)dladdr },
    {"__dyld_dlclose",									(void*)dlclose },
    {"__dyld_dlerror",									(void*)dlerror },
    {"__dyld_dlopen",									(void*)dlopen },
    {"__dyld_dlsym",									(void*)dlsym },
    {"__dyld_dlopen_preflight",							(void*)dlopen_preflight },
	{"__dyld_image_count",								(void*)_dyld_image_count },
    {"__dyld_get_image_header",							(void*)_dyld_get_image_header },
    {"__dyld_get_image_vmaddr_slide",					(void*)_dyld_get_image_vmaddr_slide },
    {"__dyld_get_image_name",							(void*)_dyld_get_image_name },
    {"__dyld_get_image_slide",							(void*)_dyld_get_image_slide },
    {"__dyld__NSGetExecutablePath",						(void*)_NSGetExecutablePath },

	// SPIs
  	{"__dyld_register_thread_helpers",					(void*)registerThreadHelpers },
	{"__dyld_fork_child",								(void*)_dyld_fork_child },
    {"__dyld_make_delayed_module_initializer_calls",	(void*)_dyld_make_delayed_module_initializer_calls },
	{"__dyld_get_all_image_infos",						(void*)_dyld_get_all_image_infos },
#if SUPPORT_ZERO_COST_EXCEPTIONS
	{"__dyld_find_unwind_sections",						(void*)client_dyld_find_unwind_sections },
#endif
#if __i386__ || __x86_64__ || __arm__ || __arm64__
	{"__dyld_fast_stub_entry",							(void*)dyld::fastBindLazySymbol },
#endif
	{"__dyld_image_path_containing_address",			(void*)dyld_image_path_containing_address },
	{"__dyld_shared_cache_some_image_overridden",		(void*)dyld_shared_cache_some_image_overridden },
	{"__dyld_process_is_restricted",					(void*)dyld::processIsRestricted },
	{"__dyld_dynamic_interpose",						(void*)dyld_dynamic_interpose },
#if DYLD_SHARED_CACHE_SUPPORT
	{"__dyld_shared_cache_file_path",					(void*)dyld::getStandardSharedCacheFilePath },
#endif
    {"__dyld_get_image_header_containing_address",		(void*)dyld_image_header_containing_address },
    {"__dyld_is_memory_immutable",						(void*)_dyld_is_memory_immutable },
    {"__dyld_objc_notify_register",						(void*)_dyld_objc_notify_register },
    {"__dyld_get_shared_cache_uuid",					(void*)_dyld_get_shared_cache_uuid },


	// deprecated
#if DEPRECATED_APIS_SUPPORTED
    {"__dyld_lookup_and_bind",						(void*)client_dyld_lookup_and_bind },
    {"__dyld_lookup_and_bind_with_hint",			(void*)_dyld_lookup_and_bind_with_hint },
    {"__dyld_lookup_and_bind_fully",				(void*)_dyld_lookup_and_bind_fully },
    {"__dyld_install_handlers",						(void*)_dyld_install_handlers },
    {"__dyld_link_edit_error",						(void*)NSLinkEditError },
    {"__dyld_unlink_module",						(void*)NSUnLinkModule },
    {"__dyld_bind_objc_module",						(void*)_dyld_bind_objc_module },
    {"__dyld_bind_fully_image_containing_address",  (void*)_dyld_bind_fully_image_containing_address },
    {"__dyld_image_containing_address",				(void*)_dyld_image_containing_address },
    {"__dyld_register_binding_handler",				(void*)_dyld_register_binding_handler },
    {"__dyld_NSNameOfSymbol",						(void*)NSNameOfSymbol },
    {"__dyld_NSAddressOfSymbol",					(void*)NSAddressOfSymbol },
    {"__dyld_NSModuleForSymbol",					(void*)NSModuleForSymbol },
    {"__dyld_NSLookupAndBindSymbol",				(void*)NSLookupAndBindSymbol },
    {"__dyld_NSLookupAndBindSymbolWithHint",		(void*)NSLookupAndBindSymbolWithHint },
    {"__dyld_NSLookupSymbolInModule",				(void*)NSLookupSymbolInModule},
    {"__dyld_NSLookupSymbolInImage",				(void*)NSLookupSymbolInImage},
    {"__dyld_NSMakePrivateModulePublic",			(void*)NSMakePrivateModulePublic},
    {"__dyld_NSIsSymbolNameDefined",				(void*)client_NSIsSymbolNameDefined},
    {"__dyld_NSIsSymbolNameDefinedWithHint",		(void*)NSIsSymbolNameDefinedWithHint },
    {"__dyld_NSIsSymbolNameDefinedInImage",			(void*)NSIsSymbolNameDefinedInImage},
    {"__dyld_NSNameOfModule",						(void*)NSNameOfModule },
    {"__dyld_NSLibraryNameForModule",				(void*)NSLibraryNameForModule },
    {"__dyld_NSAddLibrary",							(void*)NSAddLibrary },
    {"__dyld_NSAddLibraryWithSearching",			(void*)NSAddLibraryWithSearching },
    {"__dyld_NSAddImage",							(void*)NSAddImage },
    {"__dyld_launched_prebound",					(void*)_dyld_launched_prebound },
    {"__dyld_all_twolevel_modules_prebound",		(void*)_dyld_all_twolevel_modules_prebound },
    {"__dyld_call_module_initializers_for_dylib",   (void*)_dyld_call_module_initializers_for_dylib },
    {"__dyld_NSCreateObjectFileImageFromFile",			(void*)NSCreateObjectFileImageFromFile },
    {"__dyld_NSCreateObjectFileImageFromMemory",		(void*)NSCreateObjectFileImageFromMemory },
    {"__dyld_NSDestroyObjectFileImage",					(void*)NSDestroyObjectFileImage },
    {"__dyld_NSLinkModule",								(void*)NSLinkModule },
    {"__dyld_NSHasModInitObjectFileImage",				(void*)NSHasModInitObjectFileImage },
    {"__dyld_NSSymbolDefinitionCountInObjectFileImage",	(void*)NSSymbolDefinitionCountInObjectFileImage },
    {"__dyld_NSSymbolDefinitionNameInObjectFileImage",	(void*)NSSymbolDefinitionNameInObjectFileImage },
    {"__dyld_NSIsSymbolDefinedInObjectFileImage",		(void*)NSIsSymbolDefinedInObjectFileImage },
    {"__dyld_NSSymbolReferenceNameInObjectFileImage",	(void*)NSSymbolReferenceNameInObjectFileImage },
    {"__dyld_NSSymbolReferenceCountInObjectFileImage",	(void*)NSSymbolReferenceCountInObjectFileImage },
    {"__dyld_NSGetSectionDataInObjectFileImage",		(void*)NSGetSectionDataInObjectFileImage },
#if OLD_LIBSYSTEM_SUPPORT
    {"__dyld_link_module",							(void*)_dyld_link_module },
#endif
#endif //DEPRECATED_APIS_SUPPORTED

    {NULL, 0}
};



#if DEPRECATED_APIS_SUPPORTED

static void dyldAPIhalt(const char* apiName, const char* errorMsg)
{
	dyld::log("dyld: %s() error\n", apiName);
	dyld::halt(errorMsg);
}

// dyld's abstract type NSSymbol is implemented as const ImageLoader::Symbol*
inline NSSymbol SymbolToNSSymbol(const ImageLoader::Symbol* sym)
{
	return (NSSymbol)sym;
}	
inline const ImageLoader::Symbol* NSSymbolToSymbol(NSSymbol sym)
{
	return (const ImageLoader::Symbol*)sym;
}	

// dyld's abstract type NSModule is implemented as ImageLoader*
inline NSModule ImageLoaderToNSModule(const ImageLoader* image)
{
	return (NSModule)image;
}	
inline ImageLoader* NSModuleToImageLoader(NSModule module)
{
	ImageLoader* image = (ImageLoader*)module;
	if ( dyld::validImage(image) )
		return image;
	return NULL;
}

// actual definition for opaque type
struct __NSObjectFileImage
{
	ImageLoader*	image;	
	const void*		imageBaseAddress;	// not used with OFI created from files
	size_t			imageLength;		// not used with OFI created from files
};


VECTOR_NEVER_DESTRUCTED(NSObjectFileImage);
static std::vector<NSObjectFileImage> sObjectFileImages;



//
// __NSObjectFileImage are deleted in NSDestroyObjectFileImage()
// The contained image is delete in one of two places:
//	NSUnLinkModule deletes the image if there is no __NSObjectFileImage with a reference to it
//	NSDestroyObjectFileImage deletes the image if image is not in list of valid images
//



static void setLastError(NSLinkEditErrors code, int errnum, const char* file, const char* message)
{
	dyld::setErrorMessage(message);
	strncpy(sLastErrorFilePath, file, 1024);
	sLastErrorFilePath[1023] = '\0';
	sLastErrorFileCode = code;
	sLastErrorNo = errnum;
}

#endif // DEPRECATED_APIS_SUPPORTED

/*
 *_dyld_NSGetExecutablePath is the dyld side of _NSGetExecutablePath which
 * copies the path of the executable into the buffer and returns 0 if the path
 * was successfully copied in the provided buffer. If the buffer is not large
 * enough, -1 is returned and the expected buffer size is copied in *bufsize.
 * Note that _NSGetExecutablePath will return "a path" to the executable not a
 * "real path" to the executable. That is the path may be a symbolic link and
 * not the real file. And with deep directories the total bufsize needed could
 * be more than MAXPATHLEN.
 */
int _NSGetExecutablePath(char* buf, uint32_t *bufsize)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(...)\n", __func__);
	const char* exePath = dyld::getExecutablePath();
	if(*bufsize < strlen(exePath) + 1){
	    *bufsize = (uint32_t)(strlen(exePath) + 1);
	    return -1;
	}
	strcpy(buf, exePath);
	return 0;
}

uint32_t _dyld_image_count(void)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s()\n", __func__);
	return allImagesCount();
}

const struct mach_header* _dyld_get_image_header(uint32_t image_index)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%u)\n", __func__, image_index);
	return allImagesIndexedMachHeader(image_index);
}

intptr_t _dyld_get_image_vmaddr_slide(uint32_t image_index)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%u)\n", __func__, image_index);
	const struct mach_header* mh = allImagesIndexedMachHeader(image_index);
	if ( mh != NULL )
		return ImageLoaderMachO::computeSlide(mh);
	else
		return 0;
}

intptr_t _dyld_get_image_slide(const struct mach_header* mh)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p)\n", __func__, mh);
	return ImageLoaderMachO::computeSlide(mh);
}


const char* _dyld_get_image_name(uint32_t image_index)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%u)\n", __func__, image_index);
	return allImagesIndexedPath(image_index);
}

const struct mach_header * dyld_image_header_containing_address(const void* address)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p)\n", __func__, address);
#if SUPPORT_ACCELERATE_TABLES
	const mach_header* mh;
	const char* path;
	if ( dyld::addressInCache(address, &mh, &path) )
		return mh;
#endif
	ImageLoader* image = dyld::findImageContainingAddress(address);
	if ( image != NULL ) 
		return image->machHeader();
	return NULL;
}


void _dyld_register_func_for_add_image(void (*func)(const struct mach_header *mh, intptr_t vmaddr_slide))
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p)\n", __func__, (void *)func);
	dyld::registerAddCallback(func);
}

void _dyld_register_func_for_remove_image(void (*func)(const struct mach_header *mh, intptr_t vmaddr_slide))
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p)\n", __func__, (void *)func);
	dyld::registerRemoveCallback(func);
}



// called by crt before main() by programs linked with 10.4 or earlier crt1.o
static void _dyld_make_delayed_module_initializer_calls()
{
	if ( dyld::gLogAPIs )
		dyld::log("%s()\n", __func__);
		
#if SUPPORT_OLD_CRT_INITIALIZATION
	if ( dyld::gRunInitializersOldWay )
		dyld::initializeMainExecutable();
#endif
}



#if DEPRECATED_APIS_SUPPORTED

//
// _dyld_call_module_initializers_for_dylib() is the dyld side of
// __initialize_Cplusplus() which is in dylib1.o.
// It is intended to only be called inside -init rouintes.
// -init routines are called before module initializers (what C++
// initializers use).  Calling __initialize_Cplusplus() in a -init
// routine causes the module initializers for an image to be called
// which then allows C++ to be used inside a -init routine
//
static void _dyld_call_module_initializers_for_dylib(const struct mach_header* mh_dylib_header)
{
	if ( dyld::gLogAPIs )
		dyld::log("__initialize_Cplusplus()\n");
	
	// for now, do nothing...
}


void _dyld_lookup_and_bind_fully(const char* symbolName, void** address, NSModule* module)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(\"%s\", %p, %p)\n", __func__, symbolName, address, module);
	ImageLoader* image;
	const ImageLoader::Symbol* sym;
	dyld::clearErrorMessage();
	if ( dyld::flatFindExportedSymbol(symbolName, &sym, (const ImageLoader**)&image) ) {	
		try {
			image->bindAllLazyPointers(dyld::gLinkContext, true);
			if ( address != NULL)
				*address = (void*)image->getExportedSymbolAddress(sym, dyld::gLinkContext);
			if ( module != NULL)
				*module = ImageLoaderToNSModule(image);
		}
		catch (const char* msg) {
			dyldAPIhalt(__func__, msg);
		}
	}
	else {
		// on failure to find symbol return NULLs
		if ( address != NULL)
			*address = NULL;
		if ( module != NULL)
			*module = NULL;
	}
}

// Note: This cannot have public name because dyld is built with a static copy of libc.a
// which calls dyld_lookup_and_bind() and expects to find dyld's symbols not host process
static void client_dyld_lookup_and_bind(const char* symbolName, void** address, NSModule* module)
{
	if ( dyld::gLogAPIs )
		dyld::log("_dyld_lookup_and_bind(\"%s\", %p, %p)\n", symbolName, address, module);
	const ImageLoader* image;
	const ImageLoader::Symbol* sym;
	if ( dyld::flatFindExportedSymbol(symbolName, &sym, &image) ) {
		if ( address != NULL)
			*address = (void*)image->getExportedSymbolAddress(sym, dyld::gLinkContext);
		if ( module != NULL)
			*module = ImageLoaderToNSModule(image);
	}
	else {
		// on failure to find symbol return NULLs
		if ( address != NULL)
			*address = NULL;
		if ( module != NULL)
			*module = NULL;
	}
}

void _dyld_lookup_and_bind_with_hint(const char* symbolName, const char* library_name_hint, void** address, NSModule* module)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(\"%s\", \"%s\", %p, %p)\n", __func__, symbolName, library_name_hint, address, module);
	const ImageLoader* image;
	const ImageLoader::Symbol* sym;
	// Look for library whose path contains the hint.  If that fails search everywhere
	if (  dyld::flatFindExportedSymbolWithHint(symbolName, library_name_hint, &sym, &image) 
	  ||  dyld::flatFindExportedSymbol(symbolName, &sym, &image) ) {
		if ( address != NULL)
			*address = (void*)image->getExportedSymbolAddress(sym, dyld::gLinkContext);
		if ( module != NULL)
			*module = ImageLoaderToNSModule(image);
	}
	else {
		// on failure to find symbol return NULLs
		if ( address != NULL)
			*address = NULL;
		if ( module != NULL)
			*module = NULL;
	}
}


NSSymbol NSLookupAndBindSymbol(const char *symbolName)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(\"%s\")\n", __func__, symbolName);
	const ImageLoader* image;
	const ImageLoader::Symbol* sym;
	if ( dyld::flatFindExportedSymbol(symbolName, &sym, &image) ) {
		return SymbolToNSSymbol(sym);
	}
	// return NULL on failure
	return NULL;
}

NSSymbol NSLookupAndBindSymbolWithHint(const char* symbolName, const char* libraryNameHint)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(\"%s\", \"%s\")\n", __func__, symbolName, libraryNameHint);
	const ImageLoader* image;
	const ImageLoader::Symbol* sym;
	bool found = dyld::flatFindExportedSymbolWithHint(symbolName, libraryNameHint, &sym, &image);
	if ( ! found ) {
		// hint failed, do slow search of all images
		 found = dyld::flatFindExportedSymbol(symbolName, &sym, &image);
	}
	if ( found )
		return SymbolToNSSymbol(sym);
		
	// return NULL on failure and log
	if ( dyld::gLogAPIs )
		dyld::log("%s(\"%s\", \"%s\") => NULL \n", __func__, symbolName, libraryNameHint);
	return NULL;
}




static __attribute__((noinline)) 
const struct mach_header* addImage(void* callerAddress, const char* path, bool search, bool dontLoad, bool matchInstallName, bool abortOnError)
{
	ImageLoader*	image = NULL;
	std::vector<const char*> rpathsFromCallerImage;
	try {
		dyld::clearErrorMessage();
		ImageLoader* callerImage = dyld::findImageContainingAddress(callerAddress);
		// like dlopen, use rpath from caller image and from main executable
		if ( callerImage != NULL )
			callerImage->getRPaths(dyld::gLinkContext, rpathsFromCallerImage);
		ImageLoader::RPathChain callersRPaths(NULL, &rpathsFromCallerImage);
		if ( callerImage != dyld::mainExecutable() ) {
			dyld::mainExecutable()->getRPaths(dyld::gLinkContext, rpathsFromCallerImage);
		}
		dyld::LoadContext context;
		context.useSearchPaths		= search;
		context.useFallbackPaths	= search;
		context.useLdLibraryPath	= false;
		context.implicitRPath		= false;
		context.matchByInstallName	= matchInstallName;
		context.dontLoad			= dontLoad;
		context.mustBeBundle		= false;
		context.mustBeDylib			= true;
		context.canBePIE			= false;
		context.origin				= callerImage != NULL ? callerImage->getPath() : NULL; // caller's image's path
		context.rpath				= &callersRPaths; 	// rpaths from caller and main executable

		unsigned cacheIndex;
		image = load(path, context, cacheIndex);
		if ( image != NULL ) {
			if ( context.matchByInstallName )
				image->setMatchInstallPath(true);
			dyld::link(image, false, false, callersRPaths, cacheIndex);
			dyld::runInitializers(image);
			// images added with NSAddImage() can never be unloaded
			image->setNeverUnload(); 
		}
	}
	catch (const char* msg) {
		dyld::garbageCollectImages();
		if ( abortOnError) {
			char pathMsg[strlen(msg)+strlen(path)+4];
			strcpy(pathMsg, msg);
			strcat(pathMsg, " ");
			strcat(pathMsg, path);
			dyldAPIhalt("NSAddImage", pathMsg);
		}
		// not halting, so set error state for NSLinkEditError to find
		setLastError(NSLinkEditOtherError, 0, path, msg);
		free((void*)msg); 	// our free() will do nothing if msg is a string literal
		image = NULL;
	}
	// free rpaths (getRPaths() malloc'ed each string)
	for(std::vector<const char*>::iterator it=rpathsFromCallerImage.begin(); it != rpathsFromCallerImage.end(); ++it) {
		const char* str = *it;
		free((void*)str);
	}
	if ( image == NULL )
		return NULL;
	else
		return image->machHeader();
}


const struct mach_header* NSAddImage(const char* path, uint32_t options)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(\"%s\", 0x%08X)\n", __func__, path, options);
	const bool dontLoad = ( (options & NSADDIMAGE_OPTION_RETURN_ONLY_IF_LOADED) != 0 );
	const bool search = ( (options & NSADDIMAGE_OPTION_WITH_SEARCHING) != 0 );
	const bool matchInstallName = ( (options & NSADDIMAGE_OPTION_MATCH_FILENAME_BY_INSTALLNAME) != 0 );
	const bool abortOnError = ( (options & (NSADDIMAGE_OPTION_RETURN_ON_ERROR|NSADDIMAGE_OPTION_RETURN_ONLY_IF_LOADED)) == 0 );
	void* callerAddress = __builtin_return_address(1); // note layers: 1: real client, 0: libSystem glue
	return addImage(callerAddress, path, search, dontLoad, matchInstallName, abortOnError);
}

bool NSAddLibrary(const char* path)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(\"%s\")\n", __func__, path);
	void* callerAddress = __builtin_return_address(1); // note layers: 1: real client, 0: libSystem glue
	return (addImage(callerAddress, path, false, false, false, false) != NULL);
}

bool NSAddLibraryWithSearching(const char* path)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(\"%s\")\n", __func__, path);
	void* callerAddress = __builtin_return_address(1); // note layers: 1: real client, 0: libSystem glue
	return (addImage(callerAddress, path, true, false, false, false) != NULL);
}



//#define NSADDIMAGE_OPTION_NONE                  	0x0
//#define NSADDIMAGE_OPTION_RETURN_ON_ERROR       	0x1
//#define NSADDIMAGE_OPTION_MATCH_FILENAME_BY_INSTALLNAME	0x8

bool NSIsSymbolNameDefinedInImage(const struct mach_header* mh, const char* symbolName)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p, \"%s\")\n", __func__, (void *)mh, symbolName);
	ImageLoader* image = dyld::findImageByMachHeader(mh);
	if ( image != NULL ) {
		if ( image->findExportedSymbol(symbolName, true, NULL) != NULL)
			return true;
	}
	return false;
}


NSSymbol NSLookupSymbolInImage(const struct mach_header* mh, const char* symbolName, uint32_t options)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p, \"%s\", 0x%08X)\n", __func__, mh, symbolName, options);
	const ImageLoader::Symbol* symbol = NULL;
	dyld::clearErrorMessage();
	ImageLoader* image = dyld::findImageByMachHeader(mh);
	if ( image != NULL ) {
		try {
			if ( options & NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_FULLY ) {
				image->bindAllLazyPointers(dyld::gLinkContext, true);
			}
			else if ( options & NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW ) {
				image->bindAllLazyPointers(dyld::gLinkContext, false);
			}
		}
		catch (const char* msg) {
			if ( (options & NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR) == 0 ) {
				dyldAPIhalt(__func__, msg);
			}
		}
		symbol = image->findExportedSymbol(symbolName, true, NULL);
	}
	if ( dyld::gLogAPIs && (symbol == NULL) )
		dyld::log("%s(%p, \"%s\", 0x%08X) ==> NULL\n", __func__, mh, symbolName, options);
	return SymbolToNSSymbol(symbol);
}


// Note: This cannot have public name because dyld is built with a static copy of libc.a
// which calls NSIsSymbolNameDefined() and expects to find dyld's symbols not host process
static bool client_NSIsSymbolNameDefined(const char* symbolName)
{
	if ( dyld::gLogAPIs )
		dyld::log("NSIsSymbolNameDefined(\"%s\")\n", symbolName);
	const ImageLoader* image;
	const ImageLoader::Symbol* sym;
	return dyld::flatFindExportedSymbol(symbolName, &sym, &image);
}

bool NSIsSymbolNameDefinedWithHint(const char* symbolName, const char* libraryNameHint)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(\"%s\", \"%s\")\n", __func__, symbolName, libraryNameHint);
	const ImageLoader* image;
	const ImageLoader::Symbol* sym;
	bool found = dyld::flatFindExportedSymbolWithHint(symbolName, libraryNameHint, &sym, &image);
	if ( ! found ) {
		// hint failed, do slow search of all images
		 found = dyld::flatFindExportedSymbol(symbolName, &sym, &image);
	}
	if ( !found && dyld::gLogAPIs )
		dyld::log("%s(\"%s\", \"%s\") => false \n", __func__, symbolName, libraryNameHint);
	return found;
}

const char* NSNameOfSymbol(NSSymbol symbol)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p)\n", __func__, (void *)symbol);
	const char* result = NULL;
	ImageLoader* image = dyld::findImageContainingSymbol(symbol);
	if ( image != NULL ) 
		result = image->getExportedSymbolName(NSSymbolToSymbol(symbol));
	return result;
}

void* NSAddressOfSymbol(NSSymbol symbol)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p)\n", __func__, (void *)symbol);
	if ( symbol == NULL )
		return NULL;
	void* result = NULL;
	ImageLoader* image = dyld::findImageContainingSymbol(symbol);
	if ( image != NULL ) 
		result = (void*)image->getExportedSymbolAddress(NSSymbolToSymbol(symbol), dyld::gLinkContext);
	return result;
}

NSModule NSModuleForSymbol(NSSymbol symbol)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p)\n", __func__, (void *)symbol);
	NSModule result = NULL;
	ImageLoader* image = dyld::findImageContainingSymbol(symbol);
	if ( image != NULL ) 
		result = ImageLoaderToNSModule(image);
	return result;
}




bool _dyld_all_twolevel_modules_prebound(void)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s()\n", __func__);
	return FALSE; 
}

void _dyld_bind_objc_module(const void *objc_module)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p)\n", __func__, objc_module);
	// do nothing, with new dyld everything already bound
}


bool _dyld_bind_fully_image_containing_address(const void* address)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p)\n", __func__, address);
	dyld::clearErrorMessage();
	ImageLoader* image = dyld::findImageContainingAddress(address);
	if ( image != NULL ) {
		try {
			image->bindAllLazyPointers(dyld::gLinkContext, true);
			return true;
		}
		catch (const char* msg) {
			dyldAPIhalt(__func__, msg);
		}
	}
	return false;
}

bool _dyld_image_containing_address(const void* address)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p)\n", __func__, address);
	ImageLoader *imageLoader = dyld::findImageContainingAddress(address);
	return (NULL != imageLoader);
}

static NSObjectFileImage createObjectImageFile(ImageLoader* image, const void* address = NULL, size_t len=0)
{
	NSObjectFileImage result = new __NSObjectFileImage();
	result->image = image;
	result->imageBaseAddress = address;
	result->imageLength = len;
	sObjectFileImages.push_back(result);
	return result;
}

NSObjectFileImageReturnCode NSCreateObjectFileImageFromFile(const char* pathName, NSObjectFileImage *objectFileImage)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(\"%s\", ...)\n", __func__, pathName);
	try {
		void* callerAddress = __builtin_return_address(1); // note layers: 1: real client, 0: libSystem glue
		ImageLoader* callerImage = dyld::findImageContainingAddress(callerAddress);

		dyld::LoadContext context;
		context.useSearchPaths		= false;
		context.useFallbackPaths	= false;
		context.useLdLibraryPath	= false;
		context.implicitRPath		= false;
		context.matchByInstallName	= false;
		context.dontLoad			= false;
		context.mustBeBundle		= true;
		context.mustBeDylib			= false;
		context.canBePIE			= false;
		context.origin				= callerImage != NULL ? callerImage->getPath() : NULL; // caller's image's path
		context.rpath				= NULL; // support not yet implemented

		unsigned cacheIndex;
		ImageLoader* image = dyld::load(pathName, context, cacheIndex);
		// Note:  We DO NOT link the image!  NSLinkModule will do that
		if ( image != NULL ) {
			if ( !image->isBundle() ) {
				// the image must have been already loaded (since context.mustBeBundle will prevent it from being loaded)
				return NSObjectFileImageInappropriateFile;
			}
			*objectFileImage = createObjectImageFile(image);
			return NSObjectFileImageSuccess;
		}
	}
	catch (const char* msg) {
		//dyld::log("dyld: NSCreateObjectFileImageFromFile() error: %s\n", msg);
		dyld::garbageCollectImages();
		free((void*)msg);
		return NSObjectFileImageInappropriateFile;
	}
	return NSObjectFileImageFailure;
}


NSObjectFileImageReturnCode NSCreateObjectFileImageFromMemory(const void* address, size_t size, NSObjectFileImage *objectFileImage)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p, %lu, %p)\n", __func__, address, size, objectFileImage);
	
	try {
		ImageLoader* image = dyld::loadFromMemory((const uint8_t*)address, size, NULL); 
		if ( ! image->isBundle() ) {
			// this API can only be used with bundles...
			dyld::garbageCollectImages();
			return NSObjectFileImageInappropriateFile;
		}
		// Note:  We DO NOT link the image!  NSLinkModule will do that
		if ( image != NULL ) {
			*objectFileImage = createObjectImageFile(image, address, size);
			return NSObjectFileImageSuccess;
		}
	}
	catch (const char* msg) {
		free((void*)msg);
		dyld::garbageCollectImages();
		//dyld::log("dyld: NSCreateObjectFileImageFromMemory() error: %s\n", msg);
	}
	return NSObjectFileImageFailure;
}

static bool validOFI(NSObjectFileImage objectFileImage)
{
	const int ofiCount = sObjectFileImages.size();
	for (int i=0; i < ofiCount; ++i) {
		if ( sObjectFileImages[i] == objectFileImage )
			return true;
	}
	return false;
}

bool NSDestroyObjectFileImage(NSObjectFileImage objectFileImage)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p)\n", __func__, objectFileImage);
	
	if ( validOFI(objectFileImage) ) {
		// a failure during NSLinkModule will delete the image
		if ( objectFileImage->image != NULL ) {
			// if the image has never been linked or has been unlinked, the image is not in the list of valid images
			// and we should delete it
			bool linkedImage = dyld::validImage(objectFileImage->image);
			if ( ! linkedImage )  {
				ImageLoader::deleteImage(objectFileImage->image);
				objectFileImage->image = NULL;
			}
		}
		
		// remove from list of ofi's
		for (std::vector<NSObjectFileImage>::iterator it=sObjectFileImages.begin(); it != sObjectFileImages.end(); it++) {
			if ( *it == objectFileImage ) {
				sObjectFileImages.erase(it);
				break;
			}
		}
		
		// if object was created from a memory, release that memory
		// NOTE: this is the way dyld has always done this. NSCreateObjectFileImageFromMemory() hands over ownership of the memory to dyld
		if ( objectFileImage->imageBaseAddress != NULL ) {
			bool freed = false;
			if ( (dyld::gLibSystemHelpers != NULL) && (dyld::gLibSystemHelpers->version >= 6) ) {
				size_t sz = (*dyld::gLibSystemHelpers->malloc_size)(objectFileImage->imageBaseAddress);
				if ( sz != 0 ) {
					(*dyld::gLibSystemHelpers->free)((void*)(objectFileImage->imageBaseAddress));
					freed = true;
				}
			}
			if ( ! freed )
				vm_deallocate(mach_task_self(), (vm_address_t)objectFileImage->imageBaseAddress, objectFileImage->imageLength);
		}
		
		// free ofi object
		delete objectFileImage;
		
		return true;
	}
	return false;
}

bool NSHasModInitObjectFileImage(NSObjectFileImage objectFileImage)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p)\n", __func__, objectFileImage);
	return objectFileImage->image->needsInitialization();
}

uint32_t NSSymbolDefinitionCountInObjectFileImage(NSObjectFileImage objectFileImage)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p)\n", __func__, objectFileImage);
	return objectFileImage->image->getExportedSymbolCount();
}

const char* NSSymbolDefinitionNameInObjectFileImage(NSObjectFileImage objectFileImage, uint32_t ordinal)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p,%d)\n", __func__, objectFileImage, ordinal);
	const ImageLoader::Symbol* sym = objectFileImage->image->getIndexedExportedSymbol(ordinal);
	return objectFileImage->image->getExportedSymbolName(sym);	
}

uint32_t NSSymbolReferenceCountInObjectFileImage(NSObjectFileImage objectFileImage)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p)\n", __func__, objectFileImage);
	return objectFileImage->image->getImportedSymbolCount();
}

const char * NSSymbolReferenceNameInObjectFileImage(NSObjectFileImage objectFileImage, uint32_t ordinal, 
													bool* tentative_definition)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p,%d)\n", __func__, objectFileImage, ordinal);
	const ImageLoader::Symbol* sym = objectFileImage->image->getIndexedImportedSymbol(ordinal);
	if ( tentative_definition != NULL ) {
		ImageLoader::ReferenceFlags flags = objectFileImage->image->getImportedSymbolInfo(sym);
		if ( (flags & ImageLoader::kTentativeDefinition) != 0 )
			*tentative_definition = true;
		else
			*tentative_definition = false;
	}
	return objectFileImage->image->getImportedSymbolName(sym);	
}

void* NSGetSectionDataInObjectFileImage(NSObjectFileImage objectFileImage,
										const char* segmentName, const char* sectionName, unsigned long* size)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p,%s, %s)\n", __func__, objectFileImage, segmentName, sectionName);

	void* start;
	size_t length;
	if ( objectFileImage->image->getSectionContent(segmentName, sectionName, &start, &length) ) {
		if ( size != NULL )
			*size = length;
		return start;
	}
	return NULL;
}



bool NSIsSymbolDefinedInObjectFileImage(NSObjectFileImage objectFileImage, const char* symbolName)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p,%s)\n", __func__, objectFileImage, symbolName);
	const ImageLoader::Symbol* sym = objectFileImage->image->findExportedSymbol(symbolName, true, NULL);
	return ( sym != NULL );
}



NSModule NSLinkModule(NSObjectFileImage objectFileImage, const char* moduleName, uint32_t options)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p, \"%s\", 0x%08X)\n", __func__, objectFileImage, moduleName, options); 
	
	dyld::clearErrorMessage();
	try {
		if ( (options & NSLINKMODULE_OPTION_CAN_UNLOAD) != 0 )
			objectFileImage->image->setCanUnload();

		// NSLinkModule allows a bundle to be link multpile times
		// each link causes the bundle to be copied to a new address
		if ( objectFileImage->image->isLinked() ) {
			// already linked, so clone a new one and link it
			objectFileImage->image = dyld::cloneImage(objectFileImage->image);
		}
		
		// for memory based images, set moduleName as the name anyone calling _dyld_get_image_name() will see
		if ( objectFileImage->image->getPath() == NULL ) {
			objectFileImage->image->setPath(moduleName);
			// <rdar://problem/8812589> dyld has NULL paths in image info array
			dyld_image_info info;
			info.imageLoadAddress = objectFileImage->image->machHeader();
			info.imageFilePath = moduleName;
			info.imageFileModDate = 0;
			addImagesToAllImages(1, &info);
		}

		// support private bundles
		if ( (options & NSLINKMODULE_OPTION_PRIVATE) != 0 )
			objectFileImage->image->setHideExports();
	
		// set up linking options
		bool forceLazysBound = ( (options & NSLINKMODULE_OPTION_BINDNOW) != 0 );
		
		// load libraries, rebase, bind, to make this image usable
		dyld::link(objectFileImage->image, forceLazysBound, false, ImageLoader::RPathChain(NULL,NULL), UINT32_MAX);
		
		// bump reference count to keep this bundle from being garbage collected
		objectFileImage->image->incrementDlopenReferenceCount();

		// run initializers unless magic flag says not to
		if ( (options & NSLINKMODULE_OPTION_DONT_CALL_MOD_INIT_ROUTINES) == 0 )
			dyld::runInitializers(objectFileImage->image);

		return ImageLoaderToNSModule(objectFileImage->image);
	}
	catch (const char* msg) {
		dyld::garbageCollectImages();
		if ( (options & NSLINKMODULE_OPTION_RETURN_ON_ERROR) == 0 )
			dyldAPIhalt(__func__, msg);
		// not halting, so set error state for NSLinkEditError to find
		setLastError(NSLinkEditOtherError, 0, moduleName, msg);
		// dyld::link() deleted the image so lose our reference
		objectFileImage->image = NULL;
		free((void*)msg);
		return NULL;
	}
}


#if OLD_LIBSYSTEM_SUPPORT
// This is for compatibility with old libSystems (libdyld.a) which process ObjectFileImages outside dyld
static NSModule _dyld_link_module(NSObjectFileImage object_addr, size_t object_size, const char* moduleName, uint32_t options)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p, \"%s\", 0x%08X)\n", "NSLinkModule", object_addr,  moduleName, options); // note name/args translation
	ImageLoader* image = NULL;
	dyld::clearErrorMessage();
	try {
		const char* imageName = moduleName;			
		image = dyld::loadFromMemory((const uint8_t*)object_addr, object_size, imageName); 
		
		if ( image != NULL ) {		
			// support private bundles
			if ( (options & NSLINKMODULE_OPTION_PRIVATE) != 0 )
				image->setHideExports();
		
			// set up linking options
			bool forceLazysBound = ( (options & NSLINKMODULE_OPTION_BINDNOW) != 0 );
			
			// load libraries, rebase, bind, to make this image usable
			dyld::link(image, forceLazysBound, false, ImageLoader::RPathChain(NULL,NULL), UINT32_MAX);
			
			// run initializers unless magic flag says not to
			if ( (options & NSLINKMODULE_OPTION_DONT_CALL_MOD_INIT_ROUTINES) == 0 )
				dyld::runInitializers(image);
		}
	}
	catch (const char* msg) {
		if ( (options & NSLINKMODULE_OPTION_RETURN_ON_ERROR) == 0 )
			dyldAPIhalt("NSLinkModule", msg);
		// not halting, so set error state for NSLinkEditError to find
		setLastError(NSLinkEditOtherError, 0, moduleName, msg);
		// if image was created for this bundle, destroy it
		if ( image != NULL ) {
			dyld::removeImage(image);
			ImageLoader::deleteImage(image);
		}
		image = NULL;
		free((void*)msg);
	}
	return ImageLoaderToNSModule(image);
}
#endif

NSSymbol NSLookupSymbolInModule(NSModule module, const char* symbolName)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p, \"%s\")\n", __func__, (void *)module, symbolName);
	ImageLoader* image = NSModuleToImageLoader(module);
	if ( image == NULL ) 
		return NULL;
	return SymbolToNSSymbol(image->findExportedSymbol(symbolName, false, NULL));
}

const char* NSNameOfModule(NSModule module)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p)\n", __func__, module);
	ImageLoader* image = NSModuleToImageLoader(module);
	if ( image == NULL ) 
		return NULL;
	return image->getPath();
}

const char* NSLibraryNameForModule(NSModule module)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p)\n", __func__, module);
	ImageLoader* image = NSModuleToImageLoader(module);
	if ( image == NULL ) 
		return NULL;
	return image->getPath();
}

bool NSUnLinkModule(NSModule module, uint32_t options)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p, 0x%08X)\n", __func__, module, options); 
	if ( module == NULL )
		return false;
	ImageLoader* image = NSModuleToImageLoader(module);
	if ( image == NULL ) 
		return false;
	dyld::runImageStaticTerminators(image);
	if ( (dyld::gLibSystemHelpers != NULL) && (dyld::gLibSystemHelpers->version >= 13) ) {
		__cxa_range_t ranges[image->segmentCount()];
		int rangeCount = 0;
		for (unsigned int j=0; j < image->segmentCount(); ++j) {
			if ( !image->segExecutable(j) )
				continue;
			ranges[rangeCount].addr = (const void*)image->segActualLoadAddress(j);
			ranges[rangeCount].length = image->segSize(j);
			++rangeCount;
		}
		(*dyld::gLibSystemHelpers->cxa_finalize_ranges)(ranges, rangeCount);
	}
	dyld::removeImage(image);
	
	if ( (options & NSUNLINKMODULE_OPTION_KEEP_MEMORY_MAPPED) != 0 )
		image->setLeaveMapped();
		
	// TODO: NSUNLINKMODULE_OPTION_RESET_LAZY_REFERENCES

	// Only delete image if there is no ofi referencing it
	// That means the ofi was destroyed after linking, so no one is left to delete this image	
	const int ofiCount = sObjectFileImages.size();
	bool found = false;
	for (int i=0; i < ofiCount; ++i) {
		NSObjectFileImage ofi = sObjectFileImages[i];
		if ( ofi->image == image )
			found = true;
	}
	if ( !found )
		ImageLoader::deleteImage(image);
	
	return true;
}

// internal name and parameters do not match public name and parameters...
static void _dyld_install_handlers(void* undefined, void* multiple, void* linkEdit)
{
	if ( dyld::gLogAPIs )
		dyld::log("NSLinkEditErrorHandlers()\n");

	dyld::registerUndefinedHandler((dyld::UndefinedHandler)undefined);
	// no support for multiple or linkedit handlers
}




void NSLinkEditError(NSLinkEditErrors* c, int* errorNumber, const char** fileName, const char** errorString)
{
	// FIXME FIXME
	*c = sLastErrorFileCode;
	*errorNumber = sLastErrorNo;
	*fileName = sLastErrorFilePath;
	*errorString = dyld::getErrorMessage();
}



static void _dyld_register_binding_handler(void * (*bindingHandler)(const char *, const char *, void *), ImageLoader::BindingOptions bindingOptions)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s()\n", __func__);
	dyld::gLinkContext.bindingHandler = bindingHandler;
	dyld::gLinkContext.bindingOptions = bindingOptions;
}

#endif //DEPRECATED_APIS_SUPPORTED


// Call by fork() in libSystem after the kernel trap is done on the child side
void _dyld_fork_child()
{
	if ( dyld::gLogAPIs )
		dyld::log("%s()\n", __func__);
	// The implementation of fork() in libSystem knows to reset the variable mach_task_self_
	// in libSystem for the child of a fork.  But dyld is built with a static copy
	// of libc.a and has its own copy of mach_task_self_ which we reset here.
	//
	// In mach_init.h mach_task_self() is #defined to mach_task_self_ and
	// in mach_init() mach_task_self_ is initialized to task_self_trap().
	//
	extern mach_port_t	mach_task_self_;
	mach_task_self_ = task_self_trap();
 	
	// If dyld is sending load/unload notices to CoreSymbolication, the shared memory
	// page is not copied on fork. <rdar://problem/6797342>
 	// NULL the CoreSymbolication shared memory pointer to prevent a crash.
 	dyld::gProcessInfo->coreSymbolicationShmPage = NULL;
	// for safety, make sure child starts with clean systemOrderFlag
	dyld::gProcessInfo->systemOrderFlag = 0;
}


#if DEPRECATED_APIS_SUPPORTED
// returns true if prebinding was used in main executable
bool _dyld_launched_prebound()
{
	if ( dyld::gLogAPIs )
		dyld::log("%s()\n", __func__);
		
	// ¥¥¥Êif we deprecate prebinding, we may want to consider always returning true or false here
	return dyld::mainExecutablePrebound();
}


//
// _dyld_NSMakePrivateModulePublic() is the dyld side of the hack
// NSMakePrivateModulePublic() needed for the dlopen() to turn it's
// RTLD_LOCAL handles into RTLD_GLOBAL.  It just simply turns off the private
// flag on the image for this module.  If the module was found and it was
// private then everything worked and TRUE is returned else FALSE is returned.
//
static bool NSMakePrivateModulePublic(NSModule module)
{
	ImageLoader* image = NSModuleToImageLoader(module);
	if ( image != NULL ) {
		if ( image->hasHiddenExports() ) {
			image->setHideExports(false);
			return true;
		}
	}
	return false;
}

#endif // DEPRECATED_APIS_SUPPORTED

int _dyld_func_lookup(const char* name, void** address)
{
	for (const dyld_func* p = dyld_funcs; p->name != NULL; ++p) {
	    if ( strcmp(p->name, name) == 0 ) {
			if( p->implementation == unimplemented )
				dyld::log("unimplemented dyld function: %s\n", p->name);
			*address = p->implementation;
			return true;
	    }
	}
	*address = 0;
	return false;
}


static void registerThreadHelpers(const dyld::LibSystemHelpers* helpers)
{
	dyld::gLibSystemHelpers = helpers;
	
#if !SUPPORT_ZERO_COST_EXCEPTIONS
	if ( helpers->version >= 5 )  {
		// create key use by dyld exception handling
		pthread_key_t key;
		int result = helpers->pthread_key_create(&key, NULL);
		if ( result == 0 )
			__Unwind_SjLj_SetThreadKey(key);
	}
#endif
}


static void dlerrorClear()
{
	if ( dyld::gLibSystemHelpers != NULL ) {
		// <rdar://problem/10595338> dlerror buffer leak
		// dlerrorClear() should not force allocation, but zero it if already allocated
		if ( dyld::gLibSystemHelpers->version >= 10 ) {
			if ( ! (*dyld::gLibSystemHelpers->hasPerThreadBufferFor_dlerror)() )
				return;
		}

		// first char of buffer is flag whether string (starting at second char) is valid
		char* buffer = (*dyld::gLibSystemHelpers->getThreadBufferFor_dlerror)(2);
		buffer[0] = '\0';
		buffer[1] = '\0';
	}
}

static void dlerrorSet(const char* msg)
{
	if ( dyld::gLibSystemHelpers != NULL ) {
		// first char of buffer is flag whether string (starting at second char) is valid
		char* buffer = (*dyld::gLibSystemHelpers->getThreadBufferFor_dlerror)(strlen(msg)+2);
		buffer[0] = '\1';
		strcpy(&buffer[1], msg);
	}
}


bool dlopen_preflight(const char* path)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%s)\n", __func__, path);

	dlerrorClear();
	
	CRSetCrashLogMessage("dyld: in dlopen_preflight()");

	const bool leafName = (strchr(path, '/') == NULL);
	const bool absolutePath = (path[0] == '/');
#if __IPHONE_OS_VERSION_MIN_REQUIRED
	char canonicalPath[PATH_MAX]; 
	// <rdar://problem/7017050> dlopen() not opening frameworks from shared cache with // or ./ in path
	if ( !leafName ) {
		// make path canonical if it contains a // or ./
		if ( (strstr(path, "//") != NULL) || (strstr(path, "./") != NULL) ) {
			const char* lastSlash = strrchr(path, '/');
			char dirPath[PATH_MAX]; 
			if ( strlcpy(dirPath, path, sizeof(dirPath)) < sizeof(dirPath) ) {
				dirPath[lastSlash-path] = '\0';
				if ( realpath(dirPath, canonicalPath) ) {
					strlcat(canonicalPath, "/", sizeof(canonicalPath));
					if ( strlcat(canonicalPath, lastSlash+1, sizeof(canonicalPath)) < sizeof(canonicalPath) ) {
						// if all fit in buffer, use new canonical path
						path = canonicalPath;
					}
				}
			}
		}
	}
#endif
#if SUPPORT_ACCELERATE_TABLES
	if  ( dyld::isPathInCache(path) )
		return true;
#endif

#if DYLD_SHARED_CACHE_SUPPORT
	// <rdar://problem/5910137> dlopen_preflight() on image in shared cache leaves it loaded but not objc initialized
	// if requested path is to something in the dyld shared cache, always succeed
	if ( dyld::inSharedCache(path) )
		return true;
#endif
	
	bool result = false;
	std::vector<const char*> rpathsFromCallerImage;
	try {
		void* callerAddress = __builtin_return_address(1); // note layers: 1: real client, 0: libSystem glue
		ImageLoader* callerImage = dyld::findImageContainingAddress(callerAddress);
		// for dlopen, use rpath from caller image and from main executable
		if ( callerImage != NULL )
			callerImage->getRPaths(dyld::gLinkContext, rpathsFromCallerImage);
		ImageLoader::RPathChain callersRPaths(NULL, &rpathsFromCallerImage);
		if ( callerImage != dyld::mainExecutable() ) {
			dyld::mainExecutable()->getRPaths(dyld::gLinkContext, rpathsFromCallerImage);
		}

		ImageLoader*	image = NULL;
		dyld::LoadContext context;
		context.useSearchPaths	= true;
		context.useFallbackPaths= leafName;					// a partial path implies don't use fallback paths
		context.useLdLibraryPath= leafName;					// a leafname implies should search 
		context.implicitRPath	= !absolutePath;			// a non-absolute path implies try rpath searching 
		context.matchByInstallName = true;
		context.dontLoad		= false;
		context.mustBeBundle	= false;
		context.mustBeDylib		= false;
		context.canBePIE		= true;
		context.origin			= callerImage != NULL ? callerImage->getPath() : NULL; // caller's image's path
		context.rpath			= &callersRPaths;	// rpaths from caller and main executable

		unsigned cacheIndex;
		image = load(path, context, cacheIndex);
		if ( image != NULL ) {
			dyld::preflight(image, callersRPaths, cacheIndex);	// image object deleted by dyld::preflight()
			result = true;
		}
	}
	catch (const char* msg) {
		const char* str = dyld::mkstringf("dlopen_preflight(%s): %s", path, msg);
		dlerrorSet(str);
		free((void*)str);
		free((void*)msg); 	// our free() will do nothing if msg is a string literal
	}
	// free rpaths (getRPaths() malloc'ed each string)
	for(std::vector<const char*>::iterator it=rpathsFromCallerImage.begin(); it != rpathsFromCallerImage.end(); ++it) {
		const char* str = *it;
		free((void*)str);
	}
	CRSetCrashLogMessage(NULL);
	return result;
}

#if SUPPORT_ACCELERATE_TABLES
bool static callerIsNonOSApp(void* callerAddress, const char** shortName)
{
	*shortName = NULL;
	const mach_header* unusedMh;
	const char* unusedPath;
	unsigned unusedIndex;
	// any address in shared cache is not from app
	if ( dyld::addressInCache(callerAddress, &unusedMh, &unusedPath, &unusedIndex) )
		return false;

	ImageLoader* callerImage = dyld::findImageContainingAddress(callerAddress);
	if ( callerImage == NULL )
		return false;

	*shortName = callerImage->getShortName();
	return ( strncmp(callerImage->getPath(), "/var/containers/", 16) == 0 );
}
#endif

void* dlopen(const char* path, int mode)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%s, 0x%08X)\n", __func__, ((path==NULL) ? "NULL" : path), mode);

#if SUPPORT_ACCELERATE_TABLES
	if ( dyld::gLogAppAPIs ) {
		void* callerAddress = __builtin_return_address(1); // note layers: 1: real client, 0: libSystem glue
		const char* shortName;
		if ( callerIsNonOSApp(callerAddress, &shortName) ) {
			dyld::log("%s: %s(%s, 0x%08X)\n", shortName, __func__, ((path==NULL) ? "NULL" : path), mode);
		}
	}
#endif

	dlerrorClear();

	// passing NULL for path means return magic object
	if ( path == NULL ) {
		// RTLD_FIRST means any dlsym() calls on the handle should only search that handle and not subsequent images
		if ( (mode & RTLD_FIRST) != 0 )
			return RTLD_MAIN_ONLY;
		else
			return RTLD_DEFAULT;
	}
	
	// acquire global dyld lock (dlopen is special - libSystem glue does not do locking)
	bool lockHeld = false;
	if ( (dyld::gLibSystemHelpers != NULL) && (dyld::gLibSystemHelpers->version >= 4) ) {
		dyld::gLibSystemHelpers->acquireGlobalDyldLock();
		CRSetCrashLogMessage("dyld: in dlopen()");
		lockHeld = true;
	}
		
	void* result = NULL;
	const bool leafName = (strchr(path, '/') == NULL);
	const bool absolutePath = (path[0] == '/');
#if __IPHONE_OS_VERSION_MIN_REQUIRED
	char canonicalPath[PATH_MAX]; 
	// <rdar://problem/7017050> dlopen() not opening frameworks from shared cache with // or ./ in path
	if ( !leafName ) {
		// make path canonical if it contains a // or ./
		if ( (strstr(path, "//") != NULL) || (strstr(path, "./") != NULL) ) {
			const char* lastSlash = strrchr(path, '/');
			char dirPath[PATH_MAX]; 
			if ( strlcpy(dirPath, path, sizeof(dirPath)) < sizeof(dirPath) ) {
				dirPath[lastSlash-path] = '\0';
				if ( realpath(dirPath, canonicalPath) ) {
					strlcat(canonicalPath, "/", sizeof(canonicalPath));
					if ( strlcat(canonicalPath, lastSlash+1, sizeof(canonicalPath)) < sizeof(canonicalPath) ) {
						// if all fit in buffer, use new canonical path
						path = canonicalPath;
					}
				}
			}
		}
	}
#endif
#if SUPPORT_ACCELERATE_TABLES
	if ( dyld::dlopenFromCache(path, mode, &result) ) {
		// Note: dlopenFromCache() releases the lock
		if ( dyld::gLogAPIs )
			dyld::log("  %s(%s) ==> %p\n", __func__, path, result);
		return result;
	}
#endif

	ImageLoader* image = NULL;
	std::vector<const char*> rpathsFromCallerImage;
	ImageLoader::RPathChain callersRPaths(NULL, &rpathsFromCallerImage);
	try {
		void* callerAddress = __builtin_return_address(1); // note layers: 1: real client, 0: libSystem glue
		ImageLoader* callerImage = dyld::findImageContainingAddress(callerAddress);
		if ( (mode & RTLD_NOLOAD) == 0 ) {
			// for dlopen, use rpath from caller image and from main executable
			if ( callerImage != NULL )
				callerImage->getRPaths(dyld::gLinkContext, rpathsFromCallerImage);
			if ( callerImage != dyld::mainExecutable() )
				dyld::mainExecutable()->getRPaths(dyld::gLinkContext, rpathsFromCallerImage);
		}
 
		dyld::LoadContext context;
		context.useSearchPaths	= true;
		context.useFallbackPaths= leafName;				// a partial path means no fallback paths
		context.useLdLibraryPath= leafName;				// a leafname implies should search 
		context.implicitRPath   = !absolutePath;		// a non-absolute path implies try rpath searching 
		context.matchByInstallName = true;
		context.dontLoad		= ( (mode & RTLD_NOLOAD) != 0 );
		context.mustBeBundle	= false;
		context.mustBeDylib		= false;
		context.canBePIE		= true;
		context.origin			= callerImage != NULL ? callerImage->getPath() : NULL; // caller's image's path
		context.rpath			= &callersRPaths;				// rpaths from caller and main executable

		unsigned cacheIndex;
		image = load(path, context, cacheIndex);
#if SUPPORT_ACCELERATE_TABLES
		if ( (image != NULL) && (cacheIndex != UINT32_MAX) ) {
			if ( dyld::makeCacheHandle(image, cacheIndex, mode, &result) ) {
				if ( dyld::gLogAPIs )
					dyld::log("  %s(%s) ==> %p\n", __func__, path, result);
				if ( lockHeld )
					dyld::gLibSystemHelpers->releaseGlobalDyldLock();
				return result;
			}
		}
#endif
		if ( image != NULL ) {
			// bump reference count.  Do this before link() so that if an initializer calls dlopen and fails
			// this image is not garbage collected
			image->incrementDlopenReferenceCount();
			// link in all dependents
			if ( (mode & RTLD_NOLOAD) == 0 ) {
				bool alreadyLinked = image->isLinked();
				bool forceLazysBound = ( (mode & RTLD_NOW) != 0 );
				dyld::link(image, forceLazysBound, false, callersRPaths, cacheIndex);
				if ( ! alreadyLinked ) {
					// only hide exports if image is not already in use
					if ( (mode & RTLD_LOCAL) != 0 )
						image->setHideExports(true);
				}
			}
			
			// RTLD_NODELETE means don't unmap image even after dlclosed. This is what dlcompat did on Mac OS X 10.3
			// On other *nix OS's, it means dlclose() should do nothing, but the handle should be invalidated. 
			// The subtle differences are: 
			//  1) if the image has any termination routines, whether they are run during dlclose or when the process terminates
			//  2) If someone does a supsequent dlopen() on the same image, whether the same address should be used. 
			if ( (mode & RTLD_NODELETE) != 0 )
				image->setLeaveMapped();
			
			// release global dyld lock early, this enables initializers to do threaded operations
			if ( lockHeld ) {
				CRSetCrashLogMessage(NULL);
				dyld::gLibSystemHelpers->releaseGlobalDyldLock();
				lockHeld = false;
			}
			
			// RTLD_NOLOAD means dlopen should fail unless path is already loaded. 
			// don't run initializers when RTLD_NOLOAD is set.  This only matters if dlopen() is
			// called from within an initializer because it can cause initializers to run
			// out of order. Most uses of RTLD_NOLOAD are "probes".  If they want initialzers
			// to run, then don't use RTLD_NOLOAD.
			if ( (mode & RTLD_NOLOAD) == 0 ) {
				// run initializers
				dyld::runInitializers(image);
			}
			
			// RTLD_FIRST means any dlsym() calls on the handle should only search that handle and not subsequent images
			// this is tracked by setting the low bit of the handle, which is usually zero by malloc alignment
			if ( (mode & RTLD_FIRST) != 0 )
				result = (void*)(((uintptr_t)image)|1);
			else
				result = image;
		}
	}
	catch (const char* msg) {
		if ( image != NULL ) {
			// load() succeeded but, link() failed
			// back down reference count and do GC
			image->decrementDlopenReferenceCount();
			if ( image->dlopenCount() == 0 )
				dyld::garbageCollectImages();
		}
		const char* str = dyld::mkstringf("dlopen(%s, %d): %s", path, mode, msg);
		if ( dyld::gLogAPIs )
			dyld::log("  %s() failed, error: '%s'\n", __func__, str);
		dlerrorSet(str);
		free((void*)str);
		free((void*)msg); 	// our free() will do nothing if msg is a string literal
		result = NULL;
	}
	// free rpaths (getRPaths() malloc'ed each string)
	for(std::vector<const char*>::iterator it=rpathsFromCallerImage.begin(); it != rpathsFromCallerImage.end(); ++it) {
		const char* str = *it;
		free((void*)str);
	}
	
	// when context.dontLoad is set, load() returns NULL instead of throwing an exception 
	if ( (mode & RTLD_NOLOAD) && (result == NULL) ) {
		dlerrorSet("image not already loaded");
	}
	
	if ( lockHeld ) {
		CRSetCrashLogMessage(NULL);
		dyld::gLibSystemHelpers->releaseGlobalDyldLock();
	}
	if ( dyld::gLogAPIs && (result != NULL) )
		dyld::log("  %s(%s) ==> %p\n", __func__, path, result);
	return result;
}



int dlclose(void* handle)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p)\n", __func__, handle);

	// silently accept magic handles for main executable
	if ( handle == RTLD_MAIN_ONLY )
		return 0;
	if ( handle == RTLD_DEFAULT )
		return 0;
	
	ImageLoader* image = (ImageLoader*)(((uintptr_t)handle) & (-4));	// clear mode bits
	if ( dyld::validImage(image) ) {
		dlerrorClear();
		// decrement use count
		if ( image->decrementDlopenReferenceCount() ) {
			dlerrorSet("dlclose() called too many times");
			return -1;
		}
		// remove image if reference count went to zero
		if ( image->dlopenCount() == 0 )
			dyld::garbageCollectImages();
		return 0;
	}
	else {
		dlerrorSet("invalid handle passed to dlclose()");
		return -1;
	}
}



int dladdr(const void* address, Dl_info* info)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p, %p)\n", __func__, address, info);

	CRSetCrashLogMessage("dyld: in dladdr()");
#if SUPPORT_ACCELERATE_TABLES
	if ( dyld::dladdrFromCache(address, info) ) {
		CRSetCrashLogMessage(NULL);
		return 1; // success
	}
#endif

	ImageLoader* image = dyld::findImageContainingAddress(address);
	if ( image != NULL ) {
		info->dli_fname = image->getRealPath();
		info->dli_fbase = (void*)image->machHeader();
		if ( address == info->dli_fbase ) {
			// special case lookup of header
			info->dli_sname = "__dso_handle";
			info->dli_saddr = info->dli_fbase;
			CRSetCrashLogMessage(NULL);
			return 1; // success
		}
		// find closest symbol in the image
		info->dli_sname = image->findClosestSymbol(address, (const void**)&info->dli_saddr);
		// never return the mach_header symbol
		if ( info->dli_saddr == info->dli_fbase ) {
			info->dli_sname = NULL;
			info->dli_saddr = NULL;
			CRSetCrashLogMessage(NULL);
			return 1; // success
		}
		if ( info->dli_sname != NULL ) {
			if ( info->dli_sname[0] == '_' )
				info->dli_sname = info->dli_sname +1; // strip off leading underscore
			//dyld::log("dladdr(%p) => %p %s\n", address, info->dli_saddr, info->dli_sname);
			CRSetCrashLogMessage(NULL);
			return 1; // success
		}
		info->dli_sname = NULL;
		info->dli_saddr = NULL;
		CRSetCrashLogMessage(NULL);
		return 1; // success
	}
	CRSetCrashLogMessage(NULL);
	return 0;  // failure
}


char* dlerror()
{
	if ( dyld::gLogAPIs )
		dyld::log("%s()\n", __func__);

	if ( dyld::gLibSystemHelpers != NULL ) {
		// if using newer libdyld.dylib and buffer if buffer not yet allocated, return NULL
		if ( dyld::gLibSystemHelpers->version >= 10 ) {
			if ( ! (*dyld::gLibSystemHelpers->hasPerThreadBufferFor_dlerror)() )
				return NULL;
		}

		// first char of buffer is flag whether string (starting at second char) is valid
		char* buffer = (*dyld::gLibSystemHelpers->getThreadBufferFor_dlerror)(2);
		if ( buffer[0] != '\0' ) {	// if valid buffer
			buffer[0] = '\0';		// mark invalid, so next call to dlerror returns NULL
			return &buffer[1];		// return message
		}
	}
	return NULL;
}

void* dlsym(void* handle, const char* symbolName)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p, %s)\n", __func__, handle, symbolName);

#if SUPPORT_ACCELERATE_TABLES
	if ( dyld::gLogAppAPIs ) {
		void* callerAddress = __builtin_return_address(1); // note layers: 1: real client, 0: libSystem glue
		const char* shortName;
		if ( callerIsNonOSApp(callerAddress, &shortName) ) {
			dyld::log("%s: %s(%p, %s)\n", shortName, __func__, handle, symbolName);
		}
	}
#endif

	CRSetCrashLogMessage("dyld: in dlsym()");
	dlerrorClear();

	const ImageLoader* image;
	const ImageLoader::Symbol* sym;
	void* result;

	// dlsym() assumes symbolName passed in is same as in C source code
	// dyld assumes all symbol names have an underscore prefix
	char underscoredName[strlen(symbolName)+2];
	underscoredName[0] = '_';
	strcpy(&underscoredName[1], symbolName);
	
	// magic "search all" handle
	if ( handle == RTLD_DEFAULT ) {
		if ( dyld::flatFindExportedSymbol(underscoredName, &sym, &image) ) {
			CRSetCrashLogMessage(NULL);
			result = (void*)image->getExportedSymbolAddress(sym, dyld::gLinkContext, NULL, false, underscoredName);
			if ( dyld::gLogAPIs )
				dyld::log("  %s(RTLD_DEFAULT, %s) ==> %p\n", __func__, symbolName, result);
			return result;
		}
		const char* str = dyld::mkstringf("dlsym(RTLD_DEFAULT, %s): symbol not found", symbolName);
		dlerrorSet(str);
		free((void*)str);
		CRSetCrashLogMessage(NULL);
		if ( dyld::gLogAPIs )
			dyld::log("  %s(RTLD_DEFAULT, %s) ==> NULL\n", __func__, symbolName);
		return NULL;
	}
	
	// magic "search only main executable" handle
	else if ( handle == RTLD_MAIN_ONLY ) {
		image = dyld::mainExecutable();
		sym = image->findExportedSymbol(underscoredName, true, &image); // search RTLD_FIRST way
		if ( sym != NULL ) {
			CRSetCrashLogMessage(NULL);
			result = (void*)image->getExportedSymbolAddress(sym, dyld::gLinkContext, NULL, false, underscoredName);
			if ( dyld::gLogAPIs )
				dyld::log("  %s(RTLD_MAIN_ONLY, %s) ==> %p\n", __func__, symbolName, result);
			return result;
		}
		const char* str = dyld::mkstringf("dlsym(RTLD_MAIN_ONLY, %s): symbol not found", symbolName);
		dlerrorSet(str);
		free((void*)str);
		CRSetCrashLogMessage(NULL);
		if ( dyld::gLogAPIs )
			dyld::log("  %s(RTLD_MAIN_ONLY, %s) ==> NULL\n", __func__, symbolName);
		return NULL;
	}
	
	// magic "search what I would see" handle
	else if ( handle == RTLD_NEXT ) {
		void* callerAddress = __builtin_return_address(1); // note layers: 1: real client, 0: libSystem glue
#if SUPPORT_ACCELERATE_TABLES
		const mach_header* mh;
		const char* path;
		unsigned index;
		if ( dyld::addressInCache(callerAddress, &mh, &path, &index) ) {
			// if dylib in cache is calling dlsym(RTLD_NEXT,xxx) handle search differently
			result = dyld::dlsymFromCache(RTLD_NEXT, underscoredName, index);
			if ( dyld::gLogAPIs )
				dyld::log("  %s(RTLD_NEXT, %s) ==> %p\n", __func__, symbolName, result);
			return result;
		}
#endif
		ImageLoader* callerImage = dyld::findImageContainingAddress(callerAddress);
		sym = callerImage->findExportedSymbolInDependentImages(underscoredName, dyld::gLinkContext, &image); // don't search image, but do search what it links against
		if ( sym != NULL ) {
			CRSetCrashLogMessage(NULL);
			result = (void*)image->getExportedSymbolAddress(sym, dyld::gLinkContext , callerImage, false, underscoredName);
			if ( dyld::gLogAPIs )
				dyld::log("  %s(RTLD_NEXT, %s) ==> %p\n", __func__, symbolName, result);
			return result;
		}
		const char* str = dyld::mkstringf("dlsym(RTLD_NEXT, %s): symbol not found", symbolName);
		dlerrorSet(str);
		free((void*)str);
		CRSetCrashLogMessage(NULL);
		if ( dyld::gLogAPIs )
			dyld::log("  %s(RTLD_NEXT, %s) ==> NULL\n", __func__, symbolName);
		return NULL;
	}
	// magic "search me, then what I would see" handle
	else if ( handle == RTLD_SELF ) {
		void* callerAddress = __builtin_return_address(1); // note layers: 1: real client, 0: libSystem glue
#if SUPPORT_ACCELERATE_TABLES
		const mach_header* mh;
		const char* path;
		unsigned index;
		if ( dyld::addressInCache(callerAddress, &mh, &path, &index) ) {
			// if dylib in cache is calling dlsym(RTLD_SELF,xxx) handle search differently
			result = dyld::dlsymFromCache(RTLD_SELF, underscoredName, index);
			if ( dyld::gLogAPIs )
				dyld::log("  %s(RTLD_SELF, %s) ==> %p\n", __func__, symbolName, result);
			return result;
		}
#endif
		ImageLoader* callerImage = dyld::findImageContainingAddress(callerAddress);
		sym = callerImage->findExportedSymbolInImageOrDependentImages(underscoredName, dyld::gLinkContext, &image); // search image and what it links against
		if ( sym != NULL ) {
			CRSetCrashLogMessage(NULL);
			result = (void*)image->getExportedSymbolAddress(sym, dyld::gLinkContext, callerImage, false, underscoredName);
			if ( dyld::gLogAPIs )
				dyld::log("  %s(RTLD_SELF, %s) ==> %p\n", __func__, symbolName, result);
			return result;
		}
		const char* str = dyld::mkstringf("dlsym(RTLD_SELF, %s): symbol not found", symbolName);
		dlerrorSet(str);
		free((void*)str);
		CRSetCrashLogMessage(NULL);
		if ( dyld::gLogAPIs )
			dyld::log("  %s(RTLD_SELF, %s) ==> NULL\n", __func__, symbolName);
		return NULL;
	}
#if SUPPORT_ACCELERATE_TABLES
	// check for mega dylib handle
	else if ( dyld::isCacheHandle(handle) ) {
		result = dyld::dlsymFromCache(handle, underscoredName, 0);
		if ( dyld::gLogAPIs )
			dyld::log("  %s(%p, %s) ==> %p\n", __func__, handle, symbolName, result);
		return result;
	}
#endif
	// real handle
	image = (ImageLoader*)(((uintptr_t)handle) & (-4));	// clear mode bits
	if ( dyld::validImage(image) ) {
		if ( (((uintptr_t)handle) & 1) != 0 )
			sym = image->findExportedSymbol(underscoredName, true, &image); // search RTLD_FIRST way
		else
			sym = image->findExportedSymbolInImageOrDependentImages(underscoredName, dyld::gLinkContext, &image); // search image and what it links against
		
		if ( sym != NULL ) {
			CRSetCrashLogMessage(NULL);
			ImageLoader* callerImage = NULL;
			if ( sDynamicInterposing ) {
				// only take time to look up caller, if dynamic interposing in use
				void* callerAddress = __builtin_return_address(1); // note layers: 1: real client, 0: libSystem glue
				callerImage = dyld::findImageContainingAddress(callerAddress);
			}
			result = (void*)image->getExportedSymbolAddress(sym, dyld::gLinkContext, callerImage, false, underscoredName);
			if ( dyld::gLogAPIs )
				dyld::log("  %s(%p, %s) ==> %p\n", __func__, handle, symbolName, result);
			return result;
		}
		const char* str = dyld::mkstringf("dlsym(%p, %s): symbol not found", handle, symbolName);
		dlerrorSet(str);
		free((void*)str);
	}
	else {
		dlerrorSet("invalid handle passed to dlsym()");
	}
	CRSetCrashLogMessage(NULL);
	if ( dyld::gLogAPIs )
		dyld::log("  %s(%p, %s) ==> NULL\n", __func__, handle, symbolName);
	return NULL;
}










const struct dyld_all_image_infos* _dyld_get_all_image_infos()
{
	return dyld::gProcessInfo;
}


#if SUPPORT_ZERO_COST_EXCEPTIONS
static bool client_dyld_find_unwind_sections(void* addr, dyld_unwind_sections* info)
{
	//if ( dyld::gLogAPIs )
	//	dyld::log("%s(%p, %p)\n", __func__, addr, info);
	
#if SUPPORT_ACCELERATE_TABLES
	if ( dyld::findUnwindSections(addr, info) )
		return true;
#endif
	ImageLoader* image = dyld::findImageContainingAddress(addr);
	if ( image != NULL ) {
		image->getUnwindInfo(info);
		return true;
	}
	return false;
}
#endif


const char* dyld_image_path_containing_address(const void* address)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p)\n", __func__, address);

#if SUPPORT_ACCELERATE_TABLES
	const mach_header* mh;
	const char* path;
	if ( dyld::addressInCache(address, &mh, &path) )
		return path;
#endif

	ImageLoader* image = dyld::findImageContainingAddress(address);
	if ( image != NULL )
		return image->getRealPath();
	return NULL;
}



bool dyld_shared_cache_some_image_overridden()
{
 #if DYLD_SHARED_CACHE_SUPPORT
	return dyld::gSharedCacheOverridden;
 #else
    return true;
 #endif
}


void dyld_dynamic_interpose(const struct mach_header* mh, const struct dyld_interpose_tuple array[], size_t count)
{
	if ( mh == NULL )
		return;
	if ( array == NULL )
		return;
	if ( count == 0 )
		return;
	ImageLoader* image = dyld::findImageByMachHeader(mh);
	if ( image == NULL )
		return;
	
	// make pass at bound references in this image and update them
	dyld::gLinkContext.dynamicInterposeArray = array;
	dyld::gLinkContext.dynamicInterposeCount = count;
		image->dynamicInterpose(dyld::gLinkContext);
	dyld::gLinkContext.dynamicInterposeArray = NULL;
	dyld::gLinkContext.dynamicInterposeCount = 0;
	
	// leave interposing info so any future (lazy) binding will get it too
	image->addDynamicInterposingTuples(array, count);
	
	sDynamicInterposing = true;
}


bool _dyld_is_memory_immutable(const void* addr, size_t length)
{
	if ( dyld::gLogAPIs )
		dyld::log("%s(%p, %ld)\n", __func__, addr, length);

	uintptr_t checkStart = (uintptr_t)addr;
	uintptr_t checkEnd   = checkStart + length;

#if DYLD_SHARED_CACHE_SUPPORT
	// quick check to see if in r/o region of shared cache.  If so return true.
	if ( dyld_shared_cache_ranges.sharedRegionsCount > 2 ) {
		uintptr_t roStart    = dyld_shared_cache_ranges.ranges[0].start;
		uintptr_t roEnd      = roStart + dyld_shared_cache_ranges.ranges[0].length;
		if ( (roStart < checkStart) && (checkEnd < roEnd) )
			return true;
	}
#endif

	// Otherwise find if addr is in a dyld loaded image
	ImageLoader* image = dyld::findImageContainingAddress(addr);
	if ( image != NULL ) {
		// <rdar://problem/24091154> already checked for r/o portion of cache
		if ( image->inSharedCache() )
			return false;
		if ( !image->neverUnload() )
			return false;
		for (unsigned i=0, e=image->segmentCount(); i < e; ++i) {
			if ( (image->segActualLoadAddress(i) < checkStart) && (checkEnd < image->segActualEndAddress(i)) ) {
				return !image->segWriteable(i);
			}
		}
	}
	return false;
}



void _dyld_objc_notify_register(_dyld_objc_notify_mapped    mapped,
                                _dyld_objc_notify_init      init,
                                _dyld_objc_notify_unmapped  unmapped)
{
	dyld::registerObjCNotifiers(mapped, init, unmapped);
}


bool _dyld_get_shared_cache_uuid(uuid_t uuid)
{
	return dyld::sharedCacheUUID(uuid);
}
