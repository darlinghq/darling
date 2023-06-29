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
void* sysmon_copy_process_info(void) { if (verbose) puts("STUB: sysmon_copy_process_info called"); return NULL; }
void* sysmon_release(void) { if (verbose) puts("STUB: sysmon_release called"); return NULL; }
void* sysmon_request_add_attribute(void) { if (verbose) puts("STUB: sysmon_request_add_attribute called"); return NULL; }
void* sysmon_request_add_attributes(void) { if (verbose) puts("STUB: sysmon_request_add_attributes called"); return NULL; }
void* sysmon_request_alloc(void) { if (verbose) puts("STUB: sysmon_request_alloc called"); return NULL; }
void* sysmon_request_cancel(void) { if (verbose) puts("STUB: sysmon_request_cancel called"); return NULL; }
void* sysmon_request_create(void) { if (verbose) puts("STUB: sysmon_request_create called"); return NULL; }
void* sysmon_request_execute(void) { if (verbose) puts("STUB: sysmon_request_execute called"); return NULL; }
void* sysmon_request_set_flags(void) { if (verbose) puts("STUB: sysmon_request_set_flags called"); return NULL; }
void* sysmon_request_set_interval(void) { if (verbose) puts("STUB: sysmon_request_set_interval called"); return NULL; }
void* sysmon_retain(void) { if (verbose) puts("STUB: sysmon_retain called"); return NULL; }
void* sysmon_row_alloc(void) { if (verbose) puts("STUB: sysmon_row_alloc called"); return NULL; }
void* sysmon_row_apply(void) { if (verbose) puts("STUB: sysmon_row_apply called"); return NULL; }
void* sysmon_row_get_value(void) { if (verbose) puts("STUB: sysmon_row_get_value called"); return NULL; }
void* sysmon_table_alloc(void) { if (verbose) puts("STUB: sysmon_table_alloc called"); return NULL; }
void* sysmon_table_apply(void) { if (verbose) puts("STUB: sysmon_table_apply called"); return NULL; }
void* sysmon_table_copy_row(void) { if (verbose) puts("STUB: sysmon_table_copy_row called"); return NULL; }
void* sysmon_table_get_count(void) { if (verbose) puts("STUB: sysmon_table_get_count called"); return NULL; }
void* sysmon_table_get_row(void) { if (verbose) puts("STUB: sysmon_table_get_row called"); return NULL; }
void* sysmon_table_get_timestamp(void) { if (verbose) puts("STUB: sysmon_table_get_timestamp called"); return NULL; }
