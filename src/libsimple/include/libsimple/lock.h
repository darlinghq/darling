#ifndef _LIBSIMPLE_LOCK_H_
#define _LIBSIMPLE_LOCK_H_

#include <stdint.h>
#include <stdbool.h>

#include "base.h"

LIBSIMPLE_DECLARATIONS_BEGIN;

//
// lock
//

typedef struct libsimple_lock {
	uint32_t state;
} libsimple_lock_t;

#define LIBSIMPLE_LOCK_INITIALIZER {0}

LIBSIMPLE_INLINE
static void libsimple_lock_init(libsimple_lock_t* lock) {
	lock->state = 0;
};

void libsimple_lock_lock(libsimple_lock_t* lock);
void libsimple_lock_unlock(libsimple_lock_t* lock);
bool libsimple_lock_try_lock(libsimple_lock_t* lock);

//
// once
//

typedef struct libsimple_once {
	uint32_t state;
} libsimple_once_t;

typedef void (*libsimple_once_callback)(void* context);

#define LIBSIMPLE_ONCE_INITIALIZER {0}

LIBSIMPLE_INLINE
static void libsimple_once_init(libsimple_once_t* once) {
	once->state = 0;
};

void libsimple_once(libsimple_once_t* once, libsimple_once_callback callback, void* context);

//
// rwlock
//

typedef struct libsimple_rwlock {
	uint32_t state;
} libsimple_rwlock_t;

#define LIBSIMPLE_RWLOCK_INITIALIZER {0}

LIBSIMPLE_INLINE
static void libsimple_rwlock_init(libsimple_rwlock_t* rwlock) {
	rwlock->state = 0;
};

void libsimple_rwlock_lock_read(libsimple_rwlock_t* rwlock);
bool libsimple_rwlock_try_lock_read(libsimple_rwlock_t* rwlock);
void libsimple_rwlock_unlock_read(libsimple_rwlock_t* rwlock);

void libsimple_rwlock_lock_write(libsimple_rwlock_t* rwlock);
bool libsimple_rwlock_try_lock_write(libsimple_rwlock_t* rwlock);
void libsimple_rwlock_unlock_write(libsimple_rwlock_t* rwlock);

LIBSIMPLE_DECLARATIONS_END;

#endif // _LIBSIMPLE_LOCK_H_
