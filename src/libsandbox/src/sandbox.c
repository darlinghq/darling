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
#include <stdint.h>
#include <string.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

/* Extension token family. Darling does not enforce sandbox profiles, so
 * clients that issue, consume and release extensions around child setup get
 * placeholder non-zero tokens and every call succeeds without effect.
 * (These live in libsandbox.dylib on macOS, hence here and not in
 * libsystem_sandbox.) */
static int64_t sandbox_extension_next_token = 1;

int64_t sandbox_extension_issue_file(const char *path, uint32_t flags)
{
    if (verbose) puts("STUB: sandbox_extension_issue_file called");
    (void)path; (void)flags;
    return sandbox_extension_next_token++;
}

int64_t sandbox_extension_issue_mach(const char *name, uint32_t flags)
{
    if (verbose) puts("STUB: sandbox_extension_issue_mach called");
    (void)name; (void)flags;
    return sandbox_extension_next_token++;
}

int sandbox_extension_consume(int64_t token)
{
    if (verbose) puts("STUB: sandbox_extension_consume called");
    (void)token;
    return 0;
}

int sandbox_extension_release(int64_t token)
{
    if (verbose) puts("STUB: sandbox_extension_release called");
    (void)token;
    return 0;
}

/* Compile/apply family. There is no seatbelt compiler here, so
 * "compiling" a profile yields a dummy handle and applying succeeds.
 * Signatures match Apple's (the old stubs had wrong shapes). */

void* sandbox_compile_string(const char *profile, char **errorbuf)
{
    if (verbose) puts("STUB: sandbox_compile_string called");
    if (errorbuf != NULL) *errorbuf = NULL;
    (void)profile;
    return strdup("darling-sandbox-stub");
}

void* sandbox_compile_file(const char *path, char **errorbuf)
{
    if (verbose) puts("STUB: sandbox_compile_file called");
    if (errorbuf != NULL) *errorbuf = NULL;
    (void)path;
    return strdup("darling-sandbox-stub");
}

void* sandbox_compile_named(const char *name, char **errorbuf)
{
    if (verbose) puts("STUB: sandbox_compile_named called");
    if (errorbuf != NULL) *errorbuf = NULL;
    (void)name;
    return strdup("darling-sandbox-stub");
}

void* sandbox_compile_entitlements(void *entitlements, char **errorbuf)
{
    if (verbose) puts("STUB: sandbox_compile_entitlements called");
    if (errorbuf != NULL) *errorbuf = NULL;
    (void)entitlements;
    return strdup("darling-sandbox-stub");
}

int sandbox_apply(void *handle)
{
    if (verbose) puts("STUB: sandbox_apply called");
    (void)handle;
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

void* sandbox_create_params(void)
{
    if (verbose) puts("STUB: sandbox_create_params called");
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
