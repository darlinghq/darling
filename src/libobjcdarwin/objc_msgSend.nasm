global objc_msgSend

extern objc_msg_lookup
extern sel_get_any_uid

%ifidn __OUTPUT_FORMAT__, elf64

BITS 64
section text

SaveRegisters:
	pop r11
	push rdi
	push rsi
	push rdx
	push rcx
	push r8
	push r9
	; save xmms?
	jmp r11

RestoreRegisters:
	pop r11
	pop r9
	pop r8
	pop rcx
	pop rdx
	pop rsi
	pop rdi
	jmp r11

objc_msgSend:
	; Procedure:
	; 1) convert Apple selector to GNU
	; 2) run objc_msg_lookup
	; 3) jump to the pointer returned by objc_msg_lookup
	
	call SaveRegisters
	; move the second argument into the first argument
	mov rdi, rsi
	call sel_get_any_uid WRT ..plt
	; rax now has the GNU selector
	; move rax to the second argument
	mov rsi, rax
	; restore the first argument
	mov rdi, [rsp]
	call objc_msg_lookup WRT ..plt
	
	call RestoreRegisters
	jmp rax

%elifidn __OUTPUT_FORMAT__, elf

BITS 32
section text

objc_msgSend:
	; swap the first two arguments
	mov ecx, [esp+4]
	xchg [esp+8], ecx
	mov [esp+4], ecx
	
	call sel_get_any_uid WRT ..plt
	
	; eax now has the GNU selector
	; swap the arguments back
	
	mov ecx, [esp+8]
	mov [esp+4], ecx
	xchg [esp+8], eax
	call objc_msg_lookup WRT ..plt
	
	jmp eax

%else

%error "Unsupported platform"

%endif
