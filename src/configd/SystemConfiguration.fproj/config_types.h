/*
 * Copyright (c) 2000, 2001, 2003, 2005, 2007, 2013 Apple Inc. All rights reserved.
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

#ifndef _CONFIG_TYPES_H
#define _CONFIG_TYPES_H

/*
 * Keep IPC functions private to the framework
 */
#ifdef mig_external
#undef mig_external
#endif
#define mig_external __private_extern__

/* Turn MIG type checking on by default */
#ifdef __MigTypeCheck
#undef __MigTypeCheck
#endif
#define __MigTypeCheck	1

/*
 * Mach server port name
 */
#if	!TARGET_IPHONE_SIMULATOR
#define SCD_SERVER	"com.apple.SystemConfiguration.configd"
#else	// !TARGET_IPHONE_SIMULATOR
#define SCD_SERVER_HOST	"com.apple.SystemConfiguration.configd"
#define SCD_SERVER	"com.apple.SystemConfiguration.configd_sim"
#endif	// !TARGET_IPHONE_SIMULATOR

/*
 * Input arguments: serialized key's, list delimiters, ...
 *	(sent as out-of-line data in a message)
 */
typedef const char * xmlData_t;

/* Output arguments: serialized data, lists, ...
 *	(sent as out-of-line data in a message)
 */
typedef char * xmlDataOut_t;

#endif /* !_CONFIG_TYPES_H */
