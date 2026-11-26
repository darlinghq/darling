/*
 * Darling - libquarantine
 * Copyright (c) 2016 Lubos Dolezel, All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */

#include "quarantine.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <time.h>

/* The exported symbol is the name's bytes themselves (an array), not a pointer to them. */
const char _qtn_xattr_name[] = "com.apple.quarantine";

static const char* error_strings[] = {
	"Undefined error: 0",
	"Item not quarantined",
	"Quarantine is unavailable",
	"Failed to load a compatible version of libMatch"
};

/*
 * A quarantine record is kept in the com.apple.quarantine extended attribute as text:
 * flags (4 hex digits), timestamp (8 hex digits), agent identifier, metadata (an opaque
 * string, in practice a UUID), separated by semicolons. Darling quarantines nothing by
 * itself and enforces nothing: the records are only stored, read back and carried along
 * (copyfile(3) copies them through this API), which is what programs that inspect or
 * preserve the attribute rely on.
 */
#define QTN_IDENTIFIER_MAX 255		/* longest accepted identifier; longer ones give ERANGE */
#define QTN_METADATA_MAX 64		/* longest accepted metadata */
#define QTN_FLAGS_MAX 0xffff		/* flags are four hex digits in the attribute */
#define QTN_DATA_PREFIX "q/"		/* start of the serialized form */

struct _qtn_file_s
{
	uint32_t flags;
	time_t timestamp;
	char identifier[QTN_IDENTIFIER_MAX + 1];
	size_t metadata_len;
	char metadata[QTN_METADATA_MAX];
};

qtn_file_t _qtn_file_alloc()
{
	return (qtn_file_t) calloc(1, sizeof(struct _qtn_file_s));
}

void _qtn_file_free(qtn_file_t f)
{
	free(f);
}

qtn_file_t _qtn_file_clone(qtn_file_t f)
{
	qtn_file_t rv;

	if (!f)
		return NULL;
	rv = _qtn_file_alloc();
	if (rv)
		memcpy(rv, f, sizeof(*rv));
	return rv;
}

// Formats the record as the attribute value (no terminating NUL). Returns its length.
static size_t qtn_format(qtn_file_t f, uint32_t flags, char* out, size_t outsize)
{
	size_t n = (size_t) snprintf(out, outsize, "%04x;%08lx;%s;", (unsigned) flags,
		(unsigned long) f->timestamp, f->identifier);

	if (n + f->metadata_len > outsize)
		return 0;
	memcpy(out + n, f->metadata, f->metadata_len);
	return n + f->metadata_len;
}

// Parses an attribute value. Needs flags and timestamp; agent and metadata may be missing.
static int qtn_parse(qtn_file_t f, const char* text, size_t len)
{
	struct _qtn_file_s rec;
	const char* end = text + len;
	const char* field[4];
	size_t flen[4];
	int nfields = 0;
	const char* p = text;
	char num[16];
	char* e;
	unsigned long v;

	memset(&rec, 0, sizeof(rec));
	while (nfields < 4)
	{
		const char* semi = nfields < 3 ? memchr(p, ';', end - p) : NULL;

		field[nfields] = p;
		flen[nfields] = semi ? (size_t)(semi - p) : (size_t)(end - p);
		nfields++;
		if (!semi)
			break;
		p = semi + 1;
	}
	if (nfields < 2 || flen[0] == 0 || flen[0] > 8 || flen[1] == 0 || flen[1] > 15)
		return EINVAL;

	memcpy(num, field[0], flen[0]);
	num[flen[0]] = 0;
	v = strtoul(num, &e, 16);
	if (*e)
		return EINVAL;
	rec.flags = (uint32_t) v;

	memcpy(num, field[1], flen[1]);
	num[flen[1]] = 0;
	v = strtoul(num, &e, 16);
	if (*e)
		return EINVAL;
	rec.timestamp = (time_t) v;

	if (nfields > 2)
	{
		if (flen[2] > QTN_IDENTIFIER_MAX)
			return EINVAL;
		memcpy(rec.identifier, field[2], flen[2]);
	}
	if (nfields > 3)
	{
		if (flen[3] > QTN_METADATA_MAX)
			return EINVAL;
		memcpy(rec.metadata, field[3], flen[3]);
		rec.metadata_len = flen[3];
	}
	*f = rec;
	return 0;
}

// Reads the attribute of an open file or a path (the path itself, not what a link points to).
static int qtn_load_xattr(qtn_file_t file, int fd, const char* path)
{
	char buf[QTN_SERIALIZED_DATA_MAX];
	ssize_t n;

	if (!file)
		return EINVAL;
	if (path)
		n = getxattr(path, _qtn_xattr_name, buf, sizeof(buf), 0, XATTR_NOFOLLOW);
	else
		n = fgetxattr(fd, _qtn_xattr_name, buf, sizeof(buf), 0, 0);
	if (n < 0)
		return errno == ENOATTR ? QTN_NOT_QUARANTINED : errno;
	return qtn_parse(file, buf, (size_t) n);
}

static int qtn_store_xattr(qtn_file_t file, int fd, const char* path)
{
	char buf[QTN_SERIALIZED_DATA_MAX];
	uint32_t flags;
	size_t len;
	int rv;

	if (!file)
		return EINVAL;
	flags = file->flags;
	if (flags > QTN_FLAGS_MAX)
		return ERANGE;
	// QUIRK: a record whose flags are only 0, 1 or 2 is written with bit 0x80 added, and
	// flags of 0 are written as 0x81; every other value is written as given.
	// Observed through the file API (set flags, apply, read the attribute) on macOS 26.7.3
	// and 10.14.4 (tier 2, black-box); not documented.
	// Test: tests/src/quarantine_file.c (quirk_plain_flags_get_0x80)
	if ((flags & ~0x3u) == 0)
	{
		flags |= 0x80;
		if ((flags & 0x3) == 0)
			flags |= 0x1;
	}
	len = qtn_format(file, flags, buf, sizeof(buf));
	if (len == 0)
		return ERANGE;
	if (path)
		rv = setxattr(path, _qtn_xattr_name, buf, len, 0, XATTR_NOFOLLOW);
	else
		rv = fsetxattr(fd, _qtn_xattr_name, buf, len, 0, 0);
	return rv < 0 ? errno : 0;
}

int _qtn_file_init_with_fd(qtn_file_t file, int fd)
{
	return qtn_load_xattr(file, fd, NULL);
}

int _qtn_file_apply_to_fd(qtn_file_t file, int fd)
{
	return qtn_store_xattr(file, fd, NULL);
}

int _qtn_file_apply_to_path(qtn_file_t file, const char* path)
{
	if (!path)
		return EINVAL;
	return qtn_store_xattr(file, -1, path);
}

int _qtn_file_init_with_path(qtn_file_t file, const char *path)
{
	if (!path)
		return EINVAL;
	return qtn_load_xattr(file, -1, path);
}

// QUIRK: the serialized form is the attribute text behind the two bytes "q/" and followed by
// a NUL byte; text without the prefix is refused with EINVAL. Observed on macOS 26.7.3 and
// 10.14.4 (tier 2, black-box); not documented.
// Test: tests/src/quarantine_file.c (quirk_serialized_form)
int _qtn_file_init_with_data(qtn_file_t file, const void *data, size_t len)
{
	const char* text = data;
	size_t prefix = sizeof(QTN_DATA_PREFIX) - 1;

	if (!file || !data || len < prefix + 1 || memcmp(text, QTN_DATA_PREFIX, prefix) != 0
			|| text[len - 1] != 0)
		return EINVAL;
	return qtn_parse(file, text + prefix, len - prefix - 1);
}

int _qtn_file_to_data(qtn_file_t file, void * data, size_t* len)
{
	char buf[QTN_SERIALIZED_DATA_MAX];
	size_t prefix = sizeof(QTN_DATA_PREFIX) - 1;
	size_t n;

	if (!file || !len)
		return EINVAL;
	n = qtn_format(file, file->flags, buf, sizeof(buf));
	if (n == 0)
		return ERANGE;
	if (*len < prefix + n + 1 || !data)
	{
		*len = prefix + n + 1;
		return ERANGE;
	}
	memcpy(data, QTN_DATA_PREFIX, prefix);
	memcpy((char*) data + prefix, buf, n);
	((char*) data)[prefix + n] = 0;
	*len = prefix + n + 1;
	return 0;
}

const char *_qtn_error(int err)
{
	if (err >= 0 || -err >= sizeof(error_strings)/sizeof(error_strings[0]))
		return strerror(err);
	return error_strings[-err];
}

int _qtn_file_set_flags(qtn_file_t file, uint32_t flags)
{
	if (!file)
		return EINVAL;
	file->flags = flags;
	return 0;
}

uint32_t _qtn_file_get_flags(qtn_file_t file)
{
	return file ? file->flags : 0;
}

int _qtn_file_set_identifier(qtn_file_t file, const char* ident)
{
	size_t n;

	if (!file || !ident)
		return EINVAL;
	n = strlen(ident);
	if (n > QTN_IDENTIFIER_MAX)
		return ERANGE;
	memcpy(file->identifier, ident, n + 1);
	return 0;
}

const char* _qtn_file_get_identifier(qtn_file_t file)
{
	return file ? file->identifier : NULL;
}

void _qtn_file_set_timestamp(qtn_file_t file, time_t timestamp)
{
	if (file)
		file->timestamp = timestamp;
}

time_t _qtn_file_get_timestamp(qtn_file_t file)
{
	return file ? file->timestamp : 0;
}

int _qtn_file_set_metadata(qtn_file_t file, const void* data, size_t len)
{
	if (!file || (len && !data))
		return EINVAL;
	if (len > QTN_METADATA_MAX)
		return ERANGE;
	if (len)
		memcpy(file->metadata, data, len);
	file->metadata_len = len;
	return 0;
}

const void* _qtn_file_get_metadata(qtn_file_t file)
{
	return file ? file->metadata : NULL;
}

size_t _qtn_file_get_metadata_size(qtn_file_t file)
{
	return file ? file->metadata_len : 0;
}

int __esp_enabled()
{
        return QTN_NOT_QUARANTINED;
}

int __esp_check_ns(const char *a, void *b)
{
        return QTN_NOT_QUARANTINED;
}

int __esp_notify_ns(const char *a, void *b)
{
        return QTN_NOT_QUARANTINED;
}

struct _qtn_proc_t
{
	uint32_t flags;
};

qtn_proc_t _qtn_proc_alloc(void)
{
	return (qtn_proc_t) calloc(1, sizeof(struct _qtn_proc_t));
}

void _qtn_proc_free(qtn_proc_t proc)
{
	free(proc);
}

int _qtn_proc_init_with_self(qtn_proc_t proc)
{
	// The current process carries no quarantine state.
	if (proc)
		proc->flags = 0;
	return QTN_NOT_QUARANTINED;
}

// Resets the state to that of a fresh download: the download flag (0x0001) alone, whatever
// was set before (observed on macOS 26.7.3). Test: tests/quarantine-proc-init.c.
void _qtn_proc_init(qtn_proc_t proc)
{
	if (proc)
		proc->flags = 0x0001; // the download flag
}

int _qtn_proc_init_with_data(qtn_proc_t proc, void* data, size_t data_len)
{
	return 0;
}

int _qtn_proc_set_identifier(qtn_proc_t proc, const char* ident)
{
	return 0;
}

int _qtn_proc_set_flags(qtn_proc_t proc, uint32_t flags)
{
	if (proc)
		proc->flags = flags;
	return 0;
}

uint32_t _qtn_proc_get_flags(qtn_proc_t proc)
{
	return proc ? proc->flags : 0;
}

int _qtn_proc_set_tracking_data(qtn_proc_t proc, const void* data, size_t data_len)
{
	return 0;
}

int _qtn_proc_apply_to_self(qtn_proc_t proc)
{
	// Nothing is enforced, so applying always succeeds. Callers (Terminal, launchd) treat any
	// non-zero result as a failure and log it.
	if (!proc)
		return EINVAL;
	return 0;
}

qtn_proc_t qtn_proc_alloc(void)
{
	return _qtn_proc_alloc();
}

void qtn_proc_set_identifier(qtn_proc_t proc, const char* ident)
{
	_qtn_proc_set_identifier(proc, ident);
}

void qtn_proc_set_flags(qtn_proc_t proc, unsigned int flags)
{
	_qtn_proc_set_flags(proc, flags);
}

int qtn_proc_apply_to_self(qtn_proc_t proc) {
	return _qtn_proc_apply_to_self(proc);
};

void qtn_proc_free(qtn_proc_t proc)
{
	_qtn_proc_free(proc);
}

int qtn_file_apply_to_mount_point(qtn_file_t a, const char *b)
{
	return 0;
}

int qtn_file_init_with_mount_point(qtn_file_t a, char b[1024])
{
	return 0;
}

int qtn_proc_init_with_data(qtn_proc_t proc, void* data, size_t data_len) {
	return 0;
};

/* Process responsibility. Nothing here tracks who launched whom, so every process is
 * responsible for itself and disclaiming responsibility for a child is accepted. */
int responsibility_get_pid_responsible_for_pid(int pid)
{
	return pid;
}

int responsibility_spawnattrs_setdisclaim(void* attrs, int disclaim)
{
	return 0;
}
