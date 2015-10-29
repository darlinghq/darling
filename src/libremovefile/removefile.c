#include "removefile.h"
#include "removefile_priv.h"

#include <TargetConditionals.h>

removefile_state_t
removefile_state_alloc() {
	removefile_state_t state = (removefile_state_t)calloc(1, sizeof(struct _removefile_state));
	if (state != NULL)
		state->urand_file = -1;
	return state;
}

int
removefile_state_free(removefile_state_t state) {
	if (state != NULL) {
		if (state->urand_file != -1) {
			close(state->urand_file);
			state->urand_file = -1;
		}
		if (state->buffer != NULL) {
			free(state->buffer);
			state->buffer = NULL;
		}
		free(state);
	}
	return 0;
}

int
removefile_state_get(removefile_state_t state, uint32_t key, void* dst) {
	switch(key) {
		case REMOVEFILE_STATE_CONFIRM_CALLBACK:
			*(removefile_callback_t*)dst = state->confirm_callback;
			break;
		case REMOVEFILE_STATE_CONFIRM_CONTEXT:
			*(void**)dst = state->confirm_context;
			break;
		case REMOVEFILE_STATE_ERROR_CALLBACK:
			*(removefile_callback_t*)dst = state->error_callback;
			break;
		case REMOVEFILE_STATE_ERROR_CONTEXT:
			*(void**)dst = state->error_context;
			break;
		case REMOVEFILE_STATE_ERRNO:
			*(int*)dst = state->error_num;
			break;
		case REMOVEFILE_STATE_STATUS_CALLBACK:
			*(removefile_callback_t*)dst = state->status_callback;
			break;
		case REMOVEFILE_STATE_STATUS_CONTEXT:
			*(void**)dst = state->status_context;
			break;
		default:
			errno = EINVAL;
			return -1;
	}
	return 0;
}

int
removefile_state_set(removefile_state_t state, uint32_t key, const void* value) {
	switch(key) {
		case REMOVEFILE_STATE_CONFIRM_CALLBACK:
			state->confirm_callback = value;
			break;
		case REMOVEFILE_STATE_CONFIRM_CONTEXT:
			state->confirm_context = (void *) value;
			break;
		case REMOVEFILE_STATE_ERROR_CALLBACK:
			state->error_callback = value;
			break;
		case REMOVEFILE_STATE_ERROR_CONTEXT:
			state->error_context = (void *) value;
			break;
		case REMOVEFILE_STATE_ERRNO:
			state->error_num = *(int*)value;
			break;
		case REMOVEFILE_STATE_STATUS_CALLBACK:
			state->status_callback = value;
			break;
		case REMOVEFILE_STATE_STATUS_CONTEXT:
			state->status_context = (void *) value;
			break;
		default:
			errno = EINVAL;
			return -1;
   }
   return 0;
}

int
removefile(const char* path, removefile_state_t state_param, removefile_flags_t flags) {
	int res = 0;
	char* paths[] = { NULL, NULL };
	removefile_state_t state = state_param;

	// allocate the state if it was not passed in
	if (state_param == NULL) {
		state = removefile_state_alloc();
		if (state == NULL) {
			errno = ENOMEM;
			return -1;
		}
	}
	state->cancelled = 0;
	state->unlink_flags = flags;

	if (flags & (REMOVEFILE_SECURE_7_PASS | REMOVEFILE_SECURE_35_PASS | REMOVEFILE_SECURE_1_PASS | REMOVEFILE_SECURE_3_PASS)) {
		__removefile_init_random(getpid(), state);
	}

	paths[0] = strdup(path);
	if (paths[0]) {
		res = __removefile_tree_walker(paths, state);
		free(paths[0]);
	} else {
		errno = ENOMEM;
		res = -1;
	}

	// deallocate if allocated locally
	if (state_param == NULL) {
		removefile_state_free(state);
	}

	return res;
}

int
removefile_cancel(removefile_state_t state) {
	if (state == NULL) {
		errno = EINVAL;
		return -1;
	} else {
		state->cancelled = 1;
		return 0;
	}
}
