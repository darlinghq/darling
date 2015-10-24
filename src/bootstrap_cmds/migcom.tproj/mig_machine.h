#if !defined(_MIG_MACHINE_H)
#define _MIG_MACHINE_H

#define machine_alignment(SZ,ESZ)   \
(((SZ) = ((SZ) + 3) & ~3), (SZ) += (ESZ))

#define machine_padding(BYTES)  \
((BYTES & 3) ? (4 - (BYTES & 3)) : 0)

#ifndef NBBY
#define NBBY  8
#endif

#define PACK_MESSAGES TRUE

#endif
