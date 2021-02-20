/*
 * Copyright (c) 2006-2008 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
 

#if __ppc__ && __PIC__
//
// Force stub section next to __text section to minimize chance that
// a bl to a stub will be out of range.
//
	.text
 	.section __TEXT,__picsymbolstub1,symbol_stubs,pure_instructions,32
#endif

////////////////////////////////////////////////////////////////////
//
// The dyld_stub_binding_helper adds the mach_header parameter
// and then jumps into dyld via a pointer in __dyld section
//
////////////////////////////////////////////////////////////////////
	.text
	.private_extern dyld_stub_binding_helper
	.align 2
dyld_stub_binding_helper:

#if __ppc__
 // for ppc the mach_header parameter is place in r12
 // and the lazy_pointer is already in r11
 #if __PIC__
	mflr	r0
	bcl     20,31,L1
L1:	mflr    r12
	mtlr	r0
	mr      r0,r12
	addis	r12,r12,ha16(Ldyld_content_lazy_binder-L1)
	lwz     r12,lo16(Ldyld_content_lazy_binder-L1)(r12)
	mtctr	r12
	mr      r12,r0
	addis	r12,r12,ha16(dyld__mach_header-L1)
	lwz     r12,lo16(dyld__mach_header-L1)(r12)
	bctr
 #else
	lis		r12,ha16(Ldyld_content_lazy_binder)
	lwz		r12,lo16(Ldyld_content_lazy_binder)(r12)
	mtctr	r12
	lis		r12,ha16(___dso_handle)
	la		r12,lo16(___dso_handle)(r12)
	bctr
 #endif
	
#elif __ppc64__
 // for ppc the mach_header parameter is place in r12
 // and the lazy_pointer is already in r11
 // always use PIC code so we can have 4GB zero page
	mflr	r0
	bcl     20,31,L1
L1:	mflr    r12
	mtlr	r0
	mr      r0,r12
	addis	r12,r12,ha16(Ldyld_content_lazy_binder-L1)
	ld      r12,lo16(Ldyld_content_lazy_binder-L1)(r12)
	mtctr	r12
	mr      r12,r0
	addis	r12,r12,ha16(dyld__mach_header-L1)
	ld      r12,lo16(dyld__mach_header-L1)(r12)
	bctr

#elif __i386__
 // for i386 the mach_header parameter is pushed on the stack 
 // and the lazy_pointer is already on the stack
 #if __PIC__
	subl	$8,%esp
	pushl	%eax
	pushl	%ecx
	call	L1
L1:	popl	%eax
	movl	dyld__mach_header-L1(%eax),%ecx
	movl	%ecx,12(%esp)
	movl    Ldyld_content_lazy_binder-L1(%eax),%ecx
	movl	%ecx,8(%esp)
	popl	%ecx
	popl	%eax
	ret		// jumps into dyld with lp and mh on the stack
 #else
	pushl   $___dso_handle
	jmpl    *Ldyld_content_lazy_binder
 #endif

#elif __x86_64__
 // for x86_64 the mach_header parameter is pushed on the stack
 // and the lazy_pointer was in r11 and is pushed on the stack
	pushq	%r11
	leaq	___dso_handle(%rip), %r11
	pushq	%r11
	jmp		*Ldyld_content_lazy_binder(%rip)

#elif __arm__
	str	ip, [sp, #-4]!  // push address of lazy pointer
	ldr	ip, Ldyld__mach_header_pointer
#if __PIC__
Ldyld__mach_header_pointer_base:
	ldr	ip, [pc, ip]
#endif
	str	ip, [sp, #-4]!  // push address of mach header
	ldr	ip, Ldyld_lazy_symbol_binding_entry_point
#if __PIC__
Ldyld_lazy_symbol_binding_entry_point_base:
	ldr	pc, [pc, ip]    // jump to dyld_lazy_symbol_binding_entry_point
#else
	ldr	pc, [ip]
#endif

Ldyld__mach_header_pointer:
#if __PIC__
	.long	dyld__mach_header - (Ldyld__mach_header_pointer_base + 8)
#else
	.long	___dso_handle
#endif
Ldyld_lazy_symbol_binding_entry_point:
#if __PIC__
	.long	Ldyld_content_lazy_binder - (Ldyld_lazy_symbol_binding_entry_point_base + 8)
#else
	.long	Ldyld_content_lazy_binder
#endif
#endif



////////////////////////////////////////////////////////////////////
//
// cfm_stub_binding_helper
//
// only needed by ppc dylibs which support CFM clients
//
////////////////////////////////////////////////////////////////////
#if __ppc__ && CFM_GLUE
	.text
	.align 2
	.private_extern     cfm_stub_binding_helper
cfm_stub_binding_helper:
	mr      r11, r12 ; The TVector address is the binding pointer address.
	b       dyld_stub_binding_helper  ; Let the normal code handle the rest.
#endif



////////////////////////////////////////////////////////////////////
//
// __dyld_func_lookup(const char*, void**)
//
// jumps into dyld via a pointer in __dyld section
//
////////////////////////////////////////////////////////////////////
	.text
	.private_extern __dyld_func_lookup
	.align 2
__dyld_func_lookup:

#if __ppc__
 #if __PIC__
	mflr	r0
	bcl     20,31,L2
L2:	mflr    r11
	mtlr	r0
	addis	r11,r11,ha16(Ldyld_content_func_lookup-L2)
	lwz     r11,lo16(Ldyld_content_func_lookup-L2)(r11)
	mtctr	r11
	bctr
 #else
	lis		r11,ha16(Ldyld_content_func_lookup)
	lwz		r11,lo16(Ldyld_content_func_lookup)(r11)
	mtctr	r11
	bctr
 #endif

#elif __ppc64__
	mflr	r0
	bcl     20,31,L2
L2:	mflr    r11
	mtlr	r0
	addis	r11,r11,ha16(Ldyld_content_func_lookup-L2)
	ld      r11,lo16(Ldyld_content_func_lookup-L2)(r11)
	mtctr	r11
	bctr
	
#elif __i386__
 #if __PIC__
	call	L2
L2:	popl	%eax
	movl    Ldyld_content_func_lookup-L2(%eax),%eax
	jmpl    *%eax
 #else
	jmpl	*Ldyld_content_func_lookup
 #endif

#elif __x86_64__
	jmp		*Ldyld_content_func_lookup(%rip)

#elif __arm__
	ldr		ip, L__dyld_func_lookup_pointer
#if __PIC__
L__dyld_func_lookup_pointer_base:
	ldr		pc, [pc, ip]
#else
	ldr		pc, [ip]
#endif
L__dyld_func_lookup_pointer:
#if __PIC__
	.long	Ldyld_content_func_lookup - (L__dyld_func_lookup_pointer_base + 8)
#else
	.long	Ldyld_content_func_lookup
#endif
#endif



#if __LP64__
	#define		align_pointer	align	3
	#define		pointer			quad
#else
	#define		align_pointer	align	2
	#define		pointer			long
#endif



#if __ppc64__ || ((__i386__ || __ppc__ || __arm__) && __PIC__)
////////////////////////////////////////////////////////////////////
//
// dyld__mach_header
// contains a pointer to the mach_header for this linkage unit
// only needed for some code models
// 
////////////////////////////////////////////////////////////////////
	.data
	.align_pointer
dyld__mach_header:
	.pointer		___dso_handle
#endif // __x86_64__


////////////////////////////////////////////////////////////////////
//
// __dyld section content
//
//  0:	pointer to lazy symbol binder in dyld
//  1:  pointer to dyld_func_lookup implementation in dyld
//
////////////////////////////////////////////////////////////////////
#if __ppc__
	Ldyld_base_addr =	0x8fe00000
#elif __ppc64__
	Ldyld_base_addr =	0x00007fff5fc00000
#elif __i386__
	Ldyld_base_addr =	0x8fe00000
#elif __x86_64__
	Ldyld_base_addr =	0x00007fff5fc00000
#elif __arm__
	Ldyld_base_addr =	0x2fe00000
#elif __arm64__
#else
#error unknown architecture
#endif

#if !__arm64__
	.dyld
	.align_pointer
Ldyld_content_lazy_binder:
	.pointer		Ldyld_base_addr + 0x1000
Ldyld_content_func_lookup:
	.pointer		Ldyld_base_addr + 0x1008
#if CRT && !OLD_LIBSYSTEM_SUPPORT
	.pointer		___dso_handle
	.pointer		_NXArgc
	.pointer		_NXArgv
	.pointer		_environ
	.pointer		___progname
#endif
#endif

// This code has be written to allow dead code stripping
	.subsections_via_symbols

