/**
 * StartupItems.c - Startup Item management routines
 * Wilfredo Sanchez  | wsanchez@opensource.apple.com
 * Kevin Van Vechten | kevinvv@uclink4.berkeley.edu
 * $Apple$
 **
 * Copyright (c) 1999-2002 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * @APPLE_APACHE_LICENSE_HEADER_END@
 **/

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <sysexits.h>
#include <syslog.h>
#include <CoreFoundation/CoreFoundation.h>
#include "StartupItems.h"

#define kStartupItemsPath	"/StartupItems"
#define kParametersFile		"StartupParameters.plist"
#define kDisabledFile		".disabled"

#define kRunSuccess CFSTR("success")
#define kRunFailure CFSTR("failure")

static const char *argumentForAction(Action anAction)
{
	switch (anAction) {
	case kActionStart:
		return "start";
	case kActionStop:
		return "stop";
	case kActionRestart:
		return "restart";
	default:
		return NULL;
	}
}

#define checkTypeOfValue(aKey,aTypeID)					\
  {									\
    CFStringRef aProperty = CFDictionaryGetValue(aConfig, aKey);	\
    if (aProperty && CFGetTypeID(aProperty) != aTypeID)			\
      return FALSE;							\
  }

static int StartupItemValidate(CFDictionaryRef aConfig)
{
	if (aConfig && CFGetTypeID(aConfig) == CFDictionaryGetTypeID()) {
		checkTypeOfValue(kProvidesKey, CFArrayGetTypeID());
		checkTypeOfValue(kRequiresKey, CFArrayGetTypeID());

		return TRUE;
	}
	return FALSE;
}

/*
 *	remove item from waiting list
 */
void RemoveItemFromWaitingList(StartupContext aStartupContext, CFMutableDictionaryRef anItem)
{
	/* Remove the item from the waiting list. */
	if (aStartupContext && anItem && aStartupContext->aWaitingList) {
		CFRange aRange = { 0, CFArrayGetCount(aStartupContext->aWaitingList) };
		CFIndex anIndex = CFArrayGetFirstIndexOfValue(aStartupContext->aWaitingList, aRange, anItem);

		if (anIndex >= 0) {
			CFArrayRemoveValueAtIndex(aStartupContext->aWaitingList, anIndex);
		}
	}
}

/*
 *	add item to failed list, create list if it doesn't exist
 *	return and fail quietly if it can't create list
 */
void AddItemToFailedList(StartupContext aStartupContext, CFMutableDictionaryRef anItem)
{
	if (aStartupContext && anItem) {
		/* create the failed list if it doesn't exist */
		if (!aStartupContext->aFailedList) {
			aStartupContext->aFailedList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		}
		if (aStartupContext->aFailedList) {
			CFArrayAppendValue(aStartupContext->aFailedList, anItem);
		}
	}
}

/**
 * startupItemListCopyMatches returns an array of items which contain the string aService in the key aKey
 **/
static CFMutableArrayRef startupItemListCopyMatches(CFArrayRef anItemList, CFStringRef aKey, CFStringRef aService)
{
	CFMutableArrayRef aResult = NULL;

	if (anItemList && aKey && aService) {
		CFIndex anItemCount = CFArrayGetCount(anItemList);
		CFIndex anItemIndex = 0;

		aResult = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

		for (anItemIndex = 0; anItemIndex < anItemCount; ++anItemIndex) {
			CFMutableDictionaryRef anItem = (CFMutableDictionaryRef) CFArrayGetValueAtIndex(anItemList, anItemIndex);
			CFArrayRef aList = CFDictionaryGetValue(anItem, aKey);

			if (aList) {
				if (CFArrayContainsValue(aList, CFRangeMake(0, CFArrayGetCount(aList)), aService) &&
				    !CFArrayContainsValue(aResult, CFRangeMake(0, CFArrayGetCount(aResult)), anItem)) {
					CFArrayAppendValue(aResult, anItem);
				}
			}
		}
	}
	return aResult;
}

static void SpecialCasesStartupItemHandler(CFMutableDictionaryRef aConfig)
{
	static const CFStringRef stubitems[] = {
		CFSTR("Accounting"),
		CFSTR("System Tuning"),
		CFSTR("SecurityServer"),
		CFSTR("Portmap"),
		CFSTR("System Log"),
		CFSTR("Resolver"),
		CFSTR("LDAP"),
		CFSTR("NetInfo"),
		CFSTR("NetworkExtensions"),
		CFSTR("DirectoryServices"),
		CFSTR("Network Configuration"),
		CFSTR("mDNSResponder"),
		CFSTR("Cron"),
		CFSTR("Core Graphics"),
		CFSTR("Core Services"),
		CFSTR("Network"),
		CFSTR("TIM"),
		CFSTR("Disks"),
		CFSTR("NIS"),
		NULL
	};
	CFMutableArrayRef aList, aNewList;
	CFIndex i, aCount;
	CFStringRef ci, type = kRequiresKey;
	const CFStringRef *c;

      again:
	aList = (CFMutableArrayRef) CFDictionaryGetValue(aConfig, type);
	if (aList) {
		aCount = CFArrayGetCount(aList);

		aNewList = CFArrayCreateMutable(kCFAllocatorDefault, aCount, &kCFTypeArrayCallBacks);

		for (i = 0; i < aCount; i++) {
			ci = CFArrayGetValueAtIndex(aList, i);
			CF_syslog(LOG_DEBUG, CFSTR("%@: Evaluating %@"), type, ci);
			for (c = stubitems; *c; c++) {
				if (CFEqual(*c, ci))
					break;
			}
			if (*c == NULL) {
				CFArrayAppendValue(aNewList, ci);
				CF_syslog(LOG_DEBUG, CFSTR("%@: Keeping %@"), type, ci);
			}
		}

		CFDictionaryReplaceValue(aConfig, type, aNewList);
		CFRelease(aNewList);
	}
	if (type == kUsesKey)
		return;
	type = kUsesKey;
	goto again;
}

CFIndex StartupItemListCountServices(CFArrayRef anItemList)
{
	CFIndex aResult = 0;

	if (anItemList) {
		CFIndex anItemCount = CFArrayGetCount(anItemList);
		CFIndex anItemIndex = 0;

		for (anItemIndex = 0; anItemIndex < anItemCount; ++anItemIndex) {
			CFDictionaryRef anItem = CFArrayGetValueAtIndex(anItemList, anItemIndex);
			CFArrayRef aProvidesList = CFDictionaryGetValue(anItem, kProvidesKey);

			if (aProvidesList)
				aResult += CFArrayGetCount(aProvidesList);
		}
	}
	return aResult;
}

bool StartupItemSecurityCheck(const char *aPath)
{
	static struct timeval boot_time;
	struct stat aStatBuf;
	bool r = true;

	if (boot_time.tv_sec == 0) {
		int mib[] = { CTL_KERN, KERN_BOOTTIME };
		size_t boot_time_sz = sizeof(boot_time);
		int rv;

		rv = sysctl(mib, sizeof(mib) / sizeof(mib[0]), &boot_time, &boot_time_sz, NULL, 0);

		assert(rv != -1);
		assert(boot_time_sz == sizeof(boot_time));
	}

	/* should use lstatx_np() on Tiger? */
	if (lstat(aPath, &aStatBuf) == -1) {
		if (errno != ENOENT)
			syslog(LOG_ERR, "lstat(\"%s\"): %m", aPath);
		return false;
	}
	/*
	 * We check the boot time because of 5409386.
	 * We ignore the boot time if PPID != 1 because of 5503536.
	 */
	if ((aStatBuf.st_ctimespec.tv_sec > boot_time.tv_sec) && (getppid() == 1)) {
		syslog(LOG_WARNING, "\"%s\" failed sanity check: path was created after boot up", aPath);
		return false;
	}
	if (!(S_ISREG(aStatBuf.st_mode) || S_ISDIR(aStatBuf.st_mode))) {
		syslog(LOG_WARNING, "\"%s\" failed security check: not a directory or regular file", aPath);
		r = false;
	}
	if (aStatBuf.st_mode & S_IWOTH) {
		syslog(LOG_WARNING, "\"%s\" failed security check: world writable", aPath);
		r = false;
	}
	if (aStatBuf.st_mode & S_IWGRP) {
		syslog(LOG_WARNING, "\"%s\" failed security check: group writable", aPath);
		r = false;
	}
	if (aStatBuf.st_uid != 0) {
		syslog(LOG_WARNING, "\"%s\" failed security check: not owned by UID 0", aPath);
		r = false;
	}
	if (aStatBuf.st_gid != 0) {
		syslog(LOG_WARNING, "\"%s\" failed security check: not owned by GID 0", aPath);
		r = false;
	}
	if (r == false) {
		mkdir(kFixerDir, ACCESSPERMS);
		close(open(kFixerPath, O_RDWR|O_CREAT|O_NOCTTY, DEFFILEMODE));
	}
	return r;
}

CFMutableArrayRef StartupItemListCreateWithMask(NSSearchPathDomainMask aMask)
{
	CFMutableArrayRef anItemList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	char aPath[PATH_MAX];
	CFIndex aDomainIndex = 0;

	NSSearchPathEnumerationState aState = NSStartSearchPathEnumeration(NSLibraryDirectory, aMask);

	while ((aState = NSGetNextSearchPathEnumeration(aState, aPath))) {
		DIR *aDirectory;

		strlcat(aPath, kStartupItemsPath, sizeof(aPath));
		++aDomainIndex;

		/* 5485016
		 *
		 * Just in case...
		 */
		mkdir(aPath, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);

		if (!StartupItemSecurityCheck(aPath))
			continue;

		if ((aDirectory = opendir(aPath))) {
			struct dirent *aBundle;

			while ((aBundle = readdir(aDirectory))) {
				struct stat aStatBuf;
				char *aBundleName = aBundle->d_name;
				char aBundlePath[PATH_MAX];
				char aBundleExecutablePath[PATH_MAX];
				char aConfigFile[PATH_MAX];
				char aDisabledFile[PATH_MAX];

				if (aBundleName[0] == '.')
					continue;

				syslog(LOG_DEBUG, "Found item: %s", aBundleName);

				sprintf(aBundlePath, "%s/%s", aPath, aBundleName);
				sprintf(aBundleExecutablePath, "%s/%s", aBundlePath, aBundleName);
				sprintf(aConfigFile, "%s/%s", aBundlePath, kParametersFile);
				sprintf(aDisabledFile, "%s/%s", aBundlePath, kDisabledFile);

				if (lstat(aDisabledFile, &aStatBuf) == 0) {
					syslog(LOG_NOTICE, "Skipping disabled StartupItem: %s", aBundlePath);
					continue;
				}
				if (!StartupItemSecurityCheck(aBundlePath))
					continue;
				if (!StartupItemSecurityCheck(aBundleExecutablePath))
					continue;
				if (!StartupItemSecurityCheck(aConfigFile))
					continue;

				/* Stow away the plist data for each bundle */
				{
					int aConfigFileDescriptor;

					if ((aConfigFileDescriptor = open(aConfigFile, O_RDONLY|O_NOCTTY, (mode_t) 0)) != -1) {
						struct stat aConfigFileStatBuffer;

						if (stat(aConfigFile, &aConfigFileStatBuffer) != -1) {
							off_t aConfigFileContentsSize = aConfigFileStatBuffer.st_size;
							char *aConfigFileContentsBuffer;

							if ((aConfigFileContentsBuffer =
							     mmap((caddr_t) 0, aConfigFileContentsSize,
								  PROT_READ, MAP_FILE | MAP_PRIVATE,
								  aConfigFileDescriptor, (off_t) 0)) != (caddr_t) - 1) {
								CFDataRef aConfigData = NULL;
								CFMutableDictionaryRef aConfig = NULL;

								aConfigData =
									CFDataCreateWithBytesNoCopy(NULL,
												    (const UInt8 *)aConfigFileContentsBuffer,
												    aConfigFileContentsSize,
												    kCFAllocatorNull);

								if (aConfigData) {
									aConfig = (CFMutableDictionaryRef)
									    CFPropertyListCreateFromXMLData(NULL, aConfigData,
													    kCFPropertyListMutableContainers,
													    NULL);
								}
								if (StartupItemValidate(aConfig)) {
									CFStringRef aBundlePathString =
									    CFStringCreateWithCString(NULL, aBundlePath,
												      kCFStringEncodingUTF8);

									CFNumberRef aDomainNumber =
									    CFNumberCreate(NULL, kCFNumberCFIndexType,
											   &aDomainIndex);

									CFDictionarySetValue(aConfig, kBundlePathKey,
											     aBundlePathString);
									CFDictionarySetValue(aConfig, kDomainKey, aDomainNumber);
									CFRelease(aDomainNumber);
									SpecialCasesStartupItemHandler(aConfig);
									CFArrayAppendValue(anItemList, aConfig);

									CFRelease(aBundlePathString);
								} else {
									syslog(LOG_ERR, "Malformatted parameters file: %s",
									       aConfigFile);
								}

								if (aConfig)
									CFRelease(aConfig);
								if (aConfigData)
									CFRelease(aConfigData);

								if (munmap(aConfigFileContentsBuffer, aConfigFileContentsSize) ==
								    -1) {
									syslog(LOG_WARNING,
									       "Unable to unmap parameters file %s for item %s: %m",
									       aConfigFile, aBundleName);
								}
							} else {
								syslog(LOG_ERR,
								       "Unable to map parameters file %s for item %s: %m",
								       aConfigFile, aBundleName);
							}
						} else {
							syslog(LOG_ERR, "Unable to stat parameters file %s for item %s: %m",
							       aConfigFile, aBundleName);
						}

						if (close(aConfigFileDescriptor) == -1) {
							syslog(LOG_ERR, "Unable to close parameters file %s for item %s: %m",
							       aConfigFile, aBundleName);
						}
					} else {
						syslog(LOG_ERR, "Unable to open parameters file %s for item %s: %m", aConfigFile,
						       aBundleName);
					}
				}
			}
			if (closedir(aDirectory) == -1) {
				syslog(LOG_WARNING, "Unable to directory bundle %s: %m", aPath);
			}
		} else {
			if (errno != ENOENT) {
				syslog(LOG_WARNING, "Open on directory %s failed: %m", aPath);
				return (NULL);
			}
		}
	}

	return anItemList;
}

CFMutableDictionaryRef StartupItemListGetProvider(CFArrayRef anItemList, CFStringRef aService)
{
	CFMutableDictionaryRef aResult = NULL;
	CFMutableArrayRef aList = startupItemListCopyMatches(anItemList, kProvidesKey, aService);

	if (aList && CFArrayGetCount(aList) > 0)
		aResult = (CFMutableDictionaryRef) CFArrayGetValueAtIndex(aList, 0);

	if (aList) CFRelease(aList);

	return aResult;
}

CFArrayRef StartupItemListCreateFromRunning(CFArrayRef anItemList)
{
	CFMutableArrayRef aResult = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	if (aResult) {
		CFIndex anIndex, aCount = CFArrayGetCount(anItemList);
		for (anIndex = 0; anIndex < aCount; ++anIndex) {
			CFDictionaryRef anItem = CFArrayGetValueAtIndex(anItemList, anIndex);
			if (anItem) {
				CFNumberRef aPID = CFDictionaryGetValue(anItem, kPIDKey);
				if (aPID)
					CFArrayAppendValue(aResult, anItem);
			}
		}
	}
	return aResult;
}

/*
 * Append items in anItemList to aDependents which depend on
 *  aParentItem.
 * If anAction is kActionStart, dependent items are those which
 *  require any service provided by aParentItem.
 * If anAction is kActionStop, dependent items are those which provide
 *  any service required by aParentItem.
 */
static void appendDependents(CFMutableArrayRef aDependents, CFArrayRef anItemList, CFDictionaryRef aParentItem, Action anAction)
{
	CFStringRef anInnerKey, anOuterKey;
	CFArrayRef anOuterList;

	/* Append the parent item to the list (avoiding duplicates) */
	if (!CFArrayContainsValue(aDependents, CFRangeMake(0, CFArrayGetCount(aDependents)), aParentItem))
		CFArrayAppendValue(aDependents, aParentItem);

	/**
         * Recursively append any children of the parent item for kStartAction and kStopAction.
         * Do nothing for other actions.
         **/
	switch (anAction) {
	case kActionStart:
		anInnerKey = kProvidesKey;
		anOuterKey = kRequiresKey;
		break;
	case kActionStop:
		anInnerKey = kRequiresKey;
		anOuterKey = kProvidesKey;
		break;
	default:
		return;
	}

	anOuterList = CFDictionaryGetValue(aParentItem, anOuterKey);

	if (anOuterList) {
		CFIndex anOuterCount = CFArrayGetCount(anOuterList);
		CFIndex anOuterIndex;

		for (anOuterIndex = 0; anOuterIndex < anOuterCount; anOuterIndex++) {
			CFStringRef anOuterElement = CFArrayGetValueAtIndex(anOuterList, anOuterIndex);
			CFIndex anItemCount = CFArrayGetCount(anItemList);
			CFIndex anItemIndex;

			for (anItemIndex = 0; anItemIndex < anItemCount; anItemIndex++) {
				CFDictionaryRef anItem = CFArrayGetValueAtIndex(anItemList, anItemIndex);
				CFArrayRef anInnerList = CFDictionaryGetValue(anItem, anInnerKey);

				if (anInnerList &&
				    CFArrayContainsValue(anInnerList, CFRangeMake(0, CFArrayGetCount(anInnerList)),
							 anOuterElement)
				    && !CFArrayContainsValue(aDependents, CFRangeMake(0, CFArrayGetCount(aDependents)), anItem))
					appendDependents(aDependents, anItemList, anItem, anAction);
			}
		}
	}
}

CFMutableArrayRef StartupItemListCreateDependentsList(CFMutableArrayRef anItemList, CFStringRef aService, Action anAction)
{
	CFMutableArrayRef aDependents = NULL;
	CFMutableDictionaryRef anItem = NULL;

	if (anItemList && aService)
		anItem = StartupItemListGetProvider(anItemList, aService);

	if (anItem) {
		switch (anAction) {
		case kActionRestart:
		case kActionStart:
		case kActionStop:
			aDependents = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

			if (!aDependents) {
				CF_syslog(LOG_EMERG, CFSTR("Failed to allocate dependancy list for item %@"), anItem);
				return NULL;
			}
			appendDependents(aDependents, anItemList, anItem, anAction);
			break;

		default:
			break;
		}
	}
	return aDependents;
}

/**
 * countUnmetRequirements counts the number of items in anItemList
 * which are pending in aStatusDict.
 **/
static int countUnmetRequirements(CFDictionaryRef aStatusDict, CFArrayRef anItemList)
{
	int aCount = 0;
	CFIndex anItemCount = CFArrayGetCount(anItemList);
	CFIndex anItemIndex;

	for (anItemIndex = 0; anItemIndex < anItemCount; anItemIndex++) {
		CFStringRef anItem = CFArrayGetValueAtIndex(anItemList, anItemIndex);
		CFStringRef aStatus = CFDictionaryGetValue(aStatusDict, anItem);

		if (!aStatus || !CFEqual(aStatus, kRunSuccess)) {
			CF_syslog(LOG_DEBUG, CFSTR("\tFailed requirement/uses: %@"), anItem);
			aCount++;
		}
	}

	return aCount;
}

/**
 * countDependantsPresent counts the number of items in aWaitingList
 * which depend on items in anItemList.
 **/
static int countDependantsPresent(CFArrayRef aWaitingList, CFArrayRef anItemList, CFStringRef aKey)
{
	int aCount = 0;
	CFIndex anItemCount = CFArrayGetCount(anItemList);
	CFIndex anItemIndex;

	for (anItemIndex = 0; anItemIndex < anItemCount; anItemIndex++) {
		CFStringRef anItem = CFArrayGetValueAtIndex(anItemList, anItemIndex);
		CFArrayRef aMatchesList = startupItemListCopyMatches(aWaitingList, aKey, anItem);

		if (aMatchesList) {
			aCount = aCount + CFArrayGetCount(aMatchesList);
			CFRelease(aMatchesList);
		}
	}

	return aCount;
}

/**
 * pendingAntecedents returns TRUE if any antecedents of this item
 * are currently running, have not yet run, or none exist.
 **/
static Boolean
pendingAntecedents(CFArrayRef aWaitingList, CFDictionaryRef aStatusDict, CFArrayRef anAntecedentList, Action anAction)
{
	int aPendingFlag = FALSE;

	CFIndex anAntecedentCount = CFArrayGetCount(anAntecedentList);
	CFIndex anAntecedentIndex;

	for (anAntecedentIndex = 0; anAntecedentIndex < anAntecedentCount; ++anAntecedentIndex) {
		CFStringRef anAntecedent = CFArrayGetValueAtIndex(anAntecedentList, anAntecedentIndex);
		CFStringRef aKey = (anAction == kActionStart) ? kProvidesKey : kUsesKey;
		CFArrayRef aMatchesList = startupItemListCopyMatches(aWaitingList, aKey, anAntecedent);

		if (aMatchesList) {
			CFIndex aMatchesListCount = CFArrayGetCount(aMatchesList);
			CFIndex aMatchesListIndex;

			for (aMatchesListIndex = 0; aMatchesListIndex < aMatchesListCount; ++aMatchesListIndex) {
				CFDictionaryRef anItem = CFArrayGetValueAtIndex(aMatchesList, aMatchesListIndex);

				if (!anItem ||
				    !CFDictionaryGetValue(anItem, kPIDKey) || !CFDictionaryGetValue(aStatusDict, anAntecedent)) {
					aPendingFlag = TRUE;
					break;
				}
			}

			CFRelease(aMatchesList);

			if (aPendingFlag)
				break;
		}
	}
	return (aPendingFlag);
}

/**
 * checkForDuplicates returns TRUE if an item provides the same service as a
 * pending item, or an item that already succeeded.
 **/
static Boolean checkForDuplicates(CFArrayRef aWaitingList, CFDictionaryRef aStatusDict, CFDictionaryRef anItem)
{
	int aDuplicateFlag = FALSE;

	CFArrayRef aProvidesList = CFDictionaryGetValue(anItem, kProvidesKey);
	CFIndex aProvidesCount = aProvidesList ? CFArrayGetCount(aProvidesList) : 0;
	CFIndex aProvidesIndex;

	for (aProvidesIndex = 0; aProvidesIndex < aProvidesCount; ++aProvidesIndex) {
		CFStringRef aProvides = CFArrayGetValueAtIndex(aProvidesList, aProvidesIndex);

		/* If the service succeeded, return true. */
		CFStringRef aStatus = CFDictionaryGetValue(aStatusDict, aProvides);
		if (aStatus && CFEqual(aStatus, kRunSuccess)) {
			aDuplicateFlag = TRUE;
			break;
		}
		/*
		 * Otherwise test if any item is currently running which
		 * might provide that service.
		 */
		else {
			CFArrayRef aMatchesList = startupItemListCopyMatches(aWaitingList, kProvidesKey, aProvides);
			if (aMatchesList) {
				CFIndex aMatchesListCount = CFArrayGetCount(aMatchesList);
				CFIndex aMatchesListIndex;

				for (aMatchesListIndex = 0; aMatchesListIndex < aMatchesListCount; ++aMatchesListIndex) {
					CFDictionaryRef anDupItem = CFArrayGetValueAtIndex(aMatchesList, aMatchesListIndex);
					if (anDupItem && CFDictionaryGetValue(anDupItem, kPIDKey)) {
						/*
						 * Item is running, avoid
						 * race condition.
						 */
						aDuplicateFlag = TRUE;
						break;
					} else {
						CFNumberRef anItemDomain = CFDictionaryGetValue(anItem, kDomainKey);
						CFNumberRef anotherItemDomain = CFDictionaryGetValue(anDupItem, kDomainKey);
						/*
						 * If anItem was found later
						 * than aDupItem, stall
						 * anItem until aDupItem
						 * runs.
						 */
						if (anItemDomain &&
						    anotherItemDomain &&
						    CFNumberCompare(anItemDomain, anotherItemDomain,
								    NULL) == kCFCompareGreaterThan) {
							/*
							 * Item not running,
							 * but takes
							 * precedence.
							 */
							aDuplicateFlag = TRUE;
							break;
						}
					}
				}

				CFRelease(aMatchesList);
				if (aDuplicateFlag)
					break;
			}
		}
	}
	return (aDuplicateFlag);
}

CFMutableDictionaryRef StartupItemListGetNext(CFArrayRef aWaitingList, CFDictionaryRef aStatusDict, Action anAction)
{
	CFMutableDictionaryRef aNextItem = NULL;
	CFIndex aWaitingCount = CFArrayGetCount(aWaitingList);
	int aMinFailedAntecedents = INT_MAX;
	CFIndex aWaitingIndex;

	switch (anAction) {
	case kActionStart:
		break;
	case kActionStop:
		break;
	case kActionRestart:
		break;
	default:
		return NULL;
	}

	if (!aWaitingList || !aStatusDict || aWaitingCount <= 0)
		return NULL;

	/**
         * Iterate through the items in aWaitingList and look for an optimally ready item.
         **/
	for (aWaitingIndex = 0; aWaitingIndex < aWaitingCount; aWaitingIndex++) {
		CFMutableDictionaryRef anItem = (CFMutableDictionaryRef) CFArrayGetValueAtIndex(aWaitingList, aWaitingIndex);
		CFArrayRef anAntecedentList;
		int aFailedAntecedentsCount = 0;	/* Number of unmet soft
							 * depenancies */
		Boolean aBestPick = FALSE;	/* Is this the best pick
						 * so far?    */

		/* Filter out running items. */
		if (CFDictionaryGetValue(anItem, kPIDKey))
			continue;

		/*
		 * Filter out dupilicate services; if someone has
		 * provided what we provide, we don't run.
		 */
		if (checkForDuplicates(aWaitingList, aStatusDict, anItem)) {
			CF_syslog(LOG_DEBUG, CFSTR("Skipping %@ because of duplicate service."),
				  CFDictionaryGetValue(anItem, kDescriptionKey));
			continue;
		}
		/*
		 * Dependencies don't matter when restarting an item;
		 * stop here.
		 */
		if (anAction == kActionRestart) {
			aNextItem = anItem;
			break;
		}
		anAntecedentList = CFDictionaryGetValue(anItem, ((anAction == kActionStart) ? kRequiresKey : kProvidesKey));

		CF_syslog(LOG_DEBUG, CFSTR("Checking %@"), CFDictionaryGetValue(anItem, kDescriptionKey));

		if (anAntecedentList)
			CF_syslog(LOG_DEBUG, CFSTR("Antecedents: %@"), anAntecedentList);
		else
			syslog(LOG_DEBUG, "No antecedents");

			/**
	                 * Filter out the items which have unsatisfied antecedents.
	                 **/
		if (anAntecedentList &&
		    ((anAction == kActionStart) ?
		     countUnmetRequirements(aStatusDict, anAntecedentList) :
		     countDependantsPresent(aWaitingList, anAntecedentList, kRequiresKey)))
			continue;

			/**
	                 * anItem has all hard dependancies met; check for soft dependancies.
	                 * We'll favor the item with the fewest unmet soft dependancies here.
	                 **/
		anAntecedentList = CFDictionaryGetValue(anItem, ((anAction == kActionStart) ? kUsesKey : kProvidesKey));

		if (anAntecedentList)
			CF_syslog(LOG_DEBUG, CFSTR("Soft dependancies: %@"), anAntecedentList);
		else
			syslog(LOG_DEBUG, "No soft dependancies");

		if (anAntecedentList) {
			aFailedAntecedentsCount =
			    ((anAction == kActionStart) ?
			     countUnmetRequirements(aStatusDict, anAntecedentList) :
			     countDependantsPresent(aWaitingList, anAntecedentList, kUsesKey));
		} else {
			if (aMinFailedAntecedents > 0)
				aBestPick = TRUE;
		}

		/*
		 * anItem has unmet dependencies that will
		 * likely be met in the future, so delay it
		 */
		if (aFailedAntecedentsCount > 0 && pendingAntecedents(aWaitingList, aStatusDict, anAntecedentList, anAction)) {
			continue;
		}
		if (aFailedAntecedentsCount > 0)
			syslog(LOG_DEBUG, "Total: %d", aFailedAntecedentsCount);

		if (aFailedAntecedentsCount > aMinFailedAntecedents)
			continue;	/* Another item already won out */

		if (aFailedAntecedentsCount < aMinFailedAntecedents)
			aBestPick = TRUE;

		if (!aBestPick)
			continue;

		/*
		 * anItem has less unmet
		 * dependancies than any
		 * other item so far, so it
		 * wins.
		 */
		syslog(LOG_DEBUG, "Best pick so far, based on failed dependancies (%d->%d)",
		       aMinFailedAntecedents, aFailedAntecedentsCount);

		/*
		 * We have a winner!  Update success
		 * parameters to match anItem.
		 */
		aMinFailedAntecedents = aFailedAntecedentsCount;
		aNextItem = anItem;

	}			/* End of waiting list loop. */

	return aNextItem;
}

CFStringRef StartupItemCreateDescription(CFMutableDictionaryRef anItem)
{
	CFStringRef aString = NULL;

	if (anItem)
		aString = CFDictionaryGetValue(anItem, kDescriptionKey);
	if (aString)
		CFRetain(aString);
	return aString;
}

pid_t StartupItemGetPID(CFDictionaryRef anItem)
{
	CFIndex anItemPID = 0;
	CFNumberRef aPIDNumber = anItem ? CFDictionaryGetValue(anItem, kPIDKey) : NULL;
	if (aPIDNumber && CFNumberGetValue(aPIDNumber, kCFNumberCFIndexType, &anItemPID))
		return (pid_t) anItemPID;
	else
		return 0;
}

CFMutableDictionaryRef StartupItemWithPID(CFArrayRef anItemList, pid_t aPID)
{
	CFIndex anItemCount = CFArrayGetCount(anItemList);
	CFIndex anItemIndex;

	for (anItemIndex = 0; anItemIndex < anItemCount; anItemIndex++) {
		CFMutableDictionaryRef anItem = (CFMutableDictionaryRef) CFArrayGetValueAtIndex(anItemList, anItemIndex);
		CFNumberRef aPIDNumber = CFDictionaryGetValue(anItem, kPIDKey);
		CFIndex anItemPID;

		if (aPIDNumber) {
			CFNumberGetValue(aPIDNumber, kCFNumberCFIndexType, &anItemPID);

			if ((pid_t) anItemPID == aPID)
				return anItem;
		}
	}

	return NULL;
}

int StartupItemRun(CFMutableDictionaryRef aStatusDict, CFMutableDictionaryRef anItem, Action anAction)
{
	int             anError = -1;
	CFArrayRef      aProvidesList = CFDictionaryGetValue(anItem, kProvidesKey);
	static const CFStringRef stubitems[] = {
		CFSTR("BootROMUpdater"),		/* 3893064 */
		CFSTR("FCUUpdater"),			/* 3893064 */
		CFSTR("AutoProtect Daemon"),		/* 3965785 */
		CFSTR("Check For Missed Tasks"),	/* 3965785 */
		CFSTR("Privacy"),			/* 3933484 */
		CFSTR("Firmware Update Checking"),	/* 4001504 */

		CFSTR("M-Audio FireWire Audio Support"),		/* 3931757 */
		CFSTR("help for M-Audio Delta Family"),			/* 3931757 */
		CFSTR("help for M-Audio Devices"),			/* 3931757 */
		CFSTR("help for M-Audio Revo 5.1"),			/* 3931757 */
		CFSTR("M-Audio USB Duo Configuration Service"),		/* 3931757 */
		CFSTR("firmware loader for M-Audio devices"),		/* 3931757 */
		CFSTR("M-Audio MobilePre USB Configuration Service"),	/* 3931757 */
		CFSTR("M-Audio OmniStudio USB Configuration Service"),	/* 3931757 */
		CFSTR("M-Audio Transit USB Configuration Service"),	/* 3931757 */
		CFSTR("M-Audio Audiophile USB Configuration Service"),	/* 3931757 */
		NULL
	};
	const CFStringRef *c;

	if (aProvidesList && anAction == kActionStop) {
		CFIndex aProvidesCount = CFArrayGetCount(aProvidesList);
		for (c = stubitems; *c; c++) {
			if (CFArrayContainsValue(aProvidesList, CFRangeMake(0, aProvidesCount), *c)) {
				CFIndex         aPID = -1;
				CFNumberRef     aProcessNumber = CFNumberCreate(NULL, kCFNumberCFIndexType, &aPID);

				CFDictionarySetValue(anItem, kPIDKey, aProcessNumber);
				CFRelease(aProcessNumber);

				StartupItemExit(aStatusDict, anItem, TRUE);
				return -1;
			}
		}
	}

	if (anAction == kActionNone) {
		StartupItemExit(aStatusDict, anItem, TRUE);
		anError = 0;
	} else {
		CFStringRef aBundlePathString = CFDictionaryGetValue(anItem, kBundlePathKey);
		char aBundlePath[PATH_MAX];
		char anExecutable[PATH_MAX];
		char *tmp;

		if (!CFStringGetCString(aBundlePathString, aBundlePath, sizeof(aBundlePath), kCFStringEncodingUTF8)) {
			CF_syslog(LOG_EMERG, CFSTR("Internal error while running item %@"), aBundlePathString);
			return (anError);
		}
		/* Compute path to excecutable */
		tmp = rindex(aBundlePath, '/');
		snprintf(anExecutable, sizeof(anExecutable), "%s%s", aBundlePath, tmp);

		/**
	         * Run the bundle
	         **/

		if (access(anExecutable, X_OK)) {
			/*
			 * Add PID key so that this item is marked as having
			 * been run.
			 */
			CFIndex aPID = -1;
			CFNumberRef aProcessNumber = CFNumberCreate(NULL, kCFNumberCFIndexType, &aPID);

			CFDictionarySetValue(anItem, kPIDKey, aProcessNumber);
			CFRelease(aProcessNumber);

			CFDictionarySetValue(anItem, kErrorKey, kErrorPermissions);
			StartupItemExit(aStatusDict, anItem, FALSE);
			syslog(LOG_ERR, "No executable file %s", anExecutable);
		} else {
			pid_t aProccessID = fork();

			switch (aProccessID) {
			case -1:	/* SystemStarter (fork failed) */
				CFDictionarySetValue(anItem, kErrorKey, kErrorFork);
				StartupItemExit(aStatusDict, anItem, FALSE);

				CF_syslog(LOG_ERR, CFSTR("Failed to fork for item %@: %s"), aBundlePathString, strerror(errno));

				break;

			default:	/* SystemStarter (fork succeeded) */
				{
					CFIndex aPID = (CFIndex) aProccessID;
					CFNumberRef aProcessNumber = CFNumberCreate(NULL, kCFNumberCFIndexType, &aPID);

					CFDictionarySetValue(anItem, kPIDKey, aProcessNumber);
					CFRelease(aProcessNumber);

					syslog(LOG_DEBUG, "Running command (%d): %s %s",
					       aProccessID, anExecutable, argumentForAction(anAction));
					anError = 0;
				}
				break;

			case 0:	/* Child */
				{
					if (setsid() == -1)
						syslog(LOG_WARNING, "Unable to create session for item %s: %m", anExecutable);

					anError = execl(anExecutable, anExecutable, argumentForAction(anAction), NULL);

					/* We shouldn't get here. */

					syslog(LOG_ERR, "execl(\"%s\"): %m", anExecutable);

					exit(anError);
				}
			}
		}
	}

	return (anError);
}

void
StartupItemSetStatus(CFMutableDictionaryRef aStatusDict, CFMutableDictionaryRef anItem, CFStringRef aServiceName,
		     Boolean aSuccess, Boolean aReplaceFlag)
{
	void (*anAction) (CFMutableDictionaryRef, const void *, const void *) = aReplaceFlag ?
	    CFDictionarySetValue : CFDictionaryAddValue;

	if (aStatusDict && anItem) {
		CFArrayRef aProvidesList = CFDictionaryGetValue(anItem, kProvidesKey);
		if (aProvidesList) {
			CFIndex aProvidesCount = CFArrayGetCount(aProvidesList);
			CFIndex aProvidesIndex;

			/*
			 * If a service name was specified, and it is valid,
			 * use only it.
			 */
			if (aServiceName && CFArrayContainsValue(aProvidesList, CFRangeMake(0, aProvidesCount), aServiceName)) {
				aProvidesList = CFArrayCreate(NULL, (const void **)&aServiceName, 1, &kCFTypeArrayCallBacks);
				aProvidesCount = 1;
			} else {
				CFRetain(aProvidesList);
			}

			for (aProvidesIndex = 0; aProvidesIndex < aProvidesCount; aProvidesIndex++) {
				CFStringRef aService = CFArrayGetValueAtIndex(aProvidesList, aProvidesIndex);

				if (aSuccess)
					anAction(aStatusDict, aService, kRunSuccess);
				else
					anAction(aStatusDict, aService, kRunFailure);
			}

			CFRelease(aProvidesList);
		}
	}
}

void StartupItemExit(CFMutableDictionaryRef aStatusDict, CFMutableDictionaryRef anItem, Boolean aSuccess)
{
	StartupItemSetStatus(aStatusDict, anItem, NULL, aSuccess, FALSE);
}
