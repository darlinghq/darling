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

void* OSLogArchiveAddSections(void);
void* OSLogClearOptions(void);
void* OSLogCreateArchive(void);
void* OSLogCreateArchiveWithDictionary(void);
void* OSLogCreateArchiveWithLiveDataOnly(void);
void* OSLogInstallProfilePayload(void);
void* OSLogLogdAdminConnection(void);
void* OSLogRemoveProfilePayload(void);
void* OSLogSetOptions(void);
void* OSLogStateCaptureForAllPIDs(void);
void* OSLogValidateProfilePayload(void);
void* _catalog_create(void);
void* _catalog_create_with_chunk(void);
void* _catalog_destroy(void);
void* _catalog_for_each_uuid(void);
void* _catalog_lookup_procinfo_by_procid(void);
void* _catalog_lookup_procinfo_by_procinfo_key(void);
void* _catalog_procinfo_create(void);
void* _catalog_procinfo_for_each_uuidinfo(void);
void* _catalog_procinfo_lookup_pc(void);
void* _catalog_procinfo_lookup_subsystem(void);
void* _catalog_procinfo_uuidinfo_add(void);
void* _catalog_procinfo_uuidinfo_remove(void);
void* _catalog_subchunk_procinfo_add(void);
void* _catalog_update_earliest_fhbuf_ts(void);
void* _catalog_uuid_add(void);
void* _chunk_support_convert_tracepoint(void);
void* _os_log_resolve_format(void);
void* _os_trace_uuiddb_dsc_harvest(void);
void* _os_trace_uuiddb_dsc_map_resolve(void);
void* _os_trace_uuiddb_dsc_validate_hdr(void);
void* _os_trace_uuiddb_get_dsc_map_fd(void);
void* _os_trace_uuiddb_harvest(void);
void* _os_trace_uuiddb_harvest_initialize(void);
void* _os_trace_uuiddb_path_exists(void);
void* _state_support_decode_data(void);
void* _state_support_validate_chunk(void);
void* _state_support_validate_payload(void);
void* _timesync_continuous_to_wall_time(void);
void* _timesync_db_open(void);
void* _timesync_db_openat(void);
void* _timesync_range_count(void);
void* _timesync_range_create(void);
void* _timesync_range_destroy(void);
void* _timesync_validate(void);
void* _timesync_wall_time_to_continuous(void);
void* os_activity_stream_add_pid(void);
void* os_activity_stream_cancel(void);
void* os_activity_stream_for_name(void);
void* os_activity_stream_for_pid(void);
void* os_activity_stream_for_simulator(void);
void* os_activity_stream_persistence(void);
void* os_activity_stream_resume(void);
void* os_activity_stream_set_event_handler(void);
void* os_activity_stream_set_filter(void);
void* os_log_copy_formatted_message(void);
void* os_trace_copy_formatted_message(void);
void* uuidpath_lookup_fd(void);
void* uuidpath_map_header(void);
void* uuidpath_map_header_fd(void);
void* uuidpath_resolve(void);
void* uuidpath_resolve_fd(void);
void* uuidpath_sharedcache_resolve_fd(void);

#import <Foundation/Foundation.h>
#import <LoggingSupport/OSLogDevice.h>
#import <LoggingSupport/OSActivityStream.h>
#import <LoggingSupport/OSActivityCreateEvent.h>
#import <LoggingSupport/OSActivityTraceMessageEvent.h>
#import <LoggingSupport/OSActivityLogMessageEvent.h>
#import <LoggingSupport/OSActivityUserActionEvent.h>
#import <LoggingSupport/OSActivityTransitionEvent.h>
#import <LoggingSupport/OSActivityStatedumpEvent.h>
#import <LoggingSupport/OSActivityTimesyncEvent.h>
#import <LoggingSupport/OSActivityEvent.h>
#import <LoggingSupport/OSActivityEventMessage.h>
#import <LoggingSupport/_OSLogChunkStore.h>
#import <LoggingSupport/_OSLogChunkMemory.h>
#import <LoggingSupport/_OSLogChunkFile.h>
#import <LoggingSupport/_OSLogChunkFileReference.h>
#import <LoggingSupport/_OSLogIndexFile.h>
#import <LoggingSupport/_OSLogEnumeratorOversizeChunk.h>
#import <LoggingSupport/_OSLogChunkBuffer.h>
#import <LoggingSupport/_OSLogEnumeratorCatalogSubchunk.h>
#import <LoggingSupport/_OSLogEnumeratorCatalog.h>
#import <LoggingSupport/_OSLogTracepointBuffer.h>
#import <LoggingSupport/_OSLogIndexEnumerator.h>
#import <LoggingSupport/_OSLogIndex.h>
#import <LoggingSupport/OSLogPersistence.h>
#import <LoggingSupport/OSLogTermDumper.h>
#import <LoggingSupport/_OSLogDirectoryReference.h>
#import <LoggingSupport/_OSLogCollectionReference.h>
#import <LoggingSupport/_OSLogVersioning.h>
#import <LoggingSupport/OSLogPreferencesManager.h>
#import <LoggingSupport/OSLogPreferencesProcess.h>
#import <LoggingSupport/OSLogPreferencesSubsystem.h>
#import <LoggingSupport/OSLogPreferencesCategory.h>
#import <LoggingSupport/OSActivityInstrumentation.h>
