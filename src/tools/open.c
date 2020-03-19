/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>

static void usage(void);

/* TODO: Options */
int main(int argc, char *argv[])
{
	CFStringRef url_or_path;
	CFURLRef url;
	struct stat path_stat;
	if (argc != 2)
	{
		usage();
		return EXIT_FAILURE;
	}
	url_or_path = CFStringCreateWithFileSystemRepresentation(kCFAllocatorDefault, argv[1]);
	/* TODO: What is the URL is not a file URL? (http:, ftp:, etc) */
	/* url = CFURLCreateWithString(kCFAllocatorDefault, url_or_path, NULL); */
	url = NULL;

	if (url == NULL)
	{
		if (lstat(argv[1], &path_stat))
		{
			perror(argv[1]);
			return EXIT_FAILURE;
		}
		url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, url_or_path, kCFURLPOSIXPathStyle, path_stat.st_mode & S_IFDIR);
		if (url == NULL)
		{
			printf("Failed to create file url: %s\n", argv[1]);
			return EXIT_FAILURE;
		}
	}

	LSOpenCFURLRef(url, NULL);

	CFRelease(url);

	return EXIT_SUCCESS;
}

static void usage(void)
{
	printf("Usage: open [filenames]\n"
		"Help: Use open to open files, folders, and URLs from the command line.\n");
}
