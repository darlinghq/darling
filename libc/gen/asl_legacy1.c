/*
 * Copyright (c) 2007-2011 Apple Inc.  All rights reserved.
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
 * @APPLE_LICENSE_HEADER_END@ */

#include <asl_core.h>
#include <asl_legacy1.h>
#include <asl_private.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <string.h>
#include <membership.h>
#include <mach/mach.h>
#include <sys/syslimits.h>
#include <sys/types.h>
#include <time.h>
#include <sys/mman.h>

#define forever for(;;)

#define FILE_MODE 0600

#define DB_RECORD_LEN 80

#define DB_HEADER_COOKIE_OFFSET 0
#define DB_HEADER_VERS_OFFSET 12

#define DB_TYPE_EMPTY   0
#define DB_TYPE_HEADER  1
#define DB_TYPE_MESSAGE 2
#define DB_TYPE_KVLIST  3
#define DB_TYPE_STRING  4
#define DB_TYPE_STRCONT 5

/*
 * Magic Cookie for database files.
 * MAXIMUM 12 CHARS! (DB_HEADER_VERS_OFFSET)
 */
#define ASL_DB_COOKIE "ASL DB"
#define ASL_DB_COOKIE_LEN 6

#define ASL_INDEX_NULL 0xffffffff

#define DB_HLEN_EMPTY    0
#define DB_HLEN_HEADER  13
#define DB_HLEN_MESSAGE 13
#define DB_HLEN_KVLIST   9
#define DB_HLEN_STRING  25
#define DB_HLEN_STRCONT  5

#define MSG_OFF_KEY_TYPE 0
#define MSG_OFF_KEY_NEXT 1
#define MSG_OFF_KEY_ID 5
#define MSG_OFF_KEY_RUID 13
#define MSG_OFF_KEY_RGID 17
#define MSG_OFF_KEY_TIME 21
#define MSG_OFF_KEY_HOST 29
#define MSG_OFF_KEY_SENDER 37
#define MSG_OFF_KEY_FACILITY 45
#define MSG_OFF_KEY_LEVEL 53
#define MSG_OFF_KEY_PID 57
#define MSG_OFF_KEY_UID 61
#define MSG_OFF_KEY_GID 65
#define MSG_OFF_KEY_MSG 69
#define MSG_OFF_KEY_FLAGS 77

extern time_t asl_parse_time(const char *str);
extern int asl_msg_cmp(aslmsg a, aslmsg b);

#define asl_msg_list_t asl_search_result_t

#define Q_NULL 100001
#define Q_FAST 100002
#define Q_SLOW 100003
#define Q_FAIL 100004

static uint64_t
_asl_htonq(uint64_t n)
{
#ifdef __BIG_ENDIAN__
	return n;
#else
	u_int32_t t;
	union
	{
		u_int64_t q;
		u_int32_t l[2];
	} x;

	x.q = n;
	t = x.l[0];
	x.l[0] = htonl(x.l[1]);
	x.l[1] = htonl(t);

	return x.q;
#endif
}

static uint64_t
_asl_ntohq(uint64_t n)
{
#ifdef __BIG_ENDIAN__
	return n;
#else
	u_int32_t t;
	union
	{
		u_int64_t q;
		u_int32_t l[2];
	} x;

	x.q = n;
	t = x.l[0];
	x.l[0] = ntohl(x.l[1]);
	x.l[1] = ntohl(t);

	return x.q;
#endif
}

static uint16_t
_asl_get_16(char *h)
{
	uint16_t x;

	memcpy(&x, h, 2);
	return ntohs(x);
}

static uint32_t
_asl_get_32(char *h)
{
	uint32_t x;

	memcpy(&x, h, 4);
	return ntohl(x);
}

static uint64_t
_asl_get_64(char *h)
{
	uint64_t x;

	memcpy(&x, h, 8);
	return _asl_ntohq(x);
}

#define header_get_next(h)		_asl_get_32(h +  1)
#define header_get_id(h)		_asl_get_64(h +  5)
#define header_get_hash(h)		_asl_get_32(h + 17)

/*
 * callback for sorting slotlist
 * primary sort is by xid
 * secondary sort is by slot, which happens when xid is 0
 * this allows us to quickly find xids (using binary search on the xid key)
 * it's also used to find slots quickly from record_chain_free()
 */
static int
slot_comp(const void *a, const void *b)
{
	asl_legacy1_slot_info_t *ai, *bi;

	if (a == NULL)
	{
		if (b == NULL) return 0;
		return -1;
	}

	if (b == NULL) return 1;

	ai = (asl_legacy1_slot_info_t *)a;
	bi = (asl_legacy1_slot_info_t *)b;

	if (ai->xid < bi->xid) return -1;

	if (ai->xid == bi->xid)
	{
		if (ai->slot < bi->slot) return -1;
		if (ai->slot == bi->slot) return 0;
		return 1;
	}

	return 1;
}

/* find an xid in the slot list */
static uint32_t
slotlist_find(asl_legacy1_t *s, uint64_t xid, int32_t direction)
{
	uint32_t top, bot, mid, range;

	if (s == NULL) return ASL_INDEX_NULL;
	if (s->slotlist_count == 0) return ASL_INDEX_NULL;
	if (xid == 0) return ASL_INDEX_NULL;

	top = s->slotlist_count - 1;
	bot = 0;
	mid = top / 2;

	range = top - bot;
	while (range > 1)
	{
		if (xid == s->slotlist[mid].xid) return mid;
		else if (xid < s->slotlist[mid].xid) top = mid;
		else bot = mid;

		range = top - bot;
		mid = bot + (range / 2);
	}

	if (xid == s->slotlist[top].xid) return top;
	if (xid == s->slotlist[bot].xid) return bot;

	if (direction == 0) return ASL_INDEX_NULL;
	if (direction < 0) return bot;
	return top;
}

static uint32_t
slotlist_init(asl_legacy1_t *s, uint32_t count)
{
	uint32_t i, si, status, hash, addslot;
	uint64_t xid;
	uint8_t t;
	size_t rcount;
	char tmp[DB_RECORD_LEN];

	/* Start at first slot after the header */
	status = fseek(s->db, DB_RECORD_LEN, SEEK_SET);
	if (status != 0) return ASL_STATUS_READ_FAILED;

	s->slotlist = (asl_legacy1_slot_info_t *)calloc(count, sizeof(asl_legacy1_slot_info_t));
	if (s->slotlist == NULL) return ASL_STATUS_NO_MEMORY;

	si = 0;

	for (i = 1; i < count; i++)
	{
		rcount = fread(tmp, DB_RECORD_LEN, 1, s->db);
		if (rcount != 1) return ASL_STATUS_READ_FAILED;

		t = tmp[0];
		addslot = 0;
		xid = 0;
		hash = 0;

		if (t == DB_TYPE_EMPTY) addslot = 1;

		if (t == DB_TYPE_STRING)
		{
			addslot = 1;
			xid = header_get_id(tmp);
			hash = header_get_hash(tmp);
		}

		if (t == DB_TYPE_MESSAGE)
		{
			addslot = 1;
			xid = header_get_id(tmp);
		}

		if (addslot == 1)
		{
			s->slotlist[si].type = t;
			s->slotlist[si].slot = i;
			s->slotlist[si].xid = xid;
			s->slotlist[si].hash = hash;
			si++;
		}
	}

	s->slotlist = (asl_legacy1_slot_info_t *)reallocf(s->slotlist, si * sizeof(asl_legacy1_slot_info_t));
	if (s->slotlist == NULL) return ASL_STATUS_NO_MEMORY;
	s->slotlist_count = si;

	/* slotlist is sorted by xid */
	qsort((void *)s->slotlist, s->slotlist_count, sizeof(asl_legacy1_slot_info_t), slot_comp);

	return ASL_STATUS_OK;
}

uint32_t
asl_legacy1_open(const char *path, asl_legacy1_t **out)
{
	asl_legacy1_t *s;
	struct stat sb;
	int status;
	size_t rcount;
	char cbuf[DB_RECORD_LEN];
	off_t fsize;
	uint32_t count;

	memset(&sb, 0, sizeof(struct stat));
	status = stat(path, &sb);
	if (status < 0) return ASL_STATUS_FAILED;

	fsize = sb.st_size;

	s = (asl_legacy1_t *)calloc(1, sizeof(asl_legacy1_t));
	if (s == NULL) return ASL_STATUS_NO_MEMORY;

	s->db = fopen(path, "r");
	if (s->db == NULL)
	{
		free(s);
		return ASL_STATUS_INVALID_STORE;
	}

	memset(cbuf, 0, DB_RECORD_LEN);
	rcount = fread(cbuf, DB_RECORD_LEN, 1, s->db);
	if (rcount != 1)
	{
		fclose(s->db);
		free(s);
		return ASL_STATUS_READ_FAILED;
	}

	/* Check the database Magic Cookie */
	if (strncmp(cbuf, ASL_DB_COOKIE, ASL_DB_COOKIE_LEN))
	{
		fclose(s->db);
		free(s);
		return ASL_STATUS_INVALID_STORE;
	}

	count = fsize / DB_RECORD_LEN;

	status = slotlist_init(s, count);

	*out = s;
	return ASL_STATUS_OK;
}

uint32_t
asl_legacy1_close(asl_legacy1_t *s)
{
	if (s == NULL) return ASL_STATUS_INVALID_STORE;

	if (s->slotlist != NULL) free(s->slotlist);
	if (s->db != NULL) fclose(s->db);
	free(s);

	return ASL_STATUS_OK;
}

static uint32_t
string_fetch_slot(asl_legacy1_t *s, uint32_t slot, char **out)
{
	off_t offset;
	uint8_t type;
	uint32_t next, x, remaining;
	size_t rcount, len;
	int status;
	char *outstr, *p, tmp[DB_RECORD_LEN];

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (out == NULL) return ASL_STATUS_INVALID_ARG;

	*out = NULL;
	offset = slot * DB_RECORD_LEN;
	status = fseek(s->db, offset, SEEK_SET);

	if (status < 0) return ASL_STATUS_READ_FAILED;

	rcount = fread(tmp, DB_RECORD_LEN, 1, s->db);
	if (rcount != 1) return ASL_STATUS_READ_FAILED;

	type = tmp[0];
	if (type != DB_TYPE_STRING) return ASL_STATUS_INVALID_STRING;

	len = _asl_get_32(tmp + 21);
	if (len == 0) return ASL_STATUS_OK;

	next = header_get_next(tmp);

	outstr = calloc(1, len);
	if (outstr == NULL) return ASL_STATUS_NO_MEMORY;

	p = outstr;
	remaining = len;

	x = DB_RECORD_LEN - DB_HLEN_STRING;
	if (x > remaining) x = remaining;

	memcpy(p, tmp + DB_HLEN_STRING, x);
	p += x;
	remaining -= x;

	while ((next != 0) && (remaining > 0))
	{
		offset = next * DB_RECORD_LEN;
		status = fseek(s->db, offset, SEEK_SET);

		if (status < 0)
		{
			free(outstr);
			return ASL_STATUS_READ_FAILED;
		}

		rcount = fread(tmp, DB_RECORD_LEN, 1, s->db);
		if (rcount != 1)
		{
			free(outstr);
			return ASL_STATUS_READ_FAILED;
		}

		next = header_get_next(tmp);

		x = DB_RECORD_LEN - DB_HLEN_STRCONT;
		if (x > remaining) x = remaining;

		memcpy(p, tmp + DB_HLEN_STRCONT, x);
		p += x;
		remaining -= x;
	}

	if ((next != 0) || (remaining != 0))
	{
		free(outstr);
		return ASL_STATUS_READ_FAILED;
	}

	*out = outstr;
	return ASL_STATUS_OK;
}

static uint32_t
string_fetch_sid(asl_legacy1_t *s, uint64_t sid, char **out)
{
	uint32_t i, len, ref;
	uint64_t nsid;
	uint8_t inls;
	char *p;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (out == NULL) return ASL_STATUS_INVALID_ARG;

	*out = NULL;
	if (sid == ASL_REF_NULL) return ASL_STATUS_OK;

	ref = 0;

	inls = 0;
	nsid = _asl_htonq(sid);
	memcpy(&inls, &nsid, 1);
	if (inls & 0x80)
	{
		/* inline string */
		inls &= 0x0f;
		len = inls;
		*out = calloc(1, len);
		if (*out == NULL) return ASL_STATUS_NO_MEMORY;
		p = 1 + (char *)&nsid;
		memcpy(*out, p, len);
		return ASL_STATUS_OK;
	}

	/* Find the string in the database */
	i = slotlist_find(s, sid, 0);
	if (i == ASL_INDEX_NULL) return ASL_STATUS_NOT_FOUND;

	return string_fetch_slot(s, s->slotlist[i].slot, out);
}

static uint32_t
asl_legacy1_fetch_helper_32(asl_legacy1_t *s, char **p, aslmsg m, const char *key, int ignore, uint32_t ignoreval)
{
	uint32_t out, doit;
	char str[256];

	out = _asl_get_32(*p);
	*p += sizeof(uint32_t);

	if ((m == NULL) || (key == NULL)) return out;

	doit = 1;
	if ((ignore != 0) && (out == ignoreval)) doit = 0;
	if (doit != 0)
	{
		snprintf(str, sizeof(str), "%u", out);
		asl_set(m, key, str);
	}

	return out;
}

static uint64_t
asl_legacy1_fetch_helper_64(asl_legacy1_t *s, char **p, aslmsg m, const char *key)
{
	uint64_t out;
	char str[256];

	out = _asl_get_64(*p);
	*p += sizeof(uint64_t);

	if ((m == NULL) || (key == NULL)) return out;

	snprintf(str, sizeof(str), "%llu", out);
	asl_set(m, key, str);

	return out;
}

static uint64_t
asl_legacy1_fetch_helper_str(asl_legacy1_t *s, char **p, aslmsg m, const char *key, uint32_t *err)
{
	uint64_t out;
	char *val;
	uint32_t status;

	out = _asl_get_64(*p);
	*p += sizeof(uint64_t);

	val = NULL;
	status = ASL_STATUS_OK;
	if (out != 0) status = string_fetch_sid(s, out, &val);

	if (err != NULL) *err = status;
	if ((status == ASL_STATUS_OK) && (val != NULL))
	{
		asl_set(m, key, val);
		free(val);
	}

	return out;
}

static uint32_t
msg_fetch(asl_legacy1_t *s, uint32_t slot, aslmsg *out)
{
	off_t offset;
	uint32_t status, i, n, kvcount, next;
	uint16_t flags;
	uint64_t sid;
	size_t rcount;
	aslmsg msg;
	int fstatus;
	char *p, tmp[DB_RECORD_LEN], *key, *val;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (out == NULL) return ASL_STATUS_INVALID_ARG;

	*out = NULL;

	offset = slot * DB_RECORD_LEN;
	fstatus = fseek(s->db, offset, SEEK_SET);

	if (fstatus < 0) return ASL_STATUS_READ_FAILED;

	rcount = fread(tmp, DB_RECORD_LEN, 1, s->db);
	if (rcount != 1) return ASL_STATUS_READ_FAILED;

	flags = _asl_get_16(tmp + MSG_OFF_KEY_FLAGS);

	msg = asl_new(ASL_TYPE_MSG);
	if (msg == NULL) return ASL_STATUS_NO_MEMORY;

	p = tmp + 5;

	asl_legacy1_fetch_helper_64(s, &p, msg, ASL_KEY_MSG_ID);
	asl_legacy1_fetch_helper_32(s, &p, msg, ASL_KEY_READ_UID, 1, (uint32_t)-1);
	asl_legacy1_fetch_helper_32(s, &p, msg, ASL_KEY_READ_GID, 1, (uint32_t)-1);
	asl_legacy1_fetch_helper_64(s, &p, msg, ASL_KEY_TIME);
	asl_legacy1_fetch_helper_str(s, &p, msg, ASL_KEY_HOST, &status);
	asl_legacy1_fetch_helper_str(s, &p, msg, ASL_KEY_SENDER, &status);
	asl_legacy1_fetch_helper_str(s, &p, msg, ASL_KEY_FACILITY, &status);
	asl_legacy1_fetch_helper_32(s, &p, msg, ASL_KEY_LEVEL, 0, 0);
	asl_legacy1_fetch_helper_32(s, &p, msg, ASL_KEY_PID, 0, 0);
	asl_legacy1_fetch_helper_32(s, &p, msg, ASL_KEY_UID, 0, 0);
	asl_legacy1_fetch_helper_32(s, &p, msg, ASL_KEY_GID, 0, 0);
	asl_legacy1_fetch_helper_str(s, &p, msg, ASL_KEY_MSG, &status);

	next = header_get_next(tmp);

	kvcount = 0;
	n = 0;

	while (next != 0)
	{
		offset = next * DB_RECORD_LEN;
		fstatus = fseek(s->db, offset, SEEK_SET);
		if (fstatus < 0)
		{
			free(out);
			return ASL_STATUS_READ_FAILED;
		}

		rcount = fread(tmp, DB_RECORD_LEN, 1, s->db);
		if (rcount != 1)
		{
			free(out);
			return ASL_STATUS_READ_FAILED;
		}

		if (kvcount == 0) kvcount = _asl_get_32(tmp + 5);

		p = tmp + 9;

		for (i = 0; (i < 4) && (n < kvcount); i++)
		{
			key = NULL;
			sid = _asl_get_64(p);
			p += 8;
			status = string_fetch_sid(s, sid, &key);

			val = NULL;
			sid = _asl_get_64(p);
			p += 8;
			if (status == ASL_STATUS_OK) status = string_fetch_sid(s, sid, &val);

			if ((status == ASL_STATUS_OK) && (key != NULL)) asl_set(msg, key, val);
			if (key != NULL) free(key);
			if (val != NULL) free(val);

			n++;
		}

		next = header_get_next(tmp);
	}

	*out = msg;
	return ASL_STATUS_OK;
}

uint32_t
asl_legacy1_fetch(asl_legacy1_t *s, uint64_t msgid, aslmsg *out)
{
	uint32_t i, status;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (msgid == ASL_REF_NULL) return ASL_STATUS_INVALID_ARG;
	if (out == NULL) return ASL_STATUS_INVALID_ARG;

	i = slotlist_find(s, msgid, 0);
	if (i == ASL_INDEX_NULL) return ASL_STATUS_INVALID_ID;

	/* read the message */
	status = msg_fetch(s, s->slotlist[i].slot, out);
	if (status != ASL_STATUS_OK) return status;
	if (*out == NULL) return ASL_STATUS_FAILED;

	return status;
}

static uint32_t
next_search_slot(asl_legacy1_t *s, uint32_t last_si, int32_t direction)
{
	uint32_t i;

	if (direction >= 0)
	{
		for (i = last_si + 1; i < s->slotlist_count; i++)
		{
			if (s->slotlist[i].type == DB_TYPE_MESSAGE) return i;
		}

		return ASL_INDEX_NULL;
	}

	if (last_si == 0) return ASL_INDEX_NULL;
	if (last_si > s->slotlist_count) return ASL_INDEX_NULL;

	for (i = last_si - 1; i > 0; i--)
	{
		if (s->slotlist[i].type == DB_TYPE_MESSAGE) return i;
	}

	if (s->slotlist[0].type == DB_TYPE_MESSAGE) return 0;

	return ASL_INDEX_NULL;
}

static void
match_worker_cleanup(asl_msg_list_t **res)
{
	uint32_t i;

	if (res != NULL)
	{
		for (i = 0; i < (*res)->count; i++) asl_free((aslmsg)(*res)->msg[i]);
		free(*res);
	}
}

/*
 * Input to asl_legacy1_match is a list of queries.
 * A record in the store matches if it matches any query (i.e. query list is "OR"ed)
 *
 * If counting up (direction is positive) find first record with ID > start_id.
 * Else if counting down (direction is negative) find first record with ID < start_id.
 *
 * Set match flag on.
 * If any query is NULL, set match flog off (skips matching below).
 * Else if all queries only check "standard" keys, set std flag to on.
 *
 * If all queries are marked as "never matches", return NULL.
 *
 * match loop:
 *  fetch record (with std flag)
 *  if match flag is off, decode record and add it to result.
 *  else for each query:
 *    if query is NULL (shouldn't happen) decode record and add it to result.  Return to match loop.
 *    else if query never matches, ignore it.
 *    else decode record and use asl_cmp.  If it succeeds, add record to result.  Return to match loop.
 *
 * return results.
 */
static uint32_t
match_worker(asl_legacy1_t *s, asl_msg_list_t *query, asl_msg_list_t **res, uint64_t *last_id, uint64_t **idlist, uint32_t *idcount, uint64_t start_id, int32_t count, int32_t direction)
{
	uint32_t mx, si, slot, i, qcount, match, didmatch, status;
	uint64_t xid;
	aslmsg msg;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if ((res == NULL) && (idlist == NULL)) return ASL_STATUS_INVALID_ARG;
	if (last_id == NULL) return ASL_STATUS_INVALID_ARG;
	if (idcount == NULL) return ASL_STATUS_INVALID_ARG;

	if (res != NULL) *res = NULL;
	if (idlist != NULL) *idlist = NULL;

	mx = 0;

	if (direction < 0) direction = -1;
	else direction = 1;

	si = ASL_INDEX_NULL;
	if ((direction == -1) && (start_id == ASL_REF_NULL)) si = s->slotlist_count;
	else si = slotlist_find(s, start_id, direction);

	si = next_search_slot(s, si, direction);
	if (si == ASL_INDEX_NULL) return ASL_STATUS_OK;
	if (si >= s->slotlist_count) return ASL_STATUS_FAILED;

	slot = s->slotlist[si].slot;

	match = 1;
	qcount = 0;

	if (query == NULL) match = 0;
	else if (query->count == 0) match = 0;
	else qcount = query->count;

	/*
	 * initialize result list if we've been asked to return messages
	 */
	if (res != NULL)
	{
		*res = (asl_msg_list_t *)calloc(1, sizeof(asl_msg_list_t));
		if (*res == NULL) return ASL_STATUS_NO_MEMORY;
	}

	status = ASL_STATUS_OK;

	/*
	 * loop through records
	 */
	*idcount = 0;
	while ((count == 0) || (*idcount < count))
	{
		if (si == ASL_INDEX_NULL) break;
		if (si >= s->slotlist_count) break;

		slot = s->slotlist[si].slot;
		xid = s->slotlist[si].xid;

		*last_id = xid;

		status = msg_fetch(s, slot, &msg);

		didmatch = 0;
		if (match == 0)
		{
			didmatch = 1;
		}
		else
		{
			for (i = 0; i < qcount; i++)
			{
				didmatch = asl_msg_cmp((aslmsg)(query->msg[i]), msg);
				if (didmatch == 1) break;
			}
		}

		if (didmatch == 1)
		{
			if ((*res)->count == 0) (*res)->msg = (asl_msg_t **)calloc(1, sizeof(asl_msg_t *));
			else (*res)->msg = (asl_msg_t **)reallocf((*res)->msg, (1 + (*res)->count) * sizeof(asl_msg_t *));
			if ((*res)->msg == NULL)
			{
				match_worker_cleanup(res);
				return ASL_STATUS_NO_MEMORY;
			}

			(*res)->msg[(*res)->count++] = (asl_msg_t *)msg;
		}
		else
		{
			asl_free(msg);
		}

		si = next_search_slot(s, si, direction);
	}

	return status;
}

uint32_t
asl_legacy1_match(asl_legacy1_t *s, asl_msg_list_t *query, asl_msg_list_t **res, uint64_t *last_id, uint64_t start_id, uint32_t count, int32_t direction)
{
	uint32_t idcount;

	idcount = 0;
	return match_worker(s, query, res, last_id, NULL, &idcount, start_id, count, direction);
}
