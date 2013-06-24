
global __darwin_setjmp
global __darwin_sigsetjmp
global __darwin_longjmp
global __darwin_siglongjmp

extern __darwin_savesigs
extern __darwin_restoresigs

section .note.GNU-stack noalloc noexec nowrite progbits

%ifidn __OUTPUT_FORMAT__, elf64

BITS 64
section text

__darwin_setjmp:
	mov rsi, 1

__darwin_sigsetjmp:

	test rsi, rsi
	jz .no_sig_save_z

	push rdi
	call __darwin_savesigs WRT ..plt
	pop rdi
	jmp .no_sig_save

.no_sig_save_z:
	mov dword [rdi+88], 0 ; zero out signal stack flags
.no_sig_save:
	mov rax, [rsp]
	lea rcx, [rsp+8]

	mov [rdi], rbp
	mov [rdi+8], rbx
	mov [rdi+16], r12
	mov [rdi+24], r13
	mov [rdi+32], r14
	mov [rdi+40], r15
	mov [rdi+48], rax ; ret addr
	mov [rdi+56], rcx ; rsp

	stmxcsr [rdi+64]
	fnstcw [rdi+68]

	xor eax, eax
	ret

__darwin_longjmp:
__darwin_siglongjmp:

	push rdi
	call __darwin_restoresigs WRT ..plt
	pop rdi

	mov eax, 1
	test esi, esi
	cmovnz eax, esi

	mov rbp, [rdi]
	mov rbx, [rdi+8]
	mov r12, [rdi+16]
	mov r13, [rdi+24]
	mov r14, [rdi+32]
	mov r15, [rdi+40]
	mov rsp, [rdi+56]

	fninit
	ldmxcsr [rdi+64]
	fldcw [rdi+68]
	cld

	jmp [rdi+48]

%elifidn __OUTPUT_FORMAT__, elf

BITS 32
section text

extern  _GLOBAL_OFFSET_TABLE_

%macro gotcall 1
	push ebx
	call    .get_GOT 
.get_GOT: 
	pop ebx 
	add ebx,_GLOBAL_OFFSET_TABLE_+$$-.get_GOT wrt ..gotpc 
	lea ecx, [ebx + %1 WRT ..got]
	pop ebx

	call [ecx]
%endmacro

__darwin_setjmp:

	gotcall __darwin_savesigs
	jmp no_sig_save

__darwin_sigsetjmp:

	test dword [esp+8], 0
	je .no_sig_save_z

	gotcall __darwin_savesigs
	jmp no_sig_save

.no_sig_save_z:
	mov ecx, [esp+4]
	mov dword [ecx+44], 0 ; zero out signal stack flags

no_sig_save:

	mov ecx, [esp+4]
	mov [ecx], edi
	mov [ecx+4], esi
	mov [ecx+8], ebx
	mov [ecx+12], ebp
	mov eax, [esp]
	mov [ecx+16], eax
	lea eax, [esp+4]
	mov [ecx+20], eax

	stmxcsr [ecx+24]
	fnstcw [ecx+28]

	xor eax, eax
	ret

__darwin_longjmp:
__darwin_siglongjmp:
	gotcall __darwin_restoresigs

no_sig_restore:

	mov eax, 1
	test dword [esp+8], 0
	cmovne eax, [esp+8]

	mov ecx, [esp+4]
	mov edi, [ecx]
	mov esi, [ecx+4]
	mov ebx, [ecx+8]
	mov ebp, [ecx+12]
	mov esp, [ecx+20]

	fninit
	ldmxcsr [ecx+24]
	fldcw [ecx+28]
	cld

	jmp [ecx+16]

%else

%error "Unsupported platform"

%endif
