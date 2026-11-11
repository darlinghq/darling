// Copyright (C) 2026 Lubos Dolezel
//
// libquarantine's process quarantine state (_qtn_proc_*): a new state has
// no flags, flags round-trip, the current process has no quarantine state
// to read, and applying an empty state to the current process succeeds.
// The same results were observed on macOS 27.0.1 (arm64 and x86_64).
//
// The prototypes are not in a public header; they are inferred from callers.
#include <stdint.h>
#include <stdio.h>

typedef struct _qtn_proc *qtn_proc_t;
extern qtn_proc_t _qtn_proc_alloc(void);
extern void _qtn_proc_free(qtn_proc_t proc);
extern int _qtn_proc_init_with_self(qtn_proc_t proc);
extern int _qtn_proc_set_flags(qtn_proc_t proc, uint32_t flags);
extern uint32_t _qtn_proc_get_flags(qtn_proc_t proc);
extern int _qtn_proc_apply_to_self(qtn_proc_t proc);

static int failures;

static void check(const char *name, int ok)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
	failures += !ok;
}

int main(void)
{
	qtn_proc_t proc = _qtn_proc_alloc();
	check("_qtn_proc_alloc returns a state", proc != NULL);
	if (!proc)
		return 1;

	check("a new state has no flags", _qtn_proc_get_flags(proc) == 0);
	check("the current process has no quarantine state",
	      _qtn_proc_init_with_self(proc) == -1);

	check("_qtn_proc_set_flags succeeds", _qtn_proc_set_flags(proc, 5) == 0);
	check("flags round-trip", _qtn_proc_get_flags(proc) == 5);

	_qtn_proc_set_flags(proc, 0);
	check("applying an empty state to the current process succeeds",
	      _qtn_proc_apply_to_self(proc) == 0);

	_qtn_proc_free(proc);

	printf("failures=%d\n", failures);
	return failures != 0;
}
