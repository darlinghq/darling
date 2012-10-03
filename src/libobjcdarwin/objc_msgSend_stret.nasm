global __darwin_objc_msgSend_stret

extern objcdarwin_class_lookup
extern objc_msg_lookup
extern sel_get_any_uid
extern objcdarwin_SaveRegisters
extern objcdarwin_RestoreRegisters

%ifidn __OUTPUT_FORMAT__, elf64

BITS 64
section text

; Compared to ordinary msgSend, arguments are shifted by one - first arg is the stret
__darwin_objc_msgSend_stret:
	; Procedure:
	; 1) get the converted GNU class from an Apple class
	; 2) convert Apple selector to GNU
	; 3) run objc_msg_lookup
	; 4) jump to the pointer returned by objc_msg_lookup
	
	call objcdarwin_SaveRegisters WRT ..plt
	mov rdi, rsi
	call objcdarwin_class_lookup WRT ..plt
	mov [rsp+8], rax ; save the converted value
	
	; move the second argument into the first argument
	mov rdi, [rsp+16]
	call sel_get_any_uid WRT ..plt
	; rax now has the GNU selector
	; move rax to the second argument
	mov rsi, rax
	mov [rsp+16], rax
	; restore the first argument
	mov rdi, [rsp+8]
	call objc_msg_lookup WRT ..plt
	
	call objcdarwin_RestoreRegisters WRT ..plt
	jmp rax

%elifidn __OUTPUT_FORMAT__, elf

BITS 32
section text

__darwin_objc_msgSend_stret:
	
	mov ecx, [esp+8]
	push ecx ; arg for func call
	
	call objcdarwin_class_lookup ;WRT ..plt
	
	add esp, 4 ; remove argument
	mov [esp+8], eax ; change the class id
	
	mov ecx, [esp+12] ; second argument
	push ecx
	
	call sel_get_any_uid ;WRT ..plt
	
	add esp, 4
	mov [esp+12], eax
	
	push eax ; reuse the sel_get_any_uid retval
	mov eax, [esp+12]
	push eax ; class id
	
	call objc_msg_lookup ;WRT ..plt
	add esp, 8
	
	jmp eax

%else

%error "Unsupported platform"

%endif
