/*
 * Copyright (c) 2009 Apple Inc.  All rights reserved.
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kvbuf.h>

#define KVBUF_CHUNK 256

/*
 * kvbuf_t is a list of key/value dictionaries.
 *
 * First 4 bytes are the number of dictionaries.
 * For each dictionary, first 4 bytes is the key / value list count.
 * For each value list, first 4 bytes is the list length.
 * Keys and values are a 4-byte length followed by a nul-terminated string
 *
 * When the databuf needs to grow, we add memory in KVBUF_CHUNK size
 * increments to reduce malloc / realloc activity.
 * The _size variable stores the actual allocated size.
 * The datalen variable stores the used data size.
 *
 * The _dict variable holds an offset from the start of the buffer
 * to the "current" dictionary.  kvbuf_reset() resets this,
 * and kvbuf_next_dict() bumps the offset so that databuf + _dict
 * points to the next dictionary.
 *
 * The _key variable holds an offset from the start of the buffer
 * to the "current" key.  kvbuf_reset() resets this, and
 * kvbuf_next_key() bumps the offset so that databuf + _key
 * points to the next key.
 *
 * The _val variable holds an offset from the start of the buffer
 * to the "current" value.  kvbuf_reset() resets this, and
 * kvbuf_next_val() bumps the offset so that databuf + _val
 * points to the next value.
 *
 * The cache_entry_list_to_kvbuf() routine contains the only
 * code that builds an array.
 * 
 */

/*
 * kvbuf_query is a simple utility for constructing a
 * kvbuf with a single dictionary.  The format string may
 * contain the chars "k", "s", "i", and "u".  "k" denotes a key
 * (keys are always strings), "s" denotes a string value,
 * "i" denotes a 32 bit signed int, and "u" denotes an unsigned.
 */
kvbuf_t *
kvbuf_query(char *fmt, ...)
{
	va_list ap;
	char *arg, *f, str[32];
	int32_t iarg;
	uint32_t uarg;
	kvbuf_t *kv;

	if (fmt == NULL) return NULL;

	kv = kvbuf_new();
	if (kv == NULL) return NULL;

	kvbuf_add_dict(kv);

	va_start(ap, fmt);
	for (f = fmt; (*f) != '\0'; f++)
	{
		if (*f == 'k')
		{
			arg = va_arg(ap, char *);
			kvbuf_add_key(kv, arg);
		}
		else if (*f == 's')
		{
			arg = va_arg(ap, char *);
			kvbuf_add_val(kv, arg);
		}
		else if (*f == 'i')
		{
			iarg = va_arg(ap, int32_t);
			snprintf(str, sizeof(str), "%d", iarg);
			kvbuf_add_val(kv, str);
		}
		else if (*f == 'u')
		{
			uarg = va_arg(ap,uint32_t);
			snprintf(str, sizeof(str), "%u", uarg);
			kvbuf_add_val(kv, str);
		}
	}
	va_end(ap);

	return kv;
}

kvbuf_t *
kvbuf_query_key_val(const char *key, const char *val)
{
	kvbuf_t *kv;
	uint32_t x, kl, vl, vc;
	char *p;

	if (key == NULL) return NULL;

	kl = strlen(key) + 1;

	vl = 0;
	vc = 0;

	if (val != NULL) 
	{
		vl = strlen(val) + 1;
		vc = 1;
	}

	kv = (kvbuf_t *)calloc(1, sizeof(kvbuf_t));
	if (kv == NULL) return NULL;

	kv->_size = (5 * sizeof(uint32_t)) + kl + vl;
	kv->datalen = kv->_size;

	kv->databuf = calloc(1, kv->_size);
	if (kv->databuf == NULL)
	{
		free(kv);
		return NULL;
	}

	p = kv->databuf;

	/* 1 dict */
	x = htonl(1);
	memcpy(p, &x, sizeof(uint32_t));
	p += sizeof(uint32_t);

	/* 1 key */
	memcpy(p, &x, sizeof(uint32_t));
	p += sizeof(uint32_t);

	/* key length */
	x = htonl(kl);
	memcpy(p, &x, sizeof(uint32_t));
	p += sizeof(uint32_t);

	/* key */
	memcpy(p, key, kl);
	p += kl;

	/* number of values */
	x = htonl(vc);
	memcpy(p, &x, sizeof(uint32_t));
	p += sizeof(uint32_t);

	if (vc > 0)
	{
		/* value length */
		x = htonl(vl);
		memcpy(p, &x, sizeof(uint32_t));
		p += sizeof(uint32_t);

		/* value */
		memcpy(p, val, vl);
	}

	return kv;
}

kvbuf_t *
kvbuf_query_key_int(const char *key, int32_t i)
{
	char str[32];

	snprintf(str, sizeof(str), "%d", i);
	return kvbuf_query_key_val(key, str);
}

kvbuf_t *
kvbuf_query_key_uint(const char *key, uint32_t u)
{
	char str[32];

	snprintf(str, sizeof(str), "%u", u);
	return kvbuf_query_key_val(key, str);
}

kvbuf_t *
kvbuf_new_zone(malloc_zone_t *zone)
{
	kvbuf_t *kv;

	if (zone == NULL) return NULL;

	kv = (kvbuf_t *)malloc_zone_calloc(zone, 1, sizeof(kvbuf_t));
	if (kv == NULL) return NULL;

	kv->_size = KVBUF_START_SIZE;
	kv->databuf = malloc_zone_calloc(zone, 1, kv->_size);
	if (kv->databuf == NULL)
	{
		free(kv);
		return NULL;
	}

	kv->datalen = sizeof(uint32_t);
	kv->_dict = kv->datalen;

	return kv;
}

kvbuf_t *
kvbuf_new(void)
{
	return kvbuf_new_zone(malloc_default_zone());
}

kvbuf_t *
kvbuf_init_zone(malloc_zone_t *zone, char *buffer, uint32_t length)
{
	kvbuf_t *kv;

	if (zone == NULL) return NULL;

	kv = (kvbuf_t *)malloc_zone_calloc(zone, 1, sizeof(kvbuf_t));
	if (kv == NULL) return NULL;

	kv->_size = length;
	kv->datalen = length;
	if (length > 0)
	{
		kv->databuf = malloc_zone_calloc(zone, 1, length);
		if (kv->databuf == NULL)
		{
			free(kv);
			kv = NULL;
		}
		else
		{
			memcpy(kv->databuf, buffer, length);
		}
	}

	return kv;
}

kvbuf_t *
kvbuf_init(char *buffer, uint32_t length)
{
	return kvbuf_init_zone(malloc_default_zone(), buffer, length);
}

static void
kvbuf_grow(kvbuf_t *kv, uint32_t delta)
{
	uint32_t newlen;
	char *p, *newbuf;
	malloc_zone_t *zone;

	if (kv == NULL) return;
	if (delta == 0) return;

	if (kv->databuf == NULL) delta += sizeof(uint32_t);

	newlen = kv->datalen + delta;
	if (newlen <= kv->_size) return;

	kv->_size = ((newlen + KVBUF_CHUNK - 1) / KVBUF_CHUNK) * KVBUF_CHUNK;

	zone = malloc_zone_from_ptr(kv);
	if (kv->databuf == NULL)
	{
		kv->databuf = malloc_zone_calloc(zone, 1, kv->_size);
		if (kv->databuf == NULL)
		{
			memset(kv, 0, sizeof(kvbuf_t));
			return;
		}

		kv->datalen = sizeof(uint32_t);
		kv->_dict = sizeof(uint32_t);
	}
	else
	{
		newbuf = malloc_zone_realloc(zone, kv->databuf, kv->_size);
		if (newbuf == NULL) free(kv->databuf);
		kv->databuf = newbuf;
		if (kv->databuf == NULL)
		{
			memset(kv, 0, sizeof(kvbuf_t));
			return;
		}

		p = kv->databuf + kv->datalen;
		memset(p, 0, kv->_size - kv->datalen);
	}
}

void
kvbuf_add_dict(kvbuf_t *kv)
{
	char *p;
	uint32_t x, dict_count;

	if (kv == NULL) return;

	/* Add a key count */
	kvbuf_grow(kv, sizeof(uint32_t));
	if (kv->databuf == NULL) return;

	kv->_dict = kv->datalen;
	kv->datalen += sizeof(uint32_t);

	kv->_key = kv->datalen;
	kv->_vlist = 0;
	kv->_val = 0;

	/* increment and rewrite the dict count */
	p = kv->databuf;

	x = 0;
	memcpy(&x, p, sizeof(uint32_t));
	dict_count = ntohl(x);

	dict_count++;
	x = htonl(dict_count);
	memcpy(p, &x, sizeof(uint32_t));
}

void
kvbuf_add_key(kvbuf_t *kv, const char *key)
{
	uint32_t kl, x, key_count, delta;
	char *p;

	if (kv == NULL) return;
	if (key == NULL) return;

	kl = strlen(key) + 1;

	/* Grow to hold key len, key, and value list count. */
	delta = (2 * sizeof(uint32_t)) + kl;
	kvbuf_grow(kv, delta);

	if (kv->databuf == NULL) return;

	/* increment and rewrite the key count for the current dictionary */
	p = kv->databuf + kv->_dict;

	x = 0;
	memcpy(&x, p, sizeof(uint32_t));
	key_count = ntohl(x);

	if (key_count == 0) kv->_key = kv->_dict + sizeof(uint32_t);
	else kv->_key = kv->datalen;

	key_count++;
	x = htonl(key_count);
	memcpy(p, &x, sizeof(uint32_t));

	/* append key to data buffer */
	p = kv->databuf + kv->datalen;

	x = htonl(kl);
	memcpy(p, &x, sizeof(uint32_t));
	p += sizeof(uint32_t);
	memcpy(p, key, kl);
	p += kl;

	kv->_vlist = kv->datalen + sizeof(uint32_t) + kl;

	x = 0;
	memcpy(p, &x, sizeof(uint32_t));

	kv->datalen += delta;
	kv->_val = kv->datalen;
}

void
kvbuf_add_val_len(kvbuf_t *kv, const char *val, uint32_t len)
{
	uint32_t x, val_count, delta;
	char *p;

	if (kv == NULL) return;
	if (val == NULL) return;
	if (len == 0) return;

	/* Grow to hold val len and value. */
	delta = sizeof(uint32_t) + len;
	kvbuf_grow(kv, delta);

	if (kv->databuf == NULL) return;

	/* increment and rewrite the value count for the value_list dictionary */
	p = kv->databuf + kv->_vlist;

	x = 0;
	memcpy(&x, p, sizeof(uint32_t));
	val_count = ntohl(x);
	val_count++;
	x = htonl(val_count);
	memcpy(p, &x, sizeof(uint32_t));

	/* append val to data buffer */
	p = kv->databuf + kv->_val;

	x = htonl(len);
	memcpy(p, &x, sizeof(uint32_t));
	p += sizeof(uint32_t);
	memcpy(p, val, len);
	p += len;

	kv->datalen += delta;
	kv->_val = kv->datalen;
}

/* 
 * WARNING!  Kludge Alert!
 *
 * This call just looks for the buffer length encoded into a serialized kvbuf_t,
 * which preceeds a pointer to a key or value.  Obviously, calling it with anything
 * other than a pointer value which is embedded in a kvbuf_t is asking for trouble.
 */
uint32_t
kvbuf_get_len(const char *p)
{
	uint32_t x;

	x = 0;
	memcpy(&x, p - sizeof(uint32_t), sizeof(uint32_t));
	return ntohl(x);
}

void
kvbuf_add_val(kvbuf_t *kv, const char *val)
{
	if (kv == NULL) return;
	if (val == NULL) return;

	kvbuf_add_val_len(kv, val, strlen(val) + 1);
}

void
kvbuf_make_purgeable(kvbuf_t *kv)
{
	if (kv == NULL) return;

	if (kv->databuf != NULL) malloc_make_purgeable(kv->databuf);
}

int
kvbuf_make_nonpurgeable(kvbuf_t *kv)
{
	if (kv == NULL) return 0;

	/*
	 * malloc_make_nonpurgeable returns 0 even if memory was not
	 * allocated from the purgeable zone, so this is safe to call.
	 */
	if ((kv->databuf == NULL) || (malloc_make_nonpurgeable(kv->databuf) == 0)) return 0;

	/* return non-zero since something failed */
	return 1;
}

void
kvbuf_free(kvbuf_t *kv)
{
	if (kv == NULL) return;

	if (kv->databuf != NULL) free(kv->databuf);
	memset(kv, 0, sizeof(kvbuf_t));
	free(kv);
}

/* appends a kvbuf to an existing kvbuf */
void
kvbuf_append_kvbuf(kvbuf_t *kv, const kvbuf_t *kv2)
{
	uint32_t curr_count, new_count, temp;

	if (kv == NULL) return;
	if (kv2 == NULL) return;

	curr_count = 0;
	new_count = 0;

	memcpy(&temp, kv->databuf, sizeof(uint32_t));
	curr_count = ntohl(temp);

	memcpy(&temp, kv2->databuf, sizeof(uint32_t));
	new_count = ntohl(temp);

	/* nothing to do */
	if (new_count == 0) return;

	/* add the dictionary count to the current dictionary counts */
	curr_count += new_count; 

	temp = htonl(curr_count);
	memcpy(kv->databuf, &temp, sizeof(uint32_t));

	/* grow the current buffer so we can append the new buffer */
	temp = kv2->datalen - sizeof(uint32_t);

	kvbuf_grow(kv, temp);

	memcpy(kv->databuf + kv->datalen, kv2->databuf + sizeof(uint32_t), temp);
	kv->datalen += temp;
}

/* returns number of dictionaries */
uint32_t
kvbuf_reset(kvbuf_t *kv)
{
	uint32_t x;

	if (kv == NULL) return 0;
	if (kv->databuf == NULL) return 0;

	kv->_dict = 0;
	kv->_key = 0;
	kv->_vlist = 0;
	kv->_val = 0;

	if (kv->datalen < sizeof(uint32_t)) return 0;

	x = 0;
	memcpy(&x, kv->databuf, sizeof(uint32_t));
	return ntohl(x);
}

/* advance to next dictionary, returns key count */
uint32_t
kvbuf_next_dict(kvbuf_t *kv)
{
	uint32_t x, k, v, kcount, vcount, kl, vl;
	char *p;

	if (kv == NULL) return 0;
	if (kv->databuf == NULL) return 0;

	kv->_key = 0;
	kv->_vlist = 0;
	kv->_val = 0;

	if (kv->_dict == 0)
	{
		/* first dict */
		if (kv->datalen < sizeof(uint32_t)) return 0;
		kv->_dict = sizeof(uint32_t);

		if (kv->datalen < (kv->_dict + sizeof(uint32_t))) return 0;

		p = kv->databuf + kv->_dict;
		x = 0;
		memcpy(&x, p, sizeof(uint32_t));
		kcount = ntohl(x);

		return kcount;
	}

	p = kv->databuf + kv->_dict;

	x = 0;
	memcpy(&x, p, sizeof(uint32_t));
	p += sizeof(uint32_t);
	kv->_dict += sizeof(uint32_t);
	kcount = ntohl(x);

	for (k = 0; k < kcount; k++)
	{
		x = 0;
		memcpy(&x, p, sizeof(uint32_t));
		p += sizeof(uint32_t);
		kv->_dict += sizeof(uint32_t);
		kl = ntohl(x);
		p += kl;
		kv->_dict += kl;

		x = 0;
		memcpy(&x, p, sizeof(uint32_t));
		p += sizeof(uint32_t);
		kv->_dict += sizeof(uint32_t);
		vcount = ntohl(x);

		for (v = 0; v < vcount; v++)
		{
			x = 0;
			memcpy(&x, p, sizeof(uint32_t));
			p += sizeof(uint32_t);
			kv->_dict += sizeof(uint32_t);
			vl = ntohl(x);
			p += vl;
			kv->_dict += vl;
		}
	}

	if (kv->datalen < (kv->_dict + sizeof(uint32_t))) return 0;

	p = kv->databuf + kv->_dict;
	x = 0;
	memcpy(&x, p, sizeof(uint32_t));
	kcount = ntohl(x);

	return kcount;
}

/* advance to next key, returns key and sets val_count */
char *
kvbuf_next_key(kvbuf_t *kv, uint32_t *val_count)
{
	uint32_t x, kl, v, vl, vc;
	char *p, *out;

	if (kv == NULL) return NULL;
	if (val_count == NULL) return NULL;

	*val_count = 0;

	if (kv->databuf == NULL) return NULL;
	if (kv->_dict == 0) return NULL;

	kv->_vlist = 0;
	kv->_val = 0;

	if (kv->_key == 0)
	{
		/* first key */
		if (kv->datalen < (kv->_dict +  sizeof(uint32_t))) return NULL;
		kv->_key = kv->_dict + sizeof(uint32_t);
	}
	else
	{
		p = kv->databuf + kv->_key;

		x = 0;
		memcpy(&x, p, sizeof(uint32_t));
		kl = ntohl(x);

		if (kv->datalen < (kv->_key + sizeof(uint32_t) + kl)) return NULL;

		p += (sizeof(uint32_t) + kl);
		kv->_key += (sizeof(uint32_t) + kl);

		/* skip over values */
		if (kv->datalen < (kv->_key + sizeof(uint32_t))) return NULL;

		x = 0;
		memcpy(&x, p, sizeof(uint32_t));
		vc = ntohl(x);

		p += sizeof(uint32_t);
		kv->_key += sizeof(uint32_t);

		for (v = 0; v < vc; v++)
		{
			if (kv->datalen < (kv->_key + sizeof(uint32_t))) return NULL;

			x = 0;
			memcpy(&x, p, sizeof(uint32_t));
			vl = ntohl(x);

			if (kv->datalen < (kv->_key + kl)) return NULL;

			p += (sizeof(uint32_t) + vl);
			kv->_key += (sizeof(uint32_t) + vl);
		}
	}

	if (kv->datalen < (kv->_key + sizeof(uint32_t))) return NULL;

	p = kv->databuf + kv->_key;
	x = 0;
	memcpy(&x, p, sizeof(uint32_t));
	kl = ntohl(x);

	p += sizeof(uint32_t);
	out = p;

	kv->_vlist = kv->_key + sizeof(uint32_t) + kl;
	if (kv->datalen < (kv->_vlist + sizeof(uint32_t)))
	{
		kv->_vlist = 0;
		return NULL;
	}

	p = kv->databuf + kv->_vlist;
	x = 0;
	memcpy(&x, p, sizeof(uint32_t));
	*val_count = ntohl(x);

	return out;
}

char *
kvbuf_next_val(kvbuf_t *kv)
{
	return kvbuf_next_val_len(kv, NULL);
}

char *
kvbuf_next_val_len(kvbuf_t *kv, uint32_t *len)
{
	uint32_t x = 0;
	uint32_t vltemp = 0;
	char *p;

	if (kv == NULL) return NULL;
	if (kv->databuf == NULL) return NULL;
	if (kv->_vlist == 0) return NULL;

	if (kv->_val == 0)
	{
		/* first val */
		if (kv->datalen < (kv->_vlist +  sizeof(uint32_t))) return NULL;
		kv->_val = kv->_vlist + sizeof(uint32_t);

		p = kv->databuf + kv->_val;

		memcpy(&x, p, sizeof(uint32_t));
		vltemp = ntohl(x);
	}
	else
	{
		p = kv->databuf + kv->_val;

		memcpy(&x, p, sizeof(uint32_t));
		vltemp = ntohl(x);

		if (kv->datalen < (kv->_val + sizeof(uint32_t) + vltemp)) return NULL;

		p += (sizeof(uint32_t) + vltemp);
		kv->_val += (sizeof(uint32_t) + vltemp);
	}

	if (kv->datalen < (kv->_val + sizeof(uint32_t))) return NULL;

	if (len != NULL) (*len) = vltemp;
	p = kv->databuf + kv->_val + sizeof(uint32_t);
	return p;
}

/*
 * Builds a kvarray_t / kvdict_t structure on top of a kvbuf_t. 
 * It allocates the appropriate number of kvdict_t structures
 * for the array, sets all the counters, and fills in pointers
 * for keys and valuse.  The pointers are NOT to newly allocated
 * strings: they just point into the kvbuf data buffer.
 *
 * To dispose of the kvarray_t and all of the associated
 * memory AND to free the original kvbuf, clients only
 * need to call kvarray_free().
 */
kvarray_t *
kvbuf_decode(kvbuf_t *kv)
{
	kvarray_t *a;
	uint32_t x, d, k, v;
	char *p;

	if (kv == NULL) return NULL;
	if (kv->databuf == NULL) return NULL;

	if (kv->datalen < sizeof(uint32_t)) return NULL;

	p = kv->databuf;
	kv->_size = kv->datalen;

	/* array count */
	x = 0;
	memcpy(&x, p, sizeof(uint32_t));
	p += sizeof(uint32_t);
	kv->_size -= sizeof(uint32_t);
	x = ntohl(x);

	if (x == 0) return NULL;

	a = (kvarray_t *)calloc(1, sizeof(kvarray_t));
	if (a == NULL) return NULL;

	a->count = x;
	a->dict = (kvdict_t *)calloc(a->count, sizeof(kvdict_t));
	if (a->dict == NULL)
	{
		free(a);
		return NULL;
	}

	for (d = 0; d < a->count; d++)
	{
		if (kv->_size < sizeof(uint32_t))
		{
			kvarray_free(a);
			return NULL;
		}

		/* key count */
		x = 0;
		memcpy(&x, p, sizeof(uint32_t));
		p += sizeof(uint32_t);
		kv->_size -= sizeof(uint32_t);
		a->dict[d].kcount = ntohl(x);

		if (a->dict[d].kcount > 0)
		{
			a->dict[d].key = (const char **)calloc(a->dict[d].kcount, sizeof(const char *));
			if (a->dict[d].key == NULL)
			{
				kvarray_free(a);
				return NULL;
			}

			a->dict[d].vcount = (uint32_t *)calloc(a->dict[d].kcount, sizeof(uint32_t));
			if (a->dict[d].vcount == NULL)
			{
				kvarray_free(a);
				return NULL;
			}

			a->dict[d].val = (const char ***)calloc(a->dict[d].kcount, sizeof(char **));
			if (a->dict[d].val == NULL)
			{
				kvarray_free(a);
				return NULL;
			}
		}

		for (k = 0; k < a->dict[d].kcount; k++)
		{
			/* get key */
			if (kv->_size < sizeof(uint32_t))
			{
				kvarray_free(a);
				return NULL;
			}

			/* key length */
			x = 0;
			memcpy(&x, p, sizeof(uint32_t));
			p += sizeof(uint32_t);
			kv->_size -= sizeof(uint32_t);
			x = ntohl(x);

			if (kv->_size < x)
			{
				kvarray_free(a);
				return NULL;
			}

			/* key data */
			a->dict[d].key[k] = p;

			p += x;
			kv->_size -= x;

			if (kv->_size < sizeof(uint32_t))
			{
				kvarray_free(a);
				return NULL;
			}

			/* val count */
			x = 0;
			memcpy(&x, p, sizeof(uint32_t));
			p += sizeof(uint32_t);
			kv->_size -= sizeof(uint32_t);
			a->dict[d].vcount[k] = ntohl(x);

			if (a->dict[d].vcount[k] > 0)
			{
				/* N.B. we add a NULL pointer at the end of the list */
				a->dict[d].val[k] = (const char **)calloc(a->dict[d].vcount[k] + 1, sizeof(const char *));
				if (a->dict[d].val[k] == NULL)
				{
					kvarray_free(a);
					return NULL;
				}
			}

			for (v = 0; v < a->dict[d].vcount[k]; v++)
			{
				/* get val */
				if (kv->_size < sizeof(uint32_t))
				{
					kvarray_free(a);
					return NULL;
				}

				/* val length */
				x = 0;
				memcpy(&x, p, sizeof(uint32_t));
				p += sizeof(uint32_t);
				kv->_size -= sizeof(uint32_t);
				x = ntohl(x);

				if (kv->_size < x)
				{
					kvarray_free(a);
					return NULL;
				}

				/* val data */
				a->dict[d].val[k][v] = p;

				p += x;
				kv->_size -= x;
			}
		}
	}

	a->kv = kv;
	return a;
}

void
kvarray_free(kvarray_t *a)
{
	uint32_t d, k;

	if (a == NULL) return;

	for (d = 0; d < a->count; d++)
	{
		for (k = 0; k < a->dict[d].kcount; k++)
		{
			if (a->dict[d].val == NULL) continue;
			if (a->dict[d].val[k] != NULL) free(a->dict[d].val[k]);
		}

		if (a->dict[d].key != NULL) free(a->dict[d].key);
		if (a->dict[d].vcount != NULL) free(a->dict[d].vcount);
		if (a->dict[d].val != NULL) free(a->dict[d].val);
	}

	a->count = 0;

	if (a->dict != NULL) free(a->dict);
	a->dict = NULL;

	if (a->kv != NULL) kvbuf_free(a->kv);
	a->kv = NULL;

	free(a);
}

