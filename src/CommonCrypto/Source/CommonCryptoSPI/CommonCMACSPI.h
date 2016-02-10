/* 
 * Copyright (c) 2011 Apple Computer, Inc. All Rights Reserved.
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


#ifndef	_CC_CMACSPI_H_
#define _CC_CMACSPI_H_

#include <Availability.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CC_CMACAES_DIGEST_LENGTH     16          /* digest length in bytes */

/*!
@function   CCAESCmac
@abstract   Stateless, one-shot AES CMAC function
     
@param      key         Raw key bytes.
@param      data        The data to digest. 
@param      dataLength  The length of the data to digest. 
@param      macOut      The digest bytes (space provided by the caller). 
    
Output is written to caller-supplied buffer.
*/
    
void
    CCAESCmac(const void *key, const uint8_t *data, size_t dataLength, void *macOut)
__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

#ifdef __cplusplus
}
#endif

#endif /* _CC_CMACSPI_H_ */
