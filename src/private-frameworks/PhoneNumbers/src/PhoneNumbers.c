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

#include <stddef.h>

/* Load-only stub: the binaries it was generated for bind no symbol from PhoneNumbers, so it
   exports nothing and only satisfies their load command. A strong bind to a symbol this
   framework does not export still fails in dyld, which is the intended loud failure.
   Re-run tools/darling-tier1-stub-gen after an OS update to check that this still holds. */
