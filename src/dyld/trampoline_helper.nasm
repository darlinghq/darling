%ifidn __OUTPUT_FORMAT__, elf64

BITS 64
section text

;global trampoline
global reg_saveall
global reg_restoreall

reg_saveall: ; 192 bytes on stack
	pop r10 ; remove retaddr from stack
	push rax
	push rbx
	push rdi
	push rsi
	push rdx
	push rcx
	push r8
	push r9
	push r12
	push r13
	push r14
	push r15
	sub rsp, 128 ; 8*16
	movdqu [rsp], xmm0
	movdqu [rsp+16], xmm1
	movdqu [rsp+32], xmm2
	movdqu [rsp+48], xmm3
	movdqu [rsp+64], xmm4
	movdqu [rsp+80], xmm5
	movdqu [rsp+96], xmm6
	movdqu [rsp+112], xmm7
	jmp r10

reg_restoreall:
	pop r10 ; remove retaddr from stack
	movdqu xmm7, [rsp+112]
    movdqu xmm6, [rsp+96]
    movdqu xmm5, [rsp+80]
    movdqu xmm4, [rsp+64]
    movdqu xmm3, [rsp+48]
    movdqu xmm2, [rsp+32]
    movdqu xmm1, [rsp+16]
    movdqu xmm0, [rsp]
    add rsp, 128
    pop r15
    pop r14
    pop r13
    pop r12
    pop r9
    pop r8
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rbx
    pop rax
	jmp r10

; This is duplicated for every mapped function
trampoline:
	mov r10, qword 0xb0b1b2b3b4b5b6
	call r10 ; call reg_saveall

	mov rdi, 0x123456 ; index in entry table
	mov rsi, rsp
	mov rcx, qword 0xaabbccddeeff ; print function (saves orig ret address)

	call rcx

	mov r11, rax ; save the right addr

	mov r10, qword 0xc0c1c2c3c4c5c6 ; call reg_restoreall
	call r10

	; rewrite return address
	lea r10, [rel after_jump]
	mov [rsp], r10; second print function
	jmp r11 ; jump to the real function

ALIGN 2
after_jump:
	
	mov r10, qword 0xb0b1b2b3b4b5b6
	call r10 ; call reg_saveall

	mov rdi, 0x123456
	mov rsi, rsp
	mov rcx, qword 0xa0a1a2a3a4a5a6 ; retval print function (returns orig ret address)
	call rcx
	mov r11, rax

	mov r10, qword 0xc0c1c2c3c4c5c6 ; call reg_restoreall
	call r10
	jmp r11 ; now we "return" to the original function

%elifidn __OUTPUT_FORMAT__, elf

BITS 32
section text

;global trampoline
global reg_saveall
global reg_restoreall

reg_saveall: ; 24 bytes on stack
	xchg eax, [esp]

	push ebx
	push ecx
	push edx
	push esi
	push edi

    jmp eax

reg_restoreall:
	pop ebx
	mov eax, [esp+24]
	mov [esp+24], ebx

	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx

	ret

trampoline:
	push 0xa0a1a2a3 ; call reg_saveall
	ret
	push esp
	push 0x12345678 ; index in entry table
	push 0xb0b1b2b3
	ret ; call print function

	mov [esp-8], eax
	push 0xc0c1c2c3 ; call reg_restoreall
	ret

	; we cannot use IP-relative lea on 32bit x86
	call get_eip
get_eip:
	mov eax, [esp]
	add eax, 0x10 ; this gives us the address of after_jump
	mov [esp], eax
	mov eax, [esp-32]

	jmp eax ; call the real function

ALIGN 2
after_jump:
	push 0xa0a1a2a3
	ret ; call reg_saveall
	push esp
	push 0x12345678 ; index in entry table
	push 0xb9b9b9b9 ; print function 2
	ret

	mov [esp-8], eax ; real return address
	push 0xc0c1c2c3
	ret ; call reg_restoreall

	mov ecx, [esp-32]
	jmp ecx ; return to the original caller

%else

%error "Unsupported platform"

%endif
