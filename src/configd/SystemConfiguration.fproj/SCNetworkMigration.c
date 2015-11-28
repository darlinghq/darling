/*
 * Copyright (c) 2014 Apple Inc. All rights reserved.
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
//  SCNetworkMigration.c
//
//  Created by Ashish Kulkarni on 11/19/13.
//
//

#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCNetworkConfigurationPrivate.h>
#include <SystemConfiguration/SCNetworkConfigurationInternal.h>
#include "SCPreferencesInternal.h"
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IONetworkController.h>
#include <SystemConfiguration/SCPrivate.h>
#include <sys/stat.h>
#include <copyfile.h>
#include <sys/param.h>
#include <pthread.h>

#define BACK_TO_MY_MAC			CFSTR("BackToMyMac")
#define BACK_TO_MY_MAC_DSIDS		CFSTR("BackToMyMacDSIDs")
#define	PREFS_DEFAULT_DIR_PLIST		"/Library/Preferences/SystemConfiguration"
#define PREFS_DEFAULT_DIR_RELATIVE	CFSTR("Library/Preferences/SystemConfiguration/")
#define	PREFS_DEFAULT_CONFIG_PLIST	"preferences.plist"
#define	NETWORK_INTERFACES_PREFS_PLIST	"NetworkInterfaces.plist"
#define NUM_MIGRATION_PATHS		2
#define PLUGIN_ID			CFSTR("System Migration")
#define PREFERENCES_PLIST_INDEX		0
#define NETWORK_INTERFACES_PLIST_INDEX	1


const CFStringRef kSCNetworkConfigurationMigrationActionKey = CFSTR("MigrationActionKey");
const CFStringRef kSCNetworkConfigurationRepair = CFSTR("ConfigurationRepair");
#if	!TARGET_OS_IPHONE
static CFDictionaryRef
_SCNetworkMigrationCopyMappingBSDNameToBridgeServices(SCPreferencesRef prefs);

static CFDictionaryRef
_SCNetworkMigrationCopyMappingBSDNameToBondServices(SCPreferencesRef prefs);

static CFDictionaryRef
_SCNetworkMigrationCopyMappingBSDNameToVLANServices(SCPreferencesRef prefs);
#endif
static Boolean
_SCNetworkConfigurationIsInterfaceNamerMappable(SCNetworkInterfaceRef interface1, SCNetworkInterfaceRef interface2, Boolean bypassActive);

static Boolean
_SCNetworkConfigurationMigrateConfiguration(CFURLRef sourceDir, CFURLRef targetDir);

static void
_SCNetworkConfigurationCopyMigrationPathsWithBaseURL(CFURLRef baseURL, CFURLRef *prefs, CFURLRef *interfaces)
{
	if (baseURL != NULL) {
		CFRetain(baseURL);
	}
	else {
		baseURL = CFURLCreateFromFileSystemRepresentation(NULL,
								  (UInt8*)PREFS_DEFAULT_DIR_PLIST,
								  sizeof(PREFS_DEFAULT_DIR_PLIST),
								  TRUE);
	}

	*prefs = CFURLCreateFromFileSystemRepresentationRelativeToBase(NULL,
								      (UInt8*)PREFS_DEFAULT_CONFIG_PLIST,
								      sizeof(PREFS_DEFAULT_CONFIG_PLIST) - 1,
								      FALSE, baseURL);

	*interfaces = CFURLCreateFromFileSystemRepresentationRelativeToBase(NULL,
									   (UInt8*)NETWORK_INTERFACES_PREFS_PLIST,
									   sizeof(NETWORK_INTERFACES_PREFS_PLIST) - 1,
									   FALSE, baseURL);
	CFRelease(baseURL);
	return;
}

CFArrayRef
_SCNetworkConfigurationCopyMigrationPaths(CFDictionaryRef options)
{
	CFURLRef interfaces;
	CFMutableArrayRef migrationPaths = NULL;
	CFURLRef prefs;
	
	if (getenv(INSTALL_ENVIRONMENT) != NULL) {
		_sc_debug = 1;
	}
	_SCNetworkConfigurationCopyMigrationPathsWithBaseURL(NULL, &prefs, &interfaces);

	migrationPaths = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	CFArrayInsertValueAtIndex(migrationPaths, PREFERENCES_PLIST_INDEX, prefs);
	CFArrayInsertValueAtIndex(migrationPaths, NETWORK_INTERFACES_PLIST_INDEX, interfaces);

	CFRelease(prefs);
	CFRelease(interfaces);
	return migrationPaths;
}

static Boolean
_SCNetworkConfigurationRemoveConfigurationFiles(CFURLRef configDir)
{
	
	char configPathString[PATH_MAX];
	CFURLRef configPathURL = NULL;
	char configNetworkInterfacesPathString[PATH_MAX];
	CFURLRef configNetworkInterfacesPathURL = NULL;
	
	_SCNetworkConfigurationCopyMigrationPathsWithBaseURL(configDir, &configPathURL, &configNetworkInterfacesPathURL);
	
	if(CFURLGetFileSystemRepresentation(configPathURL, TRUE, (UInt8*)configPathString, sizeof(configPathString)) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationRemoveConfigurationFiles: configPathString is NULL"));
	}
	else {
		if (remove(configPathString) != 0) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationRemoveConfigurationFiles: Removing file failed from: %s. Error is %d"), configPathString, errno);
		}
	}
	
	if(CFURLGetFileSystemRepresentation(configNetworkInterfacesPathURL, TRUE, (UInt8*)configNetworkInterfacesPathString, sizeof(configNetworkInterfacesPathString)) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationRemoveConfigurationFiles: configNetwork"));
	}
	else {
		if (remove(configNetworkInterfacesPathString) != 0) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationRemoveConfigurationFiles: Removing file failed from: %s. Error is %d"), configNetworkInterfacesPathString, errno);
		}
	}
	
	if (configPathURL != NULL) {
		CFRelease(configPathURL);
	}
	if (configNetworkInterfacesPathURL != NULL) {
		CFRelease(configNetworkInterfacesPathURL);
	}
	
	return TRUE;
}

static Boolean
SCNetworkConfigurationCopyConfigurationFiles(CFURLRef	configDir,
					     CFURLRef	targetDir)      // TargetDir needs to exist
{
	errno_t error;
	mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
	char networkInterfacesPathString[PATH_MAX];
	CFURLRef networkInterfacesPathURL = NULL;
	copyfile_state_t networkInterfacesState;
	char preferencesPathString[PATH_MAX];
	CFURLRef preferencesPathURL = NULL;
	Boolean removeTargetFiles = FALSE;
	copyfile_state_t state;
	Boolean success = FALSE;
	char targetNetworkInterfacesPathString[PATH_MAX];
	CFURLRef targetNetworkInterfacesPathURL = NULL;
	char targetPathString[PATH_MAX];
	CFURLRef targetPathURL = NULL;

	_SCNetworkConfigurationCopyMigrationPathsWithBaseURL(targetDir, &targetPathURL, &targetNetworkInterfacesPathURL);
	
	if (CFURLGetFileSystemRepresentation(targetPathURL, TRUE, (UInt8*)targetPathString, sizeof(targetPathString)) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("SCNetworkConfigurationCopyConfigurationFiles: targetPathString is NULL"));
		goto done;
	}
	
	if (CFURLGetFileSystemRepresentation(targetNetworkInterfacesPathURL, TRUE, (UInt8*)targetNetworkInterfacesPathString, sizeof(targetNetworkInterfacesPathString)) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("SCNetworkConfigurationCopyConfigurationFiles: targetNetworkInterfacesPathString is NULL"));
		goto done;
	}
	
	if (configDir == NULL) {
		removeTargetFiles = TRUE;
		success = TRUE;
		goto done;
	}
	_SCNetworkConfigurationCopyMigrationPathsWithBaseURL(configDir, &preferencesPathURL, &networkInterfacesPathURL);

	if (CFURLGetFileSystemRepresentation(preferencesPathURL, TRUE, (UInt8*)preferencesPathString, sizeof(preferencesPathString)) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("SCNetworkConfigurationCopyConfigurationFiles: preferencesPathString is NULL"));
		goto done;
	}


	if (CFURLGetFileSystemRepresentation(networkInterfacesPathURL, TRUE, (UInt8*)networkInterfacesPathString, sizeof(networkInterfacesPathString)) == FALSE) {
		SCLog(_sc_debug, LOG_DEBUG, CFSTR("SCNetworkConfigurationCopyConfigurationFiles: networkInterfacePathString is NULL"));
		goto done;
	}
	
	state = copyfile_state_alloc();
	if ((error = copyfile(preferencesPathString, targetPathString, state, COPYFILE_ALL)) != 0) {
		SCLog(TRUE, LOG_ERR, CFSTR("SCNetworkConfigurationCopyConfigurationFiles: Copying failed from:%s to %s. Error is %d"), preferencesPathString, targetPathString, errno);
		copyfile_state_free(state);
		removeTargetFiles = TRUE;
		goto done;
	}
	copyfile_state_free(state);
	chmod(targetPathString, mode);

	networkInterfacesState = copyfile_state_alloc();
	if ((error = copyfile(networkInterfacesPathString, targetNetworkInterfacesPathString, networkInterfacesState, COPYFILE_ALL)) != 0) {
		SCLog(TRUE, LOG_ERR, CFSTR("SCNetworkConfigurationCopyConfigurationFiles: Copying failed from:%s to %s. Error is %d"), networkInterfacesPathString, targetNetworkInterfacesPathString, errno);
		copyfile_state_free(networkInterfacesState);
		removeTargetFiles = TRUE;
		goto done;
	}
	copyfile_state_free(networkInterfacesState);
	chmod(targetNetworkInterfacesPathString, mode);

	success = TRUE;
done:
	if (removeTargetFiles == TRUE) {
		_SCNetworkConfigurationRemoveConfigurationFiles(targetDir);
	}
	if (preferencesPathURL != NULL) {
		CFRelease(preferencesPathURL);
	}
	if (networkInterfacesPathURL != NULL) {
		CFRelease(networkInterfacesPathURL);
	}
	if (targetPathURL != NULL) {
		CFRelease(targetPathURL);
	}
	if (targetNetworkInterfacesPathURL != NULL) {
		CFRelease(targetNetworkInterfacesPathURL);
	}
	return success;
}


/* -----------------------------------------------------------------------------
 Create directories and intermediate directories as required.
 ----------------------------------------------------------------------------- */
static Boolean
_SCNetworkConfigurationMakePathIfNeeded(CFURLRef pathURL)
{
	char    *c;
	mode_t  newmask;
	char	path[PATH_MAX];
	char    thepath[PATH_MAX];
	CFIndex slen=0;
	struct stat sb;
	Boolean success = FALSE;

	if (CFURLGetFileSystemRepresentation(pathURL, TRUE, (UInt8 *)path, sizeof(path)) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMakePathIfNeeded: Could not get character array from target string"));
		return success;
	}
	SCLog(TRUE, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMakePathIfNeeded: Creating path: %s"), path);
	
	newmask = S_IRWXU | S_IRGRP | S_IROTH | S_IXGRP | S_IXOTH;

	slen = strlen(path);

	strlcpy( thepath, path, slen+1);
	c = thepath;
	if ( *c == '/' )
		c++;
	for(  ; !success; c++){
		if ( (*c == '/') || ( *c == '\0' )){
			if ( *c == '\0' )
				success = TRUE;
			else
				*c = '\0';
			if ( mkdir( thepath, newmask) ){
				if ( errno == EEXIST || errno == EISDIR){
					if ( stat(thepath, &sb) < 0){
						printf("stat returned value < 0\n");
						break;
					}
				} else {
					printf("received error: %s\n", strerror(errno));
					break;
				}
			}
			*c = '/';
		}
	}
	return success;
}

static SCPreferencesRef
__SCNetworkCreateDefaultPref(CFStringRef prefsID)
{
	SCPreferencesRef prefs;
	SCNetworkSetRef currentSet;
	CFStringRef model;
	
	prefs = SCPreferencesCreate(NULL, PLUGIN_ID, prefsID);
	if (prefs == NULL) {
		return NULL;
	}
	
	currentSet = SCNetworkSetCopyCurrent(prefs);
	if (currentSet == NULL) {
		CFBundleRef bundle;
		CFStringRef setName = NULL;
		
		currentSet = SCNetworkSetCreate(prefs);
		bundle = _SC_CFBundleGet();
		if (bundle != NULL) {
			setName = CFBundleCopyLocalizedString(bundle,
							      CFSTR("DEFAULT_SET_NAME"),
							      CFSTR("Automatic"),
							      NULL);
		}
		SCNetworkSetSetName(currentSet, (setName != NULL) ? setName : CFSTR("Automatic"));
		SCNetworkSetSetCurrent(currentSet);
		if (setName != NULL) {
			CFRelease(setName);
		}
	}
	SCNetworkSetEstablishDefaultConfiguration(currentSet);
	CFRelease(currentSet);
	
	model = SCPreferencesGetValue(prefs, MODEL);
	if (model == NULL) {
		model = _SC_hw_model(FALSE);
		SCPreferencesSetValue(prefs, MODEL, model);
	}
	
	return prefs;
}

__private_extern__
SCPreferencesRef
__SCNetworkCreateDefaultNIPrefs(CFStringRef prefsID)
{
	CFMutableArrayRef interfaces = NULL;
	CFStringRef model;
	CFArrayRef networkInterfaces;
	SCPreferencesRef ni_prefs;
	CFComparisonResult res;
	
	
	networkInterfaces = __SCNetworkInterfaceCopyAll_IONetworkInterface();
	
	if (networkInterfaces == NULL) {
		return NULL;
	}
	if (prefsID == NULL) {
		prefsID = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@/%@"), PREFS_DEFAULT_DIR, NETWORK_INTERFACES_PREFS);
	}
	else {
		CFRetain(prefsID);
	}
	
	ni_prefs = SCPreferencesCreate(NULL, PLUGIN_ID , prefsID);
	CFRelease(prefsID);
	
	if (ni_prefs == NULL) {
		goto done;
	}
	
	interfaces = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	
	for (CFIndex idx = 0; idx < CFArrayGetCount(networkInterfaces); idx++) {
		CFIndex idx2 = 0;
		CFNumberRef if_type;
		CFNumberRef if_unit;
		SCNetworkInterfaceRef interface = CFArrayGetValueAtIndex(networkInterfaces, idx);
		CFDictionaryRef interfaceEntity = __SCNetworkInterfaceCopyStorageEntity(interface);
		
		if (interfaceEntity == NULL) {
			continue;
		}
		
		if_type = _SCNetworkInterfaceGetIOInterfaceType(interface);
		if_unit = _SCNetworkInterfaceGetIOInterfaceUnit(interface);
		
		if ((if_type == NULL) || (if_unit == NULL)) {
			CFRelease(interfaceEntity);
			continue;
		}
		
		for (idx2 = 0; idx2 < CFArrayGetCount(interfaces); idx2++) {
			CFNumberRef db_type;
			CFNumberRef db_unit;
			CFDictionaryRef dict = CFArrayGetValueAtIndex(interfaces, idx2);
			
			db_type = CFDictionaryGetValue(dict, CFSTR(kIOInterfaceType));
			db_unit = CFDictionaryGetValue(dict, CFSTR(kIOInterfaceUnit));
			res = CFNumberCompare(if_type, db_type, NULL);
			if (res == kCFCompareLessThan
			|| (res == kCFCompareEqualTo
			&& (CFNumberCompare(if_unit, db_unit, NULL) == kCFCompareLessThan))) {
				break;
			}
		}
		
		CFArrayInsertValueAtIndex(interfaces, idx2, interfaceEntity);
		CFRelease(interfaceEntity);
		
	}
	SCPreferencesSetValue(ni_prefs, INTERFACES, interfaces);
	
	model = SCPreferencesGetValue(ni_prefs, MODEL);
	if (model == NULL) {
		model = _SC_hw_model(FALSE);
		SCPreferencesSetValue(ni_prefs, MODEL, model);
	}
done:
	if (interfaces != NULL) {
		CFRelease(interfaces);
	}
	if (networkInterfaces != NULL) {
		CFRelease(networkInterfaces);
	}
	
	return ni_prefs;
}


/*
 *  _SCNetworkConfigurationPerformMigration will migrate configuration between source and destination systems
 */
CF_RETURNS_RETAINED
CFArrayRef
_SCNetworkConfigurationPerformMigration(CFURLRef sourceDir, CFURLRef currentDir, CFURLRef targetDir, CFDictionaryRef options)
{
	CFURLRef currentDirConfig = NULL;
	CFURLRef currentSystemPath = NULL;
	Boolean migrationComplete = FALSE;
	CFArrayRef paths = NULL;
	Boolean removeTargetOnFailure = FALSE;
	CFURLRef sourceDirConfig = NULL;
	CFURLRef targetDirConfig = NULL;
	
	if (getenv(INSTALL_ENVIRONMENT) != NULL) {
		_sc_debug = 1;
	}

	// Both sourceDir and currentDir cannot be NULL because NULL value indicates using current system
	if (sourceDir == NULL && currentDir == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationPerformMigration: Both sourceDir and currentDir are NULL"));
		goto done;
	}

	currentSystemPath = CFURLCreateWithString(NULL,
						  PREFS_DEFAULT_DIR,
						  NULL);

	// if either of the sourceDir or currentDir are NULL, then populate it with current system path
	if (sourceDir == NULL) {
		sourceDirConfig = CFRetain(currentSystemPath);
	}
	else {
		sourceDirConfig = CFURLCreateWithFileSystemPathRelativeToBase(NULL, PREFS_DEFAULT_DIR_RELATIVE, kCFURLPOSIXPathStyle, TRUE, sourceDir);
	}

	if (currentDir != NULL) {
		currentDirConfig = CFURLCreateWithFileSystemPathRelativeToBase(NULL, PREFS_DEFAULT_DIR_RELATIVE, kCFURLPOSIXPathStyle, TRUE, currentDir);
	}
	// If the targetDir is not provided then migration will take place in currentDir
	if (targetDir == NULL) {
		targetDirConfig = CFRetain(currentSystemPath);
	}
	else {
		targetDirConfig = CFURLCreateWithFileSystemPathRelativeToBase(NULL, PREFS_DEFAULT_DIR_RELATIVE, kCFURLPOSIXPathStyle, TRUE, targetDir);
	}
	// Source directory cannot be the same as Target Directory
	if (CFEqual(sourceDirConfig, targetDirConfig)) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationPerformMigration: Source directory cannot be the same as target directory"));
		goto done;
	}
	
	if ((currentDirConfig == NULL) || (CFEqual(currentDirConfig, targetDirConfig) == FALSE)) {
		if (_SCNetworkConfigurationMakePathIfNeeded(targetDirConfig) == FALSE) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationPerformMigration: Could not create target directory as expected"));
			goto done;
		}

		if (SCNetworkConfigurationCopyConfigurationFiles(currentDirConfig, targetDirConfig) == FALSE) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationPerformMigration: Could not copy configuration files from %@ to %@"),
			      currentDirConfig, targetDirConfig);
		}
		else if (currentDirConfig != NULL) {
			removeTargetOnFailure = TRUE;	// Configuration files were copied over to target directory
							// If migration failed, then we should remove those configuration
							// files since current directory and target directory are not
							// the same
		}
	}

	// If both source and current configurations point to current system, then no migration needs to be done.
	if ((currentDirConfig != NULL) && (CFEqual(sourceDirConfig, currentDirConfig) == TRUE)) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationPerformMigration:  both source and current configurations point to same path ... No migration needs to be done"));
		migrationComplete = TRUE;
	}
	else {
		migrationComplete = _SCNetworkConfigurationMigrateConfiguration(sourceDirConfig, targetDirConfig);
	}
	SCLog(TRUE, LOG_NOTICE, CFSTR("Migration %s"), migrationComplete ? "complete" : "failed");
	if (migrationComplete == TRUE) {
		paths = _SCNetworkConfigurationCopyMigrationPaths(NULL);
	}
	else {
		// If migration fails, then remove configuration files from target config if they are
		// copied from the current directory
		if (removeTargetOnFailure == TRUE) {
			_SCNetworkConfigurationRemoveConfigurationFiles(targetDirConfig);
		}
	}
done:
	if (currentDirConfig != NULL) {
		CFRelease(currentDirConfig);
	}
	if (currentSystemPath != NULL) {
		CFRelease(currentSystemPath);
	}
	if (sourceDirConfig != NULL) {
		CFRelease(sourceDirConfig);
	}
	if (targetDirConfig != NULL) {
		CFRelease(targetDirConfig);
	}
	return paths;
}

static Boolean
_SCNetworkConfigurationMigrateIsFilePresent(CFURLRef filePath)
{
	Boolean fileExists = false;
	char filePathStr[PATH_MAX];
	int statResult = 0;
	struct stat statStruct = {0, };

	if (filePath == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateIsFilePresent: filePath is NULL"));
		goto done;
	}

	if (CFURLGetFileSystemRepresentation(filePath, TRUE, (UInt8*) filePathStr, sizeof(filePathStr)) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateIsFilePresent: Couldn't get file system representation\n"));
		goto done;
	}

	statResult = stat(filePathStr, &statStruct);

	if (statResult == 0) {
		fileExists = TRUE;
	}
done:
	return fileExists;
}

static Boolean
__SCNetworkConfigurationMigrateConfigurationFilesPresent(CFURLRef baseURL, CFArrayRef* migrationPaths)
{
	Boolean configFilesPresent = FALSE;
	CFIndex count;
	CFURLRef filePath = NULL;
	CFURLRef interfaces;
	CFMutableArrayRef migrationPathsMutable = NULL;
	CFURLRef prefs;

	if (baseURL == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("__SCNetworkConfigurationMigrateConfigurationFilesPresent: baseURL is NULL"));
		goto done;
	}

	_SCNetworkConfigurationCopyMigrationPathsWithBaseURL(baseURL, &prefs, &interfaces);

	migrationPathsMutable = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	CFArrayInsertValueAtIndex(migrationPathsMutable, PREFERENCES_PLIST_INDEX, prefs);
	CFArrayInsertValueAtIndex(migrationPathsMutable, NETWORK_INTERFACES_PLIST_INDEX, interfaces);

	CFRelease(prefs);
	CFRelease(interfaces);

	*migrationPaths = migrationPathsMutable;

	if ((*migrationPaths == NULL) ||
	    ((count = CFArrayGetCount(*migrationPaths)) == 0)) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("__SCNetworkConfigurationMigrateConfigurationFilesPresent: migrationPath is NULL or number of elements in migrationPath array is 0"));
		goto done;
	}

	for (CFIndex idx = 0; idx < count; idx++) {
		filePath = CFArrayGetValueAtIndex(*migrationPaths, idx);
		if (_SCNetworkConfigurationMigrateIsFilePresent(filePath) ==  FALSE) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("__SCNetworkConfigurationMigrateConfigurationFilesPresent: File not present: %@"), filePath);
			goto done;
		}
	}

	configFilesPresent = TRUE;	// all necessary configuration files present
done:
	return configFilesPresent;
}


static CFMutableArrayRef
_SCNetworkInterfaceCopyInterfacesFilteredByBuiltinWithPreferences     (SCPreferencesRef   ni_prefs, Boolean isBuiltin)
{
	CFIndex count = 0;
	SCNetworkInterfaceRef interface;
	CFArrayRef interfaceList = NULL;
	CFMutableArrayRef resultInterfaceList = NULL;

	interfaceList = __SCNetworkInterfaceCopyStoredWithPreferences(ni_prefs);
	if (interfaceList == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkInterfaceCopyInterfacesFilteredByBuiltinWithPreferences: interfaceList is NULL"));
		goto done;
	}

	count = CFArrayGetCount(interfaceList);
	if (count > 0) {
		resultInterfaceList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	}

	for (CFIndex i = 0; i < count; i++) {
		interface = CFArrayGetValueAtIndex(interfaceList, i);

		if (_SCNetworkInterfaceIsBuiltin(interface) == isBuiltin) {
			CFArrayAppendValue(resultInterfaceList, interface);
		}
	}

done:
	if (interfaceList != NULL) {
		CFRelease(interfaceList);
	}
	return resultInterfaceList;
}

static CFMutableDictionaryRef
_SCNetworkInterfaceStorageCopyMaxUnitPerInterfaceType(SCPreferencesRef ni_prefs)
{
	CFNumberRef cfMaxUnit;
	CFIndex count = 0;
	CFArrayRef ifList = NULL;
	SCNetworkInterfaceRef interface;
	CFMutableDictionaryRef InterfaceTypeToMaxUnitMapping = NULL;
	CFNumberRef type;
	CFNumberRef unit;

	ifList = __SCNetworkInterfaceCopyStoredWithPreferences(ni_prefs);

	if (ifList == NULL) {
		SCLog(TRUE, LOG_ERR, CFSTR("_SCNetworkInterfaceStorageCopyMaxUnitPerInterfaceType: ifList is NULL"));
		return 0;
	}

	InterfaceTypeToMaxUnitMapping = CFDictionaryCreateMutable(NULL, 0,
								    &kCFTypeDictionaryKeyCallBacks,
								    &kCFTypeDictionaryValueCallBacks);
	count = CFArrayGetCount(ifList);

	for (CFIndex idx = 0; idx < count; idx++) {
		cfMaxUnit = NULL;
		interface = CFArrayGetValueAtIndex(ifList, idx);

		if (isA_SCNetworkInterface(interface) == NULL) {
			continue;
		}

		type  = _SCNetworkInterfaceGetIOInterfaceType(interface);

		if (isA_CFNumber(type) == NULL) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkInterfaceStorageCopyMaxUnitPerInterfaceType: type is NULL"));
			continue;
		}

		if (CFDictionaryContainsKey(InterfaceTypeToMaxUnitMapping, type) == FALSE) {
			int temp = 0;
			cfMaxUnit = CFNumberCreate(NULL, kCFNumberIntType, &temp);
			CFDictionaryAddValue(InterfaceTypeToMaxUnitMapping, type, cfMaxUnit);
			CFRelease(cfMaxUnit);
		}

		if (cfMaxUnit == NULL) {
			cfMaxUnit = CFDictionaryGetValue(InterfaceTypeToMaxUnitMapping, type);
		}

		unit = _SCNetworkInterfaceGetIOInterfaceUnit(interface);

		if (isA_CFNumber(unit) == NULL) {
			continue;
		}

		if (CFNumberCompare(unit, cfMaxUnit, NULL) == kCFCompareGreaterThan) {
			CFDictionarySetValue(InterfaceTypeToMaxUnitMapping, type, unit);
		}
	}
	if (ifList != NULL) {
		CFRelease(ifList);
	}
	return InterfaceTypeToMaxUnitMapping;
}

static CFMutableDictionaryRef
_SCNetworkConfigurationCopyBuiltinMapping (SCPreferencesRef sourcePrefs, SCPreferencesRef targetPrefs)
{
	CFMutableDictionaryRef builtinMapping = NULL;
	CFIndex sourceBuiltinInterfaceCount = 0;
	CFMutableArrayRef sourceBuiltinInterfaces = NULL;
	SCNetworkInterfaceRef sourceInterface;
	CFIndex targetBuiltinInterfaceCount = 0;
	CFMutableArrayRef targetBuiltinInterfaces = NULL;
	SCNetworkInterfaceRef targetInterface;

	sourceBuiltinInterfaces = _SCNetworkInterfaceCopyInterfacesFilteredByBuiltinWithPreferences(sourcePrefs, TRUE);
	if (isA_CFArray(sourceBuiltinInterfaces) == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCopyBuiltinMapping: sourceBuiltinInterfaces is NULL"));
		goto done;
	}
	sourceBuiltinInterfaceCount = CFArrayGetCount(sourceBuiltinInterfaces);

	targetBuiltinInterfaces = _SCNetworkInterfaceCopyInterfacesFilteredByBuiltinWithPreferences(targetPrefs, TRUE);
	if (isA_CFArray(targetBuiltinInterfaces) == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCopyBuiltinMapping: targetBuiltinInterfaces is NULL"));
		goto done;
	}
	targetBuiltinInterfaceCount = CFArrayGetCount(targetBuiltinInterfaces);

	// Builtin Mapping will try to map all source interfaces into target interfaces
	for (CFIndex idx = 0; idx < sourceBuiltinInterfaceCount; idx++) {
		sourceInterface = CFArrayGetValueAtIndex(sourceBuiltinInterfaces, idx);
		for (CFIndex idx2 = 0; idx2 < targetBuiltinInterfaceCount; idx2++) {
			targetInterface = CFArrayGetValueAtIndex(targetBuiltinInterfaces, idx2);

			if (_SCNetworkConfigurationIsInterfaceNamerMappable(sourceInterface, targetInterface, FALSE) == TRUE) {
				if (builtinMapping == NULL) {
					builtinMapping = CFDictionaryCreateMutable(NULL, 0,
										   &kCFTypeDictionaryKeyCallBacks,
										   &kCFTypeDictionaryValueCallBacks);
				}
				CFDictionaryAddValue(builtinMapping, sourceInterface, targetInterface);
				CFArrayRemoveValueAtIndex(targetBuiltinInterfaces, idx2);
				break;
			}
		}
		targetBuiltinInterfaceCount = CFArrayGetCount(targetBuiltinInterfaces);
	}

done:
	if (sourceBuiltinInterfaces != NULL) {
		CFRelease(sourceBuiltinInterfaces);
	}
	if (targetBuiltinInterfaces != NULL) {
		CFRelease(targetBuiltinInterfaces);
	}
	return builtinMapping;
}

static CFMutableDictionaryRef
_SCNetworkConfigurationCopyExternalInterfaceMapping (SCPreferencesRef sourcePref, SCPreferencesRef targetPrefs)
{
	CFNumberRef cfMaxTargetUnit = NULL;
	CFNumberRef currentInterfaceUnit = NULL;
	CFMutableDictionaryRef externalMapping = NULL;
	CFMutableDictionaryRef InterfaceTypeToMaxUnitMapping = NULL;
	int maxTargetUnit;
	int newTargetUnit;
	CFIndex sourceExternalInterfaceCount = 0;
	CFMutableArrayRef sourceExternalInterfaces = NULL;
	SCNetworkInterfaceRef sourceInterface = NULL;
	CFIndex targetExternalInterfaceCount = 0;
	CFMutableArrayRef targetExternalInterfaces = NULL;
	SCNetworkInterfaceRef targetInterface = NULL;
	CFNumberRef type;

	sourceExternalInterfaces = _SCNetworkInterfaceCopyInterfacesFilteredByBuiltinWithPreferences(sourcePref, FALSE);
	if (isA_CFArray(sourceExternalInterfaces) == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCopyExternalInterfaceMapping: sourceExternalInterfaces is NULL"));
		goto done;
	}
	sourceExternalInterfaceCount = CFArrayGetCount(sourceExternalInterfaces);

	if (sourceExternalInterfaceCount == 0) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCopyExternalInterfaceMapping: sourceExternalInterfaceCount is 0"));
		goto done;
	}

	targetExternalInterfaces = _SCNetworkInterfaceCopyInterfacesFilteredByBuiltinWithPreferences(targetPrefs, FALSE);
	if (isA_CFArray(targetExternalInterfaces) == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCopyExternalInterfaceMapping: targetExternalInterfaces is NULL"));
		goto done;
	}

	InterfaceTypeToMaxUnitMapping = _SCNetworkInterfaceStorageCopyMaxUnitPerInterfaceType(targetPrefs);
	externalMapping = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	// Map all external interfaces which exist in both source and target
	for (CFIndex idx = 0; idx < sourceExternalInterfaceCount; idx++) {
		sourceInterface = CFArrayGetValueAtIndex(sourceExternalInterfaces, idx);
		targetExternalInterfaceCount = CFArrayGetCount(targetExternalInterfaces);
		currentInterfaceUnit = NULL;

		for (CFIndex idx2 = 0; idx2 < targetExternalInterfaceCount; idx2++) {
			targetInterface = CFArrayGetValueAtIndex(targetExternalInterfaces, idx2);

			if (_SCNetworkConfigurationIsInterfaceNamerMappable(sourceInterface, targetInterface, TRUE) == TRUE) {
				CFDictionaryAddValue(externalMapping, sourceInterface, targetInterface);
				CFArrayRemoveValueAtIndex(targetExternalInterfaces, idx2);
				break;
			}
		}

		if (CFDictionaryContainsKey(externalMapping, sourceInterface) == FALSE) {
			// Create new mappings for external source interfaces which don't exist in the target
			type = _SCNetworkInterfaceGetIOInterfaceType(sourceInterface);

			cfMaxTargetUnit = CFDictionaryGetValue(InterfaceTypeToMaxUnitMapping, type);
			if (cfMaxTargetUnit != NULL) {
				CFNumberGetValue(cfMaxTargetUnit, kCFNumberIntType, &maxTargetUnit);
				newTargetUnit = maxTargetUnit + 1;
			}
			else {
				newTargetUnit = 0;
			}

			cfMaxTargetUnit = CFNumberCreate(NULL, kCFNumberIntType, &newTargetUnit);
			CFDictionarySetValue(InterfaceTypeToMaxUnitMapping, type, cfMaxTargetUnit);

			targetInterface = (SCNetworkInterfaceRef)__SCNetworkInterfaceCreateCopy(NULL, sourceInterface, NULL, NULL);

			SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCopyExternalInterfaceMapping: sourceInterface: %p, target Interface: %p"), sourceInterface, targetInterface);

			currentInterfaceUnit = _SCNetworkInterfaceGetIOInterfaceUnit(targetInterface);

			if ((isA_CFNumber(currentInterfaceUnit) == NULL) ||
			    (CFEqual(currentInterfaceUnit, cfMaxTargetUnit) == FALSE)) {
				// Update the interface unit
				__SCNetworkInterfaceSetIOInterfaceUnit(targetInterface, cfMaxTargetUnit);
			}

			CFDictionaryAddValue(externalMapping, sourceInterface, targetInterface);

			CFRelease(targetInterface);
			targetInterface = NULL;
			CFRelease(cfMaxTargetUnit);
			cfMaxTargetUnit = NULL;
		}
	}
done:
	if (sourceExternalInterfaces != NULL) {
		CFRelease(sourceExternalInterfaces);
	}
	if (targetExternalInterfaces != NULL) {
		CFRelease(targetExternalInterfaces);
	}
	if (InterfaceTypeToMaxUnitMapping != NULL) {
		CFRelease(InterfaceTypeToMaxUnitMapping);
	}
	return externalMapping;
}
static Boolean
__SCNetworkConfigurationInterfaceNameIsEquiv(CFStringRef interfaceName1, CFStringRef interfaceName2);

static Boolean
_SCNetworkConfigurationIsInterfaceNamerMappable(SCNetworkInterfaceRef interface1, SCNetworkInterfaceRef interface2, Boolean bypassActive)
{
	Boolean interface1IsBuiltin;
	CFStringRef interface1Prefix;
	CFStringRef interface1Type;
	CFStringRef interface1UserDefinedName;
	Boolean interface2IsBuiltin;
	CFStringRef interface2Prefix;
	CFStringRef interface2Type;
	CFStringRef interface2UserDefinedName;

	if (interface1 == interface2) {
		// No work needs to be done
		return TRUE;
	}
	interface1IsBuiltin = _SCNetworkInterfaceIsBuiltin(interface1);
	interface2IsBuiltin = _SCNetworkInterfaceIsBuiltin(interface2);

	interface1UserDefinedName = SCNetworkInterfaceGetLocalizedDisplayName(interface1);
	interface2UserDefinedName = SCNetworkInterfaceGetLocalizedDisplayName(interface2);

	interface1Type = SCNetworkInterfaceGetInterfaceType(interface1);
	interface2Type = SCNetworkInterfaceGetInterfaceType(interface2);

	interface1Prefix = _SCNetworkInterfaceGetIOInterfaceNamePrefix(interface1);
	interface2Prefix = _SCNetworkInterfaceGetIOInterfaceNamePrefix(interface2);

	// Check if have same builtin values.
	// Check if User Defined name matches
	// Check if SCNetwork Interface Type matches

	if (interface1IsBuiltin != interface2IsBuiltin) {
		return FALSE;
	}

	if (_SC_CFEqual(interface1Type, interface2Type) == FALSE) {
		return FALSE;
	}

	if (_SC_CFEqual(interface1Prefix, interface2Prefix) == FALSE) {
		return FALSE;
	}

	if (_SC_CFEqual(interface1UserDefinedName, interface2UserDefinedName) == FALSE) {
		// Checking if we have a mismatch because of the name Ethernet and Ethernet 1
		// Checking if we have a mismatch because of the name Airport and WiFi
		if ((interface1IsBuiltin == TRUE) &&
		    (interface2IsBuiltin == TRUE) &&
		    (__SCNetworkConfigurationInterfaceNameIsEquiv(interface1UserDefinedName, interface2UserDefinedName) == TRUE)) {
			    return TRUE;
		}
		return FALSE;
	}
	return TRUE;
}

static Boolean
__SCNetworkConfigurationInterfaceNameIsEquiv(CFStringRef interfaceName1, CFStringRef interfaceName2)
{
	CFStringRef interfaceArray[] = { CFSTR("iPhone"), CFSTR("iPad"), CFSTR("iPod"), CFSTR("AppleTV") };
	const int interfaceCount = sizeof(interfaceArray) / sizeof(CFStringRef);
	CFStringRef portSuffix = CFSTR(", Port 1");
	
	if ((isA_CFString(interfaceName1) != NULL) &&
	    (isA_CFString(interfaceName2) != NULL)) {
		if (CFEqual(interfaceName1, interfaceName2) == FALSE) {
			// Check if we are looking at the WiFi interface
			if ((CFEqual(interfaceName1, CFSTR("AirPort")) ||
			     (CFEqual(interfaceName1, CFSTR("Wi-Fi")))) &&
			    (CFEqual(interfaceName2, CFSTR("AirPort")) ||
			     (CFEqual(interfaceName2, CFSTR("Wi-Fi"))))) {
				return TRUE;
			}
			
			if (((CFEqual(interfaceName1, CFSTR("Ethernet"))) ||
			     (CFEqual(interfaceName1, CFSTR("Ethernet 1")))) &&
			    ((CFEqual(interfaceName2, CFSTR("Ethernet"))) ||
			     (CFEqual(interfaceName2, CFSTR("Ethernet 1"))))) {
				return TRUE;
			}
			
			if (((CFStringHasSuffix(interfaceName1, portSuffix) == TRUE) &&
			    (CFStringCompareWithOptions(interfaceName1, interfaceName2, CFRangeMake(0, (CFStringGetLength(interfaceName1) - CFStringGetLength(portSuffix))), 0) == kCFCompareEqualTo)) ||
			    ((CFStringHasSuffix(interfaceName2, portSuffix) == TRUE) &&
			     (CFStringCompareWithOptions(interfaceName2, interfaceName1, CFRangeMake(0, (CFStringGetLength(interfaceName2) - CFStringGetLength(portSuffix))), 0) == kCFCompareEqualTo))) {
				return TRUE;
			}
			
			for (CFIndex idx = 0; idx < interfaceCount; idx++) {
				CFStringRef tempInterfaceName = interfaceArray[idx];
				if ((CFEqual(interfaceName1, tempInterfaceName) == TRUE ||
					__SCNetworkInterfaceMatchesName(interfaceName1, tempInterfaceName) == TRUE) &&
				    (CFEqual(interfaceName2, tempInterfaceName) == TRUE  ||
					__SCNetworkInterfaceMatchesName(interfaceName2, tempInterfaceName) == TRUE)) {
					return TRUE;
				}
			}
		}
		else {
			return TRUE;
		}
	}
	
	return FALSE;
}

typedef struct {
	CFDictionaryRef interfaceMapping;
	CFMutableArrayRef interfacesMissingServices;
} SCNetworkConfigurationMissingServiceContext;

typedef struct {
	CFDictionaryRef bsdNameToBridgeServices;	// Mapping of BSD Name to SCBridgeInterfaceRef
	CFDictionaryRef bsdNameToBondServices;		// Mapping of BSD Name to SCBondInterfaceRef
	CFDictionaryRef bsdNameToVLANServices;		// Mapping of BSD Name to SCVLANInterfaceRef
	CFDictionaryRef interfaceMapping;
	Boolean* isValid;
	CFMutableArrayRef interfaceToBeRemoved;	// SCNetworkInterfaceRef. Services containing the interface will be removed
	CFMutableArrayRef interfaceToBeReplaced;// SCNetworkInterfaceRef. Services containing the interface will be replaced with default service
	Boolean repair;
} SCNetworkConfigurationValidityContext;

static void
_SCNetworkConfigurationValidateInterface (const void *key, const void *value, void *context)
{
	CFStringRef bsdName = (CFStringRef)key;
	SCNetworkConfigurationValidityContext *ctx = (SCNetworkConfigurationValidityContext*)context;
	CFDictionaryRef bsdNameToBridgeServices = ctx->bsdNameToBridgeServices;
	CFDictionaryRef bsdNameToBondServices = ctx->bsdNameToBondServices;
	CFDictionaryRef bsdNameToVLANServices = ctx->bsdNameToVLANServices;
	SCNetworkInterfaceRef interface = NULL;
	CFDictionaryRef interfaceMapping = ctx->interfaceMapping;
	CFStringRef interfaceUserDefinedName = NULL;
	Boolean repair = ctx->repair;
	SCNetworkInterfaceRef serviceInterface = (SCNetworkInterfaceRef)value;
	CFStringRef serviceInterfaceUserDefinedName = NULL;
	CFMutableArrayRef interfaceToBeRemoved = ctx->interfaceToBeRemoved;
	CFMutableArrayRef interfaceToBeReplaced = ctx->interfaceToBeReplaced;
	
	// No work needs to be done if we have already made determination that configuration somewhere is not valid,
	// or we don't intend to repair invalid configuration.
	if ((*ctx->isValid == FALSE) && (repair == FALSE)) {
		return;
	}

	// There is no interface present for the service
	interface = CFDictionaryGetValue(interfaceMapping, bsdName);
	if (interface == NULL) {
		if ((((bsdNameToBridgeServices != NULL) && (CFDictionaryContainsKey(bsdNameToBridgeServices, bsdName) == FALSE))) &&
		    (((bsdNameToBondServices != NULL) && (CFDictionaryContainsKey(bsdNameToBondServices, bsdName) == FALSE))) &&
		    (((bsdNameToVLANServices != NULL) && (CFDictionaryContainsKey(bsdNameToVLANServices, bsdName) == FALSE)))) {
			// Not a virtual interface
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationValidateInterface: There is no real interface with bsd name: %@ for service"), bsdName);
			
			if (repair == TRUE) {
				CFArrayAppendValue(interfaceToBeRemoved, serviceInterface);
			}
			*ctx->isValid = FALSE;
		}
		return;
	}

	// TODO: Need to compare between both SCNetworkInterfaceRefs
	interfaceUserDefinedName = __SCNetworkInterfaceGetUserDefinedName(interface);
	serviceInterfaceUserDefinedName = __SCNetworkInterfaceGetUserDefinedName(serviceInterface);

	if (__SCNetworkConfigurationInterfaceNameIsEquiv(interfaceUserDefinedName, serviceInterfaceUserDefinedName) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationValidateInterface: Interface user defined name: %@ doesn't match service interface user defined name: %@"), interfaceUserDefinedName, serviceInterfaceUserDefinedName);
		*ctx->isValid = FALSE;
		// Add service interface to the interfaceToBeReplaced list
		if (isA_CFArray(interfaceToBeReplaced) != NULL) {
			CFArrayAppendValue(interfaceToBeReplaced, interface);
		}
		if (isA_CFArray(interfaceToBeRemoved) != NULL) {
			CFArrayAppendValue(interfaceToBeRemoved, serviceInterface);
		}
		return;
	}
}

static void
_SCNetworkConfigurationCollectMissingService(const void *key, const void *value, void *context)
{
	CFStringRef bsdName = (CFStringRef)key;
	SCNetworkConfigurationMissingServiceContext *ctx = (SCNetworkConfigurationMissingServiceContext*)context;
	SCNetworkInterfaceRef interface = (SCNetworkInterfaceRef)value;
	CFMutableArrayRef interfacesMissingServices = ctx->interfacesMissingServices;
	CFDictionaryRef serviceInterfaceMapping = ctx->interfaceMapping;

	if ((isA_SCNetworkInterface(interface) == NULL) ||
	    (_SCNetworkInterfaceIsBuiltin(interface) == FALSE)) {
		return;
	}

	// Check if services have mapping for the BSD name of the interface
	if (CFDictionaryContainsKey(serviceInterfaceMapping, bsdName) == FALSE) {
		CFArrayAppendValue(interfacesMissingServices, interface); // Adding interface since the corresponding service seems to be missing
	}
}

static Boolean
_SCNetworkConfigurationCreateBuiltinInterfaceServices(SCPreferencesRef pref,
						      SCPreferencesRef ni_pref)
{
	SCNetworkConfigurationMissingServiceContext context;
	SCNetworkInterfaceRef interface = NULL;
	CFArrayRef interfaces = NULL;
	CFMutableArrayRef interfacesWithoutService = NULL;
	CFDictionaryRef mappingBSDNameToInterface = NULL;
	CFDictionaryRef mappingServiceBSDNameToInterface = NULL;
	CFIndex missingServiceCount = 0;
	Boolean success = FALSE;

	interfaces = __SCNetworkInterfaceCopyStoredWithPreferences(ni_pref);
	if (interfaces == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCreateBuiltinInterfaceServices: interfaces is NULL or not of the correct type"));
		goto done;
	}

	mappingBSDNameToInterface = __SCNetworkInterfaceCreateMappingUsingBSDName(interfaces);
	CFRelease(interfaces);
	if (isA_CFDictionary(mappingBSDNameToInterface) == NULL) {
		goto done;
	}

	interfaces = __SCNetworkServiceCopyAllInterfaces(pref);
	if (interfaces == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCreateBuiltinInterfaceServices: interfaces is NULL for configPref or not of the correct type"));
		goto done;
	}
	mappingServiceBSDNameToInterface = __SCNetworkInterfaceCreateMappingUsingBSDName(interfaces);
	CFRelease(interfaces);
	if (isA_CFDictionary(mappingServiceBSDNameToInterface) == NULL) {
		goto done;
	}

	interfacesWithoutService = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	context.interfaceMapping = mappingServiceBSDNameToInterface;
	context.interfacesMissingServices = interfacesWithoutService;

	CFDictionaryApplyFunction(mappingBSDNameToInterface, _SCNetworkConfigurationCollectMissingService, &context);
	missingServiceCount = CFArrayGetCount(interfacesWithoutService);

	success = TRUE;

	for (CFIndex idx = 0; idx < missingServiceCount; idx++) {
		interface = CFArrayGetValueAtIndex(interfacesWithoutService, idx);

		if (__SCNetworkServiceCreate(pref, interface, NULL) == FALSE) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCreateBuiltinInterfaceServices: Could not add service for interface: %@"), interface);
			success = FALSE;
		}
	}
done:
	if (mappingBSDNameToInterface != NULL) {
		CFRelease(mappingBSDNameToInterface);
	}
	if (mappingServiceBSDNameToInterface != NULL) {
		CFRelease(mappingServiceBSDNameToInterface);
	}
	if (interfacesWithoutService != NULL) {
		CFRelease(interfacesWithoutService);
	}

	return success;
}

static void
add_default_service(const void *value, void *context)
{
	SCNetworkSetRef currentSet = NULL;
	SCNetworkInterfaceRef interface = (SCNetworkInterfaceRef)value;
	SCPreferencesRef prefs = (SCPreferencesRef)context;
	SCNetworkServiceRef service;
	
	service = SCNetworkServiceCreate(prefs, interface);
	
	if (service == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("add_default_service: Could not create new service"));
		goto done;
	}
	
	if (SCNetworkServiceEstablishDefaultConfiguration(service) == FALSE) {
		SCNetworkServiceRemove(service);
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("add_default_service: SCNetworkServiceEstablishDefaultConfiguration failed"));
		goto done;
	}
	
	// Add Service to current set
	currentSet = SCNetworkSetCopyCurrent(prefs);
	if (currentSet == NULL) {
		SCNetworkServiceRemove(service);
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("add_default_service: Could not find current set"));
		goto done;
	}
	
	if (SCNetworkSetAddService(currentSet, service) == FALSE) {
		SCNetworkServiceRemove(service);
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("add_default_service: Could not add service to current set"));
		goto done;
	}
done:
	if (service != NULL) {
		CFRelease(service);
	}
	if (currentSet != NULL) {
		CFRelease(currentSet);
	}
}

static void
remove_service(const void *value, void *context)
{
	SCNetworkInterfaceRef interface;
	SCNetworkServiceRef service = (SCNetworkServiceRef)value;
	CFArrayRef toBeRemoved = (CFArrayRef)context;
	
	interface = SCNetworkServiceGetInterface(service);
	
	if (CFArrayContainsValue(toBeRemoved, CFRangeMake(0, CFArrayGetCount(toBeRemoved)), interface)) {
		SCNetworkServiceRemove(service);
	}
}

static void
_SCNetworkConfigurationSaveOldConfiguration(SCPreferencesRef prefs)
{
	CFAbsoluteTime absoluteTime;
	CFCalendarRef currentCalendar;
	int day;
	int hour;
	CFStringRef keyList[] = {
			kSCPrefCurrentSet,
			MODEL,
			kSCPrefNetworkServices,
			kSCPrefSets,
			kSCPrefSystem,
			kSCPrefVirtualNetworkInterfaces
	};
	CFIndex keyListCount;
	int minute;
	int month;
	int second;
	int year;
	
	currentCalendar = CFCalendarCopyCurrent();
	absoluteTime = CFAbsoluteTimeGetCurrent();
	
	if (CFCalendarDecomposeAbsoluteTime(currentCalendar, absoluteTime, "yMdHms",
					    &year, &month, &day, &hour, &minute, &second) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationSaveOldConfiguration: Cannot decompose absolute time"));
	}
	keyListCount = (CFIndex)sizeof(keyList)/sizeof(CFStringRef);
	
	for (CFIndex idx = 0; idx < keyListCount; idx++) {
		CFStringRef newKey;
		CFTypeRef value = SCPreferencesGetValue(prefs, keyList[idx]);
		
		if (value != NULL) {
			newKey = CFStringCreateWithFormat(NULL, NULL,
							  CFSTR("%d-%d-%d %d:%d:%d : %@"),
							  year, month, day, hour,
							  minute, second, keyList[idx]);
			SCPreferencesSetValue(prefs, newKey, value);
			CFRelease(newKey);
		}
	}
	CFRelease(currentCalendar);
}

static Boolean
_SCNetworkConfigurationRepairUsingPreferences(SCPreferencesRef prefs,
					      CFArrayRef interfaceToBeRemoved,
					      CFArrayRef interfaceToBeReplaced)
{
	CFIndex removeCount;
	CFIndex replaceCount;
	CFArrayRef serviceList;
	
	removeCount = CFArrayGetCount(interfaceToBeRemoved);
	replaceCount = CFArrayGetCount(interfaceToBeReplaced);
	if (removeCount == 0 &&
	    replaceCount == 0) {
		// We don't have any information to repair
		return FALSE;
	}
	// Backup current preferences before making changes
	_SCNetworkConfigurationSaveOldConfiguration(prefs);
	
	serviceList = SCNetworkServiceCopyAll(prefs);
	CFArrayApplyFunction(serviceList, CFRangeMake(0, CFArrayGetCount(serviceList)), remove_service, (void*)interfaceToBeRemoved);
	
	CFArrayApplyFunction(interfaceToBeReplaced, CFRangeMake(0, replaceCount), add_default_service, (void*)prefs);
	CFRelease(serviceList);
	return TRUE;
}

static void
validate_bridge(const void *value, void *context)
{
	SCBridgeInterfaceRef bridge = (SCBridgeInterfaceRef) value;
	CFArrayRef memberInterfaces = SCBridgeInterfaceGetMemberInterfaces(bridge);
	CFMutableArrayRef memberInterfacesMutable = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	SCPreferencesRef ni_prefs = (SCPreferencesRef)context;
	
	for (CFIndex idx = 0; idx < CFArrayGetCount(memberInterfaces); idx++) {
		CFStringRef bsdName;
		SCNetworkInterfaceRef interface = (SCNetworkInterfaceRef)CFArrayGetValueAtIndex(memberInterfaces, idx);
		SCNetworkInterfaceRef memberInterface;
		
		bsdName = SCNetworkInterfaceGetBSDName(interface);
		if (bsdName == NULL) {
			continue;
		}
		
		// Check if member interface is present
		memberInterface = __SCNetworkInterfaceCreateWithNIPreferencesUsingBSDName(NULL, ni_prefs, bsdName);
		if (memberInterface != NULL) {
			CFArrayAppendValue(memberInterfacesMutable, memberInterface);
			CFRelease(memberInterface);
		}
	}
	
	if (CFArrayGetCount(memberInterfacesMutable) == 0) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("validate_bridge: Removing invalid bridge configuration: %@"), bridge);
		SCBridgeInterfaceRemove(bridge);
	}
	else {
		SCBridgeInterfaceSetMemberInterfaces(bridge, memberInterfacesMutable);
	}
	CFRelease(memberInterfacesMutable);
}
#if	!TARGET_OS_IPHONE
static void
validate_bond(const void *value, void *context)
{
	SCBondInterfaceRef bond = (SCBondInterfaceRef)value;
	CFArrayRef memberInterfaces = SCBondInterfaceGetMemberInterfaces(bond);
	CFMutableArrayRef memberInterfacesMutable = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	SCPreferencesRef ni_prefs = (SCPreferencesRef)context;
	
	for (CFIndex idx = 0; idx < CFArrayGetCount(memberInterfaces); idx++) {
		CFStringRef bsdName;
		SCNetworkInterfaceRef interface = (SCNetworkInterfaceRef)CFArrayGetValueAtIndex(memberInterfaces, idx);
		SCNetworkInterfaceRef memberInterface;
		
		bsdName = SCNetworkInterfaceGetBSDName(interface);
		if (bsdName == NULL) {
			continue;
		}
		
		// Check if member interface is present
		memberInterface = __SCNetworkInterfaceCreateWithNIPreferencesUsingBSDName(NULL, ni_prefs, bsdName);
		if (memberInterface != NULL) {
			CFArrayAppendValue(memberInterfacesMutable, memberInterface);
			CFRelease(memberInterface);
		}
	}
	
	if (CFArrayGetCount(memberInterfacesMutable) == 0) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("validate_bond: Removing invalid bond configuration: %@"), bond);
		SCBondInterfaceRemove(bond);
	}
	else {
		SCBondInterfaceSetMemberInterfaces(bond, memberInterfacesMutable);
	}
	CFRelease(memberInterfacesMutable);
}
#endif

static void
validate_vlan(const void *value, void *context)
{
	CFStringRef bsdName;
	SCNetworkInterfaceRef interface;
	Boolean isValid = TRUE;
	SCPreferencesRef ni_prefs = (SCPreferencesRef)context;
	SCNetworkInterfaceRef physicalInterface;
	SCVLANInterfaceRef vlan = (SCVLANInterfaceRef)value;
	
	physicalInterface = SCVLANInterfaceGetPhysicalInterface(vlan);
	bsdName = SCNetworkInterfaceGetBSDName(physicalInterface);
	
	if (bsdName == NULL) {
		isValid = FALSE;
		goto done;
	}
	
	// Check if the physical interface is present
	interface = __SCNetworkInterfaceCreateWithNIPreferencesUsingBSDName(NULL, ni_prefs, bsdName);
	if (interface == NULL) {
		isValid = FALSE;
		goto done;
	}
	CFRelease(interface);
	
done:
	if (isValid == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("validate_vlan: Removing invalid VLAN configuration: %@"), vlan);
		SCVLANInterfaceRemove(vlan);
	}
}

static Boolean
_SCNetworkConfigurationCheckValidityUsingPreferences(SCPreferencesRef prefs,
						     SCPreferencesRef ni_prefs,
						     CFDictionaryRef options)
{
	CFArrayRef allServices = NULL;
	CFArrayRef allSets = NULL;
	CFDictionaryRef bsdNameToBridgeServices = NULL;
	CFDictionaryRef bsdNameToBondServices = NULL;
	CFDictionaryRef bsdNameToVLANServices = NULL;
	SCNetworkConfigurationValidityContext context;
	CFArrayRef interfaces = NULL;
	Boolean isValid = TRUE;
	CFDictionaryRef mappingBSDNameToInterface = NULL;
	CFDictionaryRef mappingServiceBSDNameToInterface = NULL;
	Boolean repairConfiguration = FALSE;
	CFArrayRef setServiceOrder = NULL;
	CFArrayRef setServices = NULL;
	CFMutableArrayRef interfaceToBeRemoved = NULL;
	CFMutableArrayRef interfaceToBeReplaced = NULL;
	
	
	if  ((isA_CFDictionary(options) != NULL)) {
		CFBooleanRef repair = CFDictionaryGetValue(options, kSCNetworkConfigurationRepair);
		if (isA_CFBoolean(repair) != NULL) {
			repairConfiguration = CFBooleanGetValue(repair);
		}
	}

	/*
	 Check the validity by:
	 1) Comparing if the interfaces names mentioned in NetworkInterfaces.plist and preferences.plist match
	 Use the functions
	 CFDictionaryRef
	 __SCNetworkInterfaceCreateMappingUsingBSDName(SCPreferencesRef prefs);
	 */
	interfaces = __SCNetworkInterfaceCopyStoredWithPreferences(ni_prefs);
	if (isA_CFArray(interfaces) == NULL) {
		isValid = FALSE;
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCheckValidityUsingPreferences: interfaces is NULL or not of the correct type"));
		goto done;
	}
	mappingBSDNameToInterface = __SCNetworkInterfaceCreateMappingUsingBSDName(interfaces);
	CFRelease(interfaces);
	if (isA_CFDictionary(mappingBSDNameToInterface) == NULL) {
		isValid = FALSE;
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCheckValidityUsingPreferences: mappingBSDNameToInterface is NULL"));
		goto done;
	}

	interfaces = __SCNetworkServiceCopyAllInterfaces(prefs);
	if (isA_CFArray(interfaces) == NULL) {
		isValid = FALSE;
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCheckValidityUsingPreferences: interfaces is NULL for configPref or not of the correct type"));
		goto done;
	}
	mappingServiceBSDNameToInterface = __SCNetworkInterfaceCreateMappingUsingBSDName(interfaces);
	CFRelease(interfaces);
	if (isA_CFDictionary(mappingServiceBSDNameToInterface) == NULL) {
		isValid = FALSE;
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCheckValidityUsingPreferences: mappingServiceBSDNameToInterface is NULL"));
		goto done;
	}

	if (repairConfiguration) {
		interfaceToBeRemoved = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		interfaceToBeReplaced = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
#if	!TARGET_OS_IPHONE
		bsdNameToBridgeServices = _SCNetworkMigrationCopyMappingBSDNameToBridgeServices(prefs);
		bsdNameToBondServices = _SCNetworkMigrationCopyMappingBSDNameToBondServices(prefs);
		bsdNameToVLANServices = _SCNetworkMigrationCopyMappingBSDNameToVLANServices(prefs);
#endif
	}
	context.interfaceMapping = mappingBSDNameToInterface;
	context.isValid = &isValid;
	context.interfaceToBeRemoved = interfaceToBeRemoved;
	context.interfaceToBeReplaced = interfaceToBeReplaced;
	context.bsdNameToBridgeServices = bsdNameToBridgeServices;
	context.bsdNameToBondServices = bsdNameToBondServices;
	context.bsdNameToVLANServices = bsdNameToVLANServices;
	context.repair = repairConfiguration;

	CFDictionaryApplyFunction(mappingServiceBSDNameToInterface, _SCNetworkConfigurationValidateInterface, &context);

	if (isValid == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCheckValidityUsingPreferences: Found mismatch between interface names in NetworkInterfaces.plist and preferences.plist"));
		if (repairConfiguration) {
			isValid = _SCNetworkConfigurationRepairUsingPreferences(prefs, interfaceToBeRemoved, interfaceToBeReplaced);
			if (isValid == FALSE) {
				goto done;
			}
			// Save the changes if repair fixed an invalid configuration
			if (SCPreferencesCommitChanges(prefs) == FALSE) {
				SCLog(_sc_debug, LOG_NOTICE, CFSTR("Failed to commit changes from the repaired configuration"));
			}
		}
		else {
			goto done;
		}
	}
	/*

	 2) Check if all the network services mentioned in the SCNetworkSet are actually present in the SCNetworkService array
	 */

	allServices = SCNetworkServiceCopyAll(prefs);
	if (isA_CFArray(allServices) == NULL) {
		isValid = FALSE;
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCheckValidityUsingPreferences: allServices is NULL"));
		goto done;
	}

	allSets = SCNetworkSetCopyAll(prefs);
	if (isA_CFArray(allSets) == NULL) {
		isValid = FALSE;
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCheckValidityUsingPreferences: allSets is NULL"));
		goto done;
	}

	for (CFIndex idx = 0; ((idx < CFArrayGetCount(allSets)) && isValid); idx++) {
		SCNetworkSetRef set = CFArrayGetValueAtIndex(allSets, idx);

		if (isA_SCNetworkSet(set) == NULL) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCheckValidityUsingPreferences: set is NULL"));
			continue;
		}
		setServices = SCNetworkSetCopyServices(set);
		if (setServices == NULL) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCheckValidityUsingPreferences: setServices is NULL"));
			continue;
		}
		for (CFIndex idx2 = 0; idx2 < CFArrayGetCount(setServices); idx2++) {
			SCNetworkServiceRef service = CFArrayGetValueAtIndex(setServices, idx2);

			if (CFArrayContainsValue(allServices, CFRangeMake(0, CFArrayGetCount(allServices)), service) == FALSE) {
				isValid = FALSE;
				SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCheckValidityUsingPreferences: All network services in the network set are not present in SCNetworkService array"));
				break;
			}
		}
		if (isValid == FALSE) {
			break;
		}

		/*
		 3) Check if service IDs in service order do exist in the SET
		 */
		setServiceOrder = SCNetworkSetGetServiceOrder(set);
		if (setServiceOrder != NULL) {
			for (CFIndex idx2 = 0; idx2 < CFArrayGetCount(setServiceOrder); idx2++) {
				SCNetworkServiceRef service = CFArrayGetValueAtIndex(setServiceOrder, idx2);
				if ((CFArrayContainsValue(setServiceOrder, CFRangeMake(0, CFArrayGetCount(setServiceOrder)), service) == FALSE) &&
				    (CFArrayContainsValue(allServices, CFRangeMake(0, CFArrayGetCount(allServices)), service) == FALSE)) {
					SCLog(_sc_debug, LOG_NOTICE, CFSTR("Service: %@ is not present in the service order for set %@"), service, set);
					break;
				}
			}
		}
		if (setServices != NULL) {
			CFRelease(setServices);
			setServices = NULL;
		}
	}
	
	/*
	 4) Check if the virtual network interfaces have valid member interfaces
	 */
	CFArrayRef bridges = SCBridgeInterfaceCopyAll(prefs);
	if (bridges != NULL) {
		CFArrayApplyFunction(bridges, CFRangeMake(0, CFArrayGetCount(bridges)), validate_bridge, (void*)ni_prefs);
		CFRelease(bridges);
	}
#if	!TARGET_OS_IPHONE
	CFArrayRef bonds = SCBondInterfaceCopyAll(prefs);
	if (bonds != NULL) {
		CFArrayApplyFunction(bonds, CFRangeMake(0, CFArrayGetCount(bonds)), validate_bond, (void*)ni_prefs);
		CFRelease(bonds);
	}
#endif
	CFArrayRef vlans = SCVLANInterfaceCopyAll(prefs);
	if (vlans != NULL) {
		CFArrayApplyFunction(vlans, CFRangeMake(0, CFArrayGetCount(vlans)), validate_vlan, (void*)ni_prefs);
		CFRelease(vlans);
	}


done:
	if (mappingBSDNameToInterface != NULL) {
		CFRelease(mappingBSDNameToInterface);
	}
	if (mappingServiceBSDNameToInterface != NULL) {
		CFRelease(mappingServiceBSDNameToInterface);
	}
	if (allServices != NULL) {
		CFRelease(allServices);
	}
	if (allSets != NULL) {
		CFRelease(allSets);
	}
	if (bsdNameToBridgeServices != NULL) {
		CFRelease(bsdNameToBridgeServices);
	}
	if (bsdNameToBondServices != NULL) {
		CFRelease(bsdNameToBondServices);
	}
	if (bsdNameToVLANServices != NULL) {
		CFRelease(bsdNameToVLANServices);
	}
	if (setServices != NULL) {
		CFRelease(setServices);
	}
	if (interfaceToBeRemoved != NULL) {
		CFRelease(interfaceToBeRemoved);
	}
	if (interfaceToBeReplaced != NULL) {
		CFRelease(interfaceToBeReplaced);
	}
	return isValid;
}

Boolean
_SCNetworkConfigurationCheckValidity(CFURLRef configDir, CFDictionaryRef options)
{
	CFURLRef baseURL = NULL;
	CFURLRef configNetworkInterfaceFile = NULL;
	CFStringRef configNetworkInterfaceFileString = NULL;
	SCPreferencesRef configNetworkInterfacePref = NULL;
	SCPreferencesRef configPref = NULL;
	CFURLRef configPreferenceFile = NULL;
	CFStringRef configPreferencesFileString = NULL;
	CFArrayRef configurationFiles = NULL;
	Boolean isValid = FALSE;
	char networkInterfaceStr[PATH_MAX];
	char prefsStr[PATH_MAX];
	
	if (configDir == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCheckValidity: Migration files not found in directory: %@"), ((configDir == NULL) ? CFSTR("NULL") : CFURLGetString(configDir)));
		goto done;
	}
	baseURL = CFURLCreateWithFileSystemPathRelativeToBase(NULL, PREFS_DEFAULT_DIR_RELATIVE,
							      kCFURLPOSIXPathStyle, TRUE, configDir);
	
	configPreferenceFile = CFURLCreateFromFileSystemRepresentationRelativeToBase(NULL, (const UInt8*)PREFS_DEFAULT_CONFIG_PLIST, sizeof(PREFS_DEFAULT_CONFIG_PLIST), FALSE, baseURL);
	configNetworkInterfaceFile = CFURLCreateFromFileSystemRepresentationRelativeToBase(NULL, (const UInt8*)NETWORK_INTERFACES_PREFS_PLIST, sizeof(NETWORK_INTERFACES_PREFS_PLIST), FALSE, baseURL);
	
	if (CFURLGetFileSystemRepresentation(configPreferenceFile, TRUE, (UInt8*)prefsStr, sizeof(prefsStr)) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCheckValidity: Could not extract preferences information"));
		goto done;
	}
	if (CFURLGetFileSystemRepresentation(configNetworkInterfaceFile, TRUE, (UInt8*)networkInterfaceStr, sizeof(networkInterfaceStr)) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCheckValidity: Could not extract network interface information"));
		goto done;
	}
	
	configPreferencesFileString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%s"), prefsStr);
	configNetworkInterfaceFileString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%s"), networkInterfaceStr);

	configPref = SCPreferencesCreate(NULL,
					 PLUGIN_ID,
					 configPreferencesFileString);

	configNetworkInterfacePref = SCPreferencesCreate(NULL,
							 PLUGIN_ID,
							 configNetworkInterfaceFileString);
	if ((configPref == NULL) || (configNetworkInterfacePref == NULL)) {
		goto done;
	}

	// This function compares preferences.plist and NetworkInterfaces.plist and verifies if the values are correct
	// Checking interface mismatch for validity
	isValid = _SCNetworkConfigurationCheckValidityUsingPreferences(configPref, configNetworkInterfacePref, options);

done:
	if (baseURL != NULL) {
		CFRelease(baseURL);
	}
	if (configurationFiles != NULL) {
		CFRelease(configurationFiles);
	}
	if (configPreferencesFileString != NULL) {
		CFRelease(configPreferencesFileString);
	}
	if (configNetworkInterfaceFileString != NULL) {
		CFRelease(configNetworkInterfaceFileString);
	}
	if (configPref != NULL) {
		CFRelease(configPref);
	}
	if (configNetworkInterfacePref != NULL) {
		CFRelease(configNetworkInterfacePref);
	}
	if (configPreferenceFile != NULL) {
		CFRelease(configPreferenceFile);
	}
	if (configNetworkInterfaceFile != NULL) {
		CFRelease(configNetworkInterfaceFile);
	}
	return isValid;
}


typedef struct {
	CFMutableArrayRef externalInterfaceList;
	CFMutableArrayRef networkInterfaceList;
} SCExternalMappingContext;

static void
_SCNetworkConfigurationCollectInterfaceStorageEntity(const void *key, const void *value, void *context)
{
	SCExternalMappingContext* ctx = context;
	CFDictionaryRef interface_entity = NULL;
	SCNetworkInterfaceRef targetInterface = (SCNetworkInterfaceRef)value;

	if (CFArrayContainsValue(ctx->externalInterfaceList, CFRangeMake(0, CFArrayGetCount(ctx->externalInterfaceList)), targetInterface) == TRUE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationCollectInterfaceStorageEntity: Target Interface %@ already exists, thus do no add it to NetworkInterfaces.plist"), targetInterface);
		return; // If the target interface already exists then do not add it to NetworkInterfaces.plist
	}
	interface_entity = __SCNetworkInterfaceCopyStorageEntity(targetInterface);

	if (interface_entity != NULL) {
		CFArrayAppendValue(ctx->networkInterfaceList, interface_entity);
		CFRelease(interface_entity);
	}
}

static CFArrayRef   // CFDictionaryRef
_SCNetworkMigrationCreateNetworkInterfaceArray(SCPreferencesRef ni_prefs, CFDictionaryRef externalMapping)
{
	SCExternalMappingContext context;
	CFIndex count = 0;
	CFMutableArrayRef externalInterfaceList = NULL;
	CFArrayRef if_list = NULL;
	CFDictionaryRef interface_entity = NULL;
	CFMutableArrayRef networkInterfaceList = NULL;

	if (ni_prefs == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationCreateNetworkInterfaceArray: ni_prefs are NULL"));
		return NULL;
	}

	if_list = SCPreferencesGetValue(ni_prefs, INTERFACES);

	if ((isA_CFArray(if_list) == NULL) ||
	    ((count = CFArrayGetCount(if_list)) == 0)) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationCreateNetworkInterfaceArray: if_list is NULL or interface count is 0"));
		return NULL;
	}

	networkInterfaceList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	// Keep the same builtin and external interfaces
	for (CFIndex idx = 0; idx < count; idx++) {
		interface_entity = CFArrayGetValueAtIndex(if_list, idx);
		if (isA_CFDictionary(interface_entity) == NULL) {
			continue;
		}
		CFArrayAppendValue(networkInterfaceList, interface_entity);
	}

	if (isA_CFDictionary(externalMapping) == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationCreateNetworkInterfaceArray: externalMapping is NULL or not of the correct type"));
		goto done;
	}
	// Add any new external interfaces found
	externalInterfaceList = _SCNetworkInterfaceCopyInterfacesFilteredByBuiltinWithPreferences(ni_prefs, FALSE);
	context.externalInterfaceList = externalInterfaceList;
	context.networkInterfaceList = networkInterfaceList;

	CFDictionaryApplyFunction(externalMapping, _SCNetworkConfigurationCollectInterfaceStorageEntity, &context);

done:
	if (externalInterfaceList != NULL) {
		CFRelease(externalInterfaceList);
	}
	return networkInterfaceList;
}

static void
SCNetworkMigrationMapSourceToTargetName(const void *key, const void *value, void *context)
{
	SCNetworkInterfaceRef interfaceKey = (SCNetworkInterfaceRef)key;
	SCNetworkInterfaceRef interfaceValue = (SCNetworkInterfaceRef)value;
	CFMutableDictionaryRef mapping = (CFMutableDictionaryRef)context;
	CFStringRef sourceBSDName = NULL;
	CFStringRef targetBSDName = NULL;

	sourceBSDName = SCNetworkInterfaceGetBSDName(interfaceKey);
	if (isA_CFString(sourceBSDName) == NULL) {
		return;
	}

	targetBSDName = SCNetworkInterfaceGetBSDName(interfaceValue);
	if (isA_CFString(targetBSDName) == NULL) {
		return;
	}

	if (CFDictionaryContainsKey(mapping, sourceBSDName) == FALSE) {
		CFDictionaryAddValue(mapping, sourceBSDName, targetBSDName);
	}
	return;
}

static CFDictionaryRef
_SCNetworkMigrationCreateBSDNameMapping(CFDictionaryRef internalMapping, CFDictionaryRef externalMapping)
{
	CFMutableDictionaryRef bsdNameMapping = CFDictionaryCreateMutable(NULL, 0,
									  &kCFTypeDictionaryKeyCallBacks,
									  &kCFTypeDictionaryValueCallBacks);

	if ((internalMapping == NULL) && externalMapping == NULL) {
		goto done;
	}
	
	if (internalMapping != NULL) {
		CFDictionaryApplyFunction(internalMapping, SCNetworkMigrationMapSourceToTargetName, bsdNameMapping);
	}

	if (externalMapping != NULL) {
		CFDictionaryApplyFunction(externalMapping, SCNetworkMigrationMapSourceToTargetName, bsdNameMapping);
	}
	
done:
	return bsdNameMapping;
}

typedef struct {
	CFMutableArrayRef mutableServiceArray;
	SCPreferencesRef prefs;
} SCNetworkServiceArrayCopyContext;

static CFDictionaryRef
_SCNetworkMigrationCreateServiceSetMapping(SCPreferencesRef prefs)
{
	CFMutableDictionaryRef serviceSetMapping = CFDictionaryCreateMutable(NULL, 0,
									     &kCFTypeDictionaryKeyCallBacks,
									     &kCFTypeDictionaryValueCallBacks);
	SCNetworkServiceRef service = NULL;
	CFArrayRef services = NULL;
	CFMutableArrayRef setList = NULL;
	CFArrayRef sets = NULL;
	
	services = SCNetworkServiceCopyAll(prefs);
	if (services == NULL) {
		goto done;
	}
	for (CFIndex idx = 0; idx < CFArrayGetCount(services); idx++) {
		service = CFArrayGetValueAtIndex(services, idx);
		
		if (CFDictionaryContainsKey(serviceSetMapping, service) == FALSE) {
			setList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
			CFDictionaryAddValue(serviceSetMapping, service, setList);
			CFRelease(setList);
		}
	}
	CFRelease(services);
	
	sets = SCNetworkSetCopyAll(prefs);
	if (sets == NULL) {
		goto done;
	}
	
	for (CFIndex idx = 0; idx < CFArrayGetCount(sets); idx++) {
		SCNetworkSetRef set = CFArrayGetValueAtIndex(sets, idx);
		services = SCNetworkSetCopyServices(set);
		
		for (CFIndex idx2 = 0; idx2 < CFArrayGetCount(services); idx2++) {
			service = CFArrayGetValueAtIndex(services, idx2);
			setList = (CFMutableArrayRef)CFDictionaryGetValue(serviceSetMapping, service);
			if (setList != NULL) {
				CFArrayAppendValue(setList, set);
			}
		}
		CFRelease(services);
	}
	
done:
	if (sets != NULL) {
		CFRelease(sets);
	}
	return serviceSetMapping;
}

static CFDictionaryRef
_SCNetworkMigrationCreateSetMapping(SCPreferencesRef sourcePrefs,
				    SCPreferencesRef targetPrefs)
{
	SCNetworkSetRef currentSourceSet = NULL;
	CFMutableDictionaryRef setMapping = NULL;
	CFStringRef setName;
	CFArrayRef sourceSets = NULL;
	CFArrayRef targetSets = NULL;
	CFMutableArrayRef targetSetsMutable = NULL;
	
	sourceSets = SCNetworkSetCopyAll(sourcePrefs);
	targetSets = SCNetworkSetCopyAll(targetPrefs);
	
	if (sourceSets == NULL ||
	    targetSets == NULL) {
		goto done;
	}
	targetSetsMutable = CFArrayCreateMutableCopy(NULL, 0, targetSets);
	
	setMapping = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	
	currentSourceSet = SCNetworkSetCopyCurrent(sourcePrefs);
	
	for (CFIndex idx = 0; idx < CFArrayGetCount(sourceSets); idx++) {
		SCNetworkSetRef sourceSet = CFArrayGetValueAtIndex(sourceSets, idx);
		CFIndex targetCount = CFArrayGetCount(targetSetsMutable);
		SCNetworkSetRef targetSet;
		
		setName = SCNetworkSetGetName(sourceSet);
		if (targetCount > 0) {
			targetSet = CFArrayGetValueAtIndex(targetSetsMutable, 0);
			CFRetain(targetSet);
			CFArrayRemoveValueAtIndex(targetSetsMutable, 0);
		}
		else {
			targetSet = SCNetworkSetCreate(targetPrefs);
		}
		
		SCNetworkSetSetName(targetSet, setName);
		CFDictionaryAddValue(setMapping, sourceSet, targetSet);
		
		if (CFEqual(sourceSet, currentSourceSet) == TRUE) {
			SCNetworkSetSetCurrent(targetSet);
		}
		CFRelease(targetSet);
	}
	
done:
	if (sourceSets != NULL) {
		CFRelease(sourceSets);
	}
	if (targetSets != NULL) {
		CFRelease(targetSets);
	}
	if (targetSetsMutable != NULL) {
		CFRelease(targetSetsMutable);
	}
	if (currentSourceSet != NULL) {
		CFRelease(currentSourceSet);
	}
	return setMapping;
}

// This function finds the mapping between source and target preferences (SCNetworkServicesRef -> SCNetworkServicesRef)
// If there is no mapping found between source and target preferences, then the CFBooleanRef value indicating no value is found is stored (SCNetworkServicesRef -> kCFBooleanFalse)
static CFDictionaryRef
_SCNetworkMigrationCreateServiceMappingUsingBSDMapping(SCPreferencesRef sourcePrefs,
						       SCPreferencesRef targetPrefs,
						       CFDictionaryRef bsdNameMapping)
{
	CFStringRef bsdNameMapTarget = NULL;
	CFMutableDictionaryRef serviceMapping = NULL;                               // Mapping of services between source and target configurations
	CFStringRef sourceBSDName = NULL;
	CFIndex sourceCount = 0;
	SCNetworkInterfaceRef sourceInterface = NULL;
	CFStringRef sourceInterfaceSubType = NULL;		// Check interface type and subtype to be able to transfer VPN
	CFStringRef sourceInterfaceType = NULL;
	CFArrayRef sourceSCNetworkServices = NULL;
	CFMutableArrayRef sourceSCNetworkServicesMutable = NULL;                    // Source SCNetworkServiceRef mutable array
	SCNetworkServiceRef sourceService = NULL;
	CFStringRef targetBSDName = NULL;
	CFIndex targetCount = 0;                                   // Count of Source and Target Services
	SCNetworkInterfaceRef targetInterface = NULL;
	CFStringRef targetInterfaceSubType = NULL;		// services during migration
	CFStringRef targetInterfaceType = NULL;
	CFArrayRef targetSCNetworkServices = NULL;
	CFMutableArrayRef targetSCNetworkServicesMutable = NULL;                    // Target SCNetworkServiceRef mutable array
	SCNetworkServiceRef targetService = NULL;

	// We need BSD Mapping to successfully create service mapping
	if (bsdNameMapping == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationCreateServiceMappingUsingBSDMapping: BSD Name Mapping is NULL"));
		goto done;
	}
	sourceSCNetworkServices = SCNetworkServiceCopyAll(sourcePrefs);
	if (isA_CFArray(sourceSCNetworkServices) == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationCreateServiceMappingUsingBSDMapping: sourceSCNetworkServices is NULL or not of the correct CFType"));
		goto done;
	}
	targetSCNetworkServices = SCNetworkServiceCopyAll(targetPrefs);
	if (isA_CFArray(targetSCNetworkServices) == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationCreateServiceMappingUsingBSDMapping: targetSCNetworkServices is NULL or not of the correct CFType"));
		goto done;
	}

	sourceCount = CFArrayGetCount(sourceSCNetworkServices);

	sourceSCNetworkServicesMutable = CFArrayCreateMutableCopy(NULL, 0, sourceSCNetworkServices);
	targetSCNetworkServicesMutable = CFArrayCreateMutableCopy(NULL, 0, targetSCNetworkServices);

	serviceMapping = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	for (CFIndex idx = 0;  idx < sourceCount; idx++) {
		sourceBSDName = NULL;
		sourceService = NULL;
		sourceInterface = NULL;
		sourceInterfaceType = NULL;
		sourceInterfaceSubType = NULL;
		bsdNameMapTarget = NULL;

		targetCount = CFArrayGetCount(targetSCNetworkServicesMutable);
		sourceService = (SCNetworkServiceRef) CFArrayGetValueAtIndex(sourceSCNetworkServicesMutable, idx);

		sourceInterface = SCNetworkServiceGetInterface(sourceService);

		if (sourceInterface == NULL) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationCreateServiceMappingUsingBSDMapping: sourceInterface is NULL or not of the correct type"));
			continue;
		}

		sourceInterfaceType = __SCNetworkInterfaceGetEntityType(sourceInterface);
		if ((isA_CFString(sourceInterfaceType) != NULL) &&
		    ((CFEqual(sourceInterfaceType, kSCValNetInterfaceTypeVPN) == TRUE) ||
		     (CFEqual(sourceInterfaceType, kSCValNetInterfaceTypePPP) == TRUE))) {
			    sourceInterfaceSubType = __SCNetworkInterfaceGetEntitySubType(sourceInterface);
			    if (isA_CFString(sourceInterfaceSubType) == NULL) {
				    SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationCreateServiceMappingUsingBSDMapping: sourceInterfaceSubType is NULL or not of the correct type"));
				    continue;
			    }
		}
		else if (((isA_CFString(sourceInterfaceType) != NULL) &&
                         (CFEqual(sourceInterfaceType, kSCValNetInterfaceTypeIPSec) == FALSE) &&
			  (CFEqual(sourceInterfaceType, kSCValNetInterfaceType6to4) == FALSE) &&
			  (CFEqual(sourceInterfaceType, kSCValNetInterfaceTypeLoopback) == FALSE)) ||
			 (isA_CFString(sourceInterfaceType) == NULL)) {
			sourceBSDName = SCNetworkInterfaceGetBSDName(sourceInterface);
			if ((isA_CFString(sourceBSDName) == NULL) ||
			    (CFDictionaryContainsKey(bsdNameMapping, sourceBSDName)) == FALSE) {
				SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationCreateServiceMappingUsingBSDMapping: bsdNameMapping doesn't contain sourceBSDName: %@"), (sourceBSDName == NULL) ? CFSTR("NULL") : sourceBSDName);
				continue;
			}

			bsdNameMapTarget = CFDictionaryGetValue(bsdNameMapping, sourceBSDName);
			if (isA_CFString(bsdNameMapTarget) == NULL) {
				SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationCreateServiceMappingUsingBSDMapping: bsdNameMapTarget is NULL or not of the correct CFType"));
				continue;
			}
		}
		// Find the bsd name in target service
		for (CFIndex idx2 = 0; idx2 < targetCount; idx2++) {
			targetService = NULL;
			targetInterface = NULL;
			targetBSDName = NULL;
			targetInterfaceType = NULL;
			targetInterfaceSubType = NULL;

			targetService = (SCNetworkServiceRef) CFArrayGetValueAtIndex(targetSCNetworkServicesMutable, idx2);

			targetInterface = SCNetworkServiceGetInterface(targetService);
			if (targetInterface == NULL) {
				SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationCreateServiceMappingUsingBSDMapping: targetInterface is NULL or not of the correct type"));
				continue;
			}
                        SCLog(_sc_debug, LOG_NOTICE, CFSTR("targetInterface: %@"), targetInterface);
			if (sourceBSDName != NULL) {
				targetBSDName = SCNetworkInterfaceGetBSDName(targetInterface);
				if (isA_CFString(targetBSDName) == NULL) {
					SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationCreateServiceMappingUsingBSDMapping: targetBSDName is NULL or not of the correct type"));
					continue;
				}

				if (CFEqual(targetBSDName, bsdNameMapTarget) == TRUE) {
					SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationCreateServiceMappingUsingBSDMapping: Removing target BSD Name: %@"), targetBSDName);
					CFDictionaryAddValue(serviceMapping, sourceService, targetService);
					CFArrayRemoveValueAtIndex(targetSCNetworkServicesMutable, idx2);
					break;
				}
			}
			else {
				// Source Interface Type should be VPN
				targetInterfaceType = __SCNetworkInterfaceGetEntityType(targetInterface);
				if ((isA_CFString(targetInterfaceType) == NULL) ||
				    ((CFEqual(targetInterfaceType, kSCValNetInterfaceTypeVPN) == FALSE) &&
				     (CFEqual(targetInterfaceType, kSCValNetInterfaceTypePPP) == FALSE))) {
					    SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationCreateServiceMappingUsingBSDMapping: targetInterfaceType is NULL or not of the correct type : %@"), (targetInterfaceType != NULL) ? targetInterfaceType : CFSTR("NULL"));
					    continue;
				    }
				targetInterfaceSubType = __SCNetworkInterfaceGetEntitySubType(targetInterface);
				if (isA_CFString(targetInterfaceSubType) == NULL) {
					SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationCreateServiceMappingUsingBSDMapping: targetInterfaceSubType is NULL or not of the correct type: %@"), (targetInterfaceSubType != NULL) ? targetInterfaceSubType : CFSTR("NULL"));
					continue;
				}

				// Check if the target interface type and the target interface sub type match
				if ((CFEqual(targetInterfaceType, sourceInterfaceType) == TRUE) &&
				    (CFEqual(targetInterfaceSubType, sourceInterfaceSubType) == TRUE)) {
					SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationCreateServiceMappingUsingBSDMapping: Removing target BSD Name: %@ for VPN"), targetBSDName);
					CFDictionaryAddValue(serviceMapping, sourceService, targetService);
					CFArrayRemoveValueAtIndex(targetSCNetworkServicesMutable, idx2);
					break;
				}
			}
		}
		// Check if sourceService has found a mapping or not, if not the create a NULL mapping to indicate
		// the this service needs to be added and not replaced
		if (CFDictionaryContainsKey(serviceMapping, sourceService) == FALSE) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationCreateServiceMappingUsingBSDMapping: Service needs to be added: %@"), sourceService);
			CFDictionaryAddValue(serviceMapping, sourceService, kCFBooleanFalse);
		}
	}
done:
	if (sourceSCNetworkServices != NULL) {
		CFRelease(sourceSCNetworkServices);
	}
	if (targetSCNetworkServices != NULL) {
		CFRelease(targetSCNetworkServices);
	}
	if (sourceSCNetworkServicesMutable != NULL) {
		CFRelease(sourceSCNetworkServicesMutable);
	}
	if (targetSCNetworkServicesMutable != NULL) {
		CFRelease(targetSCNetworkServicesMutable);
	}
	return serviceMapping;
}

typedef struct {
	SCPreferencesRef targetPrefs;
	CFDictionaryRef bsdMapping;
	CFDictionaryRef setMapping;
	CFDictionaryRef serviceSetMapping;
} ServiceMigrationContext;

// value can be:
//	SCNetworkServiceRef: if target service needs replacement
//	CFBooleanRef: if target service is not present
static void
ServiceMigrationAddOrReplace(const void *key, const void *value, void *context)
{
	CFDictionaryRef bsdMapping = NULL;
	ServiceMigrationContext *ctx = (ServiceMigrationContext*)context;
	CFDictionaryRef setMapping;
	CFDictionaryRef sourceServiceSetMapping;
	SCNetworkServiceRef sourceService = (SCNetworkServiceRef)key;
	SCPreferencesRef targetPrefs = NULL;
	SCNetworkServiceRef targetService = (SCNetworkServiceRef)value;

	targetPrefs = ctx->targetPrefs;
	bsdMapping = ctx->bsdMapping;
	setMapping = ctx->setMapping;
	sourceServiceSetMapping = ctx->serviceSetMapping;
	
	if (isA_SCNetworkService(targetService) != NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("ServiceMigrationAddOrReplace: Removing target service: %@"), targetService);
		SCNetworkServiceRemove(targetService);
	}
	SCLog(_sc_debug, LOG_NOTICE, CFSTR("ServiceMigrationAddOrReplace: Adding service with %@"), sourceService);
	if (__SCNetworkServiceMigrateNew(targetPrefs, sourceService, bsdMapping, setMapping, sourceServiceSetMapping) ==  FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("Could not Add Service: %@"), sourceService);
	}
}

static Boolean
_SCNetworkMigrationDoServiceMigration(SCPreferencesRef sourcePrefs, SCPreferencesRef targetPrefs,
				      CFDictionaryRef serviceMapping, CFDictionaryRef bsdMapping,
				      CFDictionaryRef setMapping, CFDictionaryRef serviceSetMapping)
{
	ServiceMigrationContext context;
	Boolean success = FALSE;

	if ((sourcePrefs == NULL) ||
	    (targetPrefs == NULL) ||
	    (isA_CFDictionary(serviceMapping) == NULL) ||
	    (isA_CFDictionary(bsdMapping) == NULL)) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationDoServiceMigration: targetPrefs or serviceMapping or bsdMapping is NULL"));
		goto done;
	}
	context.targetPrefs = targetPrefs;
	context.bsdMapping = bsdMapping;
	context.setMapping = setMapping;
	context.serviceSetMapping = serviceSetMapping;
	
	CFDictionaryApplyFunction(serviceMapping, ServiceMigrationAddOrReplace, &context);

	success = TRUE;
done:
	return success;
}

static Boolean
_SCNetworkMigrationDoSystemMigration(SCPreferencesRef sourcePrefs, SCPreferencesRef targetPrefs)
{
	CFStringEncoding nameEncoding;
	CFStringRef computerName;
	CFStringRef hostname;
	CFStringRef localHostname;
	CFDictionaryRef btmm = NULL;
	CFDictionaryRef btmmDSID = NULL;
	CFStringRef btmmDSIDPath;
	CFStringRef btmmPath;
	
	
	if ((sourcePrefs == NULL) ||
	    (targetPrefs == NULL)) {
		return FALSE;
	}

	hostname = SCPreferencesGetHostName(sourcePrefs);
	if (hostname != NULL) {
		SCPreferencesSetHostName(targetPrefs, hostname);
	}

	localHostname = _SCPreferencesCopyLocalHostName(sourcePrefs);
	if (localHostname != NULL) {
		SCPreferencesSetLocalHostName(targetPrefs, localHostname);
		CFRelease(localHostname);
	}

	computerName = _SCPreferencesCopyComputerName(sourcePrefs, &nameEncoding);

	if (computerName != NULL) {
		SCPreferencesSetComputerName(targetPrefs, computerName, nameEncoding);
		CFRelease(computerName);
	}
	
	btmmPath = CFStringCreateWithFormat(NULL, NULL,
					    CFSTR("/%@/%@/%@"),
					    kSCPrefSystem,
					    kSCCompNetwork,
					    BACK_TO_MY_MAC);
	btmm = SCPreferencesPathGetValue(sourcePrefs, btmmPath);
	
	if (btmm != NULL) {
		SCPreferencesPathSetValue(targetPrefs, btmmPath, btmm);
	}
	CFRelease(btmmPath);
	
	btmmDSIDPath = CFStringCreateWithFormat(NULL, NULL,
						CFSTR("/%@/%@/%@"),
						kSCPrefSystem,
						kSCCompNetwork,
						BACK_TO_MY_MAC_DSIDS);
	
	btmmDSID = SCPreferencesPathGetValue(sourcePrefs, btmmDSIDPath);
	if (btmmDSID != NULL) {
		SCPreferencesPathSetValue(targetPrefs, btmmDSIDPath, btmmDSID);
	}
	CFRelease(btmmDSIDPath);

	return TRUE;
}
#if	!TARGET_OS_IPHONE

typedef struct {
	CFMutableArrayRef interfaceList;
	SCPreferencesRef ni_prefs;
	CFDictionaryRef bsdMapping;
} SCVirtualInterfaceMemberListContext;

typedef struct {
	SCPreferencesRef prefs;
	SCPreferencesRef ni_prefs;
	CFDictionaryRef bsdMapping;
	CFDictionaryRef virtualBSDMapping;
	CFDictionaryRef mappingBSDNameToService;
	CFDictionaryRef setMapping;
	CFDictionaryRef serviceSetMapping;
} SCVirtualInterfaceContext;

static void
add_virtual_interface(const void *value, void *context)
{
	SCVirtualInterfaceMemberListContext *ctx = (SCVirtualInterfaceMemberListContext*)context;
	CFMutableArrayRef interfaceList = ctx->interfaceList;
	CFDictionaryRef bsdMapping = ctx->bsdMapping;
	CFStringRef oldInterfaceBSDName = (CFStringRef)value;
	SCNetworkInterfaceRef newInterface;
	CFStringRef newInterfaceBSDName;
	
	SCLog(_sc_debug, LOG_NOTICE, CFSTR("old interface BSD name is %@"), oldInterfaceBSDName);
	newInterfaceBSDName = CFDictionaryGetValue(bsdMapping, oldInterfaceBSDName);
	if (newInterfaceBSDName == NULL) {
		return;
	}
	
	SCLog(_sc_debug, LOG_NOTICE, CFSTR("new interface BSD name is %@"), newInterfaceBSDName);
	newInterface = __SCNetworkInterfaceCreateWithNIPreferencesUsingBSDName(NULL, ctx->ni_prefs, newInterfaceBSDName);
	
	if (newInterface != NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("Adding interface to interfaceList: %@"), newInterface);
		CFArrayAppendValue(interfaceList, newInterface);
		CFRelease(newInterface);
	}
	return;
}

static void
add_target_bridge(const void *key, const void *value, void *context)
{
	CFStringRef bridgeName;
	CFDictionaryRef bridgeOptions;
	SCVirtualInterfaceContext *ctx = (SCVirtualInterfaceContext*)context;
	CFDictionaryRef bridgeBSDNameMapping = ctx->virtualBSDMapping;
	CFDictionaryRef bsdNameToServiceMapping = ctx->mappingBSDNameToService;
	SCVirtualInterfaceMemberListContext memberListContext;
	CFMutableArrayRef newInterfaceList;
	SCBridgeInterfaceRef newBridge;
	SCBridgeInterfaceRef oldBridge = (SCBridgeInterfaceRef)key;
	CFStringRef oldBSDName;
	CFArrayRef oldInterfaceList = (CFArrayRef)value;
	CFArrayRef oldServiceList;
	SCPreferencesRef prefs = ctx->prefs;
	CFDictionaryRef serviceSetMapping = ctx->serviceSetMapping;
	CFDictionaryRef setMapping = ctx->setMapping;
	
	newInterfaceList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	
	memberListContext.bsdMapping = ctx->bsdMapping;
	memberListContext.interfaceList = newInterfaceList;
	memberListContext.ni_prefs = ctx->ni_prefs;
	
	CFArrayApplyFunction(oldInterfaceList, CFRangeMake(0, CFArrayGetCount(oldInterfaceList)), add_virtual_interface, &memberListContext);
	
	newBridge = SCBridgeInterfaceCreate(prefs);
	
	if (__SCBridgeInterfaceSetMemberInterfaces(newBridge, newInterfaceList) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("add_target_bridge: Adding Member Interfaces failed"));
	}
	CFRelease(newInterfaceList);
	
	bridgeOptions = SCBridgeInterfaceGetOptions(oldBridge);
	if (bridgeOptions != NULL) {
		SCBridgeInterfaceSetOptions(newBridge, bridgeOptions);
	}
	
	bridgeName = SCNetworkInterfaceGetLocalizedDisplayName(oldBridge);
	
	if (bridgeName != NULL) {
		SCBridgeInterfaceSetLocalizedDisplayName(newBridge, bridgeName);
	}
	
	oldBSDName = SCNetworkInterfaceGetBSDName(oldBridge);
	if (oldBSDName == NULL) {
		goto done;
	}
	
	oldServiceList = CFDictionaryGetValue(bsdNameToServiceMapping, oldBSDName);
	if (oldServiceList == NULL) {
		goto done;
	}
	
	for (CFIndex idx = 0; idx < CFArrayGetCount(oldServiceList); idx++) {
		SCNetworkServiceRef oldService = CFArrayGetValueAtIndex(oldServiceList, idx);
		if (__SCNetworkServiceMigrateNew(prefs, oldService, bridgeBSDNameMapping, setMapping, serviceSetMapping) == FALSE) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("add_target_bridge: Could not migrate service: %@"), oldService);
		}
	}
done:
	CFRelease(newBridge);
}

static void
_SCNetworkMigrationRemoveBridgeServices(SCPreferencesRef prefs)
{
	CFArrayRef services = SCNetworkServiceCopyAll(prefs);
	
	for (CFIndex idx = 0; idx < CFArrayGetCount(services); idx++) {
		SCNetworkServiceRef service = CFArrayGetValueAtIndex(services, idx);
		SCNetworkInterfaceRef interface = SCNetworkServiceGetInterface(service);
		CFStringRef bsdName = SCNetworkInterfaceGetBSDName(interface);
		
		if ((bsdName != NULL) &&
		SCNetworkInterfaceGetInterfaceType(interface) == kSCNetworkInterfaceTypeBridge) {
			SCLog(TRUE, LOG_NOTICE, CFSTR("_SCNetworkMigrationRemoveBridgeServices: Removing services: %@"), service);
			SCNetworkServiceRemove(service);
		}
	}
	CFRelease(services);
}


static CFDictionaryRef
_SCNetworkMigrationCopyMappingBSDNameToBridgeServices(SCPreferencesRef prefs)
{
	CFArrayRef services = SCNetworkServiceCopyAll(prefs);
	CFMutableDictionaryRef bridgeServices = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	
	for (CFIndex idx = 0; idx < CFArrayGetCount(services); idx++) {
		SCNetworkServiceRef service = CFArrayGetValueAtIndex(services, idx);
		SCNetworkInterfaceRef interface = SCNetworkServiceGetInterface(service);
		CFStringRef bsdName = SCNetworkInterfaceGetBSDName(interface);
		
		if ((bsdName != NULL) &&
		SCNetworkInterfaceGetInterfaceType(interface) == kSCNetworkInterfaceTypeBridge) {
			CFMutableArrayRef serviceList;
			if (CFDictionaryContainsKey(bridgeServices, bsdName) == FALSE) {
				serviceList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
				CFDictionaryAddValue(bridgeServices, bsdName, serviceList);
				CFRelease(serviceList);
			}
			serviceList = (CFMutableArrayRef)CFDictionaryGetValue(bridgeServices, bsdName);
			CFArrayAppendValue(serviceList, service);
		}
	}
	CFRelease(services);
	return bridgeServices;
}


static Boolean
_SCNetworkMigrationDoBridgeMigration (SCPreferencesRef sourcePrefs,
				      SCPreferencesRef sourceNIPrefs,
				      SCPreferencesRef targetPrefs,
				      SCPreferencesRef targetNIPrefs,
				      CFDictionaryRef bsdMapping,
				      CFDictionaryRef setMapping,
				      CFDictionaryRef serviceSetMapping)
{
	CFArrayRef allSourceBridges;
	CFArrayRef allTargetBridges;
	SCBridgeInterfaceRef bridge;
	CFMutableDictionaryRef bridgeInterfaceMapping = NULL;
	CFMutableDictionaryRef bridgeMapping;
	CFDictionaryRef bsdNameToBridgeServices;
	SCVirtualInterfaceContext context;
	CFIndex count = 0;
	Boolean success = FALSE;
	
	allSourceBridges = SCBridgeInterfaceCopyAll(sourcePrefs);
	allTargetBridges = SCBridgeInterfaceCopyAll(targetPrefs);
	
	bsdNameToBridgeServices = _SCNetworkMigrationCopyMappingBSDNameToBridgeServices(sourcePrefs);
	
	bridgeInterfaceMapping = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	bridgeMapping = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	
	// Create Bridge Interface Mapping
	for (CFIndex idx = 0; idx < CFArrayGetCount(allSourceBridges); idx++) {
		bridge = CFArrayGetValueAtIndex(allSourceBridges, idx);
		CFArrayRef bridgeMembers = SCBridgeInterfaceGetMemberInterfaces(bridge);
		CFMutableArrayRef interfaceList;
		
		interfaceList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		for (CFIndex idx2 = 0; idx2 < CFArrayGetCount(bridgeMembers); idx2++) {
			CFStringRef interfaceName = NULL;
			SCNetworkInterfaceRef interface = NULL;
			
			interface = CFArrayGetValueAtIndex(bridgeMembers, idx2);
			interfaceName = SCNetworkInterfaceGetBSDName(interface);
			
			if (CFDictionaryContainsKey(bsdMapping, interfaceName) == TRUE) {
				CFStringRef bridgeNewName = CFStringCreateWithFormat(NULL, NULL, CFSTR("bridge%ld"), count);
				CFDictionaryAddValue(bridgeMapping, interfaceName, bridgeNewName);
				CFArrayAppendValue(interfaceList, interfaceName);
				CFRelease(bridgeNewName);
				count++;
			}
		}
		if (CFArrayGetCount(interfaceList) > 0) {
			CFDictionaryAddValue(bridgeInterfaceMapping, bridge, interfaceList);
		}
		CFRelease(interfaceList);
	}
	// Remove bridge services from target
	_SCNetworkMigrationRemoveBridgeServices(targetPrefs);
	
	// Remove Target Bridges
	for (CFIndex idx = 0; idx < CFArrayGetCount(allTargetBridges); idx++) {
		bridge = CFArrayGetValueAtIndex(allTargetBridges, idx);
		if (SCBridgeInterfaceRemove(bridge) == FALSE) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationDoBridgeMigration: Could not remove bridge: %@"), bridge);
			goto done;
		}
	}
	
	context.prefs = targetPrefs;
	context.ni_prefs = targetNIPrefs;
	context.bsdMapping = bsdMapping;
	context.virtualBSDMapping = bridgeMapping;
	context.mappingBSDNameToService = bsdNameToBridgeServices;
	context.setMapping = setMapping;
	context.serviceSetMapping = serviceSetMapping;
	
	// Add Bridge configurations at the target using mapping
	CFDictionaryApplyFunction(bridgeInterfaceMapping, add_target_bridge, &context);
	
	success = TRUE;
done:
	CFRelease(allSourceBridges);
	CFRelease(allTargetBridges);
	CFRelease(bridgeInterfaceMapping);
	CFRelease(bridgeMapping);
	CFRelease(bsdNameToBridgeServices);
	return success;
}


static void
add_target_bond(const void *key, const void *value, void *context)
{
	CFNumberRef bondMode;
	CFStringRef bondName;
	CFDictionaryRef bondOptions;
	SCVirtualInterfaceContext *ctx = (SCVirtualInterfaceContext*)context;
	CFDictionaryRef bondBSDNameMapping = ctx->virtualBSDMapping;
	CFDictionaryRef bsdNameToServiceMapping = ctx->mappingBSDNameToService;
	SCVirtualInterfaceMemberListContext memberListContext;
	CFMutableArrayRef newInterfaceList;
	SCBondInterfaceRef newBond;
	SCBondInterfaceRef oldBond = (SCBondInterfaceRef)key;
	CFStringRef oldBSDName;
	CFArrayRef oldInterfaceList = (CFArrayRef)value;
	CFArrayRef oldServiceList;
	SCPreferencesRef prefs = ctx->prefs;
	CFDictionaryRef serviceSetMapping = ctx->serviceSetMapping;
	CFDictionaryRef setMapping = ctx->setMapping;
	
	newInterfaceList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	
	memberListContext.bsdMapping = ctx->bsdMapping;
	memberListContext.interfaceList = newInterfaceList;
	memberListContext.ni_prefs = ctx->ni_prefs;
	
	CFArrayApplyFunction(oldInterfaceList, CFRangeMake(0, CFArrayGetCount(oldInterfaceList)), add_virtual_interface, &memberListContext);
	
	newBond = SCBondInterfaceCreate(prefs);
	if (__SCBondInterfaceSetMemberInterfaces(newBond, newInterfaceList) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("add_target_bond: Adding member interface failed."));
	}
	CFRelease(newInterfaceList);
	
	bondOptions = SCBondInterfaceGetOptions(oldBond);
	if (bondOptions != NULL) {
		SCBondInterfaceSetOptions(newBond, bondOptions);
	}
	
	bondName = SCNetworkInterfaceGetLocalizedDisplayName(oldBond);
	if (bondName != NULL) {
		SCBondInterfaceSetLocalizedDisplayName(newBond, bondName);
	}
	
	bondMode = SCBondInterfaceGetMode(oldBond);
	if (bondMode != NULL) {
		SCBondInterfaceSetMode(newBond, bondMode);
	}
	oldBSDName = SCNetworkInterfaceGetBSDName(oldBond);
	if (oldBSDName == NULL) {
		goto done;
	}
	
	oldServiceList = CFDictionaryGetValue(bsdNameToServiceMapping, oldBSDName);
	if (oldServiceList == NULL) {
		goto done;
	}
	
	for (CFIndex idx = 0; idx < CFArrayGetCount(oldServiceList); idx++) {
		SCNetworkServiceRef oldService = CFArrayGetValueAtIndex(oldServiceList, idx);
		if (__SCNetworkServiceMigrateNew(prefs, oldService, bondBSDNameMapping, setMapping, serviceSetMapping) == FALSE) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("add_target_bond: Could not migrate service: %@"), oldService);
		}
	}
done:
	CFRelease(newBond);
}

static void
_SCNetworkMigrationRemoveBondServices(SCPreferencesRef prefs)
{
	CFArrayRef services = SCNetworkServiceCopyAll(prefs);
	
	for (CFIndex idx = 0; idx < CFArrayGetCount(services); idx++) {
		SCNetworkServiceRef service = CFArrayGetValueAtIndex(services, idx);
		SCNetworkInterfaceRef interface = SCNetworkServiceGetInterface(service);
		CFStringRef bsdName = SCNetworkInterfaceGetBSDName(interface);
		
		if ((bsdName != NULL) &&
		SCNetworkInterfaceGetInterfaceType(interface) == kSCNetworkInterfaceTypeBond) {
			SCNetworkServiceRemove(service);
		}
	}
	CFRelease(services);
}


static CFDictionaryRef
_SCNetworkMigrationCopyMappingBSDNameToBondServices(SCPreferencesRef prefs)
{
	CFArrayRef services = SCNetworkServiceCopyAll(prefs);
	CFMutableDictionaryRef bondServices = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	
	for (CFIndex idx = 0; idx < CFArrayGetCount(services); idx++) {
		SCNetworkServiceRef service = CFArrayGetValueAtIndex(services, idx);
		SCNetworkInterfaceRef interface = SCNetworkServiceGetInterface(service);
		CFStringRef bsdName = SCNetworkInterfaceGetBSDName(interface);
		
		if ((bsdName != NULL) &&
		SCNetworkInterfaceGetInterfaceType(interface) == kSCNetworkInterfaceTypeBond) {
			CFMutableArrayRef serviceList;
			if (CFDictionaryContainsKey(bondServices, bsdName) == FALSE) {
				serviceList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
				CFDictionaryAddValue(bondServices, bsdName, serviceList);
				CFRelease(serviceList);
			}
			serviceList = (CFMutableArrayRef)CFDictionaryGetValue(bondServices, bsdName);
			CFArrayAppendValue(serviceList, service);
		}
	}
	CFRelease(services);
	return bondServices;
}


static Boolean
_SCNetworkMigrationDoBondMigration (SCPreferencesRef sourcePrefs,
				    SCPreferencesRef sourceNIPrefs,
				    SCPreferencesRef targetPrefs,
				    SCPreferencesRef targetNIPrefs,
				    CFDictionaryRef bsdMapping,
				    CFDictionaryRef setMapping,
				    CFDictionaryRef serviceSetMapping)
{
	CFArrayRef allSourceBonds;
	CFArrayRef allTargetBonds;
	SCBondInterfaceRef bond;
	CFMutableDictionaryRef bondInterfaceMapping = NULL;
	CFMutableDictionaryRef bondMapping;
	CFDictionaryRef bsdNameToBondServices;
	SCVirtualInterfaceContext context;
	CFIndex count = 0;
	Boolean success = FALSE;
	
	allSourceBonds = SCBondInterfaceCopyAll(sourcePrefs);
	allTargetBonds = SCBondInterfaceCopyAll(targetPrefs);
	
	bsdNameToBondServices = _SCNetworkMigrationCopyMappingBSDNameToBondServices(sourcePrefs);
	
	bondInterfaceMapping = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	bondMapping = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	// Create Bond Interface mapping
	for (CFIndex idx = 0; idx < CFArrayGetCount(allSourceBonds); idx++) {
		bond = CFArrayGetValueAtIndex(allSourceBonds, idx);
		CFArrayRef bondMembers = SCBondInterfaceGetMemberInterfaces(bond);
		CFMutableArrayRef interfaceList;
		
		interfaceList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		for (CFIndex idx2 = 0; idx2 < CFArrayGetCount(bondMembers); idx2++) {
			CFStringRef interfaceName;
			SCNetworkInterfaceRef interface;
			
			interface = CFArrayGetValueAtIndex(bondMembers, idx2);
			interfaceName = SCNetworkInterfaceGetBSDName(interface);
			
			if (CFDictionaryContainsKey(bsdMapping, interfaceName) == TRUE) {
				CFStringRef bondNewName = CFStringCreateWithFormat(NULL, NULL, CFSTR("bond%ld"), count);
				CFDictionaryAddValue(bondMapping, interfaceName, bondNewName);
				CFArrayAppendValue(interfaceList, interfaceName);
				CFRelease(bondNewName);
				count++;
			}
		}
		if (CFArrayGetCount(interfaceList) > 0) {
			CFDictionaryAddValue(bondInterfaceMapping, bond, interfaceList);
		}
		CFRelease(interfaceList);
	}
	// Remove bond services from target
	_SCNetworkMigrationRemoveBondServices(targetPrefs);
	
	// Remove Target Bonds
	for (CFIndex idx = 0; idx < CFArrayGetCount(allTargetBonds); idx++) {
		bond = CFArrayGetValueAtIndex(allTargetBonds, idx);
		if (SCBondInterfaceRemove(bond) == FALSE) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationDoBondMigration: Could not remove bond: %@"), bond);
			goto done;
		}
	}
	
	context.prefs = targetPrefs;
	context.ni_prefs = targetNIPrefs;
	context.bsdMapping = bsdMapping;
	context.virtualBSDMapping = bondMapping;
	context.mappingBSDNameToService = bsdNameToBondServices;
	context.setMapping = setMapping;
	context.serviceSetMapping = serviceSetMapping;
	
	// Add Bond configurations at the target using mapping
	CFDictionaryApplyFunction(bondInterfaceMapping, add_target_bond, &context);
	
	success = TRUE;
done:
	CFRelease(allSourceBonds);
	CFRelease(allTargetBonds);
	CFRelease(bondInterfaceMapping);
	CFRelease(bondMapping);
	CFRelease(bsdNameToBondServices);
	return success;
}

static void
add_target_vlan(const void *value, void *context)
{
	CFDictionaryRef bsdMapping;
	SCVirtualInterfaceContext *ctx = (SCVirtualInterfaceContext*)context;
	CFDictionaryRef bsdNameToServiceMapping = ctx->mappingBSDNameToService;
	SCPreferencesRef prefs = ctx->prefs;
	SCVLANInterfaceRef newVLAN = NULL;
	SCNetworkInterfaceRef newPhysicalInterface = NULL;
	CFStringRef newPhysicalInterfaceName;
	SCVLANInterfaceRef oldVLAN = (SCVLANInterfaceRef)value;
	CFStringRef oldBSDName;
	SCNetworkInterfaceRef oldPhysicalInterface;
	CFStringRef oldPhysicalInterfaceName;
	SCNetworkServiceRef oldService;
	CFArrayRef oldServiceList;
	CFDictionaryRef serviceSetMapping = ctx->serviceSetMapping;
	CFDictionaryRef setMapping = ctx->setMapping;
	CFDictionaryRef vlanBSDMapping = ctx->virtualBSDMapping;
	CFNumberRef vlanTag;
	CFStringRef vlanName;
	CFDictionaryRef vlanOptions;
	
	bsdMapping = ctx->bsdMapping;
	oldPhysicalInterface = SCVLANInterfaceGetPhysicalInterface(oldVLAN);
	
	if (oldPhysicalInterface == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("add_target_vlan: oldPhysicalInterface is NULL"));
		goto done;
	}
	oldPhysicalInterfaceName = SCNetworkInterfaceGetBSDName(oldPhysicalInterface);
	
	if (oldPhysicalInterfaceName == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("add_target_vlan: oldPhysicalInterfaceName is NULL"));
		goto done;
	}
	
	newPhysicalInterfaceName = CFDictionaryGetValue(bsdMapping, oldPhysicalInterfaceName);
	if (newPhysicalInterfaceName == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("add_target_vlan: newPhysicalInterfaceName is NULL"));
		goto done;
	}
	newPhysicalInterface = __SCNetworkInterfaceCreateWithNIPreferencesUsingBSDName(NULL, ctx->ni_prefs, newPhysicalInterfaceName);
	if (newPhysicalInterface == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("add_target_vlan: newPhysicalInterface is NULL"));
		goto done;
	}
	
	vlanTag = SCVLANInterfaceGetTag(oldVLAN);
	if (vlanTag == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("add_target_vlan: vlanTag is NULL"));
		goto done;
	}
	
	newVLAN = SCVLANInterfaceCreate(prefs, newPhysicalInterface, vlanTag);
	if (newVLAN == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("add_target_vlan: Could not create newVLAN"));
	}
	
	vlanName = SCNetworkInterfaceGetLocalizedDisplayName(oldVLAN);
	if (vlanName != NULL) {
		SCVLANInterfaceSetLocalizedDisplayName(newVLAN, vlanName);
	}
	
	vlanOptions = SCVLANInterfaceGetOptions(oldVLAN);
	if (vlanOptions != NULL) {
		SCVLANInterfaceSetOptions(newVLAN, vlanOptions);
	}
	oldBSDName = SCNetworkInterfaceGetBSDName(oldVLAN);
	
	if (oldBSDName == NULL) {
		goto done;
	}
	
	oldServiceList = CFDictionaryGetValue(bsdNameToServiceMapping, oldBSDName);
	if (oldServiceList == NULL) {
		goto done;
	}
	
	for (CFIndex idx = 0; idx < CFArrayGetCount(oldServiceList); idx++) {
		oldService = CFArrayGetValueAtIndex(oldServiceList, idx);
		if (__SCNetworkServiceMigrateNew(prefs, oldService, vlanBSDMapping, setMapping, serviceSetMapping) == FALSE) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("add_target_vlan: Could not migrate service: %@"), oldService);
		}
	}
	
done:
	if (newPhysicalInterface != NULL) {
		CFRelease(newPhysicalInterface);
	}
	if (newVLAN != NULL) {
		CFRelease(newVLAN);
	}
}

static void
_SCNetworkMigrationRemoveVLANServices(SCPreferencesRef prefs)
{
	CFArrayRef services = SCNetworkServiceCopyAll(prefs);
	
	for (CFIndex idx = 0; idx < CFArrayGetCount(services); idx++) {
		SCNetworkServiceRef service = CFArrayGetValueAtIndex(services, idx);
		SCNetworkInterfaceRef interface = SCNetworkServiceGetInterface(service);
		CFStringRef bsdName = SCNetworkInterfaceGetBSDName(interface);
		
		if ((bsdName != NULL) &&
		    SCNetworkInterfaceGetInterfaceType(interface) == kSCNetworkInterfaceTypeVLAN) {
			SCNetworkServiceRemove(service);
		}
	}
	
	CFRelease(services);
}


static CFDictionaryRef
_SCNetworkMigrationCopyMappingBSDNameToVLANServices(SCPreferencesRef prefs)
{
	CFArrayRef services = SCNetworkServiceCopyAll(prefs);
	CFMutableDictionaryRef vlanServices = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	
	for (CFIndex idx = 0; idx < CFArrayGetCount(services); idx++) {
		SCNetworkServiceRef service = CFArrayGetValueAtIndex(services, idx);
		SCNetworkInterfaceRef interface = SCNetworkServiceGetInterface(service);
		CFStringRef bsdName = SCNetworkInterfaceGetBSDName(interface);
		
		if ((bsdName != NULL) &&
		    SCNetworkInterfaceGetInterfaceType(interface) == kSCNetworkInterfaceTypeVLAN) {
			CFMutableArrayRef serviceList;
			if (CFDictionaryContainsKey(vlanServices, bsdName) == FALSE) {
				serviceList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
				CFDictionaryAddValue(vlanServices, bsdName, serviceList);
				CFRelease(serviceList);
			}
			serviceList = (CFMutableArrayRef)CFDictionaryGetValue(vlanServices, bsdName);
			CFArrayAppendValue(serviceList, service);
		}
	}
	CFRelease(services);
	return vlanServices;
}

static Boolean
_SCNetworkMigrationDoVLANMigration (SCPreferencesRef sourcePrefs,
				    SCPreferencesRef sourceNIPrefs,
				    SCPreferencesRef targetPrefs,
				    SCPreferencesRef targetNIPrefs,
				    CFDictionaryRef bsdMapping,
				    CFDictionaryRef setMapping,
				    CFDictionaryRef serviceSetMapping)
{
	CFArrayRef allSourceVLAN;
	CFArrayRef allTargetVLAN;
	SCVirtualInterfaceContext context;
	CFIndex count = 0;
	Boolean success = FALSE;
	SCVLANInterfaceRef vlan;
	CFMutableArrayRef vlanList;
	CFMutableDictionaryRef vlanMapping;
	CFDictionaryRef bsdNameToVLANServices;
	
	allSourceVLAN = SCVLANInterfaceCopyAll(sourcePrefs);
	allTargetVLAN = SCVLANInterfaceCopyAll(targetPrefs);
	
	bsdNameToVLANServices = _SCNetworkMigrationCopyMappingBSDNameToVLANServices(sourcePrefs);
	
	vlanList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	vlanMapping = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	
	for (CFIndex idx = 0; idx < CFArrayGetCount(allSourceVLAN); idx++) {
		vlan = CFArrayGetValueAtIndex(allSourceVLAN, idx);
		CFStringRef vlanBSDName = SCNetworkInterfaceGetBSDName(vlan);
		SCNetworkInterfaceRef physicalInterface = SCVLANInterfaceGetPhysicalInterface(vlan);
		CFStringRef physicalInterfaceName;
		
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationDoVLANMigration: physical interface is %@"), physicalInterface);
		
		physicalInterfaceName = SCNetworkInterfaceGetBSDName(physicalInterface);
                SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationDoVLANMigration: Physical Interface name is %@"), physicalInterfaceName);
                
		// Add VLAN to be migrated if the mapping between interfaces exists
		if (CFDictionaryContainsKey(bsdMapping, physicalInterfaceName) == TRUE) {
			CFStringRef vlanNewName = CFStringCreateWithFormat(NULL, NULL, CFSTR("vlan%ld"), count);
			CFDictionaryAddValue(vlanMapping, vlanBSDName, vlanNewName);
			CFArrayAppendValue(vlanList, vlan);
			CFRelease(vlanNewName);
			count++;
		}
	}
	// Remove vlan services from target
	_SCNetworkMigrationRemoveVLANServices(targetPrefs);
	
	// Remove Target VLANs
	for (CFIndex idx = 0; idx < CFArrayGetCount(allTargetVLAN); idx++) {
		vlan = CFArrayGetValueAtIndex(allTargetVLAN, idx);
		if (SCVLANInterfaceRemove(vlan) == FALSE) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationDoVLANMigration: Could not remove VLAN: %@"), vlan);
			goto done;
		}
	}
	
	context.prefs = targetPrefs;
	context.ni_prefs = targetNIPrefs;
	context.bsdMapping = bsdMapping;
	context.virtualBSDMapping = vlanMapping;
	context.mappingBSDNameToService = bsdNameToVLANServices;
	context.setMapping = setMapping;
	context.serviceSetMapping = serviceSetMapping;
	
	// Add VLAN configurations at the target using vlanList
	CFArrayApplyFunction(vlanList, CFRangeMake(0, CFArrayGetCount(vlanList)), add_target_vlan, &context);
	
	success = TRUE;
done:
	CFRelease(allSourceVLAN);
	CFRelease(allTargetVLAN);
	CFRelease(vlanList);
	CFRelease(vlanMapping);
	CFRelease(bsdNameToVLANServices);
	return success;
}

static Boolean
_SCNetworkMigrationDoVirtualNetworkInterfaceMigration(SCPreferencesRef sourcePrefs,
						      SCPreferencesRef sourceNIPrefs,
						      SCPreferencesRef targetPrefs,
						      SCPreferencesRef targetNIPrefs,
						      CFDictionaryRef bsdMapping,
						      CFDictionaryRef setMapping,
						      CFDictionaryRef serviceSetMapping)
{
	// Handle Bridges
	if (_SCNetworkMigrationDoBridgeMigration(sourcePrefs, sourceNIPrefs,
						 targetPrefs, targetNIPrefs,
						 bsdMapping, setMapping, serviceSetMapping) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationDoVirtualNetworkInterfaceMigration: Bridge migration failed"));
	}
	
	// Handle Bonds
	if (_SCNetworkMigrationDoBondMigration(sourcePrefs, sourceNIPrefs,
					       targetPrefs, targetNIPrefs,
					       bsdMapping, setMapping, serviceSetMapping) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationDoVirtualNetworkInterfaceMigration: Bond migration failed"));
	}
	
	// Handle VLANs
	if (_SCNetworkMigrationDoVLANMigration(sourcePrefs, sourceNIPrefs,
					       targetPrefs, targetNIPrefs,
					       bsdMapping, setMapping, serviceSetMapping) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationDoVirtualNetworkInterfaceMigration: VLAN migration failed"));
	}
	return TRUE;
}
#endif

typedef struct {
	SCPreferencesRef prefs;
	CFArrayRef serviceOrder;
	CFMutableArrayRef serviceListMutable;
	Boolean* success;
} migrated_service_context;

static void
create_migrated_order(const void *value, void *context)
{
	migrated_service_context *ctx = (migrated_service_context*)context;
	CFMutableArrayRef migratedServiceOrder = ctx->serviceListMutable;
	CFArrayRef targetServiceOrder = ctx->serviceOrder;
	CFStringRef migratedServiceID = (CFStringRef)value;
	Boolean *success = ctx->success;
	
	if (*success == FALSE) {
		return;
	}
	// Preserving the service order in the source configuration for the services
	// which were migrated into the target configuration
	for (CFIndex idx = 0; idx < CFArrayGetCount(targetServiceOrder); idx++) {
		CFStringRef targetServiceID = CFArrayGetValueAtIndex(targetServiceOrder, idx);
		if (CFEqual(migratedServiceID, targetServiceID) == TRUE) {
			CFArrayAppendValue(migratedServiceOrder, migratedServiceID);
			return;
		}
	}
}

static void
create_non_migrated_service_list(const void *value, void *context)
{
	migrated_service_context *ctx = (migrated_service_context*)context;
	CFArrayRef migratedServiceOrder = ctx->serviceOrder;
	CFMutableArrayRef nonMigratedService = ctx->serviceListMutable;
	SCPreferencesRef prefs = ctx->prefs;
	SCNetworkServiceRef service;
	Boolean *success = ctx->success;
	CFStringRef targetServiceID = (CFStringRef)value;
	
	if (*success == FALSE) {
		return;
	}
	// Adding all services not present in migratedServiceOrder into nonMigrated service
	for (CFIndex idx = 0; idx < CFArrayGetCount(migratedServiceOrder); idx++) {
		CFStringRef migratedServiceID = CFArrayGetValueAtIndex(migratedServiceOrder, idx);
		
		if (CFEqual(targetServiceID, migratedServiceID) == TRUE) {
			return;
		}
	}
	service = SCNetworkServiceCopy(prefs, targetServiceID);
	if (service == NULL) {
		*success = FALSE;
		return;
	}
	
	CFArrayAppendValue(nonMigratedService, service);
	CFRelease(service);
}

static void
preserve_service_order(const void *key, const void *value, void *context)
{
	migrated_service_context migrated_context;
	CFMutableArrayRef migratedServiceOrder;
	migrated_service_context non_migrated_context;
	CFMutableArrayRef nonMigratedServices;
	SCNetworkSetRef sourceSet = (SCNetworkSetRef)key;
	CFArrayRef sourceServiceOrder = NULL;
	Boolean *success = (Boolean*)context;
	SCNetworkSetRef targetSet = (SCNetworkSetRef)value;
	SCNetworkSetPrivateRef targetPrivate = (SCNetworkSetPrivateRef)targetSet;
	CFArrayRef targetServiceOrder = NULL;
	
	if (*success == FALSE) {
		return;
	}
	migratedServiceOrder = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	nonMigratedServices = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	
	sourceServiceOrder = SCNetworkSetGetServiceOrder(sourceSet);
	if (sourceServiceOrder == NULL) {
		goto done;
	}
	targetServiceOrder = SCNetworkSetGetServiceOrder(targetSet);
	if (targetServiceOrder == NULL) {
		goto done;
	}
	
	migrated_context.prefs = NULL;
	migrated_context.serviceOrder = targetServiceOrder;
	migrated_context.serviceListMutable = migratedServiceOrder;
	migrated_context.success = success;
	
	// Creating a list of service IDs which were migrated in the target set
	// while maintaining the service order or the source set
	CFArrayApplyFunction(sourceServiceOrder, CFRangeMake(0, CFArrayGetCount(sourceServiceOrder)), create_migrated_order, &migrated_context);
	
	if (success == FALSE) {
		goto done;
	}
	
	non_migrated_context.prefs = targetPrivate->prefs;
	non_migrated_context.serviceOrder = migratedServiceOrder;
	non_migrated_context.serviceListMutable = nonMigratedServices;
	non_migrated_context.success = success;
	
	// Creating a list of all the services which were not migrated from the source set to the
	// target set
	CFArrayApplyFunction(targetServiceOrder, CFRangeMake(0, CFArrayGetCount(targetServiceOrder)), create_non_migrated_service_list, &non_migrated_context);
	
	// Remove non migrated service
	for (CFIndex idx = 0; idx < CFArrayGetCount(nonMigratedServices); idx++) {
		SCNetworkServiceRef service = CFArrayGetValueAtIndex(nonMigratedServices, idx);
		SCNetworkSetRemoveService(targetSet, service);
	}
	// Set migrated service order
	SCNetworkSetSetServiceOrder(targetSet, migratedServiceOrder);
	
	// Add non migrated services
	for (CFIndex idx = 0; idx < CFArrayGetCount(nonMigratedServices); idx++) {
		SCNetworkServiceRef service = CFArrayGetValueAtIndex(nonMigratedServices, idx);
		SCNetworkSetAddService(targetSet, service);
	}
	
done:
	CFRelease(migratedServiceOrder);
	CFRelease(nonMigratedServices);
	return;
	
}

static Boolean
_SCNetworkMigrationDoServiceOrderMigration(SCPreferencesRef sourcePrefs,
					   SCPreferencesRef targetPrefs,
					   CFDictionaryRef setMapping)
{
	Boolean success = TRUE;
	
	if (isA_CFDictionary(setMapping) == NULL) {
		success = FALSE;
		goto done;
	}
	
	CFDictionaryApplyFunction(setMapping, preserve_service_order, &success);
done:
	return success;
}


// This is a function that looks at source and target network configuration
// and determines what network configurations can be transferred from source to
// target
static Boolean
_SCNetworkConfigurationMigrateConfiguration(CFURLRef sourceDir, CFURLRef targetDir)
{
	CFDictionaryRef bsdNameMapping = NULL;              // Mapping between BSD name and SCNetworkInterfaceRef to help with mapping services
	CFMutableDictionaryRef builtinMapping = NULL;       // Mapping between builtin interfaces between source and target configurations: (SCNetworkInterfaceRef -> SCNetworkInterfaceRef)
	CFMutableDictionaryRef externalMapping = NULL;      // Mapping between external interfaces between source and target configurations: (SCNetworkInterfaceRef -> SCNetworkInterfaceRef)
	Boolean migrationSuccess = FALSE;
	CFArrayRef newTargetNetworkInterfaceEntity = NULL;  // Array of Interface Entity which used to create new target interfaces created during migration
	CFDictionaryRef serviceMapping = NULL;              // Mapping between services of source to target. (SCNetworkServicesRef -> SCNetworkServicesRef)
	CFDictionaryRef setMapping = NULL;
	CFDictionaryRef sourceServiceSetMapping = NULL;
	CFArrayRef sourceConfigurationFiles = NULL;							// Path to the source configuration files which need to be migrated
	CFURLRef sourceNetworkInterfaceFile = NULL;			// Source CFURLRef for preferences.plist and NetworkInterfaces.plist
	char sourceNetworkInterfaceFileStr[PATH_MAX];
	CFStringRef sourceNetworkInterfaceFileString = NULL;	// Source CFStringRef for preferences.plist and NetworkInterfaces.plist
	SCPreferencesRef sourceNetworkInterfacePrefs = NULL;			// Source SCPreferencesRef for preferences.plist and NetworkInterfaces.plist
	CFURLRef sourcePreferencesFile = NULL;
	char sourcePreferencesFileStr[PATH_MAX];
	CFStringRef sourcePreferencesFileString = NULL;
	SCPreferencesRef sourcePrefs = NULL;
	CFArrayRef targetConfigurationFiles = NULL;							// Path to the target configuration files where migration will take place to
	Boolean targetConfigurationFilesPresent;
	CFURLRef targetNetworkInterfaceFile = NULL;			// Target CFURLRef for preferences.plist and NetworkInterfaces.plist
	char targetNetworkInterfaceFileStr[PATH_MAX];
	CFStringRef targetNetworkInterfaceFileString = NULL;	// Target CFStringRef for preferences.plist and NetworkInterfaces.plist
	SCPreferencesRef targetNetworkInterfacePrefs = NULL;			// Target SCPreferencesRef for preferences.plist and NetworkInterfaces.plist
	CFURLRef targetPreferencesFile = NULL;
	char targetPreferencesFileStr[PATH_MAX];
	CFStringRef targetPreferencesFileString = NULL;
	SCPreferencesRef targetPrefs = NULL;
	CFMutableDictionaryRef validityOptions = NULL;

	// Check if configuration files exist in sourceDir
	if (__SCNetworkConfigurationMigrateConfigurationFilesPresent(sourceDir, &sourceConfigurationFiles) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateConfiguration: sourceDir: (%@) doesn't contain configuration files"), sourceDir);
		goto done;
	}

	sourcePreferencesFile = CFArrayGetValueAtIndex(sourceConfigurationFiles, PREFERENCES_PLIST_INDEX);
	if (CFURLGetFileSystemRepresentation(sourcePreferencesFile, TRUE, (UInt8*)sourcePreferencesFileStr, sizeof(sourcePreferencesFileStr)) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateConfiguration: Cannot get file system representation for url: %@"), sourcePreferencesFile);
		goto done;
	}

	sourceNetworkInterfaceFile = CFArrayGetValueAtIndex(sourceConfigurationFiles, NETWORK_INTERFACES_PLIST_INDEX);
	if (CFURLGetFileSystemRepresentation(sourceNetworkInterfaceFile, TRUE, (UInt8*)sourceNetworkInterfaceFileStr, sizeof(sourceNetworkInterfaceFileStr)) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateConfiguration: Cannot get file system representation for url: %@"), sourceNetworkInterfaceFile);
		goto done;
	}

	sourcePreferencesFileString = CFStringCreateWithCString(NULL, sourcePreferencesFileStr, kCFStringEncodingUTF8);
	sourceNetworkInterfaceFileString = CFStringCreateWithCString(NULL, sourceNetworkInterfaceFileStr, kCFStringEncodingUTF8);

	sourcePrefs = SCPreferencesCreate(NULL, PLUGIN_ID, sourcePreferencesFileString);
	sourceNetworkInterfacePrefs = SCPreferencesCreate(NULL, PLUGIN_ID, sourceNetworkInterfaceFileString);
	if ((sourcePrefs == NULL) || (sourceNetworkInterfacePrefs == NULL)) {
		goto done;
	}
	
	if ((targetConfigurationFilesPresent = __SCNetworkConfigurationMigrateConfigurationFilesPresent(targetDir, &targetConfigurationFiles)) == FALSE) {
		if (targetConfigurationFiles == NULL) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateConfiguration: targetConfigurationFiles is NULL"));
			goto done;
		}
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateConfiguration: targetDir: (%@) doesn't contain configuration files ... Need to create default configuration"), targetDir);
	}
	
	targetPreferencesFile = CFArrayGetValueAtIndex(targetConfigurationFiles, PREFERENCES_PLIST_INDEX);
	if (CFURLGetFileSystemRepresentation(targetPreferencesFile, TRUE, (UInt8*)targetPreferencesFileStr, sizeof(targetPreferencesFileStr)) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateConfiguration: Cannot get file system representation for url: %@"), targetPreferencesFile);
		goto done;
	}
	targetNetworkInterfaceFile = CFArrayGetValueAtIndex(targetConfigurationFiles, NETWORK_INTERFACES_PLIST_INDEX);
	if (CFURLGetFileSystemRepresentation(targetNetworkInterfaceFile, TRUE, (UInt8*)targetNetworkInterfaceFileStr, sizeof(targetNetworkInterfaceFileStr)) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateConfiguration: Cannot get file system representation for url: %@"), targetNetworkInterfaceFile);
		goto done;
	}

	targetPreferencesFileString = CFStringCreateWithCString(NULL, targetPreferencesFileStr, kCFStringEncodingUTF8);
	targetNetworkInterfaceFileString = CFStringCreateWithCString(NULL, targetNetworkInterfaceFileStr, kCFStringEncodingUTF8);

	if (targetConfigurationFilesPresent == TRUE) {
		targetPrefs = SCPreferencesCreate(NULL, PLUGIN_ID, targetPreferencesFileString);
		targetNetworkInterfacePrefs = SCPreferencesCreate(NULL, PLUGIN_ID, targetNetworkInterfaceFileString);
		if ((targetPrefs == NULL) || (targetNetworkInterfacePrefs == NULL)) {
			goto done;
		}
	}
	else {
		targetPrefs = __SCNetworkCreateDefaultPref(targetPreferencesFileString);
		targetNetworkInterfacePrefs = __SCNetworkCreateDefaultNIPrefs(targetNetworkInterfaceFileString);
		
		if (targetPrefs == NULL ||
		    targetNetworkInterfacePrefs == NULL) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateConfiguration: Could not create default configuration."));
			goto done;
		}
	}
	validityOptions = CFDictionaryCreateMutable(NULL, 0,
						    &kCFTypeDictionaryKeyCallBacks,
						    &kCFTypeDictionaryValueCallBacks);
	CFDictionaryAddValue(validityOptions, kSCNetworkConfigurationRepair, kCFBooleanTrue);
	
	SCLog(_sc_debug, LOG_NOTICE, CFSTR("sourcePreferenceFileString: %@\n sourceNetworkInterfaceFileString:%@\n targetPreferencesFileString:%@\ntargetNetworkInterfaceFileString:%@"), sourcePreferencesFileString, sourceNetworkInterfaceFileString,
	      targetPreferencesFileString, targetNetworkInterfaceFileString);

	// Setting Bypass Interface to avoid looking at system interfaces
	__SCPreferencesSetLimitSCNetworkConfiguration(sourcePrefs, TRUE);
	__SCPreferencesSetLimitSCNetworkConfiguration(targetPrefs, TRUE);

	// Create services for builtin interfaces at source if they don't exist
	(void)_SCNetworkConfigurationCreateBuiltinInterfaceServices(sourcePrefs, sourceNetworkInterfacePrefs);
	// Checking validity of the source and destination preferences before continuing
	if (_SCNetworkConfigurationCheckValidityUsingPreferences(sourcePrefs,
								 sourceNetworkInterfacePrefs,
								 validityOptions) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateConfiguration: Source configuration is not valid"));
		goto skipServiceMigration;
	}
	// Only call this function if configuration files were not created by default
	if (targetConfigurationFilesPresent == TRUE) {
		// Create services for builtin interfaces at target if they don't exist
		(void)_SCNetworkConfigurationCreateBuiltinInterfaceServices(targetPrefs, targetNetworkInterfacePrefs);
		if (_SCNetworkConfigurationCheckValidityUsingPreferences(targetPrefs,
									 targetNetworkInterfacePrefs,
									 validityOptions) == FALSE) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateConfiguration: Target configuration is not valid"));
			goto skipServiceMigration;
		}
	}

	builtinMapping = _SCNetworkConfigurationCopyBuiltinMapping(sourceNetworkInterfacePrefs, targetNetworkInterfacePrefs);
	externalMapping = _SCNetworkConfigurationCopyExternalInterfaceMapping(sourceNetworkInterfacePrefs, targetNetworkInterfacePrefs);

	/*
	 TODO:
	 Now builtin and external interface mapping is complete, work needs to be done on updating the preferences.plist and NetworkInterface.plist.

	 Also, work needs to be done to check the validity of the data in preferences in source and target destinations, and after migration
	 */

	newTargetNetworkInterfaceEntity = _SCNetworkMigrationCreateNetworkInterfaceArray(targetNetworkInterfacePrefs, externalMapping);
	if (isA_CFArray(newTargetNetworkInterfaceEntity) == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateConfiguration: newTargetNetworkInterfaceEntity is NULL or not of correct type"));
		goto done;
	}
	// Write new interface mapping to NetworkInterfaces.plist
	if (__SCNetworkInterfaceSaveStoredWithPreferences(targetNetworkInterfacePrefs, newTargetNetworkInterfaceEntity) == FALSE)
	{
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateConfiguration: SCNetworkInterfaceSaveStoreWithPreferences failed to update NetworkInterface.plist"));
		goto done;
	}
	// Create BSD Name Mapping to facilitate mapping of services
	bsdNameMapping = _SCNetworkMigrationCreateBSDNameMapping(builtinMapping, externalMapping);

	if (isA_CFDictionary(bsdNameMapping) == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateConfiguration: BSD Name Mapping is NULL"));
		goto done;
	}
	SCLog(_sc_debug, LOG_NOTICE, CFSTR("BSD Name Mapping: %@"), bsdNameMapping);
	serviceMapping = _SCNetworkMigrationCreateServiceMappingUsingBSDMapping(sourcePrefs, targetPrefs, bsdNameMapping);

	if (isA_CFDictionary(serviceMapping) == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateConfiguration: Service Mapping is NULL"));
		goto done;
	}
	SCLog(_sc_debug, LOG_NOTICE, CFSTR("Service Mapping: %@"), serviceMapping);

	setMapping = _SCNetworkMigrationCreateSetMapping(sourcePrefs, targetPrefs);
	sourceServiceSetMapping = _SCNetworkMigrationCreateServiceSetMapping(sourcePrefs);
	
	// Perform the migration of services
	if (_SCNetworkMigrationDoServiceMigration(sourcePrefs, targetPrefs,
						  serviceMapping, bsdNameMapping,
						  setMapping, sourceServiceSetMapping) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateConfiguration: SCNetworkMigrationDoServiceMigration failed to complete successfully"));
		goto done;
	}
	
#if	!TARGET_OS_IPHONE
	// Migrating Virtual Network Interface
	if (_SCNetworkMigrationDoVirtualNetworkInterfaceMigration(sourcePrefs, sourceNetworkInterfacePrefs,
								  targetPrefs, targetNetworkInterfacePrefs,
								  bsdNameMapping, setMapping, sourceServiceSetMapping) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateConfiguration: _SCNetworkMigrationDoVirtualNetworkInterfaceMigration failed to complete successfully"));
	}
#endif
	// Migrate Service Order
	if (_SCNetworkMigrationDoServiceOrderMigration(sourcePrefs, targetPrefs, setMapping) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateConfiguration: _SCNetworkMigrationDoServiceOrderMigration failed to complete successfully"));
	}
	
skipServiceMigration:
	// Migrating System Information
	if (_SCNetworkMigrationDoSystemMigration(sourcePrefs, targetPrefs) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateConfiguration: _SCNetworkMigrationDoSystemMigration failed to complete successfully"));
	}

	if (_SCNetworkConfigurationCheckValidityUsingPreferences(targetPrefs, targetNetworkInterfacePrefs, NULL) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkConfigurationMigrateConfiguration: Migrated configuration is not valid"));
		goto done;
	}
	if (SCPreferencesCommitChanges(targetPrefs) == FALSE) {
		SCLog(TRUE, LOG_ERR, CFSTR("_SCNetworkConfigurationMigrateConfiguration: Error commiting targetPrefs, %s"), SCErrorString(SCError()));
		goto done;
	}

	if (SCPreferencesCommitChanges(targetNetworkInterfacePrefs) == FALSE) {
		SCLog(TRUE, LOG_ERR, CFSTR("_SCNetworkConfigurationMigrateConfiguration: Error commiting targetNetworkInterfacePrefs, %s"), SCErrorString(SCError()));
		goto done;
	}
	migrationSuccess = TRUE;

done:
	if (setMapping != NULL) {
		CFRelease(setMapping);
	}
	if (sourceServiceSetMapping != NULL) {
		CFRelease(sourceServiceSetMapping);
	}
	if (sourceConfigurationFiles != NULL) {
		CFRelease(sourceConfigurationFiles);
	}
	if (targetConfigurationFiles != NULL) {
		CFRelease(targetConfigurationFiles);
	}
	if (sourcePreferencesFileString != NULL) {
		CFRelease(sourcePreferencesFileString);
	}
	if (sourceNetworkInterfaceFileString != NULL) {
		CFRelease(sourceNetworkInterfaceFileString);
	}
	if (targetPreferencesFileString != NULL) {
		CFRelease(targetPreferencesFileString);
	}
	if (targetNetworkInterfaceFileString != NULL) {
		CFRelease(targetNetworkInterfaceFileString);
	}
	if (newTargetNetworkInterfaceEntity != NULL) {
		CFRelease(newTargetNetworkInterfaceEntity);
	}
	if (builtinMapping != NULL) {
		CFRelease(builtinMapping);
	}
	if (externalMapping != NULL) {
		CFRelease(externalMapping);
	}
	if (bsdNameMapping != NULL) {
		CFRelease(bsdNameMapping);
	}
	if (serviceMapping != NULL) {
		CFRelease(serviceMapping);
	}
	if (targetPrefs != NULL) {
		CFRelease(targetPrefs);
	}
	if (sourcePrefs != NULL) {
		CFRelease(sourcePrefs);
	}
	if (sourceNetworkInterfacePrefs != NULL) {
		CFRelease(sourceNetworkInterfacePrefs);
	}
	if (targetNetworkInterfacePrefs != NULL) {
		CFRelease(targetNetworkInterfacePrefs);
	}
	if (validityOptions != NULL) {
		CFRelease(validityOptions);
	}
	return migrationSuccess;
}

#define N_QUICK 64

static Boolean
_SCNetworkMigrationAreServicesIdentical( SCPreferencesRef configPref, SCPreferencesRef expectedConfigPref)
{
	const void * expected_vals_q[N_QUICK];
	const void ** expected_vals = expected_vals_q;
	CFMutableArrayRef expectedServiceArray = NULL;
	CFIndex expectedServiceArrayCount = 0;
	CFDictionaryRef expectedServiceDict = NULL;
	CFIndex expectedServiceDictCount = 0;
	CFDictionaryRef expectedServiceEntity = 0;
	Boolean foundMatch = FALSE;
	CFMutableArrayRef serviceArray = NULL;
	CFIndex serviceArrayCount = 0;
	CFDictionaryRef serviceDict = NULL;
	CFIndex serviceDictCount = 0;
	CFDictionaryRef serviceEntity = NULL;
	Boolean success = FALSE;
	const void * vals_q[N_QUICK];
	const void ** vals = vals_q;

	serviceDict = SCPreferencesGetValue(configPref, kSCPrefNetworkServices);
	if (isA_CFDictionary(serviceDict) == NULL) {
		goto done;
	}
	serviceDictCount = CFDictionaryGetCount(serviceDict);

	expectedServiceDict = SCPreferencesGetValue(expectedConfigPref, kSCPrefNetworkServices);
	if (isA_CFDictionary(expectedServiceDict) == NULL) {
		goto done;
	}
	expectedServiceDictCount = CFDictionaryGetCount(expectedServiceDict);

	if (serviceDictCount != expectedServiceDictCount) {
		goto done;
	}

	if (serviceDictCount > (sizeof(vals_q) / sizeof(CFTypeRef))) {
		vals = CFAllocatorAllocate(NULL, serviceDictCount * sizeof(CFPropertyListRef), 0);
	}

	CFDictionaryGetKeysAndValues(serviceDict, NULL, vals);
	serviceArray = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	for (CFIndex idx=0; idx < serviceDictCount; idx++) {
		serviceEntity = vals[idx];
		if (isA_CFDictionary(serviceEntity) == FALSE) {
			continue;
		}
		CFArrayAppendValue(serviceArray, serviceEntity);
	}

	serviceArrayCount = CFArrayGetCount(serviceArray);

	if (expectedServiceDictCount > (sizeof(expected_vals_q) / sizeof(CFTypeRef))) {
		expected_vals = CFAllocatorAllocate(NULL, expectedServiceDictCount, 0);
	}

	CFDictionaryGetKeysAndValues(expectedServiceDict, NULL, expected_vals);
	expectedServiceArray = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	for (CFIndex idx = 0; idx < expectedServiceDictCount; idx++) {
		serviceEntity = expected_vals[idx];

		if (isA_CFDictionary(serviceEntity) == FALSE) {
			continue;
		}
		CFArrayAppendValue(expectedServiceArray, serviceEntity);
	}
	expectedServiceArrayCount = CFArrayGetCount(expectedServiceArray);

	if (serviceArrayCount != expectedServiceArrayCount) {
		goto done;
	}

	for (CFIndex idx = 0; idx < expectedServiceArrayCount; idx++) {
		foundMatch = FALSE;
		expectedServiceEntity = CFArrayGetValueAtIndex(expectedServiceArray, idx);
		serviceArrayCount = CFArrayGetCount(serviceArray);

		for (CFIndex idx2 = 0; idx2 < serviceArrayCount; idx2++) {
			serviceEntity = CFArrayGetValueAtIndex(serviceArray, idx2);

			if (CFEqual(expectedServiceEntity, serviceEntity) == TRUE) {
				foundMatch = TRUE;
				break;
			}
		}

		if (foundMatch == FALSE) {
			break;
		}
	}

	success = foundMatch;
done:
	if (vals != vals_q) {
		CFAllocatorDeallocate(NULL, vals);
	}
	if (expected_vals != expected_vals_q) {
		CFAllocatorDeallocate(NULL, expected_vals);
	}
	return success;
}

static Boolean
_SCNetworkMigrationAreNetworkInterfaceConfigurationsIdentical (SCPreferencesRef configNetworkInterfacePref, SCPreferencesRef expectedNetworkInterfacePref)
{
	CFDictionaryRef expectedInterfaceEntity = NULL;
	CFArrayRef expectedInterfaceList = NULL;
	CFIndex expectedInterfaceListCount;
	Boolean foundMatch = FALSE;
	CFDictionaryRef interfaceEntity = NULL;
	CFArrayRef interfaceList = NULL;
	CFIndex interfaceListCount;
	CFMutableArrayRef interfaceListMutable = NULL;
	Boolean success = FALSE;

	interfaceList = SCPreferencesGetValue(configNetworkInterfacePref, INTERFACES);
	if (isA_CFArray(interfaceList) == NULL) {
		goto done;
	}
	interfaceListMutable = CFArrayCreateMutableCopy(NULL, 0, interfaceList);
	interfaceListCount = CFArrayGetCount(interfaceListMutable);

	expectedInterfaceList = SCPreferencesGetValue(expectedNetworkInterfacePref, INTERFACES);
	if (isA_CFArray(expectedInterfaceList) == NULL) {
		goto done;
	}
	expectedInterfaceListCount = CFArrayGetCount(expectedInterfaceList);

	if (interfaceListCount != expectedInterfaceListCount) {
		goto done;
	}

	for (CFIndex idx = 0; idx < expectedInterfaceListCount; idx++) {
		foundMatch = FALSE;
		expectedInterfaceEntity = CFArrayGetValueAtIndex(expectedInterfaceList, idx);
		interfaceListCount = CFArrayGetCount(interfaceListMutable);

		for (CFIndex idx2 = 0; idx2 < interfaceListCount; idx2++) {
			interfaceEntity = CFArrayGetValueAtIndex(interfaceList, idx2);
			if (CFEqual(expectedInterfaceEntity, interfaceEntity) == TRUE) {
				foundMatch = TRUE;
				break;
			}
		}
		if (foundMatch == FALSE) {
			break;
		}
	}
	success = foundMatch;

done:
	if (interfaceListMutable != NULL) {
		CFRelease(interfaceListMutable);
	}

	return success;
}

Boolean
_SCNetworkMigrationAreConfigurationsIdentical (CFURLRef configurationURL,
					       CFURLRef expectedConfigurationURL)
{
	CFURLRef baseConfigURL = NULL;
	CFURLRef baseExpectedConfigURL = NULL;
	CFURLRef configPreferencesURL = NULL;
	CFURLRef configNetworkInterfacesURL = NULL;
	SCPreferencesRef configPref = NULL;
	SCPreferencesRef configNetworkInterfacePref = NULL;
	SCPreferencesRef expectedConfigPref = NULL;
	SCPreferencesRef expectedNetworkInterfacePref = NULL;
	CFURLRef expectedNetworkInterfaceURL = NULL;
	CFURLRef expectedPreferencesURL = NULL;
	Boolean isIdentical = FALSE;
	CFStringRef networkInterfaceConfigString = NULL;
	CFStringRef networkInterfaceExpectedString = NULL;
	CFStringRef prefsConfigString = NULL;
	CFStringRef prefsExpectedString = NULL;
	char networkInterfaceConfigStr[PATH_MAX];
	char networkInterfaceExpectedStr[PATH_MAX];
	char prefsConfigStr[PATH_MAX];
	char prefsExpectedStr[PATH_MAX];
	
	if (configurationURL == NULL ||
	    expectedConfigurationURL == NULL) {
		return FALSE;
	}
	baseConfigURL = CFURLCreateWithFileSystemPathRelativeToBase(NULL, PREFS_DEFAULT_DIR_RELATIVE, kCFURLPOSIXPathStyle, TRUE, configurationURL);
	configPreferencesURL = CFURLCreateFromFileSystemRepresentationRelativeToBase(NULL, (const UInt8*) PREFS_DEFAULT_CONFIG_PLIST, sizeof(PREFS_DEFAULT_CONFIG_PLIST), FALSE, baseConfigURL);
        
        if (CFURLResourceIsReachable(configPreferencesURL, NULL) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationAreConfigurationsIdentical: preferences.plist file is not present."));
		goto done;
        }
	
	configNetworkInterfacesURL = CFURLCreateFromFileSystemRepresentationRelativeToBase(NULL, (const UInt8*)NETWORK_INTERFACES_PREFS_PLIST, sizeof(NETWORK_INTERFACES_PREFS_PLIST), FALSE, baseConfigURL);
	
	if (CFURLResourceIsReachable(configNetworkInterfacesURL, NULL) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationAreConfigurationsIdentical: NetworkInterfaces.plist file is not present."));
		goto done;
	}
	
	if (CFURLGetFileSystemRepresentation(configPreferencesURL, TRUE, (UInt8*)prefsConfigStr, sizeof(prefsConfigStr)) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationAreConfigurationsIdentical: Could not extract preferences information"));
		goto done;
	}
	if (CFURLGetFileSystemRepresentation(configNetworkInterfacesURL, TRUE, (UInt8*)networkInterfaceConfigStr, sizeof(networkInterfaceConfigStr)) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationAreConfigurationsIdentical: Could not extract network interfaces information"));
		goto done;
	}
	
	baseExpectedConfigURL = CFURLCreateWithFileSystemPathRelativeToBase(NULL, PREFS_DEFAULT_DIR_RELATIVE, kCFURLPOSIXPathStyle, TRUE, expectedConfigurationURL);
	expectedPreferencesURL = CFURLCreateFromFileSystemRepresentationRelativeToBase(NULL, (const UInt8*)PREFS_DEFAULT_CONFIG_PLIST, sizeof(PREFS_DEFAULT_CONFIG_PLIST), FALSE, baseExpectedConfigURL);
	
	if (CFURLResourceIsReachable(expectedPreferencesURL, NULL) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationAreConfigurationsIdentical: preferences.plist file is not present in the expected configuration."));
		goto done;
	}
	
	expectedNetworkInterfaceURL = CFURLCreateFromFileSystemRepresentationRelativeToBase(NULL, (const UInt8*)NETWORK_INTERFACES_PREFS_PLIST, sizeof(NETWORK_INTERFACES_PREFS_PLIST), FALSE, baseExpectedConfigURL);
	
	if (CFURLResourceIsReachable(expectedNetworkInterfaceURL, NULL) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationAreConfigurationsIdentical: NetworkInterfaces.plist file is not present in the expected configuration."));
		goto done;
	}
	
	if (CFURLGetFileSystemRepresentation(expectedPreferencesURL, TRUE, (UInt8*)prefsExpectedStr, sizeof(prefsExpectedStr)) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationAreConfigurationsIdentical: Could not extract preferences information"));
		goto done;
	}
	if (CFURLGetFileSystemRepresentation(expectedNetworkInterfaceURL, TRUE, (UInt8*)networkInterfaceExpectedStr, sizeof(networkInterfaceExpectedStr)) == FALSE) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationAreConfigurationsIdentical: Could not extract network interfaces information"));
		goto done;
	}
	
	prefsConfigString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%s"), prefsConfigStr);
	networkInterfaceConfigString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%s"), networkInterfaceConfigStr);
	prefsExpectedString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%s"), prefsExpectedStr);
	networkInterfaceExpectedString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%s"), networkInterfaceExpectedStr);
	
	configPref = SCPreferencesCreate(NULL, PLUGIN_ID, prefsConfigString);
	expectedConfigPref = SCPreferencesCreate(NULL, PLUGIN_ID, prefsExpectedString);
	configNetworkInterfacePref = SCPreferencesCreate(NULL, PLUGIN_ID, networkInterfaceConfigString);
	expectedNetworkInterfacePref = SCPreferencesCreate(NULL, PLUGIN_ID, networkInterfaceExpectedString);
done:
	if (configPref == NULL ||
	    expectedConfigPref == NULL ||
	    configNetworkInterfacePref == NULL ||
	    expectedNetworkInterfacePref == NULL) {
		SCLog(_sc_debug, LOG_NOTICE, CFSTR("_SCNetworkMigrationAreConfigurationsIdentical: One of the preferences is NULL"));
		isIdentical = FALSE;
	}
	else {
		isIdentical = (_SCNetworkMigrationAreServicesIdentical(configPref, expectedConfigPref) &&
			       _SCNetworkMigrationAreNetworkInterfaceConfigurationsIdentical(configNetworkInterfacePref, expectedNetworkInterfacePref));
	}
	if (baseConfigURL != NULL) {
		CFRelease(baseConfigURL);
	}
	if (configPreferencesURL != NULL) {
		CFRelease(configPreferencesURL);
	}
	if (configNetworkInterfacesURL != NULL) {
		CFRelease(configNetworkInterfacesURL);
	}
	if (baseExpectedConfigURL != NULL) {
		CFRelease(baseExpectedConfigURL);
	}
	if (expectedPreferencesURL != NULL) {
		CFRelease(expectedPreferencesURL);
	}
	if (expectedNetworkInterfaceURL != NULL) {
		CFRelease(expectedNetworkInterfaceURL);
	}
	if (prefsConfigString != NULL) {
		CFRelease(prefsConfigString);
	}
	if (networkInterfaceConfigString != NULL) {
		CFRelease(networkInterfaceConfigString);
	}
	if (prefsExpectedString != NULL) {
		CFRelease(prefsExpectedString);
	}
	if (networkInterfaceExpectedString != NULL) {
		CFRelease(networkInterfaceExpectedString);
	}
	if (configPref != NULL) {
		CFRelease(configPref);
	}
	if (expectedConfigPref != NULL) {
		CFRelease(expectedConfigPref);
	}
	if (configNetworkInterfacePref != NULL) {
		CFRelease(configNetworkInterfacePref);
	}
	if (expectedNetworkInterfacePref != NULL) {
		CFRelease(expectedNetworkInterfacePref);
	}
	return isIdentical;
}

CFArrayRef
_SCNetworkConfigurationCopyMigrationRemovePaths	(CFArrayRef	targetPaths,
                                                 CFURLRef	targetDir)
{
	CFURLRef affectedURL;
	char filePath[PATH_MAX];
	CFURLRef targetFile;
	CFMutableArrayRef toBeRemoved = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	
	for (CFIndex idx = 0; idx < CFArrayGetCount(targetPaths); idx++) {
		affectedURL = CFArrayGetValueAtIndex(targetPaths, idx);
		
		if (CFURLGetFileSystemRepresentation(affectedURL, TRUE, (UInt8*)filePath, sizeof(filePath)) == FALSE) {
			SCLog(_sc_debug, LOG_NOTICE, CFSTR("filesToBeRemoved: Could not get file system representation"));
			continue;
		}
		targetFile = CFURLCreateFromFileSystemRepresentationRelativeToBase(NULL, (const UInt8*)filePath,
		strnlen(filePath, sizeof(filePath)), FALSE, targetDir);
		
		if (CFURLResourceIsReachable(targetFile, NULL) == FALSE) {
			CFArrayAppendValue(toBeRemoved, affectedURL);
		}
		CFRelease(targetFile);
	}
	// If number of files to be removed is 0, return NULL
	if (CFArrayGetCount(toBeRemoved) == 0) {
		CFRelease(toBeRemoved);
		toBeRemoved = NULL;
	}
	return toBeRemoved;
}
