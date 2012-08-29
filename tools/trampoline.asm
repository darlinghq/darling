BITS 64
section text

global trampoline
trampoline:
	push rax
	push rbx
	push rdi
	push rsi
	push rdx
	push rcx
	push r8
	push r9
	sub rsp, 128 ; 8*16
	movdqu [rsp], xmm0
	movdqu [rsp+16], xmm1
	movdqu [rsp+32], xmm2
	movdqu [rsp+48], xmm3
	movdqu [rsp+64], xmm4
	movdqu [rsp+80], xmm5
	movdqu [rsp+96], xmm6
	movdqu [rsp+112], xmm7

	mov rdi, 0x123456
	mov rdx, qword 0xaabbccddeeff
	call rdx

	mov r11, rax ; save the right addr

	movdqu xmm7, [rsp+112]
	movdqu xmm6, [rsp+96]
	movdqu xmm5, [rsp+80]
	movdqu xmm4, [rsp+64]
	movdqu xmm3, [rsp+48]
	movdqu xmm2, [rsp+32]
	movdqu xmm1, [rsp+16]
	movdqu xmm0, [rsp]
	add rsp, 128
	pop r9
	pop r8
	pop rcx
	pop rdx
	pop rsi
	pop rdi
	pop rbx
	pop rax

	jmp r11

