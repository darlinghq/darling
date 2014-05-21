section .note.GNU-stack noalloc noexec nowrite progbits

extern reg_saveall
extern reg_restoreall
extern dyld_stub_binder_fixup ; void* dyld_stub_binder_fixup(void* cache, uintptr_t index)
extern _GLOBAL_OFFSET_TABLE_
global dyld_stub_binder

; Stub identifier is pushed first
; then the code pushes a pointer to cache

%ifidn __OUTPUT_FORMAT__, elf64

BITS 64
section text

dyld_stub_binder:

	call reg_saveall WRT ..plt ; 224 bytes
	mov rdi, [rsp+224] ; cache
	mov rsi, [rsp+224+8] ; offset
	sub rsp, 8 ; maintain 16-byte stack alignment

	call dyld_stub_binder_fixup WRT ..plt
	
	add rsp, 8 ; maintain 16-byte stack alignment

	mov [rsp+224], rax
	call reg_restoreall WRT ..plt
	
	mov r11, [rsp]
	add rsp, 16 ; remove arguments to dyld_stub_binder
	jmp r11

%elifidn __OUTPUT_FORMAT__, elf

BITS 32
section text

dyld_stub_binder:

	push ebx
	
	call .get_GOT
.get_GOT:
	pop ebx
	add ebx, _GLOBAL_OFFSET_TABLE_+$$-.get_GOT wrt ..gotpc

	push eax
	call reg_saveall WRT ..plt  ; 32 bytes
	mov eax, [esp+32+4] ; cache
	mov ecx, [esp+32+4+4] ; offset
	push ecx
	push eax

	call dyld_stub_binder_fixup WRT ..plt
	add esp, 8

	mov [esp+32+4], eax ; save the real addr somewhere
	call reg_restoreall WRT ..plt

	mov eax, [esp+4] ; restore the real addr
	
	pop ebx
	add esp, 8 ; remove arguments to dyld_stub_binder THIS SHOULDN'T WORK!
	
	jmp eax

%else

%error "Unsupported platform"

%endif


