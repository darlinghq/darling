.text
.globl dyld_stub_binder

#ifdef __x86_64__

// Stub identifier is pushed first
// then the code pushes a pointer to cache

dyld_stub_binder:

	call reg_saveall@PLT // 224 bytes
	movq 224(%rsp), %rdi // cache
	movq 224+8(%rsp), %rsi // offset
	subq $8, %rsp // maintain 16-byte stack alignment

	call dyld_stub_binder_fixup@PLT
	
	addq $8, %rsp // maintain 16-byte stack alignment

	movq %rax, 224(%rsp)
	call reg_restoreall@PLT
	
	movq (%rsp), %r11
	addq $16, %rsp // remove arguments to dyld_stub_binder
	jmpq *%r11

#endif

.section ".note.GNU-stack","",@progbits

