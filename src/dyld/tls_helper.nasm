;
; This file is part of Darling.
; 
; Copyright (C) 2013 Lubos Dolezel
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
;
; You should have received a copy of the GNU General Public License
; along with Darling.  If not, see <http://www.gnu.org/licenses/>.
;


section .note.GNU-stack noalloc noexec nowrite progbits

extern darling_tls_get_native
extern darling_tls_allocate

global darling_tls_get_addr

%ifidn __OUTPUT_FORMAT__, elf64

BITS 64
section text

darling_tls_get_addr:
	call darling_tls_get_native ; fast three-instruction function
	
	push rdi
	mov rdi, [rdi+8]
	mov rax, [rax+rdi*8]
	pop rdi
	
	test rax, rax
	jz slow_path

	add rax, [rdi+16] ; add the variable offset
	ret

slow_path:
	enter 592, 0
	
	; Do a register full-save, because the caller assumes that no registers
	; except for rax/rdi will be modified
	mov [rbp-8],  rdi
	mov [rbp-16], rsi
	mov [rbp-24], rdx
	mov [rbp-32], rcx
	mov [rbp-40], r8
	mov [rbp-48], r9
	mov [rbp-56], r10
	mov [rbp-64], r11
	fxsave [rbp-592]
	
	mov rdi, [rdi+8] ; pass the pthread_key
	call darling_tls_allocate
	
	fxrstor  [rbp-592]
	mov r11, [rbp-64]
	mov r10, [rbp-56]
	mov r9,  [rbp-48]
	mov r8,  [rbp-40]
	mov rcx, [rbp-32]
	mov rdx, [rbp-24]
	mov rsi, [rbp-16]
	mov rdi, [rbp-8]
	
	add rax, [rdi+16] ; add the variable offset
	leave
	ret

%elifidn __OUTPUT_FORMAT__, elf

BITS 32
section text

darling_tls_get_addr:
	mov ecx, eax     ; eax will get destroyed
	call darling_tls_get_native ; fast three-instruction function
	push ecx
	
	mov ecx, [ecx+4] ; add the pthread key
	mov eax, [eax+ecx*4]   ; read the value at the key

	test eax, eax
	jz slow_path
	
	pop ecx
	add eax, [ecx+8] ; add the offset
	ret

slow_path:
	mov ecx, [esp]
	push dword [ecx+4]     ; pass the pthread_key
	call darling_tls_allocate
	pop ecx
	pop ecx
	
	add eax, [ecx+8] ; add the offset
	ret

%else

%error "Unsupported platform"

%endif

