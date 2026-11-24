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

#ifndef _LAUNCHER_BINFMT_H
#define _LAUNCHER_BINFMT_H

#include <stdbool.h>
#include <stddef.h>

// Running a Mach-O file by its name in a host shell.
//
// The binfmt_misc entries Darling installs (lib/binfmt.d/darling.conf) name this launcher as the
// interpreter, with the O flag and no C flag. The kernel then starts the launcher as
//
//     darling <path of the file> <the arguments of the exec call, without its argv[0]>
//
// and passes an open descriptor of the file in the AT_EXECFD auxiliary vector entry. Without the C
// flag the launcher's own credentials are used (so a setuid root install keeps its setuid), never
// those of the Mach-O file, which is the same as if the user had typed "darling exec <file>".
//
// "darling <path>" cannot be told from "darling shell", "darling shutdown" or an option when the
// file is called like one of those; the descriptor is what tells a run by binfmt_misc apart. It is
// an entry the kernel puts there, and acting on it only changes how the arguments are read: it
// grants nothing that "darling exec" does not.

// The descriptor of the program that the kernel passed in AT_EXECFD, or -1 if the launcher was not
// started by a binfmt_misc entry with the O flag. The caller must close it: the kernel does not
// set close-on-exec, so it would reach muvm and the guest.
int launcherBinfmtExecFd(void);

// Writes the absolute path of the file behind execfd to path (size bytes). Returns false if the
// descriptor is not a file with a name on a file system: one that was removed since (the kernel
// would name it "<path> (deleted)", and its link count is 0, which is also the case for a memfd),
// or one with no path at all (a pipe or socket reads "pipe:[..]", not absolute). A path of size - 1
// bytes or more is taken as cut short and refused, so a path of n bytes needs size >= n + 2.
bool launcherBinfmtProgramPath(int execfd, char* path, size_t size);

// The arguments of the "exec" form, ["darling", "exec", program, arguments...], built from
// the ones the kernel passed (argc >= 2 of them: the interpreter, the path, and the arguments).
// It has argc + 1 entries and a terminating NULL. The strings of argv are not copied, but program
// is, so the caller's buffer for it may go out of scope. Returns NULL when argc < 2 or when
// memory is short.
char** launcherBinfmtExecArgv(int argc, char* const argv[], const char* program);

#endif
