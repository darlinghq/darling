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
void* kpc_counterbuf_alloc(void) { if (verbose) puts("STUB: kpc_counterbuf_alloc called"); return NULL; }
void* kpc_counterbuf_free(void) { if (verbose) puts("STUB: kpc_counterbuf_free called"); return NULL; }
void* kpc_cpu_string(void) { if (verbose) puts("STUB: kpc_cpu_string called"); return NULL; }
void* kpc_force_all_ctrs_get(void) { if (verbose) puts("STUB: kpc_force_all_ctrs_get called"); return NULL; }
void* kpc_force_all_ctrs_set(void) { if (verbose) puts("STUB: kpc_force_all_ctrs_set called"); return NULL; }
void* kpc_get_actionid(void) { if (verbose) puts("STUB: kpc_get_actionid called"); return NULL; }
void* kpc_get_config(void) { if (verbose) puts("STUB: kpc_get_config called"); return NULL; }
void* kpc_get_config_count(void) { if (verbose) puts("STUB: kpc_get_config_count called"); return NULL; }
void* kpc_get_counter_count(void) { if (verbose) puts("STUB: kpc_get_counter_count called"); return NULL; }
void* kpc_get_counting(void) { if (verbose) puts("STUB: kpc_get_counting called"); return NULL; }
void* kpc_get_cpu_counters(void) { if (verbose) puts("STUB: kpc_get_cpu_counters called"); return NULL; }
void* kpc_get_period(void) { if (verbose) puts("STUB: kpc_get_period called"); return NULL; }
void* kpc_get_thread_counters(void) { if (verbose) puts("STUB: kpc_get_thread_counters called"); return NULL; }
void* kpc_get_thread_counting(void) { if (verbose) puts("STUB: kpc_get_thread_counting called"); return NULL; }
void* kpc_pmu_version(void) { if (verbose) puts("STUB: kpc_pmu_version called"); return NULL; }
void* kpc_set_actionid(void) { if (verbose) puts("STUB: kpc_set_actionid called"); return NULL; }
void* kpc_set_config(void) { if (verbose) puts("STUB: kpc_set_config called"); return NULL; }
void* kpc_set_counting(void) { if (verbose) puts("STUB: kpc_set_counting called"); return NULL; }
void* kpc_set_period(void) { if (verbose) puts("STUB: kpc_set_period called"); return NULL; }
void* kpc_set_thread_counting(void) { if (verbose) puts("STUB: kpc_set_thread_counting called"); return NULL; }
void* kpc_sw_inc(void) { if (verbose) puts("STUB: kpc_sw_inc called"); return NULL; }
void* kperf_action_count_get(void) { if (verbose) puts("STUB: kperf_action_count_get called"); return NULL; }
void* kperf_action_count_set(void) { if (verbose) puts("STUB: kperf_action_count_set called"); return NULL; }
void* kperf_action_filter_get(void) { if (verbose) puts("STUB: kperf_action_filter_get called"); return NULL; }
void* kperf_action_filter_set_by_pid(void) { if (verbose) puts("STUB: kperf_action_filter_set_by_pid called"); return NULL; }
void* kperf_action_filter_set_by_task(void) { if (verbose) puts("STUB: kperf_action_filter_set_by_task called"); return NULL; }
void* kperf_action_kcallstack_depth_get(void) { if (verbose) puts("STUB: kperf_action_kcallstack_depth_get called"); return NULL; }
void* kperf_action_kcallstack_depth_set(void) { if (verbose) puts("STUB: kperf_action_kcallstack_depth_set called"); return NULL; }
void* kperf_action_samplers_get(void) { if (verbose) puts("STUB: kperf_action_samplers_get called"); return NULL; }
void* kperf_action_samplers_set(void) { if (verbose) puts("STUB: kperf_action_samplers_set called"); return NULL; }
void* kperf_action_ucallstack_depth_get(void) { if (verbose) puts("STUB: kperf_action_ucallstack_depth_get called"); return NULL; }
void* kperf_action_ucallstack_depth_set(void) { if (verbose) puts("STUB: kperf_action_ucallstack_depth_set called"); return NULL; }
void* kperf_action_userdata_get(void) { if (verbose) puts("STUB: kperf_action_userdata_get called"); return NULL; }
void* kperf_action_userdata_set(void) { if (verbose) puts("STUB: kperf_action_userdata_set called"); return NULL; }
void* kperf_bless_allow_preemption(void) { if (verbose) puts("STUB: kperf_bless_allow_preemption called"); return NULL; }
void* kperf_bless_get(void) { if (verbose) puts("STUB: kperf_bless_get called"); return NULL; }
void* kperf_bless_set(void) { if (verbose) puts("STUB: kperf_bless_set called"); return NULL; }
void* kperf_buffer_clearchunk(void) { if (verbose) puts("STUB: kperf_buffer_clearchunk called"); return NULL; }
void* kperf_buffer_create_chunked(void) { if (verbose) puts("STUB: kperf_buffer_create_chunked called"); return NULL; }
void* kperf_buffer_create_live(void) { if (verbose) puts("STUB: kperf_buffer_create_live called"); return NULL; }
void* kperf_buffer_free(void) { if (verbose) puts("STUB: kperf_buffer_free called"); return NULL; }
void* kperf_buffer_free_record(void) { if (verbose) puts("STUB: kperf_buffer_free_record called"); return NULL; }
void* kperf_buffer_get_stats(void) { if (verbose) puts("STUB: kperf_buffer_get_stats called"); return NULL; }
void* kperf_buffer_getbuf(void) { if (verbose) puts("STUB: kperf_buffer_getbuf called"); return NULL; }
void* kperf_buffer_next_record(void) { if (verbose) puts("STUB: kperf_buffer_next_record called"); return NULL; }
void* kperf_buffer_readdata(void) { if (verbose) puts("STUB: kperf_buffer_readdata called"); return NULL; }
void* kperf_buffer_readdata_withmore(void) { if (verbose) puts("STUB: kperf_buffer_readdata_withmore called"); return NULL; }
void* kperf_buffer_returnbuf(void) { if (verbose) puts("STUB: kperf_buffer_returnbuf called"); return NULL; }
void* kperf_buffer_set_config(void) { if (verbose) puts("STUB: kperf_buffer_set_config called"); return NULL; }
void* kperf_buffer_set_debug(void) { if (verbose) puts("STUB: kperf_buffer_set_debug called"); return NULL; }
void* kperf_buffer_set_debug_log(void) { if (verbose) puts("STUB: kperf_buffer_set_debug_log called"); return NULL; }
void* kperf_buffer_setchunk(void) { if (verbose) puts("STUB: kperf_buffer_setchunk called"); return NULL; }
void* kperf_buffer_wait(void) { if (verbose) puts("STUB: kperf_buffer_wait called"); return NULL; }
void* kperf_debug(void) { if (verbose) puts("STUB: kperf_debug called"); return NULL; }
void* kperf_kdbg_callstacks_get(void) { if (verbose) puts("STUB: kperf_kdbg_callstacks_get called"); return NULL; }
void* kperf_kdbg_callstacks_set(void) { if (verbose) puts("STUB: kperf_kdbg_callstacks_set called"); return NULL; }
void* kperf_kdbg_cswitch_set(void) { if (verbose) puts("STUB: kperf_kdbg_cswitch_set called"); return NULL; }
void* kperf_kdbg_switch_get(void) { if (verbose) puts("STUB: kperf_kdbg_switch_get called"); return NULL; }
void* kperf_kdebug_action_get(void) { if (verbose) puts("STUB: kperf_kdebug_action_get called"); return NULL; }
void* kperf_kdebug_action_set(void) { if (verbose) puts("STUB: kperf_kdebug_action_set called"); return NULL; }
void* kperf_kdebug_filter_add_class(void) { if (verbose) puts("STUB: kperf_kdebug_filter_add_class called"); return NULL; }
void* kperf_kdebug_filter_add_class_fn(void) { if (verbose) puts("STUB: kperf_kdebug_filter_add_class_fn called"); return NULL; }
void* kperf_kdebug_filter_add_class_subclass(void) { if (verbose) puts("STUB: kperf_kdebug_filter_add_class_subclass called"); return NULL; }
void* kperf_kdebug_filter_add_class_subclass_fn(void) { if (verbose) puts("STUB: kperf_kdebug_filter_add_class_subclass_fn called"); return NULL; }
void* kperf_kdebug_filter_add_debugid(void) { if (verbose) puts("STUB: kperf_kdebug_filter_add_debugid called"); return NULL; }
void* kperf_kdebug_filter_add_debugid_fn(void) { if (verbose) puts("STUB: kperf_kdebug_filter_add_debugid_fn called"); return NULL; }
void* kperf_kdebug_filter_add_desc(void) { if (verbose) puts("STUB: kperf_kdebug_filter_add_desc called"); return NULL; }
void* kperf_kdebug_filter_create(void) { if (verbose) puts("STUB: kperf_kdebug_filter_create called"); return NULL; }
void* kperf_kdebug_filter_create_desc(void) { if (verbose) puts("STUB: kperf_kdebug_filter_create_desc called"); return NULL; }
void* kperf_kdebug_filter_destroy(void) { if (verbose) puts("STUB: kperf_kdebug_filter_destroy called"); return NULL; }
void* kperf_kdebug_filter_get(void) { if (verbose) puts("STUB: kperf_kdebug_filter_get called"); return NULL; }
void* kperf_kdebug_filter_get_filter(void) { if (verbose) puts("STUB: kperf_kdebug_filter_get_filter called"); return NULL; }
void* kperf_kdebug_filter_get_n_filters(void) { if (verbose) puts("STUB: kperf_kdebug_filter_get_n_filters called"); return NULL; }
void* kperf_kdebug_filter_set(void) { if (verbose) puts("STUB: kperf_kdebug_filter_set called"); return NULL; }
void* kperf_kern_is_64bit(void) { if (verbose) puts("STUB: kperf_kern_is_64bit called"); return NULL; }
void* kperf_logging_flush(void) { if (verbose) puts("STUB: kperf_logging_flush called"); return NULL; }
void* kperf_logging_query_size(void) { if (verbose) puts("STUB: kperf_logging_query_size called"); return NULL; }
void* kperf_logging_set_kdebug(void) { if (verbose) puts("STUB: kperf_logging_set_kdebug called"); return NULL; }
void* kperf_logging_set_size(void) { if (verbose) puts("STUB: kperf_logging_set_size called"); return NULL; }
void* kperf_logging_set_typefilter(void) { if (verbose) puts("STUB: kperf_logging_set_typefilter called"); return NULL; }
void* kperf_logging_set_wrap(void) { if (verbose) puts("STUB: kperf_logging_set_wrap called"); return NULL; }
void* kperf_logging_start(void) { if (verbose) puts("STUB: kperf_logging_start called"); return NULL; }
void* kperf_logging_stop(void) { if (verbose) puts("STUB: kperf_logging_stop called"); return NULL; }
void* kperf_ns_to_ticks(void) { if (verbose) puts("STUB: kperf_ns_to_ticks called"); return NULL; }
void* kperf_reset(void) { if (verbose) puts("STUB: kperf_reset called"); return NULL; }
void* kperf_sample_get(void) { if (verbose) puts("STUB: kperf_sample_get called"); return NULL; }
void* kperf_sample_off(void) { if (verbose) puts("STUB: kperf_sample_off called"); return NULL; }
void* kperf_sample_on(void) { if (verbose) puts("STUB: kperf_sample_on called"); return NULL; }
void* kperf_sample_set(void) { if (verbose) puts("STUB: kperf_sample_set called"); return NULL; }
void* kperf_signpost_action_get(void) { if (verbose) puts("STUB: kperf_signpost_action_get called"); return NULL; }
void* kperf_signpost_action_set(void) { if (verbose) puts("STUB: kperf_signpost_action_set called"); return NULL; }
void* kperf_tick_frequency(void) { if (verbose) puts("STUB: kperf_tick_frequency called"); return NULL; }
void* kperf_ticks_to_ns(void) { if (verbose) puts("STUB: kperf_ticks_to_ns called"); return NULL; }
void* kperf_timer_action_get(void) { if (verbose) puts("STUB: kperf_timer_action_get called"); return NULL; }
void* kperf_timer_action_set(void) { if (verbose) puts("STUB: kperf_timer_action_set called"); return NULL; }
void* kperf_timer_count_get(void) { if (verbose) puts("STUB: kperf_timer_count_get called"); return NULL; }
void* kperf_timer_count_set(void) { if (verbose) puts("STUB: kperf_timer_count_set called"); return NULL; }
void* kperf_timer_period_get(void) { if (verbose) puts("STUB: kperf_timer_period_get called"); return NULL; }
void* kperf_timer_period_set(void) { if (verbose) puts("STUB: kperf_timer_period_set called"); return NULL; }
void* kperf_timer_pet_get(void) { if (verbose) puts("STUB: kperf_timer_pet_get called"); return NULL; }
void* kperf_timer_pet_set(void) { if (verbose) puts("STUB: kperf_timer_pet_set called"); return NULL; }
void* kperf_typefilter_clear(void) { if (verbose) puts("STUB: kperf_typefilter_clear called"); return NULL; }
void* kperf_typefilter_disable_class(void) { if (verbose) puts("STUB: kperf_typefilter_disable_class called"); return NULL; }
void* kperf_typefilter_disable_subclass(void) { if (verbose) puts("STUB: kperf_typefilter_disable_subclass called"); return NULL; }
void* kperf_typefilter_enable_class(void) { if (verbose) puts("STUB: kperf_typefilter_enable_class called"); return NULL; }
void* kperf_typefilter_enable_subclass(void) { if (verbose) puts("STUB: kperf_typefilter_enable_subclass called"); return NULL; }
void* kperf_typefilter_invert(void) { if (verbose) puts("STUB: kperf_typefilter_invert called"); return NULL; }
