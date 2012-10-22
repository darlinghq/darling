global __darwin_objc_msgSendSuper
global __darwin_objc_msgSendSuper2 ; requires extracting superclass from Class
global __darwin_objc_msgSendSuper2_stret
extern objc_msg_lookup_super

%ifidn __OUTPUT_FORMAT__, elf64

BITS 64
section text

__darwin_objc_msgSendSuper2:
	mov rdi, [rdi+8]
	
__darwin_objc_msgSendSuper:
	push rdi
	push rsi
	push rdx
	push rcx
	push r8
	push r9
	
	call objc_msg_lookup_super WRT ..plt
	
	pop r9
	pop r8
	pop rcx
	pop rdx
	pop rsi
	pop rdi
	
	jmp rax

__darwin_objc_msgSendSuper2_stret:
	mov rdi, [rdi+8]
__darwin_objc_msgSendSuper_stret:
	push rdi
	push rsi
	push rdx
	push rcx
	push r8
	push r9
	
	mov rdi, rsi
	mov rsi, rdx
	
	call objc_msg_lookup_super WRT ..plt
	
	pop r9
	pop r8
	pop rcx
	pop rdx
	pop rsi
	pop rdi
	
	jmp rax

%elifidn __OUTPUT_FORMAT__, elf

BITS 32
section text

__darwin_objc_msgSendSuper2:
	mov eax, [esp+4]
	mov eax, [eax+4]
	mov [esp+4], eax
	
__darwin_objc_msgSendSuper:

	mov eax, [esp+8]
	push eax
	mov eax, [esp+8]
	push eax
	call objc_msg_lookup_super
	add esp, 8
	jmp eax

__darwin_objc_msgSendSuper2_stret:
	mov eax, [esp+12]
	push eax
	mov eax, [esp+20]
	push eax
	
	call objc_msg_lookup_super
	sub esp, 8
	
	jmp eax

%else

%error "Unsupported platform"

%endif
