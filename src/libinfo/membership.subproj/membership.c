/*
 * Copyright (c) 2004-2010 Apple Inc. All rights reserved.
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

#include <stdlib.h>
#include <sys/errno.h>
#include <mach/mach.h>
#include "membership.h"
#include "membershipPriv.h"
#include <servers/bootstrap.h>
#include <libkern/OSByteOrder.h>
#ifdef DS_AVAILABLE
#include <xpc/xpc.h>
#include <xpc/private.h>
#include <opendirectory/odipc.h>
#include <pthread.h>
#include <mach-o/dyld_priv.h>
#endif

static const uuid_t _user_compat_prefix = {0xff, 0xff, 0xee, 0xee, 0xdd, 0xdd, 0xcc, 0xcc, 0xbb, 0xbb, 0xaa, 0xaa, 0x00, 0x00, 0x00, 0x00};
static const uuid_t _group_compat_prefix = {0xab, 0xcd, 0xef, 0xab, 0xcd, 0xef, 0xab, 0xcd, 0xef, 0xab, 0xcd, 0xef, 0x00, 0x00, 0x00, 0x00};

#define COMPAT_PREFIX_LEN	(sizeof(uuid_t) - sizeof(id_t))

#ifdef DS_AVAILABLE

int _si_opendirectory_disabled;
static xpc_pipe_t __mbr_pipe; /* use accessor */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
__private_extern__ xpc_object_t _od_rpc_call(const char *procname, xpc_object_t payload, xpc_pipe_t (*get_pipe)(bool));

#endif

#ifdef DS_AVAILABLE
static void
_mbr_fork_child(void)
{
	if (__mbr_pipe != NULL) {
		xpc_pipe_invalidate(__mbr_pipe);
		/* disable release due to 10649340, it will cause a minor leak for each fork without exec */
		// xpc_release(__mbr_pipe);
		__mbr_pipe = NULL;
	}
	
	pthread_mutex_unlock(&mutex);
}
#endif

#ifdef DS_AVAILABLE
static void
_mbr_fork_prepare(void)
{
	pthread_mutex_lock(&mutex);
}
#endif

#ifdef DS_AVAILABLE
static void
_mbr_fork_parent(void)
{
	pthread_mutex_unlock(&mutex);
}
#endif

#ifdef DS_AVAILABLE
XPC_RETURNS_RETAINED
static xpc_pipe_t
_mbr_xpc_pipe(bool resetPipe)
{
	static dispatch_once_t once;
	xpc_pipe_t pipe = NULL;
	
	dispatch_once(&once, ^(void) {
		char *xbs_disable;
		
		/* if this is a build environment we ignore opendirectoryd */
		xbs_disable = getenv("XBS_DISABLE_LIBINFO");
		if (xbs_disable != NULL && strcmp(xbs_disable, "YES") == 0) {
			_si_opendirectory_disabled = 1;
			return;
		}
		
		pthread_atfork(_mbr_fork_prepare, _mbr_fork_parent, _mbr_fork_child);
	});
	
	if (_si_opendirectory_disabled == 1) {
		return NULL;
	}
	
	pthread_mutex_lock(&mutex);
	if (resetPipe) {
		xpc_release(__mbr_pipe);
		__mbr_pipe = NULL;
	}
	
	if (__mbr_pipe == NULL) {
		if (!dyld_process_is_restricted() && getenv("OD_DEBUG_MODE") != NULL) {
			__mbr_pipe = xpc_pipe_create(kODMachMembershipPortNameDebug, 0);
		} else {
			__mbr_pipe = xpc_pipe_create(kODMachMembershipPortName, XPC_PIPE_FLAG_PRIVILEGED);
		}
	}
	
	if (__mbr_pipe != NULL) pipe = xpc_retain(__mbr_pipe);
	pthread_mutex_unlock(&mutex);
	
	return pipe;
}
#endif

static bool
_mbr_od_available(void)
{
#if DS_AVAILABLE
	xpc_pipe_t pipe = _mbr_xpc_pipe(false);
	if (pipe != NULL) {
		xpc_release(pipe);
		return true;
	}
#endif
	return false;
}

int
mbr_identifier_translate(int id_type, const void *identifier, size_t identifier_size, int target_type, void **result, int *rec_type)
{
#if DS_AVAILABLE
	xpc_object_t payload, reply;
#endif
	id_t tempID;
	size_t identifier_len;
	int rc = EIO;
	
	if (identifier == NULL || result == NULL || identifier_size == 0) return EIO;
	
	if (identifier_size == -1) {
		identifier_size = strlen(identifier);
	} else {
		/* 10898647: For types that are known to be strings, send the smallest necessary amount of data. */
		switch (id_type) {
		case ID_TYPE_USERNAME:
		case ID_TYPE_GROUPNAME:
		case ID_TYPE_GROUP_NFS:
		case ID_TYPE_USER_NFS:
		case ID_TYPE_X509_DN:
		case ID_TYPE_KERBEROS:
		case ID_TYPE_NAME:
			identifier_len = strlen(identifier);
			if (identifier_size > identifier_len) {
				identifier_size = identifier_len;
			}
			break;
		}
	}

	switch (target_type) {
		case ID_TYPE_GID:
		case ID_TYPE_UID:
		case ID_TYPE_UID_OR_GID:
			/* shortcut UUIDs using compatibilty prefixes */
			if (id_type == ID_TYPE_UUID) {
				const uint8_t *uu = identifier;
				
				if (identifier_size != sizeof(uuid_t)) return EINVAL;
				
				if (memcmp(uu, _user_compat_prefix, COMPAT_PREFIX_LEN) == 0) {
					id_t *tempRes = malloc(sizeof(*tempRes));
					memcpy(&tempID, &uu[COMPAT_PREFIX_LEN], sizeof(tempID));
					(*tempRes) = ntohl(tempID);
					(*result) = tempRes;
					if (rec_type != NULL) {
						(*rec_type) = MBR_REC_TYPE_USER;
					}
					return 0;
				} else if (memcmp(uu, _group_compat_prefix, COMPAT_PREFIX_LEN) == 0) {
					id_t *tempRes = malloc(sizeof(*tempRes));
					memcpy(&tempID, &uu[COMPAT_PREFIX_LEN], sizeof(tempID));
					(*tempRes) = ntohl(tempID);
					(*result) = tempRes;
					if (rec_type != NULL) {
						(*rec_type) = MBR_REC_TYPE_GROUP;
					}
					return 0;
				}
			}
			break;
			
		case ID_TYPE_UUID:
			/* if this is a UID or GID translation, we shortcut UID/GID 0 */
			/* or if no OD, we return compatibility UUIDs */
			switch (id_type) {
				case ID_TYPE_UID:
					if (identifier_size != sizeof(tempID)) return EINVAL;
					
					tempID = *((id_t *) identifier);
					if ((tempID == 0) || (_mbr_od_available() == false)) {
						uint8_t *tempUU = malloc(sizeof(uuid_t));
						uuid_copy(tempUU, _user_compat_prefix);
						*((id_t *) &tempUU[COMPAT_PREFIX_LEN]) = htonl(tempID);
						(*result) = tempUU;
						if (rec_type != NULL) {
							(*rec_type) = MBR_REC_TYPE_USER;
						}
						return 0;
					}
					break;
					
				case ID_TYPE_GID:
					if (identifier_size != sizeof(tempID)) return EINVAL;
					
					tempID = *((id_t *) identifier);
					if ((tempID == 0) || (_mbr_od_available() == false)) {
						uint8_t *tempUU = malloc(sizeof(uuid_t));
						uuid_copy(tempUU, _group_compat_prefix);
						*((id_t *) &tempUU[COMPAT_PREFIX_LEN]) = htonl(tempID);
						(*result) = tempUU;
						if (rec_type != NULL) {
							(*rec_type) = MBR_REC_TYPE_GROUP;
						}
						return 0;
					}
					break;
			}
			break;
	}
	
#if DS_AVAILABLE
	payload = xpc_dictionary_create(NULL, NULL, 0);
	if (payload == NULL) return EIO;
	
	xpc_dictionary_set_int64(payload, "requesting", target_type);
	xpc_dictionary_set_int64(payload, "type", id_type);
	xpc_dictionary_set_data(payload, "identifier", identifier, identifier_size);
	
	reply = _od_rpc_call("mbr_identifier_translate", payload, _mbr_xpc_pipe);
	if (reply != NULL) {
		const void *reply_id;
		size_t idLen;
		
		rc = (int) xpc_dictionary_get_int64(reply, "error");
		if (rc == 0) {
			reply_id = xpc_dictionary_get_data(reply, "identifier", &idLen);
			if (reply_id != NULL) {
				char *identifier = malloc(idLen);
				
				memcpy(identifier, reply_id, idLen); // should already be NULL terminated, etc.
				(*result) = identifier;
				
				if (rec_type != NULL) {
					(*rec_type) = (int) xpc_dictionary_get_int64(reply, "rectype");
				}
			} else {
				(*result) = NULL;
				rc = ENOENT;
			}
		}
		
		xpc_release(reply);
	}
	
	xpc_release(payload);
#endif
	
	return rc;
}

int
mbr_uid_to_uuid(uid_t id, uuid_t uu)
{
	return mbr_identifier_to_uuid(ID_TYPE_UID, &id, sizeof(id), uu);
}

int
mbr_gid_to_uuid(gid_t id, uuid_t uu)
{
	return mbr_identifier_to_uuid(ID_TYPE_GID, &id, sizeof(id), uu);
}

int
mbr_uuid_to_id(const uuid_t uu, uid_t *id, int *id_type)
{
	id_t *result;
	int local_type;
	int rc;
	
	rc = mbr_identifier_translate(ID_TYPE_UUID, uu, sizeof(uuid_t), ID_TYPE_UID_OR_GID, (void **) &result, &local_type);
	if (rc == 0) {
		switch (local_type) {
			case MBR_REC_TYPE_GROUP:
				(*id_type) = ID_TYPE_GID;
				break;
				
			case MBR_REC_TYPE_USER:
				(*id_type) = ID_TYPE_UID;
				break;
				
			default:
				(*id_type) = -1;
				break;
		}
		
		(*id) = (*result);
		free(result);
	}
	
	return rc;
}

int
mbr_sid_to_uuid(const nt_sid_t *sid, uuid_t uu)
{
#ifdef DS_AVAILABLE
	return mbr_identifier_to_uuid(ID_TYPE_SID, sid, sizeof(*sid), uu);
#else
	return EIO;
#endif
}

int
mbr_identifier_to_uuid(int id_type, const void *identifier, size_t identifier_size, uuid_t uu)
{
	uint8_t *result;
	int rc;
	
	rc = mbr_identifier_translate(id_type, identifier, identifier_size, ID_TYPE_UUID, (void **) &result, NULL);
	if (rc == 0) {
		uuid_copy(uu, result);
		free(result);
	}
	
	return rc;
}

int
mbr_uuid_to_sid_type(const uuid_t uu, nt_sid_t *sid, int *id_type)
{
#ifdef DS_AVAILABLE
	void *result;
	int local_type;
	int rc;
	
	rc = mbr_identifier_translate(ID_TYPE_UUID, uu, sizeof(uuid_t), ID_TYPE_SID, &result, &local_type);
	if (rc == 0) {
		memcpy(sid, result, sizeof(nt_sid_t));
		if (id_type != NULL) {
			/* remap ID types */
			switch (local_type) {
				case MBR_REC_TYPE_USER:
					(*id_type) = SID_TYPE_USER;
					break;

				case MBR_REC_TYPE_GROUP:
					(*id_type) = SID_TYPE_GROUP;
					break;

				default:
					break;
			}
		}
		
		free(result);
	}
	
	return rc;
#else
	return EIO;
#endif
}

int
mbr_uuid_to_sid(const uuid_t uu, nt_sid_t *sid)
{
#ifdef DS_AVAILABLE
	int type, status;

	type = 0;

	status = mbr_uuid_to_sid_type(uu, sid, &type);
	if (status != 0) return status;

	return 0;
#else
	return EIO;
#endif
}

int
mbr_check_membership(const uuid_t user, const uuid_t group, int *ismember)
{
	return mbr_check_membership_ext(ID_TYPE_UUID, user, sizeof(uuid_t), ID_TYPE_UUID, group, 0, ismember);
}

int
mbr_check_membership_refresh(const uuid_t user, uuid_t group, int *ismember)
{
	return mbr_check_membership_ext(ID_TYPE_UUID, user, sizeof(uuid_t), ID_TYPE_UUID, group, 1, ismember);
}

int
mbr_check_membership_ext(int userid_type, const void *userid, size_t userid_size, int groupid_type, const void *groupid, int refresh, int *isMember)
{
#ifdef DS_AVAILABLE
	xpc_object_t payload, reply;
	int rc = 0;
	
	payload = xpc_dictionary_create(NULL, NULL, 0);
	if (payload == NULL) return ENOMEM;

	xpc_dictionary_set_int64(payload, "user_idtype", userid_type);
	xpc_dictionary_set_data(payload, "user_id", userid, userid_size);
	xpc_dictionary_set_int64(payload, "group_idtype", groupid_type);
	
	switch (groupid_type) {
		case ID_TYPE_GROUPNAME:
		case ID_TYPE_GROUP_NFS:
			xpc_dictionary_set_data(payload, "group_id", groupid, strlen(groupid));
			break;
			
		case ID_TYPE_GID:
			xpc_dictionary_set_data(payload, "group_id", groupid, sizeof(id_t));
			break;
			
		case ID_TYPE_SID:
			xpc_dictionary_set_data(payload, "group_id", groupid, sizeof(nt_sid_t));
			break;
			
		case ID_TYPE_UUID:
			xpc_dictionary_set_data(payload, "group_id", groupid, sizeof(uuid_t));
			break;
			
		default:
			rc = EINVAL;
			break;
	}
	
	if (rc == 0) {
		reply = _od_rpc_call("mbr_check_membership", payload, _mbr_xpc_pipe);
		if (reply != NULL) {
			rc = (int) xpc_dictionary_get_int64(reply, "error");
			(*isMember) = xpc_dictionary_get_bool(reply, "ismember");
			xpc_release(reply);
		} else {
			rc = EIO;
		}
	}
	
	xpc_release(payload);
	
	return rc;
#else
	return EIO;
#endif
}

int
mbr_check_membership_by_id(uuid_t user, gid_t group, int *ismember)
{
	return mbr_check_membership_ext(ID_TYPE_UUID, user, sizeof(uuid_t), ID_TYPE_GID, &group, 0, ismember);
}

int
mbr_reset_cache()
{
#ifdef DS_AVAILABLE
	_od_rpc_call("mbr_cache_flush", NULL, _mbr_xpc_pipe);
	return 0;
#else
	return EIO;
#endif
}

int
mbr_user_name_to_uuid(const char *name, uuid_t uu)
{
	return mbr_identifier_to_uuid(ID_TYPE_USERNAME, name, -1, uu);
}

int
mbr_group_name_to_uuid(const char *name, uuid_t uu)
{
	return mbr_identifier_to_uuid(ID_TYPE_GROUPNAME, name, -1, uu);
}

int
mbr_check_service_membership(const uuid_t user, const char *servicename, int *ismember)
{
#ifdef DS_AVAILABLE
	xpc_object_t payload, reply;
	int result = EIO;

	if (ismember == NULL || servicename == NULL) return EINVAL;
	
	payload = xpc_dictionary_create(NULL, NULL, 0);
	if (payload == NULL) return EIO;
	
	xpc_dictionary_set_data(payload, "user_id", user, sizeof(uuid_t));
	xpc_dictionary_set_int64(payload, "user_idtype", ID_TYPE_UUID);
	xpc_dictionary_set_string(payload, "service", servicename);
	
	reply = _od_rpc_call("mbr_check_service_membership", payload, _mbr_xpc_pipe);
	if (reply != NULL) {
		result = (int) xpc_dictionary_get_int64(reply, "error");
		(*ismember) = xpc_dictionary_get_bool(reply, "ismember");
		
		xpc_release(reply);
	} else {
		(*ismember) = 0;
	}
	
	xpc_release(payload);

	return result;
#else
	return EIO;
#endif
}

#ifdef DS_AVAILABLE
static char *
ConvertBytesToDecimal(char *buffer, unsigned long long value)
{
	char *temp;
	buffer[24] = '\0';
	buffer[23] = '0';

	if (value == 0)
		return &buffer[23];

	temp = &buffer[24];
	while (value != 0)
	{
		temp--;
		*temp = '0' + (value % 10);
		value /= 10;
	}

	return temp;
}
#endif

int
mbr_sid_to_string(const nt_sid_t *sid, char *string)
{
#ifdef DS_AVAILABLE
	char *current = string;
	long long temp = 0;
	int i;
	char tempBuffer[25];

	if (sid->sid_authcount > NTSID_MAX_AUTHORITIES) return EINVAL;

	for (i = 0; i < 6; i++)
		temp = (temp << 8) | sid->sid_authority[i];

	current[0] = 'S';
	current[1] = '-';
	current += 2;
	strcpy(current, ConvertBytesToDecimal(tempBuffer, sid->sid_kind));
	current = current + strlen(current);
	*current = '-';
	current++;
	strcpy(current, ConvertBytesToDecimal(tempBuffer, temp));

	for(i=0; i < sid->sid_authcount; i++)
	{
		current = current + strlen(current);
		*current = '-';
		current++;
		strcpy(current, ConvertBytesToDecimal(tempBuffer, sid->sid_authorities[i]));
	}

	return 0;
#else
	return EIO;
#endif
}

int
mbr_string_to_sid(const char *string, nt_sid_t *sid)
{
#ifdef DS_AVAILABLE
	char *current = (char *)string+2;
	int count = 0;
	long long temp;

	if (string == NULL) return EINVAL;

	memset(sid, 0, sizeof(nt_sid_t));
	if (string[0] != 'S' || string[1] != '-') return EINVAL;

	sid->sid_kind = strtol(current, &current, 10);
	if (*current == '\0') return EINVAL;
	current++;
	temp = strtoll(current, &current, 10);

	/* convert to BigEndian before copying */
	temp = OSSwapHostToBigInt64(temp);
	memcpy(sid->sid_authority, ((char*)&temp)+2, 6);
	while (*current != '\0' && count < NTSID_MAX_AUTHORITIES)
	{
		current++;
		errno = 0;
		sid->sid_authorities[count] = (u_int32_t)strtoll(current, &current, 10);
		if ((sid->sid_authorities[count] == 0) && (errno == EINVAL)) {
			return EINVAL;
		}
		count++;
	}

	if (*current != '\0') return EINVAL;

	sid->sid_authcount = count;

	return 0;
#else
	return EIO;
#endif
}

int
mbr_uuid_to_string(const uuid_t uu, char *string)
{
	uuid_unparse_upper(uu, string);
	
	return 0;
}

int
mbr_string_to_uuid(const char *string, uuid_t uu)
{
	return uuid_parse(string, uu);
}

int 
mbr_set_identifier_ttl(int id_type, const void *identifier, size_t identifier_size, unsigned int seconds)
{
#ifdef DS_AVAILABLE
	xpc_object_t payload, reply;
	int rc = 0;
	
	payload = xpc_dictionary_create(NULL, NULL, 0);
	if (payload == NULL) return ENOMEM;
	
	xpc_dictionary_set_int64(payload, "type", id_type);
	xpc_dictionary_set_data(payload, "identifier", identifier, identifier_size);
	xpc_dictionary_set_int64(payload, "ttl", seconds);
	
	if (rc == 0) {
		reply = _od_rpc_call("mbr_set_identifier_ttl", payload, _mbr_xpc_pipe);
		if (reply != NULL) {
			rc = (int) xpc_dictionary_get_int64(reply, "error");
			xpc_release(reply);
		} else {
			rc = EIO;
		}
	}
	
	xpc_release(payload);
	
	return rc;
#else
	return EIO;
#endif
}
