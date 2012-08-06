#ifndef LOCKSET_H
#define LOCKSET_H
#include <mach/kern_return.h>
#include "task.h"

struct lock_set;
typedef lock_set* lock_set_t;

extern "C" {

kern_return_t lock_acquire(lock_set_t lock_set, int lock_id);

kern_return_t lock_handoff(lock_set_t lock_set, int lock_id);

kern_return_t lock_handoff_accept(lock_set_t lock_set, int lock_id);

kern_return_t lock_make_stable(lock_set_t lock_set, int lock_id);

kern_return_t lock_release(lock_set_t lock_set, int lock_id);

kern_return_t lock_set_create(darwin_task_t* task, lock_set_t* lock_set, int locks, int policy);

kern_return_t lock_set_destroy (darwin_task_t* task, lock_set_t lock_set);

kern_return_t lock_try (lock_set_t lock_set, int lock_id);

}


#endif
