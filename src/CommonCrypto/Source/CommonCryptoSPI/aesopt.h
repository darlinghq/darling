/*
The bulk of this file is from Dr. Brian Gladman's AES implementation, described
in the comments below.  But some code has been added to select the
implementation.  See comments below, where UseGladmanAES is defined.  The new
code does not alter Gladman's AES implementation except to completely include
or exclude it from compilation.

		-- Eric Postpischil, January 8, 2008.


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
 Issue 28/01/2004

 My thanks go to Dag Arne Osvik for devising the schemes used here for key
 length derivation from the form of the key schedule

 This file contains the compilation options for AES (Rijndael) and code
 that is common across encryption, key scheduling and table generation.

 OPERATION

 These source code files implement the AES algorithm Rijndael designed by
 Joan Daemen and Vincent Rijmen. This version is designed for the standard
 block size of 16 bytes and for key sizes of 128, 192 and 256 bits (16, 24
 and 32 bytes).

 This version is designed for flexibility and speed using operations on
 32-bit words rather than operations on bytes.  It can be compiled with
 either big or little endian internal byte order but is faster when the
 native byte order for the processor is used.

 THE CIPHER INTERFACE

 The cipher interface is implemented as an array of bytes in which lower
 AES bit sequence indexes map to higher numeric significance within bytes.

  aes_08t                 (an unsigned  8-bit type)
  aes_32t                 (an unsigned 32-bit type)
  struct aes_encrypt_ctx  (structure for the cipher encryption context)
  struct aes_decrypt_ctx  (structure for the cipher decryption context)
  aes_rval                the function return type

  C subroutine calls:

  aes_rval aes_encrypt_key128(const unsigned char *key, aes_encrypt_ctx cx[1]);
  aes_rval aes_encrypt_key192(const unsigned char *key, aes_encrypt_ctx cx[1]);
  aes_rval aes_encrypt_key256(const unsigned char *key, aes_encrypt_ctx cx[1]);
  aes_rval aes_encrypt(const unsigned char *in, unsigned char *out,
                                                  const aes_encrypt_ctx cx[1]);

  aes_rval aes_decrypt_key128(const unsigned char *key, aes_decrypt_ctx cx[1]);
  aes_rval aes_decrypt_key192(const unsigned char *key, aes_decrypt_ctx cx[1]);
  aes_rval aes_decrypt_key256(const unsigned char *key, aes_decrypt_ctx cx[1]);
  aes_rval aes_decrypt(const unsigned char *in, unsigned char *out,
                                                  const aes_decrypt_ctx cx[1]);

 IMPORTANT NOTE: If you are using this C interface with dynamic tables make sure that
 you call genTabs() before AES is used so that the tables are initialised.

 C++ aes class subroutines:

     Class AESencrypt  for encryption

      Construtors:
          AESencrypt(void)
          AESencrypt(const unsigned char *key) - 128 bit key
      Members:
          aes_rval key128(const unsigned char *key)
          aes_rval key192(const unsigned char *key)
          aes_rval key256(const unsigned char *key)
          aes_rval encrypt(const unsigned char *in, unsigned char *out) const

      Class AESdecrypt  for encryption
      Construtors:
          AESdecrypt(void)
          AESdecrypt(const unsigned char *key) - 128 bit key
      Members:
          aes_rval key128(const unsigned char *key)
          aes_rval key192(const unsigned char *key)
          aes_rval key256(const unsigned char *key)
          aes_rval decrypt(const unsigned char *in, unsigned char *out) const

    COMPILATION

    The files used to provide AES (Rijndael) are

    a. aes.h for the definitions needed for use in C.
    b. aescpp.h for the definitions needed for use in C++.
    c. aesopt.h for setting compilation options (also includes common code).
    d. aescrypt.c for encryption and decrytpion, or
    e. aeskey.c for key scheduling.
    f. aestab.c for table loading or generation.
    g. aescrypt.asm for encryption and decryption using assembler code.
    h. aescrypt.mmx.asm for encryption and decryption using MMX assembler.

    To compile AES (Rijndael) for use in C code use aes.h and set the
    defines here for the facilities you need (key lengths, encryption
    and/or decryption). Do not define AES_DLL or AES_CPP.  Set the options
    for optimisations and table sizes here.

    To compile AES (Rijndael) for use in in C++ code use aescpp.h but do
    not define AES_DLL

    To compile AES (Rijndael) in C as a Dynamic Link Library DLL) use
    aes.h and include the AES_DLL define.

    CONFIGURATION OPTIONS (here and in aes.h)

    a. set AES_DLL in aes.h if AES (Rijndael) is to be compiled as a DLL
    b. You may need to set PLATFORM_BYTE_ORDER to define the byte order.
    c. If you want the code to run in a specific internal byte order, then
       ALGORITHM_BYTE_ORDER must be set accordingly.
    d. set other configuration options decribed below.
*/

#if !defined( _CC_AESOPT_H )
#define _CC_AESOPT_H

/*	Select which AES implementation to use.  Preprocessor directives decide
	whether to define UseGladmanAES or UseAESedp or, in the future, other
	symbols.  Source files for the implementations contain preprocessor
	directives to compile their code iff the matching symbol is defined.

	The names GladmanAES and AESedp come from the directories containing the
	source code.  (I prefer putting "AES" first and am tempted to renamed
	"GladmanAES" to "AESGladman", since this groups directories by algorithm in
	listings, but I am resisting for the moment.)
*/
#if defined __i386__ || defined __x86_64__
	// On Intel architectures, use AESedp.
	#define	UseAESedp
#else
	// Otherwise, use Gladman AES.
	#define	UseGladmanAES
#endif

/*	Suppressing all source in a module would yield an empty module after
	preprocessing.  GCC allows this, but standard C requires a module to
	contain at least one external declaration.  So here we make an otherwise
	unused declaration that generates no object code.
*/
#if !defined __ASSEMBLER__
	typedef char DummyDeclarationToMakeValidC;
#endif

#include <CommonCrypto/CommonCryptoPriv.h>
#include <CommonCrypto/aes.h>

/*  CONFIGURATION - USE OF DEFINES

    Later in this section there are a number of defines that control the
    operation of the code.  In each section, the purpose of each define is
    explained so that the relevant form can be included or excluded by
    setting either 1's or 0's respectively on the branches of the related
    #if clauses.

    PLATFORM SPECIFIC INCLUDES AND BYTE ORDER IN 32-BIT WORDS

    To obtain the highest speed on processors with 32-bit words, this code
    needs to determine the byte order of the target machine. The following
    block of code is an attempt to capture the most obvious ways in which
    various environemnts define byte order. It may well fail, in which case
    the definitions will need to be set by editing at the points marked
    **** EDIT HERE IF NECESSARY **** below.  My thanks go to Peter Gutmann
    for his assistance with this endian detection nightmare.
*/

#define BRG_LITTLE_ENDIAN   1234 /* byte 0 is least significant (i386) */
#define BRG_BIG_ENDIAN      4321 /* byte 0 is most significant (mc68k) */

#if defined(__GNUC__) || defined(__GNU_LIBRARY__)
#  if defined(__FreeBSD__) || defined(__OpenBSD__)
#    include <sys/endian.h>
#  elif defined( BSD ) && BSD >= 199103
#      include <machine/endian.h>
#  elif defined(__APPLE__)
#    if defined(__BIG_ENDIAN__) && !defined( BIG_ENDIAN )
#      define BIG_ENDIAN
#    elif defined(__LITTLE_ENDIAN__) && !defined( LITTLE_ENDIAN )
#      define LITTLE_ENDIAN
#    endif
#  else
#    include <endian.h>
#    if defined(__BEOS__)
#      include <byteswap.h>
#    endif
#  endif
#endif

#if !defined(PLATFORM_BYTE_ORDER)
#  if defined(LITTLE_ENDIAN) || defined(BIG_ENDIAN)
#    if    defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#    elif !defined(LITTLE_ENDIAN) &&  defined(BIG_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#    elif defined(BYTE_ORDER) && (BYTE_ORDER == LITTLE_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#    elif defined(BYTE_ORDER) && (BYTE_ORDER == BIG_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#    endif
#  elif defined(_LITTLE_ENDIAN) || defined(_BIG_ENDIAN)
#    if    defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#    elif !defined(_LITTLE_ENDIAN) &&  defined(_BIG_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#    elif defined(_BYTE_ORDER) && (_BYTE_ORDER == _LITTLE_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#    elif defined(_BYTE_ORDER) && (_BYTE_ORDER == _BIG_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#   endif
#  elif defined(__LITTLE_ENDIAN__) || defined(__BIG_ENDIAN__)
#    if    defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
#      define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#    elif !defined(__LITTLE_ENDIAN__) &&  defined(__BIG_ENDIAN__)
#      define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#    elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __LITTLE_ENDIAN__)
#      define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#    elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __BIG_ENDIAN__)
#      define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#    endif
#  endif
#endif

/*  if the platform is still unknown, try to find its byte order    */
/*  from commonly used machine defines                              */

#if !defined(PLATFORM_BYTE_ORDER)

#if   defined( __alpha__ ) || defined( __alpha ) || defined( i386 )       || \
      defined( __i386__ )  || defined( _M_I86 )  || defined( _M_IX86 )    || \
      defined( __OS2__ )   || defined( sun386 )  || defined( __TURBOC__ ) || \
      defined( vax )       || defined( vms )     || defined( VMS )        || \
      defined( __VMS )	   || defined(__x86_64__)
#  define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN

#elif defined( AMIGA )    || defined( applec )  || defined( __AS400__ )  || \
      defined( _CRAY )    || defined( __hppa )  || defined( __hp9000 )   || \
      defined( ibm370 )   || defined( mc68000 ) || defined( m68k )       || \
      defined( __MRC__ )  || defined( __MVS__ ) || defined( __MWERKS__ ) || \
      defined( sparc )    || defined( __sparc)  || defined( SYMANTEC_C ) || \
      defined( __TANDEM ) || defined( THINK_C ) || defined( __VMCMS__ )
#  define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN

#elif 0     /* **** EDIT HERE IF NECESSARY **** */
#  define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#elif 0     /* **** EDIT HERE IF NECESSARY **** */
#  define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#else
#  error Please edit aesopt.h (line 234 or 236) to set the platform byte order
#endif

#endif

/*  SOME LOCAL DEFINITIONS  */

#define NO_TABLES              0
#define ONE_TABLE              1
#define FOUR_TABLES            4
#define NONE                   0
#define PARTIAL                1
#define FULL                   2

#if defined(bswap32)
#define aes_sw32    bswap32
#elif defined(bswap_32)
#define aes_sw32    bswap_32
#else
#define brot(x,n)   (((aes_32t)(x) <<  n) | ((aes_32t)(x) >> (32 - n)))
#define aes_sw32(x) ((brot((x),8) & 0x00ff00ff) | (brot((x),24) & 0xff00ff00))
#endif

/*  1. FUNCTIONS REQUIRED

    This implementation provides subroutines for encryption, decryption
    and for setting the three key lengths (separately) for encryption
    and decryption. When the assembler code is not being used the following
    definition blocks allow the selection of the routines that are to be
    included in the compilation.
*/
#if defined( AES_ENCRYPT )
#define ENCRYPTION
#define ENCRYPTION_KEY_SCHEDULE
#endif

#if defined( AES_DECRYPT )
#define DECRYPTION
#define DECRYPTION_KEY_SCHEDULE
#endif

/*  2. ASSEMBLER SUPPORT

    This define (which can be on the command line) enables the use of the
    assembler code routines for encryption and decryption with the C code
    only providing key scheduling
*/
#if 0 && !defined(AES_ASM)
#define AES_ASM
#endif

/*  3. BYTE ORDER WITHIN 32 BIT WORDS

    The fundamental data processing units in Rijndael are 8-bit bytes. The
    input, output and key input are all enumerated arrays of bytes in which
    bytes are numbered starting at zero and increasing to one less than the
    number of bytes in the array in question. This enumeration is only used
    for naming bytes and does not imply any adjacency or order relationship
    from one byte to another. When these inputs and outputs are considered
    as bit sequences, bits 8*n to 8*n+7 of the bit sequence are mapped to
    byte[n] with bit 8n+i in the sequence mapped to bit 7-i within the byte.
    In this implementation bits are numbered from 0 to 7 starting at the
    numerically least significant end of each byte (bit n represents 2^n).

    However, Rijndael can be implemented more efficiently using 32-bit
    words by packing bytes into words so that bytes 4*n to 4*n+3 are placed
    into word[n]. While in principle these bytes can be assembled into words
    in any positions, this implementation only supports the two formats in
    which bytes in adjacent positions within words also have adjacent byte
    numbers. This order is called big-endian if the lowest numbered bytes
    in words have the highest numeric significance and little-endian if the
    opposite applies.

    This code can work in either order irrespective of the order used by the
    machine on which it runs. Normally the internal byte order will be set
    to the order of the processor on which the code is to be run but this
    define can be used to reverse this in special situations

    NOTE: Assembler code versions rely on PLATFORM_BYTE_ORDER being set
*/
#if 1 || defined(AES_ASM)
#define ALGORITHM_BYTE_ORDER PLATFORM_BYTE_ORDER
#elif 0
#define ALGORITHM_BYTE_ORDER BRG_LITTLE_ENDIAN
#elif 0
#define ALGORITHM_BYTE_ORDER BRG_BIG_ENDIAN
#else
#error The algorithm byte order is not defined
#endif

/*  4. FAST INPUT/OUTPUT OPERATIONS.

    On some machines it is possible to improve speed by transferring the
    bytes in the input and output arrays to and from the internal 32-bit
    variables by addressing these arrays as if they are arrays of 32-bit
    words.  On some machines this will always be possible but there may
    be a large performance penalty if the byte arrays are not aligned on
    the normal word boundaries. On other machines this technique will
    lead to memory access errors when such 32-bit word accesses are not
    properly aligned. The option SAFE_IO avoids such problems but will
    often be slower on those machines that support misaligned access
    (especially so if care is taken to align the input  and output byte
    arrays on 32-bit word boundaries). If SAFE_IO is not defined it is
    assumed that access to byte arrays as if they are arrays of 32-bit
    words will not cause problems when such accesses are misaligned.
*/
#if 0 && !defined(_MSC_VER)
#define SAFE_IO
#endif

/*  5. LOOP UNROLLING

    The code for encryption and decrytpion cycles through a number of rounds
    that can be implemented either in a loop or by expanding the code into a
    long sequence of instructions, the latter producing a larger program but
    one that will often be much faster. The latter is called loop unrolling.
    There are also potential speed advantages in expanding two iterations in
    a loop with half the number of iterations, which is called partial loop
    unrolling.  The following options allow partial or full loop unrolling
    to be set independently for encryption and decryption
*/
#if 1
#define ENC_UNROLL  FULL
#elif 0
#define ENC_UNROLL  PARTIAL
#else
#define ENC_UNROLL  NONE
#endif

#if 1
#define DEC_UNROLL  FULL
#elif 0
#define DEC_UNROLL  PARTIAL
#else
#define DEC_UNROLL  NONE
#endif

/*  6. FAST FINITE FIELD OPERATIONS

    If this section is included, tables are used to provide faster finite
    field arithmetic (this has no effect if FIXED_TABLES is defined).
*/
#if 1
#define FF_TABLES
#endif

/*  7. INTERNAL STATE VARIABLE FORMAT

    The internal state of Rijndael is stored in a number of local 32-bit
    word varaibles which can be defined either as an array or as individual
    names variables. Include this section if you want to store these local
    varaibles in arrays. Otherwise individual local variables will be used.
*/
#if 0
#define ARRAYS
#endif

/* In this implementation the columns of the state array are each held in
   32-bit words. The state array can be held in various ways: in an array
   of words, in a number of individual word variables or in a number of
   processor registers. The following define maps a variable name x and
   a column number c to the way the state array variable is to be held.
   The first define below maps the state into an array x[c] whereas the
   second form maps the state into a number of individual variables x0,
   x1, etc.  Another form could map individual state colums to machine
   register names.
*/

#if defined(ARRAYS)
#define s(x,c) x[c]
#else
#define s(x,c) x##c
#endif

/*  8. FIXED OR DYNAMIC TABLES

    When this section is included the tables used by the code are compiled
    statically into the binary file.  Otherwise the subroutine gen_tabs()
    must be called to compute them before the code is first used.
*/
#if 1
#define FIXED_TABLES
#endif

/*  9. TABLE ALIGNMENT

    On some sytsems speed will be improved by aligning the AES large lookup
    tables on particular boundaries. This define should be set to a power of
    two giving the desired alignment. It can be left undefined if alignment
    is not needed.  This option is specific to the Microsft VC++ compiler -
    it seems to sometimes cause trouble for the VC++ version 6 compiler.
*/

#if 0 && defined(_MSC_VER) && (_MSC_VER >= 1300)
#define TABLE_ALIGN 64
#endif

/*  10. INTERNAL TABLE CONFIGURATION

    This cipher proceeds by repeating in a number of cycles known as 'rounds'
    which are implemented by a round function which can optionally be speeded
    up using tables.  The basic tables are each 256 32-bit words, with either
    one or four tables being required for each round function depending on
    how much speed is required. The encryption and decryption round functions
    are different and the last encryption and decrytpion round functions are
    different again making four different round functions in all.

    This means that:
      1. Normal encryption and decryption rounds can each use either 0, 1
         or 4 tables and table spaces of 0, 1024 or 4096 bytes each.
      2. The last encryption and decryption rounds can also use either 0, 1
         or 4 tables and table spaces of 0, 1024 or 4096 bytes each.

    Include or exclude the appropriate definitions below to set the number
    of tables used by this implementation.
*/

#if 1   /* set tables for the normal encryption round */
#define ENC_ROUND   FOUR_TABLES
#elif 0
#define ENC_ROUND   ONE_TABLE
#else
#define ENC_ROUND   NO_TABLES
#endif

#if 1   /* set tables for the last encryption round */
#define LAST_ENC_ROUND  FOUR_TABLES
#elif 0
#define LAST_ENC_ROUND  ONE_TABLE
#else
#define LAST_ENC_ROUND  NO_TABLES
#endif

#if 1   /* set tables for the normal decryption round */
#define DEC_ROUND   FOUR_TABLES
#elif 0
#define DEC_ROUND   ONE_TABLE
#else
#define DEC_ROUND   NO_TABLES
#endif

#if 1   /* set tables for the last decryption round */
#define LAST_DEC_ROUND  FOUR_TABLES
#elif 0
#define LAST_DEC_ROUND  ONE_TABLE
#else
#define LAST_DEC_ROUND  NO_TABLES
#endif

/*  The decryption key schedule can be speeded up with tables in the same
    way that the round functions can.  Include or exclude the following
    defines to set this requirement.
*/
#if 1
#define KEY_SCHED   FOUR_TABLES
#elif 0
#define KEY_SCHED   ONE_TABLE
#else
#define KEY_SCHED   NO_TABLES
#endif

/*  11. TABLE POINTER CACHING

    Normally tables are referenced directly, Enable this option if you wish to
    cache pointers to the tables in the encrypt/decrypt code.  Note that this
	only works if you are using FOUR_TABLES for the ROUND you enable this for.
*/
#if 1
#define ENC_ROUND_CACHE_TABLES
#endif
#if 1
#define LAST_ENC_ROUND_CACHE_TABLES
#endif
#if 1
#define DEC_ROUND_CACHE_TABLES
#endif
#if 1
#define LAST_DEC_ROUND_CACHE_TABLES
#endif

/* END OF CONFIGURATION OPTIONS */

#define RC_LENGTH   (5 * (AES_BLOCK_SIZE / 4 - 2))

/* Disable or report errors on some combinations of options */

#if ENC_ROUND == NO_TABLES && LAST_ENC_ROUND != NO_TABLES
#undef  LAST_ENC_ROUND
#define LAST_ENC_ROUND  NO_TABLES
#elif ENC_ROUND == ONE_TABLE && LAST_ENC_ROUND == FOUR_TABLES
#undef  LAST_ENC_ROUND
#define LAST_ENC_ROUND  ONE_TABLE
#endif

#if ENC_ROUND == NO_TABLES && ENC_UNROLL != NONE
#undef  ENC_UNROLL
#define ENC_UNROLL  NONE
#endif

#if DEC_ROUND == NO_TABLES && LAST_DEC_ROUND != NO_TABLES
#undef  LAST_DEC_ROUND
#define LAST_DEC_ROUND  NO_TABLES
#elif DEC_ROUND == ONE_TABLE && LAST_DEC_ROUND == FOUR_TABLES
#undef  LAST_DEC_ROUND
#define LAST_DEC_ROUND  ONE_TABLE
#endif

#if DEC_ROUND == NO_TABLES && DEC_UNROLL != NONE
#undef  DEC_UNROLL
#define DEC_UNROLL  NONE
#endif

/*  upr(x,n):  rotates bytes within words by n positions, moving bytes to
               higher index positions with wrap around into low positions
    ups(x,n):  moves bytes by n positions to higher index positions in
               words but without wrap around
    bval(x,n): extracts a byte from a word

    NOTE:      The definitions given here are intended only for use with
               unsigned variables and with shift counts that are compile
               time constants
*/

#if (ALGORITHM_BYTE_ORDER == BRG_LITTLE_ENDIAN)
#define upr(x,n)        (((aes_32t)(x) << (8 * (n))) | ((aes_32t)(x) >> (32 - 8 * (n))))
#define ups(x,n)        ((aes_32t) (x) << (8 * (n)))
#define bval(x,n)       ((aes_08t)((x) >> (8 * (n))))
#define bytes2word(b0, b1, b2, b3)  \
        (((aes_32t)(b3) << 24) | ((aes_32t)(b2) << 16) | ((aes_32t)(b1) << 8) | (b0))
#endif

#if (ALGORITHM_BYTE_ORDER == BRG_BIG_ENDIAN)
#define upr(x,n)        (((aes_32t)(x) >> (8 * (n))) | ((aes_32t)(x) << (32 - 8 * (n))))
#define ups(x,n)        ((aes_32t) (x) >> (8 * (n))))
#define bval(x,n)       ((aes_08t)((x) >> (24 - 8 * (n))))
#define bytes2word(b0, b1, b2, b3)  \
        (((aes_32t)(b0) << 24) | ((aes_32t)(b1) << 16) | ((aes_32t)(b2) << 8) | (b3))
#endif

#if defined(SAFE_IO)

#define word_in(x,c)    bytes2word(((aes_08t*)(x)+4*c)[0], ((aes_08t*)(x)+4*c)[1], \
                                   ((aes_08t*)(x)+4*c)[2], ((aes_08t*)(x)+4*c)[3])
#define word_out(x,c,v) { ((aes_08t*)(x)+4*c)[0] = bval(v,0); ((aes_08t*)(x)+4*c)[1] = bval(v,1); \
                          ((aes_08t*)(x)+4*c)[2] = bval(v,2); ((aes_08t*)(x)+4*c)[3] = bval(v,3); }

#elif (ALGORITHM_BYTE_ORDER == PLATFORM_BYTE_ORDER)

#define word_in(x,c)    (*((aes_32t*)(x)+(c)))
#define word_out(x,c,v) (*((aes_32t*)(x)+(c)) = (v))

#else

#define word_in(x,c)    aes_sw32(*((aes_32t*)(x)+(c)))
#define word_out(x,c,v) (*((aes_32t*)(x)+(c)) = aes_sw32(v))

#endif

/* the finite field modular polynomial and elements */

#define WPOLY   0x011b
#define BPOLY     0x1b

/* multiply four bytes in GF(2^8) by 'x' {02} in parallel */

#define m1  0x80808080
#define m2  0x7f7f7f7f
#define gf_mulx(x)  ((((x) & m2) << 1) ^ ((((x) & m1) >> 7) * BPOLY))

/* The following defines provide alternative definitions of gf_mulx that might
   give improved performance if a fast 32-bit multiply is not available. Note
   that a temporary variable u needs to be defined where gf_mulx is used.

#define gf_mulx(x) (u = (x) & m1, u |= (u >> 1), ((x) & m2) << 1) ^ ((u >> 3) | (u >> 6))
#define m4  (0x01010101 * BPOLY)
#define gf_mulx(x) (u = (x) & m1, ((x) & m2) << 1) ^ ((u - (u >> 7)) & m4)
*/

/* Work out which tables are needed for the different options   */

#if defined( AES_ASM )
#if defined( ENC_ROUND )
#undef  ENC_ROUND
#endif
#define ENC_ROUND   FOUR_TABLES
#if defined( LAST_ENC_ROUND )
#undef  LAST_ENC_ROUND
#endif
#define LAST_ENC_ROUND  FOUR_TABLES
#if defined( DEC_ROUND )
#undef  DEC_ROUND
#endif
#define DEC_ROUND   FOUR_TABLES
#if defined( LAST_DEC_ROUND )
#undef  LAST_DEC_ROUND
#endif
#define LAST_DEC_ROUND  FOUR_TABLES
#if defined( KEY_SCHED )
#undef  KEY_SCHED
#define KEY_SCHED   FOUR_TABLES
#endif
#endif

#if defined(ENCRYPTION) || defined(AES_ASM)
#if ENC_ROUND == ONE_TABLE
#define FT1_SET
#elif ENC_ROUND == FOUR_TABLES
#define FT4_SET
#else
#define SBX_SET
#endif
#if LAST_ENC_ROUND == ONE_TABLE
#define FL1_SET
#elif LAST_ENC_ROUND == FOUR_TABLES
#define FL4_SET
#elif !defined(SBX_SET)
#define SBX_SET
#endif
#endif

#if defined(DECRYPTION) || defined(AES_ASM)
#if DEC_ROUND == ONE_TABLE
#define IT1_SET
#elif DEC_ROUND == FOUR_TABLES
#define IT4_SET
#else
#define ISB_SET
#endif
#if LAST_DEC_ROUND == ONE_TABLE
#define IL1_SET
#elif LAST_DEC_ROUND == FOUR_TABLES
#define IL4_SET
#elif !defined(ISB_SET)
#define ISB_SET
#endif
#endif

#if defined(ENCRYPTION_KEY_SCHEDULE) || defined(DECRYPTION_KEY_SCHEDULE)
#if KEY_SCHED == ONE_TABLE
#define LS1_SET
#define IM1_SET
#elif KEY_SCHED == FOUR_TABLES
#define LS4_SET
#define IM4_SET
#elif !defined(SBX_SET)
#define SBX_SET
#endif
#endif

/* generic definitions of Rijndael macros that use tables    */

#define no_table(x,box,vf,rf,c) bytes2word( \
    box[bval(vf(x,0,c),rf(0,c))], \
    box[bval(vf(x,1,c),rf(1,c))], \
    box[bval(vf(x,2,c),rf(2,c))], \
    box[bval(vf(x,3,c),rf(3,c))])

#define one_table(x,op,tab,vf,rf,c) \
 (     tab[bval(vf(x,0,c),rf(0,c))] \
  ^ op(tab[bval(vf(x,1,c),rf(1,c))],1) \
  ^ op(tab[bval(vf(x,2,c),rf(2,c))],2) \
  ^ op(tab[bval(vf(x,3,c),rf(3,c))],3))

#define four_tables(x,tab,vf,rf,c) \
 (  tab[0][bval(vf(x,0,c),rf(0,c))] \
  ^ tab[1][bval(vf(x,1,c),rf(1,c))] \
  ^ tab[2][bval(vf(x,2,c),rf(2,c))] \
  ^ tab[3][bval(vf(x,3,c),rf(3,c))])

#define four_cached_tables(x,tab,vf,rf,c) \
(  tab##0[bval(vf(x,0,c),rf(0,c))] \
   ^ tab##1[bval(vf(x,1,c),rf(1,c))] \
   ^ tab##2[bval(vf(x,2,c),rf(2,c))] \
   ^ tab##3[bval(vf(x,3,c),rf(3,c))])

#define vf1(x,r,c)  (x)
#define rf1(r,c)    (r)
#define rf2(r,c)    ((8+r-c)&3)

/* perform forward and inverse column mix operation on four bytes in long word x in */
/* parallel. NOTE: x must be a simple variable, NOT an expression in these macros.  */

#if defined(FM4_SET)    /* not currently used */
#define fwd_mcol(x)     four_tables(x,t_use(f,m),vf1,rf1,0)
#elif defined(FM1_SET)  /* not currently used */
#define fwd_mcol(x)     one_table(x,upr,t_use(f,m),vf1,rf1,0)
#else
#define dec_fmvars      aes_32t g2
#define fwd_mcol(x)     (g2 = gf_mulx(x), g2 ^ upr((x) ^ g2, 3) ^ upr((x), 2) ^ upr((x), 1))
#endif

#if defined(IM4_SET)
#define inv_mcol(x)     four_tables(x,t_use(i,m),vf1,rf1,0)
#elif defined(IM1_SET)
#define inv_mcol(x)     one_table(x,upr,t_use(i,m),vf1,rf1,0)
#else
#define dec_imvars      aes_32t g2, g4, g9
#define inv_mcol(x)     (g2 = gf_mulx(x), g4 = gf_mulx(g2), g9 = (x) ^ gf_mulx(g4), g4 ^= g9, \
                        (x) ^ g2 ^ g4 ^ upr(g2 ^ g9, 3) ^ upr(g4, 2) ^ upr(g9, 1))
#endif

#if defined(FL4_SET)
#define ls_box(x,c)     four_tables(x,t_use(f,l),vf1,rf2,c)
#elif   defined(LS4_SET)
#define ls_box(x,c)     four_tables(x,t_use(l,s),vf1,rf2,c)
#elif defined(FL1_SET)
#define ls_box(x,c)     one_table(x,upr,t_use(f,l),vf1,rf2,c)
#elif defined(LS1_SET)
#define ls_box(x,c)     one_table(x,upr,t_use(l,s),vf1,rf2,c)
#else
#define ls_box(x,c)     no_table(x,t_use(s,box),vf1,rf2,c)
#endif

#endif	/* _CC_AESOPT_H */
