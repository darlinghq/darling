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

#ifndef _MLDR_INTERPRETER_PATH_H_
#define _MLDR_INTERPRETER_PATH_H_

#include "loader.h"

/**
 * Called when argv[0] names an interpreter: converts it from a host path to a
 * path inside the prefix ("unexpands" it). A path under the prefix root loses
 * the root, any other path gets SYSTEM_ROOT in front. The result lives in a
 * static buffer of 4096 bytes; a path that does not fit ends the process with
 * an error. Does nothing when there is no prefix root.
 */
void vchroot_unexpand_interpreter(struct load_results* lr);

#endif
