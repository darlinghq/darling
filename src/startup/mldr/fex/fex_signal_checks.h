#ifndef _MLDR_FEX_SIGNAL_CHECKS_H_
#define _MLDR_FEX_SIGNAL_CHECKS_H_

// Decisions of the guest signal path that need no FEX, so that tests/mldr/fex_bridge.c can check them.

#include <stdbool.h>
#include <stdint.h>

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

#endif // _MLDR_FEX_SIGNAL_CHECKS_H_
