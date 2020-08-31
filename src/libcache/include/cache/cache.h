/**
 * This file is part of Darling.
 *
 * Copyright (C) 2020 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CACHE_CACHE_H_
#define _CACHE_CACHE_H_

#ifdef __cplusplus
extern "C" {
#endif

void* cache_create(void);
void* cache_destroy(void);
void* cache_get(void);
void* cache_get_and_retain(void);
void* cache_get_cost_hint(void);
void* cache_get_count_hint(void);
void* cache_get_info(void);
void* cache_get_info_for_key(void);
void* cache_get_info_for_keys(void);
void* cache_get_minimum_values_hint(void);
void* cache_get_name(void);
void* cache_hash_byte_string(void);
void* cache_invoke(void);
void* cache_key_hash_cb_cstring(void);
void* cache_key_hash_cb_integer(void);
void* cache_key_is_equal_cb_cstring(void);
void* cache_key_is_equal_cb_integer(void);
void* cache_print(void);
void* cache_print_stats(void);
void* cache_release_cb_free(void);
void* cache_release_value(void);
void* cache_remove(void);
void* cache_remove_all(void);
void* cache_remove_with_block(void);
void* cache_set_and_retain(void);
void* cache_set_cost_hint(void);
void* cache_set_count_hint(void);
void* cache_set_minimum_values_hint(void);
void* cache_set_name(void);
void* cache_simulate_memory_warning_event(void);
void* cache_value_make_nonpurgeable_cb(void);
void* cache_value_make_purgeable_cb(void);

#ifdef __cplusplus
};
#endif

#endif // _CACHE_CACHE_H_
