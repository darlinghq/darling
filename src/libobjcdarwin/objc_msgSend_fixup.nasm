global __darwin_objc_msgSend_fixup
extern objc_msgSend

section .note.GNU-stack noalloc noexec nowrite progbits

%ifidn __OUTPUT_FORMAT__, elf64

BITS 64
section text

; TODO: msgSendSuper2_fixup...

__darwin_objc_msgSend_fixup:
	mov rsi, [rsi+8]
	jmp objc_msgSend WRT ..plt

%elifidn __OUTPUT_FORMAT__, elf

BITS 32
section text

; Is it even used with the old runtime?
__darwin_objc_msgSend_fixup:
	mov eax, [esp+8]
	add eax, 4
	mov eax, [eax]
	mov [esp+8], eax
	jmp objc_msgSend

%else

%error "Unsupported platform"

%endif
