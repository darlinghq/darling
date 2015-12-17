#ifndef _SKEIN_IV_H_
#define _SKEIN_IV_H_

#include "skein.h"    /* get Skein macros and types */

/*
***************** Pre-computed Skein IVs *******************
**
** NOTE: these values are not "magic" constants, but
** are generated using the Threefish block function.
** They are pre-computed here only for speed; i.e., to
** avoid the need for a Threefish call during Init().
**
** The IV for any fixed hash length may be pre-computed.
** Only the most common values are included here.
**
************************************************************
**/

#define MK_64 SKEIN_MK_64

/* blkSize =  256 bits. hashSize =  128 bits */
const u64b_t SKEIN_256_IV_128[] =
    {
    MK_64(0x46B39C3A,0xAA418D4F),
    MK_64(0x681229DD,0x06920827),
    MK_64(0xCBE067C9,0x78460238),
    MK_64(0xC388A1B7,0x4EC45EF3)
    };

/* blkSize =  256 bits. hashSize =  160 bits */
const u64b_t SKEIN_256_IV_160[] =
    {
    MK_64(0xD51846B9,0xDAE51FBB),
    MK_64(0x7D47BABD,0x6205526D),
    MK_64(0xA1A8703E,0x47B89F20),
    MK_64(0xB97D7234,0xC5927589)
    };

/* blkSize =  256 bits. hashSize =  224 bits */
const u64b_t SKEIN_256_IV_224[] =
    {
    MK_64(0xFE6720F4,0x5ED90A57),
    MK_64(0x352D51F3,0xB01B6FBC),
    MK_64(0xD764B04F,0x1785F14E),
    MK_64(0xE7F24611,0xDDD59B27)
    };

/* blkSize =  256 bits. hashSize =  256 bits */
const u64b_t SKEIN_256_IV_256[] =
    {
    MK_64(0x164290A9,0xD4EEEF1D),
    MK_64(0x8E7EAF44,0xB1B0CD15),
    MK_64(0xA8BA0822,0xF69D09AE),
    MK_64(0x0AF25C5E,0x364A6468)
    };

/* blkSize =  512 bits. hashSize =  128 bits */
const u64b_t SKEIN_512_IV_128[] =
    {
    MK_64(0x51AF0A1B,0x97A7DA9C),
    MK_64(0xEC77F8A5,0xF4C6004C),
    MK_64(0x0BB7182C,0x25CA1F6E),
    MK_64(0x1B22A2CB,0x9F9339C5),
    MK_64(0xC905E0A4,0x31216AA4),
    MK_64(0xAEE4D5D0,0xBD378696),
    MK_64(0x92744A50,0x1953D08A),
    MK_64(0x2DCAD6F9,0x85777E17)
    };

/* blkSize =  512 bits. hashSize =  160 bits */
const u64b_t SKEIN_512_IV_160[] =
    {
    MK_64(0x9A73479A,0xC7701247),
    MK_64(0xD657FBF8,0xFDE0DA1A),
    MK_64(0xB1EE72A6,0xB04DA375),
    MK_64(0xE87ED2A1,0xC20605B8),
    MK_64(0x220A0EFA,0x9B925E17),
    MK_64(0x6D72A217,0xEAF0B419),
    MK_64(0x6CD72290,0xAA33FA72),
    MK_64(0x5829089E,0x759C4256)
    };

/* blkSize =  512 bits. hashSize =  224 bits */
const u64b_t SKEIN_512_IV_224[] =
    {
    MK_64(0x10C55045,0x6BF94560),
    MK_64(0x59004AF1,0xF558ACCC),
    MK_64(0x82BD1BF9,0xB7461DFD),
    MK_64(0x46B0F3A4,0x7C2AF60E),
    MK_64(0xECC8498C,0xE80A8DCA),
    MK_64(0x50A1DA33,0x10C836EF),
    MK_64(0x3538F92A,0x39165A80),
    MK_64(0x896A4329,0xCD5DCF2A)
    };

/* blkSize =  512 bits. hashSize =  256 bits */
const u64b_t SKEIN_512_IV_256[] =
    {
    MK_64(0x85A195B1,0x8B2264EC),
    MK_64(0x7A6DAC64,0xC047C2B0),
    MK_64(0xE1A21465,0xEE3FE124),
    MK_64(0x1D211735,0x6504425A),
    MK_64(0xC962DC0F,0xC0046F2C),
    MK_64(0x8D5A3E90,0x4B1BE9C8),
    MK_64(0xAFB7174B,0xBD8FEEE9),
    MK_64(0x7FE63D9B,0xF94EDEB8)
    };

/* blkSize =  512 bits. hashSize =  384 bits */
const u64b_t SKEIN_512_IV_384[] =
    {
    MK_64(0x755C4957,0x16D7512B),
    MK_64(0xB4587127,0x14DF4CEF),
    MK_64(0x677D2E8C,0x027C060A),
    MK_64(0x8DA4F592,0x05232716),
    MK_64(0xCE454B58,0xC445AD7F),
    MK_64(0x23048344,0xACA8BC96),
    MK_64(0xF719BCC3,0x38768323),
    MK_64(0xD77E3686,0x50579DEC)
    };

/* blkSize =  512 bits. hashSize =  512 bits */
const u64b_t SKEIN_512_IV_512[] =
    {
    MK_64(0x1A9A721C,0x8A265CA5),
    MK_64(0xC9ABACF5,0xAA853978),
    MK_64(0x4AF6652A,0xB80A2883),
    MK_64(0x66F5E8A8,0x09A773C7),
    MK_64(0x7FA984B7,0x81BAAF5B),
    MK_64(0x0FE5D2D9,0x3233F397),
    MK_64(0x6E29F932,0xDCB412D7),
    MK_64(0xD40CD947,0x2F225C23)
    };

/* blkSize = 1024 bits. hashSize =  384 bits */
const u64b_t SKEIN1024_IV_384[] =
    {
    MK_64(0x9E887D47,0x2693F556),
    MK_64(0xF4553A5A,0xB3A902D8),
    MK_64(0x60A10790,0x28E4504E),
    MK_64(0x96FAA39D,0x943F8ABE),
    MK_64(0x2A769D27,0x828A22A7),
    MK_64(0xB2F274F5,0xB2C3A833),
    MK_64(0xC722C052,0x47F09222),
    MK_64(0x377C4A92,0xEE78B216),
    MK_64(0x97CFE7B2,0x039F4C9D),
    MK_64(0xC864ACFA,0xC83C8364),
    MK_64(0x73F26579,0x1D3CF723),
    MK_64(0x2464DC1E,0x5E327F97),
    MK_64(0x135D3954,0xF181CB1A),
    MK_64(0x244BBF13,0x24C5C669),
    MK_64(0xE1E258BC,0x446662E3),
    MK_64(0xCF1E0F47,0x934A469C)
    };

/* blkSize = 1024 bits. hashSize =  512 bits */
const u64b_t SKEIN1024_IV_512[] =
    {
    MK_64(0x76066F1F,0x612DD519),
    MK_64(0xD9B93D95,0x75D90191),
    MK_64(0x582D15EA,0x89696586),
    MK_64(0x4F1CA328,0xB5F10FB3),
    MK_64(0x686C454D,0xEC64B419),
    MK_64(0x2D7BD9B4,0x026EDABE),
    MK_64(0xEF346195,0x1ACD05C4),
    MK_64(0x1759E898,0x4446E275),
    MK_64(0xACFC075A,0xE724456D),
    MK_64(0x82F35D0A,0xE7704311),
    MK_64(0x99D0B103,0x9AD7E344),
    MK_64(0x85D6C81D,0x29F6204B),
    MK_64(0x0CA2A987,0x5D57632A),
    MK_64(0x069A8931,0x47A448FA),
    MK_64(0x3C42FB50,0x02815320),
    MK_64(0xF7E22C15,0x953E3125)
    };

/* blkSize = 1024 bits. hashSize = 1024 bits */
const u64b_t SKEIN1024_IV_1024[] =
    {
    MK_64(0x495E85B9,0x53876965),
    MK_64(0x1E3D5C1B,0x41E754EF),
    MK_64(0x23725455,0x2E9C10C7),
    MK_64(0x0B00AAB4,0xFA441407),
    MK_64(0x17DDA56A,0xA106337C),
    MK_64(0xF98200E9,0xCAE13F94),
    MK_64(0xF2DF7F00,0xADFF12BF),
    MK_64(0xA92673D0,0xD0CA7AD9),
    MK_64(0xC0DD64B0,0x4B27ED98),
    MK_64(0x87C36A6C,0xA0A26F90),
    MK_64(0x640C8526,0xD0850A10),
    MK_64(0x6EBFAD0C,0x93DA09AE),
    MK_64(0x617E3BCD,0xDEE4A85F),
    MK_64(0x05A4A1A7,0xD82737B7),
    MK_64(0x002BAF2C,0x3EB13D30),
    MK_64(0x28527A78,0xC83D554C)
    };

#endif /* _SKEIN_IV_H_ */
