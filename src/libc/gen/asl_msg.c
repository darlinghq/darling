/*
 * Copyright (c) 2009-2011 Apple Inc. All rights reserved.
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

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <regex.h>
#include <syslog.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <asl.h>
#include <asl_private.h>
#include <asl_core.h>
#include <sys/types.h>
#include <libkern/OSAtomic.h>
#include <assert.h>
#include "asl_msg.h"

#define TOKEN_NULL  0
#define TOKEN_OPEN  1
#define TOKEN_CLOSE 2
#define TOKEN_WORD  3
#define TOKEN_INT   4

#define MFMT_RAW 0
#define MFMT_STD 1
#define MFMT_BSD 2
#define MFMT_XML 3
#define MFMT_STR 4
#define MFMT_MSG 5

#define SEC_PER_HOUR 3600

#define forever for(;;)

#define streq(A, B) (strcmp(A, B) == 0)
#define streq_len(A, B, C) (strncmp(A, B, C) == 0)
#define strneq(A, B) (strcmp(A, B) != 0)
#define strcaseeq(A, B) (strcasecmp(A, B) == 0)
#define strcaseneq(A, B) (strcasecmp(A, B) != 0)

#ifndef ASL_KEY_OPTION
#define ASL_KEY_OPTION "ASLOption"
#endif

#ifndef ASL_QUERY_OP_FALSE
#define ASL_QUERY_OP_FALSE 0
#endif

#define AUX_0_TIME      0x00000001
#define AUX_0_TIME_NSEC 0x00000002
#define AUX_0_HOST      0x00000004
#define AUX_0_SENDER    0x00000008
#define AUX_0_FACILITY  0x00000010
#define AUX_0_PID       0x00000020
#define AUX_0_UID       0x00000040
#define AUX_0_GID       0x00000080
#define AUX_0_MSG       0x00000100
#define AUX_0_OPTION    0x00000200
#define AUX_0_LEVEL     0x00000400

extern time_t asl_parse_time(const char *in);

/* from asl_util.c */
int asl_is_utf8(const char *str);
uint8_t *asl_b64_encode(const uint8_t *buf, size_t len);

static const char *ASLStandardKey[] =
{
	ASL_KEY_TIME,
	ASL_KEY_TIME_NSEC,
	ASL_KEY_HOST,
	ASL_KEY_SENDER,
	ASL_KEY_FACILITY,
	ASL_KEY_PID,
	ASL_KEY_UID,
	ASL_KEY_GID,
	ASL_KEY_LEVEL,
	ASL_KEY_MSG,
	ASL_KEY_READ_UID,
	ASL_KEY_READ_GID,
	ASL_KEY_SESSION,
	ASL_KEY_REF_PID,
	ASL_KEY_REF_PROC,
	ASL_KEY_MSG_ID,
	ASL_KEY_EXPIRE_TIME,
	ASL_KEY_OPTION
};

static const char *MTStandardKey[] =
{
	"com.apple.message.domain",
	"com.apple.message.domain_scope",
	"com.apple.message.result",
	"com.apple.message.signature",
	"com.apple.message.signature2",
	"com.apple.message.signature3",
	"com.apple.message.success",
	"com.apple.message.uuid",
	"com.apple.message.value",
	"com.apple.message.value2",
	"com.apple.message.value3",
	"com.apple.message.value4",
	"com.apple.message.value5"
};

static uint16_t
_asl_msg_std_key(const char *s, uint32_t len)
{
	if ((len > 18) && (streq_len(s, "com.apple.message.", 18)))
	{
		if (streq(s + 18, "domain")) return ASL_MT_KEY_DOMAIN;
		else if (streq(s + 18, "domain_scope")) return ASL_MT_KEY_SCOPE;
		else if (streq(s + 18, "result")) return ASL_MT_KEY_RESULT;
		else if (streq(s + 18, "signature")) return ASL_MT_KEY_SIG;
		else if (streq(s + 18, "signature2")) return ASL_MT_KEY_SIG2;
		else if (streq(s + 18, "signature3")) return ASL_MT_KEY_SIG3;
		else if (streq(s + 18, "success")) return ASL_MT_KEY_SUCCESS;
		else if (streq(s + 18, "uuid")) return ASL_MT_KEY_UUID;
		else if (streq(s + 18, "value")) return ASL_MT_KEY_VAL;
		else if (streq(s + 18, "value2")) return ASL_MT_KEY_VAL2;
		else if (streq(s + 18, "value3")) return ASL_MT_KEY_VAL3;
		else if (streq(s + 18, "value4")) return ASL_MT_KEY_VAL4;
		else if (streq(s + 18, "value5")) return ASL_MT_KEY_VAL5;

		return 0;
	}

	switch (len)
	{
		case 3:
		{
			if streq(s, ASL_KEY_PID) return ASL_STD_KEY_PID;
			else if streq(s, ASL_KEY_UID) return ASL_STD_KEY_UID;
			else if streq(s, ASL_KEY_GID) return ASL_STD_KEY_GID;
		}
		case 4:
		{
			if streq(s, ASL_KEY_TIME) return ASL_STD_KEY_TIME;
			else if streq(s, ASL_KEY_HOST) return ASL_STD_KEY_HOST;
		}
		case 5:
		{
			if streq(s, ASL_KEY_LEVEL) return ASL_STD_KEY_LEVEL;
		}
		case 6:
		{
			if streq(s, ASL_KEY_SENDER) return ASL_STD_KEY_SENDER;
			else if streq(s, ASL_KEY_REF_PID) return ASL_STD_KEY_REF_PID;
		}
		case 7:
		{
			if streq(s, ASL_KEY_MSG) return ASL_STD_KEY_MESSAGE;
			else if streq(s, ASL_KEY_SESSION) return ASL_STD_KEY_SESSION;
			else if streq(s, ASL_KEY_READ_UID) return ASL_STD_KEY_READ_UID;
			else if streq(s, ASL_KEY_READ_GID) return ASL_STD_KEY_READ_GID;
			else if streq(s, ASL_KEY_REF_PROC) return ASL_STD_KEY_REF_PROC;
		}
		case 8:
		{
			if streq(s, ASL_KEY_FACILITY) return ASL_STD_KEY_FACILITY;
		}
		case 9:
		{
			if streq(s, ASL_KEY_OPTION) return ASL_STD_KEY_OPTION;
		}
		case 11:
		{
			if streq(s, ASL_KEY_TIME_NSEC) return ASL_STD_KEY_NANO;
		}
		case 12:
		{
			if streq(s, ASL_KEY_MSG_ID) return ASL_STD_KEY_MSG_ID;
		}
		case 13:
		{
			if streq(s, ASL_KEY_EXPIRE_TIME) return ASL_STD_KEY_EXPIRE;
		}
		default:
		{
			return 0;
		}
	}

	return 0;
}

static asl_msg_t *
_asl_msg_make_page()
{
	asl_msg_t *out;
	int i;

	out = calloc(1, sizeof(asl_msg_t));
	if (out == NULL) return NULL;

	for (i = 0; i < ASL_MSG_PAGE_SLOTS; i++)
	{
		out->key[i] = ASL_MSG_SLOT_FREE;
		out->val[i] = ASL_MSG_SLOT_FREE;
	}

	return out;
}

static const char *
_asl_msg_slot_key(asl_msg_t *page, uint32_t slot)
{
	const char *out;
	uint16_t x;

	if (page == NULL) return NULL;
	if (slot >= ASL_MSG_PAGE_SLOTS) return NULL;

	if (page->key[slot] == ASL_MSG_SLOT_FREE) return NULL;

	switch (page->key[slot] & ASL_MSG_KV_MASK)
	{
		case ASL_MSG_KV_INLINE:
		{
			return page->data + page->key[slot];
		}
		case ASL_MSG_KV_DICT:
		{
			if ((page->key[slot] > ASL_STD_KEY_BASE) && (page->key[slot] <= ASL_STD_KEY_LAST))
			{
				x = page->key[slot] - ASL_STD_KEY_BASE - 1;
				return ASLStandardKey[x];
			}
			else if ((page->key[slot] > ASL_MT_KEY_BASE) && (page->key[slot] <= ASL_MT_KEY_LAST))
			{
				x = page->key[slot] - ASL_MT_KEY_BASE - 1;
				return MTStandardKey[x];
			}

			return NULL;
		}
		case ASL_MSG_KV_EXTERN:
		{
			x = page->key[slot] & ASL_MSG_OFFSET_MASK;
			memcpy(&out, page->data + x, sizeof(char *));
			return out;
		}
	}

	return NULL;
}

static const char *
_asl_msg_slot_val(asl_msg_t *page, uint32_t slot)
{
	const char *out;
	uint16_t x, type;

	if (page == NULL) return NULL;
	if (slot >= ASL_MSG_PAGE_SLOTS) return NULL;

	if (page->val[slot] == ASL_MSG_SLOT_FREE) return NULL;

	type = page->val[slot] & ASL_MSG_KV_MASK;

	if (type == ASL_MSG_KV_INLINE)
	{
		return page->data + page->val[slot];
	}
	else if (type == ASL_MSG_KV_EXTERN)
	{
		x = page->val[slot] & ASL_MSG_OFFSET_MASK;
		memcpy(&out, page->data + x, sizeof(char *));
		return out;
	}

	return NULL;
}

/*
 * asl_new: create a new log message.
 */
asl_msg_t *
asl_msg_new(uint32_t type)
{
	asl_msg_t *out;

	out = _asl_msg_make_page();
	if (out == NULL) return NULL;

	out->type = type;
	out->refcount = 1;

	return out;
}

asl_msg_t *
asl_msg_retain(asl_msg_t *msg)
{
	int32_t new;

	if (msg == NULL) return NULL;

	new = OSAtomicIncrement32Barrier(&msg->refcount);
	assert(new >= 1);

	return msg;
}

static void
_asl_msg_free(asl_msg_t *page)
{
	uint32_t i;
	char *p;

	if (page == NULL) return;

	for (i = 0; i < ASL_MSG_PAGE_SLOTS; i++)
	{
		if ((page->key[i] & ASL_MSG_KV_MASK) == ASL_MSG_KV_EXTERN)
		{
			memcpy(&p, page->data + (page->key[i] & ASL_MSG_OFFSET_MASK), sizeof(char *));
			free(p);
		}

		if ((page->val[i] & ASL_MSG_KV_MASK) == ASL_MSG_KV_EXTERN)
		{
			memcpy(&p, page->data + (page->val[i] & ASL_MSG_OFFSET_MASK), sizeof(char *));
			free(p);
		}
	}

	free(page);
}

void
asl_msg_release(asl_msg_t *msg)
{
	int32_t new;
	asl_msg_t *next;

	if (msg == NULL) return;

	new = OSAtomicDecrement32Barrier(&msg->refcount);
	assert(new >= 0);

	if (new > 0) return;

	while (msg != NULL)
	{
		next = msg->next;
		_asl_msg_free(msg);
		msg = next;
	}
}

static uint32_t
_asl_msg_index(asl_msg_t *msg, const char *key, uint32_t *oslot, asl_msg_t **opage)
{
	uint32_t i, len, slot;
	uint16_t kx;
	asl_msg_t *page;
	const char *kp;

	if (msg == NULL) return IndexNull;
	if (key == NULL) return IndexNull;

	i = 0;
	slot = 0;
	if (oslot != NULL) *oslot = slot;

	page = msg;
	if (opage != NULL) *opage = page;

	len = strlen(key);
	kx = _asl_msg_std_key(key, len);

	forever
	{
		if (page->key[slot] != ASL_MSG_SLOT_FREE)
		{
			if (kx != 0)
			{
				if (page->key[slot] == kx) return i;
			}
			else if ((page->key[slot] & ASL_MSG_KV_MASK) == ASL_MSG_KV_DICT)
			{
				/* page->key[slot] is a dictionary key, but key is not (kx == 0) so skip this slot */
			}
			else if ((page->key[slot] & ASL_MSG_KV_MASK) == ASL_MSG_KV_EXTERN)
			{
				memcpy(&kp, page->data + (page->key[slot] & ASL_MSG_OFFSET_MASK), sizeof(char *));
				if (streq(key, kp)) return i;
			}
			else
			{
				kp = page->data + page->key[slot];
				if (streq(key, kp)) return i;
			}
		}

		i++;
		slot++;
		if (oslot != NULL) *oslot = slot;

		if (slot >= ASL_MSG_PAGE_SLOTS)
		{
			if (page->next == NULL) return IndexNull;

			slot = 0;
			if (oslot != NULL) *oslot = slot;

			page = page->next;
			if (opage != NULL) *opage = page;
		}
	}

	return IndexNull;
}

/*
 * asl_msg_key: iterate over entries
 * initial value of n should be 0
 * after that, the value of n should be previously returned value
 * sets the pointers for the next key, value, and op in the msgionary
 * returns IndexNull when there are no more entries
 */
static uint32_t
_asl_msg_fetch_internal(asl_msg_t *msg, uint32_t n, const char **keyout, const char **valout, uint32_t *opout, asl_msg_t **outpage, uint32_t *outslot)
{
	uint32_t slot;
	asl_msg_t *page;

	if (msg == NULL) return IndexNull;
	if (outpage != NULL) *outpage = NULL;
	if (outslot != NULL) *outslot = IndexNull;

	slot = n;
	page = msg;

	while (slot >= ASL_MSG_PAGE_SLOTS)
	{
		if (page->next == NULL) return IndexNull;
		page = page->next;
		slot -= ASL_MSG_PAGE_SLOTS;
	}

	while (page->key[slot] == ASL_MSG_SLOT_FREE)
	{
		slot++;
		n++;

		if (slot >= ASL_MSG_PAGE_SLOTS)
		{
			if (page->next == NULL) return IndexNull;
			page = page->next;
			slot = 0;
		}
	}

	n++;

	if (keyout != NULL) *keyout = _asl_msg_slot_key(page, slot);
	if (valout != NULL) *valout = _asl_msg_slot_val(page, slot);
	if (opout != NULL) *opout = page->op[slot];

	if (outpage != NULL) *outpage = page;
	if (outslot != NULL) *outslot = slot;

	return n;
}

uint32_t
asl_msg_fetch(asl_msg_t *msg, uint32_t n, const char **keyout, const char **valout, uint32_t *opout)
{
	return _asl_msg_fetch_internal(msg, n, keyout, valout, opout, NULL, NULL);
}

static int
_asl_msg_new_key_val_op(asl_msg_t *msg, const char *key, const char *val, uint32_t op)
{
	uint32_t slot, keylen, vallen, total;
	uint16_t kx;
	asl_msg_t *page, *last;
	char *extkey, *extval;

	if (msg == NULL) return -1;
	if (key == NULL) return -1;

	extkey = NULL;
	extval = NULL;

	keylen = strlen(key);
	kx = _asl_msg_std_key(key, keylen);

	if (kx == 0) keylen++;
	else keylen = 0;

	total = keylen;

	vallen = 0;
	if (val != NULL)
	{
		vallen = strlen(val) + 1;
		total += vallen;
	}

	/* check if one or both of key and value must be "external" (in its own malloced space) */
	if (keylen > ASL_MSG_PAGE_DATA_SIZE)
	{
		extkey = strdup(key);
		keylen = sizeof(char *);
	}

	if (vallen > ASL_MSG_PAGE_DATA_SIZE)
	{
		extval = strdup(val);
		vallen = sizeof(char *);
	}

	total = keylen + vallen;
	if ((total > ASL_MSG_PAGE_DATA_SIZE) && (extval == NULL) && (keylen > 0))
	{
		extval = strdup(val);
		vallen = sizeof(char *);
		total = keylen + vallen;
	}

	if ((total > ASL_MSG_PAGE_DATA_SIZE) && (extkey == NULL))
	{
		extkey = strdup(key);
		keylen = sizeof(char *);
		total = keylen + vallen;
	}

	if (total > ASL_MSG_PAGE_DATA_SIZE)
	{
		/* can't happen, but... */
		if (extkey != NULL) free(extkey);
		if (extval != NULL) free(extval);
		return -1;
	}

	/* find a page with space for the key and value and a free slot*/
	slot = 0;
	last = msg;

	for (page = msg; page != NULL; page = page->next)
	{
		last = page;

		if (total <= (ASL_MSG_PAGE_DATA_SIZE - page->data_size))
		{
			/* check for a free slot */
			for (slot = 0; (slot < ASL_MSG_PAGE_SLOTS) && (page->key[slot] != ASL_MSG_SLOT_FREE); slot++);
			if (slot < ASL_MSG_PAGE_SLOTS) break;
		}
	}

	if (page == NULL)
	{
		/* allocate a new page and attach it */
		page = _asl_msg_make_page();
		if (page == NULL)
		{
			if (extkey != NULL) free(extkey);
			if (extval != NULL) free(extval);
			return -1;
		}

		last->next = page;
		slot = 0;
	}

	/* copy key or external key pointer into page data */
	if (kx != 0)
	{
		page->key[slot] = kx;
	}
	else if (extkey == NULL)
	{
		page->key[slot] = page->data_size;
		memcpy(page->data + page->data_size, key, keylen);
	}
	else
	{
		page->key[slot] = page->data_size | ASL_MSG_KV_EXTERN;
		memcpy(page->data + page->data_size, &extkey, keylen);
	}

	page->data_size += keylen;

	/* copy val or external val pointer into page data */
	page->val[slot] = ASL_MSG_SLOT_FREE;

	if (val != NULL)
	{
		if (extval == NULL)
		{
			page->val[slot] = page->data_size;
			memcpy(page->data + page->data_size, val, vallen);
		}
		else
		{
			page->val[slot] = page->data_size | ASL_MSG_KV_EXTERN;
			memcpy(page->data + page->data_size, &extval, vallen);
		}

		page->data_size += vallen;
	}

	/* set op */
	page->op[slot] = op;

	/* update page count */
	page->count++;

	return 0;
}

/*
 * Most of the code in asl_msg_set_key_val_op is concerned with trying to re-use
 * space in an asl_msg_t page when given a new value for an existing key.
 * If the key is new, we just call _asl_msg_new_key_val_op.
 *
 * Note that queries can have duplicate keys, so for ASL_TYPE_QUERY we just
 * call through to _asl_msg_new_key_val_op.
 */
static int
_asl_msg_set_kvo(asl_msg_t *msg, const char *key, const char *val, uint32_t op)
{
	uint32_t i, slot, newexternal;
	asl_msg_t *page;
	uint32_t intvallen, extvallen, newvallen;
	char *intval, *extval, *newval;

	if (msg == NULL) return -1;
	if (key == NULL) return -1;

	slot = IndexNull;
	page = NULL;

	if ((msg->type == ASL_TYPE_QUERY) || (IndexNull == _asl_msg_index(msg, key, &slot, &page)))
	{
		/* add key */
		return _asl_msg_new_key_val_op(msg, key, val, op);
	}

	intval = NULL;
	intvallen = 0;

	extval = NULL;
	extvallen = 0;

	if (page->val[slot] != ASL_MSG_SLOT_FREE)
	{
		if ((page->val[slot] & ASL_MSG_KV_MASK) == ASL_MSG_KV_EXTERN)
		{
			i = page->val[slot] & ASL_MSG_OFFSET_MASK;
			memcpy(&extval, page->data + i, sizeof(char *));
			extvallen = sizeof(char *);
		}
		else
		{
			intval = page->data + page->val[slot];
			intvallen = strlen(intval) + 1;
		}
	}

	/* replace val and op for existing entry */

	/* easy case  - remove val */
	if (val == NULL)
	{
		if (extval != NULL) free(extval);
		page->val[slot] = ASL_MSG_SLOT_FREE;
		if (op != IndexNull) page->op[slot] = op;
		return 0;
	}

	/* trivial case - internal val doesn't change */
	if ((intval != NULL) && (streq(val, intval)))
	{
		if (op != IndexNull) page->op[slot] = op;
		return 0;
	}

	/* trivial case - external val doesn't change */
	if ((extval != NULL) && (streq(val, extval)))
	{
		if (op != IndexNull) page->op[slot] = op;
		return 0;
	}

	/*
	 * special case: we generally don't compress out holes in the data
	 * space, but if this is the last string in the currently used data space
	 * we can just back up the data_size and reset page->val[slot]
	 */
	i = page->val[slot] & ASL_MSG_OFFSET_MASK;
	if ((intval != NULL) && ((i + intvallen) == page->data_size))
	{
		page->val[slot] = ASL_MSG_SLOT_FREE;
		page->data_size -= intvallen;
		intval = NULL;
		intvallen = 0;
	}
	else if ((extval != NULL) && ((i + extvallen) == page->data_size))
	{
		page->val[slot] = ASL_MSG_SLOT_FREE;
		page->data_size -= extvallen;
		free(extval);
		extval = NULL;
		extvallen = 0;
	}

	/* val changes - see if it needs to be external */
	newvallen = strlen(val) + 1;
	newexternal = 0;

	if (newvallen > ASL_MSG_PAGE_DATA_SIZE)
	{
		newexternal = 1;
		newvallen = sizeof(char *);
	}

	/* check if there is room to change val in place */
	if (((extval != NULL) && (newvallen <= extvallen)) || ((extval == NULL) && (newvallen <= intvallen)))
	{
		if (extval != NULL) free(extval);
		extval = NULL;

		/* we can re-use the space of the old value */
		i = page->val[slot] & ASL_MSG_OFFSET_MASK;

		if (newexternal == 1)
		{
			/* create an external val and copy in the new pointer */
			newval = strdup(val);
			if (newval == NULL) return -1;

			page->val[slot] = i | ASL_MSG_KV_EXTERN;
			memcpy(page->data + i, &newval, sizeof(char *));
		}
		else
		{
			/* new internal value */
			page->val[slot] = i;
			memcpy(page->data + i, val, newvallen);
		}

		if (op != IndexNull) page->op[slot] = op;
		return 0;
	}

	/* we're done with the old value if it is external - free it now */
	if (extval != NULL) free(extval);
	extval = NULL;

	if (newvallen <= (ASL_MSG_PAGE_DATA_SIZE - page->data_size))
	{
		/* can't re-use the old space, but there's room on the page */
		i = page->data_size;
		page->data_size += newvallen;

		if (newexternal == 1)
		{
			/* create an external val and copy in the new pointer */
			newval = strdup(val);
			if (newval == NULL) return -1;

			page->val[slot] = i | ASL_MSG_KV_EXTERN;
			memcpy(page->data + i, &newval, sizeof(char *));
		}
		else
		{
			/* new internal value */
			page->val[slot] = i;
			memcpy(page->data + i, val, newvallen);
		}

		if (op != IndexNull) page->op[slot] = op;
		return 0;

	}

	/* no room on this page - free up existing entry and treat this as a new entry */
	if ((page->key[slot] & ASL_MSG_KV_MASK) == ASL_MSG_KV_EXTERN)
	{
		memcpy(&extval, page->data + (page->key[slot] & ASL_MSG_OFFSET_MASK), sizeof(char *));
		free(extval);
	}

	page->key[slot] = ASL_MSG_SLOT_FREE;
	page->val[slot] = ASL_MSG_SLOT_FREE;
	page->op[slot] = 0;

	return _asl_msg_new_key_val_op(msg, key, val, op);
}

int
asl_msg_set_key_val_op(asl_msg_t *msg, const char *key, const char *val, uint32_t op)
{
	char *special, buf[512];
	uint32_t i, len;
	int status;

	/* Special case handling */
	special = NULL;

	/* convert "Level" values to "0" through "7" */
	if (streq(key, ASL_KEY_LEVEL))
	{
		if (val == NULL) val = "7";
		else if ((val[0] >= '0') && (val[0] <= '7') && (val[1] == '\0')) /* do nothing */;
		else if (strcaseeq(val, ASL_STRING_EMERG)) val = "0";
		else if (strcaseeq(val, ASL_STRING_ALERT)) val = "1";
		else if (strcaseeq(val, ASL_STRING_CRIT)) val = "2";
		else if (strcaseeq(val, ASL_STRING_ERR)) val = "3";
		else if (strcaseeq(val, ASL_STRING_WARNING)) val = "4";
		else if (strcaseeq(val, ASL_STRING_NOTICE)) val = "5";
		else if (strcaseeq(val, ASL_STRING_INFO)) val = "6";
		else if (strcaseeq(val, ASL_STRING_DEBUG)) val = "7";
		else val = "7";
	}

	/* strip trailing newlines from "Message" values */
	if ((streq(key, ASL_KEY_MSG)) && (val != NULL))
	{
		len = strlen(val);
		i = len;
		while ((i > 0) && (val[i - 1] == '\n')) i--;
		if (i == 0) val = NULL;
		else if (i < len)
		{
			/* use buf if it is big enough, else malloc a temporary buffer */
			if (i < sizeof(buf))
			{
				memcpy(buf, val, i);
				buf[i] = 0;
				val = (const char *)buf;
			}
			else
			{
				special = malloc(i + 1);
				if (special == NULL) return -1;
				memcpy(special, val, i);
				special[i] = 0;
				val = (const char *)special;
			}
		}
	}

	status = _asl_msg_set_kvo(msg, key, val, op);

	if (special != NULL) free(special);
	return status;
}

int
asl_msg_set_key_val(asl_msg_t *msg, const char *key, const char *val)
{
	return asl_msg_set_key_val_op(msg, key, val, 0);
}

/*
 * Merge a key / val into a message (only ASL_TYPE_MSG).
 * Adds the key / val if the key is not found.
 * Does not replace the value if the key is found.
 */
static void
_asl_msg_merge_key_val(asl_msg_t *msg, const char *key, const char *val)
{
	uint32_t i, slot;
	asl_msg_t *page;

	if (msg == NULL) return;
	if (key == NULL) return;

	slot = IndexNull;
	page = NULL;

	i = _asl_msg_index(msg, key, &slot, &page);
	if (i != IndexNull) return;

	asl_msg_set_key_val_op(msg, key, val, 0);
}

/*
 * Merge msg into target (does not replace existing keys).
 * Creates a new asl_msg_t if target is NULL.
 * Returns target.
 */
asl_msg_t *
asl_msg_merge(asl_msg_t *target, asl_msg_t *msg)
{
	uint32_t x, slot, op, isnew = 0;
	const char *key, *val;
	asl_msg_t *page;

	if (msg == NULL) return target;

	if (target == NULL)
	{
		isnew = 1;
		target = asl_msg_new(ASL_TYPE_MSG);
	}

	for (x = _asl_msg_fetch_internal(msg, 0, &key, &val, &op, &page, &slot); x != IndexNull; x = _asl_msg_fetch_internal(msg, x, &key, &val, &op, &page, &slot))
	{
		if (isnew == 1) asl_msg_set_key_val_op(target, key, val, 0);
		else _asl_msg_merge_key_val(target, key, val);
	}

	return target;
}

/*
 * Copy msg.
 */
asl_msg_t *
asl_msg_copy(asl_msg_t *msg)
{
	return asl_msg_merge(NULL, msg);
}

/*
 * asl_msg_unset
 * Frees external key and val strings, but does not try to reclaim data space.
 */
void
asl_msg_unset(asl_msg_t *msg, const char *key)
{
	uint32_t i, slot;
	asl_msg_t *page;
	char *ext;

	if (msg == NULL) return;
	if (key == NULL) return;

	slot = IndexNull;
	page = NULL;

	i = _asl_msg_index(msg, key, &slot, &page);
	if (i == IndexNull) return;

	if ((page->key[slot] & ASL_MSG_KV_MASK) == ASL_MSG_KV_EXTERN)
	{
		memcpy(&ext, page->data + (page->key[slot] & ASL_MSG_OFFSET_MASK), sizeof(char *));
		free(ext);
	}

	if ((page->val[slot] & ASL_MSG_KV_MASK) == ASL_MSG_KV_EXTERN)
	{
		memcpy(&ext, page->data + (page->val[slot] & ASL_MSG_OFFSET_MASK), sizeof(char *));
		free(ext);
	}

	page->key[slot] = ASL_MSG_SLOT_FREE;
	page->val[slot] = ASL_MSG_SLOT_FREE;
	page->op[slot] = 0;

	page->count--;
}

int
asl_msg_lookup(asl_msg_t *msg, const char *key, const char **valout, uint32_t *opout)
{
	uint32_t i, slot;
	asl_msg_t *page;

	slot = IndexNull;
	page = NULL;

	i = _asl_msg_index(msg, key, &slot, &page);
	if (i == IndexNull) return -1;

	if (valout != NULL) *valout = _asl_msg_slot_val(page, slot);
	if (opout != NULL) *opout =  page->op[slot];

	return 0;
}

uint32_t
asl_msg_type(asl_msg_t *msg)
{
	if (msg == NULL) return 0;
	return msg->type;
}

uint32_t
asl_msg_count(asl_msg_t *msg)
{
	uint32_t total;

	total = 0;

	for (; msg != NULL; msg = msg->next) total += msg->count;
	return total;
}

/*
 * Compare messages
 */
static int
_asl_msg_equal(asl_msg_t *a, asl_msg_t *b)
{
	uint32_t x, oa, ob;
	const char *key, *va, *vb;

	if (asl_msg_count(a) != asl_msg_count(b)) return 0;

	key = NULL;
	va = NULL;
	oa = 0;


	for (x = asl_msg_fetch(a, 0, &key, &va, &oa); x != IndexNull; x = asl_msg_fetch(a, x, &key, &va, &oa))
	{
		if (asl_msg_lookup(b, key, &vb, &ob) != 0) return 0;
		if (strcmp(va, vb)) return 0;
		if ((a->type == ASL_TYPE_QUERY) && (oa != ob)) return 0;
	}

	return 1;
}

static int
_asl_isanumber(const char *s)
{
	int i;

	if (s == NULL) return 0;

	i = 0;
	if ((s[0] == '-') || (s[0] == '+')) i = 1;

	if (s[i] == '\0') return 0;

	for (; s[i] != '\0'; i++)
	{
		if (!isdigit(s[i])) return 0;
	}

	return 1;
}

static int
_asl_msg_basic_test(uint32_t op, const char *q, const char *m, uint32_t n)
{
	int cmp;
	uint32_t t;
	int64_t nq, nm;
	int rflags;
	regex_t rex;

	t = op & ASL_QUERY_OP_TRUE;

	/* NULL value from query or message string fails */
	if ((q == NULL) || (m == NULL)) return (t & ASL_QUERY_OP_NOT_EQUAL);

	if (op & ASL_QUERY_OP_REGEX)
	{
		/* greater than or less than make no sense in substring search */
		if ((t == ASL_QUERY_OP_GREATER) || (t == ASL_QUERY_OP_LESS)) return 0;

		memset(&rex, 0, sizeof(regex_t));

		rflags = REG_EXTENDED | REG_NOSUB;
		if (op & ASL_QUERY_OP_CASEFOLD) rflags |= REG_ICASE;

		/* A bad reqular expression matches nothing */
		if (regcomp(&rex, q, rflags) != 0) return (t & ASL_QUERY_OP_NOT_EQUAL);

		cmp = regexec(&rex, m, 0, NULL, 0);
		regfree(&rex);

		if (t == ASL_QUERY_OP_NOT_EQUAL) return (cmp != 0);
		return (cmp == 0);
	}

	if (op & ASL_QUERY_OP_NUMERIC)
	{
		if (_asl_isanumber(q) == 0) return (t == ASL_QUERY_OP_NOT_EQUAL);
		if (_asl_isanumber(m) == 0) return (t == ASL_QUERY_OP_NOT_EQUAL);

		nq = atoll(q);
		nm = atoll(m);

		switch (t)
		{
			case ASL_QUERY_OP_EQUAL: return (nm == nq);
			case ASL_QUERY_OP_GREATER: return (nm > nq);
			case ASL_QUERY_OP_GREATER_EQUAL: return (nm >= nq);
			case ASL_QUERY_OP_LESS: return (nm < nq);
			case ASL_QUERY_OP_LESS_EQUAL: return (nm <= nq);
			case ASL_QUERY_OP_NOT_EQUAL: return (nm != nq);
			default: return (t == ASL_QUERY_OP_NOT_EQUAL);
		}
	}

	cmp = 0;
	if (op & ASL_QUERY_OP_CASEFOLD)
	{
		if (n == 0) cmp = strcasecmp(m, q);
		else cmp = strncasecmp(m, q, n);
	}
	else
	{
		if (n == 0) cmp = strcmp(m, q);
		else cmp = strncmp(m, q, n);
	}

	switch (t)
	{
		case ASL_QUERY_OP_EQUAL: return (cmp == 0);
		case ASL_QUERY_OP_GREATER: return (cmp > 0);
		case ASL_QUERY_OP_GREATER_EQUAL: return (cmp >= 0);
		case ASL_QUERY_OP_LESS: return (cmp < 0);
		case ASL_QUERY_OP_LESS_EQUAL: return (cmp <= 0);
		case ASL_QUERY_OP_NOT_EQUAL: return (cmp != 0);
	}

	return (t == ASL_QUERY_OP_NOT_EQUAL);
}

static int
_asl_msg_test_substring(uint32_t op, const char *q, const char *m)
{
	uint32_t t, i, d, lm, lq, match, newop;

	t = op & ASL_QUERY_OP_TRUE;

	lm = 0;
	if (m != NULL) lm = strlen(m);

	lq = 0;
	if (q != NULL) lq = strlen(q);

	/* NULL is a substring of any string */
	if (lq == 0) return (t & ASL_QUERY_OP_EQUAL);

	/* A long string is defined to be not equal to a short string */
	if (lq > lm) return (t == ASL_QUERY_OP_NOT_EQUAL);

	/* greater than or less than make no sense in substring search */
	if ((t == ASL_QUERY_OP_GREATER) || (t == ASL_QUERY_OP_LESS)) return 0;

	/*
	 * We scan the string doing an equality test.
	 * If the input test is equality, we stop as soon as we hit a match.
	 * Otherwise we keep scanning the whole message string.
	 */
	newop = op & 0xff0;
	newop |= ASL_QUERY_OP_EQUAL;

	match = 0;
	d = lm - lq;
	for (i = 0; i <= d; i++)
	{
		if (_asl_msg_basic_test(newop, q, m + i, lq) != 0)
		{
			if (t & ASL_QUERY_OP_EQUAL) return 1;
			match++;
		}
	}

	/* If the input test was for equality, no matches were found */
	if (t & ASL_QUERY_OP_EQUAL) return 0;

	/* The input test was for not equal.  Return true if no matches were found */
	return (match == 0);
}

static int
_asl_msg_test_prefix(uint32_t op, const char *q, const char *m)
{
	uint32_t lm, lq, t;

	t = op & ASL_QUERY_OP_TRUE;

	lm = 0;
	if (m != NULL) lm = strlen(m);

	lq = 0;
	if (q != NULL) lq = strlen(q);

	/* NULL is a prefix of any string */
	if (lq == 0) return (t & ASL_QUERY_OP_EQUAL);

	/* A long string is defined to be not equal to a short string */
	if (lq > lm) return (t == ASL_QUERY_OP_NOT_EQUAL);

	/* Compare two equal-length strings */
	return _asl_msg_basic_test(op, q, m, lq);
}

static int
_asl_msg_test_suffix(uint32_t op, const char *q, const char *m)
{
	uint32_t lm, lq, d, t;

	t = op & ASL_QUERY_OP_TRUE;

	lm = 0;
	if (m != NULL) lm = strlen(m);

	lq = 0;
	if (q != NULL) lq = strlen(q);

	/* NULL is a suffix of any string */
	if (lq == 0) return (t & ASL_QUERY_OP_EQUAL);

	/* A long string is defined to be not equal to a short string */
	if (lq > lm) return (t == ASL_QUERY_OP_NOT_EQUAL);

	/* Compare two equal-length strings */
	d = lm - lq;
	return _asl_msg_basic_test(op, q, m + d, lq);
}

/*
 * Splits out prefix, suffix, and substring tests.
 * Sends the rest to _asl_msg_basic_test().
 */
static int
_asl_msg_test_expression(uint32_t op, const char *q, const char *m)
{
	uint32_t t;

	t = op & ASL_QUERY_OP_TRUE;
	if (t == ASL_QUERY_OP_TRUE) return 1;

	if (op & ASL_QUERY_OP_PREFIX)
	{
		if (op & ASL_QUERY_OP_SUFFIX) return _asl_msg_test_substring(op, q, m);
		return _asl_msg_test_prefix(op, q, m);
	}
	if (op & ASL_QUERY_OP_SUFFIX) return _asl_msg_test_suffix(op, q, m);

	return _asl_msg_basic_test(op, q, m, 0);
}

/*
 * Special case for comparing time values.
 * If both inputs are time strings, this compares the time
 * value in seconds.  Otherwise it just does normal matching.
 */
static int
_asl_msg_test_time_expression(uint32_t op, const char *q, const char *m)
{
	time_t tq, tm;
	uint32_t t;

	if ((op & ASL_QUERY_OP_PREFIX) || (op & ASL_QUERY_OP_SUFFIX) || (op & ASL_QUERY_OP_REGEX)) return _asl_msg_test_expression(op, q, m);
	if ((q == NULL) || (m == NULL)) return _asl_msg_test_expression(op, q, m);

	tq = asl_parse_time(q);
	if (tq < 0) return _asl_msg_test_expression(op, q, m);

	tm = asl_parse_time(m);
	if (tm < 0) return _asl_msg_test_expression(op, q, m);

	t = op & ASL_QUERY_OP_TRUE;

	switch (t)
	{
		case ASL_QUERY_OP_FALSE:
		{
			return 0;
		}
		case ASL_QUERY_OP_EQUAL:
		{
			if (tm == tq) return 1;
			return 0;
		}
		case ASL_QUERY_OP_GREATER:
		{
			if (tm > tq) return 1;
			return 0;
		}
		case ASL_QUERY_OP_GREATER_EQUAL:
		{
			if (tm >= tq) return 1;
			return 0;
		}
		case ASL_QUERY_OP_LESS:
		{
			if (tm < tq) return 1;
			return 0;
		}
		case ASL_QUERY_OP_LESS_EQUAL:
		{
			if (tm <= tq) return 1;
			return 0;
		}
		case ASL_QUERY_OP_NOT_EQUAL:
		{
			if (tm != tq) return 1;
			return 0;
		}
		case ASL_QUERY_OP_TRUE:
		{
			return 1;
		}
	}

	/* NOTREACHED */
	return 0;
}

/* test a query against a message */
static int
_asl_msg_test(asl_msg_t *q, asl_msg_t *m)
{
	uint32_t i, t, x, op;
	int cmp;
	const char *kq, *vq, *vm;

	/*
	 * Check each simple expression (key op val) separately.
	 * The query suceeds (returns 1) if all simple expressions
	 * succeed (i.e. AND the simple expressions).
	 */

	kq = NULL;
	vq = NULL;
	op = 0;

	for (x = asl_msg_fetch(q, 0, &kq, &vq, &op); x != IndexNull; x = asl_msg_fetch(q, x, &kq, &vq, &op))
	{
		/* Find query key in the message */
		vm = NULL;
		i = asl_msg_lookup(m, kq, &vm, NULL);

		/* ASL_QUERY_OP_TRUE tests if key is present in the message */
		t = op & ASL_QUERY_OP_TRUE;
		if (t == ASL_QUERY_OP_TRUE)
		{
			if (i != 0) return 0;
			continue;
		}

		/* ASL_QUERY_OP_FALSE tests if the key is NOT present in the message */
		if (t == ASL_QUERY_OP_FALSE)
		{
			if (i == 0) return 0;
			continue;
		}

		if (i != 0)
		{
			/* the message does NOT have query key - fail unless we are testing not equal */
			if (t == ASL_QUERY_OP_NOT_EQUAL) continue;
			return 0;
		}

		cmp = 1;
		if (streq(kq, ASL_KEY_TIME))
		{
			cmp = _asl_msg_test_time_expression(op, vq, vm);
		}
		else
		{
			cmp = _asl_msg_test_expression(op, vq, vm);
		}

		if (cmp == 0) return 0;
	}

	return 1;
}

int
asl_msg_cmp(asl_msg_t *a, asl_msg_t *b)
{

	if (a == NULL) return 0;
	if (b == NULL) return 0;

	if (a->type == b->type) return _asl_msg_equal(a, b);
	if (a->type == ASL_TYPE_QUERY) return _asl_msg_test(a, b);
	return _asl_msg_test(b, a);
}


static char *
_asl_time_string(const char *fmt, const char *str)
{
	time_t tick, off;
	struct tm stm;
	char *ltime;
	char *out;
	char ltbuf[32];
	out = NULL;
	time_t min;
	const char *p = fmt;

	tick = 0;
	if (str != NULL) tick = asl_parse_time(str);

	/* default format is local time zone */
	if ((fmt == NULL) || (!strcasecmp(fmt, "lcl")) || (!strcasecmp(fmt, "local")))
	{
		ltime = ctime_r(&tick, ltbuf);
		if (ltime == NULL) return NULL;
		ltime[19] = '\0';
		asprintf(&out, "%s", ltime + 4);
		return out;
	}

	if ((!strcasecmp(fmt, "sec")) || (!strcasecmp(fmt, "raw")))
	{
		asprintf(&out, "%lu", tick);
		return out;
	}

	if (!strcasecmp(fmt, "j"))
	{
		if (NULL == localtime_r(&tick, &stm)) return NULL;
		asprintf(&out, "%d-%02d-%02d %02d:%02d:%02d", stm.tm_year + 1900, stm.tm_mon + 1, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec);
		return out;
	}

	if ((!strcasecmp(fmt, "utc")) || (!strcasecmp(fmt, "zulu")))
	{
		if (NULL == gmtime_r(&tick, &stm)) return NULL;
		asprintf(&out, "%d-%02d-%02d %02d:%02d:%02dZ", stm.tm_year + 1900, stm.tm_mon + 1, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec);
		return out;
	}

	if ((fmt[1] == '\0') && (((fmt[0] >= 'a') && (fmt[0] <= 'z')) || ((fmt[0] >= 'A') && (fmt[0] <= 'Z'))))
	{
		char z = fmt[0];
		if (z >= 'a') z -= 32;

		if (z == 'Z') off = 0;
		else if ((z >= 'A') && (z <= 'I')) off = ((z - 'A') + 1) * SEC_PER_HOUR;
		else if ((z >= 'K') && (z <= 'M')) off = (z - 'A') * SEC_PER_HOUR;
		else if ((z >= 'N') && (z <= 'Y')) off = ('M' - z) * SEC_PER_HOUR;
		else return NULL;
	}
	else
	{
		if ((*p == '-') || (*p == '+')) p++;
		if ((*p) >= '0' && (*p <= '9'))
		{
			off = atoi(p);
			if (fmt[0] == '-') off *= -1;
			off *= SEC_PER_HOUR;

			p = strchr(p, ':');
			if (p != NULL)
			{
				min = atoi(p + 1);
				if (fmt[0] == '-') min *= -1;
				min *= 60;
				off += min;
			}
		}
		else
		{
			return NULL;
		}
	}

	tick += off;

	memset(&stm, 0, sizeof (struct tm));
	if (NULL == gmtime_r(&tick, &stm)) return NULL;

	if ((fmt[0] >= 'A') && (fmt[0] <= 'Z'))
	{
		asprintf(&out, "%d-%02d-%02d %02d:%02d:%02d%c", stm.tm_year + 1900, stm.tm_mon + 1, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec, fmt[0]);
	}
	else if ((fmt[0] >= 'a') && (fmt[0] <= 'z'))
	{
		asprintf(&out, "%d-%02d-%02d %02d:%02d:%02d%c", stm.tm_year + 1900, stm.tm_mon + 1, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec, fmt[0] - 32);
	}
	else if ((fmt[0] >= '0') && (fmt[0] <= '9'))
	{
		asprintf(&out, "%d-%02d-%02d %02d:%02d:%02d+%s", stm.tm_year + 1900, stm.tm_mon + 1, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec, fmt);
	}
	else
	{
		asprintf(&out, "%d-%02d-%02d %02d:%02d:%02d%s", stm.tm_year + 1900, stm.tm_mon + 1, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec, fmt);
	}

	return out;
}

/* called from asl_format_message and _asl_send_message */
__private_extern__ asl_string_t *
asl_msg_to_string_raw(uint32_t encoding, asl_msg_t *msg, const char *tfmt)
{
	uint32_t i, x, count;
	char *vtime;
	const char *key, *val;
	asl_string_t *str;

	if (msg == NULL) return NULL;

	count = asl_msg_count(msg);
	if (count == 0) return NULL;

	str = asl_string_new(encoding);
	if (str == NULL) return NULL;

	key = NULL;
	val = NULL;
	i = 0;

	for (x = asl_msg_fetch(msg, 0, &key, &val, NULL); x != IndexNull; x = asl_msg_fetch(msg, x, &key, &val, NULL))
	{
		if (key == NULL) continue;

		if (i > 0) asl_string_append_char_no_encoding(str, ' ');

		asl_string_append_char_no_encoding(str, '[');
		asl_string_append_asl_key(str, key);

		if (!strcmp(key, ASL_KEY_TIME))
		{
			asl_string_append_char_no_encoding(str, ' ');
			vtime = NULL;

			if (val != NULL) vtime = _asl_time_string(tfmt, val);

			if (vtime != NULL)
			{
				asl_string_append_no_encoding(str, vtime);
				free(vtime);
			}
			else
			{
				asl_string_append_char_no_encoding(str, '0');
			}
		}
		else if (val != NULL)
		{
			asl_string_append_char_no_encoding(str, ' ');
			asl_string_append(str, val);
		}

		asl_string_append_char_no_encoding(str, ']');

		i++;
	}

	return str;
}

static asl_string_t *
_asl_string_append_asl_msg(asl_string_t *str, asl_msg_t *msg)
{
	const char *key, *val;
	uint32_t i, op, n;

	if (msg == NULL) return str;

	if (msg->type == ASL_TYPE_QUERY) asl_string_append(str, "Q ");

	i = 0;
	n = 0;

	forever 
	{
		key = NULL;
		val = NULL;

		i = asl_msg_fetch(msg, i, &key, &val, &op);
		if (key != NULL)
		{
			if (n != 0)	asl_string_append_char_no_encoding(str, ' ');
			n++;

			asl_string_append_char_no_encoding(str, '[');

			if (msg->type == ASL_TYPE_QUERY)
			{
				asl_string_append_op(str, op);
				asl_string_append_char_no_encoding(str, ' ');
			}

			asl_string_append_asl_key(str, key);

			if (val != NULL)
			{
				asl_string_append_char_no_encoding(str, ' ');
				asl_string_append(str, val);
			}

			asl_string_append_char_no_encoding(str, ']');
		}

		if (i == IndexNull) break;
	}

	return str;
}

char *
asl_msg_to_string(asl_msg_t *msg, uint32_t *len)
{
	char *out;
	asl_string_t *str = asl_string_new(ASL_ENCODE_ASL);
	if (str == NULL) return NULL;

	str = _asl_string_append_asl_msg(str, msg);
	*len = asl_string_length(str);
	out = asl_string_free_return_bytes(str);
	return out;
}

static uint32_t
_asl_msg_op_from_string(char *o)
{
	uint32_t op, i;

	op = ASL_QUERY_OP_NULL;

	if (o == NULL) return op;

	for (i = 0; o[i] != '\0'; i++)
	{
		if (o[i] == '.') return ASL_QUERY_OP_NULL;
		if (o[i] == 'C') op |= ASL_QUERY_OP_CASEFOLD;
		if (o[i] == 'R') op |= ASL_QUERY_OP_REGEX;
		if (o[i] == 'N') op |= ASL_QUERY_OP_NUMERIC;
		if (o[i] == 'S') op |= ASL_QUERY_OP_SUBSTRING;
		if (o[i] == 'A') op |= ASL_QUERY_OP_PREFIX;
		if (o[i] == 'Z') op |= ASL_QUERY_OP_SUFFIX;
		if (o[i] == '<') op |= ASL_QUERY_OP_LESS;
		if (o[i] == '>') op |= ASL_QUERY_OP_GREATER;
		if (o[i] == '=') op |= ASL_QUERY_OP_EQUAL;
		if (o[i] == '!') op |= ASL_QUERY_OP_NOT_EQUAL;
		if (o[i] == 'T') op |= ASL_QUERY_OP_TRUE;
	}

	return op;
}

static char *
_asl_msg_get_next_word(char **p, uint32_t *tt, uint32_t spacedel)
{
	char *str, *out, c, oval;
	uint32_t i, len, n, outlen;

	*tt = TOKEN_NULL;

	if (p == NULL) return NULL;
	if (*p == NULL) return NULL;
	if (**p == '\0') return NULL;

	/* skip one space if it's there (word separator) */
	if (**p == ' ') (*p)++;

	/* skip leading white space */
	if (spacedel != 0)
	{
		while ((**p == ' ') || (**p == '\t')) (*p)++;
	}

	if (**p == '\0') return NULL;
	if (**p == '\n') return NULL;

	str = *p;

	/* opening [ */
	if (**p == '[')
	{
		*tt = TOKEN_OPEN;

		(*p)++;
		out = malloc(2);
		if (out == NULL) return NULL;

		out[0] = '[';
		out[1] = '\0';
		return out;
	}

	/* scan for token and calulate it's length (input and decoded output len) */
	len = 0;
	outlen = 0;

	forever
	{
		c = str[len];

		/* stop scanning when we hit a delimiter */
		if (((spacedel != 0) && (c == ' ')) || (c == ']') || (c == '\0')) break;

		if (c == '\\')
		{
			len++;
			c = str[len];
			if ((c == 'a') || (c == 'b') || (c == 't') || (c == 'n') || (c == 'v') || (c == 'f') || (c == 'r') || (c == 's') || (c == '[') || (c == '\\') || (c == ']'))
			{
			}
			else if (c == '^')
			{
				if (str[++len] == '\0') return NULL;
			}
			else if (c == 'M')
			{
				if (str[++len] == '\0') return NULL;
				if (str[++len] == '\0') return NULL;
			}
			else if ((c >= '0') && (c <= '3'))
			{
				if (str[++len] == '\0') return NULL;
				if (str[++len] == '\0') return NULL;
			}
			else
			{
				return NULL;
			}
		}

		len++;
		outlen++;
	}

	(*p) += len;

	if ((len == 0) && (**p == ']'))
	{
		*tt = TOKEN_CLOSE;
		(*p)++;
		out = malloc(2);
		if (out == NULL) return NULL;

		out[0] = ']';
		out[1] = '\0';
		return out;
	}

	*tt = TOKEN_INT;

	out = malloc(outlen + 1);
	if (out == NULL) return NULL;

	n = 0;
	for (i = 0; i < len; i++)
	{
		c = str[i];

		if (c == '\\')
		{
			*tt = TOKEN_WORD;

			i++;
			c = str[i];
			if (c == 'a')
			{
				out[n++] = '\a';
			}
			else if (c == 'b')
			{
				out[n++] = '\b';
			}
			else if (c == 't')
			{
				out[n++] = '\t';
			}
			else if (c == 'n')
			{
				out[n++] = '\n';
			}
			else if (c == 'v')
			{
				out[n++] = '\v';
			}
			else if (c == 'f')
			{
				out[n++] = '\f';
			}
			else if (c == 'r')
			{
				out[n++] = '\r';
			}
			else if (c == 's')
			{
				out[n++] = ' ';
			}
			else if (c == '[')
			{
				out[n++] = '[';
			}
			else if (c == '\\')
			{
				out[n++] = '\\';
			}
			else if (c == ']')
			{
				out[n++] = ']';
			}
			else if (c == '^')
			{
				i++;
				if (str[i] == '?') out[n++] = 127;
				else out[n++] = str[i] - 64;
			}
			else if (c == 'M')
			{
				i++;
				c = str[i];
				if (c == '^')
				{
					i++;
					if (str[i] == '?') out[n++] = 255;
					else out[n++] = str[i] + 64;
				}
				else if (c == '-')
				{
					i++;
					out[n++] = str[i] + 128;
				}
				else
				{
					*tt = TOKEN_NULL;
					free(out);
					return NULL;
				}

			}
			else if ((c >= '0') && (c <= '3'))
			{
				oval = (c - '0') * 64;

				i++;
				c = str[i];
				if ((c < '0') || (c > '7'))
				{
					*tt = TOKEN_NULL;
					free(out);
					return NULL;
				}

				oval += ((c - '0') * 8);

				i++;
				c = str[i];
				if ((c < '0') || (c > '7'))
				{
					*tt = TOKEN_NULL;
					free(out);
					return NULL;
				}

				oval += (c - '0');

				out[n++] = oval;
			}
			else
			{
				*tt = TOKEN_NULL;
				free(out);
				return NULL;
			}
		}
		else
		{

			if ((c < '0') || (c > '9')) *tt = TOKEN_WORD;
			out[n++] = c;
		}
	}

	out[n] = '\0';

	return out;
}

asl_msg_t *
asl_msg_from_string(const char *buf)
{
	uint32_t tt, type, op;
	char *k, *v, *o, *p;
	asl_msg_t *out;

	if (buf == NULL) return NULL;

	type = ASL_TYPE_MSG;
	p = (char *)buf;

	k = _asl_msg_get_next_word(&p, &tt, 1);
	if (k == NULL) return NULL;

	if (streq(k, "Q"))
	{
		type = ASL_TYPE_QUERY;
		free(k);

		k = _asl_msg_get_next_word(&p, &tt, 1);
	}
	else if (tt == TOKEN_INT)
	{
		/* Leading integer is a string length - skip it */
		free(k);
		k = _asl_msg_get_next_word(&p, &tt, 1);
		if (k == NULL) return NULL;
	}

	out = asl_msg_new(ASL_TYPE_MSG);
	if (out == NULL) return NULL;

	out->type = type;

	/* OPEN WORD [WORD [WORD]] CLOSE */
	while (k != NULL)
	{
		op = ASL_QUERY_OP_NULL;

		if (tt != TOKEN_OPEN)
		{
			asl_msg_release(out);
			return NULL;
		}

		free(k);

		/* get op for query type */
		if (type == ASL_TYPE_QUERY)
		{
			o = _asl_msg_get_next_word(&p, &tt, 1);
			if ((o == NULL) || (tt != TOKEN_WORD))
			{
				if (o != NULL) free(o);
				asl_msg_release(out);
				return NULL;
			}

			op = _asl_msg_op_from_string(o);
			free(o);
		}

		k = _asl_msg_get_next_word(&p, &tt, 1);
		if (tt == TOKEN_INT) tt = TOKEN_WORD;
		if ((k == NULL) || (tt != TOKEN_WORD))
		{
			if (k != NULL) free(k);
			asl_msg_release(out);
			return NULL;
		}

		v = _asl_msg_get_next_word(&p, &tt, 0);
		if (tt == TOKEN_INT) tt = TOKEN_WORD;
		if (v == NULL)
		{
			asl_msg_set_key_val_op(out, k, NULL, op);
			free(k);
			break;
		}

		if (tt == TOKEN_CLOSE)
		{
			asl_msg_set_key_val_op(out, k, NULL, op);
		}
		else if (tt == TOKEN_WORD)
		{
			asl_msg_set_key_val_op(out, k, v, op);
		}
		else
		{
			if (k != NULL) free(k);
			if (v != NULL) free(v);
			asl_msg_release(out);
			return NULL;
		}

		if (k != NULL) free(k);
		if (v != NULL) free(v);

		if (tt != TOKEN_CLOSE)
		{
			k = _asl_msg_get_next_word(&p, &tt, 1);
			if (k == NULL) break;

			if (tt != TOKEN_CLOSE)
			{
				asl_msg_release(out);
				return NULL;
			}

			free(k);
		}

		k = _asl_msg_get_next_word(&p, &tt, 1);
		if (k == NULL) break;
	}

	return out;
}

char *
asl_list_to_string(asl_search_result_t *list, uint32_t *len)
{
	uint32_t i;
	char tmp[16];
	char *out;
	asl_string_t *str = asl_string_new(ASL_ENCODE_ASL);
	if (str == NULL) return NULL;

	if (list == NULL) return NULL;
	if (list->count == 0) return NULL;
	if (list->msg == NULL) return NULL;

	snprintf(tmp, sizeof(tmp), "%u", list->count);
	asl_string_append(str, tmp);
	asl_string_append_char_no_encoding(str, '\n');

	for (i = 0; i < list->count; i++)
	{
		_asl_string_append_asl_msg(str, list->msg[i]);
		asl_string_append_char_no_encoding(str, '\n');
	}

	*len = asl_string_length(str);
	out = asl_string_free_return_bytes(str);
	return out;
}

asl_search_result_t *
asl_list_from_string(const char *buf)
{
	uint32_t i, n;
	const char *p;
	asl_search_result_t *out;
	asl_msg_t *m;

	if (buf == NULL) return NULL;
	p = buf;

	n = atoi(buf);
	if (n == 0) return NULL;

	out = (asl_search_result_t *)calloc(1, sizeof(asl_search_result_t));
	if (out == NULL) return NULL;

	out->msg = (asl_msg_t **)calloc(n, sizeof(asl_msg_t *));
	if (out->msg == NULL)
	{
		free(out);
		return NULL;
	}

	for (i = 0; i < n; i++)
	{
		p = strchr(p, '\n');
		if (p == NULL)
		{
			aslresponse_free((aslresponse)out);
			return NULL;
		}

		p++;

		m = asl_msg_from_string(p);
		if (m == NULL)
		{
			aslresponse_free((aslresponse)out);
			return NULL;
		}

		out->msg[i] = (asl_msg_t *)m;
		out->count += 1;
	}

	return out;
}

static const char *
_asl_level_string(int level)
{
	if (level == ASL_LEVEL_EMERG) return ASL_STRING_EMERG;
	if (level == ASL_LEVEL_ALERT) return ASL_STRING_ALERT;
	if (level == ASL_LEVEL_CRIT) return ASL_STRING_CRIT;
	if (level == ASL_LEVEL_ERR) return ASL_STRING_ERR;
	if (level == ASL_LEVEL_WARNING) return ASL_STRING_WARNING;
	if (level == ASL_LEVEL_NOTICE) return ASL_STRING_NOTICE;
	if (level == ASL_LEVEL_INFO) return ASL_STRING_INFO;
	if (level == ASL_LEVEL_DEBUG) return ASL_STRING_DEBUG;
	return "unknown";
}

/*
 * Find the value for a key in a message and append a formatted value to str.
 * kf may be a simple (no embedded white space) key, or one of (key) or ((key)(format)).
 * WARNING: modifies kf!
 */
static asl_string_t *
_asl_string_append_value_for_key_format(asl_string_t *str, asl_msg_t *msg, char *kf, const char *tfmt)
{
	uint32_t i, get_fmt;
	char *key, *fmt;
	const char *mval;

	if (str == NULL) return NULL;
	if (msg == NULL) return str;
	if (kf == NULL) return str;

	key = NULL;
	fmt = NULL;
	get_fmt = 0;

	for (i = 0; kf[i] != '\0'; i++)
	{
		if (kf[i] == ')')
		{
			kf[i] = '\0';
			get_fmt = 1;
		}
		else if (kf[i] != '(')
		{
			if (key == NULL) key = kf + i;
			else if ((get_fmt == 1) && (fmt == NULL)) fmt = kf + i;
		}
	}

	if (key == NULL) return str;

	asl_msg_lookup(msg, key, &mval, NULL);
	if (mval == NULL) return str;

	if (!strcmp(key, ASL_KEY_TIME))
	{
		char *fval = NULL;

		/* format in $((Time)(fmt)) overrides tfmt */
		if (fmt == NULL)
		{
			fval = _asl_time_string(tfmt, mval);
		}
		else
		{
			fval = _asl_time_string(fmt, mval);
		}

		if (fval != NULL)
		{
			asl_string_append_no_encoding(str, fval);
			free(fval);
		}
		else
		{
			asl_string_append_char_no_encoding(str, '0');
		}

		return str;
	}

	/* Level: num str */
	if (!strcmp(key, ASL_KEY_LEVEL))
	{
		if (fmt == NULL)
		{
			asl_string_append_no_encoding(str, mval);
		}
		else if (!strcmp(fmt, "str"))
		{
			mval = _asl_level_string(atoi(mval));
			asl_string_append_no_encoding(str, mval);
		}
		else
		{
			asl_string_append_no_encoding(str, mval);
		}

		return str;
	}

	return asl_string_append(str, mval);
}

/*
 * format a message for printing
 * out parameter len returns string length including trailing NUL
 */
char *
asl_format_message(asl_msg_t *msg, const char *mfmt, const char *tfmt, uint32_t text_encoding, uint32_t *len)
{
	char *out, *vtime, *k, c, skey[512];
	const char *vhost, *vpid, *vsender, *vmessage, *vlevel, *vrefproc, *vrefpid, *v, *key, *val;
	int i, j, l, mf, paren, oval, level;
	uint32_t x, cursor;
	asl_string_t *str;
	uint8_t *b64;

	out = NULL;
	*len = 0;

	if (msg == NULL) return NULL;

	mf = MFMT_RAW;

	if (mfmt == NULL) mf = MFMT_RAW;
	else if (!strcmp(mfmt, ASL_MSG_FMT_RAW)) mf = MFMT_RAW;
	else if (!strcmp(mfmt, ASL_MSG_FMT_STD)) mf = MFMT_STD;
	else if (!strcmp(mfmt, ASL_MSG_FMT_BSD)) mf = MFMT_BSD;
	else if (!strcmp(mfmt, ASL_MSG_FMT_XML)) mf = MFMT_XML;
	else if (!strcmp(mfmt, ASL_MSG_FMT_MSG)) mf = MFMT_MSG;
	else mf = MFMT_STR;

	if (mf == MFMT_RAW)
	{
		str = asl_msg_to_string_raw(text_encoding, msg, tfmt);
		asl_string_append_char_no_encoding(str, '\n');

		*len = asl_string_length(str);
		out = asl_string_free_return_bytes(str);
		return out;
	}

	if (mf == MFMT_MSG)
	{
		vmessage = NULL;

		if (asl_msg_lookup(msg, ASL_KEY_MSG, &vmessage, NULL) != 0) return NULL;

		str = asl_string_new(text_encoding);
		if (str == NULL) return NULL;

		asl_string_append(str, vmessage);
		asl_string_append_char_no_encoding(str, '\n');

		*len = asl_string_length(str);
		out = asl_string_free_return_bytes(str);
		return out;
	}

	if ((mf == MFMT_STD) || (mf == MFMT_BSD))
	{
		/* COMMON:  Mth dd hh:mm:ss host sender[pid] (refproc[refpid])*/
		/* BSD:  <COMMON>: message */
		/* STD:  <COMMON> <Level>: message */

		v = NULL;
		vhost = NULL;
		vsender = NULL;
		vpid = NULL;
		vmessage = NULL;
		vlevel = NULL;
		vrefproc = NULL;
		vrefpid = NULL;

		asl_msg_lookup(msg, ASL_KEY_TIME, &v, NULL);
		vtime = _asl_time_string(tfmt, v);

		level = 7;
		asl_msg_lookup(msg, ASL_KEY_LEVEL, &vlevel, NULL);
		if (vlevel != NULL) level = atoi(vlevel);

		asl_msg_lookup(msg, ASL_KEY_HOST, &vhost, NULL);
		if (vhost == NULL) vhost = "unknown";

		asl_msg_lookup(msg, ASL_KEY_SENDER, &vsender, NULL);
		if (vsender == NULL) vsender = "unknown";

		asl_msg_lookup(msg, ASL_KEY_PID, &vpid, NULL);
		asl_msg_lookup(msg, ASL_KEY_MSG, &vmessage, NULL);
		asl_msg_lookup(msg, ASL_KEY_REF_PROC, &vrefproc, NULL);
		asl_msg_lookup(msg, ASL_KEY_REF_PID, &vrefpid, NULL);

		/* COMMON */
		str = asl_string_new(text_encoding);
		if (str == NULL) return NULL;

		if (vtime != NULL)
		{
			asl_string_append(str, vtime);
			free(vtime);
		}
		else
		{
			asl_string_append_char_no_encoding(str, '0');
		}

		asl_string_append_char_no_encoding(str, ' ');
		asl_string_append(str, vhost);
		asl_string_append_char_no_encoding(str, ' ');
		asl_string_append(str, vsender);

		if ((vpid != NULL) && (strcmp(vpid, "-1")))
		{
			asl_string_append_char_no_encoding(str, '[');
			asl_string_append(str, vpid);
			asl_string_append_char_no_encoding(str, ']');
		}

		if ((vrefproc != NULL) || (vrefpid != NULL)) asl_string_append_no_encoding(str, " (");

		if (vrefproc != NULL) asl_string_append(str, vrefproc);
		if (vrefpid != NULL)
		{
			asl_string_append_char_no_encoding(str, '[');
			asl_string_append(str, vrefpid);
			asl_string_append_char_no_encoding(str, ']');
		}

		if ((vrefproc != NULL) || (vrefpid != NULL)) asl_string_append_char_no_encoding(str, ')');

		if (mf == MFMT_STD)
		{
			asl_string_append_no_encoding(str, " <");
			asl_string_append(str, _asl_level_string(level));
			asl_string_append_char_no_encoding(str, '>');
		}

		asl_string_append_no_encoding(str, ": ");
		if (vmessage != NULL) asl_string_append(str, vmessage);
		asl_string_append_char_no_encoding(str, '\n');

		*len = asl_string_length(str);
		out = asl_string_free_return_bytes(str);
		return out;
	}

	if (mf == MFMT_XML)
	{
		str = asl_string_new(text_encoding);
		if (str == NULL) return NULL;

		asl_string_append_char_no_encoding(str, '\t');
		asl_string_append(str, "<dict>");
		asl_string_append_char_no_encoding(str, '\n');

		for (x = asl_msg_fetch(msg, 0, &key, &val, NULL); x != IndexNull; x = asl_msg_fetch(msg, x, &key, &val, NULL))
		{
			if (asl_is_utf8(key) == 1)
			{
				asl_string_append_xml_tag(str, "key", key);
				if (!strcmp(key, ASL_KEY_TIME))
				{
					vtime = _asl_time_string(tfmt, val);
					if (vtime != NULL)
					{
						asl_string_append_xml_tag(str, "string", vtime);
						free(vtime);
					}
					else
					{
						asl_string_append_xml_tag(str, "string", "0");
					}
				}
				else
				{
					if (asl_is_utf8(val) == 1) asl_string_append_xml_tag(str, "string", val);
					else
					{
						b64 = asl_b64_encode((uint8_t *)val, strlen(val));
						asl_string_append_xml_tag(str, "data", (char *)b64);
						free(b64);
					}
				}
			}
		}

		asl_string_append_char_no_encoding(str, '\t');
		asl_string_append(str, "</dict>");
		asl_string_append_char_no_encoding(str, '\n');

		*len = asl_string_length(str);
		out = asl_string_free_return_bytes(str);
		return out;
	}

	/*
	 * Custom format
	 * The format string may contain arbitrary characters.
	 * Keys are identified by $Key or $(Key).  The value for
	 * that key is substituted.  If there are alterate formats
	 * for the value (for example a time may be formatted as
	 * raw seconds, in UTC, or a local timezone), then the
	 * key may be $((Key)(Format)).  "\$" prints a plain "$".
	 */

	str = asl_string_new(text_encoding);
	if (str == NULL) return NULL;

	/*
	 * We need enough space to copy any keys found in mfmt.
	 * The key obviously can't be longer than strlen(mfmt),
	 * in fact, keys must be shorter, since there's at least a '$'
	 * in front of the key, so we allocate a buffer with strlen(mfmt).
	 * If strlen(mfmt) <= sizeof(skey), we use skey to avoid a malloc.
	 */

	x = strlen(mfmt);
	if (x <= sizeof(skey))
	{
		k = skey;
	}
	else
	{
		k = malloc(x);
		if (k == NULL) return NULL;
	}

	cursor = 0;

	for (i = 0; mfmt[i] != '\0'; i++)
	{
		if (mfmt[i] == '$')
		{
			paren = 0;

			/* scan key, (key) or ((key)(format)) */
			for (j = i + 1; mfmt[j] != 0; j++)
			{
				if (mfmt[j] == '(')
				{
					paren++;
				}
				else if (mfmt[j] == ')')
				{
					if (paren > 0) paren--;
					if (paren == 0)
					{
						j++;
						break;
					}
				}
				else if (((mfmt[j] == ' ') || (mfmt[j] == '\t')) && (paren == 0)) break;
			}

			/* mfmt[i + 1] is the first char of the key or a '('. mfmt[j] is one char past the end. */
			l = j - (i + 1);
			memcpy(k, mfmt+i+1, l);
			k[l] = '\0';
			_asl_string_append_value_for_key_format(str, msg, k, tfmt);

			i = j - 1;
			continue;
		}

		if (mfmt[i] == '\\')
		{
			i++;
			if (mfmt[i] == '$') asl_string_append_char_no_encoding(str, '$');
			else if (mfmt[i] == 'e') asl_string_append_char_no_encoding(str, '\e');
			else if (mfmt[i] == 's') asl_string_append_char_no_encoding(str, ' ');
			else if (mfmt[i] == 'a') asl_string_append_char_no_encoding(str, '\a');
			else if (mfmt[i] == 'b') asl_string_append_char_no_encoding(str, '\b');
			else if (mfmt[i] == 'f') asl_string_append_char_no_encoding(str, '\f');
			else if (mfmt[i] == 'n') asl_string_append_char_no_encoding(str, '\n');
			else if (mfmt[i] == 'r') asl_string_append_char_no_encoding(str, '\r');
			else if (mfmt[i] == 't') asl_string_append_char_no_encoding(str, '\t');
			else if (mfmt[i] == 'v') asl_string_append_char_no_encoding(str, '\v');
			else if (mfmt[i] == '\'') asl_string_append_char_no_encoding(str, '\'');
			else if (mfmt[i] == '\\') asl_string_append_char_no_encoding(str, '\\');
			else if (isdigit(mfmt[i]))
			{
				oval = mfmt[i] - '0';
				if (isdigit(mfmt[i+1]))
				{
					i++;
					oval = (oval * 8) + (mfmt[i] - '0');
					if (isdigit(mfmt[i+1]))
					{
						i++;
						oval = (oval * 8) + (mfmt[i] - '0');
					}
				}
				c = oval;
				asl_string_append_char_no_encoding(str, c);
			}
			continue;
		}

		if (mfmt[i] == '\0') break;
		asl_string_append_char_no_encoding(str, mfmt[i]);
	}

	if (k != skey) free(k);

	asl_string_append_char_no_encoding(str, '\n');

	*len = asl_string_length(str);
	out = asl_string_free_return_bytes(str);
	return out;
}

/*
 * OLD ASLMSG COMPATIBILITY
 */
const char *
asl_key(aslmsg msg, uint32_t n)
{
	uint32_t slot, i;
	asl_msg_t *page;

	i = 0;
	for (page = (asl_msg_t *)msg; page != NULL; page = page->next)
	{
		for (slot = 0; slot < ASL_MSG_PAGE_SLOTS; slot++)
		{
			if (page->key[slot] != ASL_MSG_SLOT_FREE)
			{
				if (i == n) return _asl_msg_slot_key(page, slot);
				i++;
			}
		}
	}

	return NULL;
}

aslmsg
asl_new(uint32_t type)
{
	return (aslmsg)asl_msg_new(type);
}

int
asl_set(aslmsg msg, const char *key, const char *value)
{
	return asl_msg_set_key_val_op((asl_msg_t *)msg, key, value, IndexNull);
}

int
asl_set_query(aslmsg msg, const char *key, const char *value, uint32_t op)
{
	return asl_msg_set_key_val_op((asl_msg_t *)msg, key, value, op);
}

int
asl_unset(aslmsg msg, const char *key)
{
	asl_msg_unset((asl_msg_t *)msg, key);
	return 0;
}

const char *
asl_get(aslmsg msg, const char *key)
{
	const char *val;
	int status;

	val = NULL;
	status = asl_msg_lookup((asl_msg_t *)msg, key, &val, NULL);
	if (status != 0) return NULL;
	return val;
}

void
asl_free(aslmsg msg)
{
	asl_msg_release((asl_msg_t *)msg);
}

/* aslresponse */

/*
 * aslresponse_next: Iterate over responses returned from asl_search()
 * a: a response returned from asl_search();
 * returns: The next log message (an aslmsg) or NULL on failure
 */
aslmsg
aslresponse_next(aslresponse r)
{
	asl_search_result_t *res;
	asl_msg_t *m;

	res = (asl_search_result_t *)r;
	if (res == NULL) return NULL;

	if (res->curr >= res->count) return NULL;
	m = res->msg[res->curr];
	res->curr++;

	return (aslmsg)m;
}

/*
 * aslresponse_free: Free a response returned from asl_search() 
 * a: a response returned from asl_search()
 */
void
aslresponse_free(aslresponse r)
{
	asl_search_result_t *res;
	uint32_t i;

	res = (asl_search_result_t *)r;
	if (res == NULL) return;

	for (i = 0; i < res->count; i++) asl_msg_release(res->msg[i]);
	free(res->msg);
	free(res);
}
