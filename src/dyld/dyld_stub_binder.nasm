extern reg_saveall
extern reg_restoreall
extern dyld_stub_binder_fixup ; void* dyld_stub_binder_fixup(void* cache, uintptr_t index)
global dyld_stub_binder

; Stub identifier is pushed first
; then the code pushes a pointer to cache

%ifidn __OUTPUT_FORMAT__, elf64

BITS 64
section text

dyld_stub_binder:

	call reg_saveall ; 224 bytes
	mov rdi, [rsp+224] ; cache
	mov rsi, [rsp+224+8] ; offset

	call dyld_stub_binder_fixup

	mov r11, rax
	call reg_restoreall
	
	add rsp, 16 ; remove arguments to dyld_stub_binder
	jmp r11

%elifidn __OUTPUT_FORMAT__, elf

BITS 32
section text

dyld_stub_binder:

	call reg_saveall ; 32 bytes
	mov eax, [esp+32] ; cache
	mov ecx, [esp+32+4] ; offset
	push ecx
	push eax

	call dyld_stub_binder_fixup
	add esp, 8

	mov [esp-4], eax ; save the real addr somewhere
	call reg_restoreall

	mov eax, [esp-32-4] ; restore the real addr
	add esp, 8 ; remove arguments to dyld_stub_binder
	jmp eax

%else

%error "Unsupported platform"

%endif


