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


#include <stdlib.h>
static int verbose = 0;
__attribute__((constructor)) static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}
void* msgtracer_domain_free(void) { if (verbose) puts("STUB: msgtracer_domain_free called"); return NULL; }
void* msgtracer_domain_new(void) { if (verbose) puts("STUB: msgtracer_domain_new called"); return NULL; }
void* msgtracer_log(void) { if (verbose) puts("STUB: msgtracer_log called"); return NULL; }
void* msgtracer_log_with_keys(void) { if (verbose) puts("STUB: msgtracer_log_with_keys called"); return NULL; }
void* msgtracer_log_with_keys_skip_nulls(void) { if (verbose) puts("STUB: msgtracer_log_with_keys_skip_nulls called"); return NULL; }
void* msgtracer_msg_free(void) { if (verbose) puts("STUB: msgtracer_msg_free called"); return NULL; }
void* msgtracer_msg_new(void) { if (verbose) puts("STUB: msgtracer_msg_new called"); return NULL; }
void* msgtracer_set(void) { if (verbose) puts("STUB: msgtracer_set called"); return NULL; }
void* msgtracer_uuid_create(void) { if (verbose) puts("STUB: msgtracer_uuid_create called"); return NULL; }
void* msgtracer_vlog(void) { if (verbose) puts("STUB: msgtracer_vlog called"); return NULL; }
void* msgtracer_vlog_with_keys(void) { if (verbose) puts("STUB: msgtracer_vlog_with_keys called"); return NULL; }
void* msgtracer_vlog_with_keys_skip_nulls(void) { if (verbose) puts("STUB: msgtracer_vlog_with_keys_skip_nulls called"); return NULL; }
