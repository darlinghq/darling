#ifndef _KERN_MPQUEUE_H
#define _KERN_MPQUEUE_H
#include <kern/locks.h>

__BEGIN_DECLS

#ifdef  MACH_KERNEL_PRIVATE

/*----------------------------------------------------------------*/
/*
 *	Define macros for queues with locks.
 */
struct mpqueue_head {
	struct queue_entry      head;           /* header for queue */
	uint64_t                earliest_soft_deadline;
	uint64_t                count;
	lck_mtx_t               lock_data;
#if defined(__i386__) || defined(__x86_64__)
	lck_mtx_ext_t           lock_data_ext;
#endif
};

typedef struct mpqueue_head     mpqueue_head_t;

#define round_mpq(size)         (size)


#if defined(__i386__) || defined(__x86_64__)

#define mpqueue_init(q, lck_grp, lck_attr)              \
MACRO_BEGIN                                             \
	queue_init(&(q)->head);                         \
	lck_mtx_init_ext(&(q)->lock_data,               \
	                 &(q)->lock_data_ext,           \
	                 lck_grp,                       \
	                 lck_attr);                     \
	(q)->earliest_soft_deadline = UINT64_MAX;       \
	(q)->count = 0;                                 \
MACRO_END

#else

#define mpqueue_init(q, lck_grp, lck_attr)              \
MACRO_BEGIN                                             \
	queue_init(&(q)->head);                         \
	lck_mtx_init(&(q)->lock_data,                   \
	              lck_grp,                          \
	              lck_attr);                        \
MACRO_END
#endif


#define mpenqueue_tail(q, elt)                          \
MACRO_BEGIN                                             \
	lck_mtx_lock_spin_always(&(q)->lock_data);      \
	enqueue_tail(&(q)->head, elt);                  \
	lck_mtx_unlock_always(&(q)->lock_data);         \
MACRO_END

#define mpdequeue_head(q, elt)                          \
MACRO_BEGIN                                             \
	lck_mtx_lock_spin_always(&(q)->lock_data);      \
	if (queue_empty(&(q)->head))                    \
	        *(elt) = 0;                             \
	else                                            \
	        *(elt) = dequeue_head(&(q)->head);      \
	lck_mtx_unlock_always(&(q)->lock_data);         \
MACRO_END

#endif  /* MACH_KERNEL_PRIVATE */

__END_DECLS


#endif /* _KERN_QUEUE_H */
