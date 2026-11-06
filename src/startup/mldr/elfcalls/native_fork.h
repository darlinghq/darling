/*
This file is part of Darling.

Copyright (C) 2026 Darling Developers

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

#ifndef _MLDR_NATIVE_FORK_H_
#define _MLDR_NATIVE_FORK_H_

// Forks the process through the host libc, so that the host allocator and
// the atfork handlers of host libraries see the fork. Returns the child's PID
// in the parent, 0 in the child, or a negative Linux errno.
int __mldr_native_fork(void);

// Provided by the loader: called around the fork to keep its own state
// consistent in both processes.
void __mldr_socket_bitmap_prefork(void);
void __mldr_socket_bitmap_postfork_parent(void);
void __mldr_socket_bitmap_postfork_child(void);
void __mldr_thread_postfork_child(void);

#endif // _MLDR_NATIVE_FORK_H_
