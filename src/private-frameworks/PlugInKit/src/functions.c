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
void* oslog(void) { if (verbose) puts("STUB: oslog called"); return NULL; }
void* oslog_ext(void) { if (verbose) puts("STUB: oslog_ext called"); return NULL; }
void* oslog_ls(void) { if (verbose) puts("STUB: oslog_ls called"); return NULL; }
void* oslog_traffic(void) { if (verbose) puts("STUB: oslog_traffic called"); return NULL; }
void* pkAugmentInterface(void) { if (verbose) puts("STUB: pkAugmentInterface called"); return NULL; }
void* pkError(void) { if (verbose) puts("STUB: pkError called"); return NULL; }
void* pkErrorf(void) { if (verbose) puts("STUB: pkErrorf called"); return NULL; }
void* pkFindProtocol(void) { if (verbose) puts("STUB: pkFindProtocol called"); return NULL; }
void* pkFindProtocols(void) { if (verbose) puts("STUB: pkFindProtocols called"); return NULL; }
void* pkIssueSandboxExtension(void) { if (verbose) puts("STUB: pkIssueSandboxExtension called"); return NULL; }
void* xpc_dictionary_get_array(void) { if (verbose) puts("STUB: xpc_dictionary_get_array called"); return NULL; }
void* xpc_dictionary_get_dictionary(void) { if (verbose) puts("STUB: xpc_dictionary_get_dictionary called"); return NULL; }
