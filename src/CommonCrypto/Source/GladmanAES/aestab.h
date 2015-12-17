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
 Issue 28/01/2004

 This file contains the code for declaring the tables needed to implement
 AES. The file aesopt.h is assumed to be included before this header file.
 If there are no global variables, the definitions here can be used to put
 the AES tables in a structure so that a pointer can then be added to the
 AES context to pass them to the AES routines that need them.   If this
 facility is used, the calling program has to ensure that this pointer is
 managed appropriately.  In particular, the value of the t_dec(in,it) item
 in the table structure must be set to zero in order to ensure that the
 tables are initialised. In practice the three code sequences in aeskey.c
 that control the calls to gen_tabs() and the gen_tabs() routine itself will
 have to be changed for a specific implementation. If global variables are
 available it will generally be preferable to use them with the precomputed
 FIXED_TABLES option that uses static global tables.

 The following defines can be used to control the way the tables
 are defined, initialised and used in embedded environments that
 require special features for these purposes

    the 't_dec' construction is used to declare fixed table arrays
    the 't_set' construction is used to set fixed table values
    the 't_use' construction is used to access fixed table values

    256 byte tables:

        t_xxx(s,box)    => forward S box
        t_xxx(i,box)    => inverse S box

    256 32-bit word OR 4 x 256 32-bit word tables:

        t_xxx(f,n)      => forward normal round
        t_xxx(f,l)      => forward last round
        t_xxx(i,n)      => inverse normal round
        t_xxx(i,l)      => inverse last round
        t_xxx(l,s)      => key schedule table
        t_xxx(i,m)      => key schedule table

    Other variables and tables:

        t_xxx(r,c)      => the rcon table
*/

#if !defined( _CC_AESTAB_H )
#define _CC_AESTAB_H

#define t_dec(m,n) t_##m##n
#define t_set(m,n) t_##m##n
#define t_use(m,n) t_##m##n

#if defined(FIXED_TABLES)
#define Const const
#else
#define Const
#endif

#if defined(DO_TABLES)
#define Extern
#else
#define Extern extern
#endif

#if defined(_MSC_VER) && defined(TABLE_ALIGN)
#define Align __declspec(align(TABLE_ALIGN))
#else
#define Align
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

#if defined(DO_TABLES) && defined(FIXED_TABLES)
#define d_1(t,n,b,e)       Align Const t n[256]    =   b(e)
#define d_4(t,n,b,e,f,g,h) Align Const t n[4][256] = { b(e), b(f), b(g), b(h) }
Extern Align Const aes_32t t_dec(r,c)[RC_LENGTH] = rc_data(w0);
#else
#define d_1(t,n,b,e)       Extern Align Const t n[256]
#define d_4(t,n,b,e,f,g,h) Extern Align Const t n[4][256]
Extern Align Const aes_32t t_dec(r,c)[RC_LENGTH];
#endif

#if defined( SBX_SET )
    d_1(aes_08t, t_dec(s,box), sb_data, h0);
#endif
#if defined( ISB_SET )
    d_1(aes_08t, t_dec(i,box), isb_data, h0);
#endif

#if defined( FT1_SET )
    d_1(aes_32t, t_dec(f,n), sb_data, u0);
#endif
#if defined( FT4_SET )
    d_4(aes_32t, t_dec(f,n), sb_data, u0, u1, u2, u3);
#endif

#if defined( FL1_SET )
    d_1(aes_32t, t_dec(f,l), sb_data, w0);
#endif
#if defined( FL4_SET )
    d_4(aes_32t, t_dec(f,l), sb_data, w0, w1, w2, w3);
#endif

#if defined( IT1_SET )
    d_1(aes_32t, t_dec(i,n), isb_data, v0);
#endif
#if defined( IT4_SET )
    d_4(aes_32t, t_dec(i,n), isb_data, v0, v1, v2, v3);
#endif

#if defined( IL1_SET )
    d_1(aes_32t, t_dec(i,l), isb_data, w0);
#endif
#if defined( IL4_SET )
    d_4(aes_32t, t_dec(i,l), isb_data, w0, w1, w2, w3);
#endif

#if defined( LS1_SET )
#if defined( FL1_SET )
#undef  LS1_SET
#else
    d_1(aes_32t, t_dec(l,s), sb_data, w0);
#endif
#endif

#if defined( LS4_SET )
#if defined( FL4_SET )
#undef  LS4_SET
#else
    d_4(aes_32t, t_dec(l,s), sb_data, w0, w1, w2, w3);
#endif
#endif

#if defined( IM1_SET )
    d_1(aes_32t, t_dec(i,m), mm_data, v0);
#endif
#if defined( IM4_SET )
    d_4(aes_32t, t_dec(i,m), mm_data, v0, v1, v2, v3);
#endif

#if defined(__cplusplus)
}
#endif

#endif	/* _CC_AESTAB_H */
