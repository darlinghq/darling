/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2004-2013 Apple Inc. All rights reserved.
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

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <libproc.h>
#include <sys/param.h>
#include <mach/mach_time.h> // mach_absolute_time()
#include <mach/mach_init.h> 
#include <sys/types.h>
#include <sys/stat.h> 
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/syslog.h>
#include <sys/uio.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h> 
#include <mach-o/ldsyms.h> 
#include <libkern/OSByteOrder.h> 
#include <libkern/OSAtomic.h>
#include <mach/mach.h>
#include <sys/sysctl.h>
#include <sys/mman.h>
#include <sys/dtrace.h>
#include <libkern/OSAtomic.h>
#include <Availability.h>
#include <System/sys/codesign.h>
#include <System/sys/csr.h>
#include <_simple.h>
//#include <os/lock_private.h>
#include <System/machine/cpu_capabilities.h>
#include <signal.h>
//#include <System/sys/reason.h>
//#include <kern/kcdata.h>
#include <sandbox.h>
#include <sandbox/private.h>
#include "../dyld_kernel.h"

#include <array>

#ifndef CPU_SUBTYPE_ARM_V5TEJ
	#define CPU_SUBTYPE_ARM_V5TEJ		((cpu_subtype_t) 7)
#endif
#ifndef CPU_SUBTYPE_ARM_XSCALE
	#define CPU_SUBTYPE_ARM_XSCALE		((cpu_subtype_t) 8)
#endif
#ifndef CPU_SUBTYPE_ARM_V7
	#define CPU_SUBTYPE_ARM_V7			((cpu_subtype_t) 9)
#endif
#ifndef CPU_SUBTYPE_ARM_V7F
	#define CPU_SUBTYPE_ARM_V7F			((cpu_subtype_t) 10)
#endif
#ifndef CPU_SUBTYPE_ARM_V7S
	#define CPU_SUBTYPE_ARM_V7S			((cpu_subtype_t) 11)
#endif
#ifndef CPU_SUBTYPE_ARM_V7K
	#define CPU_SUBTYPE_ARM_V7K			((cpu_subtype_t) 12)
#endif
#ifndef LC_DYLD_ENVIRONMENT
	#define LC_DYLD_ENVIRONMENT			0x27
#endif

#ifndef CPU_SUBTYPE_X86_64_H
	#define CPU_SUBTYPE_X86_64_H		((cpu_subtype_t) 8) 
#endif	

#ifndef VM_PROT_SLIDE   
    #define VM_PROT_SLIDE 0x20
#endif

#include <vector>
#include <algorithm>

#include "mach-o/dyld_gdb.h"

#include "dyld.h"
#include "ImageLoader.h"
#include "ImageLoaderMachO.h"
#include "dyldLibSystemInterface.h"
#if DYLD_SHARED_CACHE_SUPPORT
#include "dyld_cache_format.h"
#endif
#include "dyld_process_info_internal.h"
//#include <coreSymbolicationDyldSupport.h>
#if TARGET_IPHONE_SIMULATOR
	extern "C" void xcoresymbolication_load_notifier(void *connection, uint64_t load_timestamp, const char *image_path, const struct mach_header *mach_header);
	extern "C" void xcoresymbolication_unload_notifier(void *connection, uint64_t unload_timestamp, const char *image_path, const struct mach_header *mach_header);
	#define coresymbolication_load_notifier(c, t, p, h) xcoresymbolication_load_notifier(c, t, p, h)
	#define coresymbolication_unload_notifier(c, t, p, h) xcoresymbolication_unload_notifier(c, t, p, h)
#endif

#if SUPPORT_ACCELERATE_TABLES
	#include "ImageLoaderMegaDylib.h"
#endif

#if TARGET_IPHONE_SIMULATOR
	extern "C" void* gSyscallHelpers;
#else
	#include "dyldSyscallInterface.h"
#endif


// not libc header for send() syscall interface
extern "C" ssize_t __sendto(int, const void *, size_t, int, const struct sockaddr *, socklen_t);


// ARM and x86_64 are the only architecture that use cpu-sub-types
#define CPU_SUBTYPES_SUPPORTED  ((__arm__ || __x86_64__) && !TARGET_IPHONE_SIMULATOR)

#if __LP64__
	#define LC_SEGMENT_COMMAND		LC_SEGMENT_64
	#define LC_SEGMENT_COMMAND_WRONG LC_SEGMENT
	#define LC_ENCRYPT_COMMAND		LC_ENCRYPTION_INFO
	#define macho_segment_command	segment_command_64
	#define macho_section			section_64
#else
	#define LC_SEGMENT_COMMAND		LC_SEGMENT
	#define LC_SEGMENT_COMMAND_WRONG LC_SEGMENT_64
	#define LC_ENCRYPT_COMMAND		LC_ENCRYPTION_INFO_64
	#define macho_segment_command	segment_command
	#define macho_section			section
#endif



#define CPU_TYPE_MASK 0x00FFFFFF	/* complement of CPU_ARCH_MASK */


/* implemented in dyld_gdb.cpp */
extern void resetAllImages();
extern void addImagesToAllImages(uint32_t infoCount, const dyld_image_info info[]);
extern void removeImageFromAllImages(const mach_header* mh);
extern void addNonSharedCacheImageUUID(const dyld_uuid_info& info);
extern const char* notifyGDB(enum dyld_image_states state, uint32_t infoCount, const dyld_image_info info[]);
extern size_t allImagesCount();

// magic so CrashReporter logs message
extern "C" {
	char error_string[1024];
}

// magic linker symbol for start of dyld binary
extern "C" const macho_header __dso_handle;


//
// The file contains the core of dyld used to get a process to main().  
// The API's that dyld supports are implemented in dyldAPIs.cpp.
//
//
//
//
//
namespace dyld {
	struct RegisteredDOF { const mach_header* mh; int registrationID; };
	struct DylibOverride { const char* installName; const char* override; };
}


VECTOR_NEVER_DESTRUCTED(ImageLoader*);
VECTOR_NEVER_DESTRUCTED(dyld::RegisteredDOF);
VECTOR_NEVER_DESTRUCTED(dyld::ImageCallback);
VECTOR_NEVER_DESTRUCTED(dyld::DylibOverride);
VECTOR_NEVER_DESTRUCTED(ImageLoader::DynamicReference);

VECTOR_NEVER_DESTRUCTED(dyld_image_state_change_handler);

namespace dyld {


// 
// state of all environment variables dyld uses
//
struct EnvironmentVariables {
	const char* const *			DYLD_FRAMEWORK_PATH;
	const char* const *			DYLD_FALLBACK_FRAMEWORK_PATH;
	const char* const *			DYLD_LIBRARY_PATH;
	const char* const *			DYLD_FALLBACK_LIBRARY_PATH;
	const char* const *			DYLD_INSERT_LIBRARIES;
	const char* const *			LD_LIBRARY_PATH;			// for unix conformance
	const char* const *			DYLD_VERSIONED_LIBRARY_PATH;
	const char* const *			DYLD_VERSIONED_FRAMEWORK_PATH;
	bool						DYLD_PRINT_LIBRARIES_POST_LAUNCH;
	bool						DYLD_BIND_AT_LAUNCH;
	bool						DYLD_PRINT_STATISTICS;
	bool						DYLD_PRINT_STATISTICS_DETAILS;
	bool						DYLD_PRINT_OPTS;
	bool						DYLD_PRINT_ENV;
	bool						DYLD_DISABLE_DOFS;
	bool						DYLD_PRINT_CS_NOTIFICATIONS;
                            //  DYLD_SHARED_CACHE_DONT_VALIDATE ==> sSharedCacheIgnoreInodeAndTimeStamp
                            //  DYLD_SHARED_CACHE_DIR           ==> sSharedCacheDir
							//	DYLD_ROOT_PATH					==> gLinkContext.rootPaths
							//	DYLD_IMAGE_SUFFIX				==> gLinkContext.imageSuffix
							//	DYLD_PRINT_OPTS					==> gLinkContext.verboseOpts
							//	DYLD_PRINT_ENV					==> gLinkContext.verboseEnv
							//	DYLD_FORCE_FLAT_NAMESPACE		==> gLinkContext.bindFlat
							//	DYLD_PRINT_INITIALIZERS			==> gLinkContext.verboseInit
							//	DYLD_PRINT_SEGMENTS				==> gLinkContext.verboseMapping
							//	DYLD_PRINT_BINDINGS				==> gLinkContext.verboseBind
							//  DYLD_PRINT_WEAK_BINDINGS		==> gLinkContext.verboseWeakBind
							//	DYLD_PRINT_REBASINGS			==> gLinkContext.verboseRebase
							//	DYLD_PRINT_DOFS					==> gLinkContext.verboseDOF
							//	DYLD_PRINT_APIS					==> gLogAPIs
							//	DYLD_IGNORE_PREBINDING			==> gLinkContext.prebindUsage
							//	DYLD_PREBIND_DEBUG				==> gLinkContext.verbosePrebinding
							//	DYLD_NEW_LOCAL_SHARED_REGIONS	==> gLinkContext.sharedRegionMode
							//	DYLD_SHARED_REGION				==> gLinkContext.sharedRegionMode
							//	DYLD_PRINT_WARNINGS				==> gLinkContext.verboseWarnings
							//	DYLD_PRINT_RPATHS				==> gLinkContext.verboseRPaths
							//	DYLD_PRINT_INTERPOSING			==> gLinkContext.verboseInterposing
							//  DYLD_PRINT_LIBRARIES			==> gLinkContext.verboseLoading
};



typedef std::vector<dyld_image_state_change_handler> StateHandlers;


enum EnvVarMode { envNone, envPrintOnly, envAll };

static const char* _simple_getenv(const char* envp[], const char* key)
{
	const size_t len = strlen(key);
	for (int i = 0; envp[i] != NULL; i++)
	{
		if (strncmp(envp[i], key, len) == 0 && envp[i][len] == '=')
			return envp[i] + len + 1;
	}
	return NULL;
}
	
// all global state
static const char*					sExecPath = NULL;
static const char*					sExecShortName = NULL;
static const macho_header*			sMainExecutableMachHeader = NULL;
#if CPU_SUBTYPES_SUPPORTED
static cpu_type_t					sHostCPU;
static cpu_subtype_t				sHostCPUsubtype;
#endif
static ImageLoaderMachO*			sMainExecutable = NULL;
static EnvVarMode					sEnvMode = envNone;
static size_t						sInsertedDylibCount = 0;
static std::vector<ImageLoader*>	sAllImages;
static std::vector<ImageLoader*>	sImageRoots;
static std::vector<ImageLoader*>	sImageFilesNeedingTermination;
static std::vector<RegisteredDOF>	sImageFilesNeedingDOFUnregistration;
static std::vector<ImageCallback>   sAddImageCallbacks;
static std::vector<ImageCallback>   sRemoveImageCallbacks;
static bool							sRemoveImageCallbacksInUse = false;
static void*						sSingleHandlers[7][3];
static void*						sBatchHandlers[7][3];
static ImageLoader*					sLastImageByAddressCache;
static EnvironmentVariables			sEnv;
#if __MAC_OS_X_VERSION_MIN_REQUIRED
static const char*					sFrameworkFallbackPaths[] = { "$HOME/Library/Frameworks", "/Library/Frameworks", "/Network/Library/Frameworks", "/System/Library/Frameworks", NULL };
static const char*					sLibraryFallbackPaths[] = { "$HOME/lib", "/usr/local/lib", "/usr/lib", NULL };
#else
static const char*					sFrameworkFallbackPaths[] = { "/System/Library/Frameworks", NULL };
static const char*					sLibraryFallbackPaths[] = { "/usr/local/lib", "/usr/lib", NULL };
#endif
static const char*					sRestrictedFrameworkFallbackPaths[] = { "/System/Library/Frameworks", NULL };
static const char*					sRestrictedLibraryFallbackPaths[] = { "/usr/lib", NULL };
static UndefinedHandler				sUndefinedHandler = NULL;
static ImageLoader*					sBundleBeingLoaded = NULL;	// hack until OFI is reworked
#if DYLD_SHARED_CACHE_SUPPORT
static const dyld_cache_header*		sSharedCache = NULL;
static long							sSharedCacheSlide = 0;
static bool							sSharedCacheIgnoreInodeAndTimeStamp = false;
	   bool							gSharedCacheOverridden = false;
#if __IPHONE_OS_VERSION_MIN_REQUIRED
	static const char*				sSharedCacheDir = IPHONE_DYLD_SHARED_CACHE_DIR;
	#define ENABLE_DYLIBS_TO_OVERRIDE_CACHE_SIZE 1024
#else
	static const char*				sSharedCacheDir = MACOSX_DYLD_SHARED_CACHE_DIR;
#endif
#endif
ImageLoader::LinkContext			gLinkContext;
bool								gLogAPIs = false;
#if SUPPORT_ACCELERATE_TABLES
bool								gLogAppAPIs = false;
#endif
const struct LibSystemHelpers*		gLibSystemHelpers = NULL;
#if SUPPORT_OLD_CRT_INITIALIZATION
bool								gRunInitializersOldWay = false;
#endif
static std::vector<DylibOverride>	sDylibOverrides;
#if !TARGET_IPHONE_SIMULATOR	
static int							sLogSocket = -1;
#endif
static bool							sFrameworksFoundAsDylibs = false;
#if __x86_64__ && DYLD_SHARED_CACHE_SUPPORT
static bool							sHaswell = false;
#endif
static std::vector<ImageLoader::DynamicReference> sDynamicReferences;
static OSSpinLock					sDynamicReferencesLock = 0;
#if !TARGET_IPHONE_SIMULATOR
static bool							sLogToFile = false;
#endif
static char							sLoadingCrashMessage[1024] = "dyld: launch, loading dependent libraries";

static _dyld_objc_notify_mapped		sNotifyObjCMapped;
static _dyld_objc_notify_init		sNotifyObjCInit;
static _dyld_objc_notify_unmapped	sNotifyObjCUnmapped;

#if __IPHONE_OS_VERSION_MIN_REQUIRED && !TARGET_IPHONE_SIMULATOR
static bool							sForceStderr = false;
#endif



#if SUPPORT_ACCELERATE_TABLES
static ImageLoaderMegaDylib*		sAllCacheImagesProxy = NULL;
static bool							sDisableAcceleratorTables = false;
#endif

//
// The MappedRanges structure is used for fast address->image lookups.
// The table is only updated when the dyld lock is held, so we don't
// need to worry about multiple writers.  But readers may look at this
// data without holding the lock. Therefore, all updates must be done
// in an order that will never cause readers to see inconsistent data.
// The general rule is that if the image field is non-NULL then
// the other fields are valid.
//
struct MappedRanges
{
	MappedRanges*		next;
	unsigned long		count;
	struct {
		ImageLoader*	image;
		uintptr_t		start;
		uintptr_t		end;
	} array[1];
};

static MappedRanges*	sMappedRangesStart;

void addMappedRange(ImageLoader* image, uintptr_t start, uintptr_t end)
{
	//dyld::log("addMappedRange(0x%lX->0x%lX) for %s\n", start, end, image->getShortName());
	for (MappedRanges* p = sMappedRangesStart; p != NULL; p = p->next) {
		for (unsigned long i=0; i < p->count; ++i) {
			if ( p->array[i].image == NULL ) {
				p->array[i].start = start;
				p->array[i].end = end;
				// add image field last with a barrier so that any reader will see consistent records
				OSMemoryBarrier();
				p->array[i].image = image;
				return;
			}
		}
	}
	// table must be full, chain another
#if SUPPORT_ACCELERATE_TABLES
	unsigned count = (sAllCacheImagesProxy != NULL) ? 16 : 400;
#else
	unsigned count = 400;
#endif
	size_t allocationSize = sizeof(MappedRanges) + (count-1)*3*sizeof(void*);
	MappedRanges* newRanges = (MappedRanges*)malloc(allocationSize);
	bzero(newRanges, allocationSize);
	newRanges->count = count;
	newRanges->array[0].start = start;
	newRanges->array[0].end = end;
	newRanges->array[0].image = image;
	OSMemoryBarrier();
	if ( sMappedRangesStart == NULL ) {
		sMappedRangesStart = newRanges;
	}
	else {
		for (MappedRanges* p = sMappedRangesStart; p != NULL; p = p->next) {
			if ( p->next == NULL ) {
				OSMemoryBarrier();
				p->next = newRanges;
				break;
			}
		}
	}
}

void removedMappedRanges(ImageLoader* image)
{
	for (MappedRanges* p = sMappedRangesStart; p != NULL; p = p->next) {
		for (unsigned long i=0; i < p->count; ++i) {
			if ( p->array[i].image == image ) {
				// clear with a barrier so that any reader will see consistent records
				OSMemoryBarrier();
				p->array[i].image = NULL;
			}
		}
	}
}

ImageLoader* findMappedRange(uintptr_t target)
{
	for (MappedRanges* p = sMappedRangesStart; p != NULL; p = p->next) {
		for (unsigned long i=0; i < p->count; ++i) {
			if ( p->array[i].image != NULL ) {
				if ( (p->array[i].start <= target) && (target < p->array[i].end) )
					return p->array[i].image;
			}
		}
	}
	return NULL;
}



const char* mkstringf(const char* format, ...)
{
	_SIMPLE_STRING buf = _simple_salloc();
	if ( buf != NULL ) {
		va_list	list;
		va_start(list, format);
		_simple_vsprintf(buf, format, list);
		va_end(list);
		const char*	t = strdup(_simple_string(buf));
		_simple_sfree(buf);
		if ( t != NULL )
			return t;
	}
	return "mkstringf, out of memory error";
}


void throwf(const char* format, ...) 
{
	_SIMPLE_STRING buf = _simple_salloc();
	if ( buf != NULL ) {
		va_list	list;
		va_start(list, format);
		_simple_vsprintf(buf, format, list);
		va_end(list);
		const char*	t = strdup(_simple_string(buf));
		_simple_sfree(buf);
		if ( t != NULL )
			throw t;
	}
	throw "throwf, out of memory error";
}


#if !TARGET_IPHONE_SIMULATOR
static int sLogfile = STDERR_FILENO;
#endif

#if !TARGET_IPHONE_SIMULATOR	
// based on CFUtilities.c: also_do_stderr()
static bool useSyslog()
{
	// Use syslog() for processes managed by launchd
	static bool launchdChecked = false;
	static bool launchdOwned = false;
	if ( !launchdChecked && gProcessInfo->libSystemInitialized ) {
		if ( (gLibSystemHelpers != NULL) && (gLibSystemHelpers->version >= 11) ) {
			// <rdar://problem/23520449> only call isLaunchdOwned() after libSystem is initialized
			launchdOwned = (*gLibSystemHelpers->isLaunchdOwned)();
			launchdChecked = true;
		}
	}
	if ( launchdChecked && launchdOwned )
		return true;

	// If stderr is not available, use syslog()
	struct stat sb;
	int result = fstat(STDERR_FILENO, &sb);
	if ( result < 0 )
		return true; // file descriptor 2 is closed

	return false;
}

	
static void socket_syslogv(int priority, const char* format, va_list list)
{
	// lazily create socket and connection to syslogd
	if ( sLogSocket == -1 ) {
		sLogSocket = ::socket(AF_UNIX, SOCK_DGRAM, 0);
		if (sLogSocket == -1)
			return;  // cannot log
		::fcntl(sLogSocket, F_SETFD, 1);
	
		struct sockaddr_un addr;
		addr.sun_family = AF_UNIX;
		strncpy(addr.sun_path, _PATH_LOG, sizeof(addr.sun_path));
		if ( ::connect(sLogSocket, (struct sockaddr *)&addr, sizeof(addr)) == -1 ) {
			::close(sLogSocket);
			sLogSocket = -1;
			return;
		}
	}
	
	// format message to syslogd like: "<priority>Process[pid]: message"
	_SIMPLE_STRING buf = _simple_salloc();
	if ( buf == NULL )
		return;
	if ( _simple_sprintf(buf, "<%d>%s[%d]: ", LOG_USER|LOG_NOTICE, sExecShortName, getpid()) == 0 ) {
		if ( _simple_vsprintf(buf, format, list) == 0 ) {
			const char* p = _simple_string(buf);
			::__sendto(sLogSocket, p, strlen(p), 0, NULL, 0);
		}
	}
	_simple_sfree(buf);
}



void vlog(const char* format, va_list list)
{
#if __IPHONE_OS_VERSION_MIN_REQUIRED && !TARGET_IPHONE_SIMULATOR
	// <rdar://problem/25965832> log to console when running iOS app from Xcode
	if ( !sLogToFile && !sForceStderr && useSyslog() )
#else
	if ( !sLogToFile && useSyslog() )
#endif
		socket_syslogv(LOG_ERR, format, list);
	else {
		_simple_vdprintf(sLogfile, format, list);
	}
}

void log(const char* format, ...)
{
	va_list	list;
	va_start(list, format);
	vlog(format, list);
	va_end(list);
}


void vwarn(const char* format, va_list list) 
{
	_simple_dprintf(sLogfile, "dyld: warning, ");
	_simple_vdprintf(sLogfile, format, list);
}

void warn(const char* format, ...) 
{
	va_list	list;
	va_start(list, format);
	vwarn(format, list);
	va_end(list);
}


#endif // !TARGET_IPHONE_SIMULATOR	


// <rdar://problem/8867781> control access to sAllImages through a lock 
// because global dyld lock is not held during initialization phase of dlopen()
// <rdar://problem/16145518> Use OSSpinLockLock to allow yielding
static OSSpinLock sAllImagesLock = 0;

static void allImagesLock()
{
	OSSpinLockLock(&sAllImagesLock);
}

static void allImagesUnlock()
{
	OSSpinLockUnlock(&sAllImagesLock);
}


// utility class to assure files are closed when an exception is thrown
class FileOpener {
public:
	FileOpener(const char* path);
	~FileOpener();
	int getFileDescriptor() { return fd; }
private:
	int fd;
};

FileOpener::FileOpener(const char* path)
 : fd(-1)
{
	fd = my_open(path, O_RDONLY, 0);
}

FileOpener::~FileOpener()
{
	if ( fd != -1 )
		close(fd);
}


static void	registerDOFs(const std::vector<ImageLoader::DOFInfo>& dofs)
{
	const size_t dofSectionCount = dofs.size();
	if ( !sEnv.DYLD_DISABLE_DOFS && (dofSectionCount != 0) ) {
		int fd = open("/dev/" DTRACEMNR_HELPER, O_RDWR);
		if ( fd < 0 ) {
			//dyld::warn("can't open /dev/" DTRACEMNR_HELPER " to register dtrace DOF sections\n");
		}
		else {
			// allocate a buffer on the stack for the variable length dof_ioctl_data_t type
			uint8_t buffer[sizeof(dof_ioctl_data_t) + dofSectionCount*sizeof(dof_helper_t)];
			dof_ioctl_data_t* ioctlData = (dof_ioctl_data_t*)buffer;
			
			// fill in buffer with one dof_helper_t per DOF section
			ioctlData->dofiod_count = dofSectionCount;
			for (unsigned int i=0; i < dofSectionCount; ++i) {
				strlcpy(ioctlData->dofiod_helpers[i].dofhp_mod, dofs[i].imageShortName, DTRACE_MODNAMELEN);
				ioctlData->dofiod_helpers[i].dofhp_dof = (uintptr_t)(dofs[i].dof);
				ioctlData->dofiod_helpers[i].dofhp_addr = (uintptr_t)(dofs[i].dof);
			}
			
			// tell kernel about all DOF sections en mas
			// pass pointer to ioctlData because ioctl() only copies a fixed size amount of data into kernel
			user_addr_t val = (user_addr_t)(unsigned long)ioctlData;
			if ( ioctl(fd, DTRACEHIOC_ADDDOF, &val) != -1 ) {
				// kernel returns a unique identifier for each section in the dofiod_helpers[].dofhp_dof field.
				for (unsigned int i=0; i < dofSectionCount; ++i) {
					RegisteredDOF info;
					info.mh = dofs[i].imageHeader;
					info.registrationID = (int)(ioctlData->dofiod_helpers[i].dofhp_dof);
					sImageFilesNeedingDOFUnregistration.push_back(info);
					if ( gLinkContext.verboseDOF ) {
						dyld::log("dyld: registering DOF section %p in %s with dtrace, ID=0x%08X\n", 
							dofs[i].dof, dofs[i].imageShortName, info.registrationID);
					}
				}
			}
			else {
				//dyld::log( "dyld: ioctl to register dtrace DOF section failed\n");
			}
			close(fd);
		}
	}
}

static void	unregisterDOF(int registrationID)
{
	int fd = open("/dev/" DTRACEMNR_HELPER, O_RDWR);
	if ( fd < 0 ) {
		dyld::warn("can't open /dev/" DTRACEMNR_HELPER " to unregister dtrace DOF section\n");
	}
	else {
		ioctl(fd, DTRACEHIOC_REMOVE, registrationID);
		close(fd);
		if ( gLinkContext.verboseInit )
			dyld::warn("unregistering DOF section ID=0x%08X with dtrace\n", registrationID);
	}
}


//
// _dyld_register_func_for_add_image() is implemented as part of the general image state change notification
//
static void notifyAddImageCallbacks(ImageLoader* image)
{
	// use guard so that we cannot notify about the same image twice
	if ( ! image->addFuncNotified() ) {
		for (std::vector<ImageCallback>::iterator it=sAddImageCallbacks.begin(); it != sAddImageCallbacks.end(); it++)
			(*it)(image->machHeader(), image->getSlide());
		image->setAddFuncNotified();
	}
}



// notify gdb about these new images
static const char* updateAllImages(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[])
{
	// <rdar://problem/8812589> don't add images without paths to all-image-info-list
	if ( info[0].imageFilePath != NULL )
		addImagesToAllImages(infoCount, info);
	return NULL;
}


static StateHandlers* stateToHandlers(dyld_image_states state, void* handlersArray[7][3])
{
	switch ( state ) {
		case dyld_image_state_mapped:
			return reinterpret_cast<StateHandlers*>(&handlersArray[0]);
			
		case dyld_image_state_dependents_mapped:
			return reinterpret_cast<StateHandlers*>(&handlersArray[1]);
			
		case dyld_image_state_rebased:
			return reinterpret_cast<StateHandlers*>(&handlersArray[2]);
			
		case dyld_image_state_bound:
			return reinterpret_cast<StateHandlers*>(&handlersArray[3]);
			
		case dyld_image_state_dependents_initialized:
			return reinterpret_cast<StateHandlers*>(&handlersArray[4]);

		case dyld_image_state_initialized:
			return reinterpret_cast<StateHandlers*>(&handlersArray[5]);
			
		case dyld_image_state_terminated:
			return reinterpret_cast<StateHandlers*>(&handlersArray[6]);
	}
	return NULL;
}

#if SUPPORT_ACCELERATE_TABLES
static dyld_image_state_change_handler getPreInitNotifyHandler(unsigned index)
{
	std::vector<dyld_image_state_change_handler>* handlers = stateToHandlers(dyld_image_state_dependents_initialized, sSingleHandlers);
	if ( index >= handlers->size() )
		return NULL;
	return (*handlers)[index];
}

static dyld_image_state_change_handler getBoundBatchHandler(unsigned index)
{
	std::vector<dyld_image_state_change_handler>* handlers = stateToHandlers(dyld_image_state_bound, sBatchHandlers);
	if ( index >= handlers->size() )
		return NULL;
	return (*handlers)[index];
}

static void notifySingleFromCache(dyld_image_states state, const mach_header* mh, const char* path)
{
	//dyld::log("notifySingle(state=%d, image=%s)\n", state, image->getPath());
	std::vector<dyld_image_state_change_handler>* handlers = stateToHandlers(state, sSingleHandlers);
	if ( handlers != NULL ) {
		dyld_image_info info;
		info.imageLoadAddress	= mh;
		info.imageFilePath		= path;
		info.imageFileModDate	= 0;
		for (dyld_image_state_change_handler handler : *handlers) {
			const char* result = (*handler)(state, 1, &info);
			if ( (result != NULL) && (state == dyld_image_state_mapped) ) {
				//fprintf(stderr, "  image rejected by handler=%p\n", *it);
				// make copy of thrown string so that later catch clauses can free it
				const char* str = strdup(result);
				throw str;
			}
		}
	}
	if ( (state == dyld_image_state_dependents_initialized) && (sNotifyObjCInit != NULL) && (mh->flags & MH_HAS_OBJC) ) {
		(*sNotifyObjCInit)(path, mh);
	}
}
#endif

static mach_port_t sNotifyReplyPorts[DYLD_MAX_PROCESS_INFO_NOTIFY_COUNT];


static void notifyMonitoringDyld(bool unloading, unsigned portSlot, unsigned imageCount, const dyld_image_info infos[])
{
	unsigned entriesSize = imageCount*sizeof(dyld_process_info_image_entry);
	unsigned pathsSize = 0;
	for (unsigned j=0; j < imageCount; ++j) {
		pathsSize += (strlen(infos[j].imageFilePath) + 1);
	}
	unsigned totalSize = (sizeof(dyld_process_info_notify_header) + entriesSize + pathsSize + 127) & -128;   // align
	if ( totalSize > DYLD_PROCESS_INFO_NOTIFY_MAX_BUFFER_SIZE ) {
		// Putting all image paths into one message would make buffer too big.
		// Instead split into two messages.  Recurse as needed until paths fit in buffer.
		unsigned imageHalfCount = imageCount/2;
		notifyMonitoringDyld(unloading, portSlot, imageHalfCount, infos);
		notifyMonitoringDyld(unloading, portSlot, imageCount - imageHalfCount, &infos[imageHalfCount]);
		return;
	}
	uint8_t	buffer[totalSize];
	dyld_process_info_notify_header* header = (dyld_process_info_notify_header*)buffer;
	header->version			= 1;
	header->imageCount		= imageCount;
	header->imagesOffset	= sizeof(dyld_process_info_notify_header);
	header->stringsOffset	= sizeof(dyld_process_info_notify_header) + entriesSize;
	header->timestamp		= mach_absolute_time();
	dyld_process_info_image_entry* entries = (dyld_process_info_image_entry*)&buffer[header->imagesOffset];
	char* const pathPoolStart = (char*)&buffer[header->stringsOffset];
	char* pathPool = pathPoolStart;
	for (unsigned j=0; j < imageCount; ++j) {
		strcpy(pathPool, infos[j].imageFilePath);
		uint32_t len = (uint32_t)strlen(pathPool);
		bzero(entries->uuid, 16);
		const ImageLoader* image = findImageByMachHeader(infos[j].imageLoadAddress);
		if ( image != NULL ) {
			image->getUUID(entries->uuid);
		}
#if SUPPORT_ACCELERATE_TABLES
		else if ( sAllCacheImagesProxy != NULL ) {
			const mach_header* mh;
			const char* path;
			unsigned index;
			if ( sAllCacheImagesProxy->addressInCache(infos[j].imageLoadAddress, &mh, &path, &index) ) {
				sAllCacheImagesProxy->getDylibUUID(index, entries->uuid);
			}
		}
#endif
		entries->loadAddress = (uint64_t)infos[j].imageLoadAddress;
		entries->pathStringOffset = (uint32_t)(pathPool - pathPoolStart);
		entries->pathLength  = len;
		pathPool += (len +1);
		++entries;
	}

	if ( sNotifyReplyPorts[portSlot] == 0 ) {
		if ( !mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &sNotifyReplyPorts[portSlot]) )
			mach_port_insert_right(mach_task_self(), sNotifyReplyPorts[portSlot], sNotifyReplyPorts[portSlot], MACH_MSG_TYPE_MAKE_SEND);
		//dyld::log("allocated reply port %d\n", sNotifyReplyPorts[portSlot]);
	}
	//dyld::log("found port to send to\n");
	mach_msg_header_t* h = (mach_msg_header_t*)buffer;
	h->msgh_bits		= MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND,MACH_MSG_TYPE_MAKE_SEND); // MACH_MSG_TYPE_MAKE_SEND_ONCE
	h->msgh_id			= unloading ? DYLD_PROCESS_INFO_NOTIFY_UNLOAD_ID : DYLD_PROCESS_INFO_NOTIFY_LOAD_ID;
	h->msgh_local_port	= sNotifyReplyPorts[portSlot];
	h->msgh_remote_port = dyld::gProcessInfo->notifyPorts[portSlot];
	h->msgh_reserved	= 0;
	h->msgh_size		= (mach_msg_size_t)sizeof(buffer);
	//dyld::log("sending to port[%d]=%d, size=%d, reply port=%d, id=0x%X\n", portSlot, dyld::gProcessInfo->notifyPorts[portSlot], h->msgh_size, sNotifyReplyPorts[portSlot], h->msgh_id);
	kern_return_t sendResult = mach_msg(h, MACH_SEND_MSG | MACH_RCV_MSG | MACH_SEND_TIMEOUT, h->msgh_size, h->msgh_size, sNotifyReplyPorts[portSlot], 100, MACH_PORT_NULL);
	//dyld::log("send result = 0x%X, msg_id=%d, msg_size=%d\n", sendResult, h->msgh_id, h->msgh_size);
	if ( sendResult == MACH_SEND_INVALID_DEST ) {
		// sender is not responding, detatch
		//dyld::log("process requesting notification gone. deallocation send port %d and receive port %d\n", dyld::gProcessInfo->notifyPorts[portSlot], sNotifyReplyPorts[portSlot]);
		mach_port_deallocate(mach_task_self(), dyld::gProcessInfo->notifyPorts[portSlot]);
		mach_port_deallocate(mach_task_self(), sNotifyReplyPorts[portSlot]);
		dyld::gProcessInfo->notifyPorts[portSlot] = 0;
		sNotifyReplyPorts[portSlot] = 0;
	}
}

#define MAX_KERNEL_IMAGES_PER_CALL (100)

static void flushKernelNotifications(bool loading, bool force, std::array<dyld_kernel_image_info_t,MAX_KERNEL_IMAGES_PER_CALL>& kernelInfos, uint32_t &kernelInfoCount) {
#if 0
	if ((force && kernelInfoCount != 0) || kernelInfoCount == MAX_KERNEL_IMAGES_PER_CALL) {
		if (loading) {
			task_register_dyld_image_infos(mach_task_self(), kernelInfos.data(), kernelInfoCount);
		} else {
			task_unregister_dyld_image_infos(mach_task_self(), kernelInfos.data(), kernelInfoCount);
		}
		kernelInfoCount = 0;
	}
#endif
}

static
void queueKernelNotification(const ImageLoader& image, bool loading, std::array<dyld_kernel_image_info_t,MAX_KERNEL_IMAGES_PER_CALL>& kernelInfos, uint32_t &kernelInfoCount) {
	if ( !image.inSharedCache() ) {
		ino_t inode = image.getInode();
		image.getUUID(kernelInfos[kernelInfoCount].uuid);
		memcpy(&kernelInfos[kernelInfoCount].fsobjid, &inode, 8);
		kernelInfos[kernelInfoCount].load_addr = (uint64_t)image.machHeader();
		// FIXME we should also be grabbing the device ID, but that is not necessary yet,
		// and requires threading it through the ImageLoader
		kernelInfos[kernelInfoCount].fsid.val[0] = 0;
		kernelInfos[kernelInfoCount].fsid.val[1] = 0;
		kernelInfoCount++;
	}
	flushKernelNotifications(loading, false, kernelInfos, kernelInfoCount);
}

void notifyKernel(const ImageLoader& image, bool loading) {
	std::array<dyld_kernel_image_info_t,MAX_KERNEL_IMAGES_PER_CALL> kernelInfos;
	uint32_t kernelInfoCount = 0;
	queueKernelNotification(image, loading, kernelInfos, kernelInfoCount);
	flushKernelNotifications(loading, true, kernelInfos, kernelInfoCount);
}

static void notifySingle(dyld_image_states state, const ImageLoader* image, ImageLoader::InitializerTimingList* timingInfo)
{
	//dyld::log("notifySingle(state=%d, image=%s)\n", state, image->getPath());
	std::vector<dyld_image_state_change_handler>* handlers = stateToHandlers(state, sSingleHandlers);
	if ( handlers != NULL ) {
		dyld_image_info info;
		info.imageLoadAddress	= image->machHeader();
		info.imageFilePath		= image->getRealPath();
		info.imageFileModDate	= image->lastModified();
		for (std::vector<dyld_image_state_change_handler>::iterator it = handlers->begin(); it != handlers->end(); ++it) {
			const char* result = (*it)(state, 1, &info);
			if ( (result != NULL) && (state == dyld_image_state_mapped) ) {
				//fprintf(stderr, "  image rejected by handler=%p\n", *it);
				// make copy of thrown string so that later catch clauses can free it
				const char* str = strdup(result);
				throw str;
			}
		}
	}
	if ( state == dyld_image_state_mapped ) {
		// <rdar://problem/7008875> Save load addr + UUID for images from outside the shared cache
		if ( !image->inSharedCache() ) {
			dyld_uuid_info info;
			if ( image->getUUID(info.imageUUID) ) {
				info.imageLoadAddress = image->machHeader();
				addNonSharedCacheImageUUID(info);
			}
		}
	}
	if ( (state == dyld_image_state_dependents_initialized) && (sNotifyObjCInit != NULL) && image->notifyObjC() ) {
		uint64_t t0 = mach_absolute_time();
		(*sNotifyObjCInit)(image->getRealPath(), image->machHeader());
		uint64_t t1 = mach_absolute_time();
		uint64_t t2 = mach_absolute_time();
		uint64_t timeInObjC = t1-t0;
		uint64_t emptyTime = (t2-t1)*100;
		if ( (timeInObjC > emptyTime) && (timingInfo != NULL) ) {
			timingInfo->addTime(image->getShortName(), timeInObjC);
		}
	}
#if 0
    // mach message csdlc about dynamically unloaded images
	if ( image->addFuncNotified() && (state == dyld_image_state_terminated) ) {
		notifyKernel(*image, false);

		uint64_t loadTimestamp = mach_absolute_time();
		if ( sEnv.DYLD_PRINT_CS_NOTIFICATIONS ) {
			dyld::log("dyld: coresymbolication_unload_notifier(%p, 0x%016llX, %p, %s)\n",
					  dyld::gProcessInfo->coreSymbolicationShmPage, loadTimestamp, image->machHeader(), image->getPath());
		}
		if ( dyld::gProcessInfo->coreSymbolicationShmPage != NULL) {
			coresymbolication_unload_notifier(dyld::gProcessInfo->coreSymbolicationShmPage, loadTimestamp, image->getPath(), image->machHeader());
		}
		for (int slot=0; slot < DYLD_MAX_PROCESS_INFO_NOTIFY_COUNT; ++slot) {
			if ( dyld::gProcessInfo->notifyPorts[slot] != 0 ) {
				dyld_image_info info;
				info.imageLoadAddress	= image->machHeader();
				info.imageFilePath		= image->getPath();
				info.imageFileModDate	= 0;
				notifyMonitoringDyld(true, slot, 1, &info);
			}
			else if ( sNotifyReplyPorts[slot] != 0 ) {
				// monitoring process detached from this process, so release reply port
				//dyld::log("deallocated reply port %d\n", sNotifyReplyPorts[slot]);
				mach_port_deallocate(mach_task_self(), sNotifyReplyPorts[slot]);
				sNotifyReplyPorts[slot] = 0;
			}
		}
	}
#endif
}


//
// Normally, dyld_all_image_infos is only updated in batches after an entire
// graph is loaded.  But if there is an error loading the initial set of
// dylibs needed by the main executable, dyld_all_image_infos is not yet set 
// up, leading to usually brief crash logs.
//
// This function manually adds the images loaded so far to dyld::gProcessInfo.
// It should only be called before terminating.
//
void syncAllImages()
{
	for (std::vector<ImageLoader*>::iterator it=sAllImages.begin(); it != sAllImages.end(); ++it) {
		dyld_image_info info;
		ImageLoader* image = *it;
		info.imageLoadAddress = image->machHeader();
		info.imageFilePath = image->getRealPath();
		info.imageFileModDate = image->lastModified();
		// add to all_image_infos if not already there
		bool found = false;
		int existingCount = dyld::gProcessInfo->infoArrayCount;
		const dyld_image_info* existing = dyld::gProcessInfo->infoArray;
		if ( existing != NULL ) {
			for (int i=0; i < existingCount; ++i) {
				if ( existing[i].imageLoadAddress == info.imageLoadAddress ) {
					//dyld::log("not adding %s\n", info.imageFilePath);
					found = true;
					break;
				}
			}
		}
		if ( ! found ) {
			//dyld::log("adding %s\n", info.imageFilePath);
			addImagesToAllImages(1, &info);
		}
	}
}


static int imageSorter(const void* l, const void* r)
{
	const ImageLoader* left = *((ImageLoader**)l);
	const ImageLoader* right= *((ImageLoader**)r);
	return left->compare(right);
}

static void notifyBatchPartial(dyld_image_states state, bool orLater, dyld_image_state_change_handler onlyHandler, bool preflightOnly, bool onlyObjCMappedNotification)
{
	std::vector<dyld_image_state_change_handler>* handlers = stateToHandlers(state, sBatchHandlers);
	std::array<dyld_kernel_image_info_t,MAX_KERNEL_IMAGES_PER_CALL> kernelInfos;
	uint32_t kernelInfoCount = 0;

	if ( (handlers != NULL) || ((state == dyld_image_state_bound) && (sNotifyObjCMapped != NULL)) ) {
		// don't use a vector because it will use malloc/free and we want notifcation to be low cost
        allImagesLock();
		dyld_image_info	infos[allImagesCount()+1];
        ImageLoader* images[allImagesCount()+1];
        ImageLoader** end = images;
        for (std::vector<ImageLoader*>::iterator it=sAllImages.begin(); it != sAllImages.end(); it++) {
            dyld_image_states imageState = (*it)->getState();
            if ( (imageState == state) || (orLater && (imageState > state)) )
                *end++ = *it;
        }
		if ( sBundleBeingLoaded != NULL ) {
			dyld_image_states imageState = sBundleBeingLoaded->getState();
			if ( (imageState == state) || (orLater && (imageState > state)) )
				*end++ = sBundleBeingLoaded;
		}
        const char* dontLoadReason = NULL;
		uint32_t imageCount = (uint32_t)(end-images);
		if ( imageCount != 0 ) {
			// sort bottom up
			qsort(images, imageCount, sizeof(ImageLoader*), &imageSorter);
			// build info array
			for (unsigned int i=0; i < imageCount; ++i) {
				dyld_image_info* p = &infos[i];
				ImageLoader* image = images[i];
				//dyld::log("  state=%d, name=%s\n", state, image->getPath());
				p->imageLoadAddress = image->machHeader();
				p->imageFilePath    = image->getRealPath();
				p->imageFileModDate = image->lastModified();
				// get these registered with the kernel as early as possible
				if ( state == dyld_image_state_dependents_mapped)
					queueKernelNotification(*image, true, kernelInfos, kernelInfoCount);
				// special case for add_image hook
				if ( state == dyld_image_state_bound )
					notifyAddImageCallbacks(image);
			}
			flushKernelNotifications(true, true, kernelInfos, kernelInfoCount);
		}
#if SUPPORT_ACCELERATE_TABLES
		if ( sAllCacheImagesProxy != NULL ) {
			unsigned cacheCount = sAllCacheImagesProxy->appendImagesToNotify(state, orLater, &infos[imageCount]);
			// support _dyld_register_func_for_add_image()
			if ( state == dyld_image_state_bound ) {
				for (ImageCallback callback : sAddImageCallbacks) {
					for (unsigned i=0; i < cacheCount; ++i)
						(*callback)(infos[imageCount+i].imageLoadAddress, sSharedCacheSlide);
				}
			}
			imageCount += cacheCount;
		}
#endif
		if ( imageCount != 0 ) {
			if ( !onlyObjCMappedNotification ) {
				if ( onlyHandler != NULL ) {
					const char* result = NULL;
					if ( result == NULL ) {
						result = (*onlyHandler)(state, imageCount, infos);
					}
					if ( (result != NULL) && (state == dyld_image_state_dependents_mapped) ) {
						//fprintf(stderr, "  images rejected by handler=%p\n", onlyHandler);
						// make copy of thrown string so that later catch clauses can free it
						dontLoadReason = strdup(result);
					}
				}
				else {
					// call each handler with whole array
					if ( handlers != NULL ) {
						for (std::vector<dyld_image_state_change_handler>::iterator it = handlers->begin(); it != handlers->end(); ++it) {
							const char* result = (*it)(state, imageCount, infos);
							if ( (result != NULL) && (state == dyld_image_state_dependents_mapped) ) {
								//fprintf(stderr, "  images rejected by handler=%p\n", *it);
								// make copy of thrown string so that later catch clauses can free it
								dontLoadReason = strdup(result);
								break;
							}
						}
					}
				}
			}
			// tell objc about new images
			if ( (onlyHandler == NULL) && ((state == dyld_image_state_bound) || (orLater && (dyld_image_state_bound > state))) && (sNotifyObjCMapped != NULL) ) {
				const char* paths[imageCount];
				const mach_header* mhs[imageCount];
				unsigned objcImageCount = 0;
				for (int i=0; i < imageCount; ++i) {
					const ImageLoader* image = findImageByMachHeader(infos[i].imageLoadAddress);
					bool hasObjC = false;
					if ( image != NULL ) {
						hasObjC = image->notifyObjC();
					}
#if SUPPORT_ACCELERATE_TABLES
					else if ( sAllCacheImagesProxy != NULL ) {
						const mach_header* mh;
						const char* path;
						unsigned index;
						if ( sAllCacheImagesProxy->addressInCache(infos[i].imageLoadAddress, &mh, &path, &index) ) {
							hasObjC = (mh->flags & MH_HAS_OBJC);
						}
					}
#endif
					if ( hasObjC ) {
						paths[objcImageCount] = infos[i].imageFilePath;
						mhs[objcImageCount]   = infos[i].imageLoadAddress;
						++objcImageCount;
					}
				}
				if ( objcImageCount != 0 ) {
					uint64_t t0 = mach_absolute_time();
					(*sNotifyObjCMapped)(objcImageCount, paths, mhs);
					uint64_t t1 = mach_absolute_time();
					ImageLoader::fgTotalObjCSetupTime += (t1-t0);
				}
			}
		}
        allImagesUnlock();
        if ( dontLoadReason != NULL )
            throw dontLoadReason;
		if ( !preflightOnly && (state == dyld_image_state_dependents_mapped) ) {
			if ( (dyld::gProcessInfo->coreSymbolicationShmPage != NULL) || sEnv.DYLD_PRINT_CS_NOTIFICATIONS ) {
				// mach message csdlc about loaded images
				uint64_t loadTimestamp = mach_absolute_time();
				for (unsigned j=0; j < imageCount; ++j) {
					if ( sEnv.DYLD_PRINT_CS_NOTIFICATIONS ) {
						dyld::log("dyld: coresymbolication_load_notifier(%p, 0x%016llX, %p, %s)\n",
								  dyld::gProcessInfo->coreSymbolicationShmPage, loadTimestamp, infos[j].imageLoadAddress, infos[j].imageFilePath);
					}
#if 0
					coresymbolication_load_notifier(dyld::gProcessInfo->coreSymbolicationShmPage, loadTimestamp, infos[j].imageFilePath, infos[j].imageLoadAddress);
#endif
				}
			}
			for (int slot=0; slot < DYLD_MAX_PROCESS_INFO_NOTIFY_COUNT; ++slot) {
				if ( dyld::gProcessInfo->notifyPorts[slot] )
					notifyMonitoringDyld(false, slot, imageCount, infos);
			}
		}
	}
}



static void notifyBatch(dyld_image_states state, bool preflightOnly)
{
	notifyBatchPartial(state, false, NULL, preflightOnly, false);
}

// In order for register_func_for_add_image() callbacks to to be called bottom up,
// we need to maintain a list of root images. The main executable is usally the
// first root. Any images dynamically added are also roots (unless already loaded).
// If DYLD_INSERT_LIBRARIES is used, those libraries are first.
static void addRootImage(ImageLoader* image)
{
	//dyld::log("addRootImage(%p, %s)\n", image, image->getPath());
	// add to list of roots
	sImageRoots.push_back(image);
}


static void clearAllDepths()
{
	for (std::vector<ImageLoader*>::iterator it=sAllImages.begin(); it != sAllImages.end(); it++)
		(*it)->clearDepth();
}

static void printAllDepths()
{
	for (std::vector<ImageLoader*>::iterator it=sAllImages.begin(); it != sAllImages.end(); it++)
		dyld::log("%03d %s\n",  (*it)->getDepth(), (*it)->getShortName());
}


static unsigned int imageCount()
{
    allImagesLock();
		unsigned int result = (unsigned int)sAllImages.size();
    allImagesUnlock();
	return (result);
}


static void setNewProgramVars(const ProgramVars& newVars)
{
	// make a copy of the pointers to program variables
	gLinkContext.programVars = newVars;
	
	// now set each program global to their initial value
	*gLinkContext.programVars.NXArgcPtr = gLinkContext.argc;
	*gLinkContext.programVars.NXArgvPtr = gLinkContext.argv;
	*gLinkContext.programVars.environPtr = gLinkContext.envp;
	*gLinkContext.programVars.__prognamePtr = gLinkContext.progname;
}

#if SUPPORT_OLD_CRT_INITIALIZATION
static void setRunInitialzersOldWay()
{
	gRunInitializersOldWay = true;		
}
#endif

static bool sandboxBlocked(const char* path, const char* kind)
{
#if TARGET_IPHONE_SIMULATOR || DARLING
	// sandbox calls not yet supported in simulator runtime
	return false;
#else
	sandbox_filter_type filter = (sandbox_filter_type)(SANDBOX_FILTER_PATH | SANDBOX_CHECK_NO_REPORT);
	return ( sandbox_check(getpid(), kind, filter, path) > 0 );
#endif
}

bool sandboxBlockedMmap(const char* path)
{
	return sandboxBlocked(path, "file-map-executable");
}

bool sandboxBlockedOpen(const char* path)
{
	return sandboxBlocked(path, "file-read-data");
}

bool sandboxBlockedStat(const char* path)
{
	return sandboxBlocked(path, "file-read-metadata");
}


static void addDynamicReference(ImageLoader* from, ImageLoader* to) {
	// don't add dynamic reference if target is in the shared cache (since it can't be unloaded)
	if ( to->inSharedCache() )
		return;

	// don't add dynamic reference if there already is a static one
	if ( from->dependsOn(to) )
		return;
	
	// don't add if this combination already exists
	OSSpinLockLock(&sDynamicReferencesLock);
	for (std::vector<ImageLoader::DynamicReference>::iterator it=sDynamicReferences.begin(); it != sDynamicReferences.end(); ++it) {
		if ( (it->from == from) && (it->to == to) ) {
			OSSpinLockUnlock(&sDynamicReferencesLock);
			return;
		}
	}

	//dyld::log("addDynamicReference(%s, %s\n", from->getShortName(), to->getShortName());
	ImageLoader::DynamicReference t;
	t.from = from;
	t.to = to;
	sDynamicReferences.push_back(t);
	OSSpinLockUnlock(&sDynamicReferencesLock);
}

static void addImage(ImageLoader* image)
{
	// add to master list
    allImagesLock();
        sAllImages.push_back(image);
    allImagesUnlock();
	
	// update mapped ranges
	uintptr_t lastSegStart = 0;
	uintptr_t lastSegEnd = 0;
	for(unsigned int i=0, e=image->segmentCount(); i < e; ++i) {
		if ( image->segUnaccessible(i) ) 
			continue;
		uintptr_t start = image->segActualLoadAddress(i);
		uintptr_t end = image->segActualEndAddress(i);
		if ( start == lastSegEnd ) {
			// two segments are contiguous, just record combined segments
			lastSegEnd = end;
		}
		else {
			// non-contiguous segments, record last (if any)
			if ( lastSegEnd != 0 )
				addMappedRange(image, lastSegStart, lastSegEnd);
			lastSegStart = start;
			lastSegEnd = end;
		}		
	}
	if ( lastSegEnd != 0 )
		addMappedRange(image, lastSegStart, lastSegEnd);

	
	if ( gLinkContext.verboseLoading || (sEnv.DYLD_PRINT_LIBRARIES_POST_LAUNCH && (sMainExecutable!=NULL) && sMainExecutable->isLinked()) ) {
		dyld::log("dyld: loaded: %s\n", image->getPath());
	}
	
}

//
// Helper for std::remove_if
//
class RefUsesImage {
public:
	RefUsesImage(ImageLoader* image) : _image(image) {}
	bool operator()(const ImageLoader::DynamicReference& ref) const {
		return ( (ref.from == _image) || (ref.to == _image) );
	}
private:
	ImageLoader* _image;
};



void removeImage(ImageLoader* image)
{
	// if has dtrace DOF section, tell dtrace it is going away, then remove from sImageFilesNeedingDOFUnregistration
	for (std::vector<RegisteredDOF>::iterator it=sImageFilesNeedingDOFUnregistration.begin(); it != sImageFilesNeedingDOFUnregistration.end(); ) {
		if ( it->mh == image->machHeader() ) {
			unregisterDOF(it->registrationID);
			sImageFilesNeedingDOFUnregistration.erase(it);
			// don't increment iterator, the erase caused next element to be copied to where this iterator points
		}
		else {
			++it;
		}
	}
	
	// tell all registered remove image handlers about this
	// do this before removing image from internal data structures so that the callback can query dyld about the image
	if ( image->getState() >= dyld_image_state_bound ) {
		sRemoveImageCallbacksInUse = true; // This only runs inside dyld's global lock, so ok to use a global for the in-use flag.
		for (std::vector<ImageCallback>::iterator it=sRemoveImageCallbacks.begin(); it != sRemoveImageCallbacks.end(); it++) {
			(*it)(image->machHeader(), image->getSlide());
		}
		sRemoveImageCallbacksInUse = false;

		if ( sNotifyObjCUnmapped !=  NULL && image->notifyObjC() )
			(*sNotifyObjCUnmapped)(image->getRealPath(), image->machHeader());
	}
	
	// notify 
	notifySingle(dyld_image_state_terminated, image, NULL);
	
	// remove from mapped images table
	removedMappedRanges(image);

	// remove from master list
    allImagesLock();
        for (std::vector<ImageLoader*>::iterator it=sAllImages.begin(); it != sAllImages.end(); it++) {
            if ( *it == image ) {
                sAllImages.erase(it);
                break;
            }
        }
    allImagesUnlock();
	
	// remove from sDynamicReferences
	OSSpinLockLock(&sDynamicReferencesLock);
		sDynamicReferences.erase(std::remove_if(sDynamicReferences.begin(), sDynamicReferences.end(), RefUsesImage(image)), sDynamicReferences.end());
	OSSpinLockUnlock(&sDynamicReferencesLock);

	// flush find-by-address cache (do this after removed from master list, so there is no chance it can come back)
	if ( sLastImageByAddressCache == image )
		sLastImageByAddressCache = NULL;

	// if in root list, pull it out 
	for (std::vector<ImageLoader*>::iterator it=sImageRoots.begin(); it != sImageRoots.end(); it++) {
		if ( *it == image ) {
			sImageRoots.erase(it);
			break;
		}
	}

	// log if requested
	if ( gLinkContext.verboseLoading || (sEnv.DYLD_PRINT_LIBRARIES_POST_LAUNCH && (sMainExecutable!=NULL) && sMainExecutable->isLinked()) ) {
		dyld::log("dyld: unloaded: %s\n", image->getPath());
	}

	// tell gdb, new way
	removeImageFromAllImages(image->machHeader());
}


void runImageStaticTerminators(ImageLoader* image)
{
	// if in termination list, pull it out and run terminator
	bool mightBeMore;
	do {
		mightBeMore = false;
		for (std::vector<ImageLoader*>::iterator it=sImageFilesNeedingTermination.begin(); it != sImageFilesNeedingTermination.end(); it++) {
			if ( *it == image ) {
				sImageFilesNeedingTermination.erase(it);
				if (gLogAPIs) dyld::log("dlclose(), running static terminators for %p %s\n", image, image->getShortName());
				image->doTermination(gLinkContext);
				mightBeMore = true;
				break;
			}
		}
	} while ( mightBeMore );
}

static void terminationRecorder(ImageLoader* image)
{
	sImageFilesNeedingTermination.push_back(image);
}

const char* getExecutablePath()
{
	return sExecPath;
}

static void runAllStaticTerminators(void* extra)
{
	try {
		const size_t imageCount = sImageFilesNeedingTermination.size();
		for(size_t i=imageCount; i > 0; --i){
			ImageLoader* image = sImageFilesNeedingTermination[i-1];
			image->doTermination(gLinkContext);
		}
		sImageFilesNeedingTermination.clear();
		notifyBatch(dyld_image_state_terminated, false);
	}
	catch (const char* msg) {
		halt(msg);
	}
}

void initializeMainExecutable()
{
	// record that we've reached this step
	gLinkContext.startedInitializingMainExecutable = true;

	// run initialzers for any inserted dylibs
	ImageLoader::InitializerTimingList initializerTimes[allImagesCount()];
	initializerTimes[0].count = 0;
	const size_t rootCount = sImageRoots.size();
	if ( rootCount > 1 ) {
		for(size_t i=1; i < rootCount; ++i) {
			sImageRoots[i]->runInitializers(gLinkContext, initializerTimes[0]);
		}
	}
	
	// run initializers for main executable and everything it brings up 
	sMainExecutable->runInitializers(gLinkContext, initializerTimes[0]);
	
	// register cxa_atexit() handler to run static terminators in all loaded images when this process exits
	if ( gLibSystemHelpers != NULL ) 
		(*gLibSystemHelpers->cxa_atexit)(&runAllStaticTerminators, NULL, NULL);

	// dump info if requested
	if ( sEnv.DYLD_PRINT_STATISTICS )
		ImageLoader::printStatistics((unsigned int)allImagesCount(), initializerTimes[0]);
	if ( sEnv.DYLD_PRINT_STATISTICS_DETAILS )
		ImageLoaderMachO::printStatisticsDetails((unsigned int)allImagesCount(), initializerTimes[0]);
}

bool mainExecutablePrebound()
{
	return sMainExecutable->usablePrebinding(gLinkContext);
}

ImageLoader* mainExecutable()
{
	return sMainExecutable;
}




#if SUPPORT_VERSIONED_PATHS

// forward reference
static bool getDylibVersionAndInstallname(const char* dylibPath, uint32_t* version, char* installName);


//
// Examines a dylib file and if its current_version is newer than the installed
// dylib at its install_name, then add the dylib file to sDylibOverrides.
//
static void checkDylibOverride(const char* dylibFile)
{
	//dyld::log("checkDylibOverride('%s')\n", dylibFile);
	uint32_t altVersion;
 	char sysInstallName[PATH_MAX];
	if ( getDylibVersionAndInstallname(dylibFile, &altVersion, sysInstallName) && (sysInstallName[0] =='/') ) {
		//dyld::log("%s has version 0x%08X and install name %s\n", dylibFile, altVersion, sysInstallName);
		uint32_t sysVersion;
		if ( getDylibVersionAndInstallname(sysInstallName, &sysVersion, NULL) ) {
			//dyld::log("%s has version 0x%08X\n", sysInstallName, sysVersion);
			if ( altVersion > sysVersion ) {
				//dyld::log("override found: %s -> %s\n", sysInstallName, dylibFile);
				// see if there already is an override for this dylib
				bool entryExists = false;
				for (std::vector<DylibOverride>::iterator it = sDylibOverrides.begin(); it != sDylibOverrides.end(); ++it) {
					if ( strcmp(it->installName, sysInstallName) == 0 ) {
						entryExists = true;
						uint32_t prevVersion;
						if ( getDylibVersionAndInstallname(it->override, &prevVersion, NULL) ) {
							if ( altVersion > prevVersion ) {
								// found an even newer override
								free((void*)(it->override));
								char resolvedPath[PATH_MAX];
								if ( realpath(dylibFile, resolvedPath) != NULL )
									it->override = strdup(resolvedPath);
								else
									it->override = strdup(dylibFile);
								break;
							}
						}
					}
				}
				if ( ! entryExists ) {
					DylibOverride entry;
					entry.installName = strdup(sysInstallName);
					char resolvedPath[PATH_MAX];
					if ( realpath(dylibFile, resolvedPath) != NULL )
						entry.override = strdup(resolvedPath);
					else
						entry.override = strdup(dylibFile);
					sDylibOverrides.push_back(entry);
					//dyld::log("added override: %s -> %s\n", entry.installName, entry.override);
				}
			}
		}
	}
	
}

static void checkDylibOverridesInDir(const char* dirPath)
{
	//dyld::log("checkDylibOverridesInDir('%s')\n", dirPath);
	char dylibPath[PATH_MAX];
	long dirPathLen = strlcpy(dylibPath, dirPath, PATH_MAX-1);
	if ( dirPathLen >= PATH_MAX )
		return;
	DIR* dirp = opendir(dirPath);
	if ( dirp != NULL) {
		dirent entry;
		dirent* entp = NULL;
		while ( readdir_r(dirp, &entry, &entp) == 0 ) {
			if ( entp == NULL )
				break;
			if ( entp->d_type != DT_REG ) 
				continue;
			dylibPath[dirPathLen] = '/';
			dylibPath[dirPathLen+1] = '\0';
			if ( strlcat(dylibPath, entp->d_name, PATH_MAX) >= PATH_MAX )
				continue;
			checkDylibOverride(dylibPath);
		}
		closedir(dirp);
	}
}


static void checkFrameworkOverridesInDir(const char* dirPath)
{
	//dyld::log("checkFrameworkOverridesInDir('%s')\n", dirPath);
	char frameworkPath[PATH_MAX];
	long dirPathLen = strlcpy(frameworkPath, dirPath, PATH_MAX-1);
	if ( dirPathLen >= PATH_MAX )
		return;
	DIR* dirp = opendir(dirPath);
	if ( dirp != NULL) {
		dirent entry;
		dirent* entp = NULL;
		while ( readdir_r(dirp, &entry, &entp) == 0 ) {
			if ( entp == NULL )
				break;
			if ( entp->d_type != DT_DIR ) 
				continue;
			frameworkPath[dirPathLen] = '/';
			frameworkPath[dirPathLen+1] = '\0';
			int dirNameLen = (int)strlen(entp->d_name);
			if ( dirNameLen < 11 )
				continue;
			if ( strcmp(&entp->d_name[dirNameLen-10], ".framework") != 0 )
				continue;
			if ( strlcat(frameworkPath, entp->d_name, PATH_MAX) >= PATH_MAX )
				continue;
			if ( strlcat(frameworkPath, "/", PATH_MAX) >= PATH_MAX )
				continue;
			if ( strlcat(frameworkPath, entp->d_name, PATH_MAX) >= PATH_MAX )
				continue;
			frameworkPath[strlen(frameworkPath)-10] = '\0';
			checkDylibOverride(frameworkPath);
		}
		closedir(dirp);
	}
}
#endif // SUPPORT_VERSIONED_PATHS


//
// Turns a colon separated list of strings into a NULL terminated array 
// of string pointers. If mainExecutableDir param is not NULL,
// substitutes @loader_path with main executable's dir.
//
static const char** parseColonList(const char* list, const char* mainExecutableDir)
{
	static const char* sEmptyList[] = { NULL };

	if ( list[0] == '\0' ) 
		return sEmptyList;
	
	int colonCount = 0;
	for(const char* s=list; *s != '\0'; ++s) {
		if (*s == ':') 
			++colonCount;
	}
	
	int index = 0;
	const char* start = list;
	char** result = new char*[colonCount+2];
	for(const char* s=list; *s != '\0'; ++s) {
		if (*s == ':') {
			size_t len = s-start;
			if ( (mainExecutableDir != NULL) && (strncmp(start, "@loader_path/", 13) == 0) ) {
#if __MAC_OS_X_VERSION_MIN_REQUIRED
				if ( gLinkContext.processIsRestricted ) {
					dyld::log("dyld: warning: @loader_path/ ignored in restricted process\n");
					continue;
				}
#endif
				size_t mainExecDirLen = strlen(mainExecutableDir);
				char* str = new char[mainExecDirLen+len+1];
				strcpy(str, mainExecutableDir);
				strlcat(str, &start[13], mainExecDirLen+len+1);
				str[mainExecDirLen+len-13] = '\0';
				start = &s[1];
				result[index++] = str;
			}
			else if ( (mainExecutableDir != NULL) && (strncmp(start, "@executable_path/", 17) == 0) ) {
#if __MAC_OS_X_VERSION_MIN_REQUIRED
				if ( gLinkContext.processIsRestricted ) {
					dyld::log("dyld: warning: @executable_path/ ignored in restricted process\n");
					continue;
				}
#endif
				size_t mainExecDirLen = strlen(mainExecutableDir);
				char* str = new char[mainExecDirLen+len+1];
				strcpy(str, mainExecutableDir);
				strlcat(str, &start[17], mainExecDirLen+len+1);
				str[mainExecDirLen+len-17] = '\0';
				start = &s[1];
				result[index++] = str;
			}
			else {
				char* str = new char[len+1];
				strncpy(str, start, len);
				str[len] = '\0';
				start = &s[1];
				result[index++] = str;
			}
		}
	}
	size_t len = strlen(start);
	if ( (mainExecutableDir != NULL) && (strncmp(start, "@loader_path/", 13) == 0) ) {
#if __MAC_OS_X_VERSION_MIN_REQUIRED
		if ( gLinkContext.processIsRestricted ) {
			dyld::log("dyld: warning: @loader_path/ ignored in restricted process\n");
		}
		else
#endif
		{
			size_t mainExecDirLen = strlen(mainExecutableDir);
			char* str = new char[mainExecDirLen+len+1];
			strcpy(str, mainExecutableDir);
			strlcat(str, &start[13], mainExecDirLen+len+1);
			str[mainExecDirLen+len-13] = '\0';
			result[index++] = str;
		}
	}
	else if ( (mainExecutableDir != NULL) && (strncmp(start, "@executable_path/", 17) == 0) ) {
#if __MAC_OS_X_VERSION_MIN_REQUIRED
		if ( gLinkContext.processIsRestricted ) {
			dyld::log("dyld: warning: @executable_path/ ignored in restricted process\n");
		}
		else
#endif
		{
			size_t mainExecDirLen = strlen(mainExecutableDir);
			char* str = new char[mainExecDirLen+len+1];
			strcpy(str, mainExecutableDir);
			strlcat(str, &start[17], mainExecDirLen+len+1);
			str[mainExecDirLen+len-17] = '\0';
			result[index++] = str;
		}
	}
	else {
		char* str = new char[len+1];
		strcpy(str, start);
		result[index++] = str;
	}
	result[index] = NULL;
	
	//dyld::log("parseColonList(%s)\n", list);
	//for(int i=0; result[i] != NULL; ++i)
	//	dyld::log("  %s\n", result[i]);
	return (const char**)result;
}

static void	appendParsedColonList(const char* list, const char* mainExecutableDir, const char* const ** storage)
{
	const char** newlist = parseColonList(list, mainExecutableDir);
	if ( *storage == NULL ) {
		// first time, just set
		*storage = newlist;
	}
	else {
		// need to append to existing list
		const char* const* existing = *storage;
		int count = 0;
		for(int i=0; existing[i] != NULL; ++i)
			++count;
		for(int i=0; newlist[i] != NULL; ++i)
			++count;
		const char** combinedList = new const char*[count+2];
		int index = 0;
		for(int i=0; existing[i] != NULL; ++i)
			combinedList[index++] = existing[i];
		for(int i=0; newlist[i] != NULL; ++i)
			combinedList[index++] = newlist[i];
		combinedList[index] = NULL;
		// leak old arrays
		*storage = combinedList;
	}
}

#if __MAC_OS_X_VERSION_MIN_REQUIRED
static void paths_expand_roots(const char **paths, const char *key, const char *val)
{
// 	assert(val != NULL);
// 	assert(paths != NULL);
	if(NULL != key) {
		size_t keyLen = strlen(key);
		for(int i=0; paths[i] != NULL; ++i) {
			if ( strncmp(paths[i], key, keyLen) == 0 ) {
				char* newPath = new char[strlen(val) + (strlen(paths[i]) - keyLen) + 1];
				strcpy(newPath, val);
				strcat(newPath, &paths[i][keyLen]);
				paths[i] = newPath;
			}
		}
	}
	return;
}

static void removePathWithPrefix(const char* paths[], const char* prefix)
{
    size_t prefixLen = strlen(prefix);
    int skip = 0;
    int i;
    for(i = 0; paths[i] != NULL; ++i) {
        if ( strncmp(paths[i], prefix, prefixLen) == 0 )
            ++skip;
        else
            paths[i-skip] = paths[i];
    }
    paths[i-skip] = NULL;
}
#endif


#if 0
static void paths_dump(const char **paths)
{
//   assert(paths != NULL);
  const char **strs = paths;
  while(*strs != NULL)
  {
    dyld::log("\"%s\"\n", *strs);
    strs++;
  }
  return;
}
#endif

static void printOptions(const char* argv[])
{
	uint32_t i = 0;
	while ( NULL != argv[i] ) {
		dyld::log("opt[%i] = \"%s\"\n", i, argv[i]);
		i++;
	}
}

static void printEnvironmentVariables(const char* envp[])
{
	while ( NULL != *envp ) {
		dyld::log("%s\n", *envp);
		envp++;
	}
}

void processDyldEnvironmentVariable(const char* key, const char* value, const char* mainExecutableDir)
{
	if ( strcmp(key, "DYLD_FRAMEWORK_PATH") == 0 ) {
		appendParsedColonList(value, mainExecutableDir, &sEnv.DYLD_FRAMEWORK_PATH);
	}
	else if ( strcmp(key, "DYLD_FALLBACK_FRAMEWORK_PATH") == 0 ) {
		appendParsedColonList(value, mainExecutableDir, &sEnv.DYLD_FALLBACK_FRAMEWORK_PATH);
	}
	else if ( strcmp(key, "DYLD_LIBRARY_PATH") == 0 ) {
		appendParsedColonList(value, mainExecutableDir, &sEnv.DYLD_LIBRARY_PATH);
	}
	else if ( strcmp(key, "DYLD_FALLBACK_LIBRARY_PATH") == 0 ) {
		appendParsedColonList(value, mainExecutableDir, &sEnv.DYLD_FALLBACK_LIBRARY_PATH);
	}
#if SUPPORT_ROOT_PATH
	else if ( (strcmp(key, "DYLD_ROOT_PATH") == 0) || (strcmp(key, "DYLD_PATHS_ROOT") == 0) ) {
		if ( strcmp(value, "/") != 0 ) {
			gLinkContext.rootPaths = parseColonList(value, mainExecutableDir);
			for (int i=0; gLinkContext.rootPaths[i] != NULL; ++i) {
				if ( gLinkContext.rootPaths[i][0] != '/' ) {
					dyld::warn("DYLD_ROOT_PATH not used because it contains a non-absolute path\n");
					gLinkContext.rootPaths = NULL;
					break;
				}
			}
		}
	}
#endif
	else if ( strcmp(key, "DYLD_IMAGE_SUFFIX") == 0 ) {
		gLinkContext.imageSuffix = value;
	}
	else if ( strcmp(key, "DYLD_INSERT_LIBRARIES") == 0 ) {
		sEnv.DYLD_INSERT_LIBRARIES = parseColonList(value, NULL);
#if SUPPORT_ACCELERATE_TABLES
		sDisableAcceleratorTables = true;
#endif
	}
	else if ( strcmp(key, "DYLD_PRINT_OPTS") == 0 ) {
		sEnv.DYLD_PRINT_OPTS = true;
	}
	else if ( strcmp(key, "DYLD_PRINT_ENV") == 0 ) {
		sEnv.DYLD_PRINT_ENV = true;
	}
	else if ( strcmp(key, "DYLD_DISABLE_DOFS") == 0 ) {
		sEnv.DYLD_DISABLE_DOFS = true;
	}
	else if ( strcmp(key, "DYLD_DISABLE_PREFETCH") == 0 ) {
		gLinkContext.preFetchDisabled = true;
	}
	else if ( strcmp(key, "DYLD_PRINT_LIBRARIES") == 0 ) {
		gLinkContext.verboseLoading = true;
	}
	else if ( strcmp(key, "DYLD_PRINT_LIBRARIES_POST_LAUNCH") == 0 ) {
		sEnv.DYLD_PRINT_LIBRARIES_POST_LAUNCH = true;
	}
	else if ( strcmp(key, "DYLD_BIND_AT_LAUNCH") == 0 ) {
		sEnv.DYLD_BIND_AT_LAUNCH = true;
	}
	else if ( strcmp(key, "DYLD_FORCE_FLAT_NAMESPACE") == 0 ) {
		gLinkContext.bindFlat = true;
	}
	else if ( strcmp(key, "DYLD_NEW_LOCAL_SHARED_REGIONS") == 0 ) {
		// ignore, no longer relevant but some scripts still set it
	}
	else if ( strcmp(key, "DYLD_NO_FIX_PREBINDING") == 0 ) {
	}
	else if ( strcmp(key, "DYLD_PREBIND_DEBUG") == 0 ) {
		gLinkContext.verbosePrebinding = true;
	}
	else if ( strcmp(key, "DYLD_PRINT_INITIALIZERS") == 0 ) {
		gLinkContext.verboseInit = true;
	}
	else if ( strcmp(key, "DYLD_PRINT_DOFS") == 0 ) {
		gLinkContext.verboseDOF = true;
	}
	else if ( strcmp(key, "DYLD_PRINT_STATISTICS") == 0 ) {
		sEnv.DYLD_PRINT_STATISTICS = true;
#if __IPHONE_OS_VERSION_MIN_REQUIRED && !TARGET_IPHONE_SIMULATOR
		// <rdar://problem/26614838> DYLD_PRINT_STATISTICS no longer logs to xcode console for device apps
		sForceStderr = true;
#endif
	}
	else if ( strcmp(key, "DYLD_PRINT_TO_STDERR") == 0 ) {
#if __IPHONE_OS_VERSION_MIN_REQUIRED && !TARGET_IPHONE_SIMULATOR
		// <rdar://problem/26633440> DYLD_PRINT_STATISTICS no longer logs to xcode console for device apps
		sForceStderr = true;
#endif
	}
	else if ( strcmp(key, "DYLD_PRINT_STATISTICS_DETAILS") == 0 ) {
		sEnv.DYLD_PRINT_STATISTICS_DETAILS = true;
	}
	else if ( strcmp(key, "DYLD_PRINT_SEGMENTS") == 0 ) {
		gLinkContext.verboseMapping = true;
	}
	else if ( strcmp(key, "DYLD_PRINT_BINDINGS") == 0 ) {
		gLinkContext.verboseBind = true;
	}
	else if ( strcmp(key, "DYLD_PRINT_WEAK_BINDINGS") == 0 ) {
		gLinkContext.verboseWeakBind = true;
	}
	else if ( strcmp(key, "DYLD_PRINT_REBASINGS") == 0 ) {
		gLinkContext.verboseRebase = true;
	}
	else if ( strcmp(key, "DYLD_PRINT_APIS") == 0 ) {
		gLogAPIs = true;
	}
#if SUPPORT_ACCELERATE_TABLES
	else if ( strcmp(key, "DYLD_PRINT_APIS_APP") == 0 ) {
		gLogAppAPIs = true;
	}
#endif
	else if ( strcmp(key, "DYLD_PRINT_WARNINGS") == 0 ) {
		gLinkContext.verboseWarnings = true;
	}
	else if ( strcmp(key, "DYLD_PRINT_RPATHS") == 0 ) {
		gLinkContext.verboseRPaths = true;
	}
	else if ( strcmp(key, "DYLD_PRINT_CS_NOTIFICATIONS") == 0 ) {
		sEnv.DYLD_PRINT_CS_NOTIFICATIONS = true;
	}
	else if ( strcmp(key, "DYLD_PRINT_INTERPOSING") == 0 ) {
		gLinkContext.verboseInterposing = true;
	}
	else if ( strcmp(key, "DYLD_PRINT_CODE_SIGNATURES") == 0 ) {
		gLinkContext.verboseCodeSignatures = true;
	}
	else if ( strcmp(key, "DYLD_SHARED_REGION") == 0 ) {
		if ( strcmp(value, "private") == 0 ) {
			gLinkContext.sharedRegionMode = ImageLoader::kUsePrivateSharedRegion;
		}
		else if ( strcmp(value, "avoid") == 0 ) {
			gLinkContext.sharedRegionMode = ImageLoader::kDontUseSharedRegion;
		}
		else if ( strcmp(value, "use") == 0 ) {
			gLinkContext.sharedRegionMode = ImageLoader::kUseSharedRegion;
		}
		else if ( value[0] == '\0' ) {
			gLinkContext.sharedRegionMode = ImageLoader::kUseSharedRegion;
		}
		else {
			dyld::warn("unknown option to DYLD_SHARED_REGION.  Valid options are: use, private, avoid\n");
		}
	}
#if DYLD_SHARED_CACHE_SUPPORT
	else if ( strcmp(key, "DYLD_SHARED_CACHE_DIR") == 0 ) {
        sSharedCacheDir = value;
	}
	else if ( strcmp(key, "DYLD_SHARED_CACHE_DONT_VALIDATE") == 0 ) {
		sSharedCacheIgnoreInodeAndTimeStamp = true;
	}
#endif
	else if ( strcmp(key, "DYLD_IGNORE_PREBINDING") == 0 ) {
		if ( strcmp(value, "all") == 0 ) {
			gLinkContext.prebindUsage = ImageLoader::kUseNoPrebinding;
		}
		else if ( strcmp(value, "app") == 0 ) {
			gLinkContext.prebindUsage = ImageLoader::kUseAllButAppPredbinding;
		}
		else if ( strcmp(value, "nonsplit") == 0 ) {
			gLinkContext.prebindUsage = ImageLoader::kUseSplitSegPrebinding;
		}
		else if ( value[0] == '\0' ) {
			gLinkContext.prebindUsage = ImageLoader::kUseSplitSegPrebinding;
		}
		else {
			dyld::warn("unknown option to DYLD_IGNORE_PREBINDING.  Valid options are: all, app, nonsplit\n");
		}
	}
#if SUPPORT_VERSIONED_PATHS
	else if ( strcmp(key, "DYLD_VERSIONED_LIBRARY_PATH") == 0 ) {
		appendParsedColonList(value, mainExecutableDir, &sEnv.DYLD_VERSIONED_LIBRARY_PATH);
	#if SUPPORT_ACCELERATE_TABLES
		sDisableAcceleratorTables = true;
	#endif
	}
	else if ( strcmp(key, "DYLD_VERSIONED_FRAMEWORK_PATH") == 0 ) {
		appendParsedColonList(value, mainExecutableDir, &sEnv.DYLD_VERSIONED_FRAMEWORK_PATH);
	#if SUPPORT_ACCELERATE_TABLES
		sDisableAcceleratorTables = true;
	#endif
	}
#endif
#if !TARGET_IPHONE_SIMULATOR
	else if ( (strcmp(key, "DYLD_PRINT_TO_FILE") == 0) && (mainExecutableDir == NULL) ) {
		int fd = open(value, O_WRONLY | O_CREAT | O_APPEND, 0644);
		if ( fd != -1 ) {
			sLogfile = fd;
			sLogToFile = true;
		}
		else {
			dyld::log("dyld: could not open DYLD_PRINT_TO_FILE='%s', errno=%d\n", value, errno);
		}
	}
#endif
	else {
		dyld::warn("unknown environment variable: %s\n", key);
	}
}


#if SUPPORT_LC_DYLD_ENVIRONMENT
static void checkLoadCommandEnvironmentVariables()
{
	// <rdar://problem/8440934> Support augmenting dyld environment variables in load commands
	const uint32_t cmd_count = sMainExecutableMachHeader->ncmds;
	const struct load_command* const cmds = (struct load_command*)(((char*)sMainExecutableMachHeader)+sizeof(macho_header));
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_DYLD_ENVIRONMENT:
			{
				const struct dylinker_command* envcmd = (struct dylinker_command*)cmd;
				const char* keyEqualsValue = (char*)envcmd + envcmd->name.offset;
				char mainExecutableDir[strlen(sExecPath)+2];
				strcpy(mainExecutableDir, sExecPath);
				char* lastSlash = strrchr(mainExecutableDir, '/');
				if ( lastSlash != NULL)
					lastSlash[1] = '\0';
				// only process variables that start with DYLD_ and end in _PATH
				if ( (strncmp(keyEqualsValue, "DYLD_", 5) == 0) ) {
					const char* equals = strchr(keyEqualsValue, '=');
					if ( equals != NULL ) {
						if ( strncmp(&equals[-5], "_PATH", 5) == 0 ) {
							const char* value = &equals[1];
							const size_t keyLen = equals-keyEqualsValue;
							// <rdar://problem/22799635> don't let malformed load command overflow stack
							if ( keyLen < 40 ) {
								char key[keyLen+1];
								strncpy(key, keyEqualsValue, keyLen);
								key[keyLen] = '\0';
								//dyld::log("processing: %s\n", keyEqualsValue);
								//dyld::log("mainExecutableDir: %s\n", mainExecutableDir);
								processDyldEnvironmentVariable(key, value, mainExecutableDir);
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
#endif // SUPPORT_LC_DYLD_ENVIRONMENT	

	
static bool hasCodeSignatureLoadCommand(const macho_header* mh)
{
	const uint32_t cmd_count = mh->ncmds;
	const struct load_command* const cmds = (struct load_command*)(((char*)mh)+sizeof(macho_header));
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		if (cmd->cmd == LC_CODE_SIGNATURE) 
			return true;
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	return false;
}
	

#if SUPPORT_VERSIONED_PATHS
static void checkVersionedPaths()
{
	// search DYLD_VERSIONED_LIBRARY_PATH directories for dylibs and check if they are newer
	if ( sEnv.DYLD_VERSIONED_LIBRARY_PATH != NULL ) {
		for(const char* const* lp = sEnv.DYLD_VERSIONED_LIBRARY_PATH; *lp != NULL; ++lp) {
			checkDylibOverridesInDir(*lp);
		}
	}
	
	// search DYLD_VERSIONED_FRAMEWORK_PATH directories for dylibs and check if they are newer
	if ( sEnv.DYLD_VERSIONED_FRAMEWORK_PATH != NULL ) {
		for(const char* const* fp = sEnv.DYLD_VERSIONED_FRAMEWORK_PATH; *fp != NULL; ++fp) {
			checkFrameworkOverridesInDir(*fp);
		}
	}
}
#endif	


#if __MAC_OS_X_VERSION_MIN_REQUIRED
//
// For security, setuid programs ignore DYLD_* environment variables.
// Additionally, the DYLD_* enviroment variables are removed
// from the environment, so that any child processes don't see them.
//
static void pruneEnvironmentVariables(const char* envp[], const char*** applep)
{
#if SUPPORT_LC_DYLD_ENVIRONMENT
	checkLoadCommandEnvironmentVariables();
#endif

	// delete all DYLD_* and LD_LIBRARY_PATH environment variables
	int removedCount = 0;
	const char** d = envp;
	for(const char** s = envp; *s != NULL; s++) {
	    if ( (strncmp(*s, "DYLD_", 5) != 0) && (strncmp(*s, "LD_LIBRARY_PATH=", 16) != 0) ) {
			*d++ = *s;
		}
		else {
			++removedCount;
		}
	}
	*d++ = NULL;
	// slide apple parameters
	if ( removedCount > 0 ) {
		*applep = d;
		do {
			*d = d[removedCount];
		} while ( *d++ != NULL );
		for(int i=0; i < removedCount; ++i)
			*d++ = NULL;
	}
	
	// disable framework and library fallback paths for setuid binaries rdar://problem/4589305
	sEnv.DYLD_FALLBACK_FRAMEWORK_PATH = NULL;
	sEnv.DYLD_FALLBACK_LIBRARY_PATH = NULL;

	if ( removedCount > 0 )
		strlcat(sLoadingCrashMessage, ", ignoring DYLD_* env vars", sizeof(sLoadingCrashMessage));
}
#endif

static void defaultUninitializedFallbackPaths(const char* envp[])
{
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	if ( gLinkContext.processIsRestricted ) {
		sEnv.DYLD_FALLBACK_FRAMEWORK_PATH = sRestrictedFrameworkFallbackPaths;
		sEnv.DYLD_FALLBACK_LIBRARY_PATH   = sRestrictedLibraryFallbackPaths;
		return;
	}

	// default value for DYLD_FALLBACK_FRAMEWORK_PATH, if not set in environment
	const char* home = _simple_getenv(envp, "HOME");;
	if ( sEnv.DYLD_FALLBACK_FRAMEWORK_PATH == NULL ) {
		const char** fpaths = sFrameworkFallbackPaths;
		if ( home == NULL )
			removePathWithPrefix(fpaths, "$HOME");
		else
			paths_expand_roots(fpaths, "$HOME", home);
		sEnv.DYLD_FALLBACK_FRAMEWORK_PATH = fpaths;
	}

    // default value for DYLD_FALLBACK_LIBRARY_PATH, if not set in environment
	if ( sEnv.DYLD_FALLBACK_LIBRARY_PATH == NULL ) {
		const char** lpaths = sLibraryFallbackPaths;
		if ( home == NULL )
			removePathWithPrefix(lpaths, "$HOME");
		else
			paths_expand_roots(lpaths, "$HOME", home);
		sEnv.DYLD_FALLBACK_LIBRARY_PATH = lpaths;
	}
#else
	if ( sEnv.DYLD_FALLBACK_FRAMEWORK_PATH == NULL )
		sEnv.DYLD_FALLBACK_FRAMEWORK_PATH = sFrameworkFallbackPaths;

	if ( sEnv.DYLD_FALLBACK_LIBRARY_PATH == NULL )
		sEnv.DYLD_FALLBACK_LIBRARY_PATH = sLibraryFallbackPaths;
#endif
}


static void checkEnvironmentVariables(const char* envp[])
{
	if ( sEnvMode == envNone )
		return;
	const char** p;
	for(p = envp; *p != NULL; p++) {
		const char* keyEqualsValue = *p;
	    if ( strncmp(keyEqualsValue, "DYLD_", 5) == 0 ) {
			const char* equals = strchr(keyEqualsValue, '=');
			if ( equals != NULL ) {
				strlcat(sLoadingCrashMessage, "\n", sizeof(sLoadingCrashMessage));
				strlcat(sLoadingCrashMessage, keyEqualsValue, sizeof(sLoadingCrashMessage));
				const char* value = &equals[1];
				const size_t keyLen = equals-keyEqualsValue;
				char key[keyLen+1];
				strncpy(key, keyEqualsValue, keyLen);
				key[keyLen] = '\0';
				if ( (sEnvMode == envPrintOnly) && (strncmp(key, "DYLD_PRINT_", 11) != 0) )
					continue;
				processDyldEnvironmentVariable(key, value, NULL);
			}
		}
		else if ( strncmp(keyEqualsValue, "LD_LIBRARY_PATH=", 16) == 0 ) {
			const char* path = &keyEqualsValue[16];
			sEnv.LD_LIBRARY_PATH = parseColonList(path, NULL);
		}
	}

#if SUPPORT_LC_DYLD_ENVIRONMENT
	checkLoadCommandEnvironmentVariables();
#endif // SUPPORT_LC_DYLD_ENVIRONMENT	
	
#if SUPPORT_ROOT_PATH
	// <rdar://problem/11281064> DYLD_IMAGE_SUFFIX and DYLD_ROOT_PATH cannot be used together
	if ( (gLinkContext.imageSuffix != NULL) && (gLinkContext.rootPaths != NULL) ) {
		dyld::warn("Ignoring DYLD_IMAGE_SUFFIX because DYLD_ROOT_PATH is used.\n");
		gLinkContext.imageSuffix = NULL;
	}
#endif
}

#if __x86_64__ && DYLD_SHARED_CACHE_SUPPORT
static bool isGCProgram(const macho_header* mh, uintptr_t slide)
{
	const uint32_t cmd_count = mh->ncmds;
	const struct load_command* const cmds = (struct load_command*)(((char*)mh)+sizeof(macho_header));
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_SEGMENT_COMMAND:
			{
				const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
				if (strcmp(seg->segname, "__DATA") == 0) {
					const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
					const struct macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
					for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
						if (strncmp(sect->sectname, "__objc_imageinfo", 16) == 0) {
							const uint32_t*  objcInfo = (uint32_t*)(sect->addr + slide);
							return (objcInfo[1] & 6); // 6 = (OBJC_IMAGE_SUPPORTS_GC | OBJC_IMAGE_REQUIRES_GC)
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
#endif

static void getHostInfo(const macho_header* mainExecutableMH, uintptr_t mainExecutableSlide)
{
#if CPU_SUBTYPES_SUPPORTED
#if __ARM_ARCH_7K__
	sHostCPU		= CPU_TYPE_ARM;
	sHostCPUsubtype = CPU_SUBTYPE_ARM_V7K;
#elif __ARM_ARCH_7A__
	sHostCPU		= CPU_TYPE_ARM;
	sHostCPUsubtype = CPU_SUBTYPE_ARM_V7;
#elif __ARM_ARCH_6K__
	sHostCPU		= CPU_TYPE_ARM;
	sHostCPUsubtype = CPU_SUBTYPE_ARM_V6;
#elif __ARM_ARCH_7F__
	sHostCPU		= CPU_TYPE_ARM;
	sHostCPUsubtype = CPU_SUBTYPE_ARM_V7F;
#elif __ARM_ARCH_7S__
	sHostCPU		= CPU_TYPE_ARM;
	sHostCPUsubtype = CPU_SUBTYPE_ARM_V7S;
#else
	struct host_basic_info info;
	mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;
	mach_port_t hostPort = mach_host_self();
	kern_return_t result = host_info(hostPort, HOST_BASIC_INFO, (host_info_t)&info, &count);
	if ( result != KERN_SUCCESS )
		throw "host_info() failed";
	sHostCPU		= info.cpu_type;
	sHostCPUsubtype = info.cpu_subtype;
	mach_port_deallocate(mach_task_self(), hostPort);
  #if __x86_64__
	#if DYLD_SHARED_CACHE_SUPPORT
	  sHaswell = (sHostCPUsubtype == CPU_SUBTYPE_X86_64_H);
	  // <rdar://problem/18528074> x86_64h: Fall back to the x86_64 slice if an app requires GC.
	  if ( sHaswell ) {
		if ( isGCProgram(mainExecutableMH, mainExecutableSlide) ) {
			// When running a GC program on a haswell machine, don't use and 'h slices
			sHostCPUsubtype = CPU_SUBTYPE_X86_64_ALL;
			sHaswell = false;
			gLinkContext.sharedRegionMode = ImageLoader::kDontUseSharedRegion;
		}
	  }
	#endif
  #endif
#endif
#endif
}

static void checkSharedRegionDisable()
{
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	// if main executable has segments that overlap the shared region, 
	// then disable using the shared region
	if ( sMainExecutable->overlapsWithAddressRange((void*)(uintptr_t)SHARED_REGION_BASE, (void*)(uintptr_t)(SHARED_REGION_BASE + SHARED_REGION_SIZE)) ) {
		gLinkContext.sharedRegionMode = ImageLoader::kDontUseSharedRegion;
		if ( gLinkContext.verboseMapping )
			dyld::warn("disabling shared region because main executable overlaps\n");
	}
#if __i386__
	if ( gLinkContext.processIsRestricted ) {
		// <rdar://problem/15280847> use private or no shared region for suid processes
		gLinkContext.sharedRegionMode = ImageLoader::kUsePrivateSharedRegion;
	}
#endif
#endif
	// iPhoneOS cannot run without shared region
}

bool validImage(const ImageLoader* possibleImage)
{
    const size_t imageCount = sAllImages.size();
    for(size_t i=0; i < imageCount; ++i) {
        if ( possibleImage == sAllImages[i] ) {
            return true;
        }
    }
	return false;
}

uint32_t getImageCount()
{
	return (uint32_t)sAllImages.size();
}

ImageLoader* getIndexedImage(unsigned int index)
{
	if ( index < sAllImages.size() )
		return sAllImages[index];
	return NULL;
}

ImageLoader* findImageByMachHeader(const struct mach_header* target)
{
	return findMappedRange((uintptr_t)target);
}


ImageLoader* findImageContainingAddress(const void* addr)
{
  #if SUPPORT_ACCELERATE_TABLES
	if ( sAllCacheImagesProxy != NULL ) {
		const mach_header* mh;
		const char* path;
		unsigned index;
		if ( sAllCacheImagesProxy->addressInCache(addr, &mh, &path, &index) )
			return sAllCacheImagesProxy;
	}
  #endif
	return findMappedRange((uintptr_t)addr);
}


ImageLoader* findImageContainingSymbol(const void* symbol)
{
	for (std::vector<ImageLoader*>::iterator it=sAllImages.begin(); it != sAllImages.end(); it++) {
		ImageLoader* anImage = *it;
		if ( anImage->containsSymbol(symbol) )
			return anImage;
	}
	return NULL;
}



void forEachImageDo( void (*callback)(ImageLoader*, void* userData), void* userData)
{
	const size_t imageCount = sAllImages.size();
	for(size_t i=0; i < imageCount; ++i) {
		ImageLoader* anImage = sAllImages[i];
		(*callback)(anImage, userData);
	}
}

ImageLoader* findLoadedImage(const struct stat& stat_buf)
{
	const size_t imageCount = sAllImages.size();
	for(size_t i=0; i < imageCount; ++i){
		ImageLoader* anImage = sAllImages[i];
		if ( anImage->statMatch(stat_buf) )
			return anImage;
	}
	return NULL;
}

// based on ANSI-C strstr()
static const char* strrstr(const char* str, const char* sub) 
{
	const size_t sublen = strlen(sub);
	for(const char* p = &str[strlen(str)]; p != str; --p) {
		if ( strncmp(p, sub, sublen) == 0 )
			return p;
	}
	return NULL;
}


//
// Find framework path
//
//  /path/foo.framework/foo								=>   foo.framework/foo	
//  /path/foo.framework/Versions/A/foo					=>   foo.framework/Versions/A/foo
//  /path/foo.framework/Frameworks/bar.framework/bar	=>   bar.framework/bar
//  /path/foo.framework/Libraries/bar.dylb				=>   NULL
//  /path/foo.framework/bar								=>   NULL
//
// Returns NULL if not a framework path
//
static const char* getFrameworkPartialPath(const char* path)
{
	const char* dirDot = strrstr(path, ".framework/");
	if ( dirDot != NULL ) {
		const char* dirStart = dirDot;
		for ( ; dirStart >= path; --dirStart) {
			if ( (*dirStart == '/') || (dirStart == path) ) {
				const char* frameworkStart = &dirStart[1];
				if ( dirStart == path )
					--frameworkStart;
				size_t len = dirDot - frameworkStart;
				char framework[len+1];
				strncpy(framework, frameworkStart, len);
				framework[len] = '\0';
				const char* leaf = strrchr(path, '/');
				if ( leaf != NULL ) {
					if ( strcmp(framework, &leaf[1]) == 0 ) {
						return frameworkStart;
					}
					if (  gLinkContext.imageSuffix != NULL ) {
						// some debug frameworks have install names that end in _debug
						if ( strncmp(framework, &leaf[1], len) == 0 ) {
							if ( strcmp( gLinkContext.imageSuffix, &leaf[len+1]) == 0 )
								return frameworkStart;
						}
					}
				}
			}
		}
	}
	return NULL;
}


static const char* getLibraryLeafName(const char* path)
{
	const char* start = strrchr(path, '/');
	if ( start != NULL )
		return &start[1];
	else
		return path;
}


// only for architectures that use cpu-sub-types
#if CPU_SUBTYPES_SUPPORTED 

const cpu_subtype_t CPU_SUBTYPE_END_OF_LIST = -1;


//
//	A fat file may contain multiple sub-images for the same CPU type.
//	In that case, dyld picks which sub-image to use by scanning a table
//	of preferred cpu-sub-types for the running cpu.  
//	
//	There is one row in the table for each cpu-sub-type on which dyld might run.
//  The first entry in a row is that cpu-sub-type.  It is followed by all
//	cpu-sub-types that can run on that cpu, if preferred order.  Each row ends with 
//	a "SUBTYPE_ALL" (to denote that images written to run on any cpu-sub-type are usable), 
//  followed by one or more CPU_SUBTYPE_END_OF_LIST to pad out this row.
//


#if __arm__
//      
//     ARM sub-type lists
//
const int kARM_RowCount = 8;
static const cpu_subtype_t kARM[kARM_RowCount][9] = { 

	// armv7f can run: v7f, v7, v6, v5, and v4
	{  CPU_SUBTYPE_ARM_V7F, CPU_SUBTYPE_ARM_V7, CPU_SUBTYPE_ARM_V6, CPU_SUBTYPE_ARM_V5TEJ, CPU_SUBTYPE_ARM_V4T, CPU_SUBTYPE_ARM_ALL, CPU_SUBTYPE_END_OF_LIST },

	// armv7k can run: v7k
	{  CPU_SUBTYPE_ARM_V7K, CPU_SUBTYPE_END_OF_LIST },

	// armv7s can run: v7s, v7, v7f, v7k, v6, v5, and v4
	{  CPU_SUBTYPE_ARM_V7S, CPU_SUBTYPE_ARM_V7, CPU_SUBTYPE_ARM_V7F, CPU_SUBTYPE_ARM_V6, CPU_SUBTYPE_ARM_V5TEJ, CPU_SUBTYPE_ARM_V4T, CPU_SUBTYPE_ARM_ALL, CPU_SUBTYPE_END_OF_LIST },

	// armv7 can run: v7, v6, v5, and v4
	{  CPU_SUBTYPE_ARM_V7, CPU_SUBTYPE_ARM_V6, CPU_SUBTYPE_ARM_V5TEJ, CPU_SUBTYPE_ARM_V4T, CPU_SUBTYPE_ARM_ALL, CPU_SUBTYPE_END_OF_LIST },
	
	// armv6 can run: v6, v5, and v4
	{  CPU_SUBTYPE_ARM_V6, CPU_SUBTYPE_ARM_V5TEJ, CPU_SUBTYPE_ARM_V4T, CPU_SUBTYPE_ARM_ALL, CPU_SUBTYPE_END_OF_LIST, CPU_SUBTYPE_END_OF_LIST },
	
	// xscale can run: xscale, v5, and v4
	{  CPU_SUBTYPE_ARM_XSCALE, CPU_SUBTYPE_ARM_V5TEJ, CPU_SUBTYPE_ARM_V4T, CPU_SUBTYPE_ARM_ALL, CPU_SUBTYPE_END_OF_LIST, CPU_SUBTYPE_END_OF_LIST },
	
	// armv5 can run: v5 and v4
	{  CPU_SUBTYPE_ARM_V5TEJ, CPU_SUBTYPE_ARM_V4T, CPU_SUBTYPE_ARM_ALL, CPU_SUBTYPE_END_OF_LIST, CPU_SUBTYPE_END_OF_LIST, CPU_SUBTYPE_END_OF_LIST },

	// armv4 can run: v4
	{  CPU_SUBTYPE_ARM_V4T, CPU_SUBTYPE_ARM_ALL, CPU_SUBTYPE_END_OF_LIST, CPU_SUBTYPE_END_OF_LIST, CPU_SUBTYPE_END_OF_LIST, CPU_SUBTYPE_END_OF_LIST },
};
#endif

#if __x86_64__
//      
//     x86_64 sub-type lists
//
const int kX86_64_RowCount = 2;
static const cpu_subtype_t kX86_64[kX86_64_RowCount][5] = {

	// x86_64h can run: x86_64h, x86_64h(lib), x86_64(lib), and x86_64
	{ CPU_SUBTYPE_X86_64_H, (cpu_subtype_t) CPU_SUBTYPE_LIB64|CPU_SUBTYPE_X86_64_H, (cpu_subtype_t) CPU_SUBTYPE_LIB64|CPU_SUBTYPE_X86_64_ALL, CPU_SUBTYPE_X86_64_ALL,  CPU_SUBTYPE_END_OF_LIST },

	// x86_64 can run: x86_64(lib) and x86_64
	{ CPU_SUBTYPE_X86_64_ALL, (cpu_subtype_t) CPU_SUBTYPE_LIB64|CPU_SUBTYPE_X86_64_ALL, CPU_SUBTYPE_END_OF_LIST },

};
#endif


// scan the tables above to find the cpu-sub-type-list for this machine
static const cpu_subtype_t* findCPUSubtypeList(cpu_type_t cpu, cpu_subtype_t subtype)
{
	switch (cpu) {
#if __arm__
		case CPU_TYPE_ARM:
			for (int i=0; i < kARM_RowCount ; ++i) {
				if ( kARM[i][0] == subtype )
					return kARM[i];
			}
			break;
#endif
#if __x86_64__
		case CPU_TYPE_X86_64:
			for (int i=0; i < kX86_64_RowCount ; ++i) {
				if ( kX86_64[i][0] == subtype )
					return kX86_64[i];
			}
			break;
#endif
	}
	return NULL;
}




// scan fat table-of-contents for best most preferred subtype
static bool fatFindBestFromOrderedList(cpu_type_t cpu, const cpu_subtype_t list[], const fat_header* fh, uint64_t* offset, uint64_t* len)
{
	const fat_arch* const archs = (fat_arch*)(((char*)fh)+sizeof(fat_header));
	for (uint32_t subTypeIndex=0; list[subTypeIndex] != CPU_SUBTYPE_END_OF_LIST; ++subTypeIndex) {
		for(uint32_t fatIndex=0; fatIndex < OSSwapBigToHostInt32(fh->nfat_arch); ++fatIndex) {
			if ( ((cpu_type_t)OSSwapBigToHostInt32(archs[fatIndex].cputype) == cpu) 
				&& (list[subTypeIndex] == (cpu_subtype_t)OSSwapBigToHostInt32(archs[fatIndex].cpusubtype)) ) {
				*offset = OSSwapBigToHostInt32(archs[fatIndex].offset);
				*len = OSSwapBigToHostInt32(archs[fatIndex].size);
				return true;
			}
		}
	}
	return false;
}

// scan fat table-of-contents for exact match of cpu and cpu-sub-type
static bool fatFindExactMatch(cpu_type_t cpu, cpu_subtype_t subtype, const fat_header* fh, uint64_t* offset, uint64_t* len)
{
	const fat_arch* archs = (fat_arch*)(((char*)fh)+sizeof(fat_header));
	for(uint32_t i=0; i < OSSwapBigToHostInt32(fh->nfat_arch); ++i) {
		if ( ((cpu_type_t)OSSwapBigToHostInt32(archs[i].cputype) == cpu)
			&& ((cpu_subtype_t)OSSwapBigToHostInt32(archs[i].cpusubtype) == subtype) ) {
			*offset = OSSwapBigToHostInt32(archs[i].offset);
			*len = OSSwapBigToHostInt32(archs[i].size);
			return true;
		}
	}
	return false;
}

// scan fat table-of-contents for image with matching cpu-type and runs-on-all-sub-types
static bool fatFindRunsOnAllCPUs(cpu_type_t cpu, const fat_header* fh, uint64_t* offset, uint64_t* len)
{
	const fat_arch* archs = (fat_arch*)(((char*)fh)+sizeof(fat_header));
	for(uint32_t i=0; i < OSSwapBigToHostInt32(fh->nfat_arch); ++i) {
		if ( (cpu_type_t)OSSwapBigToHostInt32(archs[i].cputype) == cpu) {
			switch (cpu) {
#if __arm__
				case CPU_TYPE_ARM:
					if ( (cpu_subtype_t)OSSwapBigToHostInt32(archs[i].cpusubtype) == CPU_SUBTYPE_ARM_ALL ) {
						*offset = OSSwapBigToHostInt32(archs[i].offset);
						*len = OSSwapBigToHostInt32(archs[i].size);
						return true;
					}
					break;
#endif
#if __x86_64__
				case CPU_TYPE_X86_64:
					if ( (cpu_subtype_t)OSSwapBigToHostInt32(archs[i].cpusubtype) == CPU_SUBTYPE_X86_64_ALL ) {
						*offset = OSSwapBigToHostInt32(archs[i].offset);
						*len = OSSwapBigToHostInt32(archs[i].size);
						return true;
					}
					break;
#endif
			}
		}
	}
	return false;
}

#endif // CPU_SUBTYPES_SUPPORTED


//
// Validate the fat_header and fat_arch array:
//
// 1) arch count would not cause array to extend past 4096 byte read buffer
// 2) no slice overlaps the fat_header and arch array
// 3) arch list does not contain duplicate cputype/cpusubtype tuples
// 4) arch list does not have two overlapping slices.
//
static bool fatValidate(const fat_header* fh)
{
	if ( fh->magic != OSSwapBigToHostInt32(FAT_MAGIC) )
		return false;

	// since only first 4096 bytes of file read, we can only handle up to 204 slices.
	const uint32_t sliceCount = OSSwapBigToHostInt32(fh->nfat_arch);
	if ( sliceCount > 204 )
		return false;

	// compare all slices looking for conflicts
	const fat_arch* archs = (fat_arch*)(((char*)fh)+sizeof(fat_header));
	for (uint32_t i=0; i < sliceCount; ++i) {
		uint32_t i_offset     = OSSwapBigToHostInt32(archs[i].offset);
		uint32_t i_size       = OSSwapBigToHostInt32(archs[i].size);
		uint32_t i_cputype    = OSSwapBigToHostInt32(archs[i].cputype);
		uint32_t i_cpusubtype = OSSwapBigToHostInt32(archs[i].cpusubtype);
		uint32_t i_end        = i_offset + i_size;
		// slice cannot overlap with header
		if ( i_offset < 4096 )
			return false;
		// slice size cannot overflow
		if ( i_end < i_offset )
			return false;
		for (uint32_t j=i+1; j < sliceCount; ++j) {
			uint32_t j_offset     = OSSwapBigToHostInt32(archs[j].offset);
			uint32_t j_size       = OSSwapBigToHostInt32(archs[j].size);
			uint32_t j_cputype    = OSSwapBigToHostInt32(archs[j].cputype);
			uint32_t j_cpusubtype = OSSwapBigToHostInt32(archs[j].cpusubtype);
			uint32_t j_end        = j_offset + j_size;
			// duplicate slices types not allowed
			if ( (i_cputype == j_cputype) && (i_cpusubtype == j_cpusubtype) )
				return false;
			// slice size cannot overflow
			if ( j_end < j_offset )
				return false;
			// check for overlap of slices
			if ( i_offset <= j_offset ) {
				if ( j_offset < i_end )
					return false; //  j overlaps end of i
			}
			else {
				//  j overlaps end of i
				if ( i_offset < j_end )
					return false;  //  i overlaps end of j
			}
		}
	}
	return true;
}

//
// A fat file may contain multiple sub-images for the same cpu-type,
// each optimized for a different cpu-sub-type (e.g G3 or G5).
// This routine picks the optimal sub-image.
//
static bool fatFindBest(const fat_header* fh, uint64_t* offset, uint64_t* len)
{
	if ( !fatValidate(fh) )
		return false;

#if CPU_SUBTYPES_SUPPORTED
	// assume all dylibs loaded must have same cpu type as main executable
	const cpu_type_t cpu = sMainExecutableMachHeader->cputype;

	// We only know the subtype to use if the main executable cpu type matches the host
	if ( (cpu & CPU_TYPE_MASK) == sHostCPU ) {
		// get preference ordered list of subtypes
		const cpu_subtype_t* subTypePreferenceList = findCPUSubtypeList(cpu, sHostCPUsubtype);
	
		// use ordered list to find best sub-image in fat file
		if ( subTypePreferenceList != NULL ) {
			if ( fatFindBestFromOrderedList(cpu, subTypePreferenceList, fh, offset, len) )
				return true;
		}

		// if running cpu is not in list, try for an exact match
		if ( fatFindExactMatch(cpu, sHostCPUsubtype, fh, offset, len) )
			return true;
	}
	
	// running on an uknown cpu, can only load generic code
	return fatFindRunsOnAllCPUs(cpu, fh, offset, len);
#else
	// just find first slice with matching architecture
	const fat_arch* archs = (fat_arch*)(((char*)fh)+sizeof(fat_header));
	for(uint32_t i=0; i < OSSwapBigToHostInt32(fh->nfat_arch); ++i) {
		if ( (cpu_type_t)OSSwapBigToHostInt32(archs[i].cputype) == sMainExecutableMachHeader->cputype) {
			*offset = OSSwapBigToHostInt32(archs[i].offset);
			*len = OSSwapBigToHostInt32(archs[i].size);
			return true;
		}
	}
	return false;
#endif
}



//
// This is used to validate if a non-fat (aka thin or raw) mach-o file can be used
// on the current processor. //
bool isCompatibleMachO(const uint8_t* firstPage, const char* path)
{
#if CPU_SUBTYPES_SUPPORTED
	// It is deemed compatible if any of the following are true:
	//  1) mach_header subtype is in list of compatible subtypes for running processor
	//  2) mach_header subtype is same as running processor subtype
	//  3) mach_header subtype runs on all processor variants
	const mach_header* mh = (mach_header*)firstPage;
	if ( mh->magic == sMainExecutableMachHeader->magic ) {
		if ( mh->cputype == sMainExecutableMachHeader->cputype ) {
			if ( (mh->cputype & CPU_TYPE_MASK) == sHostCPU ) {
				// get preference ordered list of subtypes that this machine can use
				const cpu_subtype_t* subTypePreferenceList = findCPUSubtypeList(mh->cputype, sHostCPUsubtype);
				if ( subTypePreferenceList != NULL ) {
					// if image's subtype is in the list, it is compatible
					for (const cpu_subtype_t* p = subTypePreferenceList; *p != CPU_SUBTYPE_END_OF_LIST; ++p) {
						if ( *p == mh->cpusubtype )
							return true;
					}
					// have list and not in list, so not compatible
					throwf("incompatible cpu-subtype: 0x%08X in %s", mh->cpusubtype, path);
				}
				// unknown cpu sub-type, but if exact match for current subtype then ok to use
				if ( mh->cpusubtype == sHostCPUsubtype ) 
					return true;
			}
			
			// cpu type has no ordered list of subtypes
			switch (mh->cputype) {
				case CPU_TYPE_I386:
				case CPU_TYPE_X86_64:
					// subtypes are not used or these architectures
					return true;
			}
		}
	}
#else
	// For architectures that don't support cpu-sub-types
	// this just check the cpu type.
	const mach_header* mh = (mach_header*)firstPage;
	if ( mh->magic == sMainExecutableMachHeader->magic ) {
		if ( mh->cputype == sMainExecutableMachHeader->cputype ) {
			return true;
		}
	}
#endif
	return false;
}




// The kernel maps in main executable before dyld gets control.  We need to 
// make an ImageLoader* for the already mapped in main executable.
static ImageLoaderMachO* instantiateFromLoadedImage(const macho_header* mh, uintptr_t slide, const char* path)
{
	// try mach-o loader
	if ( isCompatibleMachO((const uint8_t*)mh, path) ) {
		ImageLoader* image = ImageLoaderMachO::instantiateMainExecutable(mh, slide, path, gLinkContext);
		addImage(image);
		return (ImageLoaderMachO*)image;
	}
	
	throw "main executable not a known format";
}

#if DYLD_SHARED_CACHE_SUPPORT

#if __IPHONE_OS_VERSION_MIN_REQUIRED
static bool dylibsCanOverrideCache()
{
	uint32_t devFlags = *((uint32_t*)_COMM_PAGE_DEV_FIRM);
	if ( (devFlags & 1) == 0 )
		return false;
	return ( (sSharedCache != NULL) && (sSharedCache->cacheType == kDyldSharedCacheTypeDevelopment) );
}
#endif

static bool findInSharedCacheImage(const char* path, bool searchByPath, const struct stat* stat_buf, const macho_header** mh, const char** pathInCache, long* slide)
{
	if ( sSharedCache != NULL ) {
#if __MAC_OS_X_VERSION_MIN_REQUIRED	
		// Mac OS X always requires inode/mtime to valid cache
		// if stat() not done yet, do it now
		struct stat statb;
		if ( stat_buf == NULL ) {
			if ( my_stat(path, &statb) == -1 )
				return false;
			stat_buf = &statb;
		}
#endif
#if __IPHONE_OS_VERSION_MIN_REQUIRED	
		uint64_t hash = 0;
		for (const char* s=path; *s != '\0'; ++s)
			hash += hash*4 + *s;
#endif

		// walk shared cache to see if there is a cached image that matches the inode/mtime/path desired
		const dyld_cache_image_info* const start = (dyld_cache_image_info*)((uint8_t*)sSharedCache + sSharedCache->imagesOffset);
		const dyld_cache_image_info* const end = &start[sSharedCache->imagesCount];
#if __IPHONE_OS_VERSION_MIN_REQUIRED	
		const bool cacheHasHashInfo = (start->modTime == 0);
#endif
		for( const dyld_cache_image_info* p = start; p != end; ++p) {
#if __IPHONE_OS_VERSION_MIN_REQUIRED	
			// just check path
			const char* aPath = (char*)sSharedCache + p->pathFileOffset;
			if ( cacheHasHashInfo && (p->inode != hash) )
				continue;
			if ( strcmp(path, aPath) == 0 ) {
				// found image in cache
				*mh = (macho_header*)(p->address+sSharedCacheSlide);
				*pathInCache = aPath;
				*slide = sSharedCacheSlide;
				if ( aPath < (char*)(*mh) )  {
					// <rdar://problem/22056997> found alias, rescan list to get canonical name
					for (const dyld_cache_image_info* p2 = start; p2 != end; ++p2) {
						if ( p2->address == p->address ) {
							*pathInCache = (char*)sSharedCache + p2->pathFileOffset;
							break;
						}
					}
				}
				return true;
			}
#elif __MAC_OS_X_VERSION_MIN_REQUIRED
			// check mtime and inode first because it is fast
			bool inodeMatch = ( ((time_t)p->modTime == stat_buf->st_mtime) && ((ino_t)p->inode == stat_buf->st_ino) );
			if ( searchByPath || sSharedCacheIgnoreInodeAndTimeStamp || inodeMatch ) {
				// mod-time and inode match an image in the shared cache, now check path
				const char* aPath = (char*)sSharedCache + p->pathFileOffset;
				bool cacheHit = (strcmp(path, aPath) == 0);
				if ( inodeMatch && !cacheHit ) {
					// path does not match install name of dylib in cache, but inode and mtime does match
					// perhaps path is a symlink to the cached dylib
					struct stat pathInCacheStatBuf;
					if ( my_stat(aPath, &pathInCacheStatBuf) != -1 )
						cacheHit = ( (pathInCacheStatBuf.st_dev == stat_buf->st_dev) && (pathInCacheStatBuf.st_ino == stat_buf->st_ino) );	
				}
				if ( cacheHit ) {
					// found image in cache, return info
					*mh = (macho_header*)(p->address+sSharedCacheSlide);
					//dyld::log("findInSharedCacheImage(), mh=%p, p->address=0x%0llX, slid=0x%0lX, path=%s\n",
					//	*mh, p->address, sSharedCacheSlide, aPath);
					*pathInCache = aPath;
					*slide = sSharedCacheSlide;
					return true;
				}
			}
#endif
		}	
	}
	return false;
}

bool inSharedCache(const char* path)
{
	const macho_header* mhInCache;
	const char*			pathInCache;
	long				slide;
	return findInSharedCacheImage(path, true, NULL, &mhInCache, &pathInCache, &slide);
}

#endif

static ImageLoader* checkandAddImage(ImageLoader* image, const LoadContext& context)
{
	// now sanity check that this loaded image does not have the same install path as any existing image
	const char* loadedImageInstallPath = image->getInstallPath();
	if ( image->isDylib() && (loadedImageInstallPath != NULL) && (loadedImageInstallPath[0] == '/') ) {
		for (std::vector<ImageLoader*>::iterator it=sAllImages.begin(); it != sAllImages.end(); it++) {
			ImageLoader* anImage = *it;
			const char* installPath = anImage->getInstallPath();
			if ( installPath != NULL) {
				if ( strcmp(loadedImageInstallPath, installPath) == 0 ) {
					//dyld::log("duplicate(%s) => %p\n", installPath, anImage);
					removeImage(image);
					ImageLoader::deleteImage(image);
					return anImage;
				}
			}
		}
	}

	// some API's restrict what they can load
	if ( context.mustBeBundle && !image->isBundle() )
		throw "not a bundle";
	if ( context.mustBeDylib && !image->isDylib() )
		throw "not a dylib";

	// regular main executables cannot be loaded 
	if ( image->isExecutable() ) {
		if ( !context.canBePIE || !image->isPositionIndependentExecutable() )
			throw "can't load a main executable";
	}
	
	// don't add bundles to global list, they can be loaded but not linked.  When linked it will be added to list
	if ( ! image->isBundle() ) 
		addImage(image);
	
	return image;
}

#if TARGET_IPHONE_SIMULATOR	
static bool isSimulatorBinary(const uint8_t* firstPages, const char* path)
{
	const macho_header* mh = (macho_header*)firstPages;
	const uint32_t cmd_count = mh->ncmds;
	const load_command* const cmds = (struct load_command*)(((char*)mh)+sizeof(macho_header));
	const load_command* const cmdsEnd = (load_command*)((char*)cmds + mh->sizeofcmds);
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
	#if TARGET_OS_WATCH
			case LC_VERSION_MIN_WATCHOS:
				return true;
	#elif TARGET_OS_TV
			case LC_VERSION_MIN_TVOS:
				return true;
	#elif TARGET_OS_IOS
			case LC_VERSION_MIN_IPHONEOS:
				return true;
	#endif
			case LC_VERSION_MIN_MACOSX:
				// grandfather in a few libSystem dylibs
				if ((strcmp(path, "/usr/lib/system/libsystem_kernel.dylib") == 0) ||
				    (strcmp(path, "/usr/lib/system/libsystem_platform.dylib") == 0) ||
				    (strcmp(path, "/usr/lib/system/libsystem_pthread.dylib") == 0))
					return true;
				return false;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
		if ( cmd > cmdsEnd )
			return false;
	}
	return false;
}
#endif

// map in file and instantiate an ImageLoader
static ImageLoader* loadPhase6(int fd, const struct stat& stat_buf, const char* path, const LoadContext& context)
{
	//dyld::log("%s(%s)\n", __func__ , path);
	uint64_t fileOffset = 0;
	uint64_t fileLength = stat_buf.st_size;

	// validate it is a file (not directory)
	if ( (stat_buf.st_mode & S_IFMT) != S_IFREG ) 
		throw "not a file";

	uint8_t firstPages[MAX_MACH_O_HEADER_AND_LOAD_COMMANDS_SIZE];
	bool shortPage = false;
	
	// min mach-o file is 4K
	if ( fileLength < 4096 ) {
		if ( pread(fd, firstPages, fileLength, 0) != (ssize_t)fileLength )
			throwf("pread of short file failed: %d", errno);
		shortPage = true;
	} 
	else {
		// optimistically read only first 4KB
		if ( pread(fd, firstPages, 4096, 0) != 4096 )
			throwf("pread of first 4K failed: %d", errno);
	}
	
	// if fat wrapper, find usable sub-file
	const fat_header* fileStartAsFat = (fat_header*)firstPages;
	if ( fileStartAsFat->magic == OSSwapBigToHostInt32(FAT_MAGIC) ) {
		if ( OSSwapBigToHostInt32(fileStartAsFat->nfat_arch) > ((4096 - sizeof(fat_header)) / sizeof(fat_arch)) )
			throwf("fat header too large: %u entries", OSSwapBigToHostInt32(fileStartAsFat->nfat_arch));
		if ( fatFindBest(fileStartAsFat, &fileOffset, &fileLength) ) {
			if ( (fileOffset+fileLength) > (uint64_t)(stat_buf.st_size) )
				throwf("truncated fat file.  file length=%llu, but needed slice goes to %llu", stat_buf.st_size, fileOffset+fileLength);
			if (pread(fd, firstPages, 4096, fileOffset) != 4096)
				throwf("pread of fat file failed: %d", errno);
		}
		else {
			throw "no matching architecture in universal wrapper";
		}
	}
	
	// try mach-o loader
	if ( shortPage ) 
		throw "file too short";
	if ( isCompatibleMachO(firstPages, path) ) {

		// only MH_BUNDLE, MH_DYLIB, and some MH_EXECUTE can be dynamically loaded
		const mach_header* mh = (mach_header*)firstPages;
		switch ( mh->filetype ) {
			case MH_EXECUTE:
			case MH_DYLIB:
			case MH_BUNDLE:
				break;
			default:
				throw "mach-o, but wrong filetype";
		}

		uint32_t headerAndLoadCommandsSize = sizeof(macho_header) + mh->sizeofcmds;
		if ( headerAndLoadCommandsSize > MAX_MACH_O_HEADER_AND_LOAD_COMMANDS_SIZE )
			throwf("malformed mach-o: load commands size (%u) > %u", headerAndLoadCommandsSize, MAX_MACH_O_HEADER_AND_LOAD_COMMANDS_SIZE);

		if ( headerAndLoadCommandsSize > fileLength )
			dyld::throwf("malformed mach-o: load commands size (%u) > mach-o file size (%llu)", headerAndLoadCommandsSize, fileLength);

		if ( headerAndLoadCommandsSize > 4096 ) {
			// read more pages
			unsigned readAmount = headerAndLoadCommandsSize - 4096;
			if ( pread(fd, &firstPages[4096], readAmount, fileOffset+4096) != readAmount )
				throwf("pread of extra load commands past 4KB failed: %d", errno);
		}

#if TARGET_IPHONE_SIMULATOR	
		// <rdar://problem/14168872> dyld_sim should restrict loading osx binaries
		if ( !isSimulatorBinary(firstPages, path) ) {
	#if TARGET_OS_WATCH
			throw "mach-o, but not built for watchOS simulator";
	#elif TARGET_OS_TV
			throw "mach-o, but not built for tvOS simulator";
	#else
			throw "mach-o, but not built for iOS simulator";
	#endif
		}
#endif

		// instantiate an image
		ImageLoader* image = ImageLoaderMachO::instantiateFromFile(path, fd, firstPages, headerAndLoadCommandsSize, fileOffset, fileLength, stat_buf, gLinkContext);
		
		// validate
		return checkandAddImage(image, context);
	}
	
	// try other file formats here...
	
	
	// throw error about what was found
	switch (*(uint32_t*)firstPages) {
		case MH_MAGIC:
		case MH_CIGAM:
		case MH_MAGIC_64:
		case MH_CIGAM_64:
			throw "mach-o, but wrong architecture";
		default:
		throwf("unknown file type, first eight bytes: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X", 
			firstPages[0], firstPages[1], firstPages[2], firstPages[3], firstPages[4], firstPages[5], firstPages[6],firstPages[7]);
	}
}


static ImageLoader* loadPhase5open(const char* path, const LoadContext& context, const struct stat& stat_buf, std::vector<const char*>* exceptions)
{
	//dyld::log("%s(%s, %p)\n", __func__ , path, exceptions);

	// open file (automagically closed when this function exits)
	FileOpener file(path);
		
	// just return NULL if file not found, but record any other errors
	if ( file.getFileDescriptor() == -1 ) {
		int err = errno;
		if ( err != ENOENT ) {
			const char* newMsg;
			if ( (err == EPERM) && sandboxBlockedOpen(path) )
				newMsg = dyld::mkstringf("file system sandbox blocked open() of '%s'", path);
			else
				newMsg = dyld::mkstringf("%s: open() failed with errno=%d", path, err);
			exceptions->push_back(newMsg);
		}
		return NULL;
	}

	try {
		return loadPhase6(file.getFileDescriptor(), stat_buf, path, context);
	}
	catch (const char* msg) {
		const char* newMsg = dyld::mkstringf("%s: %s", path, msg);
		exceptions->push_back(newMsg);
		free((void*)msg);
		return NULL;
	}
}


#if __MAC_OS_X_VERSION_MIN_REQUIRED	
static ImageLoader* loadPhase5load(const char* path, const char* orgPath, const LoadContext& context, unsigned& cacheIndex, std::vector<const char*>* exceptions)
{
	//dyld::log("%s(%s, %p)\n", __func__ , path, exceptions);
	ImageLoader* image = NULL;

  #if SUPPORT_ACCELERATE_TABLES
	if ( sAllCacheImagesProxy != NULL ) {
		unsigned index;
		if ( sAllCacheImagesProxy->hasDylib(path, &index) )
			return sAllCacheImagesProxy;
	}
  #endif

	// just return NULL if file not found, but record any other errors
	struct stat stat_buf;
	if ( my_stat(path, &stat_buf) == -1 ) {
		int err = errno;
		if ( err != ENOENT ) {
			if ( (err == EPERM) && sandboxBlockedStat(path) )
				exceptions->push_back(dyld::mkstringf("%s: file system sandbox blocked stat()", path));
			else
				exceptions->push_back(dyld::mkstringf("%s: stat() failed with errno=%d", path, err));
		}
		return NULL;
	}

	// in case image was renamed or found via symlinks, check for inode match
	image = findLoadedImage(stat_buf);
	if ( image != NULL )
		return image;
	
	// do nothing if not already loaded and if RTLD_NOLOAD or NSADDIMAGE_OPTION_RETURN_ONLY_IF_LOADED
	if ( context.dontLoad )
		return NULL;

#if DYLD_SHARED_CACHE_SUPPORT
	// see if this image is in shared cache
	const macho_header* mhInCache;
	const char*			pathInCache;
	long				slideInCache;
	if ( findInSharedCacheImage(path, false, &stat_buf, &mhInCache, &pathInCache, &slideInCache) ) {
		image = ImageLoaderMachO::instantiateFromCache(mhInCache, pathInCache, slideInCache, stat_buf, gLinkContext);
		return checkandAddImage(image, context);
	}
#endif
	// file exists and is not in dyld shared cache, so open it
	return loadPhase5open(path, context, stat_buf, exceptions);
}
#endif // __MAC_OS_X_VERSION_MIN_REQUIRED



#if __IPHONE_OS_VERSION_MIN_REQUIRED 
static ImageLoader* loadPhase5stat(const char* path, const LoadContext& context, struct stat* stat_buf, 
									int* statErrNo, bool* imageFound, std::vector<const char*>* exceptions)
{
	ImageLoader* image = NULL;
	*imageFound = false;
	*statErrNo = 0;
	if ( my_stat(path, stat_buf) == 0 ) {
		// in case image was renamed or found via symlinks, check for inode match
		image = findLoadedImage(*stat_buf);
		if ( image != NULL ) {
			*imageFound = true;
			return image;
		}
		// do nothing if not already loaded and if RTLD_NOLOAD 
		if ( context.dontLoad ) {
			*imageFound = true;
			return NULL;
		}
		image = loadPhase5open(path, context, *stat_buf, exceptions);
		if ( image != NULL ) {
			*imageFound = true;
			return image;
		}
	}
	else {
		*statErrNo = errno;
	}
	return NULL;
}

// try to open file
static ImageLoader* loadPhase5load(const char* path, const char* orgPath, const LoadContext& context, unsigned& cacheIndex, std::vector<const char*>* exceptions)
{
	//dyld::log("%s(%s, %p)\n", __func__ , path, exceptions);
	struct stat stat_buf;
	bool imageFound;
	int statErrNo;
	ImageLoader* image;
#if DYLD_SHARED_CACHE_SUPPORT
  #if SUPPORT_ACCELERATE_TABLES
	if ( sAllCacheImagesProxy != NULL ) {
		if ( sAllCacheImagesProxy->hasDylib(path, &cacheIndex) )
			return sAllCacheImagesProxy;
	}
  #endif
	if ( dylibsCanOverrideCache() ) {
		// flag is set that allows installed framework roots to override dyld shared cache
		image = loadPhase5stat(path, context, &stat_buf, &statErrNo, &imageFound, exceptions);
		if ( imageFound )
			return image;
	}
	// see if this image is in shared cache
	const macho_header* mhInCache;
	const char*			pathInCache;
	long				slideInCache;
	if ( findInSharedCacheImage(path, true, NULL, &mhInCache, &pathInCache, &slideInCache) ) {
		// see if this image in the cache was already loaded via a different path
		for (std::vector<ImageLoader*>::iterator it=sAllImages.begin(); it != sAllImages.end(); ++it) {
			ImageLoader* anImage = *it;
			if ( (const macho_header*)anImage->machHeader() == mhInCache )
				return anImage;
		}
		// do nothing if not already loaded and if RTLD_NOLOAD 
		if ( context.dontLoad )
			return NULL;
		// nope, so instantiate a new image from dyld shared cache
		// <rdar://problem/7014995> zero out stat buffer so mtime, etc are zero for items from the shared cache
		bzero(&stat_buf, sizeof(stat_buf));
		image = ImageLoaderMachO::instantiateFromCache(mhInCache, pathInCache, slideInCache, stat_buf, gLinkContext);
		return checkandAddImage(image, context);
	}
	
	if ( !dylibsCanOverrideCache() ) {
		// flag is not set, and not in cache to try opening it
		image = loadPhase5stat(path, context, &stat_buf, &statErrNo, &imageFound, exceptions);
		if ( imageFound )
			return image;
	}
#else
	image = loadPhase5stat(path, context, &stat_buf, &statErrNo, &imageFound, exceptions);
	if ( imageFound )
		return image;
#endif
	// just return NULL if file not found, but record any other errors
	if ( (statErrNo != ENOENT) && (statErrNo != 0) ) {
		if ( (statErrNo == EPERM) && sandboxBlockedStat(path) )
			exceptions->push_back(dyld::mkstringf("%s: file system sandbox blocked stat()", path));
		else
			exceptions->push_back(dyld::mkstringf("%s: stat() failed with errno=%d", path, statErrNo));
	}
	return NULL;
}
#endif // __IPHONE_OS_VERSION_MIN_REQUIRED


// look for path match with existing loaded images
static ImageLoader* loadPhase5check(const char* path, const char* orgPath, const LoadContext& context)
{
	//dyld::log("%s(%s, %s)\n", __func__ , path, orgPath);
	// search path against load-path and install-path of all already loaded images
	uint32_t hash = ImageLoader::hash(path);
	//dyld::log("check() hash=%d, path=%s\n", hash, path);
	for (std::vector<ImageLoader*>::iterator it=sAllImages.begin(); it != sAllImages.end(); it++) {
		ImageLoader* anImage = *it;
		// check hash first to cut down on strcmp calls
		//dyld::log("    check() hash=%d, path=%s\n", anImage->getPathHash(), anImage->getPath());
		if ( anImage->getPathHash() == hash ) {
			if ( strcmp(path, anImage->getPath()) == 0 ) {
				// if we are looking for a dylib don't return something else
				if ( !context.mustBeDylib || anImage->isDylib() )
					return anImage;
			}
		}
		if ( context.matchByInstallName || anImage->matchInstallPath() ) {
			const char* installPath = anImage->getInstallPath();
			if ( installPath != NULL) {
				if ( strcmp(path, installPath) == 0 ) {
					// if we are looking for a dylib don't return something else
					if ( !context.mustBeDylib || anImage->isDylib() )
						return anImage;
				}
			}
		}
		// an install name starting with @rpath should match by install name, not just real path
		if ( (orgPath[0] == '@') && (strncmp(orgPath, "@rpath/", 7) == 0) ) {
			const char* installPath = anImage->getInstallPath();
			if ( installPath != NULL) {
				if ( !context.mustBeDylib || anImage->isDylib() ) {
					if ( strcmp(orgPath, installPath) == 0 )
						return anImage;
				}
			}
		}
	}
	
	//dyld::log("%s(%s) => NULL\n", __func__,   path);
	return NULL;
}


// open or check existing
static ImageLoader* loadPhase5(const char* path, const char* orgPath, const LoadContext& context, unsigned& cacheIndex, std::vector<const char*>* exceptions)
{
	//dyld::log("%s(%s, %p)\n", __func__ , path, exceptions);
	
	// check for specific dylib overrides
	for (std::vector<DylibOverride>::iterator it = sDylibOverrides.begin(); it != sDylibOverrides.end(); ++it) {
		if ( strcmp(it->installName, path) == 0 ) {
			path = it->override;
			break;
		}
	}
	
	if ( exceptions != NULL ) 
		return loadPhase5load(path, orgPath, context, cacheIndex, exceptions);
	else
		return loadPhase5check(path, orgPath, context);
}

// try with and without image suffix
static ImageLoader* loadPhase4(const char* path, const char* orgPath, const LoadContext& context, unsigned& cacheIndex, std::vector<const char*>* exceptions)
{
	//dyld::log("%s(%s, %p)\n", __func__ , path, exceptions);
	ImageLoader* image = NULL;
	if (  gLinkContext.imageSuffix != NULL ) {
		char pathWithSuffix[strlen(path)+strlen( gLinkContext.imageSuffix)+2];
		ImageLoader::addSuffix(path,  gLinkContext.imageSuffix, pathWithSuffix);
		image = loadPhase5(pathWithSuffix, orgPath, context, cacheIndex, exceptions);
	}
	if ( image == NULL )
		image = loadPhase5(path, orgPath, context, cacheIndex, exceptions);
	return image;
}

static ImageLoader* loadPhase2(const char* path, const char* orgPath, const LoadContext& context,
							   const char* const frameworkPaths[], const char* const libraryPaths[],
							   unsigned& cacheIndex, std::vector<const char*>* exceptions); // forward reference


// expand @ variables
static ImageLoader* loadPhase3(const char* path, const char* orgPath, const LoadContext& context, unsigned& cacheIndex, std::vector<const char*>* exceptions)
{
	//dyld::log("%s(%s, %p)\n", __func__ , path, exceptions);
	ImageLoader* image = NULL;
	if ( strncmp(path, "@executable_path/", 17) == 0 ) {
#if __MAC_OS_X_VERSION_MIN_REQUIRED
		// executable_path cannot be in used in any binary in a setuid process rdar://problem/4589305
		if ( gLinkContext.processIsRestricted )
			throwf("unsafe use of @executable_path in %s with restricted binary", context.origin);
#endif
		// handle @executable_path path prefix
		const char* executablePath = sExecPath;
		char newPath[strlen(executablePath) + strlen(path)];
		strcpy(newPath, executablePath);
		char* addPoint = strrchr(newPath,'/');
		if ( addPoint != NULL )
			strcpy(&addPoint[1], &path[17]);
		else
			strcpy(newPath, &path[17]);
		image = loadPhase4(newPath, orgPath, context, cacheIndex, exceptions);
		if ( image != NULL ) 
			return image;

		// perhaps main executable path is a sym link, find realpath and retry
		char resolvedPath[PATH_MAX];
		if ( realpath(sExecPath, resolvedPath) != NULL ) {
			char newRealPath[strlen(resolvedPath) + strlen(path)];
			strcpy(newRealPath, resolvedPath);
			addPoint = strrchr(newRealPath,'/');
			if ( addPoint != NULL )
				strcpy(&addPoint[1], &path[17]);
			else
				strcpy(newRealPath, &path[17]);
			image = loadPhase4(newRealPath, orgPath, context, cacheIndex, exceptions);
			if ( image != NULL ) 
				return image;
		}
	}
	else if ( (strncmp(path, "@loader_path/", 13) == 0) && (context.origin != NULL) ) {
#if __MAC_OS_X_VERSION_MIN_REQUIRED
		// @loader_path cannot be used from the main executable of a setuid process rdar://problem/4589305
		if ( gLinkContext.processIsRestricted && (strcmp(context.origin, sExecPath) == 0) )
			throwf("unsafe use of @loader_path in %s with restricted binary", context.origin);
#endif
		// handle @loader_path path prefix
		char newPath[strlen(context.origin) + strlen(path)];
		strcpy(newPath, context.origin);
		char* addPoint = strrchr(newPath,'/');
		if ( addPoint != NULL )
			strcpy(&addPoint[1], &path[13]);
		else
			strcpy(newPath, &path[13]);
		image = loadPhase4(newPath, orgPath, context, cacheIndex, exceptions);
		if ( image != NULL ) 
			return image;
		
		// perhaps loader path is a sym link, find realpath and retry
		char resolvedPath[PATH_MAX];
		if ( realpath(context.origin, resolvedPath) != NULL ) {
			char newRealPath[strlen(resolvedPath) + strlen(path)];
			strcpy(newRealPath, resolvedPath);
			addPoint = strrchr(newRealPath,'/');
			if ( addPoint != NULL )
				strcpy(&addPoint[1], &path[13]);
			else
				strcpy(newRealPath, &path[13]);
			image = loadPhase4(newRealPath, orgPath, context, cacheIndex, exceptions);
			if ( image != NULL ) 
				return image;
		}
	}
	else if ( context.implicitRPath || (strncmp(path, "@rpath/", 7) == 0) ) {
		const char* trailingPath = (strncmp(path, "@rpath/", 7) == 0) ? &path[7] : path;
		// substitute @rpath with all -rpath paths up the load chain
		for(const ImageLoader::RPathChain* rp=context.rpath; rp != NULL; rp=rp->next) {
			if (rp->paths != NULL ) {
				for(std::vector<const char*>::iterator it=rp->paths->begin(); it != rp->paths->end(); ++it) {
					const char* anRPath = *it;
					char newPath[strlen(anRPath) + strlen(trailingPath)+2];
					strcpy(newPath, anRPath);
					if ( newPath[strlen(newPath)-1] != '/' )
						strcat(newPath, "/");
					strcat(newPath, trailingPath); 
					image = loadPhase4(newPath, orgPath, context, cacheIndex, exceptions);
					if ( gLinkContext.verboseRPaths && (exceptions != NULL) ) {
						if ( image != NULL ) 
							dyld::log("RPATH successful expansion of %s to: %s\n", orgPath, newPath);
						else
							dyld::log("RPATH failed to expanding     %s to: %s\n", orgPath, newPath);
					}
					if ( image != NULL ) 
						return image;
				}
			}
		}
		
		// substitute @rpath with LD_LIBRARY_PATH
		if ( sEnv.LD_LIBRARY_PATH != NULL ) {
			image = loadPhase2(trailingPath, orgPath, context, NULL, sEnv.LD_LIBRARY_PATH, cacheIndex, exceptions);
			if ( image != NULL )
				return image;
		}
		
		// if this is the "open" pass, don't try to open @rpath/... as a relative path
		if ( (exceptions != NULL) && (trailingPath != path) )
			return NULL;
	}
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	else if ( gLinkContext.processIsRestricted && (path[0] != '/' ) ) {
		throwf("unsafe use of relative rpath %s in %s with restricted binary", path, context.origin);
	}
#endif
	
	return loadPhase4(path, orgPath, context, cacheIndex, exceptions);
}


// try search paths
static ImageLoader* loadPhase2(const char* path, const char* orgPath, const LoadContext& context,
							   const char* const frameworkPaths[], const char* const libraryPaths[], 
							   unsigned& cacheIndex, std::vector<const char*>* exceptions)
{
	//dyld::log("%s(%s, %p)\n", __func__ , path, exceptions);
	ImageLoader* image = NULL;
	const char* frameworkPartialPath = getFrameworkPartialPath(path);
	if ( frameworkPaths != NULL ) {
		if ( frameworkPartialPath != NULL ) {
			const size_t frameworkPartialPathLen = strlen(frameworkPartialPath);
			for(const char* const* fp = frameworkPaths; *fp != NULL; ++fp) {
				char npath[strlen(*fp)+frameworkPartialPathLen+8];
				strcpy(npath, *fp);
				strcat(npath, "/");
				strcat(npath, frameworkPartialPath);
				//dyld::log("dyld: fallback framework path used: %s() -> loadPhase4(\"%s\", ...)\n", __func__, npath);
				image = loadPhase4(npath, orgPath, context, cacheIndex, exceptions);
				if ( image != NULL )
					return image;
			}
		}
	}
	// <rdar://problem/12649639> An executable with the same name as a framework & DYLD_LIBRARY_PATH pointing to it gets loaded twice
	// <rdar://problem/14160846> Some apps depend on frameworks being found via library paths
	if ( (libraryPaths != NULL) && ((frameworkPartialPath == NULL) || sFrameworksFoundAsDylibs) ) {
		const char* libraryLeafName = getLibraryLeafName(path);
		const size_t libraryLeafNameLen = strlen(libraryLeafName);
		for(const char* const* lp = libraryPaths; *lp != NULL; ++lp) {
			char libpath[strlen(*lp)+libraryLeafNameLen+8];
			strcpy(libpath, *lp);
			strcat(libpath, "/");
			strcat(libpath, libraryLeafName);
			//dyld::log("dyld: fallback library path used: %s() -> loadPhase4(\"%s\", ...)\n", __func__, libpath);
			image = loadPhase4(libpath, orgPath, context, cacheIndex, exceptions);
			if ( image != NULL )
				return image;
		}
	}
	return NULL;
}

// try search overrides and fallbacks
static ImageLoader* loadPhase1(const char* path, const char* orgPath, const LoadContext& context, unsigned& cacheIndex, std::vector<const char*>* exceptions)
{
	//dyld::log("%s(%s, %p)\n", __func__ , path, exceptions);
	ImageLoader* image = NULL;

	// handle LD_LIBRARY_PATH environment variables that force searching
	if ( context.useLdLibraryPath && (sEnv.LD_LIBRARY_PATH != NULL) ) {
		image = loadPhase2(path, orgPath, context,  NULL, sEnv.LD_LIBRARY_PATH, cacheIndex,exceptions);
		if ( image != NULL )
			return image;
	}

	// handle DYLD_ environment variables that force searching
	if ( context.useSearchPaths && ((sEnv.DYLD_FRAMEWORK_PATH != NULL) || (sEnv.DYLD_LIBRARY_PATH != NULL)) ) {
		image = loadPhase2(path, orgPath, context, sEnv.DYLD_FRAMEWORK_PATH, sEnv.DYLD_LIBRARY_PATH, cacheIndex, exceptions);
		if ( image != NULL )
			return image;
	}
	
	// try raw path
	image = loadPhase3(path, orgPath, context, cacheIndex, exceptions);
	if ( image != NULL )
		return image;
	
	// try fallback paths during second time (will open file)
	const char* const* fallbackLibraryPaths = sEnv.DYLD_FALLBACK_LIBRARY_PATH;
	if ( (fallbackLibraryPaths != NULL) && !context.useFallbackPaths )
		fallbackLibraryPaths = NULL;
	if ( !context.dontLoad  && (exceptions != NULL) && ((sEnv.DYLD_FALLBACK_FRAMEWORK_PATH != NULL) || (fallbackLibraryPaths != NULL)) ) {
		image = loadPhase2(path, orgPath, context, sEnv.DYLD_FALLBACK_FRAMEWORK_PATH, fallbackLibraryPaths, cacheIndex, exceptions);
		if ( image != NULL )
			return image;
	}
		
	return NULL;
}

// try root substitutions
static ImageLoader* loadPhase0(const char* path, const char* orgPath, const LoadContext& context, unsigned& cacheIndex, std::vector<const char*>* exceptions)
{
	//dyld::log("%s(%s, %p)\n", __func__ , path, exceptions);

#if SUPPORT_ROOT_PATH
	// handle DYLD_ROOT_PATH which forces absolute paths to use a new root
	if ( (gLinkContext.rootPaths != NULL) && (path[0] == '/') ) {
		for(const char* const* rootPath = gLinkContext.rootPaths ; *rootPath != NULL; ++rootPath) {
			char newPath[strlen(*rootPath) + strlen(path)+2];
			strcpy(newPath, *rootPath);
			strcat(newPath, path);
			ImageLoader* image = loadPhase1(newPath, orgPath, context, cacheIndex, exceptions);
			if ( image != NULL )
				return image;
		}
	}
#endif

	// try raw path
	return loadPhase1(path, orgPath, context, cacheIndex, exceptions);
}

#if DYLD_SHARED_CACHE_SUPPORT
	static bool cacheablePath(const char* path) {
		if (strncmp(path, "/usr/lib/", 9) == 0)
			return true;
		if (strncmp(path, "/System/Library/", 16) == 0)
			return true;
		return false;
	}
#endif

//
// Given all the DYLD_ environment variables, the general case for loading libraries
// is that any given path expands into a list of possible locations to load.  We
// also must take care to ensure two copies of the "same" library are never loaded.
//
// The algorithm used here is that there is a separate function for each "phase" of the
// path expansion.  Each phase function calls the next phase with each possible expansion
// of that phase.  The result is the last phase is called with all possible paths.  
//
// To catch duplicates the algorithm is run twice.  The first time, the last phase checks
// the path against all loaded images.  The second time, the last phase calls open() on 
// the path.  Either time, if an image is found, the phases all unwind without checking
// for other paths.
//
ImageLoader* load(const char* path, const LoadContext& context, unsigned& cacheIndex)
{
	CRSetCrashLogMessage2(path);
	const char* orgPath = path;
	cacheIndex = UINT32_MAX;
	
	//dyld::log("%s(%s)\n", __func__ , path);
	char realPath[PATH_MAX];
	// when DYLD_IMAGE_SUFFIX is in used, do a realpath(), otherwise a load of "Foo.framework/Foo" will not match
	if ( context.useSearchPaths && ( gLinkContext.imageSuffix != NULL) ) {
		if ( realpath(path, realPath) != NULL )
			path = realPath;
	}
	
	// try all path permutations and check against existing loaded images

	ImageLoader* image = loadPhase0(path, orgPath, context, cacheIndex, NULL);
	if ( image != NULL ) {
		CRSetCrashLogMessage2(NULL);
		return image;
	}

	// try all path permutations and try open() until first success
	std::vector<const char*> exceptions;
	image = loadPhase0(path, orgPath, context, cacheIndex, &exceptions);
#if __IPHONE_OS_VERSION_MIN_REQUIRED && DYLD_SHARED_CACHE_SUPPORT && !TARGET_IPHONE_SIMULATOR
	// <rdar://problem/16704628> support symlinks on disk to a path in dyld shared cache
	if ( (image == NULL) && cacheablePath(path) && !context.dontLoad ) {
		char resolvedPath[PATH_MAX];
		realpath(path, resolvedPath);
		int myerr = errno;
		// If realpath() resolves to a path which does not exist on disk, errno is set to ENOENT
		if ( (myerr == ENOENT) || (myerr == 0) )
		{
			// see if this image is in shared cache
			const macho_header* mhInCache;
			const char*			pathInCache;
			long				slideInCache;
			if ( findInSharedCacheImage(resolvedPath, false, NULL, &mhInCache, &pathInCache, &slideInCache) ) {
				struct stat stat_buf;
				bzero(&stat_buf, sizeof(stat_buf));
				try {
					image = ImageLoaderMachO::instantiateFromCache(mhInCache, pathInCache, slideInCache, stat_buf, gLinkContext);
					image = checkandAddImage(image, context);
				}
				catch (...) {
					image = NULL;
				}
			}
		}
	}
#endif
    CRSetCrashLogMessage2(NULL);
	if ( image != NULL ) {
		// <rdar://problem/6916014> leak in dyld during dlopen when using DYLD_ variables
		for (std::vector<const char*>::iterator it = exceptions.begin(); it != exceptions.end(); ++it) {
			free((void*)(*it));
		}
#if DYLD_SHARED_CACHE_SUPPORT
		// if loaded image is not from cache, but original path is in cache
		// set gSharedCacheOverridden flag to disable some ObjC optimizations
		if ( !gSharedCacheOverridden && !image->inSharedCache() && image->isDylib() && cacheablePath(path) && inSharedCache(path) ) {
			gSharedCacheOverridden = true;
		}
#endif
		return image;
	}
	else if ( exceptions.size() == 0 ) {
		if ( context.dontLoad ) {
			return NULL;
		}
		else
			throw "image not found";
	}
	else {
		const char* msgStart = "no suitable image found.  Did find:";
		const char* delim = "\n\t";
		size_t allsizes = strlen(msgStart)+8;
		for (size_t i=0; i < exceptions.size(); ++i) 
			allsizes += (strlen(exceptions[i]) + strlen(delim));
		char* fullMsg = new char[allsizes];
		strcpy(fullMsg, msgStart);
		for (size_t i=0; i < exceptions.size(); ++i) {
			strcat(fullMsg, delim);
			strcat(fullMsg, exceptions[i]);
			free((void*)exceptions[i]);
		}
		throw (const char*)fullMsg;
	}
}



#if DYLD_SHARED_CACHE_SUPPORT



#if __i386__
	#define ARCH_NAME			"i386"
	#define ARCH_CACHE_MAGIC	"dyld_v1    i386"
#elif __x86_64__
	#define ARCH_NAME			"x86_64"
	#define ARCH_CACHE_MAGIC	"dyld_v1  x86_64"
	#define ARCH_NAME_H			"x86_64h"
	#define ARCH_CACHE_MAGIC_H	"dyld_v1 x86_64h"
#elif __ARM_ARCH_5TEJ__
	#define ARCH_NAME			"armv5"
	#define ARCH_CACHE_MAGIC	"dyld_v1   armv5"
#elif __ARM_ARCH_6K__
	#define ARCH_NAME			"armv6"
	#define ARCH_CACHE_MAGIC	"dyld_v1   armv6"
#elif __ARM_ARCH_7F__
	#define ARCH_NAME			"armv7f"
	#define ARCH_CACHE_MAGIC	"dyld_v1  armv7f"
#elif __ARM_ARCH_7K__
	#define ARCH_NAME			"armv7k"
	#define ARCH_CACHE_MAGIC	"dyld_v1  armv7k"
#elif __ARM_ARCH_7A__
	#define ARCH_NAME			"armv7"
	#define ARCH_CACHE_MAGIC	"dyld_v1   armv7"
#elif __ARM_ARCH_7S__
	#define ARCH_NAME			"armv7s"
	#define ARCH_CACHE_MAGIC	"dyld_v1  armv7s"
#elif __arm64__
	#define ARCH_NAME			"arm64"
	#define ARCH_CACHE_MAGIC	"dyld_v1   arm64"
#endif


static int __attribute__((noinline)) _shared_region_check_np(uint64_t* start_address)
{
	if ( gLinkContext.sharedRegionMode == ImageLoader::kUseSharedRegion ) 
		return syscall(294, start_address);
	return -1;
}


static void rebaseChain(uint8_t* pageContent, uint16_t startOffset, uintptr_t slideAmount, const dyld_cache_slide_info2* slideInfo)
{
    const uintptr_t   deltaMask    = (uintptr_t)(slideInfo->delta_mask);
    const uintptr_t   valueMask    = ~deltaMask;
    const uintptr_t   valueAdd     = (uintptr_t)(slideInfo->value_add);
    const unsigned	  deltaShift   = __builtin_ctzll(deltaMask) - 2;

	uint32_t pageOffset = startOffset;
	uint32_t delta = 1;
	while ( delta != 0 ) {
		uint8_t* loc = pageContent + pageOffset;
		uintptr_t rawValue = *((uintptr_t*)loc);
		delta = (uint32_t)((rawValue & deltaMask) >> deltaShift);
		uintptr_t value = (rawValue & valueMask);
		if ( value != 0 ) {
			value += valueAdd;
			value += slideAmount;
		}
		*((uintptr_t*)loc) = value;
		//dyld::log("         pageOffset=0x%03X, loc=%p, org value=0x%08llX, new value=0x%08llX, delta=0x%X\n", pageOffset, loc, (uint64_t)rawValue, (uint64_t)value, delta);
		pageOffset += delta;
	}
}


static void loadAndCheckCodeSignature(int fd, uint32_t count, const shared_file_mapping_np mappings[],
									off_t codeSignatureOffset, size_t codeSignatureSize,
									const void *firstPages, size_t firstPagesSize)
{
	// register code signature blob for whole dyld cache
	fsignatures_t siginfo;
	siginfo.fs_file_start = 0;  // cache always starts at beginning of file
	siginfo.fs_blob_start = (void*)codeSignatureOffset;
	siginfo.fs_blob_size  = codeSignatureSize;

	int result = fcntl(fd, F_ADDFILESIGS_RETURN, &siginfo);
		// <rdar://problem/12891874> don't warn in chrooted case because mapping syscall is about to fail too
	if ( result == -1 ) {
#if __IPHONE_OS_VERSION_MIN_REQUIRED
		throwf("code signature registration for shared cache failed with errno=%d\n", errno);
#else
		if ( gLinkContext.verboseMapping )
			dyld::log("dyld: code signature registration for shared cache failed with errno=%d\n", errno);
#endif
	}
	uint64_t codeSignedLength = siginfo.fs_file_start;
	for (uint32_t i = 0; i < count; ++i) {
		if ( (mappings[i].sfm_size > codeSignedLength) || (mappings[i].sfm_file_offset > (codeSignedLength - mappings[i].sfm_size)) )
			throw "dyld shared cache mapping not covered by code signature";
	}

	void *fdata = xmmap(NULL, firstPagesSize, PROT_READ|PROT_EXEC, MAP_PRIVATE, fd, 0);
	if ( fdata == MAP_FAILED )
		throwf("mmap() errno=%d validating first page of shared cache", errno);
	if ( memcmp(fdata, firstPages, firstPagesSize) != 0 )
		throwf("mmap() page compare failed for shared cache");
	munmap(fdata, firstPagesSize);
}

static int __attribute__((noinline)) _shared_region_map_and_slide_np(int fd, uint32_t count, const shared_file_mapping_np mappings[],
												long slide, void* slideInfo, unsigned long slideInfoSize)
{
	if ( gLinkContext.sharedRegionMode == ImageLoader::kUseSharedRegion ) {
		return syscall(438, fd, count, mappings, slide, slideInfo, slideInfoSize);
	}

	// remove the shared region sub-map
	vm_deallocate(mach_task_self(), (vm_address_t)SHARED_REGION_BASE, SHARED_REGION_SIZE);
	
	// notify gdb or other lurkers that this process is no longer using the shared region
	dyld::gProcessInfo->processDetachedFromSharedRegion = true;

	// map cache just for this process with mmap()
	const shared_file_mapping_np* const start = mappings;
	const shared_file_mapping_np* const end = &mappings[count];
	for (const shared_file_mapping_np* p = start; p < end; ++p ) {
		void* mmapAddress = (void*)(uintptr_t)(p->sfm_address);
		size_t size = p->sfm_size;
		//dyld::log("dyld: mapping address %p with size 0x%08lX\n", mmapAddress, size);
		int protection = 0;
		if ( p->sfm_init_prot & VM_PROT_EXECUTE )
			protection   |= PROT_EXEC;
		if ( p->sfm_init_prot & VM_PROT_READ )
			protection   |= PROT_READ;
		if ( p->sfm_init_prot & VM_PROT_WRITE )
			protection   |= PROT_WRITE;
		off_t offset = p->sfm_file_offset;
		if ( mmap(mmapAddress, size, protection, MAP_FIXED | MAP_PRIVATE, fd, offset) != mmapAddress ) {
			// failed to map some chunk of this shared cache file
			// clear shared region
			vm_deallocate(mach_task_self(), (vm_address_t)SHARED_REGION_BASE, SHARED_REGION_SIZE);
			// go back to not using shared region at all
			gLinkContext.sharedRegionMode = ImageLoader::kDontUseSharedRegion;
			if ( gLinkContext.verboseMapping ) {
				dyld::log("dyld: shared cached region cannot be mapped at address %p with size 0x%08lX\n",
							mmapAddress, size);
			}
			// return failure
			return -1;
		}
	}

	// update all __DATA pages with slide info
	const dyld_cache_slide_info* slideInfoHeader = (dyld_cache_slide_info*)slideInfo;
	if ( slideInfoHeader->version == 2 ) {
		const dyld_cache_slide_info2* slideHeader = (dyld_cache_slide_info2*)slideInfo;
		const uint32_t  page_size = slideHeader->page_size;
		const uint16_t* page_starts = (uint16_t*)((long)(slideInfo) + slideHeader->page_starts_offset);
		const uint16_t* page_extras = (uint16_t*)((long)(slideInfo) + slideHeader->page_extras_offset);
		const uintptr_t dataPagesStart = mappings[1].sfm_address;
		for (int i=0; i < slideHeader->page_starts_count; ++i) {
			uint8_t* page = (uint8_t*)(long)(dataPagesStart + (page_size*i));
			uint16_t pageEntry = page_starts[i];
			//dyld::log("page[%d]: page_starts[i]=0x%04X\n", i, pageEntry);
			if ( pageEntry == DYLD_CACHE_SLIDE_PAGE_ATTR_NO_REBASE )
				continue;
			if ( pageEntry & DYLD_CACHE_SLIDE_PAGE_ATTR_EXTRA ) {
				uint16_t chainIndex = (pageEntry & 0x3FFF);
				bool done = false;
				while ( !done ) {
					uint16_t info = page_extras[chainIndex];
					uint16_t pageStartOffset = (info & 0x3FFF)*4;
					//dyld::log("     chain[%d] pageOffset=0x%03X\n", chainIndex, pageStartOffset);
					rebaseChain(page, pageStartOffset, slide, slideHeader);
					done = (info & DYLD_CACHE_SLIDE_PAGE_ATTR_END);
					++chainIndex;
				}
			}
			else {
				uint32_t pageOffset = pageEntry * 4;
				//dyld::log("     start pageOffset=0x%03X\n", pageOffset);
				rebaseChain(page, pageOffset, slide, slideHeader);
			}
		}
	}
	else if ( slide != 0 ) {
		const uintptr_t dataPagesStart = mappings[1].sfm_address;
		const uint16_t* toc = (uint16_t*)((long)(slideInfoHeader) + slideInfoHeader->toc_offset);
		const uint8_t* entries = (uint8_t*)((long)(slideInfoHeader) + slideInfoHeader->entries_offset);
		for(uint32_t i=0; i < slideInfoHeader->toc_count; ++i) {
			const uint8_t* entry = &entries[toc[i]*slideInfoHeader->entries_size];
			const uint8_t* page = (uint8_t*)(long)(dataPagesStart + (4096*i));
			//dyld::log("page=%p toc[%d]=%d entries=%p\n", page, i, toc[i], entry);
			for(int j=0; j < 128; ++j) {
				uint8_t b = entry[j];
				//dyld::log("    entry[%d] = 0x%02X\n", j, b);
				if ( b != 0 ) {
					for(int k=0; k < 8; ++k) {
						if ( b & (1<<k) ) {
							uintptr_t* p = (uintptr_t*)(page + j*8*4 + k*4);
							uintptr_t value = *p;
							//dyld::log("        *%p was 0x%lX will be 0x%lX\n", p, value, value+sSharedCacheSlide);
							*p = value + slide;
						}
					}
				}
			}
		}
	}

	// succesfully mapped shared cache for just this process
	gLinkContext.sharedRegionMode = ImageLoader::kUsePrivateSharedRegion;
	
	return 0;
}


const void*	imMemorySharedCacheHeader()
{
	return sSharedCache;
}

const char* getStandardSharedCacheFilePath()
{
#if __IPHONE_OS_VERSION_MIN_REQUIRED
	return IPHONE_DYLD_SHARED_CACHE_DIR DYLD_SHARED_CACHE_BASE_NAME ARCH_NAME;
#else
  #if __x86_64__
	if ( sHaswell ) {
		const char* path2 = MACOSX_DYLD_SHARED_CACHE_DIR DYLD_SHARED_CACHE_BASE_NAME ARCH_NAME_H;
		struct stat statBuf;
		if ( my_stat(path2, &statBuf) == 0 )
			return path2;
	}
  #endif
	return MACOSX_DYLD_SHARED_CACHE_DIR DYLD_SHARED_CACHE_BASE_NAME ARCH_NAME;
#endif
}

int openSharedCacheFile()
{
	char path[MAXPATHLEN];
	strlcpy(path, sSharedCacheDir, MAXPATHLEN);
	strlcat(path, "/", MAXPATHLEN);
#if __x86_64__
	if ( sHaswell ) {
		strlcat(path, DYLD_SHARED_CACHE_BASE_NAME ARCH_NAME_H, MAXPATHLEN);
		int fd = my_open(path, O_RDONLY, 0);
		if ( fd != -1 ) {
			if ( gLinkContext.verboseMapping ) 
				dyld::log("dyld: Mapping%s shared cache from %s\n", (gLinkContext.sharedRegionMode == ImageLoader::kUsePrivateSharedRegion) ? " private": "", path);
			return fd;
		}
		strlcpy(path, sSharedCacheDir, MAXPATHLEN);
	}
#endif
	strlcat(path, DYLD_SHARED_CACHE_BASE_NAME ARCH_NAME, MAXPATHLEN);
#if __IPHONE_OS_VERSION_MIN_REQUIRED
	struct stat enableStatBuf;
	struct stat devCacheStatBuf;
	struct stat prodCacheStatBuf;
	if ( ((my_stat(IPHONE_DYLD_SHARED_CACHE_DIR "enable-dylibs-to-override-cache", &enableStatBuf) == 0)
			&& (enableStatBuf.st_size < ENABLE_DYLIBS_TO_OVERRIDE_CACHE_SIZE)
			&& (my_stat(IPHONE_DYLD_SHARED_CACHE_DIR DYLD_SHARED_CACHE_BASE_NAME ARCH_NAME DYLD_SHARED_CACHE_DEVELOPMENT_EXT, &devCacheStatBuf) == 0))
		|| (my_stat(IPHONE_DYLD_SHARED_CACHE_DIR DYLD_SHARED_CACHE_BASE_NAME ARCH_NAME, &prodCacheStatBuf) != 0))
		strlcat(path, DYLD_SHARED_CACHE_DEVELOPMENT_EXT, MAXPATHLEN);
#endif
	if ( gLinkContext.verboseMapping )
		dyld::log("dyld: Mapping%s shared cache from %s\n", (gLinkContext.sharedRegionMode == ImageLoader::kUsePrivateSharedRegion) ? " private": "", path);
	return my_open(path, O_RDONLY, 0);
}


static void getCacheBounds(uint32_t mappingsCount, const shared_file_mapping_np mappings[], uint64_t& lowAddress, uint64_t& highAddress)
{
	lowAddress = 0;
	highAddress = 0;
	for(uint32_t i=0; i < mappingsCount; ++i) {
		if ( lowAddress == 0 ) {
			lowAddress = mappings[i].sfm_address;
			highAddress = mappings[i].sfm_address + mappings[i].sfm_size;
		}
		else {
			if ( mappings[i].sfm_address < lowAddress )
				lowAddress = mappings[i].sfm_address;
			if ( (mappings[i].sfm_address + mappings[i].sfm_size) > highAddress )
				highAddress = mappings[i].sfm_address + mappings[i].sfm_size;
		}
	}
}

static long pickCacheSlide(uint32_t mappingsCount, shared_file_mapping_np mappings[])
{
#if __x86_64__
	// x86_64 has a two memory regions:
	//       256MB at 0x00007FFF70000000 
	//      1024MB at 0x00007FFF80000000
	// Some old shared caches have r/w region after rx region, so all regions slide within 1GB range
	// Newer shared caches have r/w region based at 0x7FFF70000000 and r/o regions at 0x7FFF80000000, so each part has max slide
	if ( (mappingsCount >= 3) && (mappings[1].sfm_init_prot == (VM_PROT_READ|VM_PROT_WRITE)) && (mappings[1].sfm_address == 0x00007FFF70000000) ) {
		const uint64_t rwSize = mappings[1].sfm_size;
		const uint64_t rwSlop = 0x10000000ULL - rwSize;
		const uint64_t roSize = (mappings[2].sfm_address + mappings[2].sfm_size) - mappings[0].sfm_address;
		const uint64_t roSlop = 0x40000000ULL - roSize;
		const uint64_t space = (rwSlop < roSlop) ? rwSlop : roSlop;
		
		// choose new random slide
		long slide = (arc4random() % space) & (-4096);
		//dyld::log("rwSlop=0x%0llX, roSlop=0x%0llX\n", rwSlop, roSlop);
		//dyld::log("space=0x%0llX, slide=0x%0lX\n", space, slide);
		
		// update mappings
		for(uint32_t i=0; i < mappingsCount; ++i) {
			mappings[i].sfm_address += slide;
		}
		
		return slide;
	}
	// else fall through to handle old style cache
#endif
	// get bounds of cache
	uint64_t lowAddress;
	uint64_t highAddress;
	getCacheBounds(mappingsCount, mappings, lowAddress, highAddress);
	
	// find slop space
	const uint64_t space = (SHARED_REGION_BASE + SHARED_REGION_SIZE) - highAddress;
	
	// choose new random slide
#if __arm__
	// <rdar://problem/20848977> change shared cache slide for 32-bit arm to always be 16k aligned
	long slide = ((arc4random() % space) & (-16384));
#else
	long slide = dyld_page_trunc(arc4random() % space);
#endif
	//dyld::log("slideSpace=0x%0llX\n", space);
	//dyld::log("slide=0x%0lX\n", slide);

	// update mappings
	for(uint32_t i=0; i < mappingsCount; ++i) {
		mappings[i].sfm_address += slide;
	}
	
	return slide;
}

static void mapSharedCache()
{
	uint64_t cacheBaseAddress = 0;
	// quick check if a cache is already mapped into shared region
	if ( _shared_region_check_np(&cacheBaseAddress) == 0 ) {
		sSharedCache = (dyld_cache_header*)cacheBaseAddress;
		// if we don't understand the currently mapped shared cache, then ignore
#if __x86_64__
		const char* magic = (sHaswell ? ARCH_CACHE_MAGIC_H : ARCH_CACHE_MAGIC);
#else
		const char* magic = ARCH_CACHE_MAGIC;
#endif
		if ( strcmp(sSharedCache->magic, magic) != 0 ) {
			sSharedCache = NULL;
			if ( gLinkContext.verboseMapping ) {
				dyld::log("dyld: existing shared cached in memory is not compatible\n");
				return;
			}
		}
		// check if cache file is slidable
		const dyld_cache_header* header = sSharedCache;
		if ( (header->mappingOffset >= 0x48) && (header->slideInfoSize != 0) ) {
			// solve for slide by comparing loaded address to address of first region
			const uint8_t* loadedAddress = (uint8_t*)sSharedCache;
			const dyld_cache_mapping_info* const mappings = (dyld_cache_mapping_info*)(loadedAddress+header->mappingOffset);
			const uint8_t* preferedLoadAddress = (uint8_t*)(long)(mappings[0].address);
			sSharedCacheSlide = loadedAddress - preferedLoadAddress;
			dyld::gProcessInfo->sharedCacheSlide = sSharedCacheSlide;
			dyld::gProcessInfo->sharedCacheBaseAddress = cacheBaseAddress;
			//dyld::log("sSharedCacheSlide=0x%08lX, loadedAddress=%p, preferedLoadAddress=%p\n", sSharedCacheSlide, loadedAddress, preferedLoadAddress);
		}
		// if cache has a uuid, copy it 
		if ( header->mappingOffset >= 0x68 ) {
			memcpy(dyld::gProcessInfo->sharedCacheUUID, header->uuid, 16);
		}
		// verbose logging
		if ( gLinkContext.verboseMapping ) {
			dyld::log("dyld: re-using existing %s shared cache mapping\n", (header->cacheType == kDyldSharedCacheTypeDevelopment ? "development" : "production"));
		}
		if (header->mappingOffset >= 0x68) {
			dyld_kernel_image_info_t kernelCacheInfo;
			memcpy(&kernelCacheInfo.uuid[0], &sSharedCache->uuid[0], sizeof(uuid_t));
			kernelCacheInfo.load_addr = (uint64_t)sSharedCache;
			kernelCacheInfo.fsobjid.fid_objno = 0;
			kernelCacheInfo.fsobjid.fid_generation = 0;
			kernelCacheInfo.fsid.val[0] = 0;
			kernelCacheInfo.fsid.val[0] = 0;
#if 0
			task_register_dyld_shared_cache_image_info(mach_task_self(), kernelCacheInfo, false, false);
#endif
		}
	}
	else {
#if __i386__ || __x86_64__
		// <rdar://problem/5925940> Safe Boot should disable dyld shared cache
		// if we are in safe-boot mode and the cache was not made during this boot cycle,
		// delete the cache file
		uint32_t	safeBootValue = 0;
		size_t		safeBootValueSize = sizeof(safeBootValue);
		if ( (sysctlbyname("kern.safeboot", &safeBootValue, &safeBootValueSize, NULL, 0) == 0) && (safeBootValue != 0) ) {
			// user booted machine in safe-boot mode
			struct stat dyldCacheStatInfo;
			//  Don't use custom DYLD_SHARED_CACHE_DIR if provided, use standard path
			if ( my_stat(MACOSX_DYLD_SHARED_CACHE_DIR DYLD_SHARED_CACHE_BASE_NAME ARCH_NAME, &dyldCacheStatInfo) == 0 ) {
				struct timeval bootTimeValue;
				size_t bootTimeValueSize = sizeof(bootTimeValue);
				if ( (sysctlbyname("kern.boottime", &bootTimeValue, &bootTimeValueSize, NULL, 0) == 0) && (bootTimeValue.tv_sec != 0) ) {
					// if the cache file was created before this boot, then throw it away and let it rebuild itself
					if ( dyldCacheStatInfo.st_mtime < bootTimeValue.tv_sec ) {
						::unlink(MACOSX_DYLD_SHARED_CACHE_DIR DYLD_SHARED_CACHE_BASE_NAME ARCH_NAME);
						gLinkContext.sharedRegionMode = ImageLoader::kDontUseSharedRegion;
						return;
					}
				}
			}
		}
#endif
		// map in shared cache to shared region
		int fd = openSharedCacheFile();
		if ( fd != -1 ) {
			uint8_t firstPages[8192];
			if ( ::read(fd, firstPages, 8192) == 8192 ) {
				dyld_cache_header* header = (dyld_cache_header*)firstPages;
		#if __x86_64__
				const char* magic = (sHaswell ? ARCH_CACHE_MAGIC_H : ARCH_CACHE_MAGIC);
		#else
				const char* magic = ARCH_CACHE_MAGIC;
		#endif
				if ( strcmp(header->magic, magic) == 0 ) {
					const dyld_cache_mapping_info* const fileMappingsStart = (dyld_cache_mapping_info*)&firstPages[header->mappingOffset];
					const dyld_cache_mapping_info* const fileMappingsEnd = &fileMappingsStart[header->mappingCount];
	#if __IPHONE_OS_VERSION_MIN_REQUIRED
					if ( (header->mappingCount != 3)
					  || (header->mappingOffset > 256)
					  || (fileMappingsStart[0].fileOffset != 0)
					  || (fileMappingsStart[0].address != SHARED_REGION_BASE)
					  || ((fileMappingsStart[0].address + fileMappingsStart[0].size) > fileMappingsStart[1].address)
					  || ((fileMappingsStart[1].address + fileMappingsStart[1].size) > fileMappingsStart[2].address)
					  || ((fileMappingsStart[0].fileOffset + fileMappingsStart[0].size) != fileMappingsStart[1].fileOffset)
					  || ((fileMappingsStart[1].fileOffset + fileMappingsStart[1].size) != fileMappingsStart[2].fileOffset) )
						throw "dyld shared cache file is invalid";
	#endif
					shared_file_mapping_np	mappings[header->mappingCount];
					unsigned int mappingCount = header->mappingCount;
					int readWriteMappingIndex = -1;
					int readOnlyMappingIndex = -1;
					// validate that the cache file has not been truncated
					bool goodCache = false;
					struct stat stat_buf;
					if ( fstat(fd, &stat_buf) == 0 ) {
						goodCache = true;
						int i=0;
						for (const dyld_cache_mapping_info* p = fileMappingsStart; p < fileMappingsEnd; ++p, ++i) {
							mappings[i].sfm_address		= p->address;
							mappings[i].sfm_size		= p->size;
							mappings[i].sfm_file_offset	= p->fileOffset;
							mappings[i].sfm_max_prot	= p->maxProt;
							mappings[i].sfm_init_prot	= p->initProt;
							// rdar://problem/5694507 old update_dyld_shared_cache tool could make a cache file
							// that is not page aligned, but otherwise ok.
							if ( p->fileOffset+p->size > (uint64_t)(stat_buf.st_size+4095 & (-4096)) ) {
								dyld::log("dyld: shared cached file is corrupt: %s" DYLD_SHARED_CACHE_BASE_NAME ARCH_NAME "\n", sSharedCacheDir);
								goodCache = false;
							}
							if ( (mappings[i].sfm_init_prot & (VM_PROT_READ|VM_PROT_WRITE)) == (VM_PROT_READ|VM_PROT_WRITE) ) {
								readWriteMappingIndex = i;
							}
							if ( mappings[i].sfm_init_prot == VM_PROT_READ ) {
								readOnlyMappingIndex = i;
							}
						}
						// if shared cache is code signed, add a mapping for the code signature
						uint64_t signatureSize = header->codeSignatureSize;
						// zero size in header means signature runs to end-of-file
						if ( signatureSize == 0 )
							signatureSize = stat_buf.st_size - header->codeSignatureOffset;
						if ( signatureSize != 0 ) {
#if __arm__ || __arm64__
							size_t alignedSignatureSize		= (signatureSize+16383) & (-16384);
#else
							size_t alignedSignatureSize		= (signatureSize+4095) & (-4096);
#endif
							// <rdar://problem/23188073> validate code signature covers entire shared cache
							loadAndCheckCodeSignature(fd, mappingCount, mappings, header->codeSignatureOffset, alignedSignatureSize, firstPages, sizeof(firstPages));
						}
#if __IPHONE_OS_VERSION_MIN_REQUIRED
						else {
							throw "dyld shared cache file not code signed";
						}
#endif
					}
#if __MAC_OS_X_VERSION_MIN_REQUIRED	
					// sanity check that /usr/lib/libSystem.B.dylib stat() info matches cache
					if ( header->imagesCount * sizeof(dyld_cache_image_info) + header->imagesOffset < 8192 ) {
						bool foundLibSystem = false;
						if ( my_stat("/usr/lib/libSystem.B.dylib", &stat_buf) == 0 ) {
							const dyld_cache_image_info* images = (dyld_cache_image_info*)&firstPages[header->imagesOffset];
							const dyld_cache_image_info* const imagesEnd = &images[header->imagesCount];
							for (const dyld_cache_image_info* p = images; p < imagesEnd; ++p) {
 								if ( ((time_t)p->modTime == stat_buf.st_mtime) && ((ino_t)p->inode == stat_buf.st_ino) ) {
									foundLibSystem = true;
									break;
								}
							}					
						}
						if ( !sSharedCacheIgnoreInodeAndTimeStamp && !foundLibSystem ) {
							dyld::log("dyld: shared cached file was built against a different libSystem.dylib, ignoring cache.\n"
									"to update dyld shared cache run: 'sudo update_dyld_shared_cache' then reboot.\n");
							goodCache = false;
						}
					}
#endif
#if __IPHONE_OS_VERSION_MIN_REQUIRED
					{
						uint64_t lowAddress;
						uint64_t highAddress;
						getCacheBounds(mappingCount, mappings, lowAddress, highAddress);
						if ( (highAddress-lowAddress) > SHARED_REGION_SIZE ) 
							throw "dyld shared cache is too big to fit in shared region";
					}
#endif

					if ( goodCache && (readWriteMappingIndex == -1) ) {
						dyld::log("dyld: shared cached file is missing read/write mapping: %s" DYLD_SHARED_CACHE_BASE_NAME ARCH_NAME "\n", sSharedCacheDir);
						goodCache = false;
					}
					if ( goodCache && (readOnlyMappingIndex == -1) ) {
						dyld::log("dyld: shared cached file is missing read-only mapping: %s" DYLD_SHARED_CACHE_BASE_NAME ARCH_NAME "\n", sSharedCacheDir);
						goodCache = false;
					}
					if ( goodCache ) {
						long cacheSlide = 0;
						void* slideInfo = (void*)(long)(mappings[readOnlyMappingIndex].sfm_address + (header->slideInfoOffset - mappings[readOnlyMappingIndex].sfm_file_offset));;
						uint64_t slideInfoSize = header->slideInfoSize;
						// check if shared cache contains slid info
						if ( slideInfoSize != 0 ) {
							// <rdar://problem/8611968> don't slide shared cache if ASLR disabled (main executable didn't slide)
							if ( sMainExecutable->isPositionIndependentExecutable() && (sMainExecutable->getSlide() == 0) ) {
								cacheSlide = 0;
							}
							else {
								// generate random slide amount
								cacheSlide = pickCacheSlide(mappingCount, mappings);
							}

							slideInfo = (void*)((uint8_t*)slideInfo + cacheSlide);
							// add VM_PROT_SLIDE bit to __DATA area of cache
							mappings[readWriteMappingIndex].sfm_max_prot  |= VM_PROT_SLIDE;
							mappings[readWriteMappingIndex].sfm_init_prot |= VM_PROT_SLIDE;
						}
						if ( gLinkContext.verboseMapping ) {
							dyld::log("dyld: calling _shared_region_map_and_slide_np() with regions:\n");
							for (int i=0; i < mappingCount; ++i) {
								dyld::log("   address=0x%08llX, size=0x%08llX, fileOffset=0x%08llX\n", mappings[i].sfm_address, mappings[i].sfm_size, mappings[i].sfm_file_offset);
							}
						}

						if (_shared_region_map_and_slide_np(fd, mappingCount, mappings, cacheSlide, slideInfo, slideInfoSize) == 0) {
							// successfully mapped cache into shared region
							sSharedCache = (dyld_cache_header*)mappings[0].sfm_address;
							sSharedCacheSlide = cacheSlide;
							dyld::gProcessInfo->sharedCacheSlide = cacheSlide;
							dyld::gProcessInfo->sharedCacheBaseAddress = mappings[0].sfm_address;
							//dyld::log("sSharedCache=%p sSharedCacheSlide=0x%08lX\n", sSharedCache, sSharedCacheSlide);
							// if cache has a uuid, copy it
							if ( header->mappingOffset >= 0x68 ) {
								const bool privateSharedCache =  gLinkContext.sharedRegionMode == ImageLoader::kUsePrivateSharedRegion;
								memcpy(dyld::gProcessInfo->sharedCacheUUID, header->uuid, 16);
								dyld_kernel_image_info_t kernelCacheInfo;
								memcpy(&kernelCacheInfo.uuid[0], &sSharedCache->uuid[0], sizeof(uuid_t));
								kernelCacheInfo.load_addr = (uint64_t)sSharedCache;
								kernelCacheInfo.fsobjid.fid_objno = 0;
								kernelCacheInfo.fsobjid.fid_generation = 0;
								kernelCacheInfo.fsid.val[0] = 0;
								kernelCacheInfo.fsid.val[0] = 0;
								if (privateSharedCache) {
									kernelCacheInfo.fsobjid = *(fsobj_id_t*)(&stat_buf.st_ino);
									struct statfs statfs_buf;
									if ( fstatfs(fd, &statfs_buf) == 0 ) {
										kernelCacheInfo.fsid = statfs_buf.f_fsid;
									}
								}
#if 0
								task_register_dyld_shared_cache_image_info(mach_task_self(), kernelCacheInfo, false, privateSharedCache);
#endif
							}
						}
						else {
#if __IPHONE_OS_VERSION_MIN_REQUIRED
							throwf("dyld shared cache could not be mapped.  errno=%d, slide=0x%08lX, slideInfo=%p, slideInfoSize=0x%08llX, mappingCount=%u, "
								   "address/size/off/init/max [0]=0x%0llX/0x%0llX/0x%0llX/0x%02X/0x%02X, [1]=0x%0llX/0x%0llX/0x%0llX/0x%02X/0x%02X, [2]=0x%0llX/0x%0llX/0x%0llX/0x%02X/0x%02X",
								   errno, cacheSlide, slideInfo, slideInfoSize, mappingCount,
								   mappings[0].sfm_address, mappings[0].sfm_size, mappings[0].sfm_file_offset, mappings[0].sfm_init_prot, mappings[0].sfm_max_prot,
								   mappings[1].sfm_address, mappings[1].sfm_size, mappings[1].sfm_file_offset, mappings[1].sfm_init_prot, mappings[1].sfm_max_prot,
								   mappings[2].sfm_address, mappings[2].sfm_size, mappings[2].sfm_file_offset, mappings[2].sfm_init_prot, mappings[2].sfm_max_prot);
#endif
							if ( gLinkContext.verboseMapping ) 
								dyld::log("dyld: shared cached file could not be mapped\n");
						}
					}
				}
				else {
					if ( gLinkContext.verboseMapping ) 
						dyld::log("dyld: shared cached file is invalid\n");
				}
			}
			else {
				if ( gLinkContext.verboseMapping ) 
					dyld::log("dyld: shared cached file cannot be read\n");
			}
			close(fd);
		}
		else {
			if ( gLinkContext.verboseMapping ) 
				dyld::log("dyld: shared cached file cannot be opened\n");
		}
	}
	
	// remember if dyld loaded at same address as when cache built
	if ( sSharedCache != NULL ) {
		gLinkContext.dyldLoadedAtSameAddressNeededBySharedCache = ((uintptr_t)(sSharedCache->dyldBaseAddress) == (uintptr_t)&_mh_dylinker_header);
	}
	
	// tell gdb where the shared cache is
	if ( sSharedCache != NULL ) {
		const dyld_cache_mapping_info* const start = (dyld_cache_mapping_info*)((uint8_t*)sSharedCache + sSharedCache->mappingOffset);
		dyld_shared_cache_ranges.sharedRegionsCount = sSharedCache->mappingCount;
		// only room to tell gdb about first four regions
		if ( dyld_shared_cache_ranges.sharedRegionsCount > 4 )
			dyld_shared_cache_ranges.sharedRegionsCount = 4;
		const dyld_cache_mapping_info* const end = &start[dyld_shared_cache_ranges.sharedRegionsCount];
		int index = 0;
		for (const dyld_cache_mapping_info* p = start; p < end; ++p, ++index ) {
			dyld_shared_cache_ranges.ranges[index].start = p->address+sSharedCacheSlide;
			dyld_shared_cache_ranges.ranges[index].length = p->size;
			if ( gLinkContext.verboseMapping ) {
				dyld::log("        0x%08llX->0x%08llX %s%s%s init=%x, max=%x\n", 
					p->address+sSharedCacheSlide, p->address+sSharedCacheSlide+p->size-1,
					((p->initProt & VM_PROT_READ) ? "read " : ""),
					((p->initProt & VM_PROT_WRITE) ? "write " : ""),
					((p->initProt & VM_PROT_EXECUTE) ? "execute " : ""),  p->initProt, p->maxProt);
			}
		#if __i386__
			// If a non-writable and executable region is found in the R/W shared region, then this is __IMPORT segments
			// This is an old cache.  Make writable.  dyld no longer supports turn W on and off as it binds
			if ( (p->initProt == (VM_PROT_READ|VM_PROT_EXECUTE)) && ((p->address & 0xF0000000) == 0xA0000000) ) {
				if ( p->size != 0 ) {
					vm_prot_t prot = VM_PROT_EXECUTE | PROT_READ | VM_PROT_WRITE;
					vm_protect(mach_task_self(), p->address, p->size, false, prot);
					if ( gLinkContext.verboseMapping ) {
						dyld::log("%18s at 0x%08llX->0x%08llX altered permissions to %c%c%c\n", "", p->address, 
							p->address+p->size-1,
							(prot & PROT_READ) ? 'r' : '.',  (prot & PROT_WRITE) ? 'w' : '.',  (prot & PROT_EXEC) ? 'x' : '.' );
					}
				}
			}
		#endif
		}
		if ( gLinkContext.verboseMapping ) {
			// list the code blob
			dyld_cache_header* header = (dyld_cache_header*)sSharedCache;
			uint64_t signatureSize = header->codeSignatureSize;
			// zero size in header means signature runs to end-of-file
			if ( signatureSize == 0 ) {
				struct stat stat_buf;
				// FIXME: need size of cache file actually used
				if ( my_stat(IPHONE_DYLD_SHARED_CACHE_DIR DYLD_SHARED_CACHE_BASE_NAME ARCH_NAME, &stat_buf) == 0 )
					signatureSize = stat_buf.st_size - header->codeSignatureOffset;
			}
			if ( signatureSize != 0 ) {
				const dyld_cache_mapping_info* const last = &start[dyld_shared_cache_ranges.sharedRegionsCount-1];
				uint64_t codeBlobStart = last->address + last->size;
				dyld::log("        0x%08llX->0x%08llX (code signature)\n", codeBlobStart, codeBlobStart+signatureSize);
			}
		}
	#if SUPPORT_ACCELERATE_TABLES
		if ( !dylibsCanOverrideCache() && !sDisableAcceleratorTables && (sSharedCache->mappingOffset > 0x80) && (sSharedCache->accelerateInfoAddr != 0) ) {
			sAllCacheImagesProxy = ImageLoaderMegaDylib::makeImageLoaderMegaDylib(sSharedCache, sSharedCacheSlide, gLinkContext);
		}
	#endif
	}
}
#endif // #if DYLD_SHARED_CACHE_SUPPORT



// create when NSLinkModule is called for a second time on a bundle
ImageLoader* cloneImage(ImageLoader* image)
{
	// open file (automagically closed when this function exits)
	FileOpener file(image->getPath());
	
	struct stat stat_buf;
	if ( fstat(file.getFileDescriptor(), &stat_buf) == -1)
		throw "stat error";
	
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
	context.origin				= NULL;
	context.rpath				= NULL;
	return loadPhase6(file.getFileDescriptor(), stat_buf, image->getPath(), context);
}


ImageLoader* loadFromMemory(const uint8_t* mem, uint64_t len, const char* moduleName)
{
	// if fat wrapper, find usable sub-file
	const fat_header* memStartAsFat = (fat_header*)mem;
	uint64_t fileOffset = 0;
	uint64_t fileLength = len;
	if ( memStartAsFat->magic == OSSwapBigToHostInt32(FAT_MAGIC) ) {
		if ( fatFindBest(memStartAsFat, &fileOffset, &fileLength) ) {
			mem = &mem[fileOffset];
			len = fileLength;
		}
		else {
			throw "no matching architecture in universal wrapper";
		}
	}

	// try each loader
	if ( isCompatibleMachO(mem, moduleName) ) {
		ImageLoader* image = ImageLoaderMachO::instantiateFromMemory(moduleName, (macho_header*)mem, len, gLinkContext);
		// don't add bundles to global list, they can be loaded but not linked.  When linked it will be added to list
		if ( ! image->isBundle() ) 
			addImage(image);
		return image;
	}
	
	// try other file formats here...
	
	// throw error about what was found
	switch (*(uint32_t*)mem) {
		case MH_MAGIC:
		case MH_CIGAM:
		case MH_MAGIC_64:
		case MH_CIGAM_64:
			throw "mach-o, but wrong architecture";
		default:
		throwf("unknown file type, first eight bytes: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X", 
			mem[0], mem[1], mem[2], mem[3], mem[4], mem[5], mem[6],mem[7]);
	}
}


void registerAddCallback(ImageCallback func)
{
	// now add to list to get notified when any more images are added
	sAddImageCallbacks.push_back(func);
	
	// call callback with all existing images
	for (std::vector<ImageLoader*>::iterator it=sAllImages.begin(); it != sAllImages.end(); it++) {
		ImageLoader* image = *it;
		if ( image->getState() >= dyld_image_state_bound && image->getState() < dyld_image_state_terminated )
			(*func)(image->machHeader(), image->getSlide());
	}
#if SUPPORT_ACCELERATE_TABLES
	if ( sAllCacheImagesProxy != NULL ) {
		dyld_image_info	infos[allImagesCount()+1];
		unsigned cacheCount = sAllCacheImagesProxy->appendImagesToNotify(dyld_image_state_bound, true, infos);
		for (unsigned i=0; i < cacheCount; ++i) {
			(*func)(infos[i].imageLoadAddress, sSharedCacheSlide);
		}
	}
#endif
}

void registerRemoveCallback(ImageCallback func)
{
	// <rdar://problem/15025198> ignore calls to register a notification during a notification
	if ( sRemoveImageCallbacksInUse )
		return;
	sRemoveImageCallbacks.push_back(func);
}

void clearErrorMessage()
{
	error_string[0] = '\0';
}

void setErrorMessage(const char* message)
{
	// save off error message in global buffer for CrashReporter to find
	strlcpy(error_string, message, sizeof(error_string));
}

const char* getErrorMessage()
{
	return error_string;
}

void halt(const char* message)
{
	dyld::log("dyld: %s\n", message);
	setErrorMessage(message);
	dyld::gProcessInfo->errorMessage = error_string;
	if ( !gLinkContext.startedInitializingMainExecutable )
		dyld::gProcessInfo->terminationFlags = 1;
	else
		dyld::gProcessInfo->terminationFlags = 0;
#if 0
	char payloadBuffer[EXIT_REASON_PAYLOAD_MAX_LEN];
	dyld_abort_payload* payload = (dyld_abort_payload*)payloadBuffer;
	payload->version               = 1;
	payload->flags                 = gLinkContext.startedInitializingMainExecutable ? 0 : 1;
	payload->targetDylibPathOffset = 0;
	payload->clientPathOffset      = 0;
	payload->symbolOffset          = 0;
	int payloadSize = sizeof(dyld_abort_payload);

	if ( dyld::gProcessInfo->errorTargetDylibPath != NULL ) {
		payload->targetDylibPathOffset = payloadSize;
		payloadSize += strlcpy(&payloadBuffer[payloadSize], dyld::gProcessInfo->errorTargetDylibPath, sizeof(payloadBuffer)-payloadSize) + 1;
	}
	if ( dyld::gProcessInfo->errorClientOfDylibPath != NULL ) {
		payload->clientPathOffset = payloadSize;
		payloadSize += strlcpy(&payloadBuffer[payloadSize], dyld::gProcessInfo->errorClientOfDylibPath, sizeof(payloadBuffer)-payloadSize) + 1;
	}
	if ( dyld::gProcessInfo->errorSymbol != NULL ) {
		payload->symbolOffset = payloadSize;
		payloadSize += strlcpy(&payloadBuffer[payloadSize], dyld::gProcessInfo->errorSymbol, sizeof(payloadBuffer)-payloadSize) + 1;
	}
	char truncMessage[EXIT_REASON_USER_DESC_MAX_LEN];
	strlcpy(truncMessage, message, EXIT_REASON_USER_DESC_MAX_LEN);
	abort_with_payload(OS_REASON_DYLD, dyld::gProcessInfo->errorKind ? dyld::gProcessInfo->errorKind : DYLD_EXIT_REASON_OTHER, payloadBuffer, payloadSize, truncMessage, 0);
#endif
	kill(0, SIGABRT);
}

static void setErrorStrings(unsigned errorCode, const char* errorClientOfDylibPath,
								const char* errorTargetDylibPath, const char* errorSymbol)
{
	dyld::gProcessInfo->errorKind = errorCode;
	dyld::gProcessInfo->errorClientOfDylibPath = errorClientOfDylibPath;
	dyld::gProcessInfo->errorTargetDylibPath = errorTargetDylibPath;
	dyld::gProcessInfo->errorSymbol = errorSymbol;
}


uintptr_t bindLazySymbol(const mach_header* mh, uintptr_t* lazyPointer)
{
	uintptr_t result = 0;
	// acquire read-lock on dyld's data structures
#if 0 // rdar://problem/3811777 turn off locking until deadlock is resolved
	if ( gLibSystemHelpers != NULL ) 
		(*gLibSystemHelpers->lockForReading)();
#endif
	// lookup and bind lazy pointer and get target address
	try {
		ImageLoader* target;
	#if __i386__
		// fast stubs pass NULL for mh and image is instead found via the location of stub (aka lazyPointer)
		if ( mh == NULL )
			target = dyld::findImageContainingAddress(lazyPointer);
		else
			target = dyld::findImageByMachHeader(mh);
	#else
		// note, target should always be mach-o, because only mach-o lazy handler wired up to this
		target = dyld::findImageByMachHeader(mh);
	#endif
		if ( target == NULL )
			throwf("image not found for lazy pointer at %p", lazyPointer);
		result = target->doBindLazySymbol(lazyPointer, gLinkContext);
	}
	catch (const char* message) {
		dyld::log("dyld: lazy symbol binding failed: %s\n", message);
		halt(message);
	}
	// release read-lock on dyld's data structures
#if 0
	if ( gLibSystemHelpers != NULL ) 
		(*gLibSystemHelpers->unlockForReading)();
#endif
	// return target address to glue which jumps to it with real parameters restored
	return result;
}


uintptr_t fastBindLazySymbol(ImageLoader** imageLoaderCache, uintptr_t lazyBindingInfoOffset)
{
	uintptr_t result = 0;
	// get image 
	if ( *imageLoaderCache == NULL ) {
		// save in cache
		*imageLoaderCache = dyld::findMappedRange((uintptr_t)imageLoaderCache);
		if ( *imageLoaderCache == NULL ) {
#if SUPPORT_ACCELERATE_TABLES
		if ( sAllCacheImagesProxy != NULL ) {
			const mach_header* mh;
			const char* path;
			unsigned index;
			if ( sAllCacheImagesProxy->addressInCache(imageLoaderCache, &mh, &path, &index) ) {
				result = sAllCacheImagesProxy->bindLazy(lazyBindingInfoOffset, gLinkContext, mh, index);
				if ( result == 0 ) {
					halt("dyld: lazy symbol binding failed for image in dyld shared\n");
				}
				return result;
			}
		}
#endif
			const char* message = "fast lazy binding from unknown image";
			dyld::log("dyld: %s\n", message);
			halt(message);
		}
	}
	
	// bind lazy pointer and return it
	try {
		result = (*imageLoaderCache)->doBindFastLazySymbol((uint32_t)lazyBindingInfoOffset, gLinkContext, 
								(dyld::gLibSystemHelpers != NULL) ? dyld::gLibSystemHelpers->acquireGlobalDyldLock : NULL,
								(dyld::gLibSystemHelpers != NULL) ? dyld::gLibSystemHelpers->releaseGlobalDyldLock : NULL);
	}
	catch (const char* message) {
		dyld::log("dyld: lazy symbol binding failed: %s\n", message);
		halt(message);
	}

	// return target address to glue which jumps to it with real parameters restored
	return result;
}



void registerUndefinedHandler(UndefinedHandler handler)
{
	sUndefinedHandler = handler;
}

static void undefinedHandler(const char* symboName)
{
	if ( sUndefinedHandler != NULL ) {
		(*sUndefinedHandler)(symboName);
	}
}

static bool findExportedSymbol(const char* name, bool onlyInCoalesced, const ImageLoader::Symbol** sym, const ImageLoader** image)
{
	// search all images in order
	const ImageLoader* firstWeakImage = NULL;
	const ImageLoader::Symbol* firstWeakSym = NULL;
	const size_t imageCount = sAllImages.size();
	for(size_t i=0; i < imageCount; ++i) {
		ImageLoader* anImage = sAllImages[i];
		// the use of inserted libraries alters search order
		// so that inserted libraries are found before the main executable
		if ( sInsertedDylibCount > 0 ) {
			if ( i < sInsertedDylibCount )
				anImage = sAllImages[i+1];
			else if ( i == sInsertedDylibCount )
				anImage = sAllImages[0];
		}
		if ( ! anImage->hasHiddenExports() && (!onlyInCoalesced || anImage->hasCoalescedExports()) ) {
			*sym = anImage->findExportedSymbol(name, false, image);
			if ( *sym != NULL ) {
				// if weak definition found, record first one found
				if ( ((*image)->getExportedSymbolInfo(*sym) & ImageLoader::kWeakDefinition) != 0 ) {
					if ( firstWeakImage == NULL ) {
						firstWeakImage = *image;
						firstWeakSym = *sym;
					}
				}
				else {
					// found non-weak, so immediately return with it
					return true;
				}
			}
		}
	}
	if ( firstWeakSym != NULL ) {
		// found a weak definition, but no non-weak, so return first weak found
		*sym = firstWeakSym;
		*image = firstWeakImage;
		return true;
	}
#if SUPPORT_ACCELERATE_TABLES
	if ( sAllCacheImagesProxy != NULL ) {
		if ( sAllCacheImagesProxy->flatFindSymbol(name, onlyInCoalesced, sym, image) )
			return true;
	}
#endif

	return false;
}

bool flatFindExportedSymbol(const char* name, const ImageLoader::Symbol** sym, const ImageLoader** image)
{
	return findExportedSymbol(name, false, sym, image);
}

bool findCoalescedExportedSymbol(const char* name, const ImageLoader::Symbol** sym, const ImageLoader** image)
{
	return findExportedSymbol(name, true, sym, image);
}


bool flatFindExportedSymbolWithHint(const char* name, const char* librarySubstring, const ImageLoader::Symbol** sym, const ImageLoader** image)
{
	// search all images in order
	const size_t imageCount = sAllImages.size();
	for(size_t i=0; i < imageCount; ++i){
		ImageLoader* anImage = sAllImages[i];
		// only look at images whose paths contain the hint string (NULL hint string is wildcard)
		if ( ! anImage->isBundle() && ((librarySubstring==NULL) || (strstr(anImage->getPath(), librarySubstring) != NULL)) ) {
			*sym = anImage->findExportedSymbol(name, false, image);
			if ( *sym != NULL ) {
				return true;
			}
		}
	}
	return false;
}


unsigned int getCoalescedImages(ImageLoader* images[], unsigned imageIndex[])
{
	unsigned int count = 0;
	for (std::vector<ImageLoader*>::iterator it=sAllImages.begin(); it != sAllImages.end(); it++) {
		ImageLoader* image = *it;
		if ( image->participatesInCoalescing() ) {
			images[count] = *it;
			imageIndex[count] = 0;
			++count;
		}
	}
#if SUPPORT_ACCELERATE_TABLES
	if ( sAllCacheImagesProxy != NULL ) {
		sAllCacheImagesProxy->appendImagesNeedingCoalescing(images, imageIndex, count);
	}
#endif
	return count;
}


static ImageLoader::MappedRegion* getMappedRegions(ImageLoader::MappedRegion* regions)
{
	ImageLoader::MappedRegion* end = regions;
	for (std::vector<ImageLoader*>::iterator it=sAllImages.begin(); it != sAllImages.end(); it++) {
		(*it)->getMappedRegions(end);
	}
	return end;
}

void registerImageStateSingleChangeHandler(dyld_image_states state, dyld_image_state_change_handler handler)
{
	// mark the image that the handler is in as never-unload because dyld has a reference into it
	ImageLoader* handlerImage = findImageContainingAddress((void*)handler);
	if ( handlerImage != NULL )
		handlerImage->setNeverUnload();

	// add to list of handlers
	std::vector<dyld_image_state_change_handler>* handlers = stateToHandlers(state, sSingleHandlers);
	if ( handlers != NULL ) {
        // <rdar://problem/10332417> need updateAllImages() to be last in dyld_image_state_mapped list
        // so that if ObjC adds a handler that prevents a load, it happens before the gdb list is updated
        if ( state == dyld_image_state_mapped )
            handlers->insert(handlers->begin(), handler);
        else
            handlers->push_back(handler);

		// call callback with all existing images
		for (std::vector<ImageLoader*>::iterator it=sAllImages.begin(); it != sAllImages.end(); it++) {
			ImageLoader* image = *it;
			dyld_image_info	 info;
			info.imageLoadAddress	= image->machHeader();
			info.imageFilePath		= image->getRealPath();
			info.imageFileModDate	= image->lastModified();
			// should only call handler if state == image->state
			if ( image->getState() == state )
				(*handler)(state, 1, &info);
			// ignore returned string, too late to do anything
		}
	}
}

void registerImageStateBatchChangeHandler(dyld_image_states state, dyld_image_state_change_handler handler)
{
	// mark the image that the handler is in as never-unload because dyld has a reference into it
	ImageLoader* handlerImage = findImageContainingAddress((void*)handler);
	if ( handlerImage != NULL )
		handlerImage->setNeverUnload();

	// add to list of handlers
	std::vector<dyld_image_state_change_handler>* handlers = stateToHandlers(state, sBatchHandlers);
	if ( handlers != NULL ) {
		// insert at front, so that gdb handler is always last
		handlers->insert(handlers->begin(), handler);
		
		// call callback with all existing images
		try {
			notifyBatchPartial(state, true, handler, false, false);
		}
		catch (const char* msg) {
			// ignore request to abort during registration
		}
	}
}


void registerObjCNotifiers(_dyld_objc_notify_mapped mapped, _dyld_objc_notify_init init, _dyld_objc_notify_unmapped unmapped)
{
	// record functions to call
	sNotifyObjCMapped	= mapped;
	sNotifyObjCInit		= init;
	sNotifyObjCUnmapped = unmapped;

	// call 'mapped' function with all images mapped so far
	try {
		notifyBatchPartial(dyld_image_state_bound, true, NULL, false, true);
	}
	catch (const char* msg) {
		// ignore request to abort during registration
	}
}

bool sharedCacheUUID(uuid_t uuid)
{
#if DYLD_SHARED_CACHE_SUPPORT
	if ( sSharedCache == NULL )
		return false;

	memcpy(uuid, sSharedCache->uuid, 16);
	return true;
#else
	return false;
#endif
}

#if SUPPORT_ACCELERATE_TABLES

bool dlopenFromCache(const char* path, int mode, void** handle)
{
	if ( sAllCacheImagesProxy == NULL )
		return false;
	bool result = sAllCacheImagesProxy->dlopenFromCache(gLinkContext, path, mode, handle);
	if ( !result && (strchr(path, '/') == NULL) ) {
		// POSIX says you can call dlopen() with a leaf name (e.g. dlopen("libz.dylb"))
		char fallbackPath[PATH_MAX];
		strcpy(fallbackPath, "/usr/lib/");
		strlcat(fallbackPath, path, PATH_MAX);
		result = sAllCacheImagesProxy->dlopenFromCache(gLinkContext, fallbackPath, mode, handle);
	}
	return result;
}

bool makeCacheHandle(ImageLoader* image, unsigned cacheIndex, int mode, void** result)
{
	if ( sAllCacheImagesProxy == NULL )
		return false;
	return sAllCacheImagesProxy->makeCacheHandle(gLinkContext, cacheIndex, mode, result);
}

bool isCacheHandle(void* handle)
{
	if ( sAllCacheImagesProxy == NULL )
		return false;
	return sAllCacheImagesProxy->isCacheHandle(handle, NULL, NULL);
}

bool isPathInCache(const char* path)
{
	if ( sAllCacheImagesProxy == NULL )
		return false;
	unsigned index;
	return sAllCacheImagesProxy->hasDylib(path, &index);
}

const char* getPathFromIndex(unsigned cacheIndex)
{
	if ( sAllCacheImagesProxy == NULL )
		return NULL;
	return sAllCacheImagesProxy->getIndexedPath(cacheIndex);
}

void* dlsymFromCache(void* handle, const char* symName, unsigned index)
{
	if ( sAllCacheImagesProxy == NULL )
		return NULL;
	return sAllCacheImagesProxy->dlsymFromCache(gLinkContext, handle, symName, index);
}

bool addressInCache(const void* address, const mach_header** mh, const char** path, unsigned* index)
{
	if ( sAllCacheImagesProxy == NULL )
		return false;
	unsigned ignore;
	return sAllCacheImagesProxy->addressInCache(address, mh, path, index ? index : &ignore);
}

bool findUnwindSections(const void* addr, dyld_unwind_sections* info)
{
	if ( sAllCacheImagesProxy == NULL )
		return false;
	return sAllCacheImagesProxy->findUnwindSections(addr, info);
}

bool dladdrFromCache(const void* address, Dl_info* info)
{
	if ( sAllCacheImagesProxy == NULL )
		return false;
	return sAllCacheImagesProxy->dladdrFromCache(address, info);
}
#endif

static ImageLoader* libraryLocator(const char* libraryName, bool search, const char* origin, const ImageLoader::RPathChain* rpaths, unsigned& cacheIndex)
{
	dyld::LoadContext context;
	context.useSearchPaths		= search;
	context.useFallbackPaths	= search;
	context.useLdLibraryPath	= false;
	context.implicitRPath		= false;
	context.matchByInstallName	= false;
	context.dontLoad			= false;
	context.mustBeBundle		= false;
	context.mustBeDylib			= true;
	context.canBePIE			= false;
	context.origin				= origin;
	context.rpath				= rpaths;
	return load(libraryName, context, cacheIndex);
}

static const char* basename(const char* path)
{
    const char* last = path;
    for (const char* s = path; *s != '\0'; s++) {
        if (*s == '/') 
			last = s+1;
    }
    return last;
}

static void setContext(const macho_header* mainExecutableMH, int argc, const char* argv[], const char* envp[], const char* apple[])
{
	gLinkContext.loadLibrary			= &libraryLocator;
	gLinkContext.terminationRecorder	= &terminationRecorder;
	gLinkContext.flatExportFinder		= &flatFindExportedSymbol;
	gLinkContext.coalescedExportFinder	= &findCoalescedExportedSymbol;
	gLinkContext.getCoalescedImages		= &getCoalescedImages;
	gLinkContext.undefinedHandler		= &undefinedHandler;
	gLinkContext.getAllMappedRegions	= &getMappedRegions;
	gLinkContext.bindingHandler			= NULL;
	gLinkContext.notifySingle			= &notifySingle;
	gLinkContext.notifyBatch			= &notifyBatch;
	gLinkContext.removeImage			= &removeImage;
	gLinkContext.registerDOFs			= &registerDOFs;
	gLinkContext.clearAllDepths			= &clearAllDepths;
	gLinkContext.printAllDepths			= &printAllDepths;
	gLinkContext.imageCount				= &imageCount;
	gLinkContext.setNewProgramVars		= &setNewProgramVars;
#if DYLD_SHARED_CACHE_SUPPORT
	gLinkContext.inSharedCache			= &inSharedCache;
#endif
	gLinkContext.setErrorStrings		= &setErrorStrings;
#if SUPPORT_OLD_CRT_INITIALIZATION
	gLinkContext.setRunInitialzersOldWay= &setRunInitialzersOldWay;
#endif
	gLinkContext.findImageContainingAddress	= &findImageContainingAddress;
	gLinkContext.addDynamicReference	= &addDynamicReference;
#if SUPPORT_ACCELERATE_TABLES
	gLinkContext.notifySingleFromCache	= &notifySingleFromCache;
	gLinkContext.getPreInitNotifyHandler= &getPreInitNotifyHandler;
	gLinkContext.getBoundBatchHandler   = &getBoundBatchHandler;
#endif
	gLinkContext.bindingOptions			= ImageLoader::kBindingNone;
	gLinkContext.argc					= argc;
	gLinkContext.argv					= argv;
	gLinkContext.envp					= envp;
	gLinkContext.apple					= apple;
	gLinkContext.progname				= (argv[0] != NULL) ? basename(argv[0]) : "";
	gLinkContext.programVars.mh			= mainExecutableMH;
	gLinkContext.programVars.NXArgcPtr	= &gLinkContext.argc;
	gLinkContext.programVars.NXArgvPtr	= &gLinkContext.argv;
	gLinkContext.programVars.environPtr	= &gLinkContext.envp;
	gLinkContext.programVars.__prognamePtr=&gLinkContext.progname;
	gLinkContext.mainExecutable			= NULL;
	gLinkContext.imageSuffix			= NULL;
	gLinkContext.dynamicInterposeArray	= NULL;
	gLinkContext.dynamicInterposeCount	= 0;
	gLinkContext.prebindUsage			= ImageLoader::kUseAllPrebinding;
#if TARGET_IPHONE_SIMULATOR
	gLinkContext.sharedRegionMode		= ImageLoader::kDontUseSharedRegion;
#else
	gLinkContext.sharedRegionMode		= ImageLoader::kUseSharedRegion;
#endif
}



//
// Look for a special segment in the mach header. 
// Its presences means that the binary wants to have DYLD ignore
// DYLD_ environment variables.
//
static bool hasRestrictedSegment(const macho_header* mh)
{
	const uint32_t cmd_count = mh->ncmds;
	const struct load_command* const cmds = (struct load_command*)(((char*)mh)+sizeof(macho_header));
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_SEGMENT_COMMAND:
			{
				const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
				
				//dyld::log("seg name: %s\n", seg->segname);
				if (strcmp(seg->segname, "__RESTRICT") == 0) {
					const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
					const struct macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
					for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
						if (strcmp(sect->sectname, "__restrict") == 0) 
							return true;
					}
				}
			}
			break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
		
	return false;
}

#if __IPHONE_OS_VERSION_MIN_REQUIRED
static bool isFairPlayEncrypted(const macho_header* mh)
{
	const uint32_t cmd_count = mh->ncmds;
	const struct load_command* const cmds = (struct load_command*)(((char*)mh)+sizeof(macho_header));
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		if ( cmd->cmd == LC_ENCRYPT_COMMAND ) {
			const encryption_info_command* enc = (encryption_info_command*)cmd;
			return (enc->cryptid != 0);
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}

	return false;
}
#endif

#if SUPPORT_VERSIONED_PATHS

static bool readFirstPage(const char* dylibPath, uint8_t firstPage[4096]) 
{
	firstPage[0] = 0;
	// open file (automagically closed when this function exits)
	FileOpener file(dylibPath);

	if ( file.getFileDescriptor() == -1 ) 
		return false;
	
	if ( pread(file.getFileDescriptor(), firstPage, 4096, 0) != 4096 )
		return false;

	// if fat wrapper, find usable sub-file
	const fat_header* fileStartAsFat = (fat_header*)firstPage;
	if ( fileStartAsFat->magic == OSSwapBigToHostInt32(FAT_MAGIC) ) {
		uint64_t fileOffset;
		uint64_t fileLength;
		if ( fatFindBest(fileStartAsFat, &fileOffset, &fileLength) ) {
			if ( pread(file.getFileDescriptor(), firstPage, 4096, fileOffset) != 4096 )
				return false;
		}
		else {
			return false;
		}
	}
	
	return true;
}

//
// Peeks at a dylib file and returns its current_version and install_name.
// Returns false on error.
//
static bool getDylibVersionAndInstallname(const char* dylibPath, uint32_t* version, char* installName)
{
	uint8_t firstPage[4096];
	const macho_header* mh = (macho_header*)firstPage;
	if ( !readFirstPage(dylibPath, firstPage) ) {
	#if DYLD_SHARED_CACHE_SUPPORT
		// If file cannot be read, check to see if path is in shared cache
		const macho_header* mhInCache;
		const char*			pathInCache;
		long				slideInCache;
		if ( !findInSharedCacheImage(dylibPath, true, NULL, &mhInCache, &pathInCache, &slideInCache) )
			return false;
		mh = mhInCache;
	#else
		return false;
	#endif
	}

	// check mach-o header
	if ( mh->magic != sMainExecutableMachHeader->magic ) 
		return false;
	if ( mh->cputype != sMainExecutableMachHeader->cputype )
		return false;

	// scan load commands for LC_ID_DYLIB
	const uint32_t cmd_count = mh->ncmds;
	const struct load_command* const cmds = (struct load_command*)(((char*)mh)+sizeof(macho_header));
	const struct load_command* const cmdsReadEnd = (struct load_command*)(((char*)mh)+4096);
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_ID_DYLIB:
			{
				const struct dylib_command* id = (struct dylib_command*)cmd;
				*version = id->dylib.current_version;
				if ( installName != NULL )
					strlcpy(installName, (char *)id + id->dylib.name.offset, PATH_MAX);
				return true;
			}
			break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
		if ( cmd > cmdsReadEnd )
			return false;
	}
	
	return false;
}
#endif // SUPPORT_VERSIONED_PATHS
						

#if 0
static void printAllImages()
{
	dyld::log("printAllImages()\n");
	for (std::vector<ImageLoader*>::iterator it=sAllImages.begin(); it != sAllImages.end(); it++) {
		ImageLoader* image = *it;
		dyld_image_states imageState = image->getState();
		dyld::log("  state=%d, dlopen-count=%d, never-unload=%d, in-use=%d, name=%s\n",
				  imageState, image->dlopenCount(), image->neverUnload(), image->isMarkedInUse(), image->getShortName());
	}
}
#endif

void link(ImageLoader* image, bool forceLazysBound, bool neverUnload, const ImageLoader::RPathChain& loaderRPaths, unsigned cacheIndex)
{
	// add to list of known images.  This did not happen at creation time for bundles
	if ( image->isBundle() && !image->isLinked() )
		addImage(image);

	// we detect root images as those not linked in yet 
	if ( !image->isLinked() )
		addRootImage(image);
	
	// process images
	try {
		const char* path = image->getPath();
#if SUPPORT_ACCELERATE_TABLES
		if ( image == sAllCacheImagesProxy )
			path = sAllCacheImagesProxy->getIndexedPath(cacheIndex);
#endif
		image->link(gLinkContext, forceLazysBound, false, neverUnload, loaderRPaths, path);
	}
	catch (const char* msg) {
		garbageCollectImages();
		throw;
	}
}


void runInitializers(ImageLoader* image)
{
	// do bottom up initialization
	ImageLoader::InitializerTimingList initializerTimes[allImagesCount()];
	initializerTimes[0].count = 0;
	image->runInitializers(gLinkContext, initializerTimes[0]);
}

// This function is called at the end of dlclose() when the reference count goes to zero.
// The dylib being unloaded may have brought in other dependent dylibs when it was loaded.
// Those dependent dylibs need to be unloaded, but only if they are not referenced by
// something else.  We use a standard mark and sweep garbage collection.
//
// The tricky part is that when a dylib is unloaded it may have a termination function that
// can run and itself call dlclose() on yet another dylib.  The problem is that this
// sort of gabage collection is not re-entrant.  Instead a terminator's call to dlclose()
// which calls garbageCollectImages() will just set a flag to re-do the garbage collection
// when the current pass is done.
//
// Also note that this is done within the dyld global lock, so it is always single threaded.
//
void garbageCollectImages()
{
	static bool sDoingGC = false;
	static bool sRedo = false;

	if ( sDoingGC ) {
		// GC is currently being run, just set a flag to have it run again.
		sRedo = true;
		return;
	}
	
	sDoingGC = true;
	do {
		sRedo = false;
		
		// mark phase: mark all images not-in-use
		for (std::vector<ImageLoader*>::iterator it=sAllImages.begin(); it != sAllImages.end(); it++) {
			ImageLoader* image = *it;
			//dyld::log("gc: neverUnload=%d name=%s\n", image->neverUnload(), image->getShortName());
			image->markNotUsed();
		}
		
		// sweep phase: mark as in-use, images reachable from never-unload or in-use image
		for (std::vector<ImageLoader*>::iterator it=sAllImages.begin(); it != sAllImages.end(); it++) {
			ImageLoader* image = *it;
			if ( (image->dlopenCount() != 0) || image->neverUnload() || (image == sMainExecutable) ) {
				OSSpinLockLock(&sDynamicReferencesLock);
					image->markedUsedRecursive(sDynamicReferences);
				OSSpinLockUnlock(&sDynamicReferencesLock);
			}
		}

		// collect phase: build array of images not marked in-use
		ImageLoader* deadImages[sAllImages.size()];
		unsigned deadCount = 0;
		int maxRangeCount = 0;
		for (std::vector<ImageLoader*>::iterator it=sAllImages.begin(); it != sAllImages.end(); it++) {
			ImageLoader* image = *it;
			if ( ! image->isMarkedInUse() ) {
				deadImages[deadCount++] = image;
				if (gLogAPIs) dyld::log("dlclose(), found unused image %p %s\n", image, image->getShortName());
				maxRangeCount += image->segmentCount();
			}
		}

		// collect phase: run termination routines for images not marked in-use
		__cxa_range_t ranges[maxRangeCount];
		int rangeCount = 0;
		for (unsigned i=0; i < deadCount; ++i) {
			ImageLoader* image = deadImages[i];
			for (unsigned int j=0; j < image->segmentCount(); ++j) {
				if ( !image->segExecutable(j) )
					continue;
				if ( rangeCount < maxRangeCount ) {
					ranges[rangeCount].addr = (const void*)image->segActualLoadAddress(j);
					ranges[rangeCount].length = image->segSize(j);
					++rangeCount;
				}
			}
			try {
				runImageStaticTerminators(image);
			}
			catch (const char* msg) {
				dyld::warn("problem running terminators for image: %s\n", msg);
			}
		}
		
		// <rdar://problem/14718598> dyld should call __cxa_finalize_ranges()
		if ( (rangeCount > 0) && (gLibSystemHelpers != NULL) && (gLibSystemHelpers->version >= 13) )
			(*gLibSystemHelpers->cxa_finalize_ranges)(ranges, rangeCount);

		// collect phase: delete all images which are not marked in-use
		bool mightBeMore;
		do {
			mightBeMore = false;
			for (std::vector<ImageLoader*>::iterator it=sAllImages.begin(); it != sAllImages.end(); it++) {
				ImageLoader* image = *it;
				if ( ! image->isMarkedInUse() ) {
					try {
						if (gLogAPIs) dyld::log("dlclose(), deleting %p %s\n", image, image->getShortName());
						removeImage(image);
						ImageLoader::deleteImage(image);
						mightBeMore = true;
						break;  // interator in invalidated by this removal
					}
					catch (const char* msg) {
						dyld::warn("problem deleting image: %s\n", msg);
					}
				}
			}
		} while ( mightBeMore );
	} while (sRedo);
	sDoingGC = false;

	//printAllImages();

}


static void preflight_finally(ImageLoader* image)
{
	if ( image->isBundle() ) {
		removeImageFromAllImages(image->machHeader());
		ImageLoader::deleteImage(image);
	}
	sBundleBeingLoaded = NULL;
	dyld::garbageCollectImages();
}


void preflight(ImageLoader* image, const ImageLoader::RPathChain& loaderRPaths, unsigned cacheIndex)
{
	try {
		if ( image->isBundle() ) 
			sBundleBeingLoaded = image;	// hack
		const char* path = image->getPath();
#if SUPPORT_ACCELERATE_TABLES
		if ( image == sAllCacheImagesProxy )
			path = sAllCacheImagesProxy->getIndexedPath(cacheIndex);
#endif
		image->link(gLinkContext, false, true, false, loaderRPaths, path);
	}
	catch (const char* msg) {	
		preflight_finally(image);
		throw;
	}
	preflight_finally(image);
}

static void loadInsertedDylib(const char* path)
{
	ImageLoader* image = NULL;
	unsigned cacheIndex;
	try {
		LoadContext context;
		context.useSearchPaths		= false;
		context.useFallbackPaths	= false;
		context.useLdLibraryPath	= false;
		context.implicitRPath		= false;
		context.matchByInstallName	= false;
		context.dontLoad			= false;
		context.mustBeBundle		= false;
		context.mustBeDylib			= true;
		context.canBePIE			= false;
		context.origin				= NULL;	// can't use @loader_path with DYLD_INSERT_LIBRARIES
		context.rpath				= NULL;
		image = load(path, context, cacheIndex);
	}
	catch (const char* msg) {
#if TARGET_IPHONE_SIMULATOR
		dyld::log("dyld: warning: could not load inserted library '%s' because %s\n", path, msg);
#else
#if __MAC_OS_X_VERSION_MIN_REQUIRED
		if ( gLinkContext.processUsingLibraryValidation )
			dyld::log("dyld: warning: could not load inserted library '%s' into library validated process because %s\n", path, msg);
		else
#endif
			halt(dyld::mkstringf("could not load inserted library '%s' because %s\n", path, msg));
#endif
	}
	catch (...) {
		halt(dyld::mkstringf("could not load inserted library '%s'\n", path));
	}
}


//
// Sets:
//	sEnvMode
//	gLinkContext.requireCodeSignature
//	gLinkContext.processIsRestricted				// Mac OS X only
//	gLinkContext.processUsingLibraryValidation		// Mac OS X only
//
static void configureProcessRestrictions(const macho_header* mainExecutableMH)
{
    uint32_t flags;
#if TARGET_IPHONE_SIMULATOR || defined(DARLING)
	sEnvMode = envAll;
	gLinkContext.requireCodeSignature = true;
#elif __IPHONE_OS_VERSION_MIN_REQUIRED
	sEnvMode = envNone;
	gLinkContext.requireCodeSignature = true;
	if ( csops(0, CS_OPS_STATUS, &flags, sizeof(flags)) != -1 ) {
		if ( flags & CS_ENFORCEMENT ) {
			if ( flags & CS_GET_TASK_ALLOW ) {
				// Xcode built app for Debug allowed to use DYLD_* variables
				sEnvMode = envAll;
			}
			else {
				// Development kernel can use DYLD_PRINT_* variables on any FairPlay encrypted app
				uint32_t secureValue = 0;
				size_t   secureValueSize = sizeof(secureValue);
				if ( (sysctlbyname("kern.secure_kernel", &secureValue, &secureValueSize, NULL, 0) == 0) && (secureValue == 0) && isFairPlayEncrypted(mainExecutableMH) ) {
					sEnvMode = envPrintOnly;
				}
			}
		}
		else {
			// Development kernel can run unsigned code
			sEnvMode = envAll;
			gLinkContext.requireCodeSignature = false;
		}
	}
	if ( issetugid() ) {
		sEnvMode = envNone;
	}
#elif __MAC_OS_X_VERSION_MIN_REQUIRED
	sEnvMode = envAll;
	gLinkContext.requireCodeSignature = false;
	gLinkContext.processIsRestricted = false;
	gLinkContext.processUsingLibraryValidation = false;
	// any processes with setuid or setgid bit set or with __RESTRICT segment is restricted
    if ( issetugid() || hasRestrictedSegment(mainExecutableMH) ) {
		gLinkContext.processIsRestricted = true;
	}
	if ( csops(0, CS_OPS_STATUS, &flags, sizeof(flags)) != -1 ) {
		// On OS X CS_RESTRICT means the program was signed with entitlements
		if ( ((flags & CS_RESTRICT) == CS_RESTRICT) && (csr_check(CSR_ALLOW_TASK_FOR_PID) != 0) ) {
			gLinkContext.processIsRestricted = true;
		}
		// Library Validation loosens searching but requires everything to be code signed
		if ( flags & CS_REQUIRE_LV ) {
			gLinkContext.processIsRestricted = false;
			//gLinkContext.requireCodeSignature = true;
			gLinkContext.processUsingLibraryValidation = true;
		}
	}
#endif
}


bool processIsRestricted()
{
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	return gLinkContext.processIsRestricted;
#else
	return false;
#endif
}


// <rdar://problem/10583252> Add dyld to uuidArray to enable symbolication of stackshots
static void addDyldImageToUUIDList()
{
	const struct macho_header* mh = (macho_header*)&__dso_handle;
	const uint32_t cmd_count = mh->ncmds;
	const struct load_command* const cmds = (struct load_command*)((char*)mh + sizeof(macho_header));
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_UUID: {
				uuid_command* uc = (uuid_command*)cmd;
				dyld_uuid_info info;
				info.imageLoadAddress = (mach_header*)mh;
				memcpy(info.imageUUID, uc->uuid, 16);
				addNonSharedCacheImageUUID(info);
				return;
			}
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
}

void notifyKernelAboutDyld()
{
	const struct macho_header* mh = (macho_header*)&__dso_handle;
	const uint32_t cmd_count = mh->ncmds;
	const struct load_command* const cmds = (struct load_command*)((char*)mh + sizeof(macho_header));
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_UUID: {
				// Add dyld to the kernel image info
				uuid_command* uc = (uuid_command*)cmd;
				dyld_kernel_image_info_t kernelInfo;
				memcpy(kernelInfo.uuid, uc->uuid, 16);
				kernelInfo.load_addr = (uint64_t)mh;
				kernelInfo.fsobjid.fid_objno = 0;
				kernelInfo.fsobjid.fid_generation = 0;
				kernelInfo.fsid.val[0] = 0;
				kernelInfo.fsid.val[1] = 0;
#if 0
				task_register_dyld_image_infos(mach_task_self(), &kernelInfo, 1);
#endif
				return;
			}
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
}

#if __MAC_OS_X_VERSION_MIN_REQUIRED
typedef int (*open_proc_t)(const char*, int, int);
typedef int (*fcntl_proc_t)(int, int, void*);
typedef int (*ioctl_proc_t)(int, unsigned long, void*);
static void* getProcessInfo() { return dyld::gProcessInfo; }
static SyscallHelpers sSysCalls = {
		7,
		// added in version 1
		(open_proc_t)&open, 
		&close, 
		&pread, 
		&write, 
		&mmap, 
		&munmap, 
		&madvise,
		&stat, 
		(fcntl_proc_t)&fcntl, 
		(ioctl_proc_t)&ioctl, 
		&issetugid, 
		&getcwd, 
		&realpath, 
		&vm_allocate, 
		&vm_deallocate,
		&vm_protect,
		&vlog, 
		&vwarn, 
		&pthread_mutex_lock, 
		&pthread_mutex_unlock,
		&mach_thread_self, 
		&mach_port_deallocate, 
		&task_self_trap,
		&mach_timebase_info,
		&OSAtomicCompareAndSwapPtrBarrier, 
		&OSMemoryBarrier,
		&getProcessInfo,
		&__error,
		&mach_absolute_time,
		// added in version 2
		&thread_switch,
		// added in version 3
		&opendir,
		&readdir_r,
		&closedir,
		// added in version 4
		NULL, //&coresymbolication_load_notifier,
		NULL, //&coresymbolication_unload_notifier,
		// Added in version 5
		&proc_regionfilename,
		&getpid,
		&mach_port_insert_right,
		&mach_port_allocate,
		&mach_msg,
		// Added in version 6
		NULL, //&abort_with_payload,
		// Added in version 7
		NULL, //&task_register_dyld_image_infos,
		NULL, //&task_unregister_dyld_image_infos,
		NULL, //&task_get_dyld_image_infos,
		NULL, //&task_register_dyld_shared_cache_image_info,
		NULL, //&task_register_dyld_set_dyld_state,
		NULL, //&task_register_dyld_get_process_state
};

__attribute__((noinline))
static const char* useSimulatorDyld(int fd, const macho_header* mainExecutableMH, const char* dyldPath,
								int argc, const char* argv[], const char* envp[], const char* apple[],
								uintptr_t* startGlue, uintptr_t* mainAddr)
{
	*startGlue = 0;
	*mainAddr = 0;

	// <rdar://problem/25311921> simulator does not support restricted processes
	uint32_t flags;
	if ( csops(0, CS_OPS_STATUS, &flags, sizeof(flags)) == -1 )
		return "csops() failed";
	if ( (flags & CS_RESTRICT) == CS_RESTRICT )
		return "dyld_sim cannot be loaded in a restricted process";
	if ( issetugid() )
		return "dyld_sim cannot be loaded in a setuid process";
	if ( hasRestrictedSegment(mainExecutableMH) )
		return "dyld_sim cannot be loaded in a restricted process";

	// get file size of dyld_sim
	struct stat sb;
	if ( fstat(fd, &sb) == -1 )
		return "stat(dyld_sim) failed";

	// read first page of dyld_sim file
	uint8_t firstPage[4096];
	if ( pread(fd, firstPage, 4096, 0) != 4096 )
		return "pread(dyld_sim) failed";

	// if fat file, pick matching slice
	uint64_t fileOffset = 0;
	uint64_t fileLength = sb.st_size;
	const fat_header* fileStartAsFat = (fat_header*)firstPage;
	if ( fileStartAsFat->magic == OSSwapBigToHostInt32(FAT_MAGIC) ) {
		if ( !fatFindBest(fileStartAsFat, &fileOffset, &fileLength) ) 
			return "no matching arch in dyld_sim";
		// re-read buffer from start of mach-o slice in fat file
		if ( pread(fd, firstPage, 4096, fileOffset) != 4096 )
			return "pread(dyld_sim) failed";
	}
	else if ( !isCompatibleMachO(firstPage, dyldPath) ) {
		return "dyld_sim not compatible mach-o";
	}
	
	// calculate total size of dyld segments
	const macho_header* mh = (const macho_header*)firstPage;
	struct macho_segment_command* lastSeg = NULL;
	struct macho_segment_command* firstSeg = NULL;
	uintptr_t mappingSize = 0;
	uintptr_t preferredLoadAddress = 0;
	const uint32_t cmd_count = mh->ncmds;
	if ( mh->sizeofcmds > 4096 )
		return "dyld_sim load commands to large";
	if ( (sizeof(macho_header) + mh->sizeofcmds) > 4096 )
		return "dyld_sim load commands to large";
	const struct load_command* const cmds = (struct load_command*)(((char*)mh)+sizeof(macho_header));
	const struct load_command* const endCmds = (struct load_command*)(((char*)mh) + sizeof(macho_header) + mh->sizeofcmds);
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		uint32_t cmdLength = cmd->cmdsize;
		if ( cmdLength < 8 )
			return "dyld_sim load command too small";
		const struct load_command* const nextCmd = (const struct load_command*)(((char*)cmd)+cmdLength);
		if ( (nextCmd > endCmds) || (nextCmd < cmd) )
			return "dyld_sim load command too large";
		switch (cmd->cmd) {
			case LC_SEGMENT_COMMAND:
				{
					struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
					if ( seg->vmaddr + seg->vmsize < seg->vmaddr )
						return "dyld_sim seg wraps address space";
					if ( seg->vmsize < seg->filesize )
						return "dyld_sim seg vmsize too small";
					if ( (seg->fileoff + seg->filesize) < seg->fileoff )
						return "dyld_sim seg size wraps address space";
					if ( lastSeg == NULL ) {
						// first segment must be __TEXT and start at beginning of file/slice
						firstSeg = seg;
						if ( strcmp(seg->segname, "__TEXT") != 0 )
							return "dyld_sim first segment not __TEXT";
						if ( seg->fileoff != 0 )
							return "dyld_sim first segment not at file offset zero";
						if ( seg->filesize < (sizeof(macho_header) + mh->sizeofcmds) )
							return "dyld_sim first segment smaller than load commands";
						preferredLoadAddress = seg->vmaddr;
					}
					else {
						// other sements must be continguous with previous segment and not executable
						if ( lastSeg->fileoff + lastSeg->filesize != seg->fileoff )
							return "dyld_sim segments not contiguous";
						if ( lastSeg->vmaddr + lastSeg->vmsize != seg->vmaddr )
							return "dyld_sim segments not address contiguous";
						if ( (seg->initprot & VM_PROT_EXECUTE) != 0 )
							return "dyld_sim non-first segment is executable";
					}
					mappingSize += seg->vmsize;
					lastSeg = seg;
				}
				break;
			case LC_SEGMENT_COMMAND_WRONG:
				return "dyld_sim wrong load segment load command";
		}
		cmd = nextCmd;
	}
	// last segment must be named __LINKEDIT and not writable
	if ( strcmp(lastSeg->segname, "__LINKEDIT") != 0 )
		return "dyld_sim last segment not __LINKEDIT";
	if ( lastSeg->initprot & VM_PROT_WRITE )
		return "dyld_sim __LINKEDIT segment writable";

	// reserve space, then mmap each segment
	vm_address_t loadAddress = 0;
	if ( ::vm_allocate(mach_task_self(), &loadAddress, mappingSize, VM_FLAGS_ANYWHERE) != 0 )
		return "dyld_sim cannot allocate space";
	cmd = cmds;
	struct linkedit_data_command* codeSigCmd = NULL;
	struct source_version_command* dyldVersionCmd = NULL;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_SEGMENT_COMMAND:
				{
					struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
					uintptr_t requestedLoadAddress = seg->vmaddr - preferredLoadAddress + loadAddress;
					void* segAddress = ::mmap((void*)requestedLoadAddress, seg->filesize, seg->initprot, MAP_FIXED | MAP_PRIVATE, fd, fileOffset + seg->fileoff);
					//dyld::log("dyld_sim %s mapped at %p\n", seg->segname, segAddress);
					if ( segAddress == (void*)(-1) )
						return "dyld_sim mmap() of segment failed";
					if ( ((uintptr_t)segAddress < loadAddress) || ((uintptr_t)segAddress+seg->filesize > loadAddress+mappingSize) )
						return "dyld_sim mmap() to wrong location";
				}
				break;
			case LC_CODE_SIGNATURE:
				codeSigCmd = (struct linkedit_data_command*)cmd;
				break;
			case LC_SOURCE_VERSION:
				dyldVersionCmd = (struct source_version_command*)cmd;
				break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}

	// must have code signature which is contained within LINKEDIT segment
	if ( codeSigCmd == NULL )
		return "dyld_sim not code signed";
	if ( codeSigCmd->dataoff < lastSeg->fileoff )
		return "dyld_sim code signature not in __LINKEDIT";
	if ( (codeSigCmd->dataoff + codeSigCmd->datasize) <  codeSigCmd->dataoff )
		return "dyld_sim code signature size wraps";
	if ( (codeSigCmd->dataoff + codeSigCmd->datasize) > (lastSeg->fileoff + lastSeg->filesize) )
		return "dyld_sim code signature extends beyond __LINKEDIT";

	fsignatures_t siginfo;
	siginfo.fs_file_start=fileOffset;							// start of mach-o slice in fat file 
	siginfo.fs_blob_start=(void*)(long)(codeSigCmd->dataoff);	// start of code-signature in mach-o file
	siginfo.fs_blob_size=codeSigCmd->datasize;					// size of code-signature
	int result = fcntl(fd, F_ADDFILESIGS_FOR_DYLD_SIM, &siginfo);
	if ( result == -1 ) {
		return mkstringf("dyld_sim fcntl(F_ADDFILESIGS_FOR_DYLD_SIM) failed with errno=%d", errno);
	}
	close(fd);
	// file range covered by code signature must extend up to code signature itself
	if ( siginfo.fs_file_start < codeSigCmd->dataoff )
		return mkstringf("dyld_sim code signature does not cover all of dyld_sim. Signature covers up to 0x%08lX. Signature starts at 0x%08X", (unsigned long)siginfo.fs_file_start, codeSigCmd->dataoff);


	// walk newly mapped dyld_sim __TEXT load commands to find entry point
	uintptr_t entry = 0;
	cmd = (struct load_command*)(((char*)loadAddress)+sizeof(macho_header));
	const uint32_t count = ((macho_header*)(loadAddress))->ncmds;
	for (uint32_t i = 0; i < count; ++i) {
		if (cmd->cmd == LC_UNIXTHREAD) {
		#if __i386__
			const i386_thread_state_t* registers = (i386_thread_state_t*)(((char*)cmd) + 16);
			// entry point must be in first segment
			if ( registers->__eip < firstSeg->vmaddr )
				return "dyld_sim entry point not in __TEXT segment";
			if ( registers->__eip > (firstSeg->vmaddr + firstSeg->vmsize) )
				return "dyld_sim entry point not in __TEXT segment";
			entry = (registers->__eip + loadAddress - preferredLoadAddress);
		#elif __x86_64__
			const x86_thread_state64_t* registers = (x86_thread_state64_t*)(((char*)cmd) + 16);
			// entry point must be in first segment
			if ( registers->__rip < firstSeg->vmaddr )
				return "dyld_sim entry point not in __TEXT segment";
			if ( registers->__rip > (firstSeg->vmaddr + firstSeg->vmsize) )
				return "dyld_sim entry point not in __TEXT segment";
			entry = (registers->__rip + loadAddress - preferredLoadAddress);
		#endif
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}

	// notify debugger that dyld_sim is loaded
	dyld_image_info info;
	info.imageLoadAddress = (mach_header*)loadAddress;
	info.imageFilePath	  = strdup(dyldPath);
	info.imageFileModDate = sb.st_mtime;
	addImagesToAllImages(1, &info);
	dyld::gProcessInfo->notification(dyld_image_adding, 1, &info);

	const char** appleParams = apple;
	// jump into new simulator dyld
	typedef uintptr_t (*sim_entry_proc_t)(int argc, const char* argv[], const char* envp[], const char* apple[],
								const macho_header* mainExecutableMH, const macho_header* dyldMH, uintptr_t dyldSlide,
								const dyld::SyscallHelpers* vtable, uintptr_t* startGlue);
	sim_entry_proc_t newDyld = (sim_entry_proc_t)entry;
	*mainAddr = (*newDyld)(argc, argv, envp, appleParams, mainExecutableMH, (macho_header*)loadAddress,
					 loadAddress - preferredLoadAddress, 
					 &sSysCalls, startGlue);
	return NULL;
}
#endif


//
// Entry point for dyld.  The kernel loads dyld and jumps to __dyld_start which
// sets up some registers and call this function.
//
// Returns address of main() in target program which __dyld_start jumps to
//
uintptr_t
_main(const macho_header* mainExecutableMH, uintptr_t mainExecutableSlide, 
		int argc, const char* argv[], const char* envp[], const char* apple[], 
		uintptr_t* startGlue)
{
	uintptr_t result = 0;
	sMainExecutableMachHeader = mainExecutableMH;
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	// if this is host dyld, check to see if iOS simulator is being run
	const char* rootPath = _simple_getenv(envp, "DYLD_ROOT_PATH");
	if ( rootPath != NULL ) {
		// Add dyld to the kernel image info before we jump to the sim
		notifyKernelAboutDyld();

		// look to see if simulator has its own dyld
		char simDyldPath[PATH_MAX]; 
		strlcpy(simDyldPath, rootPath, PATH_MAX);
		strlcat(simDyldPath, "/usr/lib/dyld_sim", PATH_MAX);
		int fd = my_open(simDyldPath, O_RDONLY, 0);
		if ( fd != -1 ) {
			const char* errMessage = useSimulatorDyld(fd, mainExecutableMH, simDyldPath, argc, argv, envp, apple, startGlue, &result);
			if ( errMessage != NULL )
				halt(errMessage);
			return result;
		}
	}
#endif

	CRSetCrashLogMessage("dyld: launch started");

	setContext(mainExecutableMH, argc, argv, envp, apple);

	// Pickup the pointer to the exec path.
	sExecPath = _simple_getenv(apple, "executable_path");

	// <rdar://problem/13868260> Remove interim apple[0] transition code from dyld
	if (!sExecPath) sExecPath = apple[0];
	
	if ( sExecPath[0] != '/' ) {
		// have relative path, use cwd to make absolute
		char cwdbuff[MAXPATHLEN];
	    if ( getcwd(cwdbuff, MAXPATHLEN) != NULL ) {
			// maybe use static buffer to avoid calling malloc so early...
			char* s = new char[strlen(cwdbuff) + strlen(sExecPath) + 2];
			strcpy(s, cwdbuff);
			strcat(s, "/");
			strcat(s, sExecPath);
			sExecPath = s;
		}
	}
	// Remember short name of process for later logging
	sExecShortName = ::strrchr(sExecPath, '/');
	if ( sExecShortName != NULL )
		++sExecShortName;
	else
		sExecShortName = sExecPath;

    configureProcessRestrictions(mainExecutableMH);

#if __MAC_OS_X_VERSION_MIN_REQUIRED
    if ( gLinkContext.processIsRestricted ) {
		pruneEnvironmentVariables(envp, &apple);
		// set again because envp and apple may have changed or moved
		setContext(mainExecutableMH, argc, argv, envp, apple);
	}
	else
#endif
	{
		checkEnvironmentVariables(envp);
		defaultUninitializedFallbackPaths(envp);
	}
	if ( sEnv.DYLD_PRINT_OPTS )
		printOptions(argv);
	if ( sEnv.DYLD_PRINT_ENV ) 
		printEnvironmentVariables(envp);
	getHostInfo(mainExecutableMH, mainExecutableSlide);
	// install gdb notifier
	stateToHandlers(dyld_image_state_dependents_mapped, sBatchHandlers)->push_back(notifyGDB);
	stateToHandlers(dyld_image_state_mapped, sSingleHandlers)->push_back(updateAllImages);
	// make initial allocations large enough that it is unlikely to need to be re-alloced
	sImageRoots.reserve(16);
	sAddImageCallbacks.reserve(4);
	sRemoveImageCallbacks.reserve(4);
	sImageFilesNeedingTermination.reserve(16);
	sImageFilesNeedingDOFUnregistration.reserve(8);

#if !TARGET_IPHONE_SIMULATOR
#ifdef WAIT_FOR_SYSTEM_ORDER_HANDSHAKE
	// <rdar://problem/6849505> Add gating mechanism to dyld support system order file generation process
	WAIT_FOR_SYSTEM_ORDER_HANDSHAKE(dyld::gProcessInfo->systemOrderFlag);
#endif
#endif


	try {
		// add dyld itself to UUID list
		addDyldImageToUUIDList();
		notifyKernelAboutDyld();

#if SUPPORT_ACCELERATE_TABLES
		bool mainExcutableAlreadyRebased = false;

reloadAllImages:
#endif

		CRSetCrashLogMessage(sLoadingCrashMessage);
		// instantiate ImageLoader for main executable
		sMainExecutable = instantiateFromLoadedImage(mainExecutableMH, mainExecutableSlide, sExecPath);
		gLinkContext.mainExecutable = sMainExecutable;
		gLinkContext.mainExecutableCodeSigned = hasCodeSignatureLoadCommand(mainExecutableMH);

#if TARGET_IPHONE_SIMULATOR
		// check main executable is not too new for this OS
		{
			if ( ! isSimulatorBinary((uint8_t*)mainExecutableMH, sExecPath) ) {
				throwf("program was built for a platform that is not supported by this runtime");
			}
			uint32_t mainMinOS = sMainExecutable->minOSVersion();

			// dyld is always built for the current OS, so we can get the current OS version
			// from the load command in dyld itself.
			uint32_t dyldMinOS = ImageLoaderMachO::minOSVersion((const mach_header*)&__dso_handle);
			if ( mainMinOS > dyldMinOS ) {
	#if TARGET_OS_WATCH
				throwf("app was built for watchOS %d.%d which is newer than this simulator %d.%d",
						mainMinOS >> 16, ((mainMinOS >> 8) & 0xFF),
						dyldMinOS >> 16, ((dyldMinOS >> 8) & 0xFF));
	#elif TARGET_OS_TV
				throwf("app was built for tvOS %d.%d which is newer than this simulator %d.%d",
						mainMinOS >> 16, ((mainMinOS >> 8) & 0xFF),
						dyldMinOS >> 16, ((dyldMinOS >> 8) & 0xFF));
	#else
				throwf("app was built for iOS %d.%d which is newer than this simulator %d.%d",
						mainMinOS >> 16, ((mainMinOS >> 8) & 0xFF),
						dyldMinOS >> 16, ((dyldMinOS >> 8) & 0xFF));
	#endif
			}
		}
#endif


	#if __MAC_OS_X_VERSION_MIN_REQUIRED
		// <rdar://problem/22805519> be less strict about old mach-o binaries
		uint32_t mainSDK = sMainExecutable->sdkVersion();
		gLinkContext.strictMachORequired = (mainSDK >= DYLD_MACOSX_VERSION_10_12) || gLinkContext.processUsingLibraryValidation;
	#else
		// simulators, iOS, tvOS, and watchOS are always strict
		gLinkContext.strictMachORequired = true;
	#endif

		// load shared cache
		checkSharedRegionDisable();
	#if DYLD_SHARED_CACHE_SUPPORT
		if ( gLinkContext.sharedRegionMode != ImageLoader::kDontUseSharedRegion ) {
			mapSharedCache();
		} else {
			dyld_kernel_image_info_t kernelCacheInfo;
			bzero(&kernelCacheInfo.uuid[0], sizeof(uuid_t));
			kernelCacheInfo.load_addr = 0;
			kernelCacheInfo.fsobjid.fid_objno = 0;
			kernelCacheInfo.fsobjid.fid_generation = 0;
			kernelCacheInfo.fsid.val[0] = 0;
			kernelCacheInfo.fsid.val[0] = 0;
#if 0
			task_register_dyld_shared_cache_image_info(mach_task_self(), kernelCacheInfo, true, false);
#endif
		}
	#endif

	#if SUPPORT_ACCELERATE_TABLES
		sAllImages.reserve((sAllCacheImagesProxy != NULL) ? 16 : INITIAL_IMAGE_COUNT);
	#else
		sAllImages.reserve(INITIAL_IMAGE_COUNT);
	#endif

		// Now that shared cache is loaded, setup an versioned dylib overrides
	#if SUPPORT_VERSIONED_PATHS
		checkVersionedPaths();
	#endif


		// dyld_all_image_infos image list does not contain dyld
		// add it as dyldPath field in dyld_all_image_infos
		// for simulator, dyld_sim is in image list, need host dyld added
#if TARGET_IPHONE_SIMULATOR
		// get path of host dyld from table of syscall vectors in host dyld
		void* addressInDyld = gSyscallHelpers;
#else
		// get path of dyld itself
		void*  addressInDyld = (void*)&__dso_handle;
#endif
		char dyldPathBuffer[MAXPATHLEN+1];
		int len = proc_regionfilename(getpid(), (uint64_t)(long)addressInDyld, dyldPathBuffer, MAXPATHLEN);
		if ( (len != 0) && (strcmp(dyldPathBuffer, gProcessInfo->dyldPath) != 0) ) {
			gProcessInfo->dyldPath = strdup(dyldPathBuffer);
		}

		// load any inserted libraries
		if	( sEnv.DYLD_INSERT_LIBRARIES != NULL ) {
			for (const char* const* lib = sEnv.DYLD_INSERT_LIBRARIES; *lib != NULL; ++lib) 
				loadInsertedDylib(*lib);
		}
		// record count of inserted libraries so that a flat search will look at 
		// inserted libraries, then main, then others.
		sInsertedDylibCount = sAllImages.size()-1;

		// link main executable
		gLinkContext.linkingMainExecutable = true;
#if SUPPORT_ACCELERATE_TABLES
		if ( mainExcutableAlreadyRebased ) {
			// previous link() on main executable has already adjusted its internal pointers for ASLR
			// work around that by rebasing by inverse amount
			sMainExecutable->rebase(gLinkContext, -mainExecutableSlide);
		}
#endif
		link(sMainExecutable, sEnv.DYLD_BIND_AT_LAUNCH, true, ImageLoader::RPathChain(NULL, NULL), -1);
		sMainExecutable->setNeverUnloadRecursive();
		if ( sMainExecutable->forceFlat() ) {
			gLinkContext.bindFlat = true;
			gLinkContext.prebindUsage = ImageLoader::kUseNoPrebinding;
		}

		// link any inserted libraries
		// do this after linking main executable so that any dylibs pulled in by inserted 
		// dylibs (e.g. libSystem) will not be in front of dylibs the program uses
		if ( sInsertedDylibCount > 0 ) {
			for(unsigned int i=0; i < sInsertedDylibCount; ++i) {
				ImageLoader* image = sAllImages[i+1];
				link(image, sEnv.DYLD_BIND_AT_LAUNCH, true, ImageLoader::RPathChain(NULL, NULL), -1);
				image->setNeverUnloadRecursive();
			}
			// only INSERTED libraries can interpose
			// register interposing info after all inserted libraries are bound so chaining works
			for(unsigned int i=0; i < sInsertedDylibCount; ++i) {
				ImageLoader* image = sAllImages[i+1];
				image->registerInterposing();
			}
		}

		// <rdar://problem/19315404> dyld should support interposition even without DYLD_INSERT_LIBRARIES
		for (long i=sInsertedDylibCount+1; i < sAllImages.size(); ++i) {
			ImageLoader* image = sAllImages[i];
			if ( image->inSharedCache() )
				continue;
			image->registerInterposing();
		}
	#if SUPPORT_ACCELERATE_TABLES
		if ( (sAllCacheImagesProxy != NULL) && ImageLoader::haveInterposingTuples() ) {
			// Accelerator tables cannot be used with implicit interposing, so relaunch with accelerator tables disabled
			ImageLoader::clearInterposingTuples();
			// unmap all loaded dylibs (but not main executable)
			for (long i=1; i < sAllImages.size(); ++i) {
				ImageLoader* image = sAllImages[i];
				if ( image == sMainExecutable )
					continue;
				if ( image == sAllCacheImagesProxy )
					continue;
				image->setCanUnload();
				ImageLoader::deleteImage(image);
			}
			// note: we don't need to worry about inserted images because if DYLD_INSERT_LIBRARIES was set we would not be using the accelerator table
			sAllImages.clear();
			sImageRoots.clear();
			sImageFilesNeedingTermination.clear();
			sImageFilesNeedingDOFUnregistration.clear();
			sAddImageCallbacks.clear();
			sRemoveImageCallbacks.clear();
			sDisableAcceleratorTables = true;
			sAllCacheImagesProxy = NULL;
			sMappedRangesStart = NULL;
			mainExcutableAlreadyRebased = true;
			gLinkContext.linkingMainExecutable = false;
			resetAllImages();
			goto reloadAllImages;
		}
	#endif

		// apply interposing to initial set of images
		for(int i=0; i < sImageRoots.size(); ++i) {
			sImageRoots[i]->applyInterposing(gLinkContext);
		}
		gLinkContext.linkingMainExecutable = false;
		
		// <rdar://problem/12186933> do weak binding only after all inserted images linked
		sMainExecutable->weakBind(gLinkContext);

	#if DYLD_SHARED_CACHE_SUPPORT
		// If cache has branch island dylibs, tell debugger about them
		if ( (sSharedCache != NULL) && (sSharedCache->mappingOffset >= 0x78) && (sSharedCache->branchPoolsOffset != 0) ) {
			uint32_t count = sSharedCache->branchPoolsCount;
			dyld_image_info info[count];
			const uint64_t* poolAddress = (uint64_t*)((char*)sSharedCache + sSharedCache->branchPoolsOffset);
			// <rdar://problem/20799203> empty branch pools can be in development cache
			if ( ((mach_header*)poolAddress)->magic == sMainExecutableMachHeader->magic ) {
				for (int poolIndex=0; poolIndex < count; ++poolIndex) {
					uint64_t poolAddr = poolAddress[poolIndex] + sSharedCacheSlide;
					info[poolIndex].imageLoadAddress = (mach_header*)(long)poolAddr;
					info[poolIndex].imageFilePath = "dyld_shared_cache_branch_islands";
					info[poolIndex].imageFileModDate = 0;
				}
				// add to all_images list
				addImagesToAllImages(count, info);
				// tell gdb about new branch island images
				gProcessInfo->notification(dyld_image_adding, count, info);
			}
		}
	#endif

		CRSetCrashLogMessage("dyld: launch, running initializers");
	#if SUPPORT_OLD_CRT_INITIALIZATION
		// Old way is to run initializers via a callback from crt1.o
		if ( ! gRunInitializersOldWay ) 
			initializeMainExecutable(); 
	#else
		// run all initializers
		initializeMainExecutable(); 
	#endif
		// find entry point for main executable
		result = (uintptr_t)sMainExecutable->getThreadPC();
		if ( result != 0 ) {
			// main executable uses LC_MAIN, needs to return to glue in libdyld.dylib
			if ( (gLibSystemHelpers != NULL) && (gLibSystemHelpers->version >= 9) )
				*startGlue = (uintptr_t)gLibSystemHelpers->startGlueToCallExit;
			else
				halt("libdyld.dylib support not present for LC_MAIN");
		}
		else {
			// main executable uses LC_UNIXTHREAD, dyld needs to let "start" in program set up for main()
			result = (uintptr_t)sMainExecutable->getMain();
			*startGlue = 0;
		}
	}
	catch(const char* message) {
		syncAllImages();
		halt(message);
	}
	catch(...) {
		dyld::log("dyld: launch failed\n");
	}

	CRSetCrashLogMessage(NULL);
	
	return result;
}



} // namespace



