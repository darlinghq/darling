/* Information pulled from:
 * "AppleSingle/AppleDouble Formats for Foreign Files Developer's Note"
 * (c) Apple Computer 1990
 * File assembled by Rob Braun (bbraun@synack.net)
 */
 
#ifndef __APPLEDOUBLE__
#define __APPLEDOUBLE__

#include <sys/types.h>
#include <stdint.h>

/* Structure of an AppleSingle file:
 *   ----------------------
 *   | AppleSingleHeader  |
 *   |--------------------|
 *   | ASH.entries # of   |
 *   | AppleSingleEntry   |
 *   | Descriptors        |
 *   |         1          |
 *   |         .          |
 *   |         .          |
 *   |         n          |
 *   |--------------------|
 *   |   Datablock 1      |
 *   |--------------------|
 *   |   Datablock 2      |
 *   |--------------------|
 *   |   Datablock n      |
 *   ----------------------
 */

struct AppleSingleHeader {
	uint32_t     magic;       /* Magic Number (0x00051600 for AS) */
	uint32_t     version;     /* Version #.  0x00020000 */
	char         filler[16];  /* All zeros */
	uint16_t     entries;     /* Number of entries in the file */
};

#define XAR_ASH_SIZE 26   /* sizeof(struct AppleSingleHeader) will be wrong
                           * due to padding. */

#define APPLESINGLE_MAGIC 0x00051600
#define APPLEDOUBLE_MAGIC 0x00051607

#define APPLESINGLE_VERSION 0x00020000
#define APPLEDOUBLE_VERSION 0x00020000

struct AppleSingleEntry {
	uint32_t     entry_id;    /* What the entry is.  See defines below */
	uint32_t     offset;      /* offset of data, offset beginning of file */
	uint32_t     length;      /* length of data.  can be 0 */
};

/* Valid entry_id values */
/* Entries 1, 3, and 8 are typically created for all files.
 * Macintosh Icon entries are rare, since those are typically in the resource 
 * fork.
 */
#define AS_ID_DATA       1  /* Data fork */
#define AS_ID_RESOURCE   2  /* Resource fork */
#define AS_ID_NAME       3  /* Name of the file */
#define AS_ID_COMMENT    4  /* Standard Macintosh comment */
#define AS_ID_BWICON     5  /* Standard Macintosh B&W icon */
#define AS_ID_COLORICON  6  /* Standard Macintosh Color icon */
/* There is no 7 */
#define AS_ID_DATES      8  /* File creation date, modification date, etc. */
#define AS_ID_FINDER     9  /* Finder Information */
#define AS_ID_MAC       10  /* Macintosh File information, attributes, etc. */
#define AS_ID_PRODOS    11  /* ProDOS file information */
#define AS_ID_MSDOS     12  /* MS-DOS file information */
#define AS_ID_SHORTNAME 13  /* AFP short name */
#define AS_ID_AFPINFO   14  /* AFP file information */
#define AS_ID_AFPDIR    15  /* AFP directory id */
/* 1-0x7FFFFFFF are reserved by Apple */

/* File Dates are stored as the # of seconds before or after
 * 12am Jan 1, 2000 GMT.  The default value is 0x80000000.
 */
struct MacTimes {
	uint32_t  creation;
	uint32_t  modification;
	uint32_t  backup;
	uint32_t  access;
};

/* Finder Information is two 16 byte quantities. 
 * Newly created files have all 0's in both entries.
 */

/* Macintosh File Info entry (10) a 32 bit bitmask. */

/* Entries can be placed in any order, although Apple recommends:
 * Place the data block (1) last.
 * Finder Info, File Dates Info, and Macintosh File Info first.
 * Allocate resource for entries in 4K blocks.
 */

/* AppleDouble files are simply AppleSingle files without the data fork.
 * The magic number is different as a read optimization. 
 */

#endif /* __APPLEDOUBLE__ */
