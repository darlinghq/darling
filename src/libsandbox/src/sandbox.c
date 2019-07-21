/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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


#include <sandbox/sandbox.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* sandbox_apply(void)
{
    if (verbose) puts("STUB: sandbox_apply called");
    return NULL;
}

void* sandbox_apply_container(void)
{
    if (verbose) puts("STUB: sandbox_apply_container called");
    return NULL;
}

void* sandbox_compile_entitlements(void)
{
    if (verbose) puts("STUB: sandbox_compile_entitlements called");
    return NULL;
}

void* sandbox_compile_file(void)
{
    if (verbose) puts("STUB: sandbox_compile_file called");
    return NULL;
}

void* sandbox_compile_named(void)
{
    if (verbose) puts("STUB: sandbox_compile_named called");
    return NULL;
}

void* sandbox_compile_string(void)
{
    if (verbose) puts("STUB: sandbox_compile_string called");
    return NULL;
}

void* sandbox_container_paths_iterate_items(void)
{
    if (verbose) puts("STUB: sandbox_container_paths_iterate_items called");
    return NULL;
}

void* sandbox_create_params(void)
{
    if (verbose) puts("STUB: sandbox_create_params called");
    return NULL;
}

void* sandbox_free_params(void)
{
    if (verbose) puts("STUB: sandbox_free_params called");
    return NULL;
}

void* sandbox_free_profile(void)
{
    if (verbose) puts("STUB: sandbox_free_profile called");
    return NULL;
}

void* sandbox_inspect_pid(void)
{
    if (verbose) puts("STUB: sandbox_inspect_pid called");
    return NULL;
}

void* sandbox_inspect_smemory(void)
{
    if (verbose) puts("STUB: sandbox_inspect_smemory called");
    return NULL;
}

void* sandbox_set_param(void)
{
    if (verbose) puts("STUB: sandbox_set_param called");
    return NULL;
}

void* sandbox_set_user_state_item(void)
{
    if (verbose) puts("STUB: sandbox_set_user_state_item called");
    return NULL;
}

void* sandbox_user_state_item_buffer_create(void)
{
    if (verbose) puts("STUB: sandbox_user_state_item_buffer_create called");
    return NULL;
}

void* sandbox_user_state_item_buffer_destroy(void)
{
    if (verbose) puts("STUB: sandbox_user_state_item_buffer_destroy called");
    return NULL;
}

void* sandbox_user_state_item_buffer_send(void)
{
    if (verbose) puts("STUB: sandbox_user_state_item_buffer_send called");
    return NULL;
}

void* sandbox_user_state_iterate_items(void)
{
    if (verbose) puts("STUB: sandbox_user_state_iterate_items called");
    return NULL;
}
