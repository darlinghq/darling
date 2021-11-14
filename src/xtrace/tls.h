#ifndef _XTRACE_TLS_H_
#define _XTRACE_TLS_H_

#include <stddef.h>
#include <stdbool.h>
#include <mach/port.h>
#include "base.h"

XTRACE_DECLARATIONS_BEGIN;

#define DEFINE_XTRACE_TLS_VAR(type, name, default_value) \
	static char name ## _key = '\0'; \
	XTRACE_INLINE \
	type get_ ## name(void) { \
		bool created = false; \
		type* var = (type*)xtrace_tls(&(name ## _key), sizeof(type), &created); \
		if (created) { \
			*var = default_value; \
		} \
		return *var; \
	}; \
	XTRACE_INLINE \
	void set_ ## name(type value) { \
		bool created = false; \
		type* var = (type*)xtrace_tls(&(name ## _key), sizeof(type), NULL); \
		if (created) { \
			*var = default_value; \
		} \
		*var = value; \
	}; \
	XTRACE_INLINE \
	type* get_ptr_ ## name(void) { \
		bool created = false; \
		type* var = (type*)xtrace_tls(&(name ## _key), sizeof(type), &created); \
		if (created) { \
			*var = default_value; \
		} \
		return var; \
	};

void* xtrace_tls(void* key, size_t size, bool* created);

XTRACE_DECLARATIONS_END;

#endif // _XTRACE_TLS_H_
