/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.1 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/* 
 * Printer database lookup routines
 * Copyright (c) 1989 by NeXT, Inc. 
 */

#ifndef _PRDB_H_
#define _PRDB_H_

typedef struct prdb_property {
	char *pp_key;
	char *pp_value;
} prdb_property;

typedef struct prdb_ent {
	char **pe_name;
	unsigned pe_nprops;
	prdb_property *pe_prop;
} prdb_ent;

#include <sys/cdefs.h>

__BEGIN_DECLS

void prdb_set __P((const char *));
const prdb_ent *prdb_get __P((void));
const prdb_ent *prdb_getbyname __P((const char *));
void prdb_end __P((void));

__END_DECLS

#endif /* !_PRDB_H_ */
