/*
 ---------------------------------------------------------------------------
 Copyright (c) 2003, Dr Brian Gladman, Worcester, UK.   All rights reserved.

 LICENSE TERMS

 The free distribution and use of this software in both source and binary
 form is allowed (with or without changes) provided that:

   1. distributions of this source code include the above copyright
      notice, this list of conditions and the following disclaimer;

   2. distributions in binary form include the above copyright
      notice, this list of conditions and the following disclaimer
      in the documentation and/or other associated materials;

   3. the copyright holder's name is not used to endorse products
      built using this software without specific written permission.

 ALTERNATIVELY, provided that this notice is retained in full, this product
 may be distributed under the terms of the GNU General Public License (GPL),
 in which case the provisions of the GPL apply INSTEAD OF those given above.

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
 Issue 31/01/2006

 These subroutines implement multiple block AES modes for ECB, CBC, CFB,
 OFB and CTR encryption,  The code provides support for the VIA Advanced 
 Cryptography Engine (ACE).

 NOTE: In the following subroutines, the AES contexts (ctx) must be
 16 byte aligned if VIA ACE is being used
*/

#if defined __i386__ || defined __x86_64__

/* modified 3/5/10 cclee */
/* Clean up those related to VIA ACE and hand optimize aes_cbc_encrypt and aes_cbc_decrypt */
/* move the xmm registers save/restore originally inside the callee functions into these 2 caller functions */

/* HW-AES specific implementation cclee 3-12-10 */
/* In aes_encrypt_cbc and aes_decrypt_cbc, __cpu_capabilities is polled, 
	and if kHasAES is detected, branch to the hw-specific functions here */


/* 	
	This files defines _aesedp_encrypt_cbc_hw and _aesedp_decrypt_cbc_hw --- Intel Westmere HW AES-based implementation
	of _aes_encrypt_cbc and _aes_decrypt_cbc. 

	These 2 functions SHOULD BE entried ONLY after the AES HW is verified to be available. 
	They SHOULD NOT be called without AES HW detection. It might cause xnu to crash.

	The AES HW is detected 1st thing in 
		_aes_encrypt_cbc (aes_modes_asm.s) 
		_aes_decrypt_cbc (aes_modes_asm.s)
	and, if AES HW is detected, branch without link (ie, jump) to the functions here.

	The implementation here follows the examples in an Intel White Paper
	"Intel Advanced Encryption Standard (AES) Instruction Set" Rev.2 01

	Note: Rev. 03 Final 2010 01 26 is available. Looks like some code change from Rev.2 01

	cclee 3-13-10
*/

/* 
	The function _aesedp_decrypt_cbc_hw previously simply serially decrypts block by block
	in our group meeting, Eric/Ali suggested that I perhaps should take a look of combining multiple blocks
	in a loop and interleaving multiple aesdec instructions to absorb/hide stalls to improve the decrypt thoughput.

	The idea was actually described in the Intel AES Instruction Set White Paper (Rev. 2.0 page 53-55) 

	This modification interleaves the aesdec/aesdeclast instructions for 4 blocks in cbc mode.
	On a 2.4GHz core-i5/2.66GHz core-i7, the x86_64 decrypt throughput (in xnu-iokit) has been improved
	from 1180/1332 to 1667/1858 MBytes/sec. This is approximately 1.40 times speedup in the decryption.
	The encrypt throughput is not changed.  

	I also enhanced the assembly code comments.

	cclee-4-30-10 (Do you know 4-30 is National Honesty Day in the US? No need to know. I've been honest all the time.)

*/

/* cclee 7-30-10
    per murf's request, I changed the type of the 2nd argument iv in aes_encrypt_cbc/aes_decrypt_cbc
    from "const unsigned char *" to "unsigned char *". That is, the updated *iv (16-bytes) is written back
    to the memory in the caller function pointed by the input argument.
    In the implementation, *iv is locally saved in %xmm7.
    Before return, we now write %xmm7 back to *iv.

    Note: we only do this in CommonCrypto. In the kernel, there are some other functions (IOKit/vm_pageout, e.g.)
    that might assume *iv is read only, and therefore should not be changed. This is being tracked in

    <rdar://problem/8256020> xnu : add cbc feature in bsd/crypto/aes/i386/
    
*/

/* ---------------------------------------------------------------------------------------------------------------- 

	aes_encrypt_cbc function (see aes_modes.c or aes_modes_asm.s) :

	For simplicity, I am assuming all variables are in 128-bit data type.

	aes_rval aes_encrypt_cbc(const __m128 *ibuf, __m128 *iv, int num_blk, __m128 *obuf, const aes_encrypt_ctx *ctx)
	{
		while(num_blk--) {
			*iv ^= *ibuf++;
			aes_encrypt(iv, iv, ctx);
			*obuf++ = *iv;
		}
		return 0;
	}

	The following is an implementation of this function using Intel AESNI.
	This function _aesedp_encrypt_cbc_hw SHOULD NOT be called directly. 
	Developer should still call _aes_encrypt_cbc (in aes_modes_asm.s) which will poll cpu_capabilities and branch
	to this aesni-based function should it detecs that aesni is available.
	Blindly call this function SURELY will cause a CRASH on systems with no aesni support. 

	Note that each block starts with *iv, which is the output of the previous block. Therefore, the cbc blocks
	are serially chained. This prevents us from arranging several blocks for encryption in parallel.

   ----------------------------------------------------------------------------------------------------------------*/

	.text
	.align	4,0x90
	.globl	_aesedp_encrypt_cbc_hw
_aesedp_encrypt_cbc_hw:

	// push/save registers for local use
#if	defined	__i386__

	push	%ebp
	movl	%esp, %ebp
	push	%ebx
	push	%edi

	#define	sp	%esp

#else	// __x86_64__

	push	%rbp
	mov		%rsp, %rbp
	push	%rbx
	push	%r13
	push	%r14
	push	%r15

	#define	sp	%rsp

#endif

	// if this is kernel code, need to save used xmm registers
#ifdef	KERNEL

#if defined __i386__
	sub		$(8*16), %esp			// for possible xmm0-xmm7 save/restore
#else
	sub		$(16*16), %rsp		// xmm0-xmm15 save/restore	
#endif

	movaps	%xmm0, (sp)
	movaps	%xmm1, 16(sp)
	movaps	%xmm2, 32(sp)
	movaps	%xmm3, 48(sp)
	movaps	%xmm4, 64(sp)
	movaps	%xmm5, 80(sp)
	movaps	%xmm6, 96(sp)
	movaps	%xmm7, 112(sp)
#if defined	__x86_64__
	movaps	%xmm8, 16*8(sp)
	movaps	%xmm9, 16*9(sp)
	movaps	%xmm10, 16*10(sp)
	movaps	%xmm11, 16*11(sp)
	movaps	%xmm12, 16*12(sp)
	movaps	%xmm13, 16*13(sp)
	movaps	%xmm14, 16*14(sp)
	movaps	%xmm15, 16*15(sp)
#endif	// __x86_64__

#endif	// KERNEL

	#define	iv	%xmm0

#ifdef	__i386__

	mov		12(%ebp), %eax			// &iv[0]
	mov		24(%ebp), %edx			// ctx
	movups	(%eax), iv				// iv = in_iv	
	mov		8(%ebp), %ebx			// ibuf
	mov		16(%ebp), %ecx			// num_blk
	mov		20(%ebp), %edi			// obuf

	#define	ibuf	%ebx
	#define	obuf	%edi
	#define num_blk	%ecx	
	#define	ctx		%edx

#else

	mov		%rdi, %rbx				// ibuf
	movups	(%rsi), iv				// iv = in_iv
	mov		%rdx, %r13				// num_blk
	mov		%rcx, %r14				// obuf
	mov		%r8, %r15				// ctx	

	#define	ibuf	%rbx
	#define	num_blk	%r13d
	#define	obuf	%r14	
	#define	ctx		%r15

#endif

	mov		240(ctx), %eax			// aes length
	cmp		$160, %eax				// aes-128 encrypt ?
	je		L_encrypt_128
	cmp		$192, %eax				// aes-192 encrypt ?
	je		L_encrypt_192
	cmp		$224, %eax				// aes-256 encrypt ?
	je		L_encrypt_256
	mov		$-1, %eax				// return error
	jmp		L_error	

	//
	// aes-128 encrypt_cbc operation, up to L_HW_cbc_done
	//

L_encrypt_128:

	cmp		$1, num_blk				// check number of block
	jl		L_HW_cbc_done			// should it be less than 1, nothing to do

	movups	(ctx), %xmm2			// key0
	movups	16(ctx), %xmm3			// key1
	movups	32(ctx), %xmm4			// key2
	movups	48(ctx), %xmm5			// key3
	movups	64(ctx), %xmm6			// key4
	movups	80(ctx), %xmm7			// key5
#if defined	__x86_64__
	movups	96(ctx), %xmm8			// key6
	movups	112(ctx), %xmm9			// key7
	movups	128(ctx), %xmm10		// key8
	movups	144(ctx), %xmm11		// key9
	movups	160(ctx), %xmm12		// keyA
#endif

	// while (num_blk--) {
	//			*iv ^= *ibuf++;
	//			aes_encrypt(iv, iv, ctx);
	//			*obuf++ = *iv;
	// }
0:
	movups	(ibuf), %xmm1				// *ibuf
	pxor    %xmm2, iv					// 1st instruction inside aes_encrypt
	pxor	%xmm1, iv					// *iv ^= *ibuf

	// finishing up the rest of aes_encrypt
    aesenc  %xmm3, iv
    aesenc  %xmm4, iv
    aesenc  %xmm5, iv
    aesenc  %xmm6, iv
    aesenc  %xmm7, iv
#if defined	__x86_64__
    aesenc  %xmm8, iv
    aesenc  %xmm9, iv
    aesenc  %xmm10, iv
    aesenc  %xmm11, iv
    aesenclast  %xmm12, iv
#else
	movups	96(ctx), %xmm1				// key6
    aesenc  %xmm1, iv
	movups	112(ctx), %xmm1				// key7
    aesenc  %xmm1, iv
	movups	128(ctx), %xmm1				// key8
    aesenc  %xmm1, iv
	movups	144(ctx), %xmm1				// key9
    aesenc  %xmm1, iv
	movups	160(ctx), %xmm1				// keyA
    aesenclast  %xmm1, iv
#endif

	movups	iv, (obuf)					// *obuf = *iv;
	add		$16, obuf					// obuf++;
	add		$16, ibuf					// ibuf++;
	sub		$1, num_blk					// num_blk --
	jg		0b							// if num_blk > 0, repeat the loop

	// the following will be branched to from all other cases (encrypt/decrypt 128/192/256)

L_HW_cbc_done:

	// save the updated *iv
#if defined __i386__
	mov		12(%ebp), %eax
	movups	iv, (%eax)
#else
	movups	iv, (%rsi)
#endif

	xor		%eax, %eax				// to return CRYPT_OK

L_error:

	// if kernel, restore xmm registers
#ifdef	KERNEL 
	movaps	0(sp), %xmm0
	movaps	16(sp), %xmm1
	movaps	32(sp), %xmm2
	movaps	48(sp), %xmm3
	movaps	64(sp), %xmm4
	movaps	80(sp), %xmm5
	movaps	96(sp), %xmm6
	movaps	112(sp), %xmm7
#if defined	__x86_64__
	movaps	16*8(sp), %xmm8
	movaps	16*9(sp), %xmm9
	movaps	16*10(sp), %xmm10
	movaps	16*11(sp), %xmm11
	movaps	16*12(sp), %xmm12
	movaps	16*13(sp), %xmm13
	movaps	16*14(sp), %xmm14
	movaps	16*15(sp), %xmm15
#endif	// __x86_64__
#endif	// KERNEL

	// release used stack memory, restore used callee-saved registers, and return 
#if	defined	__i386__
#ifdef	KERNEL
	add		$(8*16), %esp
#endif
	pop		%edi
	pop		%ebx
#else
#ifdef	KERNEL
	add		$(16*16), %rsp	
#endif
	pop		%r15
	pop		%r14
	pop		%r13
	pop		%rbx
#endif
	leave
	ret

	//
	// aes-192 encrypt_cbc operation, after completion, branch to L_HW_cbc_done
	//

L_encrypt_192:

	cmp		$1, num_blk				// check number of block
	jl		L_HW_cbc_done			// should it be less than 1, nothing to do

	movups	(ctx), %xmm2			// key0
	movups	16(ctx), %xmm3			// key1
	movups	32(ctx), %xmm4			// key2
	movups	48(ctx), %xmm5			// key3
	movups	64(ctx), %xmm6			// key4
	movups	80(ctx), %xmm7			// key5
#if defined	__x86_64__
	movups	96(ctx), %xmm8			// key6
	movups	112(ctx), %xmm9			// key7
	movups	128(ctx), %xmm10		// key8
	movups	144(ctx), %xmm11		// key9
	movups	160(ctx), %xmm12		// keyA
	movups	176(ctx), %xmm13		// keyB
	movups	192(ctx), %xmm14		// keyC
#endif
	
	// while (num_blk--) {
	//			*iv ^= *ibuf++;
	//			aes_encrypt(iv, iv, ctx);
	//			*obuf++ = *iv;
	// }
0:
	movups	(ibuf), %xmm1			// *ibuf
	pxor	%xmm1, iv				// *iv ^= ibuf

	// aes_encrypt(iv, iv, ctx);

	pxor    %xmm2, iv
    aesenc  %xmm3, iv
    aesenc  %xmm4, iv
    aesenc  %xmm5, iv
    aesenc  %xmm6, iv
    aesenc  %xmm7, iv
#if defined	__x86_64__
    aesenc  %xmm8, iv
    aesenc  %xmm9, iv
    aesenc  %xmm10, iv
    aesenc  %xmm11, iv
    aesenc  %xmm12, iv
    aesenc  %xmm13, iv
    aesenclast  %xmm14, iv
#else
	movups	96(ctx), %xmm1
    aesenc  %xmm1, iv
	movups	112(ctx), %xmm1
    aesenc  %xmm1, iv
	movups	128(ctx), %xmm1
    aesenc  %xmm1, iv
	movups	144(ctx), %xmm1
    aesenc  %xmm1, iv
	movups	160(ctx), %xmm1
    aesenc  %xmm1, iv
	movups	176(ctx), %xmm1
    aesenc  %xmm1, iv
	movups	192(ctx), %xmm1
    aesenclast  %xmm1, iv
#endif

	movups	iv, (obuf)				// *obuf = *iv;
	add		$16, ibuf				// ibuf++
	add		$16, obuf				// obuf++

	sub		$1, num_blk				// num_blk --
	jg		0b						// if num_blk > 0, repeat the loop

	jmp		L_HW_cbc_done			// share with the common exit code

	//
	// aes-256 encrypt_cbc operation, after completion, branch to L_HW_cbc_done
	//

L_encrypt_256:

	cmp		$1, num_blk				// check number of block
	jl		L_HW_cbc_done			// should it be less than 1, nothing to do

	movups	(ctx), %xmm2			// key0
	movups	16(ctx), %xmm3			// key1
	movups	32(ctx), %xmm4			// key2
	movups	48(ctx), %xmm5			// key3
	movups	64(ctx), %xmm6			// key4
	movups	80(ctx), %xmm7			// key5
#if defined	__x86_64__
	movups	96(ctx), %xmm8			// key6
	movups	112(ctx), %xmm9			// key7
	movups	128(ctx), %xmm10		// key8
	movups	144(ctx), %xmm11		// key9
	movups	160(ctx), %xmm12		// keyA
	movups	176(ctx), %xmm13		// keyB
	movups	192(ctx), %xmm14		// keyC
	movups	208(ctx), %xmm15		// keyD
	// movups	224(ctx), %xmm1		// keyE
#endif

	// while (num_blk--) {
	//			*iv ^= *ibuf++;
	//			aes_encrypt(iv, iv, ctx);
	//			*obuf++ = *iv;
	// }
0:
	movups	(ibuf), %xmm1			// *ibuf
	pxor	%xmm1, iv				// *iv ^= ibuf
	
	// aes_encrypt(iv, iv, ctx);
	pxor    %xmm2, iv
    aesenc  %xmm3, iv
    aesenc  %xmm4, iv
    aesenc  %xmm5, iv
    aesenc  %xmm6, iv
    aesenc  %xmm7, iv
#if defined	__x86_64__
	movups	224(ctx), %xmm1			// keyE
    aesenc  %xmm8, iv
    aesenc  %xmm9, iv
    aesenc  %xmm10, iv
    aesenc  %xmm11, iv
    aesenc  %xmm12, iv
    aesenc  %xmm13, iv
    aesenc  %xmm14, iv
    aesenc  %xmm15, iv
    aesenclast  %xmm1, iv
#else
	movups	96(ctx), %xmm1			// key6
    aesenc  %xmm1, iv
	movups	112(ctx), %xmm1			// key7
    aesenc  %xmm1, iv
	movups	128(ctx), %xmm1			// key8
    aesenc  %xmm1, iv
	movups	144(ctx), %xmm1			// key9
    aesenc  %xmm1, iv
	movups	160(ctx), %xmm1			// keyA
    aesenc  %xmm1, iv
	movups	176(ctx), %xmm1			// keyB
    aesenc  %xmm1, iv
	movups	192(ctx), %xmm1			// keyC
    aesenc  %xmm1, iv
	movups	208(ctx), %xmm1			// keyD
    aesenc  %xmm1, iv
	movups	224(ctx), %xmm1			// keyE
    aesenclast  %xmm1, iv
#endif

	movups	iv, (obuf)				// *obuf = *iv;
	add		$16, ibuf				// ibuf++
	add		$16, obuf				// obuf++

	sub		$1, num_blk				// num_blk --
	jg		0b						// if num_blk > 0, repeat the loop

	jmp		L_HW_cbc_done			// share with the common exit code



	//
	// --------- END of aesedp_encrypt_cbc_hw  -------------------
	//


/* ---------------------------------------------------------------------------------------------------------------- 

	aes_decrypt_cbc function (see aes_modes.c or aes_modes_asm.s) :

	For simplicity, I am assuming all variables are in 128-bit data type.

	aes_rval aes_decrypt_cbc(const __m128 *ibuf, __m128 *iv, int num_blk, __m128 *obuf, const aes_decrypt_ctx *ctx)
	{
		while(num_blk--) {
			aes_decrypt(ibuf, obuf, ctx);
			*obuf++ ^= *iv;
			*iv = *ibuf++;
		}
		return 0;
	}

	The following is an implementation of this function using Intel AESNI.
	This function _aesedp_decrypt_cbc_hw SHOULD NOT be called directly. 
	Developer should still call _aes_decrypt_cbc (in aes_modes_asm.s) which will poll cpu_capabilities and branch
	to this aesni-based function should it detecs that aesni is available.
	Blindly call this function SURELY will cause a CRASH on systems with no aesni support. 

	Note that the decryption operation is not related over blocks.
	This gives opportunity of arranging aes_decrypt operations in parallel to speed up code.
	This is equivalent to what has been described in the Intel AES Instruction Set White Paper (Rev. 2.0 page 53-55)
	The following assembly code exploits this idea to achieve ~ 1.4 speed up in aes_decrypt_cbc.

	Example C code for packing 4 blocks in an iteration is shown as follows:

		while ((num_blk-=4)>=0) {

			// the following 4 functions can be interleaved to exploit parallelism
			aes_decrypt(ibuf, obuf, ctx);
			aes_decrypt(ibuf+1, obuf+1, ctx);
			aes_decrypt(ibuf+2, obuf+2, ctx);
			aes_decrypt(ibuf+3, obuf+3, ctx);

			obuf[0] ^= *iv; obuf[1] ^= ibuf[1]; obuf[2] ^= ibuf[1]; obuf[3] ^= ibuf[2];
			*iv = ibuf[3];		ibuf += 4; 	obuf += 4;
		}
		num_blk+=4;

   ----------------------------------------------------------------------------------------------------------------*/

	.text
	.align	4,0x90
	.globl	_aesedp_decrypt_cbc_hw
_aesedp_decrypt_cbc_hw:

	// push/save registers for local use
#if	defined	__i386__

	push	%ebp
	movl	%esp, %ebp
	push	%ebx					// ibuf
	push	%edi					// obuf

	#define	sp	%esp

#else	// __x86_64__

	push	%rbp
	mov		%rsp, %rbp
	push	%rbx
	push	%r13
	push	%r14
	push	%r15

	#define	sp	%rsp

#endif


	// if kernel, allocate stack space to save xmm registers
#ifdef	KERNEL
#if defined __i386__
	sub		$(8*16), %esp
#else
	sub		$(16*16), %rsp
#endif
	movaps	%xmm0, (sp)
	movaps	%xmm1, 16(sp)
	movaps	%xmm2, 32(sp)
	movaps	%xmm3, 48(sp)
	movaps	%xmm4, 64(sp)
	movaps	%xmm5, 80(sp)
	movaps	%xmm6, 96(sp)
	movaps	%xmm7, 112(sp)
#if defined	__x86_64__
	movaps	%xmm8, 16*8(sp)
	movaps	%xmm9, 16*9(sp)
	movaps	%xmm10, 16*10(sp)
	movaps	%xmm11, 16*11(sp)
	movaps	%xmm12, 16*12(sp)
	movaps	%xmm13, 16*13(sp)
	movaps	%xmm14, 16*14(sp)
	movaps	%xmm15, 16*15(sp)
#endif	// __x86_64__
#endif

	#undef	iv
	#define	iv	%xmm0

#if defined	__i386__
	mov		12(%ebp), %eax			// &iv[0]
	mov		24(%ebp), %edx			// ctx
	movups	(%eax), iv				// iv = in_iv	
	mov		8(%ebp), %ebx			// ibuf
	mov		16(%ebp), %ecx			// num_blk
	mov		20(%ebp), %edi			// obuf

	#define	ibuf	%ebx
	#define	obuf	%edi
	#define num_blk	%ecx	
	#define	ctx		%edx

#else	//	__x86_64__, rdi/rsi/rdx/rcx/r8

	mov		%rdi, %rbx				// ibuf
	movups	(%rsi), iv				// iv = in_iv
	mov		%rdx, %r13				// num_blk
	mov		%rcx, %r14				// obuf
	mov		%r8, %r15				// ctx	

	#define	ibuf	%rbx
	#define	num_blk	%r13d
	#define	obuf	%r14	
	#define	ctx		%r15

#endif

	mov		240(ctx), %eax			// aes length
	cmp		$160, %eax				// aes-128 decrypt
	je		L_decrypt_128
	cmp		$192, %eax				// aes-192 decrypt
	je		L_decrypt_192
	cmp		$224, %eax				// aes-256 decrypt
	je		L_decrypt_256

	mov		$-1, %eax				// wrong aes length, to return -1
	jmp		L_error					// early exit due to wrong aes length


	//
	// aes-128 decrypt_cbc operation, after completion, branch to L_HW_cbc_done
	//

L_decrypt_128:

	cmp		$1, num_blk
	jl		L_HW_cbc_done			// if num_blk < 1, early return

	// aes-128 decrypt expanded keys
	movups	160(ctx), %xmm3
	movups	144(ctx), %xmm4
	movups	128(ctx), %xmm5
	movups	112(ctx), %xmm6
	movups	96(ctx), %xmm7
#if defined	__x86_64__
	movups	80(ctx), %xmm8
	movups	64(ctx), %xmm9
	movups	48(ctx), %xmm10
	movups	32(ctx), %xmm11
	movups	16(ctx), %xmm12
	movups	0(ctx), %xmm13
#endif

	// performs 4 block decryption in an iteration to exploit decrypt in parallel

	//		while ((num_blk-=4)>=0) {
	//			aes_decrypt(ibuf, obuf, ctx);
	//			aes_decrypt(ibuf+1, obuf+1, ctx);
	//			aes_decrypt(ibuf+2, obuf+2, ctx);
	//			aes_decrypt(ibuf+3, obuf+3, ctx);
	//			obuf[0] ^= *iv; obuf[1] ^= ibuf[1]; obuf[2] ^= ibuf[1]; obuf[3] ^= ibuf[2];
	//			*iv = ibuf[3]; ibuf += 4; obuf += 4;
	//		}

	sub		$4, num_blk					// pre decrement num_blk by 4
	jl		9f							// if num_blk < 4, skip the per-4-blocks processing code

0:


#if defined	__x86_64__

	movups	(ibuf), %xmm1				// tmp = 1st ibuf
	movups	16(ibuf), %xmm2				// tmp = 2nd ibuf
	movups	32(ibuf), %xmm14			// tmp = 3rd ibuf
	movups	48(ibuf), %xmm15			// tmp = 4th ibuf

	// for x86_64, the expanded keys are already stored in xmm3-xmm13

	// aes-128 decrypt round 0 per 4 blocks
	pxor    %xmm3, %xmm1
	pxor    %xmm3, %xmm2
	pxor    %xmm3, %xmm14
	pxor    %xmm3, %xmm15

	// aes-128 decrypt round 1 per 4 blocks
    aesdec  %xmm4, %xmm1
    aesdec  %xmm4, %xmm2
    aesdec  %xmm4, %xmm14
    aesdec  %xmm4, %xmm15

	// aes-128 decrypt round 2 per 4 blocks
    aesdec  %xmm5, %xmm1
    aesdec  %xmm5, %xmm2
    aesdec  %xmm5, %xmm14
    aesdec  %xmm5, %xmm15

	// aes-128 decrypt round 3 per 4 blocks
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm14
    aesdec  %xmm6, %xmm15

	// aes-128 decrypt round 4 per 4 blocks
    aesdec  %xmm7, %xmm1
    aesdec  %xmm7, %xmm2
    aesdec  %xmm7, %xmm14
    aesdec  %xmm7, %xmm15

	// aes-128 decrypt round 5 per 4 blocks
    aesdec  %xmm8, %xmm1
    aesdec  %xmm8, %xmm2
    aesdec  %xmm8, %xmm14
    aesdec  %xmm8, %xmm15

	// aes-128 decrypt round 6 per 4 blocks
    aesdec  %xmm9, %xmm1
    aesdec  %xmm9, %xmm2
    aesdec  %xmm9, %xmm14
    aesdec  %xmm9, %xmm15

	// aes-128 decrypt round 7 per 4 blocks
    aesdec  %xmm10, %xmm1
    aesdec  %xmm10, %xmm2
    aesdec  %xmm10, %xmm14
    aesdec  %xmm10, %xmm15

	// aes-128 decrypt round 8 per 4 blocks
    aesdec  %xmm11, %xmm1
    aesdec  %xmm11, %xmm2
    aesdec  %xmm11, %xmm14
    aesdec  %xmm11, %xmm15

	// aes-128 decrypt round 9 per 4 blocks
    aesdec  %xmm12, %xmm1
    aesdec  %xmm12, %xmm2
    aesdec  %xmm12, %xmm14
    aesdec  %xmm12, %xmm15

	// aes-128 decrypt round 10 (last) per 4 blocks
    aesdeclast  %xmm13, %xmm1
    aesdeclast  %xmm13, %xmm2
    aesdeclast  %xmm13, %xmm14
    aesdeclast  %xmm13, %xmm15

	pxor	iv, %xmm1				// obuf[0] ^= *iv; 
	movups	(ibuf), iv				// ibuf[0]
	pxor	iv, %xmm2				// obuf[1] ^= ibuf[0]; 
	movups	16(ibuf), iv			// ibuf[1]
	pxor	iv, %xmm14				// obuf[2] ^= ibuf[1]; 
	movups	32(ibuf), iv			// ibuf[2] 
	pxor	iv, %xmm15				// obuf[3] ^= obuf[2]; 
	movups	48(ibuf), iv			// *iv = ibuf[3]

	movups	%xmm1, (obuf)			// write 1st obuf
	movups	%xmm2, 16(obuf)			// write 2nd obuf
	movups	%xmm14, 32(obuf)		// write 3rd obuf
	movups	%xmm15, 48(obuf)		// write 4th obuf


#else

	// aes_decrypt_cbc per 4 blocks using aes-128 for i386
	// xmm1/xmm2/xmm4/xmm5 used for obuf per block
	// xmm3 = key0
	// xmm0 = iv
	// xmm6/xmm7 dynamically load with other expanded keys

	movups	(ibuf), %xmm1			// tmp = 1st ibuf
	movups	16(ibuf), %xmm2			// tmp = 2nd ibuf
	movups	32(ibuf), %xmm4			// tmp = 3rd ibuf
	movups	48(ibuf), %xmm5			// tmp = 4th ibuf

	// aes_decrypt
	// for i386, sequentially load expanded keys into xmm6/xmm7

	movups	144(ctx), %xmm6			// key1

	// aes-128 decrypt round 0 per 4 blocks
	pxor    %xmm3, %xmm1
	pxor    %xmm3, %xmm2
	pxor    %xmm3, %xmm4
	pxor    %xmm3, %xmm5

	movups	128(ctx), %xmm7			// key2

	// aes-128 decrypt round 1 per 4 blocks
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm4
    aesdec  %xmm6, %xmm5

	movups	112(ctx), %xmm6			// key3

	// aes-128 decrypt round 2 per 4 blocks
    aesdec  %xmm7, %xmm1
    aesdec  %xmm7, %xmm2
    aesdec  %xmm7, %xmm4
    aesdec  %xmm7, %xmm5

	movups	96(ctx), %xmm7			// key4

	// aes-128 decrypt round 3 per 4 blocks
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm4
    aesdec  %xmm6, %xmm5

	movups	80(ctx), %xmm6			// key5

	// aes-128 decrypt round 4 per 4 blocks
    aesdec  %xmm7, %xmm1
    aesdec  %xmm7, %xmm2
    aesdec  %xmm7, %xmm4
    aesdec  %xmm7, %xmm5

	movups	64(ctx), %xmm7			// key6

	// aes-128 decrypt round 5 per 4 blocks
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm4
    aesdec  %xmm6, %xmm5

	movups	48(ctx), %xmm6			// key7

	// aes-128 decrypt round 6 per 4 blocks
    aesdec  %xmm7, %xmm1
    aesdec  %xmm7, %xmm2
    aesdec  %xmm7, %xmm4
    aesdec  %xmm7, %xmm5

	movups	32(ctx), %xmm7			// key8

	// aes-128 decrypt round 7 per 4 blocks
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm4
    aesdec  %xmm6, %xmm5

	movups	16(ctx), %xmm6			// key9

	// aes-128 decrypt round 8 per 4 blocks
    aesdec  %xmm7, %xmm1
    aesdec  %xmm7, %xmm2
    aesdec  %xmm7, %xmm4
    aesdec  %xmm7, %xmm5

	movups	0(ctx), %xmm7			// keyA

	// aes-128 decrypt round 9 per 4 blocks
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm4
    aesdec  %xmm6, %xmm5

	// aes-128 decrypt round 10 (last) per 4 blocks
    aesdeclast  %xmm7, %xmm1
    aesdeclast  %xmm7, %xmm2
    aesdeclast  %xmm7, %xmm4
    aesdeclast  %xmm7, %xmm5

	pxor	iv, %xmm1				// 1st obuf ^= iv; 
	movups	(ibuf), iv				// 1st memcpy(iv, tmp, AES_BLOCK_SIZE);
	pxor	iv, %xmm2				// 2nd obuf ^= iv; 
	movups	16(ibuf), iv			// 2nd memcpy(iv, tmp, AES_BLOCK_SIZE);
	pxor	iv, %xmm4				// 3rd obuf ^= iv; 
	movups	32(ibuf), iv			// 3rd memcpy(iv, tmp, AES_BLOCK_SIZE);
	pxor	iv, %xmm5				// 4th obuf ^= iv; 
	movups	48(ibuf), iv			// 4th memcpy(iv, tmp, AES_BLOCK_SIZE);

	movups	%xmm1, (obuf)			// write 1st obuf
	movups	%xmm2, 16(obuf)			// write 2nd obuf
	movups	%xmm4, 32(obuf)			// write 3rd obuf
	movups	%xmm5, 48(obuf)			// write 4th obuf
#endif

	add		$64, ibuf				// ibuf += 4; 
	add		$64, obuf				// obuf += 4;	

	sub		$4, num_blk				// num_blk -= 4
	jge		0b						// if num_blk > 0, repeat the loop

9:	add		$4, num_blk				// post incremtn num_blk by 4
	je		L_HW_cbc_done			// if num_blk == 0, no need for forthur processing code

#if defined	__i386__
	// updated as they might be needed as expanded keys in the remaining
	movups	144(ctx), %xmm4
	movups	128(ctx), %xmm5
	movups	112(ctx), %xmm6
	movups	96(ctx), %xmm7
#endif

	test	$2, num_blk				// check whether num_blk has 2 blocks
	je		9f						// if num_blk & 2 == 0, skip the per-pair processing code

	// do the remaining 2 blocks together

	movups	(ibuf), %xmm1				// tmp = 1st ibuf
	movups	16(ibuf), %xmm2				// tmp = 2nd ibuf

	// aes_decrypt
	pxor    %xmm3, %xmm1
	pxor    %xmm3, %xmm2
    aesdec  %xmm4, %xmm1
    aesdec  %xmm4, %xmm2
    aesdec  %xmm5, %xmm1
    aesdec  %xmm5, %xmm2
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
#if defined	__x86_64__
    aesdec  %xmm7, %xmm1
    aesdec  %xmm7, %xmm2
    aesdec  %xmm8, %xmm1
    aesdec  %xmm8, %xmm2
    aesdec  %xmm9, %xmm1
    aesdec  %xmm9, %xmm2
    aesdec  %xmm10, %xmm1
    aesdec  %xmm10, %xmm2
    aesdec  %xmm11, %xmm1
    aesdec  %xmm11, %xmm2
    aesdec  %xmm12, %xmm1
    aesdec  %xmm12, %xmm2
    aesdeclast  %xmm13, %xmm1
    aesdeclast  %xmm13, %xmm2
#else
	movups	80(ctx), %xmm6
    aesdec  %xmm7, %xmm1
    aesdec  %xmm7, %xmm2
	movups	64(ctx), %xmm7
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
	movups	48(ctx), %xmm6
    aesdec  %xmm7, %xmm1
    aesdec  %xmm7, %xmm2
	movups	32(ctx), %xmm7
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
	movups	16(ctx), %xmm6
    aesdec  %xmm7, %xmm1
    aesdec  %xmm7, %xmm2
	movups	0(ctx), %xmm7
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdeclast  %xmm7, %xmm1
    aesdeclast  %xmm7, %xmm2
	movups	112(ctx), %xmm6
	movups	96(ctx), %xmm7
#endif

	pxor	iv, %xmm1				// obuf[0] ^= *iv; 
	movups	(ibuf), iv				// ibuf[0]
	pxor	iv, %xmm2				// obuf[1] ^= ibuf[0]
	movups	16(ibuf), iv			// *iv = ibuf[1]

	movups	%xmm1, (obuf)			// write obuf[0]
	movups	%xmm2, 16(obuf)			// write obuf[1]

	add		$32, ibuf				// ibuf += 2
	add		$32, obuf				// obuf += 2

9:
	test	$1, num_blk				// check whether num_blk has residual 1 block
	je		L_HW_cbc_done			// if num_blk == 0, no need for residual processing code
	
	movups	(ibuf), %xmm2				// tmp = ibuf
	// aes_decrypt
	pxor    %xmm3, %xmm2
    aesdec  %xmm4, %xmm2
    aesdec  %xmm5, %xmm2
    aesdec  %xmm6, %xmm2
    aesdec  %xmm7, %xmm2
#if defined	__x86_64__
    aesdec  %xmm8, %xmm2
    aesdec  %xmm9, %xmm2
    aesdec  %xmm10, %xmm2
    aesdec  %xmm11, %xmm2
    aesdec  %xmm12, %xmm2
    aesdeclast  %xmm13, %xmm2
#else
	movups	80(ctx), %xmm1
    aesdec  %xmm1, %xmm2
	movups	64(ctx), %xmm1
    aesdec  %xmm1, %xmm2
	movups	48(ctx), %xmm1
    aesdec  %xmm1, %xmm2
	movups	32(ctx), %xmm1
    aesdec  %xmm1, %xmm2
	movups	16(ctx), %xmm1
    aesdec  %xmm1, %xmm2
	movups	(ctx), %xmm1
    aesdeclast  %xmm1, %xmm2
#endif

	pxor	iv, %xmm2			// *obuf ^= *iv; 
	movups	(ibuf), iv			// *iv = *ibuf;
	movups	%xmm2, (obuf)		// write *obuf

	jmp		L_HW_cbc_done

	//
	// aes-192 decrypt_cbc operation, after completion, branch to L_HW_cbc_done
	//

L_decrypt_192:

	cmp		$1, num_blk
	jl		L_HW_cbc_done			// if num_blk < 1, early return

	// aes-192 decryp expanded keys
	movups	192(ctx), %xmm3
	movups	176(ctx), %xmm4
	movups	160(ctx), %xmm5
	movups	144(ctx), %xmm6
	movups	128(ctx), %xmm7
#if defined	__x86_64__
	movups	112(ctx), %xmm8
	movups	96(ctx), %xmm9
	movups	80(ctx), %xmm10
	movups	64(ctx), %xmm11
	movups	48(ctx), %xmm12
	movups	32(ctx), %xmm13
	movups	16(ctx), %xmm14
	movups	(ctx), %xmm15
#endif

	// performs 4 block decryption in an iteration to exploit decrypt in parallel

	//		while ((num_blk-=4)>=0) {
	//			aes_decrypt(ibuf, obuf, ctx);
	//			aes_decrypt(ibuf+1, obuf+1, ctx);
	//			aes_decrypt(ibuf+2, obuf+2, ctx);
	//			aes_decrypt(ibuf+3, obuf+3, ctx);
	//			obuf[0] ^= *iv; obuf[1] ^= ibuf[1]; obuf[2] ^= ibuf[1]; obuf[3] ^= ibuf[2];
	//			*iv = ibuf[3]; ibuf += 4; obuf += 4;
	//		}

	sub		$4, num_blk					// pre decrement num_blk by 4
	jl		9f							// if num_blk < 4, skip the per-4-blocks processing code
0:

#if defined	__x86_64__

	movups	(ibuf), %xmm1				// tmp = 1st ibuf
	movups	16(ibuf), %xmm2				// tmp = 2nd ibuf
	movups	32(ibuf), %xmm14			// tmp = 3rd ibuf
	movups	48(ibuf), %xmm15			// tmp = 4th ibuf

	// aes_decrypt, for x86_64, the expanded keys are already stored in xmm3-xmm13
	// use %xmm12/%xmm13 ts dynamic keys in the middle, restored afterwards

	// round 0 for 4 blocks
	pxor    %xmm3, %xmm1
	pxor    %xmm3, %xmm2
	pxor    %xmm3, %xmm14
	pxor    %xmm3, %xmm15

	// round 1 for 4 blocks
    aesdec  %xmm4, %xmm1
    aesdec  %xmm4, %xmm2
    aesdec  %xmm4, %xmm14
    aesdec  %xmm4, %xmm15

	// round 2 for 4 blocks
    aesdec  %xmm5, %xmm1
    aesdec  %xmm5, %xmm2
    aesdec  %xmm5, %xmm14
    aesdec  %xmm5, %xmm15

	// round 3 for 4 blocks
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm14
    aesdec  %xmm6, %xmm15

	// round 4 for 4 blocks
    aesdec  %xmm7, %xmm1
    aesdec  %xmm7, %xmm2
    aesdec  %xmm7, %xmm14
    aesdec  %xmm7, %xmm15

	// round 5 for 4 blocks
    aesdec  %xmm8, %xmm1
    aesdec  %xmm8, %xmm2
    aesdec  %xmm8, %xmm14
    aesdec  %xmm8, %xmm15

	// round 6 for 4 blocks
    aesdec  %xmm9, %xmm1
    aesdec  %xmm9, %xmm2
    aesdec  %xmm9, %xmm14
    aesdec  %xmm9, %xmm15

	// round 7 for 4 blocks
    aesdec  %xmm10, %xmm1
    aesdec  %xmm10, %xmm2
    aesdec  %xmm10, %xmm14
    aesdec  %xmm10, %xmm15

	// round 8 for 4 blocks
    aesdec  %xmm11, %xmm1
    aesdec  %xmm11, %xmm2
    aesdec  %xmm11, %xmm14
    aesdec  %xmm11, %xmm15

	// round 9 for 4 blocks
    aesdec  %xmm12, %xmm1
    aesdec  %xmm12, %xmm2
    aesdec  %xmm12, %xmm14
    aesdec  %xmm12, %xmm15

	movups	16(ctx), %xmm12

	// round A for 4 blocks
    aesdec  %xmm13, %xmm1
    aesdec  %xmm13, %xmm2
    aesdec  %xmm13, %xmm14
    aesdec  %xmm13, %xmm15

	movups	(ctx), %xmm13

	// round B for 4 blocks
    aesdec  %xmm12, %xmm1
    aesdec  %xmm12, %xmm2
    aesdec  %xmm12, %xmm14
    aesdec  %xmm12, %xmm15

	movups	48(ctx), %xmm12		// restore %xmm12 to its original key

	// round C (last) for 4 blocks
    aesdeclast  %xmm13, %xmm1
    aesdeclast  %xmm13, %xmm2
    aesdeclast  %xmm13, %xmm14
    aesdeclast  %xmm13, %xmm15

	movups	32(ctx), %xmm13		// restore %xmm13 to its original key

	pxor	iv, %xmm1				// obuf[0] ^= *iv; 
	movups	(ibuf), iv				// ibuf[0]
	pxor	iv, %xmm2				// obuf[1] ^= ibuf[0] 
	movups	16(ibuf), iv			// ibuf[1]
	pxor	iv, %xmm14				// obuf[2] ^= ibuf[1] 
	movups	32(ibuf), iv			// ibuf[2] 
	pxor	iv, %xmm15				// obuf[3] ^= ibuf[2] 
	movups	48(ibuf), iv			// *iv = ibuf[3] 

	movups	%xmm1, (obuf)			// write 1st obuf
	movups	%xmm2, 16(obuf)			// write 2nd obuf
	movups	%xmm14, 32(obuf)		// write 3rd obuf
	movups	%xmm15, 48(obuf)		// write 4th obuf

	add		$64, ibuf				// ibuf += 4; 
	add		$64, obuf				// obuf += 4;	

	sub		$4, num_blk				// num_blk -= 4
	jge		0b						// if num_blk > 0, repeat the loop

9:	add		$4, num_blk				// post incremtn num_blk by 4
	je		L_HW_cbc_done			// if num_blk == 0, prepare to return 

	movups	16(ctx), %xmm14			// restore %xmm14 to its key
	movups	(ctx), %xmm15			// restore %xmm15 to its key

#else

	movups	(ibuf), %xmm1			// tmp = 1st ibuf
	movups	16(ibuf), %xmm2			// tmp = 2nd ibuf
	movups	32(ibuf), %xmm4			// tmp = 3rd ibuf
	movups	48(ibuf), %xmm5			// tmp = 4th ibuf

	// aes_decrypt
	// for i386, sequentially load expanded keys into xmm6/xmm7
	movups	176(ctx), %xmm6
	pxor    %xmm3, %xmm1
	pxor    %xmm3, %xmm2
	pxor    %xmm3, %xmm4
	pxor    %xmm3, %xmm5

	movups	160(ctx), %xmm7
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm4
    aesdec  %xmm6, %xmm5

	movups	144(ctx), %xmm6
	aesdec    %xmm7, %xmm1
	aesdec    %xmm7, %xmm2
	aesdec    %xmm7, %xmm4
	aesdec    %xmm7, %xmm5

	movups	128(ctx), %xmm7
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm4
    aesdec  %xmm6, %xmm5

	movups	112(ctx), %xmm6
    aesdec  %xmm7, %xmm1
    aesdec  %xmm7, %xmm2
    aesdec  %xmm7, %xmm4
    aesdec  %xmm7, %xmm5

	movups	96(ctx), %xmm7
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm4
    aesdec  %xmm6, %xmm5

	movups	80(ctx), %xmm6
    aesdec  %xmm7, %xmm1
    aesdec  %xmm7, %xmm2
    aesdec  %xmm7, %xmm4
    aesdec  %xmm7, %xmm5

	movups	64(ctx), %xmm7
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm4
    aesdec  %xmm6, %xmm5

	movups	48(ctx), %xmm6
    aesdec  %xmm7, %xmm1
    aesdec  %xmm7, %xmm2
    aesdec  %xmm7, %xmm4
    aesdec  %xmm7, %xmm5

	movups	32(ctx), %xmm7
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm4
    aesdec  %xmm6, %xmm5

	movups	16(ctx), %xmm6
    aesdec  %xmm7, %xmm1
    aesdec  %xmm7, %xmm2
    aesdec  %xmm7, %xmm4
    aesdec  %xmm7, %xmm5

	movups	0(ctx), %xmm7
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm4
    aesdec  %xmm6, %xmm5

    aesdeclast  %xmm7, %xmm1
    aesdeclast  %xmm7, %xmm2
    aesdeclast  %xmm7, %xmm4
    aesdeclast  %xmm7, %xmm5

	pxor	iv, %xmm1				// 1st obuf ^= iv; 
	movups	(ibuf), iv				// 1st memcpy(iv, tmp, AES_BLOCK_SIZE);
	pxor	iv, %xmm2				// 2nd obuf ^= iv; 
	movups	16(ibuf), iv			// 2nd memcpy(iv, tmp, AES_BLOCK_SIZE);
	pxor	iv, %xmm4				// 3rd obuf ^= iv; 
	movups	32(ibuf), iv			// 3rd memcpy(iv, tmp, AES_BLOCK_SIZE);
	pxor	iv, %xmm5				// 4th obuf ^= iv; 
	movups	48(ibuf), iv			// 4th memcpy(iv, tmp, AES_BLOCK_SIZE);
	movups	%xmm1, (obuf)			// write 1st obuf
	movups	%xmm2, 16(obuf)			// write 2nd obuf
	movups	%xmm4, 32(obuf)			// write 3rd obuf
	movups	%xmm5, 48(obuf)			// write 4th obuf

	add		$64, ibuf				// ibuf += AES_BLOCK_SIZE * 4; 
	add		$64, obuf				// obuf += AES_BLOCK_SIZE * 4;	

	sub		$4, num_blk				// num_blk -= 4
	jge		0b						// if num_blk > 0, repeat the loop


9:	add		$4, num_blk				//	post incremtn num_blk by 4
	je		L_HW_cbc_done			// if num_blk == 0, no need for forthur processing code

	movups	176(ctx), %xmm4
	movups	160(ctx), %xmm5
	movups	144(ctx), %xmm6
	movups	128(ctx), %xmm7

#endif

	// per-block aes_decrypt_cbc loop

0:
	movups	(ibuf), %xmm2				// tmp = ibuf

	// aes_decrypt
	pxor    %xmm3, %xmm2
    aesdec  %xmm4, %xmm2
    aesdec  %xmm5, %xmm2
    aesdec  %xmm6, %xmm2
    aesdec  %xmm7, %xmm2
#if defined	__x86_64__
    aesdec  %xmm8, %xmm2
    aesdec  %xmm9, %xmm2
    aesdec  %xmm10, %xmm2
    aesdec  %xmm11, %xmm2
    aesdec  %xmm12, %xmm2
    aesdec  %xmm13, %xmm2
    aesdec  %xmm14, %xmm2
    aesdeclast  %xmm15, %xmm2
#else
	movups	112(ctx), %xmm1
    aesdec  %xmm1, %xmm2
	movups	96(ctx), %xmm1
    aesdec  %xmm1, %xmm2
	movups	80(ctx), %xmm1
    aesdec  %xmm1, %xmm2
	movups	64(ctx), %xmm1
    aesdec  %xmm1, %xmm2
	movups	48(ctx), %xmm1
    aesdec  %xmm1, %xmm2
	movups	32(ctx), %xmm1
    aesdec  %xmm1, %xmm2
	movups	16(ctx), %xmm1
    aesdec  %xmm1, %xmm2
	movups	(ctx), %xmm1
    aesdeclast  %xmm1, %xmm2
#endif

	pxor	iv, %xmm2			// obuf ^= iv; 
	movups	(ibuf), iv			// memcpy(iv, tmp, AES_BLOCK_SIZE);

	movups	%xmm2, (obuf)		// write obuf

	add		$16, ibuf				// ibuf += AES_BLOCK_SIZE; 
	add		$16, obuf				// obuf += AES_BLOCK_SIZE;	
	sub		$1, num_blk				// num_blk --
	jg		0b						// if num_blk > 0, repeat the loop

	jmp		L_HW_cbc_done

	//
	// aes-256 decrypt_cbc operation, after completion, branch to L_HW_cbc_done
	//

L_decrypt_256:

	cmp		$1, num_blk
	jl		L_HW_cbc_done	

	movups	224(ctx), %xmm3
	movups	208(ctx), %xmm4
	movups	192(ctx), %xmm5
	movups	176(ctx), %xmm6
	movups	160(ctx), %xmm7
#if defined	__x86_64__
	movups	144(ctx), %xmm8
	movups	128(ctx), %xmm9
	movups	112(ctx), %xmm10
	movups	96(ctx), %xmm11
	movups	80(ctx), %xmm12
	movups	64(ctx), %xmm13
	movups	48(ctx), %xmm14
	movups	32(ctx), %xmm15
//	movups	16(ctx), %xmm14
//	movups	(ctx), %xmm15
#endif

#if defined	__x86_64__

	sub		$4, num_blk					// pre decrement num_blk by 4
	jl		9f							// if num_blk < 4, skip the per-4-blocks processing code
0:
	movups	(ibuf), %xmm1				// tmp = 1st ibuf
	movups	16(ibuf), %xmm2				// tmp = 2nd ibuf
	movups	32(ibuf), %xmm14			// tmp = 3rd ibuf
	movups	48(ibuf), %xmm15			// tmp = 4th ibuf

	// aes_decrypt, for x86_64, the expanded keys are already stored in xmm3-xmm13
	pxor    %xmm3, %xmm1
	pxor    %xmm3, %xmm2
	pxor    %xmm3, %xmm14
	pxor    %xmm3, %xmm15

    aesdec  %xmm4, %xmm1
    aesdec  %xmm4, %xmm2
    aesdec  %xmm4, %xmm14
    aesdec  %xmm4, %xmm15

    aesdec  %xmm5, %xmm1
    aesdec  %xmm5, %xmm2
    aesdec  %xmm5, %xmm14
    aesdec  %xmm5, %xmm15

    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm14
    aesdec  %xmm6, %xmm15

    aesdec  %xmm7, %xmm1
    aesdec  %xmm7, %xmm2
    aesdec  %xmm7, %xmm14
    aesdec  %xmm7, %xmm15

    aesdec  %xmm8, %xmm1
    aesdec  %xmm8, %xmm2
    aesdec  %xmm8, %xmm14
    aesdec  %xmm8, %xmm15

    aesdec  %xmm9, %xmm1
    aesdec  %xmm9, %xmm2
    aesdec  %xmm9, %xmm14
    aesdec  %xmm9, %xmm15

    aesdec  %xmm10, %xmm1
    aesdec  %xmm10, %xmm2
    aesdec  %xmm10, %xmm14
    aesdec  %xmm10, %xmm15

    aesdec  %xmm11, %xmm1
    aesdec  %xmm11, %xmm2
    aesdec  %xmm11, %xmm14
    aesdec  %xmm11, %xmm15

    aesdec  %xmm12, %xmm1
    aesdec  %xmm12, %xmm2
    aesdec  %xmm12, %xmm14
    aesdec  %xmm12, %xmm15
	movups	48(ctx), %xmm12

    aesdec  %xmm13, %xmm1
    aesdec  %xmm13, %xmm2
    aesdec  %xmm13, %xmm14
    aesdec  %xmm13, %xmm15
	movups	32(ctx), %xmm13

    aesdec  %xmm12, %xmm1
    aesdec  %xmm12, %xmm2
    aesdec  %xmm12, %xmm14
    aesdec  %xmm12, %xmm15
	movups	16(ctx), %xmm12

    aesdec  %xmm13, %xmm1
    aesdec  %xmm13, %xmm2
    aesdec  %xmm13, %xmm14
    aesdec  %xmm13, %xmm15
	movups	(ctx), %xmm13

    aesdec  %xmm12, %xmm1
    aesdec  %xmm12, %xmm2
    aesdec  %xmm12, %xmm14
    aesdec  %xmm12, %xmm15
	movups	80(ctx), %xmm12

    aesdeclast  %xmm13, %xmm1
    aesdeclast  %xmm13, %xmm2
    aesdeclast  %xmm13, %xmm14
    aesdeclast  %xmm13, %xmm15
	movups	64(ctx), %xmm13

	pxor	iv, %xmm1				// obuf ^= iv; 
	movups	(ibuf), iv				// memcpy(iv, tmp, AES_BLOCK_SIZE);
	pxor	iv, %xmm2				// obuf ^= iv; 
	movups	16(ibuf), iv			// memcpy(iv, tmp, AES_BLOCK_SIZE);
	pxor	iv, %xmm14				// obuf ^= iv; 
	movups	32(ibuf), iv			// memcpy(iv, tmp, AES_BLOCK_SIZE);
	pxor	iv, %xmm15				// obuf ^= iv; 
	movups	48(ibuf), iv			// memcpy(iv, tmp, AES_BLOCK_SIZE);

	movups	%xmm1, (obuf)			// write 1st obuf
	movups	%xmm2, 16(obuf)			// write 2nd obuf
	movups	%xmm14, 32(obuf)		// write 3rd obuf
	movups	%xmm15, 48(obuf)		// write 4th obuf

	add		$64, ibuf				// ibuf += AES_BLOCK_SIZE*4; 
	add		$64, obuf				// obuf += AES_BLOCK_SIZE*4;	

	sub		$4, num_blk				// num_blk -= 4
	jge		0b						// if num_blk > 0, repeat the loop

9:	add		$4, num_blk				//	post incremtn num_blk by 4
	je		L_HW_cbc_done			// if num_blk == 0, no need for forthur processing code

	movups	48(ctx), %xmm14
	movups	32(ctx), %xmm15

#else

	sub		$4, num_blk				// pre decrement num_blk by 4
	jl		9f						// if num_blk < 4, skip the per-pair processing code
0:
	movups	(ibuf), %xmm1			// tmp = 1st ibuf
	movups	16(ibuf), %xmm2			// tmp = 2nd ibuf
	movups	32(ibuf), %xmm4			// tmp = 3rd ibuf
	movups	48(ibuf), %xmm5			// tmp = 4th ibuf

	// aes_decrypt
	// for i386, sequentially load expanded keys into xmm6/xmm7
	movups	208(ctx), %xmm6
	pxor    %xmm3, %xmm1
	pxor    %xmm3, %xmm2
	pxor    %xmm3, %xmm4
	pxor    %xmm3, %xmm5

	movups	192(ctx), %xmm7
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm4
    aesdec  %xmm6, %xmm5

	movups	176(ctx), %xmm6
	aesdec  %xmm7, %xmm1
	aesdec	%xmm7, %xmm2
	aesdec	%xmm7, %xmm4
	aesdec	%xmm7, %xmm5

	movups	160(ctx), %xmm7
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm4
    aesdec  %xmm6, %xmm5

	movups	144(ctx), %xmm6
	aesdec	%xmm7, %xmm1
	aesdec	%xmm7, %xmm2
	aesdec	%xmm7, %xmm4
	aesdec	%xmm7, %xmm5

	movups	128(ctx), %xmm7
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm4
    aesdec  %xmm6, %xmm5

	movups	112(ctx), %xmm6
    aesdec  %xmm7, %xmm1
    aesdec  %xmm7, %xmm2
    aesdec  %xmm7, %xmm4
    aesdec  %xmm7, %xmm5

	movups	96(ctx), %xmm7
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm4
    aesdec  %xmm6, %xmm5

	movups	80(ctx), %xmm6
    aesdec  %xmm7, %xmm1
    aesdec  %xmm7, %xmm2
    aesdec  %xmm7, %xmm4
    aesdec  %xmm7, %xmm5

	movups	64(ctx), %xmm7
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm4
    aesdec  %xmm6, %xmm5

	movups	48(ctx), %xmm6
    aesdec  %xmm7, %xmm1
    aesdec  %xmm7, %xmm2
    aesdec  %xmm7, %xmm4
    aesdec  %xmm7, %xmm5

	movups	32(ctx), %xmm7
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm4
    aesdec  %xmm6, %xmm5

	movups	16(ctx), %xmm6
    aesdec  %xmm7, %xmm1
    aesdec  %xmm7, %xmm2
    aesdec  %xmm7, %xmm4
    aesdec  %xmm7, %xmm5

	movups	0(ctx), %xmm7
    aesdec  %xmm6, %xmm1
    aesdec  %xmm6, %xmm2
    aesdec  %xmm6, %xmm4
    aesdec  %xmm6, %xmm5

    aesdeclast  %xmm7, %xmm1
    aesdeclast  %xmm7, %xmm2
    aesdeclast  %xmm7, %xmm4
    aesdeclast  %xmm7, %xmm5

	pxor	iv, %xmm1				// 1st obuf ^= iv; 
	movups	(ibuf), iv				// 1st memcpy(iv, tmp, AES_BLOCK_SIZE);
	pxor	iv, %xmm2				// 2nd obuf ^= iv; 
	movups	16(ibuf), iv			// 2nd memcpy(iv, tmp, AES_BLOCK_SIZE);
	pxor	iv, %xmm4				// 3rd obuf ^= iv; 
	movups	32(ibuf), iv			// 3rd memcpy(iv, tmp, AES_BLOCK_SIZE);
	pxor	iv, %xmm5				// 4th obuf ^= iv; 
	movups	48(ibuf), iv			// 4th memcpy(iv, tmp, AES_BLOCK_SIZE);
	movups	%xmm1, (obuf)			// write 1st obuf
	movups	%xmm2, 16(obuf)			// write 2nd obuf
	movups	%xmm4, 32(obuf)			// write 3rd obuf
	movups	%xmm5, 48(obuf)			// write 4th obuf

	add		$64, ibuf				// ibuf += AES_BLOCK_SIZE * 4; 
	add		$64, obuf				// obuf += AES_BLOCK_SIZE * 4;	

	sub		$4, num_blk				// num_blk -= 4
	jge		0b						// if num_blk > 0, repeat the loop


9:	add		$4, num_blk				//	post incremtn num_blk by 4
	je		L_HW_cbc_done			// if num_blk == 0, no need for forthur processing code

	movups	208(ctx), %xmm4
	movups	192(ctx), %xmm5
	movups	176(ctx), %xmm6
	movups	160(ctx), %xmm7

#endif

0:
	movups	(ibuf), %xmm2				// tmp = ibuf

	// aes_decrypt
	pxor	%xmm3, %xmm2
    aesdec  %xmm4, %xmm2
    aesdec  %xmm5, %xmm2
    aesdec  %xmm6, %xmm2
    aesdec  %xmm7, %xmm2
#if defined	__x86_64__
    aesdec  %xmm8, %xmm2
    aesdec  %xmm9, %xmm2
    aesdec  %xmm10, %xmm2
    aesdec  %xmm11, %xmm2
    aesdec  %xmm12, %xmm2
    aesdec  %xmm13, %xmm2
    aesdec  %xmm14, %xmm2
    aesdec  %xmm15, %xmm2
#else
	movups	144(ctx), %xmm1
    aesdec  %xmm1, %xmm2
	movups	128(ctx), %xmm1
    aesdec  %xmm1, %xmm2
	movups	112(ctx), %xmm1
    aesdec  %xmm1, %xmm2
	movups	96(ctx), %xmm1
    aesdec  %xmm1, %xmm2
	movups	80(ctx), %xmm1
    aesdec  %xmm1, %xmm2
	movups	64(ctx), %xmm1
    aesdec  %xmm1, %xmm2
	movups	48(ctx), %xmm1
    aesdec  %xmm1, %xmm2
	movups	32(ctx), %xmm1
    aesdec  %xmm1, %xmm2
#endif
	movups	16(ctx), %xmm1
    aesdec  %xmm1, %xmm2
	movups	(ctx), %xmm1
    aesdeclast  %xmm1, %xmm2

	pxor	iv, %xmm2			// obuf ^= iv; 
	movups	(ibuf), iv			// memcpy(iv, tmp, AES_BLOCK_SIZE);

	movups	%xmm2, (obuf)		// write obuf

	add		$16, ibuf				// ibuf += AES_BLOCK_SIZE; 
	add		$16, obuf				// obuf += AES_BLOCK_SIZE;	
	sub		$1, num_blk				// num_blk --
	jg		0b						// if num_blk > 0, repeat the loop

	jmp		L_HW_cbc_done

	//
	// --------- END of aesedp_decrypt_cbc_hw  -------------------
	//
#endif /* x86 based build */
