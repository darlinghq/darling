/*
 * Copyright (c) 2015 Apple Computer, Inc.  All rights reserved.
 */

#ifndef __REMOVEFILE_H__
#define __REMOVEFILE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Flags
 */
typedef uint32_t removefile_flags_t;

enum {
	REMOVEFILE_RECURSIVE = (1 << 0),            // If path is a directory, recurse (depth first traversal)
	REMOVEFILE_KEEP_PARENT = (1 << 1),          // Remove contents but not directory itself
	REMOVEFILE_SECURE_7_PASS = (1 << 2),        // 7 pass DoD algorithm
	REMOVEFILE_SECURE_35_PASS  = (1 << 3),      // 35-pass Gutmann algorithm (overrides REMOVEFILE_SECURE_7_PASS)
	REMOVEFILE_SECURE_1_PASS = (1 << 4),        // 1 pass single overwrite
	REMOVEFILE_SECURE_3_PASS = (1 << 5),        // 3 pass overwrite
	REMOVEFILE_SECURE_1_PASS_ZERO = (1 << 6),   // Single-pass overwrite, with 0 instead of random data
	REMOVEFILE_CROSS_MOUNT = (1 << 7),          // Cross mountpoints when deleting recursively.
};

/*
 * State object to pass in callback information
 */ 
typedef struct _removefile_state * removefile_state_t;

removefile_state_t removefile_state_alloc(void);
int removefile_state_free(removefile_state_t);

int removefile_state_get(removefile_state_t state, uint32_t key, void * dst);
int removefile_state_set(removefile_state_t state, uint32_t key, const void* value);

enum {
	REMOVEFILE_STATE_CONFIRM_CALLBACK = 1,	// removefile_callback_t
	REMOVEFILE_STATE_CONFIRM_CONTEXT = 2,		// void*
	REMOVEFILE_STATE_ERROR_CALLBACK = 3,		// removefile_callback_t
	REMOVEFILE_STATE_ERROR_CONTEXT = 4,		// void*
	REMOVEFILE_STATE_ERRNO = 5,					// int (read-only)
	REMOVEFILE_STATE_STATUS_CALLBACK = 6,		// removefile_callback_t
	REMOVEFILE_STATE_STATUS_CONTEXT = 7,		// void*
};

typedef int (*removefile_callback_t)(removefile_state_t state, const char* path, void* context);

/* 
 * Callback return values
 */ 
enum {
	REMOVEFILE_PROCEED = 0,
	REMOVEFILE_SKIP = 1,
	REMOVEFILE_STOP = 2,
};


int removefile(const char* path, removefile_state_t state, removefile_flags_t flags);

int removefile_cancel(removefile_state_t state);

#ifdef __cplusplus
}
#endif

#endif /* __REMOVEFILE_H__ */
