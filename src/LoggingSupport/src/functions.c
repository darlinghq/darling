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
void* OSLogArchiveAddSections(void) { if (verbose) puts("STUB: OSLogArchiveAddSections called"); return NULL; }
void* OSLogClearOptions(void) { if (verbose) puts("STUB: OSLogClearOptions called"); return NULL; }
void* OSLogCreateArchive(void) { if (verbose) puts("STUB: OSLogCreateArchive called"); return NULL; }
void* OSLogCreateArchiveWithDictionary(void) { if (verbose) puts("STUB: OSLogCreateArchiveWithDictionary called"); return NULL; }
void* OSLogCreateArchiveWithLiveDataOnly(void) { if (verbose) puts("STUB: OSLogCreateArchiveWithLiveDataOnly called"); return NULL; }
void* OSLogInstallProfilePayload(void) { if (verbose) puts("STUB: OSLogInstallProfilePayload called"); return NULL; }
void* OSLogLogdAdminConnection(void) { if (verbose) puts("STUB: OSLogLogdAdminConnection called"); return NULL; }
void* OSLogRemoveProfilePayload(void) { if (verbose) puts("STUB: OSLogRemoveProfilePayload called"); return NULL; }
void* OSLogSetOptions(void) { if (verbose) puts("STUB: OSLogSetOptions called"); return NULL; }
void* OSLogStateCaptureForAllPIDs(void) { if (verbose) puts("STUB: OSLogStateCaptureForAllPIDs called"); return NULL; }
void* OSLogValidateProfilePayload(void) { if (verbose) puts("STUB: OSLogValidateProfilePayload called"); return NULL; }
void* _catalog_create(void) { if (verbose) puts("STUB: _catalog_create called"); return NULL; }
void* _catalog_create_with_chunk(void) { if (verbose) puts("STUB: _catalog_create_with_chunk called"); return NULL; }
void* _catalog_destroy(void) { if (verbose) puts("STUB: _catalog_destroy called"); return NULL; }
void* _catalog_for_each_uuid(void) { if (verbose) puts("STUB: _catalog_for_each_uuid called"); return NULL; }
void* _catalog_lookup_procinfo_by_procid(void) { if (verbose) puts("STUB: _catalog_lookup_procinfo_by_procid called"); return NULL; }
void* _catalog_lookup_procinfo_by_procinfo_key(void) { if (verbose) puts("STUB: _catalog_lookup_procinfo_by_procinfo_key called"); return NULL; }
void* _catalog_procinfo_create(void) { if (verbose) puts("STUB: _catalog_procinfo_create called"); return NULL; }
void* _catalog_procinfo_for_each_uuidinfo(void) { if (verbose) puts("STUB: _catalog_procinfo_for_each_uuidinfo called"); return NULL; }
void* _catalog_procinfo_lookup_pc(void) { if (verbose) puts("STUB: _catalog_procinfo_lookup_pc called"); return NULL; }
void* _catalog_procinfo_lookup_subsystem(void) { if (verbose) puts("STUB: _catalog_procinfo_lookup_subsystem called"); return NULL; }
void* _catalog_procinfo_uuidinfo_add(void) { if (verbose) puts("STUB: _catalog_procinfo_uuidinfo_add called"); return NULL; }
void* _catalog_procinfo_uuidinfo_remove(void) { if (verbose) puts("STUB: _catalog_procinfo_uuidinfo_remove called"); return NULL; }
void* _catalog_subchunk_procinfo_add(void) { if (verbose) puts("STUB: _catalog_subchunk_procinfo_add called"); return NULL; }
void* _catalog_update_earliest_fhbuf_ts(void) { if (verbose) puts("STUB: _catalog_update_earliest_fhbuf_ts called"); return NULL; }
void* _catalog_uuid_add(void) { if (verbose) puts("STUB: _catalog_uuid_add called"); return NULL; }
void* _chunk_support_convert_tracepoint(void) { if (verbose) puts("STUB: _chunk_support_convert_tracepoint called"); return NULL; }
void* _os_log_resolve_format(void) { if (verbose) puts("STUB: _os_log_resolve_format called"); return NULL; }
void* _os_trace_uuiddb_dsc_harvest(void) { if (verbose) puts("STUB: _os_trace_uuiddb_dsc_harvest called"); return NULL; }
void* _os_trace_uuiddb_dsc_map_resolve(void) { if (verbose) puts("STUB: _os_trace_uuiddb_dsc_map_resolve called"); return NULL; }
void* _os_trace_uuiddb_dsc_validate_hdr(void) { if (verbose) puts("STUB: _os_trace_uuiddb_dsc_validate_hdr called"); return NULL; }
void* _os_trace_uuiddb_get_dsc_map_fd(void) { if (verbose) puts("STUB: _os_trace_uuiddb_get_dsc_map_fd called"); return NULL; }
void* _os_trace_uuiddb_harvest(void) { if (verbose) puts("STUB: _os_trace_uuiddb_harvest called"); return NULL; }
void* _os_trace_uuiddb_harvest_initialize(void) { if (verbose) puts("STUB: _os_trace_uuiddb_harvest_initialize called"); return NULL; }
void* _os_trace_uuiddb_path_exists(void) { if (verbose) puts("STUB: _os_trace_uuiddb_path_exists called"); return NULL; }
void* _state_support_decode_data(void) { if (verbose) puts("STUB: _state_support_decode_data called"); return NULL; }
void* _state_support_validate_chunk(void) { if (verbose) puts("STUB: _state_support_validate_chunk called"); return NULL; }
void* _state_support_validate_payload(void) { if (verbose) puts("STUB: _state_support_validate_payload called"); return NULL; }
void* _timesync_continuous_to_wall_time(void) { if (verbose) puts("STUB: _timesync_continuous_to_wall_time called"); return NULL; }
void* _timesync_db_open(void) { if (verbose) puts("STUB: _timesync_db_open called"); return NULL; }
void* _timesync_db_openat(void) { if (verbose) puts("STUB: _timesync_db_openat called"); return NULL; }
void* _timesync_range_count(void) { if (verbose) puts("STUB: _timesync_range_count called"); return NULL; }
void* _timesync_range_create(void) { if (verbose) puts("STUB: _timesync_range_create called"); return NULL; }
void* _timesync_range_destroy(void) { if (verbose) puts("STUB: _timesync_range_destroy called"); return NULL; }
void* _timesync_validate(void) { if (verbose) puts("STUB: _timesync_validate called"); return NULL; }
void* _timesync_wall_time_to_continuous(void) { if (verbose) puts("STUB: _timesync_wall_time_to_continuous called"); return NULL; }
void* os_activity_stream_add_pid(void) { if (verbose) puts("STUB: os_activity_stream_add_pid called"); return NULL; }
void* os_activity_stream_cancel(void) { if (verbose) puts("STUB: os_activity_stream_cancel called"); return NULL; }
void* os_activity_stream_for_name(void) { if (verbose) puts("STUB: os_activity_stream_for_name called"); return NULL; }
void* os_activity_stream_for_pid(void) { if (verbose) puts("STUB: os_activity_stream_for_pid called"); return NULL; }
void* os_activity_stream_for_simulator(void) { if (verbose) puts("STUB: os_activity_stream_for_simulator called"); return NULL; }
void* os_activity_stream_persistence(void) { if (verbose) puts("STUB: os_activity_stream_persistence called"); return NULL; }
void* os_activity_stream_resume(void) { if (verbose) puts("STUB: os_activity_stream_resume called"); return NULL; }
void* os_activity_stream_set_event_handler(void) { if (verbose) puts("STUB: os_activity_stream_set_event_handler called"); return NULL; }
void* os_activity_stream_set_filter(void) { if (verbose) puts("STUB: os_activity_stream_set_filter called"); return NULL; }
void* os_log_copy_formatted_message(void) { if (verbose) puts("STUB: os_log_copy_formatted_message called"); return NULL; }
void* os_trace_copy_formatted_message(void) { if (verbose) puts("STUB: os_trace_copy_formatted_message called"); return NULL; }
void* uuidpath_lookup_fd(void) { if (verbose) puts("STUB: uuidpath_lookup_fd called"); return NULL; }
void* uuidpath_map_header(void) { if (verbose) puts("STUB: uuidpath_map_header called"); return NULL; }
void* uuidpath_map_header_fd(void) { if (verbose) puts("STUB: uuidpath_map_header_fd called"); return NULL; }
void* uuidpath_resolve(void) { if (verbose) puts("STUB: uuidpath_resolve called"); return NULL; }
void* uuidpath_resolve_fd(void) { if (verbose) puts("STUB: uuidpath_resolve_fd called"); return NULL; }
void* uuidpath_sharedcache_resolve_fd(void) { if (verbose) puts("STUB: uuidpath_sharedcache_resolve_fd called"); return NULL; }
