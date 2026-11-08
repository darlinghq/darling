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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <darling-config.h>
#include "interpreter_path.h"

void vchroot_unexpand_interpreter(struct load_results* lr) {
	static char unexpanded[4096];
	size_t length;

	if (lr->root_path) {
		length = strlen(lr->argv[0]);

		if (strncmp(lr->argv[0], lr->root_path, lr->root_path_length) == 0) {
			if (length - lr->root_path_length >= sizeof(unexpanded)) {
				fprintf(stderr, "Interpreter path is too long (%zu bytes, limit %zu)\n", length - lr->root_path_length, sizeof(unexpanded) - 1);
				exit(1);
			}
			memmove(unexpanded, lr->argv[0] + lr->root_path_length, length - lr->root_path_length + 1);
		} else {
			if (length + sizeof(SYSTEM_ROOT) > sizeof(unexpanded)) {
				fprintf(stderr, "Interpreter path was too big\n");
				exit(1);
			}
			memmove(unexpanded + sizeof(SYSTEM_ROOT) - 1, lr->argv[0], length + 1);
			memcpy(unexpanded, SYSTEM_ROOT, sizeof(SYSTEM_ROOT) - 1);
		}

		lr->argv[0] = unexpanded;
	}
}
