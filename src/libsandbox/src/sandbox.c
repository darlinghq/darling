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
#include <errno.h>
#include <darling/emulation/linux_premigration/ext/for-sandbox.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

/* Compile/apply family. There is no seatbelt compiler here, so
 * "compiling" a profile yields an empty profile and applying it succeeds.
 * A profile has the documented layout (builtin name, compiled data and
 * its size); here all of them are empty. */
struct sandbox_profile {
    char *builtin;
    unsigned char *data;
    size_t size;
};

static void* sandbox_empty_profile(char **errorbuf)
{
    if (errorbuf != NULL) *errorbuf = NULL;
    return calloc(1, sizeof(struct sandbox_profile));
}

void* sandbox_compile_string(const char *data, void *params, char **errorbuf)
{
    if (verbose) puts("STUB: sandbox_compile_string called");
    (void)data; (void)params;
    return sandbox_empty_profile(errorbuf);
}

void* sandbox_compile_file(const char *path, void *params, char **errorbuf)
{
    if (verbose) puts("STUB: sandbox_compile_file called");
    (void)path; (void)params;
    return sandbox_empty_profile(errorbuf);
}

void* sandbox_compile_named(const char *name, void *params, char **errorbuf)
{
    if (verbose) puts("STUB: sandbox_compile_named called");
    (void)name; (void)params;
    return sandbox_empty_profile(errorbuf);
}

void* sandbox_compile_entitlements(void *entitlements, void *params, char **errorbuf)
{
    if (verbose) puts("STUB: sandbox_compile_entitlements called");
    (void)entitlements; (void)params;
    return sandbox_empty_profile(errorbuf);
}

int sandbox_apply(void *handle)
{
    if (verbose) puts("STUB: sandbox_apply called");
    (void)handle;
    // Nothing is enforced; only record that the process is sandboxed, as
    // sandbox_init does (sandbox_check reports it)
    (void)__darling_sandbox_mark_applied();
    return 0;
}

void sandbox_free_profile(void *handle)
{
    if (verbose) puts("STUB: sandbox_free_profile called");
    free(handle);
}

void* sandbox_apply_container(void)
{
    if (verbose) puts("STUB: sandbox_apply_container called");
    return NULL;
}

void* sandbox_container_paths_iterate_items(void)
{
    if (verbose) puts("STUB: sandbox_container_paths_iterate_items called");
    return NULL;
}

/* A parameter set is an opaque object. Callers treat NULL as failure, so hand out a
 * real one; parameters only matter to profile compilation, which does not happen here. */
struct sandbox_params {
    unsigned int count;
};

void* sandbox_create_params(void)
{
    if (verbose) puts("STUB: sandbox_create_params called");
    return calloc(1, sizeof(struct sandbox_params));
}

void sandbox_free_params(void *params)
{
    if (verbose) puts("STUB: sandbox_free_params called");
    free(params);
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

int sandbox_set_param(void *params, const char *key, const char *value)
{
    if (verbose) puts("STUB: sandbox_set_param called");
    if (params == NULL || key == NULL || value == NULL) {
        errno = EINVAL;
        return -1;
    }
    ((struct sandbox_params *)params)->count++;
    return 0;
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
