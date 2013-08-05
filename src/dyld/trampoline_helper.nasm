; 
; This file is part of Darling.
; 
; Copyright (C) 2012 Lubos Dolezel
; 
; Darling is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
; 
; Darling is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.

; You should have received a copy of the GNU General Public License
; along with Darling.  If not, see <http://www.gnu.org/licenses/>.

section .note.GNU-stack noalloc noexec nowrite progbits

global trampoline_start
global trampoline_end
global reg_saveall
global reg_restoreall

%ifidn __OUTPUT_FORMAT__, elf64

BITS 64
section text

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
trampoline_start:
	mov r10, qword 0xb0b1b2b3b4b5b6
	call r10 ; call reg_saveall

	mov rdi, 0x123456 ; index in entry table
	mov rsi, rsp
	mov rcx, qword 0xaabbccddeeff ; print function (saves orig ret address)

	;sub rsp, 8 ; align stack to 16
	call rcx
	;add rsp, 8

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
	
	;sub rsp, 8 ; align stack to 16
	call rcx
	;add rsp, 8
	
	mov r11, rax

	mov r10, qword 0xc0c1c2c3c4c5c6 ; call reg_restoreall
	call r10
	jmp r11 ; now we "return" to the original function
trampoline_end:
	nop
%elifidn __OUTPUT_FORMAT__, elf

BITS 32
section text

%define SavedRegLen 32

reg_saveall: ; 24 bytes on stack
	pop eax

	push ebx
	push ecx
	push edx
	push esi
	push edi
	sub esp, 8
	fstp qword [esp]

	jmp eax

reg_restoreall:
	pop ebx
	mov eax, [esp+SavedRegLen-4]
	mov [esp+SavedRegLen-4], ebx

	fld qword [esp]
	add esp, 8
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx

	ret

trampoline_start:
	push eax
	mov eax, 0xa0a1a2a3 ; call reg_saveall
	call eax
	
	push esp
	push 0x12345678 ; index in entry table
	mov eax, 0xb0b1b2b3
	call eax ; call print function
	add esp, 8

	mov [esp-8], eax
	mov eax, 0xc0c1c2c3 ; call reg_restoreall
	call eax

	; we cannot use IP-relative lea on 32bit x86
	call get_eip
get_eip:
	pop eax
	add eax, 0xD ; this gives us the address of after_jump
	mov [esp], eax
	mov eax, [esp-SavedRegLen-8]

	jmp eax ; call the real function

ALIGN 2
after_jump:
	push eax
	mov eax, 0xa0a1a2a3
	call eax ; call reg_saveall
	push esp
	push 0x12345678 ; index in entry table
	mov eax, 0xb9b9b9b9 ; print function 2
	call eax
	add esp, 8

	mov [esp-8], eax ; real return address
	mov eax, 0xc0c1c2c3
	call eax ; call reg_restoreall

	mov ecx, [esp-SavedRegLen-8]
	jmp ecx ; return to the original caller
trampoline_end:
	nop
%else

%error "Unsupported platform"

%endif
