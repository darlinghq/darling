global __darwin_objc_msgSendSuper
global __darwin_objc_msgSendSuper2 ; requires extracting superclass from Class
global __darwin_objc_msgSendSuper2_stret
extern objc_msg_lookup_super

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

; TODO: needs fixing like above
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
