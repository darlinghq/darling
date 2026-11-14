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


#ifndef _sandbox_H_
#define _sandbox_H_

/* Profiles and parameter sets are opaque objects (sandbox_profile_t and sandbox_params_t in the SPI
 * declarations open-source clients carry). Compile calls return NULL and set errorbuf on failure. */

int sandbox_apply(void* profile);
void* sandbox_apply_container(void);
void* sandbox_compile_entitlements(void* entitlements, void* params, char** errorbuf);
void* sandbox_compile_file(const char* path, void* params, char** errorbuf);
void* sandbox_compile_named(const char* name, void* params, char** errorbuf);
void* sandbox_compile_string(const char* profile, void* params, char** errorbuf);
void* sandbox_container_paths_iterate_items(void);
void* sandbox_create_params(void);
void sandbox_free_params(void* params);
void sandbox_free_profile(void* profile);
void* sandbox_inspect_pid(void);
void* sandbox_inspect_smemory(void);
int sandbox_set_param(void* params, const char* key, const char* value);
void* sandbox_set_user_state_item(void);
void* sandbox_user_state_item_buffer_create(void);
void* sandbox_user_state_item_buffer_destroy(void);
void* sandbox_user_state_item_buffer_send(void);
void* sandbox_user_state_iterate_items(void);

#endif
