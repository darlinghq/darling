/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 *
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _MEMBERSHIPPRIV_H_
#define _MEMBERSHIPPRIV_H_

#include <uuid/uuid.h>
#include <ntsid.h>

#define MBR_UU_STRING_SIZE 37
#define MBR_MAX_SID_STRING_SIZE 200

#define SID_TYPE_USER 0
#define SID_TYPE_GROUP 1

#define MBR_REC_TYPE_USER 1
#define MBR_REC_TYPE_GROUP 2

/* only supported by mbr_identifier_translate for target type */
#define ID_TYPE_UID_OR_GID 30
#define ID_TYPE_NAME 31
#define ID_TYPE_WINDOWS_FQN 32

__BEGIN_DECLS

int mbr_reset_cache();
int mbr_user_name_to_uuid(const char *name, uuid_t uu);
int mbr_group_name_to_uuid(const char *name, uuid_t uu);
int mbr_check_membership_by_id(uuid_t user, gid_t group, int *ismember);
int mbr_check_membership_refresh(const uuid_t user, uuid_t group, int *ismember);

/* mbr_uuid_to_string should use uuid_unparse from uuid.h */
int mbr_uuid_to_string(const uuid_t uu, char *string) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_8, __IPHONE_NA, __IPHONE_NA);

/* mbr_string_to_uuid should use uuid_parse from uuid.h */
int mbr_string_to_uuid(const char *string, uuid_t uu) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_8, __IPHONE_NA, __IPHONE_NA);

int mbr_uuid_to_sid_type(const uuid_t uu, nt_sid_t *sid, int *id_type);
int mbr_set_identifier_ttl(int id_type, const void *identifier, size_t identifier_size, unsigned int seconds);

/* new SPI to allow translation from any-to-any type, pass ID_TYPE_UID_OR_GID when translating to a UID */
int mbr_identifier_translate(int id_type, const void *identifier, size_t identifier_size, int target_type, void **result, int *rec_type);

/* 
 * groupid_type does not support ID_TYPE_GSS_EXPORT_NAME
 */
int mbr_check_membership_ext(int userid_type, const void *userid, size_t userid_size, int groupid_type, const void *groupid, int refresh, int *isMember);

__END_DECLS

#endif /* !_MEMBERSHIPPRIV_H_ */
