/*
This file is part of Darling.

Copyright (C) 2017 Lubos Dolezel

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
#include "libxcselect.h"
#include <stdlib.h>
#include <string.h>

int main(int argc, const char** argv)
{
	const char* tool;
	const char* pname = getprogname();

	if (*pname == '/')
		tool = pname + 1;
	else
		tool = pname;

	if (strcasecmp(tool, "xcrun") == 0)
		tool = NULL;

	return xcselect_invoke_xcrun(tool, argc - 1, &argv[1], 0);
}

