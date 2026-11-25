/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * vm_remap() and mach_vm_remap() map a region of one task's address space into another
 * (here: into the same task). vm_remap() takes pointer-sized addresses and sizes, so on i386
 * its arguments are 32 bits wide; it must give the results mach_vm_remap() gives: with
 * copy set the new region holds a private copy, without it the region is shared with the
 * source, VM_FLAGS_ANYWHERE lets the system choose the address, VM_FLAGS_FIXED with
 * VM_FLAGS_OVERWRITE replaces what is at the address.
 */
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/vm_map.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;

static void expectTrue(int ok, const char *what) {
	printf("%s %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

#define SIZE 8192

int main(void) {
	setvbuf(stdout, NULL, _IONBF, 0);
	task_t self = mach_task_self();

	vm_address_t src = 0;
	expectTrue(vm_allocate(self, &src, SIZE, VM_FLAGS_ANYWHERE) == KERN_SUCCESS, "source region allocated");
	memset((void *) src, 0x5a, SIZE);

	/* copy, anywhere */
	vm_address_t dest = 0;
	vm_prot_t cur = 0, max = 0;
	kern_return_t kr = vm_remap(self, &dest, SIZE, 0, VM_FLAGS_ANYWHERE, self, src, TRUE, &cur, &max, VM_INHERIT_COPY);
	expectTrue(kr == KERN_SUCCESS && dest != 0 && dest != src, "vm_remap with copy, anywhere");
	if (kr == KERN_SUCCESS && dest != 0) {
		expectTrue(((unsigned char *) dest)[0] == 0x5a && ((unsigned char *) dest)[SIZE - 1] == 0x5a,
		           "the copy holds the contents of the source");
		((unsigned char *) dest)[0] = 0x11;
		expectTrue(((unsigned char *) src)[0] == 0x5a, "writing the copy leaves the source alone");
		vm_deallocate(self, dest, SIZE);
	}

	/* shared, anywhere */
	dest = 0;
	kr = vm_remap(self, &dest, SIZE, 0, VM_FLAGS_ANYWHERE, self, src, FALSE, &cur, &max, VM_INHERIT_SHARE);
	expectTrue(kr == KERN_SUCCESS && dest != 0 && dest != src, "vm_remap without copy, anywhere");
	if (kr == KERN_SUCCESS && dest != 0) {
		((unsigned char *) dest)[1] = 0x22;
		expectTrue(((unsigned char *) src)[1] == 0x22, "the shared region shows writes to either mapping");
		vm_deallocate(self, dest, SIZE);
	}

	/* fixed, overwriting an existing region */
	vm_address_t target = 0;
	vm_allocate(self, &target, SIZE, VM_FLAGS_ANYWHERE);
	memset((void *) target, 0, SIZE);
	dest = target;
	kr = vm_remap(self, &dest, SIZE, 0, VM_FLAGS_FIXED | VM_FLAGS_OVERWRITE, self, src, TRUE, &cur, &max, VM_INHERIT_COPY);
	expectTrue(kr == KERN_SUCCESS && dest == target, "vm_remap with copy, fixed address, overwriting");
	if (kr == KERN_SUCCESS)
		expectTrue(((unsigned char *) target)[SIZE - 1] == 0x5a, "the fixed region holds the source contents");
	vm_deallocate(self, target, SIZE);

	/* the same through mach_vm_remap: both entry points agree */
	mach_vm_address_t mdest = 0;
	kr = mach_vm_remap(self, &mdest, SIZE, 0, VM_FLAGS_ANYWHERE, self, src, TRUE, &cur, &max, VM_INHERIT_COPY);
	expectTrue(kr == KERN_SUCCESS && mdest != 0 && *(unsigned char *) (uintptr_t) mdest == 0x5a,
	           "mach_vm_remap with copy, anywhere");
	if (kr == KERN_SUCCESS)
		mach_vm_deallocate(self, mdest, SIZE);

	vm_deallocate(self, src, SIZE);
	printf("%s\n", failures ? "FAIL" : "PASS");
	return failures != 0;
}
