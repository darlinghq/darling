#ifndef _XTRACE_TLS_H_
#define _XTRACE_TLS_H_

#include <mach/port.h>
#include "base.h"

XTRACE_DECLARATIONS_BEGIN;

#define DEFINE_XTRACE_TLS_VAR(type, name) \
	char name ## _key = '\0'; \
	XTRACE_INLINE \
	type get_ ## name(void) { \
		return *(type*)xtrace_tls(&(name ## _key), sizeof(type)); \
	}; \
	XTRACE_INLINE \
	void set_ ## name(type value) { \
		*(type*)xtrace_tls(&(name ## _key), sizeof(type)) = value; \
	}; \
	XTRACE_INLINE \
	type* get_ptr_ ## name(void) { \
		return (type*)xtrace_tls(&(name ## _key), sizeof(type)); \
	};

void* xtrace_tls(void* key, size_t size);

XTRACE_DECLARATIONS_END;

#endif // _XTRACE_TLS_H_
