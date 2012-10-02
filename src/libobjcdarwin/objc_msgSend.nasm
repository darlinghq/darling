global __darwin_objc_msgSend

extern objcdarwin_class_lookup
extern objc_msg_lookup
extern sel_get_any_uid

%ifidn __OUTPUT_FORMAT__, elf64

BITS 64
section text

SaveRegisters:
	pop r11
	
	push r9
	push r8
	push rcx
	push rdx
	push rsi
	push rdi
	
	; save xmms?
	jmp r11

RestoreRegisters:
	pop r11
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop r8
	pop r9
	jmp r11

__darwin_objc_msgSend:
	; Procedure:
	; 1) get the converted GNU class from an Apple class
	; 2) convert Apple selector to GNU
	; 3) run objc_msg_lookup
	; 4) jump to the pointer returned by objc_msg_lookup
	
	call SaveRegisters
	call objcdarwin_class_lookup WRT ..plt
	mov [rsp], rax ; save the converted value
	
	; move the second argument into the first argument
	mov rdi, [rsp+8]
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;; TODO!!!

__darwin_objc_msgSend:
	;enter 8
	
	mov ecx, [esp+4]
	push ecx ; arg for func call
	
	call objcdarwin_class_lookup ;WRT ..plt
	
	add esp, 4 ; remove argument
	mov [esp+4], eax ; change the class id
	
	mov ecx, [esp+8] ; second argument
	push ecx
	
	call sel_get_any_uid ;WRT ..plt
	
	add esp, 4
	mov [esp+8], eax
	
	push eax ; reuse the sel_get_any_uid retval
	mov eax, [esp+8]
	push eax ; class id
	
	call objc_msg_lookup ;WRT ..plt
	add esp, 8
	
	jmp eax

%else

%error "Unsupported platform"

%endif
