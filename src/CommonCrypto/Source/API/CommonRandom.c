/*
 * Copyright (c) 2006-2010 Apple Inc. All Rights Reserved.
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
 * CommonRandom.c - Access to PRNGs
 *
 * this code was modified from the original embedded SecRandomCopyBytes.
 */


#include "CommonRandomSPI.h"
#include "CommonCryptor.h"
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <dispatch/dispatch.h>



/* Default random ref for /dev/random. */
CCRandomRef kCCRandomDefault = (CCRandomRef) NULL;

/* File descriptor for "/dev/random". */
static int kCCRandomFD;

int CCRandomCopyBytes(CCRandomRef rnd, void *bytes, size_t count) {
    static dispatch_once_t randopen;
    
    if (rnd != kCCRandomDefault) return kCCParamError;
    
    dispatch_once(&randopen, ^{ 
        kCCRandomFD = open("/dev/random", O_RDONLY); 
        if(kCCRandomFD > -1) fcntl(kCCRandomFD, F_SETFD, fcntl(kCCRandomFD, F_GETFD, 0) | FD_CLOEXEC);
    });
    
    if (kCCRandomFD < 0) return -1;
    
    while (count) {
        ssize_t bytes_read = read(kCCRandomFD, bytes, count);
 
        if (bytes_read == -1) {
            if (errno == EINTR) continue;
            return -1;
        }
        
        if (bytes_read == 0) return -1;

        bytes += bytes_read;
        count -= bytes_read;
    }
	return kCCSuccess;
}

