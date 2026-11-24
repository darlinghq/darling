/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * vm_remap() and mach_vm_remap() refuse a source task that is not a task port: they return
 * an error (KERN_INVALID_ARGUMENT in the kernel's contract for a null map) and leave the
 * caller and the system running.
 */
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/vm_map.h>
#include <stdio.h>

static int failures;

static void expectTrue(int ok, const char *what) {
	printf("%s %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

int main(void) {
	setvbuf(stdout, NULL, _IONBF, 0);
	task_t self = mach_task_self();
	vm_address_t src = 0;
	vm_allocate(self, &src, 4096, VM_FLAGS_ANYWHERE);
	vm_prot_t cur, max;

	vm_address_t dest = 0;
	kern_return_t kr = vm_remap(self, &dest, 4096, 0, VM_FLAGS_ANYWHERE, MACH_PORT_NULL, src, TRUE, &cur, &max, VM_INHERIT_COPY);
	expectTrue(kr != KERN_SUCCESS, "vm_remap from a null task fails");

	dest = 0;
	kr = vm_remap(self, &dest, 4096, 0, VM_FLAGS_ANYWHERE, MACH_PORT_NULL, src, FALSE, &cur, &max, VM_INHERIT_SHARE);
	expectTrue(kr != KERN_SUCCESS, "vm_remap without copy from a null task fails");

	mach_vm_address_t mdest = 0;
	kr = mach_vm_remap(self, &mdest, 4096, 0, VM_FLAGS_ANYWHERE, MACH_PORT_NULL, src, TRUE, &cur, &max, VM_INHERIT_COPY);
	expectTrue(kr != KERN_SUCCESS, "mach_vm_remap from a null task fails");

	/* the process and the system still work afterwards */
	dest = 0;
	expectTrue(vm_allocate(self, &dest, 4096, VM_FLAGS_ANYWHERE) == KERN_SUCCESS, "Mach calls still work afterwards");
	printf("%s\n", failures ? "FAIL" : "PASS");
	return failures != 0;
}
