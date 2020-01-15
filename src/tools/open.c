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
#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>

void usage(void);

int main(int argc, char *argv[])
{
	if (argc < 0)
	{
		usage();
		return 1;
	}
	CFStringRef str = CFStringCreateWithCString(kCFAllocatorDefault, argv[1], kCFStringEncodingMacRoman);
	CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, str, kCFURLPOSIXPathStyle, FALSE);



	return 0;
}

void usage(void)
{
	printf("Usage: open [filenames]\n"
		"Help: Use open to open files, folders, and URLs from the command line.\n");
}
