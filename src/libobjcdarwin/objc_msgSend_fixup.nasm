global objc_msgSend_fixup

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

objc_msgSend_fixup:
	; Procedure:
	; 1) convert Apple selector to GNU
	; 2) run objc_msg_lookup
	; 3) save the method address into the second argument
	; 4) save the right selector into the second argument
	; 5) jump to the pointer returned by objc_msg_lookup
	
	call SaveRegisters
	; move the second argument into the first argument
	mov rdi, rsi
	add rdi, 8 ; selector name is the second pointer in struct
	call sel_get_any_uid WRT ..plt
	; rax now has the GNU selector
	mov rsi, [rsp+8] ; put the second argument back to rsi
	mov [rsi+8], rax ; store the GNU selector
	add rsi, 8 ; pass the selector itself
	; restore the first argument
	mov rdi, [rsp]
	call objc_msg_lookup WRT ..plt
	
	call RestoreRegisters
	; can we do it like this? don't we still need a proxy?
	mov [rsi], rax ; fix the pointer to make the call direct next time
	jmp rax

%elifidn __OUTPUT_FORMAT__, elf

BITS 32
section text

objc_msgSend_fixup:
	; swap the first two arguments
	mov ecx, [esp+4]
	xchg [esp+8], ecx
	add ecx, 4 ; move to the second struct element
	mov [esp+4], ecx
	
	call sel_get_any_uid WRT ..plt
	
	; eax now has the GNU selector
	; swap the arguments back
	
	mov ecx, [esp+4]
	mov [ecx], eax ; fix the selector in the original struct
	
	; swap the arguments back
	mov ecx, [esp+4]
	xchg [esp+8], ecx
	mov [esp+4], ecx
	call objc_msg_lookup WRT ..plt
	
	jmp eax

%else

%error "Unsupported platform"

%endif
