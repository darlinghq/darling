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

#ifdef __i386__

// Argument is in %eax
darling_tls_get_addr:

	movl 4(%eax), %ecx
	movl %fs:0x0(,%ecx,4),%ecx

	testl %ecx, %ecx
	jz .Lslow_path
	
	addl 8(%eax), %ecx // add the offset

	ret

.Lslow_path:
	pushl %ebx
	calll 1f
1:
	popl %ebx
2:
	addl $_GLOBAL_OFFSET_TABLE_+(2b-1b), %ebx

	pushl 4(%eax)     // pass the pthread_key
	call darling_tls_allocate@PLT
	addl $4, %esp      // remove the argument
	
	addl 8(%eax), %ecx // add the offset

	popl %ebx
	ret

#endif

.section ".note.GNU-stack","",@progbits

