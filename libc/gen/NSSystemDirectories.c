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
#include <NSSystemDirectories.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>
#include <pthread.h>

#define NSUserDomainIndex	0
#define NSLocalDomainIndex	1
#define NSNetworkDomainIndex	2
#define NSSystemDomainIndex	3

#define numDomains		(NSSystemDomainIndex + 1)
#define DomainMask		((1 << numDomains) - 1)

#define addNextRoot(x)		(*(x) == '/' || *(x) == 0)

#define Network					"/Network"
#define System					"/System"
#define Tilde					"~"

#define NSApplicationDirectoryBase		"/Applications"
#define NSDemoApplicationDirectoryBase		"/Applications/Demos"
#define NSDeveloperApplicationDirectoryBase	"/Developer/Applications"
#define NSAdminApplicationDirectoryBase		"/Applications/Utilities"
#define NSLibraryDirectoryBase			"/Library"
#define NSDeveloperDirectoryBase		"/Developer"
#define NSUserDirectoryBase			"/Users"
#define NSDocumentationDirectoryBase		"/Library/Documentation"
#define NSDocumentDirectoryBase			"/Documents"
#define NSCoreServiceDirectoryBase		"/Library/CoreServices"
#define NSAutosavedDocumentsDirectoryBase	"/Library/Autosave Information"
#define NSDesktopDirectoryBase			"/Desktop"
#define NSCachesDirectoryBase			"/Library/Caches"
#define NSInputMethodsDirectoryBase		"/Library/Input Methods"
#define NSMoviesDirectoryBase			"/Movies"
#define NSMusicDirectoryBase			"/Music"
#define NSPicturesDirectoryBase			"/Pictures"
#define NSPrinterDescriptionDirectoryBase	"/Library/Printers/PPDs"
#define NSSharedPublicDirectoryBase		"/Public"
#define NSPreferencePanesDirectoryBase		"/Library/PreferencePanes"
#define NSApplicationSupportDirectoryBase	"/Library/Application Support"
#define NSDownloadsDirectoryBase		"/Downloads"

static const char * const prefixAll[] = {
    Tilde,
    "",
    Network,
    ""
};
static const char * const prefixAllSystem[] = {
    Tilde,
    "",
    Network,
    System
};
static const char * const prefixNoUserSystem[] = {
    NULL,
    "",
    Network,
    NULL
};
static const char * const prefixNoNetwork[] = {
    Tilde,
    "",
    NULL,
    System
};
static const char * const prefixSystemOnly[] = {
    NULL,
    NULL,
    NULL,
    System
};
static const char * const prefixUserOnly[] = {
    Tilde,
    NULL,
    NULL,
    NULL
};

static const char * const _prefixNetwork4[] = {
    Network,
    Network,
    Network,
    Network
};
static const char * const _prefixNone4[] = {
    "",
    "",
    "",
    ""
};
static const char * const _prefixTilde4[] = {
    Tilde,
    Tilde,
    Tilde,
    Tilde
};
static const char * const * const prefixAllApplicationsDirectory[] = {
    _prefixTilde4,
    _prefixNone4,
    _prefixNetwork4,
    _prefixNone4
};
static const char * const baseAllApplicationsDirectory[] = {
    NSApplicationDirectoryBase,
    NSAdminApplicationDirectoryBase,
    NSDeveloperApplicationDirectoryBase,
    NSDemoApplicationDirectoryBase
};

static const char * const _prefixNetwork2[] = {
    Network,
    Network
};
static const char * const _prefixNone2[] = {
    "",
    ""
};
static const char * const _prefixSystemNone2[] = {
    System,
    ""
};
static const char * const _prefixTilde2[] = {
    Tilde,
    Tilde
};
static const char * const * const prefixAllLibrariesDirectory[] = {
    _prefixTilde2,
    _prefixNone2,
    _prefixNetwork2,
    _prefixSystemNone2
};
static const char * const baseAllLibrariesDirectory[] = {
    NSLibraryDirectoryBase,
    NSDeveloperDirectoryBase
};

// The dirInfo table drives path creation
static struct {
    int pathsPerDomain;
    const void * const * const prefix;
    const void * const base;
} dirInfo[] = {
    { // NSApplicationDirectory
	1,
	(const void * const * const)prefixAll,
	(const void * const)NSApplicationDirectoryBase
    },
    { // NSDemoApplicationDirectory
	1,
	(const void * const * const)prefixAll,
	(const void * const)NSDemoApplicationDirectoryBase
    },
    { // NSDeveloperApplicationDirectory
	1,
	(const void * const * const)prefixAll,
	(const void * const)NSDeveloperApplicationDirectoryBase
    },
    { // NSAdminApplicationDirectory
	1,
	(const void * const * const)prefixAll,
	(const void * const)NSAdminApplicationDirectoryBase
    },
    { // NSLibraryDirectory
	1,
	(const void * const * const)prefixAllSystem,
	(const void * const)NSLibraryDirectoryBase
    },
    { // NSDeveloperDirectory
	1,
	(const void * const * const)prefixAll,
	(const void * const)NSDeveloperDirectoryBase
    },
    { // NSUserDirectory
	1,
	(const void * const * const)prefixNoUserSystem,
	(const void * const)NSUserDirectoryBase
    },
    { // NSDocumentationDirectory
	1,
	(const void * const * const)prefixAllSystem,
	(const void * const)NSDocumentationDirectoryBase
    },
    { // NSDocumentDirectory
	1,
	(const void * const * const)prefixUserOnly,
	(const void * const)NSDocumentDirectoryBase
    },
    { // NSCoreServiceDirectory
	1,
	(const void * const * const)prefixSystemOnly,
	(const void * const)NSCoreServiceDirectoryBase
    },
    { // NSAutosavedInformationDirectory
	1,
	(const void * const * const)prefixUserOnly,
	(const void * const)NSAutosavedDocumentsDirectoryBase
    },
    { // NSDesktopDirectory
	1,
	(const void * const * const)prefixUserOnly,
	(const void * const)NSDesktopDirectoryBase
    },
    { // NSCachesDirectory
	1,
	(const void * const * const)prefixNoNetwork,
	(const void * const)NSCachesDirectoryBase
    },
    { // NSApplicationSupportDirectory
	1,
	(const void * const * const)prefixAll,
	(const void * const)NSApplicationSupportDirectoryBase
    },
    { // NSDownloadsDirectory
	1,
	(const void * const * const)prefixUserOnly,
	(const void * const)NSDownloadsDirectoryBase
    },
    { // NSInputMethodsDirectory
	1,
	(const void * const * const)prefixAllSystem,
	(const void * const)NSInputMethodsDirectoryBase
    },
    { // NSMoviesDirectory
	1,
	(const void * const * const)prefixUserOnly,
	(const void * const)NSMoviesDirectoryBase
    },
    { // NSMusicDirectory
	1,
	(const void * const * const)prefixUserOnly,
	(const void * const)NSMusicDirectoryBase
    },
    { // NSPicturesDirectory
	1,
	(const void * const * const)prefixUserOnly,
	(const void * const)NSPicturesDirectoryBase
    },
    { // NSPrinterDescriptionDirectory
	1,
	(const void * const * const)prefixSystemOnly,
	(const void * const)NSPrinterDescriptionDirectoryBase
    },
    { // NSSharedPublicDirectory
	1,
	(const void * const * const)prefixUserOnly,
	(const void * const)NSSharedPublicDirectoryBase
    },
    { // NSPreferencePanesDirectory
	1,
	(const void * const * const)prefixNoNetwork,
	(const void * const)NSPreferencePanesDirectoryBase
    },
    { // NSAllApplicationsDirectory
	4,
	(const void * const * const)prefixAllApplicationsDirectory,
	(const void * const)baseAllApplicationsDirectory
    },
    { // NSAllLibrariesDirectory
	2,
	(const void * const * const)prefixAllLibrariesDirectory,
	(const void * const)baseAllLibrariesDirectory
    }
};

#define Index(dir)	(((dir) >= NSApplicationDirectory && (dir) <= NSPreferencePanesDirectory) ? ((dir) - 1) : (((dir) >= NSAllApplicationsDirectory && (dir) <= NSAllLibrariesDirectory) ? ((dir) - NSAllApplicationsDirectory + NSPreferencePanesDirectory) : -1))

#define invalidDomains	0x00	// some domains may be invalid on non-Mach systems
#define ByteMask	0xff
#define DirShift	24
#define IndexShift	16

NSSearchPathEnumerationState NSStartSearchPathEnumeration(NSSearchPathDirectory dir, NSSearchPathDomainMask domainMask) {
    // The state is AABBCCCC, where
    // AA is the dir(s) requested
    // BB is the current state of dirs (if AA < 100, then this is always 0; otherwise it goes up to number of dirs)
    // CCCC is the domains requested
    // the state always contains the next item; if CCCC is 0, then we're done
    int i;

    if((i = Index(dir)) < 0) {
	return 0;
    }
    domainMask = domainMask & DomainMask & ~invalidDomains;	// Just leave useful bits in there

    // Trim Duplicates - This assumes the compiler generates a single address
    // for multiple occurrences of the same literal strings.
    if ((domainMask & (NSLocalDomainMask | NSSystemDomainMask)) == (NSLocalDomainMask | NSSystemDomainMask) && dirInfo[i].prefix[NSLocalDomainIndex] == dirInfo[i].prefix[NSSystemDomainIndex]) {
	 domainMask &= ~NSSystemDomainMask;
    }

    return (dir << DirShift) + domainMask;
}

static const char       *nextRoot = NULL;
static pthread_once_t   nextRoot_init_once = PTHREAD_ONCE_INIT;

static void
nextRoot_init(void)
{
    if (!issetugid() && (nextRoot = getenv("NEXT_ROOT")) != NULL) {
	nextRoot = strdup(nextRoot);
    }
    if (nextRoot == NULL) {
	nextRoot = "";
    }
}

NSSearchPathEnumerationState NSGetNextSearchPathEnumeration(NSSearchPathEnumerationState state, char *path) {
    int dir = (state >> DirShift) & ByteMask;
    int domainMask = state & DomainMask;
    int domain, i, n;
    const char *prefix, *base;
    
    if ((i = Index(dir)) < 0 || (domain = ffs(domainMask)) == 0)
	return 0;
    domain--; // adjust to zero-based index

    if ((n = dirInfo[i].pathsPerDomain) == 1) {
	const char * const *p = (const char * const *)dirInfo[i].prefix;
	for (;;) { // loop, skipping over invalid domains (prefix is NULL)
	    domainMask &= ~(1 << domain);
	    if ((prefix = p[domain]) != NULL) {
		break;
	    }
	    if ((domain = ffs(domainMask)) == 0) {
		return 0;
	    }
	    domain--; // adjust to zero-based index
	}
	base = (const char *)dirInfo[i].base;
	state = (dir << DirShift) + domainMask;
    } else { // multiple paths per domain
	const char * const **p = (const char * const **)dirInfo[i].prefix;
	const char * const *b = (const char * const *)dirInfo[i].base;
	int dirIndex = (state >> IndexShift) & ByteMask;

	if (dirIndex >= n) { // done with the current domain, go to the next
	    domainMask &= ~(1 << domain);
	    if ((domain = ffs(domainMask)) == 0) {
		return 0;
	    }
	    domain--; // adjust to zero-based index
	    dirIndex = 0;
	}
	prefix = p[domain][dirIndex];
	base = b[dirIndex];
	state = (dir << DirShift) + (++dirIndex << IndexShift) + domainMask;
    }

    if (addNextRoot(prefix)) {
	if (pthread_once(&nextRoot_init_once, nextRoot_init) != 0 || nextRoot == NULL)// Error
	    return 0;
	strlcpy(path, nextRoot, PATH_MAX);
    } else {
	*path = 0;
    }
    strlcat(path, prefix, PATH_MAX);
    strlcat(path, base, PATH_MAX);
 
    return state;
}
