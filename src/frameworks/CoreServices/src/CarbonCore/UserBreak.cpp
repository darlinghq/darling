
/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

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

#include <pthread.h>
#include <cstdlib>
#include "UserBreak.h"

static bool doBreak = false;

static void initUserBreak()
{
	const char* userbreak = getenv("USERBREAK");
	if (userbreak && atoi(userbreak))
		doBreak = true;
}

__attribute__((visibility("hidden")))
void doUserBreak()
{
	static pthread_once_t once = PTHREAD_ONCE_INIT;
	pthread_once(&once, initUserBreak);
	if (doBreak)
		__builtin_debugtrap();
}
