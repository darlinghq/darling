/*
This file is part of Darling.

Copyright (C) 2026 Darling Developers

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT WARRANTY OF ANY KIND; see the GNU General Public License
for more details.
*/

#ifndef _LAUNCHER_OPENARGS_H
#define _LAUNCHER_OPENARGS_H

#include <stdbool.h>
#include <stddef.h>

// The launcher must translate only the file operands of "darling open" into guest paths. Its guest
// open(1) accepts the same options (including -a, -b and --args), so option arguments must be passed
// through untouched. This classifies the arguments of that command without running it.
//
// argv[0..start-1] are not inspected. On success, *indexes holds the argv indexes of the operands, in
// order, and must be freed by the caller. False means an option was malformed, as open(1) would also
// reject it.
struct launcher_open_operands
{
	int* indexes;
	size_t count;
};

bool launcherOpenOperands(int argc, char* const argv[], int start, struct launcher_open_operands* out);

#endif
