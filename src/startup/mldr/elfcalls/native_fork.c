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

#include <errno.h>
#include <unistd.h>
#include "native_fork.h"

int __mldr_native_fork(void)
{
	__mldr_socket_bitmap_prefork();
	int result = fork();
	int saved_errno = errno;
	if (result == 0) {
		__mldr_thread_postfork_child();
		__mldr_socket_bitmap_postfork_child();
	} else {
		__mldr_socket_bitmap_postfork_parent();
	}
	/* Preserve fork's errno even if lock cleanup changes it. */
	return result < 0 ? -saved_errno : result;
}
