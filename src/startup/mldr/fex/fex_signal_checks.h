#ifndef _MLDR_FEX_SIGNAL_CHECKS_H_
#define _MLDR_FEX_SIGNAL_CHECKS_H_

// Decisions of the guest signal path that need no FEX, so that tests/mldr/fex_bridge.c can check them.

#include <stdbool.h>
#include <stdint.h>
#include "fex_callback_frames.h"

// FEXCore's call-return stack: generated code moves its pointer down 16 bytes per guest call and up per ret.
// Calls the guest leaves by longjmp are never popped, so the pointer drifts into the guard pages around the
// mapping; the entries are only hints, and a fault there is answered by putting the pointer back at the start.
static inline uint64_t fex_callret_start(uint64_t base, uint64_t size)
{
	return base + size / 4; // FEX's start for a new thread: room below for calls, above for returns
}

// Whether a fault at `addr` is the call-return pointer leaving the mapping `base`, `size` (guard pages included).
static inline bool fex_callret_fault(uint64_t addr, uint64_t base, uint64_t size, uint64_t guard)
{
	return addr >= base - guard && addr < base + size + guard;
}

// The saved host context in a guest signal frame is read back from memory the guest can write, so rt_sigreturn
// refuses a context that is not the one this process saved for this frame.
enum fex_backup_verdict {
	FEX_BACKUP_OK,
	FEX_BACKUP_UNREADABLE, // the frame or its saved context is not mapped
	FEX_BACKUP_BAD_SLOT,   // the frame does not point at its own saved context
	FEX_BACKUP_BAD_COOKIE, // not written by this process, or already used
	FEX_BACKUP_BAD_FRAME,  // saved for another frame
	FEX_BACKUP_BAD_SP,     // a host stack pointer this thread's dispatcher cannot have been running on
};

// `slot` is the address the frame holds, `expected` where the context lies; `saved_frame` and `frame` are the
// frame address the context was saved for and the one rt_sigreturn came through; `saved_sp` is the host stack
// pointer saved and `sp` the current one: the dispatcher of the handler ran at or below the saved one.
static inline enum fex_backup_verdict fex_backup_check(bool readable, uint64_t slot, uint64_t expected, uint64_t cookie,
	uint64_t secret, uint64_t saved_frame, uint64_t frame, uint64_t saved_sp, uint64_t sp)
{
	if (!readable)
		return FEX_BACKUP_UNREADABLE;
	if (slot != expected)
		return FEX_BACKUP_BAD_SLOT;
	if (cookie != secret)
		return FEX_BACKUP_BAD_COOKIE;
	if (saved_frame != frame)
		return FEX_BACKUP_BAD_FRAME;
	if (saved_sp < sp || saved_sp - sp > (64ull << 20))
		return FEX_BACKUP_BAD_SP;
	return FEX_BACKUP_OK;
}

// Linux sends SIGSEGV when the signal frame does not fit the stack the handler is to run on. A frame that
// starts at `entry_sp` and ends below `top` fits the alternate stack when it lies on it; on any other stack
// the write itself finds out.
static inline bool fex_frame_fits(uint64_t entry_sp, uint64_t top, const struct fex_guest_stack* altstack)
{
	return !fex_on_guest_stack(altstack, top) || entry_sp >= altstack->sp;
}

#endif // _MLDR_FEX_SIGNAL_CHECKS_H_
