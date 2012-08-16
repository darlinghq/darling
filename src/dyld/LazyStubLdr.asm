SEGMENT .text

extern lazystub_resolve

global lazystub_loader64
lazystub_loader64:
	push rdi
	push rsi
	push rdx
	push rcx
	push r8
	push r9
	push xmm0
	push xmm1
	push xmm2
	push xmm3
	push xmm4
	push xmm5
	push xmm6
	push xmm7

	xor rax, rax
	mov edx, ebx
	call lazystub_resolve

	pop xmm7
	pop xmm6
	pop xmm5
	pop xmm4
	pop xmm3
	pop xmm2
	pop xmm1
	pop r9
	pop r8
	pop rcx
	pop rdx
	pop rsi
	pop rdi

	jmp rax

