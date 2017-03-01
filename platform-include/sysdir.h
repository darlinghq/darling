/*
 * Copyright (c) 1999-2015 Apple Inc. All rights reserved.
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

#ifndef __SYSTEM_DIRECTORIES_H__
#define __SYSTEM_DIRECTORIES_H__

#include <os/base.h>
#include <Availability.h>

// Available OSX 10.12, iOS 10.0, WatchOS 3.0 and TVOS 10.0. Not all enum identifiers return a useful path on all platforms.
OS_ENUM(sysdir_search_path_directory, unsigned int,
    SYSDIR_DIRECTORY_APPLICATION            = 1,    // supported applications (Applications)
    SYSDIR_DIRECTORY_DEMO_APPLICATION       = 2,    // unsupported applications, demonstration versions (Applications/Demos)
    SYSDIR_DIRECTORY_DEVELOPER_APPLICATION  = 3,    // developer applications (Developer/Applications) Soft deprecated as of __MAC_10_5 - there is no one single Developer directory
    SYSDIR_DIRECTORY_ADMIN_APPLICATION      = 4,    // system and network administration applications (Applications/Utilities)
    SYSDIR_DIRECTORY_LIBRARY                = 5,    // various user-visible documentation, support, and configuration files, resources (Library)
    SYSDIR_DIRECTORY_DEVELOPER              = 6,    // developer resources (Developer) Soft deprecated as of __MAC_10_5 - there is no one single Developer directory
    SYSDIR_DIRECTORY_USER                   = 7,    // user home directories (Users)
    SYSDIR_DIRECTORY_DOCUMENTATION          = 8,    // documentation (Library/Documentation)
    SYSDIR_DIRECTORY_DOCUMENT               = 9,    // documents (Documents)
    SYSDIR_DIRECTORY_CORESERVICE            = 10,   // location of core services (Library/CoreServices)
    SYSDIR_DIRECTORY_AUTOSAVED_INFORMATION  = 11,   // location of user's directory for use with autosaving (Library/Autosave Information)
    SYSDIR_DIRECTORY_DESKTOP                = 12,   // location of user's Desktop (Desktop)
    SYSDIR_DIRECTORY_CACHES                 = 13,   // location of discardable cache files (Library/Caches)
    SYSDIR_DIRECTORY_APPLICATION_SUPPORT    = 14,   // location of application support files (plug-ins, etc) (Library/Application Support)
    SYSDIR_DIRECTORY_DOWNLOADS              = 15,   // location of user's Downloads directory (Downloads)
    SYSDIR_DIRECTORY_INPUT_METHODS          = 16,   // input methods (Library/Input Methods)
    SYSDIR_DIRECTORY_MOVIES                 = 17,   // location of user's Movies directory (Movies)
    SYSDIR_DIRECTORY_MUSIC                  = 18,   // location of user's Music directory (Music)
    SYSDIR_DIRECTORY_PICTURES               = 19,   // location of user's Pictures directory (Pictures)
    SYSDIR_DIRECTORY_PRINTER_DESCRIPTION    = 20,   // location of system's PPDs directory (Library/Printers/PPDs)
    SYSDIR_DIRECTORY_SHARED_PUBLIC          = 21,   // location of user's Public sharing directory (Public)
    SYSDIR_DIRECTORY_PREFERENCE_PANES       = 22,   // location of the PreferencePanes directory for use with System Preferences (Library/PreferencePanes)
    SYSDIR_DIRECTORY_ALL_APPLICATIONS       = 100,  // all directories where applications can occur (Applications, Applications/Utilities, Developer/Applications, ...)
    SYSDIR_DIRECTORY_ALL_LIBRARIES          = 101,  // all directories where resources can occur (Library, Developer)
);

// Available OSX 10.12, iOS 10.0, WatchOS 3.0 and TVOS 10.0. Not all enum identifiers are useful on all platforms.
OS_ENUM(sysdir_search_path_domain_mask, unsigned int,
    SYSDIR_DOMAIN_MASK_USER                 = ( 1UL << 0 ), // user's home directory --- place to install user's personal items (~)
    SYSDIR_DOMAIN_MASK_LOCAL                = ( 1UL << 1 ), // local to the current machine --- place to install items available to everyone on this machine
    SYSDIR_DOMAIN_MASK_NETWORK              = ( 1UL << 2 ), // publically available location in the local area network --- place to install items available on the network (/Network)
    SYSDIR_DOMAIN_MASK_SYSTEM               = ( 1UL << 3 ), // provided by Apple
    SYSDIR_DOMAIN_MASK_ALL                  = 0x0ffff,      // all domains: all of the above and more, future items
);

typedef unsigned int sysdir_search_path_enumeration_state;

__BEGIN_DECLS

extern sysdir_search_path_enumeration_state
sysdir_start_search_path_enumeration(sysdir_search_path_directory_t dir, sysdir_search_path_domain_mask_t domainMask) __API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0));

extern sysdir_search_path_enumeration_state
sysdir_get_next_search_path_enumeration(sysdir_search_path_enumeration_state state, char *path) __API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0));

__END_DECLS

#endif /* defined(__SYSTEM_DIRECTORIES_H__) */
