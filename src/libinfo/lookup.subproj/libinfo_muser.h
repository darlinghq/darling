/*
 * Copyright (c) 2015 Apple Inc.  All rights reserved.
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

#ifndef _LIBINFO_MUSER_H_
#define _LIBINFO_MUSER_H_

/*
 * SPI defining the interface between the libinfo "muser" module and
 * the process providing the service.
 */

#define kLibinfoMultiuserPortName "com.apple.system.libinfo.muser"

/* Request type: one of the kLIMMessageRequest* defines below */
#define kLIMMessageReqtype "reqtype"
/* Available: query whether the muser system is active */
#define kLIMMessageAvailable "available"
/* Query: request dependent query item */
#define kLIMMessageQuery "query"
/* Name: the name of the calling libinfo function */
#define kLIMMessageRPCName "procname"
/* Version: the API version of this request */
#define kLIMMessageVersion "version"

/* 
 * Request a passwd structure for a given username, query type should be
 * an string value with the requested username.
 */
#define kLIMMessageRequestUsername "username"
#define kLIMMessageRequestUID "uid"
#define kLIMMessageRequestGroupname "groupname"
#define kLIMMessageRequestGID "gid"
#define kLIMMessageRequestGrouplist "grouplist"

/* Reply keys from the availability query. */
#define kLIMMessageReplyAvailable "available" // bool

/* Reply keys from user queries. */
#define kLIMMessageReplyName "pw_name" // string
#define kLIMMessageReplyPasswd "pw_passwd" // string
#define kLIMMessageReplyUID "pw_uid" // int64
#define kLIMMessageReplyGID "pw_gid" // int64
#define kLIMMessageReplyHome "pw_dir" // string
#define kLIMMessageReplyShell "pw_shell" // string

/* Reply keys from group queries. */
#define kLIMMessageReplyGroupname "gr_name" // string
#define kLIMMessageReplyGroupID "gr_gid" // int64
#define kLIMMessageReplyGroupMembers "gr_members" // array of strings

/* Reply keys from grouplist queries. */
#define kLIMMessageReplyGrouplist "grouplist" // array of int64

#endif /* _LIBINFO_MUSER_H_ */
