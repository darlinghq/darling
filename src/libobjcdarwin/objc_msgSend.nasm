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

extern  _GLOBAL_OFFSET_TABLE_

__darwin_objc_msgSend_noarg:
	push ebx
	call    .get_GOT 
.get_GOT: 
	pop ebx 
	add ebx,_GLOBAL_OFFSET_TABLE_+$$-.get_GOT wrt ..gotpc 
	lea eax, [ebx + objc_msgSend WRT ..got]
	pop ebx

	jmp eax

%else

%error "Unsupported platform"

%endif
