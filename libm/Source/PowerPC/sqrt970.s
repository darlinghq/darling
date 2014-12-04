#import	<architecture/ppc/asm_help.h>
#import	<architecture/ppc/pseudo_inst.h>
#import	<architecture/ppc/mode_independent_asm.h>

.text
.align 2

.private_extern _hw_sqrt
_hw_sqrt:
Lhw_sqrt_end:
		nop
.set Lhw_sqrt_len, Lhw_sqrt_end - _hw_sqrt

.const
.private_extern _hw_sqrt_len
_hw_sqrt_len:
       .long Lhw_sqrt_len
