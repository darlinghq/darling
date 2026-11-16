// Copyright (C) 2026 Lubos Dolezel
//
// Host-side test of mldr's executable-memory policy: a 32-bit x86 executable
// without MH_NO_HEAP_EXECUTION gets an executable heap; MH_ALLOW_STACK_EXECUTION
// makes data and the stack executable on both x86 architectures; nothing else
// does. The behaviour is from Mojave 10.14.4 black-box runs of vd-upstream's
// heapx probes (code copied into malloc, mmap, data, bss, stack and file mappings).
#include <stdio.h>
#include "exec_policy.h"

#define MH_ALLOW_STACK_EXECUTION 0x20000u
#define MH_NO_HEAP_EXECUTION 0x1000000u
#define MH_PIE 0x200000u

static int failures;
static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

static void expect(bool is_i386, unsigned flags, bool data, bool stack, const char* what)
{
	struct mldr_exec_policy p = mldr_exec_policy_for(is_i386, flags);
	check(p.exec_data == data && p.exec_stack == stack, what);
}

int main(void)
{
	expect(true, 0, true, false, "i386 without MH_NO_HEAP_EXECUTION: executable heap, no executable stack");
	expect(true, MH_PIE, true, false, "i386 with unrelated flags: executable heap");
	expect(true, MH_NO_HEAP_EXECUTION, false, false, "i386 with MH_NO_HEAP_EXECUTION: nothing executable");
	expect(true, MH_NO_HEAP_EXECUTION | MH_PIE, false, false, "i386 NO_HEAP_EXECUTION|PIE: nothing executable");
	expect(false, 0, false, false, "x86_64 without MH_NO_HEAP_EXECUTION: nothing executable");
	expect(false, MH_NO_HEAP_EXECUTION, false, false, "x86_64 with MH_NO_HEAP_EXECUTION: nothing executable");
	expect(true, MH_ALLOW_STACK_EXECUTION, true, true, "i386 MH_ALLOW_STACK_EXECUTION: data and stack executable");
	expect(true, MH_ALLOW_STACK_EXECUTION | MH_NO_HEAP_EXECUTION, true, true,
		"i386 MH_ALLOW_STACK_EXECUTION with MH_NO_HEAP_EXECUTION: data and stack executable");
	expect(false, MH_ALLOW_STACK_EXECUTION, true, true, "x86_64 MH_ALLOW_STACK_EXECUTION: data and stack executable");
	expect(false, MH_ALLOW_STACK_EXECUTION | MH_NO_HEAP_EXECUTION, true, true,
		"x86_64 MH_ALLOW_STACK_EXECUTION with MH_NO_HEAP_EXECUTION: data and stack executable");
	return failures ? 1 : 0;
}
