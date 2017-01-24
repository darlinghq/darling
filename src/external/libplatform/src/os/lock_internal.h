/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#ifndef __OS_LOCK_INTERNAL__
#define __OS_LOCK_INTERNAL__

#define OS_LOCK_S_INTERNAL(type) _os_lock_##type##_s
#define OS_LOCK_T_INTERNAL(type) _os_lock_##type##_t
#define OS_LOCK_STRUCT_INTERNAL(type) struct OS_LOCK_S_INTERNAL(type)
#define OS_LOCK_T_MEMBER(type)	\
		OS_LOCK_STRUCT(type) *_osl_opaque_##type; \
		OS_LOCK_STRUCT_INTERNAL(type) *_osl_##type

#define OS_LOCK_STRUCT_DECL_INTERNAL(type, ...) \
		typedef struct OS_LOCK_S_INTERNAL(type) { \
			OS_LOCK_TYPE_STRUCT(type) * const osl_type; \
			__VA_ARGS__ \
		} OS_LOCK_S_INTERNAL(type); \
		typedef OS_LOCK_STRUCT_INTERNAL(type) *OS_LOCK_T_INTERNAL(type)

#define OS_LOCK_METHODS_DECL(type) \
		void _os_lock_##type##_lock(OS_LOCK_T_INTERNAL(type)); \
		bool _os_lock_##type##_trylock(OS_LOCK_T_INTERNAL(type)); \
		void _os_lock_##type##_unlock(OS_LOCK_T_INTERNAL(type))

#define OS_LOCK_TYPE_STRUCT_DECL(type) \
		OS_LOCK_TYPE_STRUCT(type) { \
			const char *osl_kind; \
			void (*osl_lock)(os_lock_t); \
			bool (*osl_trylock)(os_lock_t); \
			void (*osl_unlock)(os_lock_t); \
		} OS_LOCK_TYPE_REF(type)

#define OS_LOCK_TYPE_INSTANCE(type) \
		OS_LOCK_TYPE_STRUCT_DECL(type) = { \
			.osl_kind = #type, \
			.osl_lock = _os_lock_##type##_lock, \
			.osl_trylock = _os_lock_##type##_trylock, \
			.osl_unlock = _os_lock_##type##_unlock, \
		}

#include "os/internal.h"

#endif // __OS_LOCK_INTERNAL__
