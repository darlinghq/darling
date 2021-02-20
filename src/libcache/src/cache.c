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

#include <cache/cache.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

// can't use an initializer because we're part of libSystem
//__attribute__((constructor))
static void initme(void) {
	static int inited = 0;
	if (!inited) {
		inited = 1;
		verbose = getenv("STUB_VERBOSE") != NULL;
	}
}

void* cache_create(void)
{
	initme();
	if (verbose) puts("STUB: cache_create called");
	return NULL;
}

void* cache_destroy(void)
{
	initme();
	if (verbose) puts("STUB: cache_destroy called");
	return NULL;
}

void* cache_get(void)
{
	initme();
	if (verbose) puts("STUB: cache_get called");
	return NULL;
}

void* cache_get_and_retain(void)
{
	initme();
	if (verbose) puts("STUB: cache_get_and_retain called");
	return NULL;
}

void* cache_get_cost_hint(void)
{
	initme();
	if (verbose) puts("STUB: cache_get_cost_hint called");
	return NULL;
}

void* cache_get_count_hint(void)
{
	initme();
	if (verbose) puts("STUB: cache_get_count_hint called");
	return NULL;
}

void* cache_get_info(void)
{
	initme();
	if (verbose) puts("STUB: cache_get_info called");
	return NULL;
}

void* cache_get_info_for_key(void)
{
	initme();
	if (verbose) puts("STUB: cache_get_info_for_key called");
	return NULL;
}

void* cache_get_info_for_keys(void)
{
	initme();
	if (verbose) puts("STUB: cache_get_info_for_keys called");
	return NULL;
}

void* cache_get_minimum_values_hint(void)
{
	initme();
	if (verbose) puts("STUB: cache_get_minimum_values_hint called");
	return NULL;
}

void* cache_get_name(void)
{
	initme();
	if (verbose) puts("STUB: cache_get_name called");
	return NULL;
}

void* cache_hash_byte_string(void)
{
	initme();
	if (verbose) puts("STUB: cache_hash_byte_string called");
	return NULL;
}

void* cache_invoke(void)
{
	initme();
	if (verbose) puts("STUB: cache_invoke called");
	return NULL;
}

void* cache_key_hash_cb_cstring(void)
{
	initme();
	if (verbose) puts("STUB: cache_key_hash_cb_cstring called");
	return NULL;
}

void* cache_key_hash_cb_integer(void)
{
	initme();
	if (verbose) puts("STUB: cache_key_hash_cb_integer called");
	return NULL;
}

void* cache_key_is_equal_cb_cstring(void)
{
	initme();
	if (verbose) puts("STUB: cache_key_is_equal_cb_cstring called");
	return NULL;
}

void* cache_key_is_equal_cb_integer(void)
{
	initme();
	if (verbose) puts("STUB: cache_key_is_equal_cb_integer called");
	return NULL;
}

void* cache_print(void)
{
	initme();
	if (verbose) puts("STUB: cache_print called");
	return NULL;
}

void* cache_print_stats(void)
{
	initme();
	if (verbose) puts("STUB: cache_print_stats called");
	return NULL;
}

void* cache_release_cb_free(void)
{
	initme();
	if (verbose) puts("STUB: cache_release_cb_free called");
	return NULL;
}

void* cache_release_value(void)
{
	initme();
	if (verbose) puts("STUB: cache_release_value called");
	return NULL;
}

void* cache_remove(void)
{
	initme();
	if (verbose) puts("STUB: cache_remove called");
	return NULL;
}

void* cache_remove_all(void)
{
	initme();
	if (verbose) puts("STUB: cache_remove_all called");
	return NULL;
}

void* cache_remove_with_block(void)
{
	initme();
	if (verbose) puts("STUB: cache_remove_with_block called");
	return NULL;
}

void* cache_set_and_retain(void)
{
	initme();
	if (verbose) puts("STUB: cache_set_and_retain called");
	return NULL;
}

void* cache_set_cost_hint(void)
{
	initme();
	if (verbose) puts("STUB: cache_set_cost_hint called");
	return NULL;
}

void* cache_set_count_hint(void)
{
	initme();
	if (verbose) puts("STUB: cache_set_count_hint called");
	return NULL;
}

void* cache_set_minimum_values_hint(void)
{
	initme();
	if (verbose) puts("STUB: cache_set_minimum_values_hint called");
	return NULL;
}

void* cache_set_name(void)
{
	initme();
	if (verbose) puts("STUB: cache_set_name called");
	return NULL;
}

void* cache_simulate_memory_warning_event(void)
{
	initme();
	if (verbose) puts("STUB: cache_simulate_memory_warning_event called");
	return NULL;
}

void* cache_value_make_nonpurgeable_cb(void)
{
	initme();
	if (verbose) puts("STUB: cache_value_make_nonpurgeable_cb called");
	return NULL;
}

void* cache_value_make_purgeable_cb(void)
{
	initme();
	if (verbose) puts("STUB: cache_value_make_purgeable_cb called");
	return NULL;
}
