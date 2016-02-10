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

#ifndef _MEMBERSHIP_H_
#define _MEMBERSHIP_H_

#include <uuid/uuid.h>
#include <ntsid.h>

/*!
	@defined    ID_TYPE_UID
	@abstract   is of type uid_t
	@discussion is of type uid_t
*/
#define ID_TYPE_UID				0

/*!
	@defined    ID_TYPE_GID
	@abstract   is of type gid_t
	@discussion is of type gid_t
*/
#define ID_TYPE_GID				1

/*!
    @defined    ID_TYPE_SID
    @abstract   is of type ntsid_t
    @discussion is of type ntsid_t
*/
#define ID_TYPE_SID				3

/*!
    @defined    ID_TYPE_USERNAME
    @abstract   is a NULL terminated UTF8 string
    @discussion is a NULL terminated UTF8 string
*/
#define ID_TYPE_USERNAME		4

/*!
    @defined    ID_TYPE_GROUPNAME
    @abstract   is a NULL terminated UTF8 string
    @discussion is a NULL terminated UTF8 string
*/
#define ID_TYPE_GROUPNAME		5

/*!
	@defined	ID_TYPE_UUID
	@abstract	is of type uuid_t
	@discussion	is of type uuid_t
*/
#define ID_TYPE_UUID			6

/*!
    @defined    ID_TYPE_GROUP_NFS
    @abstract   is a NULL terminated UTF8 string
    @discussion is a NULL terminated UTF8 string
*/
#define ID_TYPE_GROUP_NFS	7

/*!
    @defined    ID_TYPE_USER_NFS
    @abstract   is a NULL terminated UTF8 string
    @discussion is a NULL terminated UTF8 string
*/
#define ID_TYPE_USER_NFS		8

/*!
	@defined    ID_TYPE_GSS_EXPORT_NAME
	@abstract	is a gss exported name
	@discussion	is the data in gss_buffer_t as returned from gss_export_name.
*/
#define ID_TYPE_GSS_EXPORT_NAME	10

/*!
	@defined    ID_TYPE_X509_DN
	@abstract	is a NULL terminated string representation of the X.509 certificate identity
	@discussion	is a NULL terminated string with the format of:
 
				<I>DN of the Certificate authority<S>DN of the holder
 
				Example:
 
				<I>DC=com,DC=example,CN=CertificatAuthority<S>DC=com,DC=example,CN=username
*/
#define ID_TYPE_X509_DN			11

/*!
	@defined    ID_TYPE_KERBEROS
	@abstract	is a NULL terminated string representation of a Kerberos principal
	@discussion	is a NULL terminated string in the form of user\@REALM representing a typical 
				Kerberos principal.
*/
#define ID_TYPE_KERBEROS		12

__BEGIN_DECLS

/*!
	@function   mbr_uid_to_uuid
	@abstract	convert a UID to a corresponding UUID
	@discussion	will convert a UID of a user to a corresponding UUID value.
				This call will always succeed and may return a synthesized
				UUID with the prefix FFFFEEEE-DDDD-CCCC-BBBB-AAAAxxxxxxxx,
				where 'xxxxxxxx' is a hex conversion of the UID. The returned
				UUID can be used for any operation including ACL and SACL
				memberships, even if a UUID is later assigned to the user
				record.
	@param		uid the uid_t to be converted
	@param		uu is the UUID found for the provided UID
	@result		returns 0 on success or appropriate errno code.
*/
int mbr_uid_to_uuid(uid_t uid, uuid_t uu);

/*!
	@function   mbr_gid_to_uuid
	@abstract   convert a GID to a corresponding UUID
	@discussion will convert a GID of a group to a corresponding UUID value.
				This call will always succeed and may return a synthesized
				UUID with the prefix AAAABBBB-CCCC-DDDD-EEEE-FFFFxxxxxxxx,
				where 'xxxxxxxx' is a hex conversion of the UID. The returned
				UUID can be used for any operation including ACL and SACL
				memberships, even if a UUID is later assigned to the group
				record.
	@param		gid the gid_t to be converted
	@param		uu is the UUID found for the provided GID
	@result		returns 0 on success or appropriate errno code.
*/
int mbr_gid_to_uuid(gid_t gid, uuid_t uu);

/*!
	@function   mbr_sid_to_uuid
	@abstract	convert a SID to a corresponding UUID
	@discussion will convert a SID to a corresponding UUID value. This call
				can fail for records that do not have a valid SID or RID.
	@param		sid the nt_sid_t to be converted
	@param		uu is the UUID found for the provided GID
	@result		returns 0 on success or appropriate errno code.
*/
int mbr_sid_to_uuid(const nt_sid_t* sid, uuid_t uu);

/*!
	@function	mbr_identifier_to_uuid
	@abstract	resolves various identifiers to corresponding UUID
	@discussion	will resolve various identifiers such as X.509 Distinguished 
				Names, Kerberos ID or other forms of security identifiers to a
				corresponding UUID.
	@param		id_type is one of the defined types
	@param		identifier is a generic pointer as defined by the type
	@param		identifier_size is the size of the data pointed to in identifier
	@param		uu is the UUID found for the identifier
	@result		returns 0 on success or appropriate errno code.
*/
int mbr_identifier_to_uuid(int id_type, const void *identifier, size_t identifier_size,
						   uuid_t uu);

/*!
	@function	mbr_uuid_to_id
	@abstract	resolves a UUID to a corresponding ID and type
	@discussion	will resolve a UUID to a corresponding GID or UID and return
				the type of ID (ID_TYPE_UID or ID_TYPE_GID).  Synthesized
				UUID values will be directly translated to corresponding ID.
				A UID will always be returned even if the UUID is not found.  
				The returned ID is not persistant, but can be used to map back
				to the UUID during runtime.
	@param		uu is the UUID to be resolved
	@param		uid_or_gid is the UID or GID found for the UUID
	@param		id_type is the type of ID
	@result		returns 0 on success or appropriate errno code.
*/
int mbr_uuid_to_id(const uuid_t uu, id_t* uid_or_gid, int* id_type);

/*!
	@function	mbr_uuid_to_sid
	@abstract	resolves a UUID to a corresponding SID
	@discussion will resolve a UUID to a corresponding SID.
	@param		uu is the UUID to be resolved
	@param		sid is the SID found for the UUID
	@result		returns 0 on success or appropriate errno code.
*/
int mbr_uuid_to_sid(const uuid_t uu, nt_sid_t* sid);

/*!
	@function	mbr_sid_to_string
	@abstract	convert a SID to a corresponding character string representation
	@discussion for use in situations where an external representation of a SID is required.
	@param		sid is the SID to be converted
	@param		string is a buffer that will be filled in with a nul-terminated string
				representation of the SID.  The buffer must be at least 194 characters in length.
	@result    returns 0 on success or appropriate errno code.
 */
int mbr_sid_to_string(const nt_sid_t *sid, char *string);

/*!
	@function	mbr_string_to_sid
	@abstract	convert a character string representation of a sid to an nt_sid_t value
	@discussion for converting an external representation of a sid.
	@param		string is a buffer containing a nul-terminated string representation of a SID
	@param		sid is the target of the conversion
	@result    returns 0 on success or appropriate errno code.
 */
int mbr_string_to_sid(const char *string, nt_sid_t *sid);

/*!
	@function	mbr_check_membership
	@abstract	checks if a user is a member of a group
	@discussion	will check if a user is a member of a group either through
				direct membership or via nested group membership.
	@param		user is the UUID of the user in question
	@param		group is the UUID of the group to be checked
	@param		ismember is set to 1 if user is a member of the group,
				otherwise 0 is returned
	@result		returns 0 on success or appropriate errno code.
*/
int mbr_check_membership(const uuid_t user, const uuid_t group, int* ismember);

/*!
	@function	mbr_check_service_membership
	@abstract   checks if a user is part of a service group
	@discussion will check if a user is a member of a service access group.
				The servicename provided will be automatically prefixed with
				"com.apple.access_" (e.g., "afp" becomes "com.apple.access_afp").
				In addition a special service group "com.apple.access_all_services"
				will be checked in addition to the specific service.
	@param		user is the UUID of the user in question
	@param		servicename is the service type (e.g., "afp", "ftp", etc.)
	@param		ismember is set to 1 if user is a member of the group,
				otherwise 0 is returned
	@result		returns 0 on success or appropriate errno code.
*/
int mbr_check_service_membership(const uuid_t user, const char *servicename, 
								 int *ismember);

__END_DECLS

#endif /* !_MEMBERSHIP_H_ */
