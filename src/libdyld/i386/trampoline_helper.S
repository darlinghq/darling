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

.globl trampoline_start
.globl trampoline_end
.globl reg_saveall
.globl reg_restoreall
.hidden trampoline_start
.hidden trampoline_end
.hidden reg_saveall
.hidden reg_restoreall

#ifdef __i386__

#define SavedRegLen 32

reg_saveall: // 24 bytes on stack
	popl %eax

	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esi
	pushl %edi
	subl 8, %esp
	fstpl (%esp)

	jmpl *%eax

reg_restoreall:
	popl %ebx
	movl -4+SavedRegLen(%esp), %eax
	mov %ebx, -4+SavedRegLen(%esp)

	fldl (%esp)
	addl $8, %esp
	popl %edi
	popl %esi
	popl %edx
	popl %ecx
	popl %ebx

	ret

trampoline_start:
	pushl %eax
	movl 0xa0a1a2a3, %eax // call reg_saveall
	calll *%eax
	
	pushl %esp
	pushl 0x12345678 // index in entry table
	movl 0xb0b1b2b3, %eax
	calll *%eax // call print function
	addl $8, %esp

	movl %eax, -8(%esp)
	movl 0xc0c1c2c3, %eax // call reg_restoreall
	calll *%eax

	// we cannot use IP-relative lea on 32bit x86
	calll .Lget_eip
.Lget_eip:
	popl %eax
	addl $0xD, %eax // this gives us the address of after_jump
	movl %eax, (%esp)
	movl -8-SavedRegLen(%esp), %eax

	jmpl *%eax // call the real function

.align 2
after_jump:
	pushl %eax
	movl 0xa0a1a2a3, %eax
	calll *%eax // call reg_saveall
	pushl %esp
	pushl 0x12345678 // index in entry table
	movl 0xb9b9b9b9, %eax // print function 2
	calll *%eax
	addl $8, %esp

	movl %eax, -8(%esp) // real return address
	movl 0xc0c1c2c3, %eax
	calll *%eax // call reg_restoreall

	movl -8-SavedRegLen, %ecx
	jmpl *%ecx // return to the original caller
trampoline_end:
	nop

#endif

.section ".note.GNU-stack","",@progbits
