/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "od_passwd_file.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_FIELDS 10

static bool valueIsValid(const char* value)
{
	return value == NULL || strpbrk(value, ":\n\r") == NULL;
}

// Splits a line without its newline in place; returns the number of fields
static int splitFields(char* line, char* fields[MAX_FIELDS])
{
	int count = 0;
	char* p = line;

	for (;;)
	{
		if (count == MAX_FIELDS + 1)
			return count;
		if (count < MAX_FIELDS)
			fields[count] = p;
		count++;
		p = strchr(p, ':');
		if (p == NULL)
			return count;
		*p++ = '\0';
	}
}

static int writeEntry(FILE* out, char* fields[MAX_FIELDS], int count)
{
	for (int i = 0; i < count; i++)
	{
		if ((i > 0 && fputc(':', out) == EOF) || fputs(fields[i], out) == EOF)
			return EOF;
	}
	return 0;
}

int odpw_update_user(const char* path, enum odpw_format format, const char* name,
		const struct odpw_changes* changes, bool* found)
{
	const int expected = format == ODPW_MASTER ? 10 : 7;
	const int gecosIndex = format == ODPW_MASTER ? 7 : 4;
	const int shellIndex = expected - 1;
	FILE* in = NULL;
	FILE* out = NULL;
	char* tmpPath = NULL;
	char* line = NULL;
	size_t lineSize = 0;
	int err = 0;
	int tmpFd = -1;
	struct stat st;

	*found = false;
	if (!valueIsValid(changes->gecos) || !valueIsValid(changes->shell) || strpbrk(name, ":\n") != NULL)
		return EINVAL;

	in = fopen(path, "r");
	if (in == NULL)
		return errno;
	if (fstat(fileno(in), &st) != 0)
	{
		err = errno;
		goto done;
	}

	size_t pathLen = strlen(path);
	tmpPath = malloc(pathLen + sizeof(".XXXXXX"));
	if (tmpPath == NULL)
	{
		err = ENOMEM;
		goto done;
	}
	memcpy(tmpPath, path, pathLen);
	strcpy(tmpPath + pathLen, ".XXXXXX");
	tmpFd = mkstemp(tmpPath);
	if (tmpFd == -1)
	{
		err = errno;
		goto done;
	}
	out = fdopen(tmpFd, "w");
	if (out == NULL)
	{
		err = errno;
		goto done;
	}
	tmpFd = -1;

	// Ownership is restored only as far as we may; permissions always
	if (fchown(fileno(out), st.st_uid, st.st_gid) != 0 && errno != EPERM)
	{
		err = errno;
		goto done;
	}
	if (fchmod(fileno(out), st.st_mode & 07777) != 0)
	{
		err = errno;
		goto done;
	}

	ssize_t length;
	while ((length = getline(&line, &lineSize, in)) != -1)
	{
		bool hasNewline = length > 0 && line[length - 1] == '\n';
		char* fields[MAX_FIELDS];
		char* copy = NULL;
		int count = 0;

		// Entries are found on a copy, the line itself is written as it was
		// unless it is the entry to change.
		if (line[0] != '#' && line[0] != '\n' && !*found)
		{
			copy = strdup(line);
			if (copy == NULL)
			{
				err = ENOMEM;
				goto done;
			}
			if (hasNewline)
				copy[length - 1] = '\0';
			count = splitFields(copy, fields);
		}

		if (copy != NULL && count == expected && strcmp(fields[0], name) == 0)
		{
			*found = true;
			if (changes->gecos != NULL)
				fields[gecosIndex] = (char*) changes->gecos;
			if (changes->shell != NULL)
				fields[shellIndex] = (char*) changes->shell;
			if (writeEntry(out, fields, count) == EOF || (hasNewline && fputc('\n', out) == EOF))
				err = errno ? errno : EIO;
		}
		else if (fwrite(line, 1, length, out) != (size_t) length)
		{
			err = errno ? errno : EIO;
		}
		free(copy);
		if (err != 0)
			goto done;
	}

	if (!*found)
		goto done;

	if (fflush(out) == EOF || fsync(fileno(out)) != 0 || fclose(out) == EOF)
	{
		err = errno ? errno : EIO;
		out = NULL;
		goto done;
	}
	out = NULL;

	if (rename(tmpPath, path) != 0)
	{
		err = errno;
		goto done;
	}
	free(tmpPath);
	tmpPath = NULL;

done:
	if (tmpFd != -1)
		close(tmpFd);
	if (out != NULL)
		fclose(out);
	if (tmpPath != NULL)
	{
		unlink(tmpPath);
		free(tmpPath);
	}
	if (in != NULL)
		fclose(in);
	free(line);
	if (err != 0)
		*found = false;
	return err;
}
