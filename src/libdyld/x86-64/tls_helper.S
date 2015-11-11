//
// This file is part of Darling.
// 
// Copyright (C) 2013 Lubos Dolezel
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
//


.text
.globl darling_tls_get_addr
.hidden darling_tls_get_addr

#ifdef __x86_64__

darling_tls_get_addr:
	movq 8(%rdi), %rax
	
	// %gs on x86-64, %fs on i386
	movq %gs:0x0(,%rax,8),%rax
	
	testq %rax, %rax
	jz .Lslow_path

	addq 16(%rdi), %rax // add the variable offset
	ret

.Lslow_path:
	enter $592, $0
	
	// Do a register full-save, because the caller assumes that no registers
	// except for rax/rdi will be modified
	movq %rdi, -8(%rbp)
	movq %rsi, -16(%rbp)
	movq %rdx, -24(%rbp)
	movq %rcx, -32(%rbp)
	movq %r8, -40(%rbp)
	movq %r9, -48(%rbp)
	movq %r10,-56(%rbp)
	movq %r11,-64(%rbp)
	fxsave -592(%rbp)
	
	movq 8(%rdi), %rdi // pass the pthread_key
	call darling_tls_allocate@PLT
	
	fxrstor  -592(%rbp)
	movq -64(%rbp), %r11
	movq -56(%rbp), %r10
	movq -48(%rbp), %r9
	movq -40(%rbp), %r8
	movq -32(%rbp), %rcx
	movq -24(%rbp), %rdx
	movq -16(%rbp), %rsi
	movq -8(%rbp), %rdi
	
	addq 16(%rdi), %rax // add the variable offset
	leave
	ret

#endif

.section ".note.GNU-stack","",@progbits

