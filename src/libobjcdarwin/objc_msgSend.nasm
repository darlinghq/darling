global __darwin_objc_msgSend_noarg

extern objc_msgSend

section .note.GNU-stack noalloc noexec nowrite progbits

%ifidn __OUTPUT_FORMAT__, elf64

BITS 64
section text

__darwin_objc_msgSend_noarg:
	jmp objc_msgSend WRT ..plt

%elifidn __OUTPUT_FORMAT__, elf

BITS 32
section text

__darwin_objc_msgSend_noarg:
	jmp objc_msgSend

%else

%error "Unsupported platform"

%endif
