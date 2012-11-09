global __darwin_objc_msgSendSuper
global __darwin_objc_msgSendSuper2 ; requires extracting superclass from Class
global __darwin_objc_msgSendSuper2_stret
extern objc_msg_lookup_super

section .note.GNU-stack noalloc noexec nowrite progbits

%ifidn __OUTPUT_FORMAT__, elf64

BITS 64
section text

%macro SaveRegisters 0
	push rdi
	push rsi
	push rdx
	push rcx
	push r8
	push r9
%endmacro

%macro RestoreRegisters 0
	pop r9
	pop r8
	pop rcx
	pop rdx
	pop rsi
	pop rdi
%endmacro

__darwin_objc_msgSendSuper2:
	SaveRegisters
	
	; Make a copy of the struct on stack
	mov rax, [rdi]
	mov [rsp-16], rax
	mov rax, [rdi+8]
	mov rax, [rax+8] ; load superclass
	mov [rsp-8], rax

	sub rsp, 16
	mov rdi, rsp
	
	call objc_msg_lookup_super WRT ..plt
	
	add rsp, 16
	
	RestoreRegisters
	
	mov rdi, [rdi]
	jmp rax
	
__darwin_objc_msgSendSuper:
	SaveRegisters
	
	call objc_msg_lookup_super WRT ..plt
	
	RestoreRegisters
	
	mov rdi, [rdi]
	jmp rax

__darwin_objc_msgSendSuper2_stret:
	SaveRegisters
	
	; Make a copy of the struct on stack
	mov rax, [rsi]
	mov [rsp-16], rax
	mov rax, [rsi+8]
	mov rax, [rax+8] ; load superclass
	mov [rsp-8], rax

	sub rsp, 16
	mov rdi, rsp
	mov rsi, rdx
	
	call objc_msg_lookup_super WRT ..plt
	
	add rsp, 16
	
	RestoreRegisters
	
	mov rsi, [rsi]
	jmp rax
__darwin_objc_msgSendSuper_stret:
	SaveRegisters
	
	mov rdi, rsi
	mov rsi, rdx
	
	call objc_msg_lookup_super WRT ..plt
	
	RestoreRegisters
	
	mov rsi, [rsi]
	jmp rax

%elifidn __OUTPUT_FORMAT__, elf

BITS 32
section text

extern  _GLOBAL_OFFSET_TABLE_

%macro DereferenceArgument  1
	mov ecx, [ebp+(%1*4)+8] ; get objc_super*
	mov ecx, [ecx] ; get the first member's value
	mov [ebp+(%1*4)+8], ecx ; fix the first argument
%endmacro

; NOT USED
__darwin_objc_msgSendSuper2:
	mov eax, [esp+4] ; get objc_super*
	; make a copy on the stack
	mov ecx, [eax] ; 1st elem
	mov [esp-8], ecx
	mov ecx, [eax+4] ; 2nd elem
	mov ecx, [ecx+4] ; get superclass from objc_class
	mov [esp-4], ecx

	sub esp, 8
	mov eax, [esp+16]; SEL (2nd argument)
	push eax
	lea eax, [esp+4]; fixed objc_super (1st argument)
	push eax

	call objc_msg_lookup_super WRT ..plt

	add esp, 16 ; remove args & struct from the stack

	DereferenceArgument 0

	jmp eax
	
__darwin_objc_msgSendSuper:

	push ebp
	mov ebp, esp
	push ebx
	call .get_GOT
.get_GOT:
	pop ebx
	add ebx, _GLOBAL_OFFSET_TABLE_+$$-.get_GOT wrt ..gotpc

	mov eax, [ebp+12]
	push eax
	mov eax, [ebp+8]
	push eax
	call objc_msg_lookup_super WRT ..plt
	add esp, 8

	DereferenceArgument 0

	mov ebx, [ebp-4]
	mov esp, ebp
	pop ebp

	jmp eax

__darwin_objc_msgSendSuper2_stret:
	push ebp
	mov ebp, esp
	push ebx
	call .get_GOT
.get_GOT:
	pop ebx
	add ebx, _GLOBAL_OFFSET_TABLE_+$$-.get_GOT wrt ..gotpc

	mov eax, [ebp+16]
	push eax
	mov eax, [ebp+12]
	push eax
	
	call objc_msg_lookup_super WRT ..plt
	sub esp, 8

	DereferenceArgument 1

	mov ebx, [ebp-4]
	mov esp, ebp
	pop ebp
	
	jmp eax

%else

%error "Unsupported platform"

%endif
