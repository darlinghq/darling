/*
 * Copyright (c) 2007-2011 Apple Inc. All rights reserved.
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

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <asl.h>
#include <asl_private.h>
#include <asl_core.h>
#include <asl_store.h>
#include <notify.h>

extern time_t asl_parse_time(const char *str);
extern uint64_t asl_file_cursor(asl_file_t *s);
extern uint32_t asl_file_match_start(asl_file_t *s, uint64_t start_id, int32_t direction);
extern uint32_t asl_file_match_next(asl_file_t *s, aslresponse query, asl_msg_t **msg, uint64_t *last_id, int32_t direction, int32_t ruid, int32_t rgid);
extern int asl_file_create(const char *path, uid_t uid, gid_t gid, mode_t mode);

#define SECONDS_PER_DAY 86400

/* 
 * The ASL Store is organized as a set of files in a common directory.
 * Files are prefixed by the date (YYYY.MM.DD) of their contents.
 *
 * Messages with no access controls are saved in YYYY.MM.DD.asl
 * Messages with access limited to UID uuu are saved in YYYY.MM.DD.Uuuu.asl
 * Messages with access limited to GID ggg are saved in YYYY.MM.DD.Gggg.asl
 * Messages with access limited to UID uuu and GID ggg are saved in YYYY.MM.DD.Uuuu.Gggg.asl
 *
 * Messages that have a value for ASLExpireTime are saved in BB.YYYY.MM.DD.asl
 * where the timestamp is the "Best Before" date of the file.  Access controls
 * are implemented as above with Uuuu and Gggg in the file name.  Note that the
 * Best Before files are for the last day of the month, so a single file contains
 * messages that expire in that month.
 *
 * An external tool runs daily and deletes "old" files.
 */

static time_t
_asl_start_today()
{
	time_t now;
	struct tm ctm;

	memset(&ctm, 0, sizeof(struct tm));
	now = time(NULL);

	if (localtime_r((const time_t *)&now, &ctm) == NULL) return 0;

	ctm.tm_sec = 0;
	ctm.tm_min = 0;
	ctm.tm_hour = 0;

	return mktime(&ctm);
}

/*
 * The base directory contains a data file which stores
 * the last record ID.
 *
 * | MAX_ID (uint64_t) |
 *
 */
uint32_t
asl_store_open_write(const char *basedir, asl_store_t **s)
{
	asl_store_t *out;
	struct stat sb;
	uint32_t i, flags;
	char *path;
	FILE *sd;
	uint64_t last_id;
	time_t start;

	if (s == NULL) return ASL_STATUS_INVALID_ARG;

	start = _asl_start_today();
	if (start == 0) return ASL_STATUS_FAILED;

	if (basedir == NULL) basedir = PATH_ASL_STORE;

	memset(&sb, 0, sizeof(struct stat));
	if (stat(basedir, &sb) != 0) return ASL_STATUS_INVALID_STORE;
	if (!S_ISDIR(sb.st_mode)) return ASL_STATUS_INVALID_STORE;

	path = NULL;
	asprintf(&path, "%s/%s", basedir, FILE_ASL_STORE_DATA);
	if (path == NULL) return ASL_STATUS_NO_MEMORY;

	sd = NULL;

	memset(&sb, 0, sizeof(struct stat));
	if (stat(path, &sb) != 0)
	{
		if (errno != ENOENT)
		{
			free(path);
			return ASL_STATUS_FAILED;
		}

		sd = fopen(path, "w+");
		free(path);

		if (sd == NULL) return ASL_STATUS_FAILED;

		last_id = 0;

		/* Create new StoreData file (8 bytes ID + 4 bytes flags) */

		if (fwrite(&last_id, sizeof(uint64_t), 1, sd) != 1)
		{
			fclose(sd);
			return ASL_STATUS_WRITE_FAILED;
		}

		flags = 0;
		if (fwrite(&flags, sizeof(uint32_t), 1, sd) != 1)
		{
			fclose(sd);
			return ASL_STATUS_WRITE_FAILED;
		}

		/* flush data */
		fflush(sd);
	}
	else
	{
		sd = fopen(path, "r+");
		free(path);

		if (sd == NULL) return ASL_STATUS_FAILED;
		if (fread(&last_id, sizeof(uint64_t), 1, sd) != 1)
		{
			fclose(sd);
			return ASL_STATUS_READ_FAILED;
		}

		last_id = asl_core_ntohq(last_id);
	}

	out = (asl_store_t *)calloc(1, sizeof(asl_store_t));
	if (out == NULL)
	{
		fclose(sd);
		return ASL_STATUS_NO_MEMORY;
	}

	if (basedir == NULL) out->base_dir = strdup(PATH_ASL_STORE);
	else out->base_dir = strdup(basedir);

	if (out->base_dir == NULL)
	{
		fclose(sd);
		free(out);
		return ASL_STATUS_NO_MEMORY;
	}

	out->start_today = start;
	out->start_tomorrow = out->start_today + SECONDS_PER_DAY;
	out->storedata = sd;
	out->next_id = last_id + 1;

	for (i = 0; i < FILE_CACHE_SIZE; i++)
	{
		memset(&out->file_cache[i], 0, sizeof(asl_cached_file_t));
		out->file_cache[i].u = -1;
		out->file_cache[i].g = -1;
	}

	*s = out;
	return ASL_STATUS_OK;
}

uint32_t
asl_store_statistics(asl_store_t *s, aslmsg *msg)
{
	aslmsg out;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (msg == NULL) return ASL_STATUS_INVALID_ARG;

	out = asl_new(ASL_TYPE_MSG);
	if (out == NULL) return ASL_STATUS_NO_MEMORY;

	/* does nothing for now */

	*msg = out;
	return ASL_STATUS_OK;
}

uint32_t
asl_store_open_read(const char *basedir, asl_store_t **s)
{
	asl_store_t *out;
	struct stat sb;

	if (s == NULL) return ASL_STATUS_INVALID_ARG;

	if (basedir == NULL) basedir = PATH_ASL_STORE;

	memset(&sb, 0, sizeof(struct stat));
	if (stat(basedir, &sb) != 0) return ASL_STATUS_INVALID_STORE;
	if (!S_ISDIR(sb.st_mode)) return ASL_STATUS_INVALID_STORE;

	out = (asl_store_t *)calloc(1, sizeof(asl_store_t));
	if (out == NULL) return ASL_STATUS_NO_MEMORY;

	if (basedir == NULL) out->base_dir = strdup(PATH_ASL_STORE);
	else out->base_dir = strdup(basedir);

	if (out->base_dir == NULL)
	{
		free(out);
		return ASL_STATUS_NO_MEMORY;
	}

	*s = out;
	return ASL_STATUS_OK;
}

uint32_t
asl_store_max_file_size(asl_store_t *s, size_t max)
{
	if (s == NULL) return ASL_STATUS_INVALID_STORE;

	s->max_file_size = max;
	return ASL_STATUS_OK;
}

__private_extern__ void
asl_store_file_closeall(asl_store_t *s)
{
	uint32_t i;

	if (s == NULL) return;

	for (i = 0; i < FILE_CACHE_SIZE; i++)
	{
		if (s->file_cache[i].f != NULL) asl_file_close(s->file_cache[i].f);
		s->file_cache[i].f = NULL;
		if (s->file_cache[i].path != NULL) free(s->file_cache[i].path);
		s->file_cache[i].path = NULL;
		s->file_cache[i].u = -1;
		s->file_cache[i].g = -1;
		s->file_cache[i].bb = 0;
		s->file_cache[i].ts = 0;
	}
}

uint32_t
asl_store_close(asl_store_t *s)
{
	if (s == NULL) return ASL_STATUS_OK;

	if (s->base_dir != NULL) free(s->base_dir);
	s->base_dir = NULL;
	asl_store_file_closeall(s);
	if (s->storedata != NULL) fclose(s->storedata);

	free(s);

	return ASL_STATUS_OK;
}

uint32_t
asl_store_signal_sweep(asl_store_t *s)
{
	char *str;
	int semfd;
	uint64_t xid;
	uint32_t status;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;

	asprintf(&str, "%s/%s", s->base_dir, FILE_ASL_STORE_SWEEP_SEMAPHORE);
	if (str == NULL) return ASL_STATUS_NO_MEMORY;

	semfd = open(str, O_WRONLY | O_CREAT | O_NONBLOCK, 0644);
	free(str);

	if (semfd <  0) return ASL_STATUS_WRITE_FAILED;

	status = ASL_STATUS_OK;

	/* write the current message ID in the SweepStore file */
	xid = asl_core_htonq(s->next_id - 1);
	if (write(semfd, &xid, sizeof(uint64_t)) != sizeof(uint64_t)) status = ASL_STATUS_WRITE_FAILED;

	close(semfd);
	return status;
}

/*
 * Sweep the file cache.
 * Close any files that have not been used in the last FILE_CACHE_TTL seconds.
 * Returns least recently used or unused cache slot.
 */
static uint32_t
asl_store_file_cache_lru(asl_store_t *s, time_t now, uint32_t ignorex)
{
	time_t min;
	uint32_t i, x;

	if (s == NULL) return 0;

	x = 0;
	min = now - FILE_CACHE_TTL;

	for (i = 0; i < FILE_CACHE_SIZE; i++)
	{
		if ((i != ignorex) && (s->file_cache[i].ts < min))
		{
			asl_file_close(s->file_cache[i].f);
			s->file_cache[i].f = NULL;
			if (s->file_cache[i].path != NULL) free(s->file_cache[i].path);
			s->file_cache[i].path = NULL;
			s->file_cache[i].u = -1;
			s->file_cache[i].g = -1;
			s->file_cache[i].bb = 0;
			s->file_cache[i].ts = 0;
		}

		if (s->file_cache[i].ts < s->file_cache[x].ts) x = i;
	}

	return x;
}

uint32_t
asl_store_sweep_file_cache(asl_store_t *s)
{
	if (s == NULL) return ASL_STATUS_INVALID_STORE;

	asl_store_file_cache_lru(s, time(NULL), FILE_CACHE_SIZE);
	return ASL_STATUS_OK;
}

static char *
asl_store_make_ug_path(const char *dir, const char *base, const char *ext, uid_t ruid, gid_t rgid, uid_t *u, gid_t *g, mode_t *m)
{
	char *path  = NULL;

	*u = 0;
	*g = 0;
	*m = 0644;

	if (ruid == -1)
	{
		if (rgid == -1)
		{
			if (ext == NULL) asprintf(&path, "%s/%s", dir, base);
			else asprintf(&path, "%s/%s.%s", dir, base, ext);
		}
		else
		{
			*g = rgid;
			*m = 0600;
			if (ext == NULL) asprintf(&path, "%s/%s.G%d", dir, base, *g);
			else asprintf(&path, "%s/%s.G%d.%s", dir, base, *g, ext);
		}
	}
	else
	{
		*u = ruid;
		if (rgid == -1)
		{
			*m = 0600;
			if (ext == NULL) asprintf(&path, "%s/%s.U%d", dir, base, *u);
			else asprintf(&path, "%s/%s.U%d.%s", dir, base, *u, ext);
		}
		else
		{
			*g = rgid;
			*m = 0600;
			if (ext == NULL) asprintf(&path, "%s/%s.U%d.G%d", dir, base, *u, *g);
			else asprintf(&path, "%s/%s.U%d.G%u.%s", dir, base, *u, *g, ext);
		}
	}

	return path;
}

static uint32_t
asl_store_file_open_write(asl_store_t *s, char *tstring, int32_t ruid, int32_t rgid, time_t bb, asl_file_t **f, time_t now, uint32_t check_cache)
{
	char *path;
	mode_t m;
	int32_t i, x;
	uid_t u;
	gid_t g;
	uint32_t status;
	asl_file_t *out;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;

	/* see if the file is already open and in the cache */
	for (i = 0; i < FILE_CACHE_SIZE; i++)
	{
		if ((s->file_cache[i].u == ruid) && (s->file_cache[i].g == rgid) && (s->file_cache[i].bb == bb) && (s->file_cache[i].f != NULL))
		{
			s->file_cache[i].ts = now;
			*f = s->file_cache[i].f;
			if (check_cache == 1) asl_store_file_cache_lru(s, now, i);
			return ASL_STATUS_OK;
		}
	}

	u = 0;
	g = 0;
	m = 0644;
	path = asl_store_make_ug_path(s->base_dir, tstring, "asl", (uid_t)ruid, (gid_t)rgid, &u, &g, &m);
	if (path == NULL) return ASL_STATUS_NO_MEMORY;

	out = NULL;
	status = asl_file_open_write(path, m, u, g, &out);
	if (status != ASL_STATUS_OK)
	{
		free(path);
		return status;
	}

	x = asl_store_file_cache_lru(s, now, FILE_CACHE_SIZE);
	if (s->file_cache[x].f != NULL) asl_file_close(s->file_cache[x].f);
	if (s->file_cache[x].path != NULL) free(s->file_cache[x].path);

	s->file_cache[x].f = out;
	s->file_cache[x].path = path;
	s->file_cache[x].u = ruid;
	s->file_cache[x].g = rgid;
	s->file_cache[x].bb = bb;
	s->file_cache[x].ts = time(NULL);

	*f = out;

	return ASL_STATUS_OK;
}

__private_extern__ char *
asl_store_file_path(asl_store_t *s, asl_file_t *f)
{
	uint32_t i;

	if (s == NULL) return NULL;

	for (i = 0; i < FILE_CACHE_SIZE; i++)
	{
		if (s->file_cache[i].f == f)
		{
			if (s->file_cache[i].path == NULL) return NULL;
			return strdup(s->file_cache[i].path);
		}
	}

	return NULL;
}

__private_extern__ void
asl_store_file_close(asl_store_t *s, asl_file_t *f)
{
	uint32_t i;

	if (s == NULL) return;
	if (f == NULL) return;

	for (i = 0; i < FILE_CACHE_SIZE; i++)
	{
		if (s->file_cache[i].f == f)
		{
			asl_file_close(s->file_cache[i].f);
			s->file_cache[i].f = NULL;
			if (s->file_cache[i].path != NULL) free(s->file_cache[i].path);
			s->file_cache[i].path = NULL;
			s->file_cache[i].u = -1;
			s->file_cache[i].g = -1;
			s->file_cache[i].bb = 0;
			s->file_cache[i].ts = 0;
			return;
		}
	}
}

uint32_t
asl_store_save(asl_store_t *s, aslmsg msg)
{
	struct tm ctm;
	time_t msg_time, now, bb;
	char *path, *tmp_path, *tstring, *scratch;
	const char *val;
	uid_t ruid;
	gid_t rgid;
	asl_file_t *f;
	uint32_t status, check_cache, signal_sweep, len;
	uint64_t xid, ftime;
	size_t fsize;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (msg == NULL) return ASL_STATUS_INVALID_ARG;

	now = time(NULL);

	check_cache = 0;
	if ((s->last_write + FILE_CACHE_TTL) <= now) check_cache = 1;

	signal_sweep = 0;

	msg_time = 0;
	val = asl_get(msg, ASL_KEY_TIME);
	if (val == NULL) msg_time = now;
	else msg_time = asl_parse_time(val);

	if (msg_time >= s->start_tomorrow)
	{
		if (now >= s->start_tomorrow)
		{
			/* new day begins */
			check_cache = 0;
			signal_sweep = 1;
			asl_store_file_closeall(s);

			/*
			 * _asl_start_today should never fail, but if it does,
			 * just push forward one day.  That will probably be correct, and if
			 * it isn't, the next message that gets saved will push it ahead again
			 * until we get to the right date.
			 */
			s->start_today = _asl_start_today();
			if (s->start_today == 0) s->start_today = s->start_tomorrow;

			s->start_tomorrow = s->start_today + SECONDS_PER_DAY;
		}
	}

	val = asl_get(msg, ASL_KEY_READ_UID);
	ruid = -1;
	if (val != NULL) ruid = atoi(val);

	val = asl_get(msg, ASL_KEY_READ_GID);
	rgid = -1;
	if (val != NULL) rgid = atoi(val);

	bb = 0;
	val = asl_get(msg, ASL_KEY_EXPIRE_TIME);
	if (val != NULL)
	{
		bb = 1;
		msg_time = asl_parse_time(val);
	}

	if (fseeko(s->storedata, 0, SEEK_SET) != 0) return ASL_STATUS_WRITE_FAILED;

	xid = asl_core_htonq(s->next_id);
	if (fwrite(&xid, sizeof(uint64_t), 1, s->storedata) != 1) return ASL_STATUS_WRITE_FAILED;

	/* flush data */
	fflush(s->storedata);

	xid = s->next_id;
	s->next_id++;

	s->last_write = now;

	if (localtime_r((const time_t *)&msg_time, &ctm) == NULL) return ASL_STATUS_FAILED;

	tstring = NULL;
	if (bb == 1)
	{
		/*
		 * This supports 12 monthly "Best Before" buckets.
		 * We advance the actual expiry time to day zero of the following month.
		 * mktime() is clever enough to know that you actually mean the last day
		 * of the previous month.  What we get back from localtime is the last
		 * day of the month in which the message expires, which we use in the name.
		 */
		ctm.tm_sec = 0;
		ctm.tm_min = 0;
		ctm.tm_hour = 0;
		ctm.tm_mday = 0;
		ctm.tm_mon += 1;

		bb = mktime(&ctm);

		if (localtime_r((const time_t *)&bb, &ctm) == NULL) return ASL_STATUS_FAILED;
		asprintf(&tstring, "BB.%d.%02d.%02d", ctm.tm_year + 1900, ctm.tm_mon + 1, ctm.tm_mday);
	}
	else
	{
		asprintf(&tstring, "%d.%02d.%02d", ctm.tm_year + 1900, ctm.tm_mon + 1, ctm.tm_mday);
	}

	if (tstring == NULL) return ASL_STATUS_NO_MEMORY;

	status = asl_store_file_open_write(s, tstring, ruid, rgid, bb, &f, now, check_cache);
	free(tstring);
	tstring = NULL;

	if (status != ASL_STATUS_OK) return status;

	status = asl_file_save(f, msg, &xid);
	if (status != ASL_STATUS_OK) return status;

	fsize = asl_file_size(f);
	ftime = asl_file_ctime(f);

	/* if file is larger than max_file_size, rename it and touch semaphore file in the store */
	if ((s->max_file_size != 0) && (fsize > s->max_file_size))
	{
		signal_sweep = 1;
		status = ASL_STATUS_OK;

		path = asl_store_file_path(s, f);

		asl_store_file_close(s, f);

		if (path != NULL)
		{
			tmp_path = NULL;

			len = strlen(path);
			if ((len >= 4) && (!strcmp(path + len - 4, ".asl")))
			{
				/* rename xxxxxxx.asl to xxxxxxx.timestamp.asl */
				scratch = strdup(path);
				if (scratch != NULL)
				{
					scratch[len - 4] = '\0';
					asprintf(&tmp_path, "%s.%llu.asl", scratch, ftime);
					free(scratch);

				}
			}
			else
			{
				/* append timestamp */
				asprintf(&tmp_path, "%s.%llu", path, ftime);
			}

			if (tmp_path == NULL)
			{
				status = ASL_STATUS_NO_MEMORY;
			}
			else
			{
				if (rename(path, tmp_path) != 0) status = ASL_STATUS_FAILED;
				free(tmp_path);
			}

			free(path);
		}
	}

	if (signal_sweep != 0) asl_store_signal_sweep(s);

	return status;
}

static uint32_t
asl_store_mkdir(asl_store_t *s, const char *dir, mode_t m)
{
	char *tstring = NULL;
	int status;
	struct stat sb;

	asprintf(&tstring, "%s/%s", s->base_dir, dir);
	if (tstring == NULL) return ASL_STATUS_NO_MEMORY;

	memset(&sb, 0, sizeof(struct stat));
	status = stat(tstring, &sb);

	if (status == 0)
	{
		/* must be a directory */
		if (!S_ISDIR(sb.st_mode))
		{
			free(tstring);
			return ASL_STATUS_INVALID_STORE;
		}
	}
	else
	{
		if (errno == ENOENT)
		{
			/* doesn't exist - create it */
			if (mkdir(tstring, m) != 0)
			{
				free(tstring);
				return ASL_STATUS_WRITE_FAILED;
			}
		}
		else
		{
			/* stat failed for some other reason */
			free(tstring);
			return ASL_STATUS_FAILED;
		}
	}

	free(tstring);
	return ASL_STATUS_OK;
}

uint32_t
asl_store_open_aux(asl_store_t *s, aslmsg msg, int *out_fd, char **url)
{
	struct tm ctm;
	time_t msg_time, bb;
	char *path, *dir, *tstring;
	const char *val;
	uid_t ruid, u;
	gid_t rgid, g;
	mode_t m;
	uint32_t status;
	uint64_t fid;
	int fd;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (msg == NULL) return ASL_STATUS_INVALID_ARG;
	if (out_fd == NULL) return ASL_STATUS_INVALID_ARG;
	if (url == NULL) return ASL_STATUS_INVALID_ARG;

	msg_time = time(NULL);

	val = asl_get(msg, ASL_KEY_READ_UID);
	ruid = -1;
	if (val != NULL) ruid = atoi(val);

	val = asl_get(msg, ASL_KEY_READ_GID);
	rgid = -1;
	if (val != NULL) rgid = atoi(val);

	bb = 0;
	val = asl_get(msg, ASL_KEY_EXPIRE_TIME);
	if (val != NULL)
	{
		bb = 1;
		msg_time = asl_parse_time(val);
	}

	if (localtime_r((const time_t *)&msg_time, &ctm) == NULL) return ASL_STATUS_FAILED;

	dir = NULL;
	if (bb == 1)
	{
		/*
		 * This supports 12 monthly "Best Before" buckets.
		 * We advance the actual expiry time to day zero of the following month.
		 * mktime() is clever enough to know that you actually mean the last day
		 * of the previous month.  What we get back from localtime is the last
		 * day of the month in which the message expires, which we use in the name.
		 */
		ctm.tm_sec = 0;
		ctm.tm_min = 0;
		ctm.tm_hour = 0;
		ctm.tm_mday = 0;
		ctm.tm_mon += 1;

		bb = mktime(&ctm);

		if (localtime_r((const time_t *)&bb, &ctm) == NULL) return ASL_STATUS_FAILED;
		asprintf(&dir, "BB.AUX.%d.%02d.%02d", ctm.tm_year + 1900, ctm.tm_mon + 1, ctm.tm_mday);
	}
	else
	{
		asprintf(&dir, "AUX.%d.%02d.%02d", ctm.tm_year + 1900, ctm.tm_mon + 1, ctm.tm_mday);
	}

	if (dir == NULL) return ASL_STATUS_NO_MEMORY;

	status = asl_store_mkdir(s, dir, 0755);
	if (status != ASL_STATUS_OK)
	{
		free(dir);
		return status;
	}

	fid = s->next_id;
	s->next_id++;
	tstring = NULL;

	asprintf(&tstring, "%s/%llu", dir, fid);
	free(dir);
	if (tstring == NULL) return ASL_STATUS_NO_MEMORY;

	u = 0;
	g = 0;
	m = 0644;
	path = asl_store_make_ug_path(s->base_dir, tstring, NULL, ruid, rgid, &u, &g, &m);
	free(tstring);
	if (path == NULL) return ASL_STATUS_NO_MEMORY;

	fd = asl_file_create(path, u, g, m);
	if (fd < 0)
	{
		free(path);
		*out_fd = -1;
		return ASL_STATUS_WRITE_FAILED;
	}

	/* URL is file://<path> */
	*url = NULL;
	asprintf(url, "file://%s", path);
	free(path);

	*out_fd = fd;

	return status;
}

uint32_t
asl_store_match_timeout(asl_store_t *s, aslresponse query, aslresponse *res, uint64_t *last_id, uint64_t start_id, uint32_t count, int32_t direction, uint32_t usec)
{
	DIR *dp;
	struct dirent *dent;
	uint32_t status;
	asl_file_t *f;
	char *path;
	asl_file_list_t *files;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (res == NULL) return ASL_STATUS_INVALID_ARG;

	files = NULL;

	/*
	 * Open all readable files
	 */
	dp = opendir(s->base_dir);
	if (dp == NULL) return ASL_STATUS_READ_FAILED;

	while ((dent = readdir(dp)) != NULL)
	{
		if (dent->d_name[0] == '.') continue;

		path = NULL;
		asprintf(&path, "%s/%s", s->base_dir, dent->d_name);

		/* NB asl_file_open_read will fail if path is NULL, if the file is not an ASL store file, or if it isn't readable */
		status = asl_file_open_read(path, &f);
		if (path != NULL) free(path);
		if ((status != ASL_STATUS_OK) || (f == NULL)) continue;

		files = asl_file_list_add(files, f);
	}

	closedir(dp);

	status = asl_file_list_match_timeout(files, query, res, last_id, start_id, count, direction, usec);
	asl_file_list_close(files);
	return status;
}

uint32_t
asl_store_match(asl_store_t *s, aslresponse query, aslresponse *res, uint64_t *last_id, uint64_t start_id, uint32_t count, int32_t direction)
{
	return asl_store_match_timeout(s, query, res, last_id, start_id, count, direction, 0);
}

uint32_t
asl_store_match_start(asl_store_t *s, uint64_t start_id, int32_t direction)
{
	DIR *dp;
	struct dirent *dent;
	uint32_t status;
	asl_file_t *f;
	char *path;
	asl_file_list_t *files;

	if (s == NULL) return ASL_STATUS_INVALID_STORE;

	if (s->work != NULL) asl_file_list_match_end(s->work);
	s->work = NULL;

	files = NULL;

	/*
	 * Open all readable files
	 */
	dp = opendir(s->base_dir);
	if (dp == NULL) return ASL_STATUS_READ_FAILED;

	while ((dent = readdir(dp)) != NULL)
	{
		if (dent->d_name[0] == '.') continue;

		path = NULL;
		asprintf(&path, "%s/%s", s->base_dir, dent->d_name);

		/* NB asl_file_open_read will fail if path is NULL, if the file is not an ASL store file, or if it isn't readable */
		status = asl_file_open_read(path, &f);
		if (path != NULL) free(path);
		if ((status != ASL_STATUS_OK) || (f == NULL)) continue;

		files = asl_file_list_add(files, f);
	}

	closedir(dp);

	s->work = asl_file_list_match_start(files, start_id, direction);
	if (s->work == NULL) return ASL_STATUS_FAILED;

	return ASL_STATUS_OK;
}

uint32_t
asl_store_match_next(asl_store_t *s, aslresponse query, aslresponse *res, uint32_t count)
{
	if (s == NULL) return ASL_STATUS_INVALID_STORE;
	if (s->work == NULL) return ASL_STATUS_OK;

	return asl_file_list_match_next(s->work, query, res, count);
}
