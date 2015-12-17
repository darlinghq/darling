/*
	This file "aesxts.s" provides x86_64 / i386 optimization of the following functions

	0. xts_mult_x_on_xmm7 : a code macro that is used throughout all other functions
	1. void xts_mult_x(uint8_t *I);
	2. int tweak_crypt(const uint8_t *P, uint8_t *C, uint8_t *T, aesedp_encrypt_ctx *ctx);
	3. int tweak_crypt_group(const uint8_t *P, uint8_t *C, uint8_t *T, aesedp_encrypt_ctx *ctx, uint32_t lim); 
	4. int tweak_uncrypt(const uint8_t *C, uint8_t *P, uint8_t *T, aesedp_decrypt_ctx *ctx);
	5. int tweak_uncrypt_group(const uint8_t *C, uint8_t *P, uint8_t *T, aesedp_decrypt_ctx *ctx, uint32_t lim); 

	This file should be compiled together with xtsClearC.c

	functions 1,2,4 are supposed to replace the C functions in xtsClearC.c for x86_64/i386 architectures
	functions 3,5 are only given here, no C code is available, they are called in xts_encrypt/xts_decrypt (xtsClearC.c)
	  - we can possibly add C code for functions 3 and 5 for future porting to other architectures

	cclee 4-29-10

*/

#if defined __i386__ || defined __x86_64__
#include <System/i386/cpu_capabilities.h>
#define	CRYPT_OK	0		// can not include "crypt.h" in which CRYPT_OK is from enum  

/*
	The following macro is used throughout the functions in this file.
	It is the core function within the function xts_mult_x defined in (xtsClearC.c)

	upon entry, %xmm7 = the input tweak (128-bit), 
	on return, %xmm7 = the updated tweak (128-bit)
	the macro uses %xmm1/%xmm2/%ecx in the computation
	the operation can be described as follows :
	0. let x = %xmm7; 					// 128-bit little-endian input 
	1. x = rotate_left(x,1);			// rotate left by 1 -bit
	2. if (x&1) x ^= 0x0000...0086;		// if least significant bit = 1, least significant byte ^= 0x86;
	3. return x; 

	It's a pity that SSE does not support shifting of the whole 128-bit xmm registers.
	The workaround is 
		1. using parallel dual quad (8-byte) shifting, 1 for the 2 bottom 63-bits, 1 for the 2 leading bits
		2. manipulating the shifted quad words to form the 128-bit shifted result.

	Input : %xmm7
	Output : %xmm7
	Used : %xmm1/%xmm2/%ecx

	The macro is good for both x86_64 and i386. 

*/	

	.macro		xts_mult_x_on_xmm7			// input : x = %xmm7, MS = most significant, LS = least significant
	movaps		%xmm7, %xmm1				// %xmm1 = a copy of x 
	movaps		%xmm7, %xmm2				// %xmm2 = a copy of x
	psllq		$$1, %xmm7					// 1-bit left shift of 2 quad words (x1<<1, x0<<1), zero-filled 
	psrlq		$$63, %xmm1					// 2 leading bits, each in the least significant bit of a quad word 
	psrad		$$31, %xmm2					// the MS 32-bit will be either 0 or -1, depending on the MS bit of x
	pshufd		$$0xc6, %xmm1, %xmm1		// switch the positions of the 2 leading bits
	pshufd		$$0x03, %xmm2, %xmm2		// the LS 32-bit will be either 0 or -1, depending on the MS bit of x
	por			%xmm1, %xmm7				// we finally has %xmm7 = rotate_left(x,1);
	movl		$$0x86, %ecx				// a potential byte to xor the bottom byte
	movd		%ecx, %xmm1					// copy it to %xmm1, the other is 0
	pand		%xmm2, %xmm1				// %xmm1 = 0 or 0x86, depending on the MS bit of x
	pxor		%xmm1, %xmm7				// rotate_left(x,1) ^= 0 or 0x86 depending on the MS bit of x
	.endm


/* 
	function : void xts_mult_x(uint8_t *I);

	1. load (__m128*) (I) into xmm7
	2. macro xts_mult_x_on_xmm7 (i/o @ xmm7, used xmm1/xmm2/ecx) 
	3. save output (%xmm7) to memory pointed by I

	input : 16-byte memory pointed by I
	output : same 16-byte memory pointed by I

	if kernel code, xmm1/xmm2/xmm7 saved and restored
	other used registers : eax/ecx

 */	
	.text
	.align  4,0x90
	.globl	_aesxts_mult_x
_aesxts_mult_x:

#if defined __x86_64__
	#define	I 	%rdi						// 1st argument at %rdi for x86_64
	#define	sp	%rsp
#else
	mov		4(%esp), %eax					// 1st argument at stack, offset 4 for ret_addr for i386
	#define	I	%eax
	#define	sp	%esp
#endif	

	// if KERNEL code, allocate memory and save xmm1/xmm2/xmm7
#ifdef	KERNEL
#if defined __x86_64__
	sub		$0x38, sp						// 8-bytes alignment + 3 * 16 bytes	
#else
	sub		$0x3c, sp						// 12-bytes alignment + 3 * 16 bytes 
#endif
	movaps	%xmm1, (sp)
	movaps	%xmm2, 16(sp)
	movaps	%xmm7, 32(sp)
#endif

	// load, compute, and save
	movups	(I), %xmm7						// load input tweak 128-bit into %xmm7
	xts_mult_x_on_xmm7						// the macro (also used else where) will update %xmm7 as the output
	movups	%xmm7, (I)						// save the xts_mult_x output 

	// if KERNEL code, restore xmm1/xmm2/xmm7 and deallocate stack memory
#ifdef	KERNEL
	movaps	(sp), %xmm1
	movaps	16(sp), %xmm2
	movaps	32(sp), %xmm7
#if defined __x86_64__
	add		$0x38, sp						// 8-bytes alignment + 3 * 16 bytes	
#else
	add		$0x3c, sp						// 12-bytes alignment + 3 * 16 bytes	
#endif
#endif

	ret										// return

	#undef	I
	#undef	sp

/* 
	The following is x86_64/i386 assembly implementation of 

	int tweak_crypt(const uint8_t *P, uint8_t *C, uint8_t *T, aesedp_encrypt_ctx *ctx);

	Its C code implementation is given in xtsClearC.c

	all pointers P/C/T points to a block of 16 bytes. In the following description, P/C/T represent 128-bit data.

	The operation of tweak_crypt

	1. C = P ^ T
	2. err = aes_encryp(C, C, ctx); if (err != CRYPT_OK) return err;
	3. C = C ^ T
	4. xts_mult_x(T)
	5. return CRYPT_OK;

	The following is the assembly implementation flow

	1. save used xmm registers (xmm1/xmm7) if kernel code 
	2. load xmm1 = P, xmm7 = T
	3. xmm1 = C = P ^ T
	4. write xmm1 to C
	5. call aes_encryp(C,C,ctx); note that it will use aesni if available, also xmm will return intact
	6. load xmm1 = C
	7. xmm1 = C = C^T = xmm1 ^ xmm7
	8. write xmm1 to C
	9. update T (in xmm7) via xts_mult_x macro
	a. restore xmm registers (xmm1/xmm7) if kernel code
	b. return CRYPT_OK (in eax) 

	Note: used xmm registers : xmm1/xmm2/xmm7, xmm2 in xts_mult_x macro

*/

	.text
	.align  4,0x90
	.globl	_aesxts_tweak_crypt
_aesxts_tweak_crypt:
#if defined	__i386__

	// push into stack for local use
	push	%ebp
	mov		%esp, %ebp
	push	%ebx
	push	%edi
	push	%esi

	// alllocate stack memory for local use
	sub		$12+16*4, %esp				// 12 (alignment) + 3*16 (xmm save/restore) + 16 (aes_crypt calling arguments)

	// load with called arguments
	mov		8(%ebp), %eax				// P, we need this only briefly, so eax is fine
	mov		12(%ebp), %edi				// C
	mov		16(%ebp), %ebx				// T
	mov		20(%ebp), %esi				// ctx

	#define	P	%eax
	#define	C	%edi
	#define	T	%ebx
	#define	ctx	%esi
	#define	sp	%esp

#else
	// x86_64 calling argument order : rdi/rsi/rdx/rcx/r8

	// push into stack for local use
	push	%rbp
	mov		%rsp, %rbp
	push	%r12
	push	%r13
	push	%r14
	push	%r15

	// alllocate stack memory for local use, if kernel code, need to save/restore xmm registers
#ifdef KERNEL
	sub		$4*16, %rsp					// only need 3*16, add 16 extra so to make save/restore xmm common to i386
#endif

	// load with called arguments, release rdi/rsi/rdx/rcx/r8, as need to call aes_encrypt
	mov		%rsi, %r13
	mov		%rdx, %r14
	mov		%rcx, %r15

	#define	P 	%rdi
	#define	C	%r13
	#define	T	%r14
	#define	ctx	%r15
	#define	sp	%rsp

#endif

	// if kernel, save used xmm registers
#ifdef	KERNEL
	movaps	%xmm1, 16(sp)
	movaps	%xmm2, 32(sp)
	movaps	%xmm7, 48(sp)
#endif

	movups	(P), %xmm1					// P
	movups	(T), %xmm7					// T

	// setup caliing arguments for aes_encrypt
#if defined	__i386__
	mov		C, (%esp)					// C
	mov		C, 4(%esp)					// C
	mov		ctx, 8(%esp)				// ctx
#else
	mov		C, %rdi						// C
	mov		C, %rsi						// C
	mov		ctx, %rdx					// ctx
#endif

	pxor	%xmm7, %xmm1				// C = P ^ T	
	movups	%xmm1, (C)					// save C into memory

	call	_aes_encrypt				// err = aes_encrypt(C,C,ctx);

	cmp		$CRYPT_OK, %eax				// check err == CRYPT_OK
	jne		9f							// if err != CRYPT_OK, exit

	movups	(C), %xmm1					// load xmm1 = C
	pxor	%xmm7, %xmm1				// C ^= T
	movups	%xmm1, (C)					// write C with xmm1, xmm1 is freed now, will be changed in the following macro

	xts_mult_x_on_xmm7					// update T (on xmm7)

	movups	%xmm7, (T)					// write xmm7 to T
9:

	// restore used xmm registers if this is for kernel
#ifdef	KERNEL
	movaps	16(sp), %xmm1
	movaps	32(sp), %xmm2
	movaps	48(sp), %xmm7
#endif

	// free stack memory and restore callee registers
#if defined	__i386__
	add		$12+16*4, %esp				// 12 (alignment) + 3*16 (xmm save/restore) + 16 (aes_crypt calling arguments)
	pop		%esi
	pop		%edi
	pop		%ebx
#else
#ifdef	KERNEL
	add		$4*16, %rsp					// only need 3*16, add 16 extra so make save/restore xmm common to i386
#endif
	pop		%r15
	pop		%r14
	pop		%r13
	pop		%r12
#endif

	// return, eax/rax already has the return val
	leave
	ret

	#undef	P
	#undef	C
	#undef	T
	#undef	ctx
	#undef	sp

/* 
	The following is x86_64/i386 assembly implementation of 

	int tweak_crypt_group(const uint8_t *P, uint8_t *C, uint8_t *T, aesedp_encrypt_ctx *ctx, uint32_t lim);

	TODO : Its C code implementation is YET to be provided in xtsClearC.c (for the benefit of porting to other ISAs)
	This function is grouped version of the above function tweak_crypt(), so xmm registers save/restore only need
	to happen once for all grouped blocks.

	The implementation here probes __cpu_capabilities to detect whether aesni (or hw-aes instruction) is available.
	If aesni is available, the code branch to optimized code that uses aesni.

	The optimized aesni code operates as follows:

	while (more than 4 consecutive blocks available) {

		do xts_mult_x macro 4 times and write the 4 tweaks on stack (16-byte aligned)
	
		perform 4 C = P ^ T;	// T is on 16-byte aligned stack

		perform 4 aes_encrypt (all aes_encrypt instruction interleaved to achieve better throughtput)

		perform 4 C = C ^ T		// T is on 16-byte aligned stack

	}

	The code then falls through to the scalar code, that sequentially performs what tweak_crypt does

	1. C = P ^ T
	2. err = aes_encryp(C, C, ctx); if (err != CRYPT_OK) return err;
	3. C = C ^ T
	4. xts_mult_x(T)

	Note: used xmm registers : 
			xmm0-xmm5, xmm7 if aesni is available
			xmm0-xmm4, xmm7 if aesni is not available.

*/


    .text
	.align  4,0x90
	.globl	_aesxts_tweak_crypt_group
_aesxts_tweak_crypt_group:

#if defined	__i386__

	// push callee-saved registers for local use
	push	%ebp
	mov		%esp, %ebp
	push	%ebx
	push	%edi
	push	%esi

	// allocate stack memory for local use and/or xmm register save for kernel code
	sub		$(12+8*16+16*4), %esp		// 12 (alignment) + 8*16 (xmm) + 4*16 (pre-computed tweaks) aesni
										// 12 (alignment) + 8*16 (xmm) + 4*16 (only 12 used for aes_encrypt) no aesni 
	// transfer calling arguments
	mov		20(%ebp), %eax				// ctx
	mov		12(%ebp), %edi				// C
	mov		16(%ebp), %ebx				// T
	mov		8(%ebp), %esi				// P
	mov		%eax, 8(%esp)				// ctx as the 3rd parameter to aes_decrypt

	#define	P	%esi
	#define	C	%edi
	#define	T	%ebx
	#define	lim	24(%ebp)
	#define	sp	%esp

#else

	// push callee-saved registers for local use
	push	%rbp
	mov		%rsp, %rbp
	push	%rbx
	push	%r12
	push	%r13
	push	%r14
	push	%r15

	// allocate stack memory for local use and/or xmm register save for kernel code
	sub		$(8+8*16+16*5), %rsp		// 8 (alignment) + 8*16 (xmm) + 4*16 (pre-computed tweaks) + 16 (common to i386)

	// rdi/rsi/rdx/rcx/r8
	// transfer calling arguments
	mov		%rdi, %r12	
	mov		%rsi, %r13
	mov		%rdx, %r14
	mov		%rcx, %r15
	mov		%r8,  %rbx

	#define	P 	%r12
	#define	C	%r13
	#define	T	%r14
	#define	ctx	%r15
	#define	lim	%ebx
	#define	sp	%rsp
#endif

#ifdef	KERNEL
	movaps	%xmm0, 0x50(sp)
	movaps	%xmm1, 0x60(sp)
	movaps	%xmm2, 0x70(sp)
	movaps	%xmm3, 0x80(sp)
	movaps	%xmm4, 0x90(sp)
	movaps	%xmm7, 0xa0(sp)
#endif

	// probe __cpu_capabilities to detect aesni
#if defined __x86_64__
    movq    __cpu_capabilities@GOTPCREL(%rip), %rax         // %rax -> __cpu_capabilities
    mov     (%rax), %eax                                    // %eax = __cpu_capabilities
#else		// i386
#if defined KERNEL
    leal    __cpu_capabilities, %eax                        // %eax -> __cpu_capabilities
    mov     (%eax), %eax                                    // %eax = __cpu_capabilities
#else
    mov     _COMM_PAGE_CPU_CAPABILITIES, %eax
#endif
#endif
	test    $(kHasAES), %eax
	je		L_crypt_group_sw								// if aesni not available, jump to sw-based implementation

	// aesni-based implementation

	sub		$4, lim											// pre-decrement lim by 4
	jl		9f												// if lim < 4, skip the following code

	movups	(T), %xmm7										// xmm7 is the tweak before encrypting every 4 blocks	
#ifdef	KERNEL
	movaps	%xmm5, 0xb0(sp)									// hw-aes-based uses extra xmm5
#endif

0:
	// derive 4 tweaks using xts_mult_x macro, and save on aligned stack space
	// xmm7 will be the tweak for next 4-blocks iteration

	#define	tweak1	16(sp)
	#define	tweak2	32(sp)
	#define	tweak3	48(sp)
	#define	tweak4	64(sp)

	movaps	%xmm7, tweak1									// save 1st tweak on stack
	xts_mult_x_on_xmm7										// compute 2nd tweak
	movaps	%xmm7, tweak2									// save 2nd tweak on stack
	xts_mult_x_on_xmm7										// compute 3rd tweak
	movaps	%xmm7, tweak3									// save 3rd tweak on stack
	xts_mult_x_on_xmm7										// compute 4th tweak
	movaps	%xmm7, tweak4									// save 4th tweak on stack
	xts_mult_x_on_xmm7										// compute 1st tweak for next iteration

	// read 4 Ps
	movups	(P), %xmm0
	movups	16(P), %xmm1
	movups	32(P), %xmm2
	movups	48(P), %xmm3

	// 4 C = P ^ T
	pxor	tweak1, %xmm0
	pxor	tweak2, %xmm1
	pxor	tweak3, %xmm2
	pxor	tweak4, %xmm3

	// 4 interleaved aes_encrypt

#if defined	__i386__
	mov		8(sp), %ecx	// ctx
	#undef	ctx
	#define	ctx	%ecx
#endif

	mov		240(ctx), %eax					// aes length 

	cmp		$160, %eax						// AES-128 ?
	je		160f
	cmp		$192, %eax						// AES-192 ?
	je		192f
	cmp		$224, %eax						// AES-256 ?
	je		224f
	mov		$-1, %eax						// error : non-supported aes length
#ifdef	KERNEL
	movaps	0xb0(sp), %xmm5					// hw-aes-based uses extra xmm5
#endif
	jmp		L_error_crypt

	// definitions, macros, and constructs for 4 blocks hw-aes-encrypt

	// the following key definitions will also be used in tweak_uncrypt_group 
	#define	key0			0(ctx)
	#define	key1			16(ctx)
	#define	key2			32(ctx)
	#define	key3			48(ctx)
	#define	key4			64(ctx)
	#define	key5			80(ctx)
	#define	key6			96(ctx)
	#define	key7			112(ctx)
	#define	key8			128(ctx)
	#define	key9			144(ctx)
	#define	keyA			160(ctx)
	#define	keyB			176(ctx)
	#define	keyC			192(ctx)
	#define	keyD			208(ctx)
	#define	keyE			224(ctx)

	#define	aes		aesenc
	#define	aeslast	aesenclast

	// all aes encrypt operations start with the following sequence
	.macro	aes_common_part
	movups	key0, %xmm4
	movups	key1, %xmm5
	pxor	%xmm4, %xmm0
	pxor	%xmm4, %xmm1
	pxor	%xmm4, %xmm2
	pxor	%xmm4, %xmm3
	movups	key2, %xmm4
	aes		%xmm5, %xmm0
	aes		%xmm5, %xmm1
	aes		%xmm5, %xmm2
	aes		%xmm5, %xmm3
	movups	key3, %xmm5
	aes		%xmm4, %xmm0
	aes		%xmm4, %xmm1
	aes		%xmm4, %xmm2
	aes		%xmm4, %xmm3
	movups	key4, %xmm4
	aes		%xmm5, %xmm0
	aes		%xmm5, %xmm1
	aes		%xmm5, %xmm2
	aes		%xmm5, %xmm3
	movups	key5, %xmm5
	aes		%xmm4, %xmm0
	aes		%xmm4, %xmm1
	aes		%xmm4, %xmm2
	aes		%xmm4, %xmm3
	movups	key6, %xmm4
	aes		%xmm5, %xmm0
	aes		%xmm5, %xmm1
	aes		%xmm5, %xmm2
	aes		%xmm5, %xmm3
	movups	key7, %xmm5
	aes		%xmm4, %xmm0
	aes		%xmm4, %xmm1
	aes		%xmm4, %xmm2
	aes		%xmm4, %xmm3
	movups	key8, %xmm4
	aes		%xmm5, %xmm0
	aes		%xmm5, %xmm1
	aes		%xmm5, %xmm2
	aes		%xmm5, %xmm3
	movups	key9, %xmm5
	aes		%xmm4, %xmm0
	aes		%xmm4, %xmm1
	aes		%xmm4, %xmm2
	aes		%xmm4, %xmm3
	movups	keyA, %xmm4
	aes		%xmm5, %xmm0
	aes		%xmm5, %xmm1
	aes		%xmm5, %xmm2
	aes		%xmm5, %xmm3
	.endm

	// all aes encypt operations end with the following 4 instructions	
	.macro	aes_last
	aeslast	%xmm4, %xmm0
	aeslast	%xmm4, %xmm1
	aeslast	%xmm4, %xmm2
	aeslast	%xmm4, %xmm3
	.endm

	.macro	aes_128
	aes_common_part			// encrypt common part
	aes_last				// encrypt ending part
	.endm
	
	.macro	aes_192
	aes_common_part			// encrypt common part

	// 10 extra instructions in between common and ending
	movups	keyB, %xmm5
	aes		%xmm4, %xmm0
	aes		%xmm4, %xmm1
	aes		%xmm4, %xmm2
	aes		%xmm4, %xmm3
	movups	keyC, %xmm4
	aes		%xmm5, %xmm0
	aes		%xmm5, %xmm1
	aes		%xmm5, %xmm2
	aes		%xmm5, %xmm3

	aes_last				// encrypt ending part
	.endm

	.macro	aes_256
	aes_common_part			// encrypt common part

	// 20 extra instructions in between common and ending
	movups	keyB, %xmm5
	aes		%xmm4, %xmm0
	aes		%xmm4, %xmm1
	aes		%xmm4, %xmm2
	aes		%xmm4, %xmm3
	movups	keyC, %xmm4
	aes		%xmm5, %xmm0
	aes		%xmm5, %xmm1
	aes		%xmm5, %xmm2
	aes		%xmm5, %xmm3
	movups	keyD, %xmm5
	aes		%xmm4, %xmm0
	aes		%xmm4, %xmm1
	aes		%xmm4, %xmm2
	aes		%xmm4, %xmm3
	movups	keyE, %xmm4
	aes		%xmm5, %xmm0
	aes		%xmm5, %xmm1
	aes		%xmm5, %xmm2
	aes		%xmm5, %xmm3

	aes_last				// encrypt ending part
	.endm

160:	// AES-128 encrypt
	aes_128
	jmp		8f

192:	// AES-192 encrypt
	aes_192
	jmp		8f

224:	// AES-256 encrypt	
	aes_256

8:	

	// 4 C = C ^ T
	pxor	tweak1, %xmm0
	pxor	tweak2, %xmm1
	pxor	tweak3, %xmm2
	pxor	tweak4, %xmm3

	// write 4 Cs
	movups	%xmm0, (C)
	movups	%xmm1, 16(C)
	movups	%xmm2, 32(C)
	movups	%xmm3, 48(C)

	add	$64, P
	add	$64, C

	sub		$4, lim
	jge		0b

#ifdef	KERNEL
	movaps	0xb0(sp), %xmm5				// hw-aes-based uses extra xmm5
#endif
	movups	%xmm7, (T)

9:
	xor		%eax, %eax					// to return CRYPT_OK
	add		$4, lim						// post-increment lim by 4
	je		9f							// if lim==0, branch to prepare to return	

L_crypt_group_sw:

	movups	(T), %xmm7					// T, xmm7 will be used as T (128-bit) throughtout the loop

	sub		$1, lim						// pre-decrement lim by 1	
	jl		1f							// if lim < 1, branch to prepare to return
0:
	movups	(P), %xmm0					// P

	// prepare for calling aes_encrypt
#if defined	__i386__
	mov		C, (%esp)					// C
	mov		C, 4(%esp)					// C
										// ctx was prepared previously in preamble
#else
	mov		C, %rdi						// C
	mov		C, %rsi						// C
	mov		ctx, %rdx					// ctx
#endif

	pxor	%xmm7, %xmm0				// C = P ^ T	
	movups	%xmm0, (C)					// save C into memory

	call	_aes_encrypt_xmm_no_save	// err = aes_encrypt(C,C,ctx);

	cmp		$CRYPT_OK, %eax				// err == CRYPT_OK ? 
	jne		9f							// if err != CRYPT_OK, branch to exit with error

	movups	(C), %xmm0					// load xmm0 with C
	pxor	%xmm7, %xmm0				// C ^= T
	movups	%xmm0, (C)					// save output C

	xts_mult_x_on_xmm7

	add		$16, C						// next C
	add		$16, P						// next P
	sub		$1, lim						// lim--
	jge		0b							// if (lim>0) repeat the scalar loop

1:	movups	%xmm7, (T)					// save final tweak 
L_error_crypt:
9:
	// if kernel, restore used xmm registers
#ifdef	KERNEL
	movaps	0x50(sp), %xmm0
	movaps	0x60(sp), %xmm1
	movaps	0x70(sp), %xmm2
	movaps	0x80(sp), %xmm3
	movaps	0x90(sp), %xmm4
	movaps	0xa0(sp), %xmm7
#endif

#if defined	__i386__
	add		$(12+16*8+16*4), %esp
	pop		%esi
	pop		%edi
	pop		%ebx
#else
	add		$(8+16*8+16*5), %rsp
	pop		%r15
	pop		%r14
	pop		%r13
	pop		%r12
	pop		%rbx
#endif
	leave
	ret

	#undef	P
	#undef	C
	#undef	T
	#undef	ctx
	#undef	sp

/* 
	The following is x86_64/i386 assembly implementation of 

	int tweak_uncrypt(const uint8_t *C, uint8_t *P, uint8_t *T, aesedp_decrypt_ctx *ctx);

	Its C code implementation is given in xtsClearC.c

	all pointers C/P/T points to a block of 16 bytes. In the following description, C/P/T represent 128-bit data.

	The operation of tweak_crypt

	1. P = C ^ T
	2. err = aes_decryp(P, P, ctx); if (err != CRYPT_OK) return err;
	3. P = P ^ T
	4. xts_mult_x(T)
	5. return CRYPT_OK;

	The following is the assembly implementation flow

	1. save used xmm registers (xmm1/xmm7) if kernel code 
	2. load xmm1 = C, xmm7 = T
	3. xmm1 = P = C ^ T
	4. write xmm1 to P
	5. call aes_decryp(P,P,ctx); note that it will use aesni if available, also xmm will return intact
	6. load xmm1 = P
	7. xmm1 = P = P^T = xmm1 ^ xmm7
	8. write xmm1 to P
	9. update T (in xmm7) via xts_mult_x macro
	a. restore xmm registers (xmm1/xmm7) if kernel code
	b. return CRYPT_OK (in eax) 

	Note: used xmm registers : xmm1/xmm2/xmm7, xmm2 in xts_mult_x macro

*/

	.text
	.align  4,0x90
	.globl	_aesxts_tweak_uncrypt
_aesxts_tweak_uncrypt:
#if defined	__i386__

	// push into stack for local use
	push	%ebp
	mov		%esp, %ebp
	push	%ebx
	push	%edi
	push	%esi

	// alllocate stack memory for local use
	sub		$12+16*4, %esp				// 12 (alignment) + 3*16 (xmm save/restore) + 16 (aes_crypt calling arguments)

	// load with called arguments
	mov		8(%ebp), %eax				// C, we need this only briefly, so eax is fine
	mov		12(%ebp), %edi				// P
	mov		16(%ebp), %ebx				// T
	mov		20(%ebp), %esi				// ctx

	#define	C	%eax
	#define	P	%edi
	#define	T	%ebx
	#define	ctx	%esi
	#define	sp	%esp

#else
	// x86_64 calling argument order : rdi/rsi/rdx/rcx/r8

	// push into stack for local use
	push	%rbp
	mov		%rsp, %rbp
	push	%r12
	push	%r13
	push	%r14
	push	%r15

	// alllocate stack memory for local use, if kernel code, need to save/restore xmm registers
#ifdef KERNEL
	sub		$4*16, %rsp					// only need 3*16, add 16 extra so to make save/restore xmm common to i386
#endif

	// load with called arguments, release rdi/rsi/rdx/rcx/r8, as need to call aes_decrypt
	mov		%rsi, %r13
	mov		%rdx, %r14
	mov		%rcx, %r15

	#define	C 	%rdi
	#define	P	%r13
	#define	T	%r14
	#define	ctx	%r15
	#define	sp	%rsp

#endif

	// if kernel, save used xmm registers
#ifdef	KERNEL
	movaps	%xmm1, 16(sp)
	movaps	%xmm2, 32(sp)
	movaps	%xmm7, 48(sp)
#endif

	movups	(C), %xmm1					// C
	movups	(T), %xmm7					// T

	// setup caliing arguments for aes_decrypt
#if defined	__i386__
	mov		P, (%esp)					// P
	mov		P, 4(%esp)					// P
	mov		ctx, 8(%esp)				// ctx
#else
	mov		P, %rdi						// P
	mov		P, %rsi						// P
	mov		ctx, %rdx					// ctx
#endif

	pxor	%xmm7, %xmm1				// P = C ^ T	
	movups	%xmm1, (P)					// save P into memory

	call	_aes_decrypt				// err = aes_decrypt(P,P,ctx);

	cmp		$CRYPT_OK, %eax				// check err == CRYPT_OK
	jne		9f							// if err != CRYPT_OK, exit

	movups	(P), %xmm1					// load xmm1 = P
	pxor	%xmm7, %xmm1				// P ^= T
	movups	%xmm1, (P)					// write P with xmm1, xmm1 is freed now, will be changed in the following macro

	xts_mult_x_on_xmm7					// update T (on xmm7)

	movups	%xmm7, (T)					// write xmm7 to T
9:

	// restore used xmm registers if this is for kernel
#ifdef	KERNEL
	movaps	16(sp), %xmm1
	movaps	32(sp), %xmm2
	movaps	48(sp), %xmm7
#endif

	// free stack memory and restore callee registers
#if defined	__i386__
	add		$12+16*4, %esp				// 12 (alignment) + 3*16 (xmm save/restore) + 16 (aes_crypt calling arguments)
	pop		%esi
	pop		%edi
	pop		%ebx
#else
#ifdef	KERNEL
	add		$4*16, %rsp					// only need 3*16, add 16 extra so make save/restore xmm common to i386
#endif
	pop		%r15
	pop		%r14
	pop		%r13
	pop		%r12
#endif

	// return, eax/rax already has the return val
	leave
	ret

	#undef	P
	#undef	C
	#undef	T
	#undef	ctx
	#undef	sp

/* 
	The following is x86_64/i386 assembly implementation of 

	int tweak_uncrypt_group(const uint8_t *C, uint8_t *P, uint8_t *T, aesedp_decrypt_ctx *ctx, uint32_t lim);

	TODO : Its C code implementation is YET to be provided in xtsClearC.c (for the benefit of porting to other ISAs)
	This function is grouped version of the above function tweak_uncrypt(), so xmm registers save/restore only need
	to happen once for all grouped blocks.

	The implementation here probes __cpu_capabilities to detect whether aesni (or hw-aes instruction) is available.
	If aesni is available, the code branch to optimized code that uses aesni.

	The optimized aesni code operates as follows:

	while (more than 4 consecutive blocks available) {

		do xts_mult_x macro 4 times and write the 4 tweaks on stack (16-byte aligned)
	
		perform 4 P = C ^ T;	// T is on 16-byte aligned stack

		perform 4 aes_decrypt (all aes_decrypt instruction interleaved to achieve better throughtput)

		perform 4 P = P ^ T		// T is on 16-byte aligned stack

	}

	The code then falls through to the scalar code, that sequentially performs what tweak_crypt does

	1. P = C ^ T
	2. err = aes_decryp(P, P, ctx); if (err != CRYPT_OK) return err;
	3. P = P ^ T
	4. xts_mult_x(T)

	Note: used xmm registers : 
			xmm0-xmm5, xmm7 if aesni is available
			xmm0-xmm4, xmm7 if aesni is not available.

*/


    .text
	.align  4,0x90
	.globl	_aesxts_tweak_uncrypt_group
_aesxts_tweak_uncrypt_group:

#if defined	__i386__

	// push callee-saved registers for local use
	push	%ebp
	mov		%esp, %ebp
	push	%ebx
	push	%edi
	push	%esi

	// allocate stack memory for local use and/or xmm register save for kernel code
	sub		$(12+8*16+16*4), %esp		// 12 (alignment) + 8*16 (xmm) + 4*16 (pre-computed tweaks) aesni
										// 12 (alignment) + 8*16 (xmm) + 4*16 (only 12 used for aes_decrypt) no aesni 
	// transfer calling arguments
	mov		20(%ebp), %eax				// ctx
	mov		12(%ebp), %edi				// P
	mov		16(%ebp), %ebx				// T
	mov		8(%ebp), %esi				// C
	mov		%eax, 8(%esp)				// ctx as the 3rd parameter to aes_decrypt

	#define	C	%esi
	#define	P	%edi
	#define	T	%ebx
	#define	lim	24(%ebp)
	#define	sp	%esp

#else

	// push callee-saved registers for local use
	push	%rbp
	mov		%rsp, %rbp
	push	%rbx
	push	%r12
	push	%r13
	push	%r14
	push	%r15

	// allocate stack memory for local use and/or xmm register save for kernel code
	sub		$(8+8*16+16*5), %rsp		// 8 (alignment) + 8*16 (xmm) + 4*16 (pre-computed tweaks) + 16 (common to i386)

	// rdi/rsi/rdx/rcx/r8
	// transfer calling arguments
	mov		%rdi, %r12	
	mov		%rsi, %r13
	mov		%rdx, %r14
	mov		%rcx, %r15
	mov		%r8,  %rbx

	#define	C 	%r12
	#define	P	%r13
	#define	T	%r14
	#define	ctx	%r15
	#define	lim	%ebx
	#define	sp	%rsp
#endif

#ifdef	KERNEL
	movaps	%xmm0, 0x50(sp)
	movaps	%xmm1, 0x60(sp)
	movaps	%xmm2, 0x70(sp)
	movaps	%xmm3, 0x80(sp)
	movaps	%xmm4, 0x90(sp)
	movaps	%xmm7, 0xa0(sp)
#endif

	// probe __cpu_capabilities to detect aesni
#if defined __x86_64__
    movq    __cpu_capabilities@GOTPCREL(%rip), %rax         // %rax -> __cpu_capabilities
    mov     (%rax), %eax                                    // %eax = __cpu_capabilities
#else		// i386
#if defined KERNEL
    leal    __cpu_capabilities, %eax                        // %eax -> __cpu_capabilities
    mov     (%eax), %eax                                    // %eax = __cpu_capabilities
#else
    movl     _COMM_PAGE_CPU_CAPABILITIES, %eax
#endif
#endif
	test    $(kHasAES), %eax
	je		L_uncrypt_group_sw								// if aesni not available, jump to sw-based implementation

	// aesni-based implementation

	sub		$4, lim											// pre-decrement lim by 4
	jl		9f												// if lim < 4, skip the following code

	movups	(T), %xmm7										// xmm7 is the tweak before decrypting every 4 blocks	
#ifdef	KERNEL
	movaps	%xmm5, 0xb0(sp)									// hw-aes-based uses extra xmm5
#endif

0:
	// derive 4 tweaks using xts_mult_x macro, and save on aligned stack space
	// xmm7 will be the tweak for next 4-blocks iteration

	#define	tweak1	16(sp)
	#define	tweak2	32(sp)
	#define	tweak3	48(sp)
	#define	tweak4	64(sp)

	movaps	%xmm7, tweak1									// save 1st tweak on stack
	xts_mult_x_on_xmm7										// compute 2nd tweak
	movaps	%xmm7, tweak2									// save 2nd tweak on stack
	xts_mult_x_on_xmm7										// compute 3rd tweak
	movaps	%xmm7, tweak3									// save 3rd tweak on stack
	xts_mult_x_on_xmm7										// compute 4th tweak
	movaps	%xmm7, tweak4									// save 4th tweak on stack
	xts_mult_x_on_xmm7										// compute 1st tweak for next iteration

	// read 4 Cs
	movups	(C), %xmm0
	movups	16(C), %xmm1
	movups	32(C), %xmm2
	movups	48(C), %xmm3

	// 4 P = C ^ T
	pxor	tweak1, %xmm0
	pxor	tweak2, %xmm1
	pxor	tweak3, %xmm2
	pxor	tweak4, %xmm3

	// 4 interleaved aes_decrypt

#if defined	__i386__
	mov		8(sp), %ecx	// ctx
	#undef	ctx
	#define	ctx	%ecx
#endif

	mov		240(ctx), %eax					// aes length 

	cmp		$160, %eax						// AES-128 ?
	je		160f
	cmp		$192, %eax						// AES-192 ?
	je		192f
	cmp		$224, %eax						// AES-256 ?
	je		224f
	mov		$-1, %eax						// error : non-supported aes length
#ifdef	KERNEL
	movaps	0xb0(sp), %xmm5					// hw-aes-based uses extra xmm5
#endif
	jmp		L_error_uncrypt

	// definitions, macros to construc hw-aes-decrypt
	// will reuse previously defined key0 = (ctx), key1 = 16(ctx), ....
	#undef	aes
	#undef	aeslast
	#define aes	aesdec
	#define	aeslast aesdeclast

	.macro	aes_decrypt_common
	movups	key8, %xmm4
	aes		%xmm5, %xmm0
	aes		%xmm5, %xmm1
	aes		%xmm5, %xmm2
	aes		%xmm5, %xmm3
	movups	key7, %xmm5
	aes		%xmm4, %xmm0
	aes		%xmm4, %xmm1
	aes		%xmm4, %xmm2
	aes		%xmm4, %xmm3
	movups	key6, %xmm4
	aes		%xmm5, %xmm0
	aes		%xmm5, %xmm1
	aes		%xmm5, %xmm2
	aes		%xmm5, %xmm3
	movups	key5, %xmm5
	aes		%xmm4, %xmm0
	aes		%xmm4, %xmm1
	aes		%xmm4, %xmm2
	aes		%xmm4, %xmm3
	movups	key4, %xmm4
	aes		%xmm5, %xmm0
	aes		%xmm5, %xmm1
	aes		%xmm5, %xmm2
	aes		%xmm5, %xmm3
	movups	key3, %xmm5
	aes		%xmm4, %xmm0
	aes		%xmm4, %xmm1
	aes		%xmm4, %xmm2
	aes		%xmm4, %xmm3
	movups	key2, %xmm4
	aes		%xmm5, %xmm0
	aes		%xmm5, %xmm1
	aes		%xmm5, %xmm2
	aes		%xmm5, %xmm3
	movups	key1, %xmm5
	aes		%xmm4, %xmm0
	aes		%xmm4, %xmm1
	aes		%xmm4, %xmm2
	aes		%xmm4, %xmm3
	movups	key0, %xmm4
	aes		%xmm5, %xmm0
	aes		%xmm5, %xmm1
	aes		%xmm5, %xmm2
	aes		%xmm5, %xmm3
	aeslast	%xmm4, %xmm0
	aeslast	%xmm4, %xmm1
	aeslast	%xmm4, %xmm2
	aeslast	%xmm4, %xmm3
	.endm

	.macro	aes_dec_128
	movups	keyA, %xmm4
	movups	key9, %xmm5
	pxor	%xmm4, %xmm0
	pxor	%xmm4, %xmm1
	pxor	%xmm4, %xmm2
	pxor	%xmm4, %xmm3
	aes_decrypt_common
	.endm

	.macro	aes_dec_192
	movups	keyC, %xmm4
	movups	keyB, %xmm5
	pxor	%xmm4, %xmm0
	pxor	%xmm4, %xmm1
	pxor	%xmm4, %xmm2
	pxor	%xmm4, %xmm3
	movups	keyA, %xmm4
	aes		%xmm5, %xmm0
	aes		%xmm5, %xmm1
	aes		%xmm5, %xmm2
	aes		%xmm5, %xmm3
	movups	key9, %xmm5
	aes		%xmm4, %xmm0
	aes		%xmm4, %xmm1
	aes		%xmm4, %xmm2
	aes		%xmm4, %xmm3
	aes_decrypt_common
	.endm

	.macro	aes_dec_256
	movups	keyE, %xmm4
	movups	keyD, %xmm5
	pxor	%xmm4, %xmm0
	pxor	%xmm4, %xmm1
	pxor	%xmm4, %xmm2
	pxor	%xmm4, %xmm3
	movups	keyC, %xmm4
	aes		%xmm5, %xmm0
	aes		%xmm5, %xmm1
	aes		%xmm5, %xmm2
	aes		%xmm5, %xmm3
	movups	keyB, %xmm5
	aes		%xmm4, %xmm0
	aes		%xmm4, %xmm1
	aes		%xmm4, %xmm2
	aes		%xmm4, %xmm3
	movups	keyA, %xmm4
	aes		%xmm5, %xmm0
	aes		%xmm5, %xmm1
	aes		%xmm5, %xmm2
	aes		%xmm5, %xmm3
	movups	key9, %xmm5
	aes		%xmm4, %xmm0
	aes		%xmm4, %xmm1
	aes		%xmm4, %xmm2
	aes		%xmm4, %xmm3
	aes_decrypt_common
	.endm

160:	// AES-128 decrypt
	aes_dec_128
	jmp		8f

192:	// AES-192 decrypt
	aes_dec_192
	jmp		8f

224:	// AES-256 decrypt	
	aes_dec_256

8:	

	// 4 P = P ^ T
	pxor	tweak1, %xmm0
	pxor	tweak2, %xmm1
	pxor	tweak3, %xmm2
	pxor	tweak4, %xmm3

	// write 4 Ps
	movups	%xmm0, (P)
	movups	%xmm1, 16(P)
	movups	%xmm2, 32(P)
	movups	%xmm3, 48(P)

	add	$64, C
	add	$64, P

	sub		$4, lim
	jge		0b

#ifdef	KERNEL
	movaps	0xb0(sp), %xmm5				// hw-aes-based uses extra xmm5
#endif
	movups	%xmm7, (T)

9:
	xor		%eax, %eax					// to return CRYPT_OK
	add		$4, lim						// post-increment lim by 4
	je		9f							// if lim==0, branch to prepare to return	

L_uncrypt_group_sw:

	movups	(T), %xmm7					// T, xmm7 will be used as T (128-bit) throughtout the loop

	sub		$1, lim						// pre-decrement lim by 1	
	jl		1f							// if lim < 1, branch to prepare to return
0:
	movups	(C), %xmm0					// C

	// prepare for calling aes_decrypt
#if defined	__i386__
	mov		P, (%esp)					// P
	mov		P, 4(%esp)					// P
										// ctx was prepared previously in preamble
#else
	mov		P, %rdi						// P
	mov		P, %rsi						// P
	mov		ctx, %rdx					// ctx
#endif

	pxor	%xmm7, %xmm0				// P = C ^ T	
	movups	%xmm0, (P)					// save P into memory

	call	_aes_decrypt_xmm_no_save	// err = aes_decrypt(P,P,ctx);

	cmp		$CRYPT_OK, %eax				// err == CRYPT_OK ? 
	jne		9f							// if err != CRYPT_OK, branch to exit with error

	movups	(P), %xmm0					// load xmm0 with P
	pxor	%xmm7, %xmm0				// P ^= T
	movups	%xmm0, (P)					// save output P

	xts_mult_x_on_xmm7

	add		$16, C						// next C
	add		$16, P						// next P
	sub		$1, lim						// lim--
	jge		0b							// if (lim>0) repeat the scalar loop

1:	movups	%xmm7, (T)					// save final tweak 
L_error_uncrypt:
9:
	// if kernel, restore used xmm registers
#ifdef	KERNEL
	movaps	0x50(sp), %xmm0
	movaps	0x60(sp), %xmm1
	movaps	0x70(sp), %xmm2
	movaps	0x80(sp), %xmm3
	movaps	0x90(sp), %xmm4
	movaps	0xa0(sp), %xmm7
#endif

#if defined	__i386__
	add		$(12+16*8+16*4), %esp
	pop		%esi
	pop		%edi
	pop		%ebx
#else
	add		$(8+16*8+16*5), %rsp
	pop		%r15
	pop		%r14
	pop		%r13
	pop		%r12
	pop		%rbx
#endif
	leave
	ret
#endif /* x86 based build */
