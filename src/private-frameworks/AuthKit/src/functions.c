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

#include <stdio.h>
#include <stdlib.h>
static int verbose = 0;
__attribute__((constructor)) static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}
void* AKLoginCodeNotificationCreate(void) { if (verbose) puts("STUB: AKLoginCodeNotificationCreate called"); return NULL; }
void* AKStringFromServiceType(void) { if (verbose) puts("STUB: AKStringFromServiceType called"); return NULL; }
void* _AKLogSystem(void) { if (verbose) puts("STUB: _AKLogSystem called"); return NULL; }
void* _AKTrafficLogSubsystem(void) { if (verbose) puts("STUB: _AKTrafficLogSubsystem called"); return NULL; }
