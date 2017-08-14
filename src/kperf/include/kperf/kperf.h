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

void* kpc_counterbuf_alloc(void);
void* kpc_counterbuf_free(void);
void* kpc_cpu_string(void);
void* kpc_force_all_ctrs_get(void);
void* kpc_force_all_ctrs_set(void);
void* kpc_get_actionid(void);
void* kpc_get_config(void);
void* kpc_get_config_count(void);
void* kpc_get_counter_count(void);
void* kpc_get_counting(void);
void* kpc_get_cpu_counters(void);
void* kpc_get_period(void);
void* kpc_get_thread_counters(void);
void* kpc_get_thread_counting(void);
void* kpc_pmu_version(void);
void* kpc_set_actionid(void);
void* kpc_set_config(void);
void* kpc_set_counting(void);
void* kpc_set_period(void);
void* kpc_set_thread_counting(void);
void* kpc_sw_inc(void);
void* kperf_action_count_get(void);
void* kperf_action_count_set(void);
void* kperf_action_filter_get(void);
void* kperf_action_filter_set_by_pid(void);
void* kperf_action_filter_set_by_task(void);
void* kperf_action_kcallstack_depth_get(void);
void* kperf_action_kcallstack_depth_set(void);
void* kperf_action_samplers_get(void);
void* kperf_action_samplers_set(void);
void* kperf_action_ucallstack_depth_get(void);
void* kperf_action_ucallstack_depth_set(void);
void* kperf_action_userdata_get(void);
void* kperf_action_userdata_set(void);
void* kperf_bless_allow_preemption(void);
void* kperf_bless_get(void);
void* kperf_bless_set(void);
void* kperf_buffer_clearchunk(void);
void* kperf_buffer_create_chunked(void);
void* kperf_buffer_create_live(void);
void* kperf_buffer_free(void);
void* kperf_buffer_free_record(void);
void* kperf_buffer_get_stats(void);
void* kperf_buffer_getbuf(void);
void* kperf_buffer_next_record(void);
void* kperf_buffer_readdata(void);
void* kperf_buffer_readdata_withmore(void);
void* kperf_buffer_returnbuf(void);
void* kperf_buffer_set_config(void);
void* kperf_buffer_set_debug(void);
void* kperf_buffer_set_debug_log(void);
void* kperf_buffer_setchunk(void);
void* kperf_buffer_wait(void);
void* kperf_debug(void);
void* kperf_kdbg_callstacks_get(void);
void* kperf_kdbg_callstacks_set(void);
void* kperf_kdbg_cswitch_set(void);
void* kperf_kdbg_switch_get(void);
void* kperf_kdebug_action_get(void);
void* kperf_kdebug_action_set(void);
void* kperf_kdebug_filter_add_class(void);
void* kperf_kdebug_filter_add_class_fn(void);
void* kperf_kdebug_filter_add_class_subclass(void);
void* kperf_kdebug_filter_add_class_subclass_fn(void);
void* kperf_kdebug_filter_add_debugid(void);
void* kperf_kdebug_filter_add_debugid_fn(void);
void* kperf_kdebug_filter_add_desc(void);
void* kperf_kdebug_filter_create(void);
void* kperf_kdebug_filter_create_desc(void);
void* kperf_kdebug_filter_destroy(void);
void* kperf_kdebug_filter_get(void);
void* kperf_kdebug_filter_get_filter(void);
void* kperf_kdebug_filter_get_n_filters(void);
void* kperf_kdebug_filter_set(void);
void* kperf_kern_is_64bit(void);
void* kperf_logging_flush(void);
void* kperf_logging_query_size(void);
void* kperf_logging_set_kdebug(void);
void* kperf_logging_set_size(void);
void* kperf_logging_set_typefilter(void);
void* kperf_logging_set_wrap(void);
void* kperf_logging_start(void);
void* kperf_logging_stop(void);
void* kperf_ns_to_ticks(void);
void* kperf_reset(void);
void* kperf_sample_get(void);
void* kperf_sample_off(void);
void* kperf_sample_on(void);
void* kperf_sample_set(void);
void* kperf_signpost_action_get(void);
void* kperf_signpost_action_set(void);
void* kperf_tick_frequency(void);
void* kperf_ticks_to_ns(void);
void* kperf_timer_action_get(void);
void* kperf_timer_action_set(void);
void* kperf_timer_count_get(void);
void* kperf_timer_count_set(void);
void* kperf_timer_period_get(void);
void* kperf_timer_period_set(void);
void* kperf_timer_pet_get(void);
void* kperf_timer_pet_set(void);
void* kperf_typefilter_clear(void);
void* kperf_typefilter_disable_class(void);
void* kperf_typefilter_disable_subclass(void);
void* kperf_typefilter_enable_class(void);
void* kperf_typefilter_enable_subclass(void);
void* kperf_typefilter_invert(void);
