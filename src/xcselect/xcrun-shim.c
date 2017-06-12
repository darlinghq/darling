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
#include "xcselect.h"
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
	return xcselect_invoke_xcrun(TOOL_NAME, argc - 1, &argv[1],
			REQUIRE_XCODE ? XCSELECT_FLAG_REQUIRE_XCODE : 0);
}

