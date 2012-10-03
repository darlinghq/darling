%ifidn __OUTPUT_FORMAT__, elf64

BITS 64
section text

global objcdarwin_SaveRegisters
global objcdarwin_RestoreRegisters

objcdarwin_SaveRegisters:
    pop r11

    sub rsp, 128 ; 8*16
    movdqu [rsp], xmm0
    movdqu [rsp+16], xmm1
    movdqu [rsp+32], xmm2
    movdqu [rsp+48], xmm3
    movdqu [rsp+64], xmm4
    movdqu [rsp+80], xmm5
    movdqu [rsp+96], xmm6
    movdqu [rsp+112], xmm7

    push r9
    push r8
    push rcx
    push rdx
    push rsi
    push rdi

    jmp r11

objcdarwin_RestoreRegisters:
    pop r11
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop r8
    pop r9
    movdqu xmm7, [rsp+112]
    movdqu xmm6, [rsp+96]
    movdqu xmm5, [rsp+80]
    movdqu xmm4, [rsp+64]
    movdqu xmm3, [rsp+48]
    movdqu xmm2, [rsp+32]
    movdqu xmm1, [rsp+16]
    movdqu xmm0, [rsp]
    add rsp, 128

    jmp r11
%endif

