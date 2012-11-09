global __darwin_objc_msgSend_fixup
global objc_msgSendSuper2_fixup
global objc_msgSendSuper2_stret_fixup
global __darwin_objc_msgSend_fpret_fixup
global __darwin_objc_msgSend_fp2ret_fixup
extern objc_msgSend
extern objc_msgSend_fpret
extern __darwin_objc_msgSendSuper2
extern __darwin_objc_msgSendSuper2_stret

section .note.GNU-stack noalloc noexec nowrite progbits

%ifidn __OUTPUT_FORMAT__, elf64

BITS 64
section text

__darwin_objc_msgSend_fp2ret_fixup:
__darwin_objc_msgSend_fpret_fixup:
	mov rsi, [rsi+8]
	jmp objc_msgSend_fpret WRT ..plt

__darwin_objc_msgSend_fixup:
	mov rsi, [rsi+8]
	jmp objc_msgSend WRT ..plt

objc_msgSendSuper2_fixup:
	mov rsi, [rsi+8]
	jmp __darwin_objc_msgSendSuper2 WRT ..plt

objc_msgSendSuper2_stret_fixup:
	mov rdx, [rdx+8]
	jmp __darwin_objc_msgSendSuper2_stret WRT ..plt

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
