/* 
 * Copyright (c) 2004 Apple Computer, Inc. All Rights Reserved.
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

/*
 *  ccdebug.c - CommonCrypto debug macros
 *  MacTomCrypt
 *
 */

#include "ccdebug.h"
#include <stdlib.h>
#include <asl.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

static char *std_log_prefix = "###CommonCrypto Function: %s - %s";
static const char *std_ident = "CommonCrypto";
static const char *std_facility = "CipherSuite";
static uint32_t	std_options = 0;

static aslclient aslhandle = NULL;
static aslmsg msgptr = NULL;

static void
ccdebug_init() {
	char *ccEnvStdErr = getenv("CC_STDERR");
	
	if(ccEnvStdErr != NULL && strncmp(ccEnvStdErr, "yes", 3) == 0) std_options |= ASL_OPT_STDERR;
	aslhandle = asl_open(std_ident, std_facility, std_options);

	msgptr = asl_new(ASL_TYPE_MSG);
	asl_set(msgptr, ASL_KEY_FACILITY, "com.apple.infosec");
}
	
	
void
ccdebug_imp(int level, char *funcname, char *format, ...) {
	va_list argp;
	char fmtbuffer[256];

	if(aslhandle == NULL) ccdebug_init();
	
	sprintf(fmtbuffer, std_log_prefix, funcname, format);
	va_start(argp, format);
	asl_vlog(aslhandle, msgptr, level, fmtbuffer, argp);
	va_end(argp);
}
