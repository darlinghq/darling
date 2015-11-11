// 
// This file is part of Darling.
// 
// Copyright (C) 2012 Lubos Dolezel
// 
// Darling is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Darling is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Darling.  If not, see <http://www.gnu.org/licenses/>.

.text
.globl trampoline_start
.globl trampoline_end
.globl reg_saveall
.globl reg_restoreall
.hidden trampoline_start
.hidden trampoline_end
.hidden reg_saveall
.hidden reg_restoreall

#ifdef __x86_64__

reg_saveall: // 192 bytes on stack
	popq %r10 // remove retaddr from stack
	pushq %rax
	pushq %rbx
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %r8
	pushq %r9
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
	subq $128, %rsp // 8*16
	movdqu %xmm0, (%rsp)
	movdqu %xmm1, 16(%rsp)
	movdqu %xmm2, 32(%rsp)
	movdqu %xmm3, 48(%rsp)
	movdqu %xmm4, 64(%rsp)
	movdqu %xmm5, 80(%rsp)
	movdqu %xmm6, 96(%rsp)
	movdqu %xmm7, 112(%rsp)
	jmpq *%r10

reg_restoreall:
	popq %r10 // remove retaddr from stack
	movdqu 112(%rsp), %xmm7
	movdqu 96(%rsp), %xmm6
	movdqu 80(%rsp), %xmm5
	movdqu 64(%rsp), %xmm4
	movdqu 48(%rsp), %xmm3
	movdqu 32(%rsp), %xmm2
	movdqu 16(%rsp), %xmm1
	movdqu (%rsp), %xmm0
	addq $128, %rsp
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %r9
	popq %r8
	popq %rcx
	popq %rdx
	popq %rsi
	popq %rdi
	popq %rbx
	popq %rax
	jmpq *%r10

// This is duplicated for every mapped function
trampoline_start:
	movq 0xb0b1b2b3b4b5b6, %r10
	callq *%r10 // call reg_saveall

	movq 0x123456, %rdi // index in entry table
	movq %rsp, %rsi
	movq 0xaabbccddeeff, %rcx // print function (saves orig ret address)

	//sub rsp, 8 ; align stack to 16
	callq *%rcx
	//add rsp, 8

	movq %rax, %r11 // save the right addr

	movq 0xc0c1c2c3c4c5c6, %r10 // call reg_restoreall
	callq *%r10

	// rewrite return address
	leaq 1f(%rip), %r10
	movq %r10, (%rsp) // second print function
	jmpq *%r11 // jump to the real function

.align 2
1:
	
	movq 0xb0b1b2b3b4b5b6, %r10
	callq *%r10 // call reg_saveall

	movq 0x123456, %rdi
	movq %rsp, %rsi
	movq 0xa0a1a2a3a4a5a6, %rcx // retval print function (returns orig ret address)
	
	// sub rsp, 8 ; align stack to 16
	callq  *%rcx
	// add rsp, 8
	
	movq %rax, %r11

	movq 0xc0c1c2c3c4c5c6, %r10 // call reg_restoreall
	callq *%r10
	jmpq *%r11 // now we "return" to the original function
trampoline_end:
	nop

#endif

.section ".note.GNU-stack","",@progbits

