.globl __darwin_longjmp
.globl __darwin_setjmp

__darwin_longjmp:

ldmia r0!, { r4-r8, r10-r14 }
vldmia r0, { d8-d15 }
movs r0, r1
moveq r0, #1
bx r14

__darwin_setjmp:

stmia r0!, { r4-r8, r10-r14 }
vstmia r0, { d8-d15 }
mov r0, #0
bx r14

// TODO: signal save variants

