#ifdef __arm64__

#define LIBFFI_ASM
#include <fficonfig.h>
#include <ffi.h>
#include <ffi_cfi.h>
#include "internal.h"

#ifdef FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#include <mach/machine/vm_param.h>
#endif

#ifdef HAVE_MACHINE_ASM_H
# include <machine/asm.h>
#else
# ifdef __USER_LABEL_PREFIX__
#  define CONCAT1(a, b) CONCAT2(a, b)
#  define CONCAT2(a, b) a ## b
#  define CNAME(x) CONCAT1 (__USER_LABEL_PREFIX__, x)
# else
#  define CNAME(x) x
# endif
#endif

.set page_max_size, PAGE_MAX_SIZE
.align PAGE_MAX_SHIFT
.text
.globl CNAME(ffi_closure_trampoline_table_page)
CNAME(ffi_closure_trampoline_table_page):
    .rept PAGE_MAX_SIZE / FFI_TRAMPOLINE_SIZE
#ifdef FFI_TRAMPOLINE_WHOLE_DYLIB
    adr x16, -(2 * PAGE_MAX_SIZE)
#else
    adr x16, -PAGE_MAX_SIZE
#endif
    ldp x17, x16, [x16]
    BRANCH_TO_REG x16
	nop		/* each entry in the trampoline config page is 2*sizeof(void*) so the trampoline itself cannot be smaller that 16 bytes */
    .endr

#endif /* FFI_EXEC_TRAMPOLINE_TABLE */
#endif /* __arm64 */
