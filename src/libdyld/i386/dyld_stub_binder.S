.text
.globl dyld_stub_binder

#ifdef __i386__

dyld_stub_binder:

	pushl %ebx
	
	calll 1f
1:
	popl %ebx
2:
	addl $_GLOBAL_OFFSET_TABLE_+(2b-1b), %ebx

	pushl %eax
	call reg_saveall@PLT  // 32 bytes
	movl 32+4(%esp), %eax // cache
	movl 32+8(%esp), %ecx // offset
	pushl %ecx
	pushl %eax

	call dyld_stub_binder_fixup@PLT
	addl $8, %esp

	movl %eax, 32+4(%esp) // save the real addr somewhere
	call reg_restoreall@PLT

	movl 4(%esp), %eax // restore the real addr
	
	popl %ebx
	addl $8, %esp // remove arguments to dyld_stub_binder THIS SHOULDN'T WORK!
	
	jmpl *%eax

#endif

.section ".note.GNU-stack","",@progbits

