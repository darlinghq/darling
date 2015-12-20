#define LAZY_INFO_OFFSET 28
#define IMAGE_LOADER_OFFSET 24

// extern dyld_stub_binder_fixup ; void* dyld_stub_binder_fixup(void* cache, uintptr_t index)

.syntax unified


/*    
 * sp+4	lazy binding info offset
 * sp+0	address of ImageLoader cache
 */
	.text
	.align 2
	.globl	dyld_stub_binder
	.type dyld_stub_binder, %function 
dyld_stub_binder:
	.fnstart
	stmfd	sp!, {r0,r1,r2,r3,r7,lr}	// save registers
	add	r7, sp, #16			// point FP to previous FP

	ldr	r0, [sp, #IMAGE_LOADER_OFFSET]			// move address ImageLoader cache to 1st parameter
	ldr	r1, [sp, #LAZY_INFO_OFFSET]			// move lazy info offset 2nd parameter

	// call dyld_stub_binder_fixup(loadercache, index)
	bl	dyld_stub_binder_fixup
	mov	ip, r0				// move the symbol`s address into ip

	ldmfd	sp!, {r0,r1,r2,r3,r7,lr}	// restore registers
	add	sp, sp, #8			// remove meta-parameters

	bx	ip				// jump to the symbol`s address that was bound
	.fnend
