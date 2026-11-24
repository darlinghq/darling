#ifndef _MLDR_FEX_CALLBACK_FRAMES_H_
#define _MLDR_FEX_CALLBACK_FRAMES_H_

// Which guest callbacks a guest has left without returning from them, decided from its stack pointer.
// Kept free of FEX so that it can be tested on its own (tests/mldr/fex_bridge.c).

#include <stdbool.h>
#include <stdint.h>
#include "fex_bridge.h"

// A guest callback running nested in a host call on this thread, innermost first.
struct fex_callback_frame {
	// The guest's stack pointer in the host call: the callback's frames lie at least 16 bytes (the return
	// slot its entry pushes) below it, and a callback that returns leaves exactly this stack pointer.
	uint64_t caller_rsp;
	// The guest stack that holds caller_rsp, [stack_lo, stack_hi); stack_hi == 0 if the bridge does not
	// know it, and then same_stack below decides.
	uint64_t stack_lo, stack_hi;
	const struct fex_callback_frame* outer;
};

static inline bool fex_on_guest_stack(const struct fex_guest_stack* s, uint64_t sp)
{
	return s && !(s->flags & FEX_GUEST_SS_DISABLE) && sp > s->sp && sp - s->sp <= s->size;
}

// The outermost callback the guest has left, given its stack pointer `rsp` as a syscall starts, or NULL.
// A callback is left when rsp is at or above its host call's stack pointer on the same stack. rsp on
// another stack (a coroutine stack the callback switched to, a handler on the alternate stack `altstack`
// when the host call was not on it) means the callback is still running. same_stack(a, b) says whether
// b lies on the stack that holds a.
static inline const struct fex_callback_frame* fex_left_callback(const struct fex_callback_frame* top, uint64_t rsp,
	const struct fex_guest_stack* altstack, bool (*same_stack)(uint64_t a, uint64_t b))
{
	const struct fex_callback_frame* left = NULL;
	for (const struct fex_callback_frame* cb = top; cb && rsp >= cb->caller_rsp; cb = cb->outer) {
		bool same = cb->stack_hi ? (rsp >= cb->stack_lo && rsp < cb->stack_hi) : same_stack(cb->caller_rsp, rsp);
		if (!same || (fex_on_guest_stack(altstack, rsp) && !fex_on_guest_stack(altstack, cb->caller_rsp)))
			break;
		left = cb;
	}
	return left;
}

// A callback's dispatcher returned with the guest stack pointer `rsp`: the callback whose host call has
// that stack pointer is the one that returned (the ones nested in it were left), or NULL if none has.
static inline const struct fex_callback_frame* fex_returned_callback(const struct fex_callback_frame* top, uint64_t rsp)
{
	for (const struct fex_callback_frame* cb = top; cb; cb = cb->outer)
		if (cb->caller_rsp == rsp)
			return cb;
	return NULL;
}

// FEXCore's call-return stack holds a guest call's return target for the matching `ret`; the generated code
// moves its pointer down for every call and up for every ret, and a mismatching entry only costs the slower
// lookup. A guest that leaves calls without returning from them (longjmp out of a signal handler, for one)
// leaves the pointer lower by the depth it left, every time, until it runs off the guard page below the
// mapping. The pointer is a hint, so it may be set to the start of the mapping at any point where the guest
// is in a syscall (the generated code reloads it afterwards).
static inline uint64_t fex_callret_start(uint64_t base, uint64_t size)
{
	return base + size / 4; // FEX's own choice for a new thread: room below for calls, above for returns
}

// `sp` as the guest's next syscall should leave it: unchanged while it is further than a sixteenth of the
// mapping from either end, otherwise back at the start.
static inline uint64_t fex_callret_resync(uint64_t sp, uint64_t base, uint64_t size)
{
	const uint64_t margin = size / 16;
	return sp >= base + margin && sp <= base + size - margin ? sp : fex_callret_start(base, size);
}

#endif // _MLDR_FEX_CALLBACK_FRAMES_H_
