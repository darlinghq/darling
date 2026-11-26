/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * mach_vm_remap() without copy maps the pages of a source region into the target task so that both
 * mappings show the same memory. With VM_FLAGS_FIXED the new mapping goes to the address given in
 * *address (not to an address chosen by the system); with VM_FLAGS_OVERWRITE added it may replace
 * what is mapped there, and without it a range that is in use makes the call fail.
 */
#include <mach/mach.h>
#include <mach/mach_vm.h>
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
	vm_prot_t cur = 0, max = 0;
	kern_return_t kr;

	mach_vm_address_t src = 0;
	expectTrue(mach_vm_allocate(self, &src, SIZE, VM_FLAGS_ANYWHERE) == KERN_SUCCESS, "source region allocated");
	memset((void *) (uintptr_t) src, 0x5a, SIZE);
	unsigned char *s = (unsigned char *) (uintptr_t) src;

	/* fixed address, overwriting a reservation: the mapping lands exactly there and is shared */
	mach_vm_address_t target = 0;
	expectTrue(mach_vm_allocate(self, &target, SIZE, VM_FLAGS_ANYWHERE) == KERN_SUCCESS, "reservation allocated");
	memset((void *) (uintptr_t) target, 0x11, SIZE);
	mach_vm_address_t dest = target;
	kr = mach_vm_remap(self, &dest, SIZE, 0, VM_FLAGS_FIXED | VM_FLAGS_OVERWRITE, self, src, FALSE, &cur, &max, VM_INHERIT_SHARE);
	expectTrue(kr == KERN_SUCCESS, "shared remap, fixed, overwrite succeeds");
	expectTrue(dest == target, "the address given is kept");
	if (kr == KERN_SUCCESS && dest == target) {
		unsigned char *d = (unsigned char *) (uintptr_t) target;
		expectTrue(d[0] == 0x5a && d[SIZE - 1] == 0x5a, "the old contents are replaced by the source's");
		d[1] = 0x22;
		expectTrue(s[1] == 0x22, "a write through the new mapping shows in the source");
		s[SIZE - 1] = 0x33;
		expectTrue(d[SIZE - 1] == 0x33, "a write through the source shows in the new mapping");
	}
	mach_vm_deallocate(self, target, SIZE);

	/* fixed address in free space, no overwrite */
	mach_vm_address_t freespot = 0;
	mach_vm_allocate(self, &freespot, SIZE, VM_FLAGS_ANYWHERE);
	mach_vm_deallocate(self, freespot, SIZE);
	dest = freespot;
	kr = mach_vm_remap(self, &dest, SIZE, 0, VM_FLAGS_FIXED, self, src, FALSE, &cur, &max, VM_INHERIT_SHARE);
	expectTrue(kr == KERN_SUCCESS && dest == freespot, "shared remap, fixed, free range, keeps the address");
	if (kr == KERN_SUCCESS)
		mach_vm_deallocate(self, dest, SIZE);

	/* fixed address on a range in use, no overwrite: refused, and the range is left alone */
	mach_vm_address_t busy = 0;
	mach_vm_allocate(self, &busy, SIZE, VM_FLAGS_ANYWHERE);
	memset((void *) (uintptr_t) busy, 0x44, SIZE);
	dest = busy;
	kr = mach_vm_remap(self, &dest, SIZE, 0, VM_FLAGS_FIXED, self, src, FALSE, &cur, &max, VM_INHERIT_SHARE);
	expectTrue(kr == KERN_NO_SPACE, "shared remap, fixed, range in use, no overwrite fails with KERN_NO_SPACE");
	expectTrue(((unsigned char *) (uintptr_t) busy)[0] == 0x44, "the range in use is unchanged");
	mach_vm_deallocate(self, busy, SIZE);

	/* anywhere: unchanged behaviour, a different address, shared */
	dest = 0;
	kr = mach_vm_remap(self, &dest, SIZE, 0, VM_FLAGS_ANYWHERE, self, src, FALSE, &cur, &max, VM_INHERIT_SHARE);
	expectTrue(kr == KERN_SUCCESS && dest != 0 && dest != src, "shared remap, anywhere");
	if (kr == KERN_SUCCESS) {
		((unsigned char *) (uintptr_t) dest)[2] = 0x66;
		expectTrue(s[2] == 0x66, "the anywhere mapping is shared too");
		mach_vm_deallocate(self, dest, SIZE);
	}

	mach_vm_deallocate(self, src, SIZE);
	printf("%s\n", failures ? "FAIL" : "PASS");
	return failures != 0;
}
