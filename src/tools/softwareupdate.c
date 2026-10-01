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
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	if (argc > 1 && strcmp(argv[1], "-l") == 0)
	{
		printf("Software Update Tool\n\n");
		printf("Finding available software\n");
		if (access("/Library/Developer/CommandLineTools/usr/bin/git", F_OK) != 0) {
			printf("Software Update found the following new or updated software:\n");
			printf("   * Label: Command Line Tools for Xcode-13.4\n");
			printf("\tCommand Line Tools for Xcode (13.4), 2500000K [recommended]\n");
		} else {
			printf("No new software available.\n");
		}
		return 0;
	}
	else if (argc > 2 && strcmp(argv[1], "-i") == 0)
	{
		if (strstr(argv[2], "Command Line Tools") != NULL)
		{
			printf("Software Update Tool\n\n");
			printf("Installing %s\n", argv[2]);
			return system("export AUTO_ACCEPT_LICENSE=1 && /usr/libexec/darling/clt_install.py");
		}
	}
	
	printf("Software Update Tool\n\n");
	return 0;
}
