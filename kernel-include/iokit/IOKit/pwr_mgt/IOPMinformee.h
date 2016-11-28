/*
 * Copyright (c) 1998-2007 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
#ifndef _IOKIT_IOPMINFORMEE_H
#define _IOKIT_IOPMINFORMEE_H

#include <IOKit/IOService.h>
#include <IOKit/IOReturn.h>

class IOPMinformee : public OSObject
{
    OSDeclareDefaultStructors(IOPMinformee)
    friend class IOPMinformeeList;
    
public:
    static IOPMinformee * withObject( IOService * theObject );

    void initialize( IOService * theObject );

    void free( void ) APPLE_KEXT_OVERRIDE;

public:
    IOService *     whatObject;     // interested driver  
    int32_t         timer;          // -1, 0, or positive number of ticks
    IOPMinformee *  nextInList;     // linkage pointer
    AbsoluteTime    startTime;      // start time of last inform
    bool            active;         // enable flag
};

#endif /* !_IOKIT_IOPMINFORMEE_H */
